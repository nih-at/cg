/*
  $NiH: checkgroup.c,v 1.44 2002/04/17 17:46:28 dillo Exp $

  checkgroup.c -- main program
  Copyright (C) 2002 Dieter Baron and Thomas Klausner

  This file is part of cg, a program to assemble and decode binary Usenet
  postings.  The authors can be contacted at <nih@giga.or.at>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <getopt.h>
#include <regex.h>
#include <map.h>

#include "checkgroup.h"
#include "decode.h"
#include "header.h"
#include "mime.h"
#include "newsrc.h"
#include "ranges.h"
#include "sockets.h"
#include "stream.h"
#include "stream_types.h"
#include "util.h"

#define NNTPHOSTFILE "/etc/nntpserver"
#define DEFAULTEDITOR "vi"
#define BUFSIZE 8192


#define MAX_PATTERNS 6

char *spattern[MAX_PATTERNS] = {  /* ! ( ! */
    "(.*) - (.*[^ ]) *[[(]([0-9]*)/([0-9]*)[])]",   /* "c - n ()" */
    "(([^-]|-+[^ -])+)-+ (.*[^ ]) *[[(]([0-9]*)[/\\]([0-9]*)[])]", /* c - n () */
    "- (.*[^ ]) *[[(]([0-9]*)/([0-9]*)[])] *(.*)",  /* "- n () c" mac groups */
    "(.*)()[[(]([0-9]*)/([0-9]*)[])]",              /* "n ()" desperate */
    "(.*)[[( ]([0-9]+) of ([0-9]+)[]) ](.*)",       /* "c[x of y]n" */
    "(.*)  *([^ ]*\\.[a-z0-9][a-z0-9][a-z0-9]) *$"  /* "c n.ext" */
};

regex_t pattern[MAX_PATTERNS];

int pat_key[MAX_PATTERNS] =     { 2,  3, 1, 1, 4 /* 2 */, 2 };
int pat_comment[MAX_PATTERNS] = { 1,  1, 4, 2, 1 /* 4 */, 1 };
int pat_part[MAX_PATTERNS] =    { 3,  4, 2, 3, 2 /* 1 */, 0 };
int pat_npart[MAX_PATTERNS] =   { 4,  5, 3, 4, 3 /* 3 */, 0 };

char *prg;
char *nntp_response, *nntp_group;
char *nntp_host, *nntp_user, *nntp_pass;
struct range *rcmap;

char *newsrc;
#define DEFAULT_NEWSRC  "~/.newsrc"
int mark_complete;

FILE *conin, *conout;

volatile int save_and_quit;



#include "config.h"

char version_string[] =
PACKAGE " " VERSION "\n\
Copyright (C) 1997, 2001, 2002 Dieter Baron, Thomas Klausner\n"
PACKAGE " comes with ABSOLUTELY NO WARRANTY, to the extent permitted by law.\n\
You may redistribute copies of\n"
PACKAGE " under the terms of the GNU General Public License.\n\
For more information about these matters, see the files named COPYING.\n";

char usage_string[] = "\
Usage: %s [-chmV] [-s server] [-n rcfile] [-u user] [-p passwd] group ...\n";

char help_string[] = "\
\n\
  -h, --help            display this help message\n\
  -V, --version         display version number\n\
\n\
  -c, --mark-complete   mark only parts of complete files as read\n\
  -d, --debug           write debugging information to .debug\n\
  -n, --newsrc FILE     use FILE as newsrc file\n\
  -p, --pass PASS       specify password for authentication\n\
  -s, --server SERVER   NNTP server\n\
  -u, --user USER       specify user for authentication\n\
\n\
Report bugs to <nih@giga.or.at>.\n";

#define OPTIONS	"hVn:cdu:p:s:"

struct option options[] = {
    { "debug",         0, 0, 'd' },
    { "help",          0, 0, 'h' },
    { "mark-complete", 0, 0, 'c' },
    { "newsrc",        1, 0, 'n' },
    { "pass",          1, 0, 'p' },
    { "server",        1, 0, 's' },
    { "user",          1, 0, 'u' },
    { "version",       0, 0, 'V' },
    { NULL,            0, 0, 0   }
};



long complete(map *parts, long no_file, struct file **todec);
long *choose(struct file **todec, long no_complete, char *group);
int parse(map *parts, FILE *f);
char *extract(char *s, regmatch_t m);
int do_decode(struct file *value, out_state *out);
int nntp_resp(void);
int nntp_put(char *fmt, ...);
void writegrouptorc(FILE *copy, char *compstr);



