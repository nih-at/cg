#ifndef _HAD_UTIL_H
#define _HAD_UTIL_H

#include <stdlib.h>

#include "decode.h"
#include "header.h"
#include "output.h"
#include "stream.h"

#define BUFSIZE 8192
#define ERRFILESIZE 8192

#define DEBUG_SUBJ	0x0001	/* unrecognized subjects */
#define DEBUG_PART	0x0002	/* duplicate/illegal parts */
#define DEBUG_XOVR	0x0004	/* weird xover lines */
#define DEBUG_PACK	0x0008  /* subject line tags */
#define DEBUG_LINE	0x0010	/* line tokens outside files */
#define DEBUG_DEBUG	0x0020	/* debug output */
#define DEBUG_TOK	0x0040	/* other tokens */
#define DEBUG_FNAM	0x0080	/* output file name */
#define DEBUG_ERROR	0x0100	/* error output */
#define DEBUG_SIZE	0x0200	/* size of output file */

#define DEBUG_ALL	0xffff	/* all debugging */

extern char *prg;



int append_file(char *t, char *s, char *sep);
void copy_stream(stream *in, out_state *out);
void debug(out_state *out, char *fmt, ...);
char *expand(char *path);
FILE *fopen_uniq(char **s);
char *getline(FILE *f);
void output_header(out_state *out, symbol name, struct header *h);
void prdebug(int level, char *fmt, ...);
void prdebug_init(int do_file, int do_stdout);
void skip_rest(FILE *f);
token *skip_to(stream *in, enum token_type type);
int str2hex(char *s);
char *our_basename(char *name);
void *xmalloc(size_t size);
void *xrealloc(void *p, size_t size);
char *xstrdup(char *s);

#endif /* util.h */
