#ifndef _HAD_RANGES_H
#define _HAD_RANGES_H

/*
  $NiH: ranges.h,v 1.3 2002/04/10 16:21:18 wiz Exp $

  ranges.h -- internal .newsrc format handling
  Copyright (C) 2002 Dieter Baron and Thomas Klaunser

  This file is part of cg, a program to assemble and decode binary Usenet
  postings.  The authors can be contacted at <nih@giga.or.at>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

struct range {
    int first, length;
    unsigned char *bitmap;
};



struct range *range_init(int lower, int upper, int num);
void range_free(struct range *r);
int range_isin(struct range *r, int n);
void range_set(struct range *r, int n);
void range_clear(struct range *r, int n);
void range_fill(struct range *r, int lower, int upper, int bit);
int range_get(struct range *r, int *lower, int *upper, int bit);

#endif /* ranges.h */
