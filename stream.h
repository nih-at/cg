#ifndef _HAD_STREAM_H
#define _HAD_STREAM_H

#include <stdlib.h>
#include <stdio.h>

enum token_type { TOK_EOF = -1, TOK_LINE, TOK_DATA,
		  TOK_EOS, TOK_EOH, TOK_EOA,
		  TOK_FNAME, TOK_DEBUG, TOK_ERR };

struct token {
    enum token_type type;
    int n;			/* data len or error number */
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

int stream_close(stream *st);
void stream_dequeue(stream *st);
token *stream_enqueue(stream *st);
int stream_eof(stream *st);
token *stream_get(stream *st);
token *stream_queue_peek(stream *st);

token *token_new(enum token_type type, char *line);
token *token_copy(token *d, token *s);
void token_free(token *t);
token *token_set(token *t, enum token_type type, char *line);
token *token_set3(token *t, enum token_type type, int n, char *line);

#endif /* stream.h */

