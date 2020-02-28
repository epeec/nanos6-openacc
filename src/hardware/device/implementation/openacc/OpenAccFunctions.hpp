/*
	This file is part of Nanos6 and is licensed under the terms contained in the COPYING file.

	Copyright (C) 2015-2019 Barcelona Supercomputing Center (BSC)
*/

#ifndef OPENACC_FUNCTIONS_HPP
#define OPENACC_FUNCTIONS_HPP

#include <cassert>
#include <cstdlib>
#include <queue>

#include <openacc.h>

#include <nanos6/openacc_device.h>

#include "OpenAccContext.hpp"
#include "hardware/places/DeviceComputePlace.hpp"
#include "hardware/places/DeviceMemoryPlace.hpp"
#include "hardware/device/DeviceFunctionsInterface.hpp"
#include "lowlevel/SpinLock.hpp"
#include "memory/vmm/VirtualMemoryArea.hpp"
#include "tasks/Task.hpp"

class OpenAccFunctions: public DeviceFunctionsInterface {

private:
	SpinLock _depsLock;
	const nanos6_device_t device_type = nanos6_openacc_device;
	bool _correctlyInitialized;

	acc_device_t _dev_type = acc_device_default; // as long as this works let it like that

public:
	std::vector<std::pair<void *, OPENACC_DEVICE_DEP *>> _openaccDeps;

	OpenAccFunctions()
	{
		_devices.push_back(new Device(nanos6_openacc_device, 0));

		_correctlyInitialized = false;
		int deviceCount = acc_get_num_devices(_dev_type);

		if (deviceCount == 0) {
			// warn of no device present
			return;
		}

		assert(deviceCount > 0);
		_openaccDeps.resize(deviceCount);

		/* Instead of 'setDevice', openACC inits the runtime for all devices of given type.
		 * An issue with these calls is that, being void, they make error handling impossible.
		 */
		//acc_init(_dev_type);	/* For the time being this appears to cause broken CUDA contexts, so leave it out */

		for (int i = 0; i < deviceCount; ++i) {
			DeviceComputePlace *cp = new DeviceComputePlace(new DeviceMemoryPlace(i, nanos6_openacc_device),
					nanos6_device_t::nanos6_openacc_device, 0, i, this, nullptr);
			_devices[0]->addComputePlace(cp);

			_openaccDeps[i].first = (void *) cp;
			_openaccDeps[i].second = new OPENACC_DEVICE_DEP();
		}

		_correctlyInitialized = true;
	}

	~OpenAccFunctions()
	{
	}

	void shutdown()
	{
		nanos6_unregister_polling_service("taskFinisher",
				(nanos6_polling_service_t) DeviceComputePlace::pollingFinishTasks, this);
	}

	OPENACC_DEVICE_DEP *getDeps(void *ptr)
	{
		for (auto p : _openaccDeps) {
			if (p.first == ptr)
				return p.second;
		}
		return nullptr;
	}

	//memory functions: malloc, free, memcpy
	int malloc(void **ptr, size_t size)
	{
		// TODO: probably something with acc_data_create
	}

	void free(void *ptr)
	{
		// TODO
	}

	int memcpy(void *dst, void *src, size_t size, deviceMemcpy type)
	{
		// TODO
	}

	int setDevice(int device)
	{
		acc_set_device_num(device, _dev_type);
		// Unfortunately there is no error return from OpenACC

		return 0;
	}

	const char *getName()
	{
		static std::string *str = new std::string("OpenACC");
		return str->c_str();
	}

	void runTask(Task *task, ComputePlace *cp)
	{
		assert(task != nullptr);
		assert(cp != nullptr);

		((DeviceComputePlace *) cp)->runTask(task);
	}

	nanos6_device_t getType()
	{
		return device_type;
	}