void
sighandle(int sigtype)
{
    save_and_quit = 1;

    return;
}



int
main(int argc, char **argv)
{
    map *parts;
    int debug, err, fd, verbose, i, c, k;
    long no_art, lower, upper, no_complete, *toget, ndecoded, j, no_file;
    char b[BUFSIZE];
    struct file **todec;
    FILE *fp;
    out_state *out;

    prg = argv[0];

    nntp_group = NULL;
    newsrc = DEFAULT_NEWSRC;
    debug = mark_complete = 0;

    mime_init();
    header_init();

    nntp_host = nntp_user = nntp_pass = NULL;
    verbose = 1;

    opterr = 0;
    while ((c=getopt_long(argc, argv, OPTIONS, options, 0)) != EOF) {
	switch (c) {
	case 'c':
	    mark_complete = 1;
	    break;
	case 'd':
	    debug = 1;
	    break;
	case 'n':
	    newsrc = optarg;
	    break;
	case 'p':
	    nntp_pass = optarg;
	    break;
	case 's':
	    nntp_host = optarg;
	    break;
	case 'u':
	    nntp_user = optarg;
	    break;

	case 'h':
	    printf(usage_string, prg);
	    fputs(help_string, stdout);
	    exit(0);
	case 'V':
	    fputs(version_string, stdout);
	    exit(0);
	default:
	    fprintf(stderr, usage_string, prg);
	    exit(1);
	}
    }

    if (optind == argc) {
	fprintf(stderr, usage_string, prg);
	exit(1);
    }

    prdebug_init(debug ? (DEBUG_ALL^(DEBUG_SUBJ|DEBUG_DEBUG)) : 0,
		 DEBUG_ALL^(DEBUG_LINE|DEBUG_SUBJ|DEBUG_PART|DEBUG_TOK));

    newsrc = expand(newsrc);

    for (i=0;i<MAX_PATTERNS;i++) {
	if ((err=regcomp(&pattern[i], spattern[i], REG_EXTENDED|REG_ICASE))
	    != 0) {
	    regerror(err, &pattern[i], b, BUFSIZE);
	    fprintf(stderr, "%s: can't compile regex pattern %d: %s\n", prg,
		    i, b);
	    exit(1);
	}
    }

    err = 0;

    if (nntp_host == NULL) {
	if ((nntp_host=getenv("NNTPSERVER")) == NULL) {
	    if ((fp=fopen(NNTPHOSTFILE, "r")) == NULL) {
		fprintf(stderr, "%s: NNTPSERVER not set, and can't open "
			"%s: %s\n", prg, NNTPHOSTFILE, strerror(errno));
		exit(7);
	    }
	    if (fgets(b, BUFSIZE, fp) == NULL) {
		fprintf(stderr, "%s: can't read newsserver from %s: %s\n",
			prg, NNTPHOSTFILE, strerror(errno));
		exit(7);
	    }
	    fclose(fp);
	    if (b[strlen(b)-1]=='\n')
		b[strlen(b)-1]='\0';
	    if ((nntp_host=strdup(b)) == NULL) {
		fprintf(stderr, "%s: can't strdup nntp_host from `%s': "
			"shoot me\n", prg, b);
		exit(77);
	    }
	}
    }

    /* talk to server */
    if ((fd=sopen(nntp_host, "nntp", AF_UNSPEC)) == -1)
	return -1;

    conin = fdopen(fd, "r");
    conout = fdopen(fd, "w");

    if ((nntp_resp() & ~1) != 200) {
	fprintf(stderr, "%s: server %s says: %s\n", prg, nntp_host,
		nntp_response);
	exit(3);
    }

    nntp_put("mode reader");

    for (i=optind;i<argc;i++) {
	if (nntp_put("group %s", argv[i]) != 211) {
	    fprintf(stderr, "%s: group %s failed: %s\n", prg, argv[i],
		    nntp_response);
	    continue;
	}

	if (sscanf(nntp_response, "%*d %ld %ld %ld", &no_art, &lower, &upper)
	    != 3) {
	    fprintf(stderr, "%s: can't parse group %s reply: %s\n", prg,
		    argv[i], nntp_response);
	    continue;
	}

	if (no_art == 0) {
	    printf("%s: no new files found in %s\n", prg, argv[i]);
	    continue;
	}

	if (lower > upper || no_art < 0 || lower < 0 || upper < 0) {
	    fprintf(stderr, "%s: invalid response from newsserver"
		    " for group %s: %s\n", prg, argv[i], nntp_response);
	    continue;
	}

	readrc(argv[i], lower, upper, no_art);

	if (nntp_put("xover %ld-%ld", lower, upper) != 224) {
	    fprintf(stderr, "%s: xover for group %s failed: %s\n", prg,
		    argv[i], nntp_response);
	    continue;
	}

	if ((parts=map_new(no_art*2)) == NULL) {
	    fprintf(stderr, "%s: can't create part map\n", prg);
	    exit(1);
	}

	no_file = parse(parts, conin);

	todec = (struct file **)xmalloc(sizeof(struct file *)*no_file);

	no_complete = complete(parts, no_file, todec);

	map_free(parts, 0);

	if (no_complete == 0) {
	    printf("%s: no new files found in %s\n", prg, argv[i]);
	    continue;
	}

	toget = choose(todec, no_complete, argv[i]);
	/* XXX: The lines below are just a patch -- better handling ??*/
	if (toget == NULL) {       /* error handling */
	    for (j=0; j<no_complete; j++) {
		free(todec[j]->comment);
		free(todec[j]->tag);
		free(todec[j]->artno);
		free(todec[j]);
	    }
	    free(todec);
	    continue;
	}

	/* signal handling */
	save_and_quit = 0;
	signal(SIGINT, sighandle);

	ndecoded = 0;
	out = output_new();

	for (j=0; toget[j]!=-1; j++) {
	    if (save_and_quit || (do_decode(todec[toget[j]], out) <= 0)) {
		for (k=0; k<=todec[toget[j]]->npart; k++)
		    if (todec[toget[j]]->artno[k] != -1)
			range_clear(rcmap, todec[toget[j]]->artno[k]);
	    }
	    else
		ndecoded++;
	}

	output_free(out);

	no_file = j;

	for (j=0; j<no_complete; j++) {
	    free(todec[j]->comment);
	    free(todec[j]->tag);
	    free(todec[j]->artno);
	    free(todec[j]);
	}
	free(toget);
	free(todec);

	writerc(argv[i]);

	if (verbose)
	    printf("%s: %ld found, %ld chosen, %ld decoded\n",
		   argv[i], no_complete, no_file, ndecoded);

	if (save_and_quit) {
	    printf("quitting after interrupt signal\n");
	    break;
	}
    }

    exit(0);
}



