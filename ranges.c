/*
  $NiH$
  
  ranges.c -- internal .newsrc format handling
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "util.h"
#include "ranges.h"

static void _range_grow_left(struct range *r, int low);
static void _range_grow_right(struct range *r, int high);
static int _range_findbit(struct range *r, int first, int bit);



struct range *
range_init(int lower, int upper, int num)
{
    struct range *r;

    r=(struct range *)xmalloc(sizeof(struct range));

    /* XXX: check for enormous (>100000) ranges */

    r->first = lower;
    r->length = (upper-lower+1+7)/8;
    r->length = r->length*8;
    r->bitmap=(char *)xmalloc(r->length/8);

    memset(r->bitmap, 0, r->length/8);

    return r;
}



void
range_free(struct range *r)
{
    free(r->bitmap);
    free(r);
}



int
range_isin(struct range *r, int n)
{
    if (n < r->first)
	return 1;
    if (n >= r->first+r->length)
	return 0;

    n -= r->first;

    return r->bitmap[n/8]&(0x80>>(n%8));
}



void
range_set(struct range *r, int n)
{
    if (n < r->first)
	return;
    if (n >= r->first+r->length) {
	_range_grow_right(r, n);
    }

    n -= r->first;

    r->bitmap[n/8] |= (0x80>>(n%8));
}



void
range_clear(struct range *r, int n)
{
    if (n < r->first)
	_range_grow_left(r, n);
    if (n >= r->first+r->length)
	return;

    n -= r->first;
    
    r->bitmap[n/8] &= ~(0x80>>(n%8));
}



void
range_fill(struct range *r, int lower, int upper, int bit)
{
    int first, length, loff, roff;

    if (upper < lower) {
	lower ^= upper;
	upper ^= lower;
	lower ^= upper;
    }

    if (bit && upper < r->first)
	return;
    if (!bit && lower >= r->first+r->length)
	return;

    if (lower < r->first) {
	if (bit)
	    lower = r->first;
	else
	    _range_grow_left(r, lower);
    }
    if (upper >= r->first+r->length) {
	if (!bit)
	    upper = r->first+r->length-1;
	else
	    _range_grow_right(r, upper);
    }

    upper++; /* right end is inclusive */

    first = (lower-r->first+7)/8;
    loff = first*8-(lower-r->first);
    length = (upper-lower-loff)/8;
    roff = upper-lower-loff-length*8;

    if (length)
	memset(r->bitmap+first, (bit ? 255 : 0), length);
    if (bit) {
	if (loff)
	    r->bitmap[first-1] |= 0xff >> (8-loff);
	if (roff)
	    r->bitmap[first+length] |= (0xff << (8-roff)) & 0xff;
    }
    else {
	if (loff)
	    r->bitmap[first-1] &= (0xff << loff) & 0xff;
	if (roff)
	    r->bitmap[first+length] &= 0xff >> roff;
    }
}



int
range_get(struct range *r, int *lower, int *upper, int bit)
{
    int low;

    if (*upper == -1)
	return -1;
    if ((low=_range_findbit(r, (*upper)+1, bit)) == -1)
	return -1;
    *lower = low;
    *upper = _range_findbit(r, low+1, !bit)-1;

    return 0;
}



static void
_range_grow_left(struct range *r, int low)
{
    int oldlen;

    oldlen = r->length;

    r->length += r->first-low+512;
    r->length = (r->length+7)/8;
    r->length *= 8;
    r->first = low-512;
    if ((r->bitmap=(unsigned char *)realloc(r->bitmap, r->length/8))
	== NULL) {
	fprintf(stderr, "%s: realloc failure\n", prg);
	exit(1);
    }

    memmove(r->bitmap+((r->length-oldlen)/8), r->bitmap, (oldlen/8));
    memset(r->bitmap, 0, (r->length-oldlen)/8);
}



static void
_range_grow_right(struct range *r, int high)
{
    int oldlen;

    oldlen = r->length;
    
    r->length = (high-r->first+512+7)/8;
    r->length *= 8;
    if ((r->bitmap=(unsigned char *)realloc(r->bitmap, r->length/8))
	== NULL) {
	fprintf(stderr, "%s: realloc failure\n", prg);
	exit(1);
    }

    memset(r->bitmap+(oldlen/8), 0, (r->length-oldlen)/8);
}



static int
_range_findbit(struct range *r, int first, int bit)
{
    int fbyte, lbits, mask, value;

    if (first < r->first) {
	if (bit)
	    return first;
	else
	    first = r->first;
    }
    if (first >= r->first+r->length) {
	if (bit)
	    return -1;
	else
	    return first;
    }

    fbyte = (first-r->first)/8;
    lbits = r->first+(fbyte+1)*8 - first;

    if (lbits != 0) {
	mask = 0x01 << (lbits-1);
	value = (bit ? mask : 0);

	for (; mask; mask>>=1,value>>=1,first++) {
	    if ((r->bitmap[fbyte]&mask) == value)
		return first;
	}
	fbyte++;
    }

    value = (bit ? 0 : 0xff);

    while (r->bitmap[fbyte] == value) {
	fbyte++;
	first += 8;
	if (fbyte >= r->length/8) {
	    if (bit)
		return -1;
	    return first;
	}
    }

    mask = 0x80;
    value = (bit ? 0x80 : 0);

    for (; mask; mask>>=1,value>>=1,first++) {
	if ((r->bitmap[fbyte]&mask) == value)
	    return first;
    }

    /* can't happen */
    return -1;
}
