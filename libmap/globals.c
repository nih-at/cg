/*
  globals -- global variables
  Copyright (C) 1996 Dieter Baron

  This file is part of libmap, a library for associative arrays.

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



#include <string.h>
#include "map.h"
#include "config.h"

#ifndef HAVE_STRDUP
char *map__strdup(const char *s);
#define strdup map__strdup
#endif



int map_def_size = 1024;
int (*map_def_equal)(void *key1, void *key2) = strcmp;
unsigned int (*map_def_hash)(void *key, int size) = map_strhash;
void *(*map_def_cpykey)(void *key) = strdup;
void (*map_def_delkey)(void *key) = free;
void (*map_def_delval)(void *value)) = NULL;
