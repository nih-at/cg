#include <stdlib.h>
#include "map.h"

map_iter *
map_start(map *m)
{
	map_iter *mi = (map_iter *)malloc(sizeof(map_iter));

	mi->m = m;
	mi->index = -1;
	mi->entry = NULL;
}

int
map_next(map_iter *mi, void **key, void **value)
{
	while (mi->entry == NULL) {
		if (mi->index < mi->m->size-1) {
			mi->index++;
			mi->entry = mi->m->entry[mi->index].next;
		}
		else
			return -1;
	}

	*key = mi->entry->key;
	*value = mi->entry->value;
	mi->entry = mi->entry->next;

	return 0;
}

void
map_stop(map_iter *mi)
{
	 free(mi);
}
