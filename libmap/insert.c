/*
  insert -- insert entry in map
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

map_entry *_map_lookup(map *m, void *k);



void **
map_insert(map *m, void *k)
{
    map_entry *me;

    me = _map_lookup(m, k);

    if (me->next == NULL) {
	if ((me->next=(map_entry *)malloc(sizeof(map_entry))) == NULL)
	    return NULL;
	if (m->cpykey)
	    me->next->key = m->cpykey(k);
	else
	    me->next->key = k;
	me->next->value = NULL;
	me->next->next = NULL;
    }

    return &(me->next->value);
}
