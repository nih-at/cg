/*
  new -- create new map
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



map *
map_new(int size)
{
    map *m;
    int i;

    if (size < 0)
	return NULL;

    if ((m=(map *)malloc(sizeof(map))) == NULL)
	return NULL;

    m->size = size ? size : map_def_size;
    m->equal = map_def_equal;
    m->hash = map_def_hash;
    m->cpykey = map_def_cpykey;
    m->delkey = map_def_delkey;
    m->delval = map_def_delval;

    if ((m->entry=(map_entry *)malloc(sizeof(map_entry)*m->size))
	== NULL) {
	free(m);
	return NULL;
    }

    for (i=0; i<m->size; i++) {
	m->entry[i].key = NULL;
	m->entry[i].value = NULL;
	m->entry[i].next = NULL;
    }
	
    return m;
}
