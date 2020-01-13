/*
	This file is part of Nanos6 and is licensed under the terms contained in the COPYING file.

	Copyright (C) 2015-2019 Barcelona Supercomputing Center (BSC)
*/

#ifndef SCHEDULER_INTERFACE_HPP
#define SCHEDULER_INTERFACE_HPP

#include "hardware/places/ComputePlace.hpp"
#include "scheduling/schedulers/HostScheduler.hpp"
#include "scheduling/schedulers/device/CUDADeviceScheduler.hpp"
#include "scheduling/schedulers/device/DeviceScheduler.hpp"
#include "scheduling/schedulers/device/FPGADeviceScheduler.hpp"
#include "scheduling/schedulers/device/OpenAccDeviceScheduler.hpp"
#include "tasks/Task.hpp"
#include "tasks/TaskImplementation.hpp"

#include <HardwareCounters.hpp>
#include <InstrumentTaskStatus.hpp>
#include <Monitoring.hpp>

class SchedulerInterface {
	HostScheduler *_hostScheduler;
	DeviceScheduler *_deviceSchedulers[nanos6_device_type_num];

public:
	SchedulerInterface();
	virtual ~SchedulerInterface();

	virtual inline void addReadyTask(Task *task, ComputePlace *computePlace, ReadyTaskHint hint = NO_HINT)
	{
		nanos6_device_t taskType = (nanos6_device_t) task->getDeviceType();
		assert(taskType != nanos6_cluster_device);

		if (taskType == nanos6_host_device) {
			_hostScheduler->addReadyTask(task, computePlace, hint);
		} else {
			assert(taskType == _deviceSchedulers[taskType]->getDeviceType());
			_deviceSchedulers[taskType]->addReadyTask(task, computePlace, hint);
		}
	}

	virtual inline Task *getReadyTask(ComputePlace *computePlace, ComputePlace *deviceComputePlace = nullptr)
	{
		assert(computePlace == nullptr || computePlace->getType() == nanos6_host_device);
		nanos6_device_t computePlaceType = (deviceComputePlace == nullptr) ? nanos6_host_device : deviceComputePlace->getType();

		if (computePlaceType == nanos6_host_device) {
			return _hostScheduler->getReadyTask(computePlace);
		} else {
			assert(deviceComputePlace->getType() != nanos6_cluster_device);
			return _deviceSchedulers[computePlaceType]->getReadyTask(computePlace, deviceComputePlace);
		}
	}

	//! \brief Check if the scheduler has available work for the current CPU
	//!
	//! \param[in] computePlace The host compute place
	//! \param[in] deviceComputePlace The target device compute place if it exists
	virtual bool hasAvailableWork(ComputePlace *computePlace, ComputePlace *deviceComputePlace = nullptr)
	{
		assert(computePlace != nullptr);
		assert(computePlace->getType() == nanos6_host_device);
		nanos6_device_t computePlaceType = (deviceComputePlace == nullptr) ?
			nanos6_host_device : deviceComputePlace->getType();

		if (computePlaceType == nanos6_host_device) {
			return _hostScheduler->hasAvailableWork(computePlace);
		} else {
			assert(deviceComputePlace->getType() != nanos6_cluster_device);
			return _deviceSchedulers[computePlaceType]->hasAvailableWork(deviceComputePlace);
		}
	}

	//! \brief Notify the scheduler that a CPU is about to be disabled
	//! in case any tasks must be unassigned
	//!
	//! \param[in] cpuId The id of the cpu that will be disabled
	//! \param[in] task A task assigned to the current thread or nullptr
	//!
	//! \return Whether work was reassigned upon disabling this CPU
	inline bool disablingCPU(size_t cpuId, Task *task)
	{
		return _hostScheduler->disablingCPU(cpuId, task);
	}

	virtual std::string getName() const = 0;
};

#endif // SCHEDULER_INTERFACE_HPP
