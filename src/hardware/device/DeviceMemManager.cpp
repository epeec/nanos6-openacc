/*
	This file is part of Nanos6 and is licensed under the terms contained in the COPYING file.

	Copyright (C) 2020 Barcelona Supercomputing Center (BSC)
*/

#include <algorithm>

#include "DeviceMemManager.hpp"

#include "hardware/HardwareInfo.hpp"

#include <DataAccessRegistration.hpp>
#include <DataAccessRegistrationImplementation.hpp>

const size_t DeviceMemManager::totalDevices = CUDAFunctions::getDeviceCount();
RWSpinLock DeviceMemManager::_lock;
std::map<void *, DeviceMemManager::DeviceMemEntry> DeviceMemManager::_directory;

void *DeviceMemManager::alloc(size_t size, int deviceNum)
{
	assert((size > 0) && (deviceNum >= 0) && (totalDevices > 0));

	// deviceNum temporarily will be just (deviceNum % totalDevices)

	// Our very sophisticated hash...
	deviceNum = deviceNum % (int)totalDevices;

	// For starters, only play with CUDAAttached mode; Global is automatic in simple malloc
	// substitution by PGI + UnifiedMemory flag.
	void *ret = CUDAFunctions::mallocManaged(size, deviceNum, CUDAFunctions::CUDAAttached);
	// CUDAErrorHandler will abort in case of failure; assume correct result

	DeviceMemEntry entry = {size, deviceNum};
	_lock.writeLock();
	_directory.insert(std::make_pair(ret, entry));
	_lock.writeUnlock();

	return ret;
}

void DeviceMemManager::free(void *ptr)
{
	assert(contains(ptr));

	_lock.writeLock();
	_directory.erase(ptr);
	_lock.writeUnlock();
	CUDAFunctions::free(ptr);
}

bool DeviceMemManager::contains(void *ptr)
{
	assert(ptr != nullptr);

	_lock.readLock();
	bool ret = (_directory.find(ptr) != _directory.end());
	_lock.readUnlock();

	return ret;
}

DeviceMemManager::DeviceMemEntry DeviceMemManager::getEntry(void *ptr)
{
	assert(ptr != nullptr);
	DeviceMemEntry ret = {0, 0};

	_lock.readLock();
	auto it = _directory.find(ptr);
	_lock.readUnlock();

	if (it != _directory.end()) {
		ret = {it->second.size, it->second.deviceNum};
	}

	return ret;
}

size_t DeviceMemManager::computeDeviceAffinity(Task *task)
{
	size_t deviceScore[totalDevices] = { };

	DataAccessRegistration::processAllDataAccesses(task,
		[&](const DataAccess *access) -> bool {
			if (access->getType() != REDUCTION_ACCESS_TYPE && !access->isWeak()) {
				void *address = access->getAccessRegion().getStartAddress();
				if (contains(address)) {
					int device = getDataLocation(address);
					size_t accessSize = access->getAccessRegion().getSize();
					deviceScore[device] += accessSize;
				}
			}
			return true;
		}
	);

	size_t ret = std::distance(deviceScore, std::max_element(deviceScore, deviceScore + totalDevices));
	return ret;

}

