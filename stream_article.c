#include <ctype.h>
#include <errno.h>
#include <stddef.h>

#include "header.h"
#include "mime.h"
#include "stream.h"
#include "stream_types.h"
#include "util.h"

enum art_state { AR_EMPTY, AR_HEADER, AR_BODY };

struct stream_art {
    stream st;

    enum art_state state;
    int kept;		/* have we kept (part of) a header */
    int bufno;		/* buffer number to use for next header */
    char *buf[2];	/* buffers */
    int buflen[2];	/* length of line in buffers */
    int buf_alen[2];	/* allocation length of buffers */
};

static int art_close(struct stream_art *st);
static token *art_get(struct stream_art *st);
static void grow_buffer(struct stream_art *this, int bufno, int size);



stream *
stream_article_open(struct stream *source)
{
    struct stream_art *this;

    this = (struct stream_art *)stream_new(sizeof(struct stream_art),
					       art_get, art_close, source);

    this->state = AR_EMPTY;
    this->bufno = this->kept = 0;
    this->buflen[0] = this->buflen[1] = 0;
    this->buf_alen[0] = this->buf_alen[1] = 0;
    this->buf[0] = this->buf[1] = NULL;

    return (stream *)this;
}



static int
art_close(struct stream_art *this)
{
    /* XXX: skip to EOF? */
      
    free(this->buf[0]);
    free(this->buf[1]);
    stream_free((stream *)this);

    return 0;
}



static token *
art_get(struct stream_art *this)
{
    token *t;
    int len;

    for (;;) {
	t=stream_get(this->st.source);

	if (this->state != AR_BODY) {
	    if (t->type == TOK_LINE) {
		len = strlen(t->line);

		if (len == 0) {
		    if (this->kept) {
			token_set(stream_enqueue((stream *)this),
				  TOK_LINE, this->buf[this->bufno]);
			this->kept = 0;
			this->bufno = 1-this->bufno;
		    }

		    this->state = AR_BODY;
		    return token_set(&this->st.tok, TOK_EOH, NULL);
		}
		else if (!isspace(t->line[0])) {
		    /* XXX: check if valid header line */

		    this->state = AR_HEADER;
		    this->bufno = 1-this->bufno;
		    
		    if (len+1 >= this->buf_alen[this->bufno])
			grow_buffer(this, this->bufno, len+1);
		    strcpy(this->buf[this->bufno], t->line);
		    this->buflen[this->bufno] = len;

		    if (this->kept) {
			return token_set(&this->st.tok,
					 TOK_LINE, this->buf[1-this->bufno]);
		    }
		    else
			this->kept = 1;
		}
		else {
		    if (this->buflen[this->bufno]+len+1
			>= this->buf_alen[this->bufno])
			grow_buffer(this, this->bufno,
				    this->buflen[this->bufno]+len+1);
		    strcpy(this->buf[this->bufno]+this->buflen[this->bufno],
			   t->line);
		    this->buflen[this->bufno] += len;
		}
	    }
	    else if (t->type == TOK_EOA || t->type == TOK_EOF) {
		if (this->kept) {
		    token_set(stream_enqueue((stream *)this),
			      TOK_LINE, this->buf[this->bufno]);
		    this->kept = 0;
		    this->bufno = 1-this->bufno;
		}

		if (this->state == AR_HEADER)
		    token_set(stream_enqueue((stream *)this), TOK_EOH, NULL);
		return t;
	    }
	    else
		return t;
	}
	else {
	    if (t->type == TOK_EOA)
		this->state = AR_EMPTY;
	    return t;
	}
    }
}



static void
grow_buffer(struct stream_art *this, int bufno, int size)
{
    if (this->buf_alen[bufno] == 0)
	this->buf_alen[bufno] = 64;
    while (this->buf_alen[bufno] <= size)
	this->buf_alen[bufno] *= 2;
	
    this->buf[bufno] = xrealloc(this->buf[bufno], this->buf_alen[bufno]);
}
