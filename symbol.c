#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "symbol.h"

#define TBLSIZE 2047

struct entry {
    struct entry *next;
    symbol s;
};

struct entry table[TBLSIZE];

int yourhash(char *s);



symbol
intern(char *s)
{
    int i;
    struct entry *e;
    
    i = yourhash(s);

    if (table[i].s) {
	for (e=&table[i]; e->next; e = e->next) {
	    if (strcmp(e->s, s) == 0)
		return e->s;
	}
	if ((e->next=(struct entry *)malloc(sizeof(struct entry))) == NULL)
	    return NULL;

	if ((e->next->s=strdup(s)) == NULL) {
	    free(e->next);
	    e->next = NULL;
	    return NULL;
	}
	e=e->next;
	e->next = NULL;
	return e->s;
    }

    return table[i].s = strdup(s);
}



symbol
intern_lower(char *s)
{
    char b[8192], *t;

    for (t=b; (*t=tolower(*s)); t++,s++)
	;

    return intern(b);
}



symbol
intern_caps(char *s)
{
    char b[8192], *t;
    int c, id;

    for (c=1,id=0,t=b; (*t=(c?toupper:tolower)(*s)); t++,s++) {
	switch (*t) {
	case '-':
	    c = 1;
	    break;
	case 'I':
	    if (c)
		id = 1;
	    c = 0;
	    break;
	case 'd':
	    if (id)
		*t = 'D';
	    id = c = 0;
	    break;
	default:
	    c = 0;
	}
    }

    return intern(b);
}



int
yourhash(char *s)
{
    int i, j, k;

    i = j = 0;
    
    while (*(s+i) != '\0') {
	j += (i+1)*(int)*(i+s);
	j %= TBLSIZE;
	i++;
    }

    k = j;

    j *= j;
    j %= TBLSIZE;
    j *= k;
    j %= TBLSIZE;

    return j;
    
}
