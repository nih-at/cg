#include <stdio.h>
#include <stdlib.h>

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



#if 0
struct header *
header_read(FILE *f, int dotp)
{
    struct header *h;

    
}
#endif



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
