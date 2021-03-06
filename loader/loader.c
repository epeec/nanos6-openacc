/*
	This file is part of Nanos6 and is licensed under the terms contained in the COPYING file.

	Copyright (C) 2015-2017 Barcelona Supercomputing Center (BSC)
*/

#if HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if HAVE_DLINFO
#include <link.h>
#endif

#include "api/nanos6/debug.h"

#include "main-wrapper.h"
#include "loader.h"


#define MAX_LIB_PATH 8192


__attribute__ ((visibility ("hidden"))) void *_nanos6_lib_handle = NULL;

__attribute__ ((visibility ("hidden"))) int _nanos6_has_started = 0;
int _nanos6_exit_with_error = 0;
char _nanos6_error_text[ERROR_TEXT_SIZE];



static char lib_name[MAX_LIB_PATH+1];

static void _nanos6_loader_set_up_lib_name(char const *variant, char const *dependencies, char const *path, char const *suffix)
{
	if (path != NULL) {
		if (suffix != NULL)
			snprintf(lib_name, MAX_LIB_PATH, "%s/libnanos6-%s-%s.so", path, variant, dependencies);
		else
			snprintf(lib_name, MAX_LIB_PATH, "%s/libnanos6-%s-%s.so.%s", path, variant, dependencies, suffix);
	} else {
		if (suffix != NULL)
			snprintf(lib_name, MAX_LIB_PATH, "libnanos6-%s-%s.so", variant, dependencies);
		else
			snprintf(lib_name, MAX_LIB_PATH, "libnanos6-%s-%s.so.%s", variant, dependencies, suffix);
	}

}

static void _nanos6_loader_try_load(_Bool verbose, char const *variant, char const *dependencies, char const *path)
{
	_nanos6_loader_set_up_lib_name(variant, dependencies, path, SONAME_SUFFIX);

	if (verbose) {
		fprintf(stderr, "Nanos6 loader trying to load: %s\n", lib_name);
	}

	_nanos6_lib_handle = dlopen(lib_name, RTLD_LAZY | RTLD_GLOBAL);
	if (_nanos6_lib_handle != NULL) {
		if (verbose) {
			fprintf(stderr, "Successfully loaded: %s\n", nanos6_get_runtime_path());
		}
		return;
	}

	if (verbose) {
		fprintf(stderr, "Failed: %s\n", dlerror());
	}

	_nanos6_loader_set_up_lib_name(variant, dependencies, path, SONAME_MAJOR);
	if (verbose) {
		fprintf(stderr, "Nanos6 loader trying to load: %s\n", lib_name);
	}

	_nanos6_lib_handle = dlopen(lib_name, RTLD_LAZY | RTLD_GLOBAL);
	if (_nanos6_lib_handle != NULL) {
		if (verbose) {
			fprintf(stderr, "Successfully loaded: %s\n", nanos6_get_runtime_path());
		}
		return;
	}

	if (verbose) {
		fprintf(stderr, "Failed: %s\n", dlerror());
	}
}


static void _nanos6_loader_try_load_without_major(_Bool verbose, char const *variant, char const *dependencies, char const *path)
{
	_nanos6_loader_set_up_lib_name(variant, dependencies, path, NULL);
	if (verbose) {
		fprintf(stderr, "Nanos6 loader trying to load: %s\n", lib_name);
	}

	_nanos6_lib_handle = dlopen(lib_name, RTLD_LAZY | RTLD_GLOBAL);
	if (_nanos6_lib_handle != NULL) {
		if (verbose) {
			fprintf(stderr, "Successfully loaded: %s\n", nanos6_get_runtime_path());
		}
		return;
	}
}


static const char *_nanos6_get_requested_variant()
{
	char const *variant = getenv("NANOS6");
	if (variant != NULL) {
		if (strcmp(variant, "") != 0) {
			return variant;
		}
	}
	return NULL;
}

static void _nanos6_check_disabled_variant(char const *variant, char const *dependencies)
{
	assert(_nanos6_lib_handle != NULL);
	assert(variant != NULL);
	assert(dependencies != NULL);

	void *disabled_symbol = dlsym(_nanos6_lib_handle, "nanos6_disabled_variant");
	if (disabled_symbol != NULL) {
		snprintf(_nanos6_error_text, ERROR_TEXT_SIZE,
			"This installation of Nanos6 does not include the %s variant with %s dependencies.",
			variant, dependencies);
		_nanos6_exit_with_error = 1;
	}
}


