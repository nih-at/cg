#ifndef _HAD_STREAM_H
#define _HAD_STREAM_H

/*
  $NiH: stream.h,v 1.5 2002/04/16 22:46:11 wiz Exp $

  stream.h -- stream interface
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

#include <stdlib.h>
#include <stdio.h>

enum token_type { TOK_EOF = -1, TOK_LINE, TOK_DATA,
		  TOK_EOS, TOK_EOH, TOK_EOA, TOK_EOP,
		  TOK_FNAME, TOK_DEBUG, TOK_ERR };

/* if you change this, don't forget to update initializer of tok_eof
   in stream.c */
struct token {
    enum token_type type;
    int n;			/* data len or error number */
    int alloced;		/* is line malloc()ed? */
    char *line;
};

typedef struct token token;

struct token_list {
    token tok;
    struct token_list *next;
};

typedef struct token_list token_list;

struct stream {
    token *(*get)();
    int (*close)();

    token tok;
    token *tok_cleanup;		/* token to free on next get */
    token_list queue;
    token_list *queue_tail;
    int queue_len;
    int queue_cleanup;

    struct stream *source;
    int eof;
};

typedef struct stream stream;



extern token *TOKEN_EOF;



/* internal */
void stream_free(stream *st);
stream *stream_new(size_t size, token *(*get)(),
		   int (*close)(), stream *source);

/* external */
int stream_close(stream *st);
void stream_dequeue(stream *st);
token *stream_enqueue(stream *st);
int stream_eof(stream *st);
token *stream_get(stream *st);
token *stream_queue_peek(stream *st);

#define token_clean(t) ((t)->alloced ? free((t)->line),(t)->line=NULL : NULL)
token *token_copy(token *d, token *s);
void token_free(token *t);
token *token_new(enum token_type type, char *line);
token *token_printf3(token *t, enum token_type type, int n, char *fmt, ...);
token *token_set(token *t, enum token_type type, char *line);
token *token_set3(token *t, enum token_type type, int n, char *line);

#endif /* stream.h */

