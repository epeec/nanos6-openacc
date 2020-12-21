/*
	This file is part of Nanos6 and is licensed under the terms contained in the COPYING file.

	Copyright (C) 2019-2020 Barcelona Supercomputing Center (BSC)
*/

#ifndef DEVICE_UNSYNC_SCHEDULER_HPP
#define DEVICE_UNSYNC_SCHEDULER_HPP

#include "scheduling/schedulers/UnsyncScheduler.hpp"

class DeviceUnsyncScheduler : public UnsyncScheduler {
private:
	size_t _totalDevices;
	Container::vector<ReadyQueue *> _readyTasksDevice;

public:
	DeviceUnsyncScheduler(
		SchedulingPolicy policy,
		bool enablePriority,
		bool enableImmediateSuccessor,
		size_t totalDevices
	);

	virtual ~DeviceUnsyncScheduler();

	//! \brief Get a ready task for execution
	//!
	//! \param[in] computePlace the hardware place asking for scheduling orders
	//!
	//! \returns a ready task or nullptr
	Task *getReadyTask(ComputePlace *computePlace);

	//! \brief Add a (ready) task that has been created or freed
	//!
	//! \param[in] task the task to be added
	//! \param[in] computePlace the hardware place of the creator or the liberator
	//! \param[in] hint a hint about the relation of the task to the current task
	void addReadyTask(Task *task, ComputePlace *computePlace, ReadyTaskHint hint) override;

};


#endif // DEVICE_UNSYNC_SCHEDULER_HPP
