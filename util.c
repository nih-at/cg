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

#define HOUSENUMBER 102400

char errfilename[ERRFILESIZE+1];
int errpartno, errlineno;



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



void
prerror(enum errtype type, char *fmt, ...)
{
    va_list argp;

    fprintf(stderr, "%s: ", prg);

    if (type >= errfile) {
	if (errfilename) {
	    fprintf(stderr, "%s", errfilename);
	    if (type >= errpart) {
		fprintf(stderr, ":%d", errpartno);
		if (type == errline)
		    fprintf(stderr, ":%d", errlineno);
	    }
	    fprintf(stderr, ": ");
	}
    }

    va_start(argp, fmt);
    vfprintf(stderr, fmt, argp);
    va_end(argp);
    fputc('\n', stderr);
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

    len=strlen(b);
    
    while (b[len-1] != '\n') {
	/* line too long */
	bsize += BUFSIZE;
	if (bsize >= HOUSENUMBER) {
	    prerror(errline, "line longer than %d", HOUSENUMBER);
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
	return b+1;
    }
    return b;    
}



void
skip_rest(FILE *f)
{
    while (getline(f) != NULL)
	;
}
