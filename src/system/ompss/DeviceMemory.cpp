/*
	This file is part of Nanos6 and is licensed under the terms contained in the COPYING file.

	Copyright (C) 2015-2020 Barcelona Supercomputing Center (BSC)
*/

/* Currently only working with NVIDIA GPUS (intended for CUDA + OpenACC use) */

#include "hardware/device/DeviceMemManager.hpp"

extern "C"
void *nanos6_device_alloc(size_t size, int device_num)
{
	return DeviceMemManager::alloc(size, device_num);
}

extern "C"
void nanos6_device_free(void *ptr)
{
	DeviceMemManager::free(ptr);
}

