# -*- shell-script -*-
# process the file with autoconf to produce configure script

# initialize autoconf and check an id file to find src dir
#AC_REVISION
AC_INIT(./config/noclique-id)
AC_CONFIG_AUX_DIR(./config)
AC_CONFIG_MACRO_DIR([m4])

# initialize auxiliary shell scripts

# make automake clean emacs ~ files for "make clean"
CLEANFILES="*~"
AC_SUBST(CLEANFILES)

# for debugging configure
AC_ARG_ENABLE(echo, [  --enable-echo           echo configure script commands],
[set -x])

# checks for programs
CXXFLAGS=-g
AC_PROG_CC
AC_PROG_CXX
AC_ISC_POSIX
AC_PROG_LIBTOOL

AC_LANG_CPLUSPLUS

# checks for libraries

# checks for header files
AC_STDC_HEADERS
AC_HAVE_HEADERS([malloc.h unistd.h])

# checks for typedefs

# checks for structures

# checks for compiler characteristics
AC_C_CONST
AC_C_INLINE

# checks for library functions
AC_FUNC_ALLOCA

# config header that contains defines and stuff
AM_CONFIG_HEADER(config/config.h:config/config.h.in)

AC_CONFIG_FILES([])

# specify optional files to create
AC_OUTPUT()

# specify additional shell commands
AC_OUTPUT_COMMANDS([echo Configuration scripts created.])

# properly define output variable set_make for Makefile variable $MAKE
# @SET_MAKE@ must be issued in Makefile.in for this to function
AC_PROG_MAKE_SET