int compfile (const void *a, const void *b);

long
complete (map *parts, long no_file, struct file **todec)
{
    long i,j;
    map_iter *iterate;
    char *key;
    struct file *value;

    if ((iterate=map_start(parts)) == NULL) {
	fprintf(stderr, "%s: can't iterate part map\n", prg);
	exit(1);
    }

    for (i=0; map_next(iterate, (void *)&key, (void *)&value) == 0; ) {
	if (value->new) {
	    for (j=1; j<=value->npart; j++) {
		if (value->artno[j] == -1)
		    break;
	    }
	}
	if ((j == value->npart+1) && value->new) {
	    todec[i++]=value;
	    if (mark_complete) {
		for (j=0; j<value->npart; j++)
		    range_set(rcmap, value->artno[j]);
	    }
	}
	else {
	    free(value->comment);
	    free(value->tag);
	    free(value->artno);
	    free(value);
	}
    }
    map_stop(iterate);

    qsort(todec, i, sizeof(struct file *), compfile);

    return i;
}



long *
choose (struct file **todec, long no_complete, char *group)
{
    char b[BUFSIZE], fname[BUFSIZE], *editor;
    long i, j;
    long *chosen;
    FILE *temp;

    sprintf(fname, "00-%s-%d", group, (int)getpid());

    if ((temp=fopen(fname, "w")) == NULL) {
	fprintf(stderr,"%s: tempfile failure: Und Tschuess!\n", prg);
	exit(2);
    }

    for (i=0; i<no_complete; i++) {
	fprintf(temp,"%5ld [%5ldk] %.80s --- %s\n", i+1,
		(todec[i]->size+1023)/1024, todec[i]->tag,
		todec[i]->comment);
    }

    fclose(temp);

    editor=getenv("VISUAL");
    if (editor == NULL)
	editor=getenv("EDITOR");
    if (editor == NULL)
	editor=DEFAULTEDITOR;

    sprintf(b,"%s %s", editor, fname);
    system(b);

    if ((temp=fopen(fname,"r")) == NULL) {
	fprintf(stderr, "%s: couldn't open %s for reading: %s\n",
		prg, fname, strerror(errno));
	return NULL;
    }

    i=0;

    chosen = (long *)xmalloc(sizeof(long)*(no_complete+1));

    while (fgets(b, BUFSIZE, temp) != NULL) {
	j = 0;
	sscanf(b,"%ld",&j);
	if ((j > 0) && (j < no_complete+1))
	    chosen[i++]=j-1;
    }

    fclose(temp);

    chosen[i]=-1;

    return chosen;
}




