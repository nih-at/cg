/*
  $NiH: stream_decode.c,v 1.3 2002/04/10 16:23:33 wiz Exp $

  stream_decode.c -- low-level decoding
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

enum decode_state { DS_OK, DS_ILL, DS_EOF };

struct stream_decode {
    stream st;

    int *table;			/* decoding table to use */

    enum decode_state state;	/* state we're in */
    char *buf;			/* buffer for decoded data */
    int buf_alen;		/* allocation length of buf */
    int rest;			/* partially decoded quadruple */
    int no;			/* number of characters in rest */
};

static int dec_close(struct stream_decode *st);
static int dec_fill(struct stream_decode *this, int i, int no, int rest);
static token *dec_get(struct stream_decode *st);



stream *
stream_decode_open(struct stream *source, int *table)
{
    struct stream_decode *this;

    this = (struct stream_decode *)stream_new(sizeof(struct stream_decode),
					      dec_get, dec_close, source);

    this->table = table;
    this->rest = this->no = 0;
    this->buf = NULL;
    this->buf_alen = 0;
    this->state = DS_OK;

    return (stream *)this;
}



static int
dec_close(struct stream_decode *this)
{
    /* XXX: skip to EOF? */

    stream_free((stream *)this);

    return 0;
}



static token *
dec_get(struct stream_decode *this)
{
    token *t;
    int rest, no, b, i;

    switch (this->state) {
    case DS_ILL:
	/* XXX: error number and text */
	this->state = DS_EOF;
	return token_set(&this->st.tok, TOK_ERR, NULL);

    case DS_EOF:
	return TOKEN_EOF;

    case DS_OK:
	t = stream_get(this->st.source);

	rest = this->rest;
	no = this->no;
	i = 0;

	if (t->type != TOK_LINE) {
	    if (no) {
		i = dec_fill(this, 0, no, rest);
		token_set3(stream_enqueue((stream *)this),
			   TOK_DATA, i, this->buf);
		this->rest = this->no = 0;
	    }
	    return t;
	}

	while (*t->line) {
	    b = this->table[(unsigned char)*(t->line++)];
	    if (b < 0) {
		switch (b) {
		case DEC_END:
		    if (this->state != DS_OK)
			this->state = DS_ILL;
		    else if (no == 0)
			this->state = DS_EOF;
		    else
			this->state = DS_ILL;
		    break;

		case DEC_PAD:
		    if (this->state == DS_OK) {
			switch (no) {
			case 0:
			case 1:
			    this->state = DS_ILL;
			    break;
			case 2:
			case 3:
			    this->state = DS_EOF;
			    break;
			}
		    }
		    break;

		default:
		    /* XXX: recover, don't abort decoding */
		    no = 0;
		    this->state = DS_ILL;
		}
	    }
	    else {
		rest = (rest << 6) | (b & 0x3f);
		no++;
	    }

	    if (no == 4 || this->state != DS_OK) {
		i += dec_fill(this, i, no, rest);
		rest = no = 0;
	    }
	}
    }

    this->rest = rest;
    this->no = no;

    return token_set3(&this->st.tok, TOK_DATA, i, this->buf);
}



static int
dec_fill(struct stream_decode *this, int i, int no, int rest)
{
    if (i+2 >= this->buf_alen) {
	this->buf_alen = (this->buf_alen ? this->buf_alen*2 : 64);
	this->buf = xrealloc(this->buf, this->buf_alen);
    }

    switch (no) {
    case 2:
	this->buf[i++] = rest >> 4;
	break;

    case 3:
	this->buf[i++] = rest >> 10;
	this->buf[i++] = (rest>>2) & 0xff;
	break;
	
    case 4:
	this->buf[i++] = rest >> 16;
	this->buf[i++] = (rest>>8) & 0xff;
	this->buf[i++] = (rest & 0xff);
	break;
    }

    return no-1;
}
