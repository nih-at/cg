#include <stdlib.h>
#include "map.h"

void
free_map(map *m)
{
	int i;
	map_entry *e, *p;

	for (i=0; i<m->size; i++)
		for (e=m->entry[i].next; e;) {
			m->delkey(e->key);
			m->delval(e->value);
			p = e;
			e = e->next;
			free(p);
		}

	free(m->entry);
	free(m);
}