__attribute__ ((visibility ("hidden"), constructor)) void _nanos6_loader(void)
{
	if (_nanos6_lib_handle != NULL) {
		return;
	}

	_Bool verbose = (getenv("NANOS6_LOADER_VERBOSE") != NULL);

	// Check the name of the replacement library
	char const *variant = _nanos6_get_requested_variant();
	if (variant == NULL) {
		variant = "optimized";
	}

	if (verbose) {
		fprintf(stderr, "Nanos6 loader using variant: %s\n", variant);
	}

	char const *dependencies = getenv("NANOS6_DEPENDENCIES");
	if (dependencies == NULL) {
		dependencies = "linear-regions-fragmented";
	}

	if (verbose) {
		fprintf(stderr, "Nanos6 loader using dependency implementation: %s\n", dependencies);
	}

	char *lib_path = getenv("NANOS6_LIBRARY_PATH");
	if (lib_path != NULL) {
		if (verbose) {
			fprintf(stderr, "Nanos6 loader using path from NANOS6_LIBRARY_PATH: %s\n", lib_path);
		}
	}

	// Try the global or the NANOS6_LIBRARY_PATH scope
	_nanos6_loader_try_load(verbose, variant, dependencies, lib_path);
	if (_nanos6_lib_handle != NULL) {
		// Check if this is a disabled variant
		_nanos6_check_disabled_variant(variant, dependencies);
		return;
	}

	// Attempt to load it from the same path as this library
	Dl_info di;
	int rc = dladdr((void *)_nanos6_loader, &di);
	assert(rc != 0);

	lib_path = strdup(di.dli_fname);
	for (int i = strlen(lib_path); i > 0; i--) {
		if (lib_path[i] == '/') {
			lib_path[i] = 0;
			break;
		}
	}
	_nanos6_loader_try_load(verbose, variant, dependencies, lib_path);
	if (_nanos6_lib_handle != NULL) {
		// Check if this is a disabled variant
		_nanos6_check_disabled_variant(variant, dependencies);
		return;
	}

	snprintf(_nanos6_error_text, ERROR_TEXT_SIZE, "Nanos6 loader failed to load the runtime library.");
	_nanos6_exit_with_error = 1;

	//
	// Diagnose the problem
	//

	// Check the variant
	if (verbose) {
		fprintf(stderr, "Checking if the variant was not correct\n");
	}

	_nanos6_loader_try_load(verbose, "optimized", "linear-regions-fragmented", getenv("NANOS6_LIBRARY_PATH"));
	if (_nanos6_lib_handle == NULL) {
		_nanos6_loader_try_load(verbose, "optimized", "linear-regions-fragmented", lib_path);
	}
	if (_nanos6_lib_handle != NULL) {
		fprintf(stderr, "Error: the %s variant of the runtime with the dependencies implementation %s is not available in this installation.\n", variant, dependencies);
		fprintf(stderr, "\tPlease check that the NANOS6 environment variable is valid.\n");

		dlclose(_nanos6_lib_handle);
		_nanos6_lib_handle = NULL;

		return;
	}

	// Check for version mismatch
	if (verbose) {
		fprintf(stderr, "Checking for a mismatch between the linked version and the installed version\n");
	}

	_nanos6_loader_try_load_without_major(verbose, variant, dependencies, getenv("NANOS6_LIBRARY_PATH"));
	if (_nanos6_lib_handle == NULL) {
		_nanos6_loader_try_load_without_major(verbose, variant, dependencies, lib_path);
	}
	if (_nanos6_lib_handle != NULL) {
		fprintf(stderr, "Error: there is a mismatch between the installed runtime so version and the linked so version\n");
		fprintf(stderr, "\tExpected so version: %s or at least %s\n", SONAME_SUFFIX, SONAME_MAJOR);
		fprintf(stderr, "\tFound instead this so: %s\n", nanos6_get_runtime_path());
		fprintf(stderr, "\tPlease recompile your application.\n");

		dlclose(_nanos6_lib_handle);
		_nanos6_lib_handle = NULL;

		return;
	}

	if (_nanos6_get_requested_variant() != NULL) {
		fprintf(stderr, "Please check that the value of the NANOS6 environment variable is correct and set the NANOS6_LIBRARY_PATH environment variable if the runtime is installed in a different location than the loader.\n");
	} else if (getenv("NANOS6_DEPENDENCIES") != NULL) {
		fprintf(stderr, "Please check that the value of the NANOS6_DEPENDENCIES environment variable is correct and set the NANOS6_LIBRARY_PATH environment variable if the runtime is installed in a different location than the loader.\n");
	} else {
		fprintf(stderr, "Please set or check the NANOS6_LIBRARY_PATH environment variable if the runtime is installed in a different location than the loader.\n");
	}

}


#pragma GCC visibility push(default)

char const *nanos6_get_runtime_path(void)
{
#if HAVE_DLINFO
	if (_nanos6_lib_handle == NULL) {
		_nanos6_loader();
	}

	static char const *lib_path = NULL;
	static int initialized = 0;

	if (initialized == 0) {
		void *symbol = dlsym(_nanos6_lib_handle, "nanos6_preinit");
		if (symbol == NULL) {
			lib_path = strdup(dlerror());
		} else {
			Dl_info di;
			int rc = dladdr(symbol, &di);

			if (rc != 0) {
				lib_path = strdup(di.dli_fname);
			} else {
				lib_path = strdup(dlerror());
			}
		}

		initialized = 1;
	}

	return lib_path;
#else
	return "not available";
#endif
}

#pragma GCC visibility pop
