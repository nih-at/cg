#include <stdlib.h>

#define BUFSIZE 8192

extern char *prg;



void *xmalloc(size_t size);
char *expand(char *path);
char *getline(FILE *f);
FILE *fopen_uniq(char **s);
void prerror(char *fmt, ...);
