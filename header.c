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
header_read(FILE *f, int dotp)
{
    struct header h, *act;
    char *line, *p;
    int len;

    act = &h;
    h.next = NULL;
    h.value = NULL;
    h.type = NULL;
    
    
/*    if (bsize == 0) {
	bsize=BUFSIZE;
	b=(char *)xmalloc(bsize);
	} */

    while ((line=getline(f)) != NULL) {
	if (line[0] == '\0')
	    break;
	else if (line[0] == ' ') {
	    /* continued header */
	    if (act->value) {
		len = strlen(act->value);
		act->value = (char *)xrealloc(act->value,
					      len+strlen(line)+1);
		strcpy(act->value+len, line);
	    }
	    else {
		if (act == &h)
		    break;

		act->value = strdup(line);
	    }
	}

	for (p=line; (*p > ' ') && (*p < 127); p++) {
	    if (*p == ':') {
		if ((*(p+1) == ' ') || (*(p+1) == '\t')) {
		    *(p++) = '\0';
		    act->next
			= (struct header *)xmalloc(sizeof(struct header));
		    act = act->next;
		    act->next = NULL;
		    act->value = NULL;
		    if ((act->type=intern_caps(line)) == NULL) {
			/* XXX: better handling */
			break;
		    }
		    p = p + strspn (p, " \t");
		    if (strlen(p) > 0)
			if ((act->value=strdup(p)) == NULL) {
			    prerror("strdup failure");
			    exit(1);
			}
		    break;
		}
		else {
		    /* not really a header after all */
		    break;
		}
	    }
	}
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
