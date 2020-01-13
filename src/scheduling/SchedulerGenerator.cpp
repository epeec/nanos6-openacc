/*
	This file is part of Nanos6 and is licensed under the terms contained in the COPYING file.
	
	Copyright (C) 2015-2019 Barcelona Supercomputing Center (BSC)
*/

#include "SchedulerGenerator.hpp"
#include "lowlevel/FatalErrorHandler.hpp"
#include "scheduling/schedulers/HostScheduler.hpp"
#include "scheduling/schedulers/device/CUDADeviceScheduler.hpp"
#include "scheduling/schedulers/device/FPGADeviceScheduler.hpp"
#include "scheduling/schedulers/device/OpenAccDeviceScheduler.hpp"

HostScheduler *SchedulerGenerator::createHostScheduler(SchedulingPolicy policy, bool enablePriority, bool enableImmediateSuccessor)
{
	return new HostScheduler(policy, enablePriority, enableImmediateSuccessor);
}

DeviceScheduler *SchedulerGenerator::createDeviceScheduler(SchedulingPolicy policy, bool enablePriority, bool enableImmediateSuccessor, nanos6_device_t deviceType)
{
	switch(deviceType) {
		case nanos6_cuda_device:
			return new CUDADeviceScheduler(policy, enablePriority, enableImmediateSuccessor, deviceType);
		case nanos6_openacc_device:
			return new OpenAccDeviceScheduler(policy, enablePriority, enableImmediateSuccessor, deviceType);
		case nanos6_opencl_device:
			FatalErrorHandler::failIf(1, "OpenCL is not supported yet.");
			break;
		case nanos6_fpga_device:
			return new FPGADeviceScheduler(policy, enablePriority, enableImmediateSuccessor, deviceType);
		case nanos6_cluster_device:
			FatalErrorHandler::failIf(1, "Cluster is not actually a device.");
			break;
		default:
			FatalErrorHandler::failIf(1, "Unknown or unsupported device.");
	}
	return nullptr;
}