int
compfile (const void *a, const void *b)
{
    return strcasecmp((*(struct file **)a)->tag, (*(struct file **)b)->tag);
}



int
parse(map *parts, FILE *f)
{
    regmatch_t match[6];
    struct file *val, **valp;
    char *key, *s, *subj, *comment;
    char b[8192];
    int npart, part, i, end, l, lines;
    long artno, no_file, size;

    end = 0;
    no_file = 0;

    while (fgets(b, 8192, f)) {
	l=strlen(b);

	/* normalization */
	if (b[l-1] == '\n')
	    b[--l] = '\0';
	if (b[l-1] == '\r')
	    b[--l] = '\0';

	if (b[0] == '.' && b[1] == '\0') {
	    end = 1;
	    break;
	}

	s = strtok(b, "\t");
	artno = strtol(s, NULL, 10);

	subj = strtok(NULL, "\t");

	/* auth = */  strtok(NULL, "\t");
	/* date = */  strtok(NULL, "\t");
	/* msgid = */ strtok(NULL, "\t");

	s = strtok(NULL, "\t");
	while (s != NULL && s[0] == '<')
	    s = strtok(NULL, "\t"); /* message-ID & references */
	if (s == NULL) {
	    prdebug(DEBUG_XOVR, "xover for article %ld is weird", artno);
	    size = 0;
	}
	else {
	    size = strtol(s, NULL, 10);
	    if ((s=strtok(NULL, "\t")) == NULL) {
		prdebug(DEBUG_XOVR, "xover for article %ld is weird", artno);
		lines = 0;
	    }
	    else {
		lines = strtol(s, NULL, 10);
		/* xref = */ strtok(NULL, "\t");
	    }
	}

	for (i=0; i<MAX_PATTERNS; i++) {
	    if (regexec(&pattern[i], subj, 6, match, 0) == 0)
		break;
	}

	/* didn't match a pattern, judge by file size */
	if (i == MAX_PATTERNS) {
	    /* deep magic */
	    if ((lines > 100) && ((size - 1024)/lines > 60)) {
		key = strdup(subj);
		comment = NULL;
		part = 1;
		npart = 1;
	    }
	    else {
		prdebug(DEBUG_SUBJ, "unrecognized subject: %s", subj);
		continue;
	    }
	}
	else {
	    key = extract(subj, match[pat_key[i]]);
	    comment = extract(subj, match[pat_comment[i]]);
	    if (pat_part[i]) {
		s = extract(subj, match[pat_part[i]]);
		part = atoi(s);
		free(s);
	    }
	    else
		part = 1;
	    if (pat_npart[i]) {
		s = extract(subj, match[pat_npart[i]]);
		npart = atoi(s);
		free(s);
	    }
	    else
		npart = 1;
	}

	if ((npart == 0) || (npart > 10000) || (part > 10000)
	    || (part > npart)) {
	    prdebug(DEBUG_PART, "%s: ignored: part %d of %d",
		    subj, part, npart);
	    free(key);
	    free(comment);
	    if (!mark_complete)
		range_set(rcmap, artno);
	    continue;
	}

	sprintf(b, "%s_%d", key, npart);
	valp = (struct file **)map_insert(parts, b);
	if (*valp == NULL) {
	    no_file++;
	    val = (struct file *)xmalloc(sizeof(struct file));
	    val->tag = key;
	    val->comment = comment;
	    val->npart = npart;
	    val->artno = (long *)xmalloc(sizeof(long)*(npart+1));
	    for (i=0; i<=npart; i++)
		val->artno[i] = -1;
	    val->new=0;
	    val->size = 0;

	    *valp = val;
	}
	else {
	    val = *valp;
	    free(key);
	    free(comment);
	}

	if (val->artno[part] != -1 ) {
	    prdebug(DEBUG_PART, "%s: ignored: duplicate part %d",
		    val->tag, part);
	    if (!mark_complete)
		range_set(rcmap, artno);
	    continue;
	}

	val->artno[part] = artno;

	if (part != 0) {
	    /* previously unseen part */
	    if (!range_isin(rcmap, artno))
		val->new=1;

	    if (strcasestr(subj, "yenc") != NULL)
		val->size += size*98/100;
	    else
		val->size += size*3/4;
	}

	if (!mark_complete)
	    range_set(rcmap, artno);

    }

    return no_file;
}



