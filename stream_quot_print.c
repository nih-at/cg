/*
  $NiH: stream_quot_print.c,v 1.4 2002/04/16 22:46:14 wiz Exp $

  stream_quot_print.c -- decode MIME quoted printables
  Copyright (C) 2002 Dieter Baron and Thomas Klausner

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
#include <string.h>

#include "stream.h"
#include "stream_types.h"
#include "util.h"

struct stream_quot_print {
    stream st;

    char *buf;		/* buffer */
    int buf_alen;	/* allocation length for buffer */
};

static int qp_close(struct stream_quot_print *st);
static token *qp_get(struct stream_quot_print *st);
static void qp_grow(struct stream_quot_print *st);



stream *
stream_quot_print_open(struct stream *source)
{
    struct stream_quot_print *this;

    this = (struct stream_quot_print *)stream_new(sizeof(*this),
						  qp_get, qp_close, source);
    this->buf = NULL;
    this->buf_alen = 0;

    return (stream *)this;
}



static int
qp_close(struct stream_quot_print *this)
{
    /* XXX: skip to EOF? */

    stream_free((stream *)this);

    return 0;
}



static token *
qp_get(struct stream_quot_print *this)
{
    token *tok;
    char *p, *s;
    int off, len;

    tok = stream_get(this->st.source);

    if (tok->type == TOK_LINE) {
	s = tok->line;
	if ((p=strchr(s, '='))) {
	    off = 0;
	    for (; p; p=strchr(s, '=')) {
		len = p-s;
		if (off+len+2 >= this->buf_alen)
		    qp_grow(this);
		strncpy(this->buf+off, s, len);
		off += len;

		if (p[1]) {
		    this->buf[off++] = str2hex(p+1);
		    s = p+3;
		}
		else {
		    tok = stream_get(this->st.source);
		    if (tok->type != TOK_LINE) {
			token_copy(stream_enqueue((stream*)this), tok);
			token_set3(stream_enqueue((stream*)this), TOK_ERR,
				   TOK_ERR_ERROR,
				   "no line after continuation");
			return token_set(&this->st.tok, TOK_LINE, this->buf);
		    }
		    s = tok->line;
		}
	    }

	    len = strlen(s);
	    if (off+len+1 >= this->buf_alen)
		qp_grow(this);
	    strcpy(this->buf+off, s);
	    return token_set(&this->st.tok, TOK_LINE, this->buf);
	}
    }

    return tok;
}



static void
qp_grow(struct stream_quot_print *this)
{
    if (this->buf_alen == 0)
	this->buf_alen = 128;
    else
	this->buf_alen *= 2;

    this->buf = xrealloc(this->buf, this->buf_alen);
}
