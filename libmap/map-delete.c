#include <stdlib.h>
#include "map.h"

map_entry *map__lookup(map *m, void *k);
int map__hash(void *key);

void
map_delete(map *m, void *k)
{
	map_entry *me, *p;

	p = map__lookup(m, k);
	if ((me = p->next) != NULL) {
		m->delkey(me->key);
		m->delval(me->value);
		p->next = me->next;
		free(me);
	}

	return;
}
