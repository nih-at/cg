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
    /* XXX: preliminary version only */
    if (fgets(this->buf, this->buf_len, this->f) == NULL)
	return TOKEN_EOF;

    this->buf[strlen(this->buf)-1] = '\0';
    return token_set(&this->st.tok, TOK_LINE, this->buf);
}
