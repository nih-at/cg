#include <stdlib.h>
#include <string.h>
#include "map.h"

#define DEF_SIZE	1023
#define DEF_EQUAL	strcmp
#define DEF_HASH	map__hash
#define DEF_DELETE	free

map_entry *map__lookup(map *m, void *k);
int map__hash(void *key);

map *
new_map(int size, int (*equal)(), int (*hash)(), void *(*cpykey)(),
	void (*delkey)(), void (*delval)())
{
    map *m;
    int i;

    if ((m=(map *)malloc(sizeof(map))) == NULL)
	return NULL;

    m->size = size ? size : DEF_SIZE;
    m->equal = equal ? equal : DEF_EQUAL;
    m->hash = hash ? hash : DEF_HASH;
    m->cpykey = cpykey;
    m->delkey = delkey ? delkey : DEF_DELETE;
    m->delval = delval ? delval : DEF_DELETE;

    m->entry = (map_entry *)malloc(sizeof(map_entry)*m->size);
    for (i=0; i<m->size; i++) {
	m->entry[i].key = NULL;
	m->entry[i].value = NULL;
	m->entry[i].next = NULL;
    }
	
    return m;
}

int
map__hash(void *key)
{
    char *s = key;
    int i = 0;
    
    while (*s)
	i += *(s++);
    
    return i*i*i;
}
