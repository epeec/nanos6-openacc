/*
	This file is part of Nanos6 and is licensed under the terms contained in the COPYING file.

	Copyright (C) 2015-2019 Barcelona Supercomputing Center (BSC)
*/

#include <cassert>

#include <nanos6.h>

#include "DataAccessRegistration.hpp"
#include "TaskBlocking.hpp"
#include "UserMutex.hpp"
#include "executors/threads/CPUManager.hpp"
#include "executors/threads/ThreadManager.hpp"
#include "executors/threads/ThreadManagerPolicy.hpp"
#include "executors/threads/WorkerThread.hpp"
#include "lowlevel/SpinLock.hpp"
#include "lowlevel/SpinWait.hpp"
#include "scheduling/Scheduler.hpp"
#include "tasks/Task.hpp"
#include "tasks/TaskImplementation.hpp"

#include <HardwareCounters.hpp>
#include <InstrumentUserMutex.hpp>
#include <Monitoring.hpp>


typedef std::atomic<UserMutex *> mutex_t;


void nanos6_user_lock(void **handlerPointer, __attribute__((unused)) char const *invocationSource)
{
	assert(handlerPointer != nullptr);
	mutex_t &userMutexReference = (mutex_t &) *handlerPointer;

	WorkerThread *currentThread = WorkerThread::getCurrentWorkerThread();
	assert(currentThread != nullptr);

	Task *currentTask = currentThread->getTask();
	assert(currentTask != nullptr);

	ComputePlace *computePlace = currentThread->getComputePlace();
	assert(computePlace != nullptr);

	// Allocation
	if (__builtin_expect(userMutexReference == nullptr, 0)) {
		UserMutex *newMutex = new UserMutex(true);

		UserMutex *expected = nullptr;
		if (userMutexReference.compare_exchange_strong(expected, newMutex)) {
			// Successfully assigned new mutex
			assert(userMutexReference == newMutex);

			Instrument::acquiredUserMutex(newMutex);

			// Since we allocate the mutex in the locked state, the thread already owns it and the work is done
			return;
		} else {
			// Another thread managed to initialize it before us
			assert(expected != nullptr);
			assert(userMutexReference == expected);

			delete newMutex;

			// Continue through the "normal" path
		}
	}

	// The mutex has already been allocated and cannot change, so skip the atomic part from now on
	UserMutex &userMutex = *(userMutexReference.load());

	if (currentTask->isTaskfor()) {
		// Fast path
		while (!userMutex.tryLock()) {
			spinWait();
		}
		Instrument::acquiredUserMutex(&userMutex);
		return;
	} else {
		// Fast path
		if (userMutex.tryLock()) {
			Instrument::acquiredUserMutex(&userMutex);
			return;
		}

		// Acquire the lock if possible. Otherwise queue the task.
		if (userMutex.lockOrQueue(currentTask)) {
			// Successful
			Instrument::acquiredUserMutex(&userMutex);
			return;
		}

		Monitoring::taskChangedStatus(currentTask, blocked_status);
		HardwareCounters::stopTaskMonitoring(currentTask);

		Instrument::taskIsBlocked(currentTask->getInstrumentationTaskId(), Instrument::in_mutex_blocking_reason);
		Instrument::blockedOnUserMutex(&userMutex);

		DataAccessRegistration::handleEnterBlocking(currentTask);
		TaskBlocking::taskBlocks(currentThread, currentTask, ThreadManagerPolicy::POLICY_NO_INLINE);

		// Update the CPU since the thread may have migrated
		computePlace = currentThread->getComputePlace();
		assert(computePlace != nullptr);
		Instrument::ThreadInstrumentationContext::updateComputePlace(computePlace->getInstrumentationId());

		DataAccessRegistration::handleExitBlocking(currentTask);

		// This in combination with a release from other threads makes their changes visible to this one
		std::atomic_thread_fence(std::memory_order_acquire);

		Instrument::acquiredUserMutex(&userMutex);
		Instrument::taskIsExecuting(currentTask->getInstrumentationTaskId());

		assert(currentTask->getThread() != nullptr);
		HardwareCounters::startTaskMonitoring(currentTask);
		Monitoring::taskChangedStatus(currentTask, executing_status);
	}
}


void nanos6_user_unlock(void **handlerPointer)
{
	assert(handlerPointer != nullptr);
	assert(*handlerPointer != nullptr);

	// This in combination with an acquire from another thread makes the changes visible to that one
	std::atomic_thread_fence(std::memory_order_release);

	WorkerThread *currentThread = WorkerThread::getCurrentWorkerThread();
	assert(currentThread != nullptr);

	CPU *cpu = currentThread->getComputePlace();
	assert(cpu != nullptr);

	mutex_t &userMutexReference = (mutex_t &) *handlerPointer;
	UserMutex &userMutex = *(userMutexReference.load());
	Instrument::releasedUserMutex(&userMutex);

	Task *releasedTask = userMutex.dequeueOrUnlock();
	if (releasedTask != nullptr) {
		Task *currentTask = currentThread->getTask();
		assert(currentTask != nullptr);

		if (!currentTask->isTaskfor() && ThreadManagerPolicy::checkIfUnblockedMustPreemtUnblocker(currentTask, releasedTask, cpu)) {
			WorkerThread *releasedThread = releasedTask->getThread();
			assert(releasedThread != nullptr);

			// Try to get an idle CPU and offload the released task's execution in it
			CPU *idleCPU = (CPU *) CPUManager::getIdleCPU();
			if (idleCPU != nullptr) {
				releasedThread->resume(idleCPU, false);
			} else {
				// No idle CPUs available, first re-add the current task to the scheduler
				Scheduler::addReadyTask(currentTask, cpu, UNBLOCKED_TASK_HINT);

				// After adding a task, the CPUManager may want to unidle CPUs
				CPUManager::executeCPUManagerPolicy((ComputePlace *) cpu, ADDED_TASKS, 1);

				// Now switch to the released thread
				currentThread->switchTo(releasedThread);

				// Update the CPU since the thread may have migrated
				cpu = currentThread->getComputePlace();
				assert(cpu != nullptr);
				Instrument::ThreadInstrumentationContext::updateComputePlace(cpu->getInstrumentationId());
			}
		} else {
			Scheduler::addReadyTask(releasedTask, cpu, UNBLOCKED_TASK_HINT);

			// After adding a task, the CPUManager may want to unidle CPUs
			CPUManager::executeCPUManagerPolicy((ComputePlace *) cpu, ADDED_TASKS, 1);
		}
	}
}

