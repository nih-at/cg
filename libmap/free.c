/*
  free -- delete map
  Copyright (C) 1996 Dieter Baron

  This file is part of libmap, a library for associative arrays.
  The author can be contacted at <dillo@giga.or.at>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/



#include <stdlib.h>
#include "map.h"



void
map_free(map *m, int dv)
{
    int i;
    map_entry *e, *p;

    for (i=0; i<m->size; i++)
	for (e=m->entry[i].next; e;) {
	    if (m->delkey)
		m->delkey(e->key);
	    if (dv && m->delval)
		m->delval(e->value);
	    p = e;
	    e = e->next;
	    free(p);
	}

    free(m->entry);
    free(m);
}
