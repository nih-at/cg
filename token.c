/*
  $NiH: token.c,v 1.4 2002/04/16 22:46:17 wiz Exp $

  token.c -- token handling
  Copyright (C) 2002 Dieter Baron and Thomas Klausner

  This file is part of cg, a program to assemble and decode binary Usenet
  postings.  The authors can be contacted at <nih@giga.or.at>

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

#include <stdarg.h>
#include <stdlib.h>

#include "stream.h"
#include "util.h"

token *
token_new(enum token_type type, char *line)
{
    return token_set(xmalloc(sizeof(token)), type, line);
}



token *
token_printf3(token *t, enum token_type type, int n, char *fmt, ...)
{
    char *s;
    va_list argp;

    va_start(argp, fmt);
    vasprintf(&s, fmt, argp);
    va_end(argp);

    token_set3(t, type, n, s);
    t->alloced = 1;

    return t;
}



token *
token_set(token *t, enum token_type type, char *line)
{
    return token_set3(t, type, 0, line);
}



token *
token_set3(token *t, enum token_type type, int n, char *line)
{
    t->alloced = 0;
    t->type = type;
    t->n = n;
    t->line = line;

    return t;
}



void
token_free(token *t)
{
    if (t == NULL)
	return;

    if (t->alloced)
	free(t->line);
    free(t);
}



token *
token_copy(token *d, token *s)
{
    return token_set3(d, s->type, s->n, s->line);
}
