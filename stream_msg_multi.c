#include <string.h>

#include "stream.h"
#include "stream_types.h"
#include "util.h"

struct stream_msg_mul {
    stream st;

    char *boundary;
    int len;
};

static int msg_mul_close(struct stream_msg_mul *st);
static token *msg_mul_get(struct stream_msg_mul *st);



stream *
stream_msg_multi_open(struct stream *source, struct mime_hdr *m)
{
    struct stream_msg_mul *this;

    this = (struct stream_msg_mul *)stream_new(sizeof(struct stream_msg_mul),
					       msg_mul_get, msg_mul_close,
					       source);
    
    /* XXX: handle missing MIME_CT_BOUNDARY */
    this->boundary = xstrdup(mime_option_get(m, MIME_CT_BOUNDARY));
    this->len = strlen(this->boundary);

    return (stream *)this;
}



static int
msg_mul_close(struct stream_msg_mul *this)
{
    /* XXX: skip to EOF? */
      
    free(this->boundary);
    stream_free((stream *)this);

    return 0;
}



static token *
msg_mul_get(struct stream_msg_mul *this)
{
    token *t;

    t = stream_get(this->st.source);

    if (t->type == TOK_LINE && t->line[0] == '-' && t->line[1] == '-'
	&& strncmp(t->line+2, this->boundary, this->len) == 0) {
	if (t->line[this->len+2] == '\0')
	    return token_set(&this->st.tok, TOK_EOS, NULL);
	else if (t->line[this->len+2] == '-' && t->line[this->len+3] == '-'
		 && t->line[this->len+4] == '\0')
	    return TOKEN_EOF;
    }

    return t;
}
