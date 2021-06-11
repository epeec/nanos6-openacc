/*
	This file is part of Nanos6 and is licensed under the terms contained in the COPYING file.

	Copyright (C) 2021 Barcelona Supercomputing Center (BSC)
*/

#include "resolve.h"

#pragma GCC visibility push(default)

int nanos6_get_current_acc_queue(void)
{
	typedef int nanos6_get_current_acc_queue(void);

	static nanos6_get_current_acc_queue_t *symbol = NULL;
	if (__builtin_expect(symbol == NULL, 0)) {
		symbol = (nanos6_get_current_acc_queue *)
			_nanos6_resolve_symbol("nanos6_get_current_acc_queue", "openacc", NULL);
	}

	return (*symbol)();
}

#pragma GCC visibility pop

