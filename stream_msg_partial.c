#include <errno.h>
#include <stddef.h>
#include <string.h>

#include "header.h"
#include "mime.h"
#include "stream.h"
#include "stream_types.h"
#include "util.h"

enum mp_state { MP_HEADER, MP_BODY };

struct stream_mprt {
    stream st;

    enum mp_state state;
    int ct_seen;	/* whether we've seen a Content-Type header */
    int i;	/* number of current article */
    int n;	/* total number of articles */
    char *id;	/* article id */
};

static int get_int(struct mime_hdr *m, symbol opt);
static int mprt_close(struct stream_mprt *st);
static token *mprt_get(struct stream_mprt *st);



stream *
stream_msg_partial_open(struct stream *source, struct mime_hdr *m)
{
    struct stream_mprt *this;

    this = (struct stream_mprt *)stream_new(sizeof(struct stream_mprt),
					       mprt_get, mprt_close, source);

    this->state = MP_BODY;

    if ((this->id=xstrdup(mime_option_get(m, MIME_CT_ID))) == NULL)
	token_set3(stream_enqueue((stream *)this), TOK_ERR, 1,
		   "message/partial missing id");

    if ((this->i = get_int(m, MIME_CT_NUMBER)) < 0)
	token_set3(stream_enqueue((stream *)this), TOK_ERR, 1,
		   "message/partial missing number");
    else if (this->i != 1)
	token_set3(stream_enqueue((stream *)this), TOK_ERR, 1,
		   "message/partial: unexpected part");
    if ((this->n = get_int(m, MIME_CT_TOTAL)) < 0)
	token_set3(stream_enqueue((stream *)this), TOK_ERR, 1,
		   "message/partial missing total");
    
    return (stream *)this;
}



static int
mprt_close(struct stream_mprt *this)
{
    /* XXX: skip to EOF? */
      
    free(this->id);
    stream_free((stream *)this);

    return 0;
}



static token *
mprt_get(struct stream_mprt *this)
{
    token *t;
    struct mime_hdr *m;
    char *s;
    int i;

    for (;;) {
	t = stream_get(this->st.source);
	
	switch (t->type) {
	case TOK_LINE:
	    switch (this->state) {
	    case MP_BODY:
		return t;
		
	    case MP_HEADER:
		if (strncasecmp(t->line, "Content-Type:", 13) == 0) {
		    this->ct_seen = 1;
		    s = t->line + 13;
		    if ((m=mime_parse(s+strspn(s, " \t"))) == NULL) {
			token_set3(stream_enqueue((stream *)this), TOK_ERR, 1,
				   "cannot parse Content-Type header");
		    }
		    else {
			if (m->type != MIME_CT_MSG_PART)
			    token_set3(stream_enqueue((stream *)this),
				       TOK_ERR, 1,
				       "part not of type message/partial");
			else {
			    if (this->id) {
				if ((s=mime_option_get(m, MIME_CT_ID)) == NULL)
				    token_set3(stream_enqueue((stream *)this),
					       TOK_ERR, 1, "part missing id");
				else if (strcmp(this->id, s) != 0)
				    token_set3(stream_enqueue((stream *)this),
					       TOK_ERR, 1, "id mismatch");
			    }
			    if (this->i > 0) {
				if ((i=get_int(m, MIME_CT_NUMBER)) < 0) {
				    token_set3(stream_enqueue((stream *)this),
					       TOK_ERR, 1,
					       "part missing number");
				}
				else if (this->i+1 != i)
				    token_set3(stream_enqueue((stream *)this),
					       TOK_ERR, 1,
					       "unexpected part number");
				this->i++;
			    }
			    if (this->n > 0) {
				if (this->i > this->n)
				    token_set3(stream_enqueue((stream *)this),
					       TOK_ERR, 1,
					       "more than total parts");
				if ((i=get_int(m, MIME_CT_TOTAL)) < 0) {
				    token_set3(stream_enqueue((stream *)this),
					       TOK_ERR, 1,
					       "part missing total");
				}
				else if (this->n != i)
				    token_set3(stream_enqueue((stream *)this),
					       TOK_ERR, 1, "total mismatch");
			    }
			    mime_free(m);
			}
		    }
		}
		break;
	    }
	    break;
	    
	case TOK_EOH:
	    this->state = MP_BODY;
	    if (!this->ct_seen)
		return token_set3(&this->st.tok,
				  TOK_ERR, 1, "missing Content-Type header");
	    break;
	    
	case TOK_EOA:
	    this->state = MP_HEADER;
	    this->ct_seen = 0;
	    break;
	    
	case TOK_EOF:
	    if (this->i>0 && this->n>0 && this->i != this->n)
		token_set3(stream_enqueue((stream *)this), TOK_ERR, 1,
			   "EOF before last part");
	    return TOKEN_EOF;

	default:
	    return t;
	}
    }
}



static int
get_int(struct mime_hdr *m, symbol opt)
{
    char *p, *q;
    int i;

    if ((p=mime_option_get(m, opt)) == NULL)
	return -1;

    errno = 0;
    i = strtol(p, &q, 10);
    if (errno || !q || *q)
	return -1;

    return i;
}
