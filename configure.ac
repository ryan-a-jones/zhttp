AC_PREREQ([2.69])
AC_INIT([ZHTTP Library], [0.0.1], [ryan@rjones.io])
AC_CONFIG_SRCDIR([src/zhttp.c])
AC_CONFIG_HEADERS([zconfig.h])

# Initialize libtool and automake
LT_INIT
AM_INIT_AUTOMAKE

# Check for programs
AC_PROG_CC

# Check for external libraries
PKG_CHECK_MODULES(ZMQ, libczmq)

# Generate files
AC_CONFIG_FILES([
    Makefile
    src/Makefile])

AC_OUTPUT
