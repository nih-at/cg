#include <stdlib.h>

#include <mime.h>



void
mime_init(void)
{
    int i;

    for (i=0; i<MIME_MAX; i++)
	mime_sym[i] = intern(mime_string[i]);
}



void
mime_free(struct mime_hdr *m)
{
    int i;

    if (m == NULL)
	return;
    
    for (i=0; m->option[i].name; i++)
	free(m->option[i].val);

    free(m->option);
    free(m);
}


