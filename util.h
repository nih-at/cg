#ifndef _HAD_UTIL_H
#define _HAD_UTIL_H

#include <stdlib.h>

#include "decode.h"
#include "stream.h"

#define BUFSIZE 8192
#define ERRFILESIZE 8192

extern char *prg;
extern char errfilename[];
extern int errlineno, errpartno;
enum errtype {errnone, errfile, errpart, errline};



int append_file(char *t, char *s, char *sep);
void copy_stream(stream *in, out_state *out);
void debug(out_state *out, char *fmt, ...);
char *expand(char *path);
FILE *fopen_uniq(char **s);
char *getline(FILE *f);
void prerror(enum errtype, char *fmt, ...);
void skip_rest(FILE *f);
token *skip_to(stream *in, enum token_type type);
char *our_basename(char *name);
void *xmalloc(size_t size);
void *xrealloc(void *p, size_t size);
char *xstrdup(char *s);

#endif /* util.h */
