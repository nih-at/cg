/*
  $NiH: decode.c,v 1.33 2002/04/10 16:23:27 wiz Exp $

  vasprintf.c -- replacement function
  Copyright (C) 2002 Dieter Baron

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#if  defined(__sun)
#include <sys/varargs.h>
#else
#include <stdarg.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BS 8192

int
vasprintf(char **s, const char *format, va_list ap)
{
    int ret;
    char b[BS], *p;
    va_list ap2;

    va_copy(ap2, ap);

    ret = vsnprintf(b, BS, format, ap);

    if (ret <= BS) {
	if ((p=strdup(b)) == NULL)
	    return -1;
	*s = p;
	return (ret < 0 ? strlen(p)+1 : ret);
    }
    
    if ((p=malloc(ret)) == NULL)
	return -1;
    *s = p;
    return vsnprintf(p, ret, format, ap2);
}
