#include <stdio.h>

#include "checkgroup.h"
#include "stream.h"
#include "stream_types.h"
#include "util.h"

struct stream_cat {
    stream st;

    int i;
    struct file *file;
    FILE *cur;
};

static void close_current(struct stream_cat *this);
static int cat_close(struct stream_cat *st);
static token *cat_get(struct stream_cat *st);



stream *
stream_cat_open(struct file *file)
{
    struct stream_cat *this;

    this = (struct stream_cat *)stream_new(sizeof(struct stream_cat),
					   cat_get, cat_close, NULL);

    this->i = 0;
    this->file = file;
    this->cur = NULL;

    return (stream *)this;
}



static int
cat_close(struct stream_cat *this)
{
    close_current(this);
    stream_free((stream *)this);

    return 0;
}



static token *
cat_get(struct stream_cat *this)
{
    token *t;
    int ret;

    if (this->st.source == NULL) {
	if (this->i > this->file->npart) {
	    close_current(this);
	    return TOKEN_EOF;
	}

	/* skip part 0 if it doesn't exist */
	if (this->i == 0 && this->file->artno[0] == -1)
	    this->i++;

	ret = nntp_put("article %ld", this->file->artno[this->i++]);
	if (ret != 220 && ret != 224) {
	    token_set3(stream_enqueue((stream *)this),
		       TOK_ERR, 0, "article failed");
	    return token_set(&this->st.tok, TOK_EOA, NULL);
	}
	
	this->st.source = stream_file_open(conin, 1);
    }

    t = stream_get(this->st.source);

    if (t->type == EOF) {
	close_current(this);
	
	return token_set(&this->st.tok, TOK_EOA, NULL);
    }

    return t;
}



static void
close_current(struct stream_cat *this)
{
    if (this->st.source) {
	stream_close(this->st.source);
	/* fclose(this->cur); */
	this->st.source = NULL;
	this->cur = NULL;
    }
}
