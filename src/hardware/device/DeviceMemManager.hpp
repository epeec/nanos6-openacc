/*
	This file is part of Nanos6 and is licensed under the terms contained in the COPYING file.

	Copyright (C) 2020 Barcelona Supercomputing Center (BSC)
*/

#ifndef DEVICE_MEM_MANAGER_HPP
#define DEVICE_MEM_MANAGER_HPP

#include <map>

#include "hardware/device/cuda/CUDAFunctions.hpp"
#include "lowlevel/RWSpinLock.hpp"

class DeviceMemManager {

	struct DeviceMemEntry {
		size_t size;
		int deviceNum;
	};

private:
	static std::map<void *, DeviceMemEntry> _directory;

	static RWSpinLock _lock;

	static const size_t totalDevices;

public:
	static void initialize()
	{
	}

	static void shutdown()
	{
	}

	// Allocate Unified Memory (host+device at once) and add an entry in _directory
	static void *alloc(size_t size, int deviceNum);

	// Free symbol from Unified Memory and delete its entry from _directory
	static void free(void *ptr);

	// Check if the symbol referenced is contained in _directory
	static bool contains(void *ptr);

	// Get _directory entry data of a given pointer-symbol
	static DeviceMemEntry getEntry(void *ptr);

	// For a given pointer-symbol, return the device number it has been allocated on
	// (this is a wrapper to avoid redundant handling of the getEntry result)
	static inline int getDataLocation(void *ptr)
	{
		return getEntry(ptr).deviceNum;
	}

};

#endif // DEVICE_MEM_MANAGER_HPP

