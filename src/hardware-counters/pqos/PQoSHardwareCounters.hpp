/*
	This file is part of Nanos6 and is licensed under the terms contained in the COPYING file.

	Copyright (C) 2019-2020 Barcelona Supercomputing Center (BSC)
*/

#ifndef PQOS_HARDWARE_COUNTERS_HPP
#define PQOS_HARDWARE_COUNTERS_HPP

#include "hardware-counters/CPUHardwareCountersInterface.hpp"
#include "hardware-counters/HardwareCountersInterface.hpp"
#include "hardware-counters/TaskHardwareCountersInterface.hpp"
#include "hardware-counters/ThreadHardwareCountersInterface.hpp"
#include "tasks/Task.hpp"


class PQoSHardwareCounters : public HardwareCountersInterface {

private:

	//! Whether PQoS HW Counter instrumentation is enabled
	bool _enabled;

	//! An enumeration containing the events that we monitor
	enum pqos_mon_event _monitoredEvents;

	//! The enabled events
	bool _enabledEvents[HWCounters::PQOS_NUM_EVENTS];

public:

	PQoSHardwareCounters(bool, const std::string &, const std::vector<HWCounters::counters_t> &enabledEvents);

	~PQoSHardwareCounters();

	void cpuBecomesIdle(
		CPUHardwareCountersInterface *cpuCounters,
		ThreadHardwareCountersInterface *threadCounters
	) override;

	void threadInitialized(ThreadHardwareCountersInterface *threadCounters) override;

	void threadShutdown(ThreadHardwareCountersInterface *threadCounters) override;

	void taskReinitialized(TaskHardwareCountersInterface *taskCounters) override;

	void taskStarted(
		CPUHardwareCountersInterface *cpuCounters,
		ThreadHardwareCountersInterface *threadCounters,
		TaskHardwareCountersInterface *taskCounters
	) override;

	void taskStopped(
		CPUHardwareCountersInterface *cpuCounters,
		ThreadHardwareCountersInterface *threadCounters,
		TaskHardwareCountersInterface *taskCounters
	) override;

};

#endif // PQOS_HARDWARE_COUNTERS_HPP