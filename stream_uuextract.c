/*
  $NiH: stream_uuextract.c,v 1.4 2002/04/10 16:21:23 wiz Exp $

  stream_uuextract.c -- extrace uuencode data
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

#include "header.h"
#include "stream.h"
#include "stream_types.h"
#include "util.h"

enum uu_state { UU_FULL, UU_SHORT, UU_EMPTY, UU_END, UU_POST, UU_PRE,
		UU_HEADER };

struct stream_uux {
    stream st;

    enum uu_state state;
    char buf[66];
};

static int uux_close(struct stream_uux *st);
static token *uux_get(struct stream_uux *st);



stream *
stream_uuextract_open(struct stream *source)
{
    struct stream_uux *this;

    this = (struct stream_uux *)stream_new(sizeof(struct stream_uux),
					   uux_get, uux_close, source);

    this->state = UU_FULL;

    return (stream *)this;
}



static int
uux_close(struct stream_uux *this)
{
    /* XXX: skip to EOF? */

    stream_free((stream *)this);

    return 0;
}



static token *
uux_get(struct stream_uux *this)
{
    token *t;
    int len, line_kept;
    enum uu_state state, old_state;

    old_state = this->state;
    line_kept = 0;

    for (;;) {
	t = stream_get(this->st.source);

	switch (t->type) {
	case TOK_LINE:
	    len = strlen(t->line);

	    if (old_state == UU_HEADER)
		state = UU_HEADER;
	    else if (t->line[0] == 'M' && len == 61)
		state = UU_FULL;
	    else if (len == 1 && (t->line[0] == ' ' || t->line[0] == '`'))
		state = UU_EMPTY;
	    else if (strcmp(t->line, "end") == 0)
		state = UU_END;
	    else if (t->line[0] > ' ' && t->line[0] < 'M'
		     && (len-1+3)/4 == (t->line[0]-' '+2)/3)
		state = UU_SHORT;
	    else
		state = UU_POST;

	    switch (state) {
	    case UU_HEADER:
		break;

	    case UU_END:
		if (line_kept)
		    token_set(stream_enqueue((stream *)this),
			      TOK_LINE, this->buf);
		return TOKEN_EOF;

	    case UU_EMPTY:
		if (old_state == UU_EMPTY) {
		    token_set3(stream_enqueue((stream *)this), TOK_ERR, 1,
			       "end expected after empty line");
		}
		break;

	    case UU_FULL:
		if (old_state != UU_FULL && old_state != UU_PRE) {
		    /* error: end or empty expected */
		    token_set3(stream_enqueue((stream *)this), TOK_ERR, 1,
			       "long line unexpected");
		}
		this->state = UU_FULL;
		return token_set(&this->st.tok, TOK_LINE, t->line+1);

	    case UU_SHORT:
		if (old_state != UU_FULL && old_state != UU_PRE) {
		    token_set3(stream_enqueue((stream *)this), TOK_ERR, 1,
			       "unexpected short line");
		}
		strcpy(this->buf, t->line+1);
		line_kept = 1;
		break;

	    case UU_POST:
		if (old_state == UU_PRE)
		    state = old_state;
		break;

	    default:
		break;
	    }
	    break;

	case TOK_EOA:
	    state = UU_HEADER;
	    break;

	case TOK_EOF:
	    if (line_kept)
		token_set(stream_enqueue((stream *)this), TOK_LINE, this->buf);
	    token_set3(stream_enqueue((stream *)this), TOK_ERR, 1,
		       "missing end in uu stream");
	    return TOKEN_EOF;

	case TOK_EOH:
	    state = UU_PRE;
	    break;

	default:
	    return t;
#if 0
	    token_set3(stream_enqueue((stream *)this), TOK_ERR, 1,
		       "unexpected token type");
	    if (old_state == UU_PRE || old_state == UU_HEADER)
		state = old_state;
	    else
		state = UU_POST;
	    break;
#endif
	}

	old_state = state;
    }
}
