AC_PREREQ([2.69])
AC_INIT([ZHTTP Library], [0.0.1], [ryan@rjones.io])
AC_CONFIG_SRCDIR([src/zhttp.c])
AC_CONFIG_HEADERS([zconfig.h])

# Initialize libtool and automake
LT_INIT
AM_INIT_AUTOMAKE

# Check for programs
AC_PROG_CC
AC_CHECK_PROG(GPERF, gperf, yes)
if test x"${GPERF}" == x"yes" ; then
    AC_SUBST(GPERF, gperf)
else
    AC_MSG_ERROR([gperf must be installed in order to build ZHTTP])
fi

# Check for external libraries
PKG_CHECK_MODULES(ZMQ, libczmq)

# Generate files
AC_CONFIG_FILES([
    Makefile
    src/Makefile
    tests/Makefile])

AC_OUTPUT
