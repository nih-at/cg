#include <errno.h>
#include <stddef.h>

#include "header.h"
#include "mime.h"
#include "stream.h"
#include "stream_types.h"
#include "util.h"

struct stream_art {
    stream st;

    int inheader;	/* are we in the article's header */
    int kept;		/* have we kept (part of) a header */
    int bufno;		/* buffer number to use for next header */
    char *buf[2];	/* buffers */
    int buflen[2];	/* length of line in buffers */
    int buf_alen[2];	/* allocation length of buffers */
};

static int art_close(struct stream_art *st);
static token *art_get(struct stream_art *st);
static void grow_buffer(struct stream_art *this, int bufno);



stream *
stream_article_open(struct stream *source)
{
    struct stream_art *this;

    this = (struct stream_art *)stream_new(sizeof(struct stream_art),
					       art_get, art_close, source);

    this->inheader = 1;
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

	if (this->inheader) {
	    if (t->type == TOK_LINE && isspace(t->line[0])) {
		len = strlen(t->line);
		if (this->buflen[this->bufno]+len+1
		    >= this->buf_alen[this->bufno])
		    grow_buffer(this, this->bufno);
		strcpy(this->buf[this->bufno]+this->buflen[this->bufno],
		       t->line);
		this->buflen[this->bufno] += len;
	    }
	    else if (this->kept) {
		token_set(stream_enqueue((stream *)this),
			  TOK_LINE, this->buf[this->bufno]);
		this->kept = 0;
		this->bufno = 1-this->bufno;
	    }
	    
	    if (t->type == TOK_EOA || t->type == TOK_EOF) {
		token_set(stream_enqueue((stream *)this), TOK_EOH, NULL);
		return t;
	    }
	    else if (t->type == TOK_LINE) {
		len = strlen(t->line);

		/* XXX: check if valid header line */

		if (len+1 >= this->buf_alen[this->bufno])
		    grow_buffer(this, this->bufno);
		strcpy(this->buf[this->bufno], t->line);
		this->buflen[this->bufno] += len;
	    }
	    else
		return t;
	}
	else {
	    if (t->type == TOK_EOA)
		this->inheader = 1;
	    return t;
	}
    }
}



static void
grow_buffer(struct stream_art *this, int bufno)
{
    if (this->buf_alen[bufno] == 0)
	this->buf_alen[bufno] = 64;
    else
	this->buf_alen[bufno] *= 2;

    this->buf[bufno] = xrealloc(this->buf[bufno], this->buf_alen[bufno]);
}
