/*
	This file is part of Nanos6 and is licensed under the terms contained in the COPYING file.

	Copyright (C) 2020 Barcelona Supercomputing Center (BSC)
*/

#ifndef INSTRUMENT_NULL_SCHEDULER_SUBSYTEM_ENTRY_POINTS_HPP
#define INSTRUMENT_NULL_SCHEDULER_SUBSYTEM_ENTRY_POINTS_HPP

#include "../api/InstrumentSchedulerSubsystemEntryPoints.hpp"

namespace Instrument {

	//! \brief Enters the scheduler addReadyTask method
	inline void enterAddReadyTask() {}

	//! \brief Exits the scheduler addReadyTask method
	inline void exitAddReadyTask() {}

	//! \brief Enters the scheduler addReadyTask method
	inline void enterGetReadyTask() {}

	//! \brief Exits the scheduler addReadyTask method
	inline void exitGetReadyTask(__attribute__((unused)) Task *task) {}

}

#endif // INSTRUMENT_NULL_SCHEDULER_SUBSYTEM_ENTRY_POINTS_HPP
