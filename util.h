#include <stdlib.h>

#define BUFSIZE 8192
#define ERRFILESIZE 8192

extern char *prg;
extern char errfilename[];
extern int errlineno, errpartno;
enum errtype={errnone, errfile, errpart, errline};



void *xmalloc(size_t size);
void *xrealloc(void *p, size_t size);
char *expand(char *path);
char *getline(FILE *f);
FILE *fopen_uniq(char **s);
void prerror(enum errtype, char *fmt, ...);
void skip_rest(FILE *f);
