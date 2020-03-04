#	This file is part of Nanos6 and is licensed under the terms contained in the COPYING file.
#	
#	Copyright (C) 2015-2020 Barcelona Supercomputing Center (BSC)

AC_DEFUN([AC_CHECK_PGI],
	[
		AC_ARG_WITH(
			[pgi],
			[AS_HELP_STRING([--with-pgi=prefix], [specify the installation prefix of PGI])],
			[ ac_cv_use_pgi_prefix=$withval ],
			[ ac_cv_use_pgi_prefix="" ]
		)
		
		if test x"${ac_cv_use_pgi_prefix}" != x"" ; then
			AC_MSG_CHECKING([the PGI installation prefix])
			AC_MSG_RESULT([${ac_cv_use_pgi_prefix}])
			# hacky way to obtain the quoted string for rpath; $(ECHO) seems to not be provided
			pgi_lib_path_q=`echo \"${ac_cv_use_pgi_prefix}/lib\"`
			openacc_LIBS="-L${ac_cv_use_pgi_prefix}/lib -laccapi -lpgc -laccn -laccns"
			openacc_LIBS="${openacc_LIBS} -Wl,-rpath,${pgi_lib_path_q}"
			ac_use_openacc=yes
		else
			PKG_CHECK_MODULES(
				[pgi],
				[pgi],
				[
					AC_MSG_CHECKING([the PGI installation prefix])
					AC_MSG_RESULT([retrieved from pkg-config])
					pgi_lib_path_q=`echo \"${PGI_LIBS}/lib\"`
					openacc_LIBS="-L${PGI_LIBS} -laccapi -lpgc -laccn -laccns"
					openacc_LIBS="${openacc_LIBS} -Wl,-rpath,${pgi_lib_path_q}"
					ac_use_openacc=yes
				], [
					AC_MSG_CHECKING([the PGI installation prefix])
					AC_MSG_RESULT([not available])
				]
			)
		fi
		
		if test x"${ac_use_openacc}" = x"" ; then
			
			# check AC_PATH_PROG
			# manipulate to find the bin/
			# swap to lib
			AC_MSG_CHECKING([the PGI compiler installation])
			AC_PATH_PROG([PGICXX],[pgc++],[not available])
			if test "$PGICXX" != na ; then
				# hacky way to obtain the lib path from bin and produce quoted for rpath...
				PGI_LIB_PATH=`echo $PGICXX | sed s@bin/pgc++@lib@g`
				AC_MSG_RESULT([found PGI installed libraries ${PGI_LIB_PATH}])
				PGI_LIB_PATH_Q=`echo \"${PGI_LIB_PATH}\"`
				openacc_LIBS="-L${PGI_LIB_PATH} -laccapi -lpgc -laccn -laccns"
			    openacc_LIBS="${openacc_LIBS} -Wl,-rpath,${PGI_LIB_PATH_Q}"
				ac_use_openacc=yes
			else
				AC_MSG_WARN([PGI compiler not found. Check current PATH or provide installation path using --with-pgi.])
				ac_use_openacc=no
			fi
		fi
		
		AM_CONDITIONAL(HAVE_OPENACC, test x"${ac_use_openacc}" = x"yes")
		
		AC_SUBST([openacc_LIBS])
	]
)
