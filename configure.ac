AC_INIT(checkgroup.c)
AM_INIT_AUTOMAKE(cg, 0.4a)
AM_CONFIG_HEADER(config.h)
AC_CONFIG_SUBDIRS(libmap)

# We want these before the checks, so the checks can modify their values.
#test -z "$CFLAGS" && CFLAGS='-g -Wall' auto_cflags=1

AC_CANONICAL_HOST

dnl programs

AC_PROG_CC
AM_PROG_LEX

# covered by LEX test above
# AC_DECL_YYTEXT

dnl libraries

AC_CHECK_LIB(socket, main)
AC_CHECK_LIB(nsl, main)
AC_CHECK_LIB(resolv, main)

dnl headers

AC_CHECK_HEADERS([stdarg.h])
NIH_CHECK_DECL([#include "netdb.h"], h_errno)

dnl functions

AC_CHECK_FUNCS(getopt_long, , [AC_LIBOBJ(getopt1) AC_LIBOBJ(getopt)])
AC_REPLACE_FUNCS(hstrerror getaddrinfo)
need_snprintf=no
AC_CHECK_FUNCS(snprintf vsnprintf vasprintf, , [need_snprintf=yes])
if test $need_snprintf = yes
then
	AC_LIBOBJ(snprintf)
fi

case $host_os
in
	*bsd*) MANFMT=mdoc;;
	*) MANFMT=man;;
esac

AC_LINK_FILES(cg.${MANFMT}, cg.1)

AC_OUTPUT(Makefile regress/Makefile)
