#ifndef HAD_MAP_H
#define HAD_MAP_H

typedef struct map_entry {
    void *key, *value;
    struct map_entry *next;
} map_entry;

typedef struct map {
    int size;
    int (*equal)(), (*hash)();
    void *(*cpykey)();
    void (*delkey)(), (*delval)();
    map_entry *entry;
} map;

typedef struct map_iter {
    map *m;
    int index;
    map_entry *entry;
} map_iter;

map *new_map(int size, int (*equal)(), int (*hash)(), void *(*cpykey)(),
	     void (*delkey)(), void (*delval)());
void free_map(map *m);
void *map_lookup(map *m, void *k);
void **map_insert(map *m, void *k);
void map_delete(map *m, void *k);
map_iter *map_start(map *m);
int map_next(map_iter *mi, void **key, void **value);
void map_stop(map_iter *mi);

#endif /* map.h */
