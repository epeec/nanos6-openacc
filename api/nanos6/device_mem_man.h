/*
	This file is part of Nanos6 and is licensed under the terms contained in the COPYING file.

	Copyright (C) 2020 Barcelona Supercomputing Center (BSC)
*/

#ifndef NANOS6_DEVICE_MEM_MAN_H
#define NANOS6_DEVICE_MEM_MAN_H

#pragma GCC visibility push(default)

#ifdef __cplusplus
extern "C" {
#endif

void *nanos6_device_alloc(size_t size, int device_num);

void nanos6_device_free(void *ptr);

#ifdef __cplusplus
}
#endif

#pragma GCC visibility pop

#endif /* NANOS6_DEVICE_MEM_MAN_H */
