#ifndef HAD_MAP_H
#define HAD_MAP_H

/*
  libmap -- associative arrays
  Copyright (C) 1996 Dieter Baron

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



#define MAP_DFL -1

typedef struct map_entry {
    void *key, *value;
    struct map_entry *next;
} map_entry;

typedef struct map {
    int size;
    int (*equal)(void *key1, void *key2);
    unsigned int (*hash)(void *key, int size);
    void *(*cpykey)(void *key);
    void (*delkey)(void *key),
	(*delval)(void *value);
    map_entry *entry;
} map;

typedef struct map_iter {
    map *m;
    int index;
    map_entry *entry;
} map_iter;



map *new_map(int size);
map *new_map2(int size,
	      int (*equal)(void *key1, void *key2),
	      unsigned int (*hash)(void *key, int size),
	      void *(*cpykey)(void *key),
	      void (*delkey)(void *key),
	      void (*delval)(void *value));
void free_map(map *m, int delvaluep);
void *map_lookup(map *m, void *key);
void **map_insert(map *m, void *key);
void map_delete(map *m, void *key, int delvaluep);
map_iter *map_start(map *m);
int map_next(map_iter *mi, void **key, void **value);
void map_stop(map_iter *mi);

unsigned int map_strhash(void *key, int size);



extern int map_def_size;
extern int (*map_def_equal)(void *key1, void *key2);
extern unsigned int (*map_def_hash)(void *key, int size);
extern void *(*map_def_cpykey)(void *key);
extern void (*map_def_delkey)(void *key);
extern void (*map_def_delval)(void *value);

#endif /* map.h */
