/*
  $NiH$
  
  symbol.c -- simple symbol table
  Copyright (C) 2002 Dieter Baron and Thomas Klaunser

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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "symbol.h"

#define TBLSIZE 2047

struct entry {
    struct entry *next;
    symbol s;
};

struct entry table[TBLSIZE];

int yourhash(char *s);



symbol
intern(char *s)
{
    int i;
    struct entry *e;
    
    i = yourhash(s);

    if (table[i].s) {
	for (e=&table[i]; e->next; e = e->next) {
	    if (strcmp(e->s, s) == 0)
		return e->s;
	}
	if (strcmp(e->s, s) == 0)
	    return e->s;
	
	if ((e->next=(struct entry *)malloc(sizeof(struct entry))) == NULL)
	    return NULL;

	if ((e->next->s=strdup(s)) == NULL) {
	    free(e->next);
	    e->next = NULL;
	    return NULL;
	}
	e=e->next;
	e->next = NULL;
	return e->s;
    }

    return table[i].s = strdup(s);
}



symbol
intern_lower(char *s)
{
    char b[8192], *t;

    for (t=b; (*t=tolower(*s)); t++,s++)
	;

    return intern(b);
}



symbol
intern_caps(char *s)
{
    char b[8192], *t;
    int c, id;

    for (c=1,id=0,t=b; (*t=(c?toupper:tolower)(*s)); t++,s++) {
	switch (*t) {
	case '-':
	    c = 1;
	    break;
	case 'I':
	    if (c)
		id = 1;
	    c = 0;
	    break;
	case 'd':
	    if (id)
		*t = 'D';
	    id = c = 0;
	    break;
	default:
	    c = 0;
	}
    }

    return intern(b);
}



int
yourhash(char *s)
{
    int i, j, k;

    i = j = 0;
    
    while (*(s+i) != '\0') {
	j += (i+1)*(int)*(i+s);
	j %= TBLSIZE;
	i++;
    }

    k = j;

    j *= j;
    j %= TBLSIZE;
    j *= k;
    j %= TBLSIZE;

    return j;
    
}
