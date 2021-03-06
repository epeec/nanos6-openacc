/*
	This file is part of Nanos6 and is licensed under the terms contained in the COPYING file.

	Copyright (C) 2015-2019 Barcelona Supercomputing Center (BSC)
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef TASK_IMPLEMENTATION_HPP
#define TASK_IMPLEMENTATION_HPP

#include "StreamExecutor.hpp"
#include "Task.hpp"


#include <DataAccessRegistration.hpp>
#include <InstrumentTaskId.hpp>

#ifdef DISCRETE_DEPS
#include <TaskDataAccesses.hpp>
#define __nondiscrete_unused
#else
#include <TaskDataAccessesImplementation.hpp>
#define __nondiscrete_unused __attribute__((unused))
#endif

#include <cstring>

inline Task::Task(
	void *argsBlock,
	size_t argsBlockSize,
	nanos6_task_info_t *taskInfo,
	nanos6_task_invocation_info_t *taskInvokationInfo,
	Task *parent,
	Instrument::task_id_t instrumentationTaskId,
	size_t flags,
	__nondiscrete_unused void * seqs,
	__nondiscrete_unused void * addresses,
	__nondiscrete_unused size_t numDeps
)
	: _argsBlock(argsBlock),
	_argsBlockSize(argsBlockSize),
	_taskInfo(taskInfo),
	_taskInvokationInfo(taskInvokationInfo),
	_countdownToBeWokenUp(1),
	_parent(parent),
	_priority(0),
	_thread(nullptr),
#ifdef DISCRETE_DEPS
	_dataAccesses(seqs, addresses, numDeps),
#else
	_dataAccesses(),
#endif
	_flags(flags),
	_predecessorCount(0),
	_instrumentationTaskId(instrumentationTaskId),
	_schedulerInfo(nullptr),
	_computePlace(nullptr),
	_memoryPlace(nullptr),
	_countdownToRelease(1),
	_workflow(nullptr),
	_executionStep(nullptr),
	_taskStatistics(),
	_taskPredictions(),
	_taskCounters(),
	_taskCountersPredictions(),
	_clusterContext(nullptr),
	_parentSpawnCallback(nullptr)
{
	if (parent != nullptr) {
		parent->addChild(this);
	}
}

inline Task::~Task()
{
	if (_clusterContext != nullptr) {
		delete _clusterContext;
	}
}

inline void Task::reinitialize(
	void *argsBlock,
	size_t argsBlockSize,
	nanos6_task_info_t *taskInfo,
	nanos6_task_invocation_info_t *taskInvokationInfo,
	Task *parent,
	Instrument::task_id_t instrumentationTaskId,
	size_t flags
)
{
	_argsBlock = argsBlock;
	_argsBlockSize = argsBlockSize;
	_taskInfo = taskInfo;
	_taskInvokationInfo = taskInvokationInfo;
	_countdownToBeWokenUp = 1;
	_parent = parent;
	_priority = 0;
	_thread = nullptr;
	_flags = flags;
	_predecessorCount = 0;
	_instrumentationTaskId = instrumentationTaskId;
	_schedulerInfo = nullptr;
	_computePlace = nullptr;
	_memoryPlace = nullptr;
	_countdownToRelease = 1;
	_workflow = nullptr;
	_executionStep = nullptr;
	_clusterContext = nullptr;
	_parentSpawnCallback = nullptr;

	if (parent != nullptr) {
		parent->addChild(this);
	}
}

inline bool Task::markAsFinished(ComputePlace *computePlace)
{
	// Non-runnable taskfors should avoid these checks
	if (isRunnable()) {
		if (getDeviceType() == nanos6_device_t::nanos6_host_device) {
			// Not true anymore. A task might have been offloaded
			// to a remote device, in which case it wouldn't have
			// a thread assigned to it.
			//assert(_thread != nullptr);
			_thread = nullptr;
		} else {
			assert(_computePlace != nullptr);
			_computePlace = nullptr;
		}
	}

	// If the task has a wait clause, the release of dependencies must be
	// delayed (at least) until the task finishes its execution and all
	// its children complete and become disposable
	if (mustDelayRelease()) {
		CPUDependencyData hpDependencyData;

		//! We need to pass 'nullptr' here as a ComputePlace to notify
		//! the DataAccessRegistration system that it is creating
		//! taskwait fragments for a 'wait' task.
		DataAccessRegistration::handleEnterTaskwait(this, nullptr, hpDependencyData);

		if (!decreaseRemovalBlockingCount()) {
			return false;
		}

		// All its children are completed, so the delayed release of
		// dependencies has successfully completed
		completeDelayedRelease();
		DataAccessRegistration::handleExitTaskwait(this, computePlace, hpDependencyData);
		increaseRemovalBlockingCount();
	}

	// Return whether all external events have been also fulfilled, so
	// the dependencies can be released
	return decreaseReleaseCount();
}

// Return if the task can release its dependencies
inline bool Task::markAllChildrenAsFinished(ComputePlace *computePlace)
{
	assert(_thread == nullptr);

	CPUDependencyData hpDependencyData;

	// Complete the delayed release of dependencies
	completeDelayedRelease();
	DataAccessRegistration::handleExitTaskwait(this, computePlace, hpDependencyData);
	increaseRemovalBlockingCount();

	// Return whether all external events have been also fulfilled, so
	// the dependencies can be released
	return decreaseReleaseCount();
}

#endif // TASK_IMPLEMENTATION_HPP
