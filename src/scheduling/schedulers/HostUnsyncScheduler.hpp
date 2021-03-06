/*
	This file is part of Nanos6 and is licensed under the terms contained in the COPYING file.

	Copyright (C) 2019 Barcelona Supercomputing Center (BSC)
*/

#ifndef HOST_UNSYNC_SCHEDULER_HPP
#define HOST_UNSYNC_SCHEDULER_HPP

#include "MemoryAllocator.hpp"
#include "scheduling/schedulers/UnsyncScheduler.hpp"

class Taskfor;

class HostUnsyncScheduler : public UnsyncScheduler {
	std::vector<Taskfor *> _groupSlots;

public:
	HostUnsyncScheduler(SchedulingPolicy policy, bool enablePriority, bool enableImmediateSuccessor)
		: UnsyncScheduler(policy, enablePriority, enableImmediateSuccessor)
	{
		size_t groups = CPUManager::getNumTaskforGroups();

		_groupSlots = std::vector<Taskfor *>(groups, nullptr);

		if (enableImmediateSuccessor) {
			_immediateSuccessorTaskfors = std::vector<Task *>(groups*2, nullptr);
		}
	}

	virtual ~HostUnsyncScheduler()
	{}

	//! \brief Get a ready task for execution
	//!
	//! \param[in] computePlace The hardware place asking for scheduling orders
	//!
	//! \returns A ready task or nullptr
	Task *getReadyTask(ComputePlace *computePlace);

	//! \brief Check if the scheduler has available work for the current CPU
	//!
	//! \param[in] computePlace The host compute place
	bool hasAvailableWork(ComputePlace *computePlace);

};

#endif // HOST_UNSYNC_SCHEDULER_HPP
