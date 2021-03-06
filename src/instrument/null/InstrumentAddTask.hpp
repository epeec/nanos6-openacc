/*
	This file is part of Nanos6 and is licensed under the terms contained in the COPYING file.
	
	Copyright (C) 2015-2017 Barcelona Supercomputing Center (BSC)
*/

#ifndef INSTRUMENT_NULL_ADD_TASK_HPP
#define INSTRUMENT_NULL_ADD_TASK_HPP


#include "../api/InstrumentAddTask.hpp"


namespace Instrument {
	inline task_id_t enterAddTask(
		__attribute__((unused)) nanos6_task_info_t *taskInfo,
		__attribute__((unused)) nanos6_task_invocation_info_t *taskInvokationInfo,
		__attribute__((unused)) size_t flags,
		__attribute__((unused)) InstrumentationContext const &context
	) {
		return task_id_t();
	}
	
	inline void createdArgsBlock(
		__attribute__((unused)) task_id_t taskId,
		__attribute__((unused)) void *argsBlockPointer,
		__attribute__((unused)) size_t originalArgsBlockSize,
		__attribute__((unused)) size_t argsBlockSize,
		__attribute__((unused)) InstrumentationContext const &context)
	{
	}
	
	inline void createdTask(
		__attribute__((unused)) void *task,
		__attribute__((unused)) task_id_t taskId,
		__attribute__((unused)) InstrumentationContext const &context
	) {
	}
	
	inline void exitAddTask(
		__attribute__((unused)) task_id_t taskId,
		__attribute__((unused)) InstrumentationContext const &context
	) {
	}
	
	inline task_id_t enterAddTaskforCollaborator(
		__attribute__((unused)) task_id_t taskforId,
		__attribute__((unused)) nanos6_task_info_t *taskInfo,
		__attribute__((unused)) nanos6_task_invocation_info_t *taskInvokationInfo,
		__attribute__((unused)) size_t flags,
		__attribute__((unused)) InstrumentationContext const &context
	) {
		return task_id_t();
	}
	
	inline void exitAddTaskforCollaborator(
		__attribute__((unused)) task_id_t taskforId,
		__attribute__((unused)) task_id_t collaboratorId,
		__attribute__((unused)) InstrumentationContext const &context
	) {
	}
}


#endif // INSTRUMENT_NULL_ADD_TASK_HPP
