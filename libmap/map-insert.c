#include <stddef.h>
#include "map.h"

map_entry *map__lookup(map *m, void *k);
int map__hash(void *key);

void **
map_insert(map *m, void *k)
{
    map_entry *me;

    me = map__lookup(m, k);

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
