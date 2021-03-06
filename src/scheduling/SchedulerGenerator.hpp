/*
	This file is part of Nanos6 and is licensed under the terms contained in the COPYING file.
	
	Copyright (C) 2015-2019 Barcelona Supercomputing Center (BSC)
*/

#ifndef SCHEDULER_GENERATOR_HPP
#define SCHEDULER_GENERATOR_HPP

#include <nanos6/task-instantiation.h>

#include "scheduling/ReadyQueue.hpp"

class DeviceScheduler;
class HostScheduler;

class SchedulerGenerator {
public:
	static HostScheduler *createHostScheduler(SchedulingPolicy policy, bool enablePriority, bool enableImmediateSuccessor);
	static DeviceScheduler *createDeviceScheduler(SchedulingPolicy policy, bool enablePriority, bool enableImmediateSuccessor, nanos6_device_t deviceType);
};


#endif // SCHEDULER_GENERATOR_HPP
