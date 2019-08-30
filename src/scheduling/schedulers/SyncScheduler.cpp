/*
	This file is part of Nanos6 and is licensed under the terms contained in the COPYING file.
	
	Copyright (C) 2019 Barcelona Supercomputing Center (BSC)
*/


#include "SyncScheduler.hpp"
#include "scheduling/schedulers/device/DeviceScheduler.hpp"
#include "scheduling/schedulers/device/SubDeviceScheduler.hpp"

Task *SyncScheduler::getTask(ComputePlace *computePlace, ComputePlace *deviceComputePlace, bool device, bool subdevice)
{
	assert(!(device && subdevice));
	
	Task *task = nullptr;
	ComputePlace *deviceComputePlaceOrComputePlace =
		(deviceComputePlace != nullptr) ? deviceComputePlace : computePlace;
	
	// Special case for device polling services that get ready tasks.
	if (computePlace == nullptr) {
		_lock.lock();
		task = _scheduler->getReadyTask(deviceComputePlaceOrComputePlace);
		_lock.unsubscribe();
		assert(task == nullptr || task->isRunnable());
		return task;
	}
	
	assert(computePlace != nullptr);
	assert(computePlace->getType() == nanos6_host_device);
	
	uint64_t const cpuIndex = computePlace->getIndex();
	if (device) {
		((DeviceScheduler *)this)->setCPUToDevice(cpuIndex, deviceComputePlace);
	} else if (subdevice) {
		((SubDeviceScheduler *)this)->setCPUToSubDevice(cpuIndex, deviceComputePlace);
	}
	
	// Subscribe to the lock.
	uint64_t const ticket = _lock.subscribeOrLock(cpuIndex);
	
	if (getAssignedTask(cpuIndex, ticket, task)) {
		// Someone got the lock and gave me work to do.
		assert(task->isRunnable());
		return task;
	}
	
	// I own the lock!
	// First of all, get all the tasks in the addQueues into the ready queue.
	processReadyTasks();
	
	uint64_t cpu;
	uint64_t i = ticket + 1;
	const std::vector<CPU *> &computePlaces = CPUManager::getCPUListReference();
	
	// Serve all the subscribers, while there is work to give them.
	while (_lock.popWaitingCPU(i, cpu)) {
		ComputePlace *resultComputePlace = nullptr;
		if (device) {
			resultComputePlace = ((DeviceScheduler *)this)->getCPUToDevice(cpuIndex);
		} else if (subdevice) {
			resultComputePlace = ((SubDeviceScheduler *)this)->getCPUToSubDevice(cpuIndex);
		} else {
			resultComputePlace = computePlaces[cpu];
		}
		assert(resultComputePlace != nullptr);
		
		task = _scheduler->getReadyTask(resultComputePlace);
		if (task == nullptr)
			break;
		
		assert(task->isRunnable());
		
		if (device) {
			((DeviceScheduler *)this)->setCPUToDevice(cpuIndex, nullptr);
		} else if (subdevice) {
			((SubDeviceScheduler *)this)->setCPUToSubDevice(cpuIndex, nullptr);
		}
		
		// Put a task into the subscriber slot.
		assignTask(cpu, i, task);
		
		// Advance the ticket of the subscriber just served.
		_lock.unsubscribe();
		i++;
	}
	
	// No more subscribers. Try to get work for myself.
	task = _scheduler->getReadyTask(deviceComputePlaceOrComputePlace);
	_lock.unsubscribe();
	
	assert(task == nullptr || task->isRunnable());
	return task;
}