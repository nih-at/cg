#ifndef _HAD_HEADER_H
#define _HAD_HEADER_H

/*
  $NiH$
  
  header.h -- RFC822 header parsing
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

#include "output.h"
#include "stream.h"
#include "symbol.h"

struct header {
    struct header *next;
    symbol type;
    char *value;
};



#define HDR_SUBJECT		hdr_sym[0]
#define HDR_MIME_VERSION	hdr_sym[1]
#define HDR_CONTENT_TYPE	hdr_sym[2]
#define HDR_CONTENT_TRENC	hdr_sym[3]
#define HDR_CONTENT_DISP	hdr_sym[4]

extern symbol hdr_sym[];



struct header *header_read(stream *stm, out_state *out);
void header_free(struct header *h);
char *header_get(struct header *header, symbol field);
void header_init(void);
int header_print(FILE *f, struct header *header);

#endif /* header.h */
