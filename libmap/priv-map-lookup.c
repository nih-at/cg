#include "map.h"

map_entry *
map__lookup(map *m, void *k)
{
	map_entry *me;
	int index;

	index = m->hash(k) % m->size;
	if (index < 0)
		index = (-index) % m->size;

	for (me = &m->entry[index];
	     me->next && m->equal(me->next->key, k) != 0;
	     me = me->next)
		;

	return me;
}
