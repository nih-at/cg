#include <stddef.h>

#include "stream.h"
#include "stream_types.h"
#include "util.h"

struct stream_section {
    stream st;

    char *boundary;
};

static int sec_close(struct stream_section *st);
static token *sec_get(struct stream_section *st);



stream *
stream_section_open(struct stream *source, char *boundary)
{
    struct stream_section *this;

    this = (struct stream_section *)stream_new(sizeof(struct stream_section),
					       sec_get, sec_close, source);
    this->boundary = xstrdup(boundary);

    return (stream *)this;
}



static int
sec_close(struct stream_section *this)
{
    /* XXX: skip to EOF? */
      
    free(this->boundary);
    stream_free((stream *)this);

    return 0;
}



static token *
sec_get(struct stream_section *this)
{
    token *t;

    t = stream_get(this->st.source);

    if (t->type == TOK_EOS || (this->boundary
			       && t->type == TOK_LINE
			       && strcmp(t->line, this->boundary) == 0)) {
	return TOKEN_EOF;
    }

    return t;
}
