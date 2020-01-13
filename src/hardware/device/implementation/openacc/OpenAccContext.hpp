/*
	This file is part of Nanos6 and is licensed under the terms contained in the COPYING file.

	Copyright (C) 2015-2019 Barcelona Supercomputing Center (BSC)
*/

#ifndef OPENACC_CONTEXT_HPP
#define OPENACC_CONTEXT_HPP

#include <cstddef>
#include <queue>

#include <openacc.h>

#include "tasks/Task.hpp"

#define OPENACC_STARTING_QUEUE_NUM 0

class OpenAccQueue {

private:
	size_t _index;

public:
	int _queueId; // async queue ID
	Task *_task;

	OpenAccQueue(size_t index): _index(index)
	{
		_queueId = (int)index;	/* FIXME get a valid ID, keep the IDs centrally */
	}

	//Disable copy constructor
	OpenAccQueue(OpenAccQueue const &) = delete;
	OpenAccQueue operator=(OpenAccQueue const &) = delete;

	~OpenAccQueue()
	{
		/* delete the ID */
	}

	//! \brief Get the assigned index of the stream
	size_t getIndex() const
	{
		return _index;
	}

	//! \brief Get the underlying int async ID
	int getQueue() const
	{
		return _queueId;
	}

	//! \brief
	bool finished()
	{
		if (acc_async_test(_queueId) != 0)
			return true;

		return false;
	}

};

class OpenAccQueuePool {
private:
	std::queue<OpenAccQueue *> _pool;
	unsigned int _size;
	int _next_async;	//normally should also be unsigned, but OpenACC spec expects an int

public:

	OpenAccQueuePool()
	{
		_next_async = 1;
		_size = OPENACC_STARTING_QUEUE_NUM;
		_pool.push(new OpenAccQueue(_next_async++));
	}

	~OpenAccQueuePool()
	{
		assert(_pool.size() == _size);

		while (!_pool.empty()) {
			delete _pool.front();
			_pool.pop();
		}
	}

	//!	\!brief Get an OpenACC async queue
	//!	Get a queue from the pool.
	//!	If no queues are available a new one is returned, which will be eventually returned to the pool instead of released.
	OpenAccQueue *getQueue()
	{
		if (_pool.empty()) {
			return new OpenAccQueue(_next_async++);
		} else {
			OpenAccQueue *queue = _pool.front();
			_pool.pop();
			return queue;
		}
	}
	//!	\!brief Return an OpenACC queue to the pool
	void returnQueue(OpenAccQueue *queue)
	{
		_pool.push(queue);
	}
};

struct OPENACC_DEVICE_DEP {
	OpenAccQueuePool _queuePool;
	std::vector<OpenAccQueue *> _activeQueues;
};

#endif //OPENACC_CONTEXT_HPP

