/*
  testmap -- test program for libmap
  Copyright (C) 1996 Dieter Baron

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



#include <stdio.h>
#include <stdlib.h>
#include "map.h"

struct st_entries {
    char *key, *value;
    int had;
};

struct st_entries entries[] = {
    { "a", "va" },
    { "long-key", "long-value-long" },
    { "3", "three" },
    { NULL }
};
	


int
main(int argc, char **argv)
{
    map *m;
    map_iter *mi;
    int i;
    char **p, *k, *v;

    printf("creating map");
    fflush(stdout);
    
    if ((m=map_new(0)) == NULL) {
	printf(" -- FAILED!\n");
	exit(1);
    }
    printf("\n");
	
    printf("inserting");
    fflush(stdout);

    for (i=0; entries[i].key; i++) {
	p = (char **)map_insert(m, entries[i].key);
	if (p == NULL || *p != NULL) {
	    printf(" -- FAILED (%s)!\n", entries[i].key);
	    exit(1);
	}
	else
	    *p = entries[i].value;
    }
    printf("\n");

    printf("looking up");
    fflush(stdout);

    for (i=0; entries[i].key; i++) {
	if (map_lookup(m, entries[i].key) != entries[i].value) {
	    printf(" -- FAILED (%s)!\n", entries[i].key);
	    exit(1);
	}
    }
    printf("\n");

    printf("iterating");
    fflush(stdout);

    for (mi=map_start(m); map_next(mi, (void **)&k, (void **)&v) == 0;) {
	for (i=0; entries[i].key; i++) {
	    if (strcmp(entries[i].key, k) == 0) {
		if (v != entries[i].value) {
		    printf(" -- FAILED (%s): wrong value\n",
			   entries[i].key);
		    exit(1);
		}
		if (entries[i].had) {
		    printf(" -- FAILED (%s): duplicate\n",
			   entries[i].key);
		    exit(1);
		}
		entries[i].had = 1;
		break;
	    }
	}
	if (entries[i].key == NULL) {
	    printf(" -- FAILED: unknown key\n");
	    exit(1);
	}
    }
    for (i=0; entries[i].key; i++) {
	if (entries[i].had == 0) {
	    printf(" -- FAILED (%s): missed\n", entries[i].key);
	    exit(1);
	}
    }
    printf("\n");
	
    printf("deleting");
    fflush(stdout);

    map_delete(m, entries[0].key, 0);
    if (map_lookup(m, entries[0].key) != NULL) {
	printf(" -- FAILED\n");
	exit(1);
    }
    printf("\n");

    printf("freeing");
    fflush(stdout);
    
    map_free(m, 0);
    printf("\n");

    exit(0);
}
    
