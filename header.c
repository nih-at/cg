#include <stdio.h>
#include <stdlib.h>

#include "header.h"



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
    struct header *h;

    
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