char *
extract(char *s, regmatch_t m)
{
    char *t;

    t = (char *)xmalloc(m.rm_eo-m.rm_so+1);

    strncpy(t, s+m.rm_so, m.rm_eo-m.rm_so);
    t[m.rm_eo-m.rm_so] = '\0';

    return t;
}



int
do_decode(struct file *val, out_state *out)
{
    int err;
    stream *stm, *st2;
    token t;

    prdebug(DEBUG_PACK, "--> Decoding `%s':", val->tag);

    stm = stream_cat_open(val);
    st2 = stream_article_open(stm);
    err = decode(st2, out);
    stream_close(st2);
    stream_close(stm);

    output(out, token_set(&t, TOK_EOP, NULL));

    if (err <= 0)
	prdebug(DEBUG_ERROR, "no file found");

    return err;
}



int
nntp_put(char *fmt, ...)
{
    int fd, ret, tries, writeerr;
    char buf[BUFSIZE];
    va_list argp;

    ret = tries = writeerr = 0;

    if (conout == NULL)
	return -1;

    va_start(argp, fmt);
    vsprintf(buf, fmt, argp);
    va_end(argp);

    for (;;) {
	fprintf(conout, "%s\r\n", buf);

	if (fflush(conout) || ferror(conout))
	    writeerr = 1;
	else
	    ret = nntp_resp();

	if (writeerr || (ret == 400) || (ret == 503)) {
	    /* connection to server closed -- reconnect */

	    writeerr = 0;
	    if (tries == 0) {
		tries++;
		fclose(conin);
		fclose(conout);

		if ((fd=sopen(nntp_host, "nntp", AF_UNSPEC)) == -1)
		    return -1;

		conin = fdopen(fd, "r");
		conout = fdopen(fd, "w");

		if ((nntp_resp() & ~1) != 200) {
		    fprintf(stderr, "%s: server %s says: %s\n", prg, nntp_host,
			    nntp_response);
		    return -1;
		}

		fprintf(conout, "mode reader\r\n");
		if (fflush(conout) || ferror(conout)) /* XXX: retry? */
		    return -1;

		nntp_resp();

		if (nntp_group != NULL) {
		    fprintf(conout, "%s\r\n", nntp_group);
		    if (fflush(conout) || ferror(conout)) /* XXX: retry? */
			return -1;

		    if (nntp_resp() != 211) {
			fprintf(stderr, "%s: timed out -- can't reconnect",
				prg);
			return -1;
		    }
		}

		fprintf(stderr, "%s: timed out -- but reconnected\n", prg);
	    }
	    else {
		fprintf(stderr, "%s: timed out -- can't reconnect\n", prg);
		return -1;
	    }
	}
	else if (ret == 480) {
	    /* authentication required */

	    if (nntp_user == NULL) {
		/* XXX prompt for user */
		fprintf(stderr,
			"%s: authentication required but no user given\n",
			prg);
		return -1;
	    }

	    fprintf(conout, "authinfo user %s\r\n", nntp_user);
	    if (fflush(conout) || ferror(conout)) {
                /* XXX retry? */
		fprintf(stderr, "%s: write error in authentication: %s\n",
			prg, strerror(errno));
		return -1;
	    }

	    if (nntp_resp() == 381) {
		if (nntp_pass == NULL) {
		    /* XXX prompt for user */
		    fprintf(stderr,
			    "%s: authentication required but no pass given\n",
			    prg);
		    return -1;
		}

		fprintf(conout, "authinfo pass %s\r\n", nntp_pass);
		if (fflush(conout) || ferror(conout)) {
		    /* XXX retry? */
		    fprintf(stderr, "%s: write error in authentication: %s\n",
			    prg, strerror(errno));
		    return -1;
		}
	    }
	    if (nntp_resp() != 281) {
		fprintf(stderr, "%s: authentication as %s failed: %s\n",
			prg, nntp_user, nntp_response);
		return -1;
	    }
	}
	else
	    break;
    }

    if (strncasecmp(buf, "group ", 6) == 0) {
	free(nntp_group);
	nntp_group = strdup(buf);
    }

    return ret;
}



int
nntp_resp(void)
{
    char line[BUFSIZE];
    int resp, l;

    if (conin == NULL)
	return -1;

    clearerr(conin);
    if (fgets(line, BUFSIZE, conin) == NULL)
	return -1;

    resp = atoi(line);
    l = strlen(line);
    if (line[l-2] == '\r')
	line[l-2] = '\0';
    else
	line[l-1] = '\0';
    free (nntp_response);
    nntp_response = strdup(line);

    return resp;
}
