#include "stream.h"
#include "util.h"

token *
token_new(enum token_type type, char *line)
{
    return token_set(xmalloc(sizeof(token)), type, line);
}



token *
token_set(token *t, enum token_type type, char *line)
{
    return token_set3(t, type, 0, line);
}



token *
token_set3(token *t, enum token_type type, int n, char *line)
{
    t->type = type;
    t->n = n;
    t->line = line;

    return t;
}



void
token_free(token *t)
{
    if (t == NULL)
	return;
    
    free(t->line);
    free(t);
}



token *
token_copy(token *d, token *s)
{
    return token_set3(d, s->type, s->n, s->line);
}
