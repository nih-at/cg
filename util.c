#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <pwd.h>

#include "util.h"
#include "stream.h"

#define HOUSENUMBER 102400

#define DEBUG_FILE_NAME ".debug"

static FILE *debug_file;
static int do_debug_file, do_debug_stdout;



void *
xmalloc(size_t size)
{
    void *p;
    
    if ((p=malloc(size)) == NULL) {
	fprintf(stderr, "%s: malloc failure (size=%ld)\n", prg, (long)size);
	exit(1);
    }

    return p;
}



void *
xrealloc(void *p, size_t size)
{
    if ((p=realloc(p, size)) == NULL) {
	fprintf(stderr, "%s: realloc failure (size=%ld)\n", prg, (long)size);
	exit(1);
    }

    return p;
}



char *
expand(char *path)
{
    char *home, *s, *p;
    struct passwd *pw;
	    
    if (path[0] != '~')
	return strdup(path);

    if (path[1] == '/' || path[1] == '\0') {
	s = path+1;

	if ((home=getenv("HOME")) == NULL) {
	    if ((pw=getpwuid(getuid())) == NULL
		|| (home=pw->pw_dir) == NULL)
		return NULL;
	}
    }
    else {
	if ((s=strchr(path, '/')))
	    *s = '\0';
	if ((pw=getpwnam(path+1)) == NULL
	    || (home=pw->pw_dir) == NULL) {
	    if (s)
		*s = '/';
	    return NULL;
	}
	if (s)
	    *s = '/';
	else
	    s = path+strlen(path);
    }

    if ((p=(char *)malloc(strlen(home)+strlen(s)+1)) == NULL)
	return NULL;

    sprintf(p, "%s%s", home, s);

    return p;
}



FILE *
fopen_uniq(char **fnp)
{
    char *s, b[8192];
    int fd, i;
    FILE *f;

    s=*fnp;
    if (s == NULL) {
	s = "UNKNOWN";
	strcpy(b, "UNKNOWN.001");
	i = 1;
    }
    else {
	strcpy(b, s);
	i = 0;
    }
    
    for (;;) {
	if ((fd=open(b, O_WRONLY | O_CREAT | O_EXCL, 0666)) < 0) {
	    if (errno != EEXIST) {
		return NULL;
	    }
	}
	else {
	    if ((f=fdopen(fd, "wb")) == NULL) {
		close(fd);
		return NULL;
	    }
	    *fnp = strdup(b);
	    return f;
	}

	sprintf(b, (*fnp) ? "%s.%d" : "%s.%03d", s, ++i);
    }
}



char *
getline(FILE *f)
{
    static char *b;
    static int bsize;
    int len;
    
    if (bsize == 0) {
	bsize = BUFSIZE;
	b = (char *)xmalloc(bsize);
    }

    if (fgets(b, bsize, f) == NULL) {
	return NULL;
    }

    len = strlen(b);
    
    while (b[len-1] != '\n') {
	/* line too long */
	bsize += BUFSIZE;
	if (bsize >= HOUSENUMBER) {
	    /* prerror(errline, "line longer than %d", HOUSENUMBER); */
	    bsize -= BUFSIZE;
	    return b;
	}
	b = xrealloc(b, bsize);
	if (fgets(b+bsize-BUFSIZE, BUFSIZE, f) == NULL) {
	    return NULL;
	}
	len += strlen(b+bsize-BUFSIZE);
    }

    if (b[len-2] == '\r')
	b[len-2] = '\0';
    else
	b[len-1] = '\0';


    if (b[0] == '.') {
	if (b[1] == '\0')
	    return NULL;
	/* errlineno++; */
	return b+1;
    }

    /* errlineno++; */
    return b;    
}



void
skip_rest(FILE *f)
{
    while (getline(f) != NULL)
	;
}



char *
xstrdup(char *s)
{
    char *t;

    if (s == NULL)
	return NULL;

    t = xmalloc(strlen(s)+1);
    strcpy(t, s);

    return t;
}



token *
skip_to(stream *in, enum token_type type)
{
    token *t;

    while ((t=stream_get(in))->type != TOK_EOF && t->type != type)
	;

    return t;
}



void
copy_stream(stream *in, out_state *out)
{
    token *t;

    while ((t=stream_get(in))->type != TOK_EOF)
	output(out, t);
}



void
debug(out_state *out, char *fmt, ...)
{
    token t;
    char *s;
    va_list argp;

    va_start(argp, fmt);
    vasprintf(&s, fmt, argp);
    va_end(argp);

    output(out, token_set(&t, TOK_DEBUG, s));

    free(s);
}



int
append_file(char *t, char *s, char *sep)
{
    FILE *fin, *fout;
    char b[8192];
    int n, m, i;

    if ((fin=fopen(s, "rb")) == NULL)
	return EOF;
    if ((fout=fopen(t, "ab")) == NULL) {
	fclose(fin);
	return EOF;
    }

    if (fputs(sep, fout) != 0) {
	fclose(fin);
	fclose(fout);
	return EOF;
    }

    while ((n=fread(b, 1, 8192, fin)) > 0)
	for (i=0; n>0; n-=m,i+=m)
	    if ((m=fwrite(b+i, 1, n, fout)) <= 0) {
		fclose(fin);
		fclose(fout);
		return EOF;
	    }

    if (ferror(fin)) {
	fclose(fin);
	fclose(fout);
	return EOF;
    }
    
    fclose(fin);
    return fclose(fout);
}



char *
our_basename(char *name)
{
    char *p;

    if (name == NULL)
	return NULL;

    if ((p=strrchr(name, '/')))
	return p+1;
    if ((p=strrchr(name, '\\')))
	return p+1;
    return name;
}



void
prdebug(int mask, char *fmt, ...)
{
    char *s;
    va_list argp;

    if (!(mask & do_debug_file) && !(mask & do_debug_stdout))
	return;

    va_start(argp, fmt);
    vasprintf(&s, fmt, argp);
    va_end(argp);

    if (mask & do_debug_file)
	fprintf(debug_file, "%s\n", s);
    if (mask & do_debug_stdout)
	puts(s);

    free(s);    
}



void
prdebug_init(int do_file, int do_stdout)
{
    if (do_file) {
	if ((debug_file=fopen(DEBUG_FILE_NAME, "w")) == NULL)
	    do_file = INT_MAX;
    }
    do_debug_file = do_file;
    do_debug_stdout = do_stdout;
}
