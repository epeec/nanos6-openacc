/*
	This file is part of Nanos6 and is licensed under the terms contained in the COPYING file.

	Copyright (C) 2015-2019 Barcelona Supercomputing Center (BSC)
*/

#ifndef OPENACC_SEL_HPP
#define OPENACC_SEL_HPP

#include <config.h>

#ifdef USE_OPENACC
#include "openacc/OpenAccFunctions.hpp"
#else
#include "NULLDevice.hpp"
typedef NULLDevice OpenAccFunctions;
#endif

#endif

