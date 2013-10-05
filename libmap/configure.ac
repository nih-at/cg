dnl Process this file with autoconf to produce a configure script.

dnl init

AC_INIT([libmap], [0.2])
AC_CONFIG_SRCDIR(strhash.c)
AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE

# We want these before the checks, so the checks can modify their values.
test -z "$CFLAGS" && CFLAGS=-g auto_cflags=1

dnl system type

dnl programs

AC_PROG_CC
AC_PROG_RANLIB

dnl libraries
dnl header files
dnl typedefs
dnl structures
dnl compiler characteristics
dnl library functions

AC_CHECK_FUNCS(strdup, , [AC_LIBOBJ(strdup)])

dnl system services

dnl output

AC_OUTPUT(Makefile)
