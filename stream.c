#include "stream.h"
#include "util.h"



static token _tok_eof = { TOK_EOF, NULL };
token * TOKEN_EOF = &_tok_eof;



token *
stream_get(stream *st)
{
    token *t;

    if (st->queue_cleanup) {
	stream_dequeue(st);
	st->queue_cleanup = 0;
    }

    if (st->eof)
	return TOKEN_EOF;

    if (st->queue_len == 0)
	t = st->get(st);
    else
	t = NULL;
    if (st->queue_len > 0) {
	if (t)
	    token_copy(stream_enqueue(st), t);
	t = stream_queue_peek(st);
	st->queue_cleanup = 1;
    }
    if (t->type == TOK_EOF)
	st->eof = 1;

#if STREAM_TRACE
    printf("stream_get(%p) = %d [%s]\n", st, t->type, t->line);
    fflush(stdout);
#endif
    
    return t;
}



int
stream_close(stream *st)
{
    while (st->queue_len > 0)
	stream_dequeue(st);

    return st->close(st);
}



stream *
stream_new(size_t size, token *(*get)(), int (*close)(), stream *source)
{
    stream *st;

    st = xmalloc(size);
    st->close = close;
    st->get = get;
    st->source = source;
    st->eof = 0;

    st->queue.next = NULL;
    st->queue_tail = &st->queue;
    st->queue_len = 0;
    st->queue_cleanup = 0;

    return st;
}



void
stream_free(stream *st)
{
    if (st == NULL)
	return;
    free(st);
}



int
stream_eof(stream *st)
{
    return st->eof;
}



token *
stream_enqueue(stream *st)
{
    token_list *e;

    e = xmalloc(sizeof(*e));

    e->next = NULL;
    st->queue_tail->next = e;
    st->queue_tail = e;
    st->queue_len++;

    return &e->tok;
}



void
stream_dequeue(stream *st)
{
    token_list *e;
    
    if (st->queue_len <= 0)
	return;

    e = st->queue.next;
    st->queue.next = e->next;
    if (--st->queue_len == 0)
	st->queue_tail = &st->queue;

    free(e);
}



token *
stream_queue_peek(stream *st)
{
    if (st->queue_len <= 0)
	return NULL;

    return &st->queue.next->tok;
}