	void *generateDeviceExtra(Task *task, void *)
	{
		nanos6_openacc_device_environment_t *env =
				(nanos6_openacc_device_environment_t *) ::malloc(
						sizeof(nanos6_openacc_device_environment_t));

		OpenAccQueue *deviceDataQueue = getDeps(task->getComputePlace())->_queuePool.getQueue();
		task->setDeviceData((void *) deviceDataQueue);
		env->asyncId = deviceDataQueue->_queueId;
		deviceDataQueue->_launched = false; // set this as a guard to running getFinishedTasks before actually launching
		deviceDataQueue->_task = task;
		getDeps(task->getComputePlace())->_activeQueues.push_back(deviceDataQueue);

		return (void *) env;
	}

	void postBodyDevice(Task *task, void *)
	{
		// Find the corresponding asynq queue and set the launched flag;
		// This was needed as we noticed sometimes getFinishedTasks being called before
		// the actual kernel launch, which resulted in acc_async_test returning true.
		//
		// Since a 'better' way will require lots of changes, we'll just iterate to find the appropriate task...
		auto taskcp = task->getComputePlace();
		auto deps = getDeps(taskcp);
		std::vector<OpenAccQueue *> *_activeQueues = &deps->_activeQueues;
		if (_activeQueues != nullptr) {
			std::lock_guard<SpinLock> guard(_depsLock);
			auto it = _activeQueues->begin();
			while (it != _activeQueues->end()) {
				if ((*it)->_task == task) {
					OpenAccQueue* queue = *it;
					queue->_launched = true;
					return;
				}
				else
					++it;
			}
		}
	}

	void bodyDevice(Task *, void *)
	{

	}

	void getFinishedTasks(std::vector<Task *>& finishedTasks)
	{
		for (unsigned int i = 0; i < _openaccDeps.size(); ++i) {
			std::vector<OpenAccQueue *> *_activeQueues = &_openaccDeps[i].second->_activeQueues;
			if (_activeQueues != nullptr) {

				std::lock_guard<SpinLock> guard(_depsLock);
				auto it = _activeQueues->begin();
				while (it != _activeQueues->end()) {
					if ((*it)->finished()) {
						OpenAccQueue* queue = *it;
						Task *task = queue->_task;
						finishedTasks.push_back(task);
						_openaccDeps[i].second->_queuePool.returnQueue(queue);
						if (task != nullptr) {
							DeviceComputePlace *devComputePlace =
								(DeviceComputePlace *) (task->getComputePlace());

							devComputePlace->disposeTask();
							// why so complex on CUDA?
						}

						it = _activeQueues->erase(it);
					}
					else {
						++it;
					}
				}
			}
		}
	}

	void unifiedAsyncPrefetch(void *pHost, size_t size, int dstDevice)
	{
		//setDevice(dstDevice);
		//
		//void *dev;
		//cudaHostGetDevicePointer(&dev, pHost, 0);
		//memcpy(pHost, dev, size, HOST_TO_DEVICE);

	}

	void *unifiedGetDevicePointer(void *pHost)
	{
		//void *dev;
		//cudaHostGetDevicePointer(&dev, pHost, 0);
		return nullptr;
	}

	void unifiedMemRegister(void *pHost, size_t size)
	{

	}

	void unifiedMemUnregister(void *pHost)
	{

	}

	bool initialize()
	{
		if (_correctlyInitialized) {
			nanos6_register_polling_service("taskFinisher",
					(nanos6_polling_service_t) DeviceComputePlace::pollingFinishTasks, this);

			for (Device *device : _devices) {
				for (int i = 0; i< device->getNumDevices(); ++i) {
					device->getComputePlace(i)->activatePollingService();
				}
			}
		}

		return _correctlyInitialized;
	}

	bool getInitStatus()
	{
		return _correctlyInitialized;
	}

	void getDevices(std::vector<Device *> &dev)
	{
		dev = _devices;
	}

};

#endif // OPENACC_FUNCTIONS_HPP

