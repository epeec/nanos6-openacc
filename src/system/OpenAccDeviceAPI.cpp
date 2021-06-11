/*
	This file is part of Nanos6 and is licensed under the terms contained in the COPYING file.

	Copyright (C) 2021 Barcelona Supercomputing Center (BSC)
*/

#include "hardware/device/openacc/OpenAccAccelerator.hpp"
#include "tasks/Task.hpp"

#include <nanos6/openacc_device.h>

// If called from the context of an OpenACC device outlined task function,
// returns the async queue ID the runtime has allocated to the task.
extern "C"
int nanos6_get_current_acc_queue(void)
{
	Task *currentTask = OpenAccAccelerator::getCurrentTask();
	nanos6_openacc_device_environment_t &env =	currentTask->getDeviceEnvironment().openacc;
	return env.asyncId;
}

