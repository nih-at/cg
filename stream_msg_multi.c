/*
  $NiH: stream_msg_multi.c,v 1.2 2002/04/10 16:21:21 wiz Exp $

  stream_msg_multi.c -- parse MIME multipart message
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

#include <string.h>

#include "stream.h"
#include "stream_types.h"
#include "util.h"

struct stream_msg_mul {
    stream st;

    char *boundary;
    int len;
};

static int msg_mul_close(struct stream_msg_mul *st);
static token *msg_mul_get(struct stream_msg_mul *st);



stream *
stream_msg_multi_open(struct stream *source, struct mime_hdr *m)
{
    struct stream_msg_mul *this;

    this = (struct stream_msg_mul *)stream_new(sizeof(struct stream_msg_mul),
					       msg_mul_get, msg_mul_close,
					       source);

    /* XXX: handle missing MIME_CT_BOUNDARY */
    this->boundary = xstrdup(mime_option_get(m, MIME_CT_BOUNDARY));
    this->len = strlen(this->boundary);

    return (stream *)this;
}



static int
msg_mul_close(struct stream_msg_mul *this)
{
    /* XXX: skip to EOF? */

    free(this->boundary);
    stream_free((stream *)this);

    return 0;
}



static token *
msg_mul_get(struct stream_msg_mul *this)
{
    token *t;

    t = stream_get(this->st.source);

    if (t->type == TOK_LINE && t->line[0] == '-' && t->line[1] == '-'
	&& strncmp(t->line+2, this->boundary, this->len) == 0) {
	if (t->line[this->len+2] == '\0')
	    return token_set(&this->st.tok, TOK_EOS, NULL);
	else if (t->line[this->len+2] == '-' && t->line[this->len+3] == '-'
		 && t->line[this->len+4] == '\0')
	    return TOKEN_EOF;
    }

    return t;
}
