#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "header.h"

#define HDR_MAX 5

char *hdr_string[HDR_MAX] = {
    "Subject",
    "Mime-Version",
    "Content-Type",
    "Content-Transfer-Encoding",
    "Content-Disposition"
};

symbol hdr_sym[HDR_MAX];



void
header_init(void)
{
    int i;

    for (i=0; i<HDR_MAX; i++)
	hdr_sym[i] = intern(hdr_string[i]);
}



struct header *
header_read(stream *in, out_state *out)
{
    struct header h, *act;
    token *t;
    char *p;

    act = &h;
    h.next = NULL;
    h.value = NULL;
    h.type = NULL;
    

    while ((t=stream_get(in))->type != TOK_EOH && t->type != TOK_EOF) {
	if (t->type == TOK_LINE) {
	    if (strncmp(t->line, "From ", 5) == 0) {
		/* mbox envelope */
		continue;
	    }
	    else {
		for (p=t->line; (*p > ' ') && (*p < 127); p++) {
		    if (*p == ':') {
			if ((*(p+1) == ' ') || (*(p+1) == '\t')) {
			    *(p++) = '\0';
			    act->next = xmalloc(sizeof(struct header));
			    act = act->next;
			    act->next = NULL;
			    act->value = NULL;
			    if ((act->type=intern_caps(t->line)) == NULL) {
				/* XXX: better handling */
				break;
			    }
			    p = p + strspn (p, " \t");
			    if (strlen(p) > 0)
			    act->value = xstrdup(p);
			    break;
			}
			else {
			    /* not really a header after all */
			    break;
			}
		    }
		}
	    }
	}
	else
	    output(out, t);
    }
	
    return h.next;
}



void
header_free(struct header *h)
{
    struct header *h2;

    while (h) {
	h2 = h;
	h = h->next;
	free(h2->value);
	free(h2);
    }
}



char *
header_get(struct header *h, symbol field)
{
    for (; h; h=h->next) {
	if (h->type == field)
	    return h->value;
    }

    return NULL;
}
