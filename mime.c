#include <stdlib.h>

#include <mime.h>

void _mime_lws(char **s);
symbol _mime_token(char **s);



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



struct mime_hdr *
mime_parse(char *h)
{
    struct mime_hdr *m;
    struct mime_opt o[256];
    char *s, *q, b[8192];
    int i;
 
    if ((m=(struct mime_hdr *)malloc(sizeof(struct mime_hdr))) == NULL)
	return NULL;

    if ((m->type=_mime_token(&h)) == NULL) {
	free(m);
	return NULL;
    }

    _mime_lws(&h);
    while (*h == ';') {
	if ((o[i].name=_mime_token(&h)) == NULL) {
	    break;
	}
	_mime_lws(&h);
	if (*h == '=')
	    o[i].value = _mime_value(&h);
	else
	    o[i].value = NULL;

        i++;
	/* XXX: handle buffer overrun */
    }

    o[i].name = o[i].value = NULL;
    i++;
    
    if ((m->option=(struct mime_opt *)malloc(sizeof(struct mime_opt)*i))
	== NULL) {
	free(m);
	while (i)
	    free(o[--i].value);
	return NULL;
    }
    memcpy(m->option, o, sizeof(struct mime_opt)*i);

    return m;
}



symbol
_mime_token(char **s)
{
    symbol m;
    char c, *p;

    _mime_lws(s);
    p = s+strcspn(*s, " \t()<>@,;:\\\"[]?=");
    c = *p;
    *p = '\0';
    if (strlen(s) > 0)
	m = intern_lower(*s);
    else
	m = 0;
    *p = c;
    *s = p;
    return m;
}



void
_mime_lws(char **s)
{
    /* XXX: handle () comments */
    
    *s += strspn(" \t");
}



char *
_mime_value(char **s)
{
    

    _mime_lws(s);
    
    if (**s == '"') {
	/* quoted string */
    }
    else {
	/* token */
    }
}
