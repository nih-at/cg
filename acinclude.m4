dnl  $NiH: acinclude.m4,v 1.6 2001/12/13 21:14:49 dillo Exp $
dnl
dnl  acinclude.m4 -- test for autoconf
dnl  Copyright (C) 2000, 2001 Dieter Baron
dnl
dnl  This file is part of cg, a program to assemble and decode binary Usenet
dnl  postings.  The authors can be contacted at <nih@giga.or.at>
dnl
dnl  This program is free software; you can redistribute it and/or modify
dnl  it under the terms of the GNU General Public License as published by
dnl  the Free Software Foundation; either version 2 of the License, or
dnl  (at your option) any later version.
dnl
dnl  This program is distributed in the hope that it will be useful,
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl  GNU General Public License for more details.
dnl
dnl  You should have received a copy of the GNU General Public License
dnl  along with this program; if not, write to the Free Software
dnl  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

dnl Usage:
dnl NIH_CHECK_DECL(includes, variable, a-if-fnd, a-if-not-fnd)
AC_DEFUN(NIH_CHECK_DECL,
[AC_MSG_CHECKING(for declaration of $2)
AC_CACHE_VAL(nih_cv_check_decl_$2,
[AC_TRY_COMPILE([$1], [$2 = 0;],
 [nih_cv_check_decl_$2=yes],
 [nih_cv_check_decl_$2=no])])
if test "x$nih_cv_check_decl_$2" = xyes; then
AC_MSG_RESULT(yes)
ifelse([$4], ,
[AC_DEFINE([HAVE_DECL_]translit($2, [a-z], [A-Z]),
  1,[Define if you have member $3 in struct $2])],
[$4])
else
AC_MSG_RESULT(no)
ifelse([$5], , , [$5])
fi])
