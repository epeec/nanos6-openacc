/*
	This file is part of Nanos6 and is licensed under the terms contained in the COPYING file.

	Copyright (C) 2019 Barcelona Supercomputing Center (BSC)
*/

#ifndef OPENACC_DEVICE_SCHEDULER_HPP
#define OPENACC_DEVICE_SCHEDULER_HPP

#include "DeviceScheduler.hpp"
#include "hardware/places/DeviceComputePlace.hpp"
#include "hardware/device/DeviceInfoImplementation.hpp"

class OpenAccDeviceScheduler : public DeviceScheduler {
	size_t _totalDevices;
	std::vector<ComputePlace *> _cpuToDevice;

public:
	OpenAccDeviceScheduler(SchedulingPolicy policy, bool enablePriority, bool enableImmediateSuccessor, nanos6_device_t deviceType) :
		DeviceScheduler(policy, enablePriority, enableImmediateSuccessor, deviceType),
		_cpuToDevice(_totalCPUs, nullptr)
	{
		DeviceInfoImplementation *deviceInfo = static_cast<DeviceInfoImplementation*>(HardwareInfo::getDeviceInfo(deviceType));
		assert(deviceInfo != nullptr);

		// OpenACC has a single subtype for now, but we could maybe explore this...
		Device *subDeviceType = deviceInfo->getDevice(0);
		assert(subDeviceType != nullptr);

		_totalDevices = subDeviceType->getNumDevices();
	}

	virtual ~OpenAccDeviceScheduler()
	{}

	Task *getReadyTask(ComputePlace *computePlace, ComputePlace *deviceComputePlace)
	{
		assert(deviceComputePlace != nullptr);
		assert(deviceComputePlace->getType() == _deviceType);

		Task *result = getTask(computePlace, deviceComputePlace);
		assert(result == nullptr || result->getDeviceType() == _deviceType);
		return result;
	}

	inline std::string getName() const
	{
		return "OpenAccDeviceScheduler";
	}

protected:
	inline ComputePlace *getRelatedComputePlace(uint64_t cpuIndex) const
	{
		return _cpuToDevice[cpuIndex];
	}

	inline void setRelatedComputePlace(uint64_t cpuIndex, ComputePlace *computePlace)
	{
		assert(computePlace == nullptr || computePlace->getType() == _deviceType);
		_cpuToDevice[cpuIndex] = computePlace;
	}
};

#endif // OPENACC_DEVICE_SCHEDULER_HPP

