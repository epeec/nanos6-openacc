/*
	This file is part of Nanos6 and is licensed under the terms contained in the COPYING file.

	Copyright (C) 2020 Barcelona Supercomputing Center (BSC)
*/

#include "resolve.h"

#pragma GCC visibility push(default)

void *nanos6_device_alloc(size_t size, int device_num)
{
	typedef void *nanos6_device_alloc_t(size_t size, int device_num);

	static nanos6_device_alloc_t *symbol = NULL;
	if (__builtin_expect(symbol == NULL, 0)) {
		symbol = (nanos6_device_alloc_t *)
			_nanos6_resolve_symbol("nanos6_device_alloc", "device-mem", NULL /* fallback */);
	}

	return (*symbol)(size, device_num);
}

void nanos6_device_free(void *ptr)
{
	typedef void nanos6_device_free_t(void *ptr);

	static nanos6_device_free_t *symbol = NULL;
	if (__builtin_expect(symbol == NULL, 0)) {
		symbol = (nanos6_device_free_t *)
			_nanos6_resolve_symbol("nanos6_device_free", "device-mem", NULL /* fallback */);
	}

	return (*symbol)(ptr)l
}

#pragma GCC visibility pop

