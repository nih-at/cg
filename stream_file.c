/*
  $NiH: stream_file.c,v 1.4 2002/04/10 16:23:34 wiz Exp $

  stream_file.c -- read single file
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

#include <stdio.h>

#include "stream.h"
#include "util.h"



struct stream_file {
    stream st;

    FILE *f;
    char *buf;
    int buf_len;
    int is_nntp;
};

static int file_close(struct stream_file *st);
static token *file_get(struct stream_file *st);



stream *
stream_file_open(FILE *f, int is_nntp)
{
    struct stream_file *this;

    this = (struct stream_file *)stream_new(sizeof(struct stream_file),
					    file_get, file_close, NULL);
    this->f = f;
    this->is_nntp = is_nntp;
    this->buf_len = 1024;
    this->buf = xmalloc(this->buf_len);

    return (stream *)this;
}



static int
file_close(struct stream_file *this)
{
    int ret;

    ret = ferror(this->f);
    free(this->buf);
    stream_free((stream *)this);
    return ret;
}



static token *
file_get(struct stream_file *this)
{
    int off, len;

    /* XXX: preliminary version only */
    if (fgets(this->buf, this->buf_len, this->f) == NULL)
	return TOKEN_EOF;

    len = strlen(this->buf);
    if (this->buf[len-2] == '\r') {
	this->buf[len-2] = '\0';
	--len;
    }
    else
	this->buf[len-1] = '\0';
    --len;

    off = 0;
    if (this->is_nntp && this->buf[0] == '.') {
	off = 1;
	if (len == 1)
	    return TOKEN_EOF;
    }
    return token_set(&this->st.tok, TOK_LINE, this->buf+off);
}
