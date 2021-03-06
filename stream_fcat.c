/*
  $NiH: stream_fcat.c,v 1.5 2002/04/22 22:12:31 dillo Exp $

  stream_fcat.c -- concatenate files
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

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "stream.h"
#include "stream_types.h"
#include "util.h"

struct stream_fcat {
    stream st;

    int n, i;
    char **names;
    FILE *cur;
};

static void close_current(struct stream_fcat *this);
static int fcat_close(struct stream_fcat *st);
static token *fcat_get(struct stream_fcat *st);



stream *
stream_fcat_open(int n, char **names)
{
    struct stream_fcat *this;

    this = (struct stream_fcat *)stream_new(sizeof(struct stream_fcat),
					    fcat_get, fcat_close, NULL);

    this->n = n;
    this->names = names;
    this->i = 0;

    return (stream *)this;
}



static int
fcat_close(struct stream_fcat *this)
{
    close_current(this);
    stream_free((stream *)this);

    return 0;
}



static token *
fcat_get(struct stream_fcat *this)
{
    token *t;

    if (this->st.source == NULL) {
	if (this->i >= this->n) {
	    close_current(this);
	    return TOKEN_EOF;
	}
	if ((this->cur=fopen(this->names[this->i++], "r")) == NULL) {
	    token_printf3(stream_enqueue((stream *)this), TOK_ERR,
			  TOK_ERR_ERROR, "cannot open file `%s': %s",
			  this->names[this->i-1], strerror(errno));
	    return token_set(&this->st.tok, TOK_EOA, NULL);
	}
	this->st.source = stream_file_open(this->cur, 0);
    }

    t = stream_get(this->st.source);

    if (t->type == EOF) {
	close_current(this);

	return token_set(&this->st.tok, TOK_EOA, NULL);
    }

    return t;
}



static void
close_current(struct stream_fcat *this)
{
    if (this->st.source) {
	stream_close(this->st.source);
	fclose(this->cur);
	this->st.source = NULL;
	this->cur = NULL;
    }
}
