#include <stddef.h>
#include "map.h"

map_entry *map__lookup(map *m, void *k);
int map__hash(void *key);

void *
map_lookup(map *m, void *k)
{
	map_entry *me;
	
	me = map__lookup(m, k)->next;
	return me ? me->value : NULL;
}
