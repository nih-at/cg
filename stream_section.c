/*
  $NiH$
  
  stream_section.c -- read one section from MIME multipart message
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

#include <stddef.h>

#include "stream.h"
#include "stream_types.h"
#include "util.h"

struct stream_section {
    stream st;

    char *boundary;
};

static int sec_close(struct stream_section *st);
static token *sec_get(struct stream_section *st);



stream *
stream_section_open(struct stream *source, char *boundary)
{
    struct stream_section *this;

    this = (struct stream_section *)stream_new(sizeof(struct stream_section),
					       sec_get, sec_close, source);
    this->boundary = xstrdup(boundary);

    return (stream *)this;
}



static int
sec_close(struct stream_section *this)
{
    /* XXX: skip to EOF? */
      
    free(this->boundary);
    stream_free((stream *)this);

    return 0;
}



static token *
sec_get(struct stream_section *this)
{
    token *t;

    t = stream_get(this->st.source);

    if (t->type == TOK_EOS || (this->boundary
			       && t->type == TOK_LINE
			       && strcmp(t->line, this->boundary) == 0)) {
	return TOKEN_EOF;
    }

    return t;
}
