#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <getopt.h>
#include <regex.h>
#include <map.h>
#include "ranges.h"
#include "util.h"

#define NNTPHOST "news.tuwien.ac.at"
#define DEFAULTEDITOR "vi"
#define DECODER "uudeview -f -i -n -q"
#define BINHEX "bhdeview"
#define BUFSIZE 8192
#define RCNAME ".decoderc"

struct file {
    char *tag;
    char *comment;
    int npart;
    long *msgid;
};

#define MAX_PATTERNS 3

char *spattern[MAX_PATTERNS] = {
    "(.*) - (.*[^ ]) *[[(]([0-9]*)/([0-9]*)[])]", /* "c - n ()" */
    "- (.*[^ ]) *[[(]([0-9]*)/([0-9]*)[])] *(.*)", /* "- n () c" mac groups */
    "(.*)()[[(]([0-9]*)/([0-9]*)[])]" /* "n ()" desperate */
};

regex_t pattern[MAX_PATTERNS]; 

int pat_key[MAX_PATTERNS] =     { 2, 1, 1 };
int pat_comment[MAX_PATTERNS] = { 1, 4, 2 };
int pat_part[MAX_PATTERNS] =    { 3, 2, 3 };
int pat_npart[MAX_PATTERNS] =   { 4, 3, 4 };

char *prg;
char *nntp_response;
char *decoder = DECODER;
struct range *rcmap;

FILE *conin, *conout, *dfile;



#include "config.h"

char version_string[] = 
PACKAGE " " VERSION "\n\
Copyright (C) 1997 Dieter Baron, Thomas Klausner\n"
PACKAGE " comes with ABSOLUTELY NO WARRANTY, to the extent permitted by law.\n\
You may redistribute copies of\n"
PACKAGE " under the terms of the GNU General Public License.\n\
For more information about these matters, see the files named COPYING.\n";

char usage_string[] = "\
Usage: %s [-m] group ...\n";

char help_string[] = "\
\n\
  -h, --help            display this help message\n\
  -V, --version         display version number\n\
\n\
  -m, --mac             call hexbin decoder\n\
\n\
Report bugs to <cg-bugs@giga.or.at>.\n";

#define OPTIONS	"hVm"

struct option options[] = {
    { "help",      0, 0, 'h' },
    { "version",   0, 0, 'V' },
    { "mac",       0, 0, 'm' },
    { NULL,        0, 0, 0   }
};


/* extern */
int parserc (char *gname, FILE *rc, long lo, long hi, long no_arts);
int sopen(char *host, char *service);

/* intern */
long complete (map *parts, long no_file, struct file **todec);
long *choose (struct file **todec, long no_complete, char *group);
int parse(map *parts, FILE *f);
char *extract(char *s, regmatch_t m);
int decode(struct file *value);
int nntp_resp(void);
int nntp_put(char *fmt, ...);
int writerc(char *group);
int readrc(char *group, long lower, long upper, long no_art);
void writegrouptorc (FILE *copy, char *compstr);



int
main(int argc, char **argv)
{
    map *parts;
    int err, fd, verbose, i, c;
    long no_art, lower, upper, no_complete, *toget, gute, j, no_file;
    char b[BUFSIZE];
    struct file **todec;

    prg = argv[0];

    opterr = 0;
    while ((c=getopt_long(argc, argv, OPTIONS, options, 0)) != EOF) {
	switch (c) {
	case 'm':
	    decoder = BINHEX;
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

    dfile = fopen(".debug", "w");

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

    /* talk to server */
    if ((fd=sopen(NNTPHOST, "nntp")) == -1)
	return -1;
    
    conin = fdopen(fd, "r");
    conout = fdopen(fd, "w");

    if (nntp_resp() != 200) {
	fprintf(stderr, "%s: server %s says: %s\n", prg, NNTPHOST,
		nntp_response);
	exit(3);
    }

    for (i=optind;i<argc;i++) {
	nntp_put("group %s", argv[i]);

	if (nntp_resp() != 211) {
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

	readrc(argv[i], lower, upper, no_art);
	
	nntp_put("xover %ld-%ld", lower, upper);

	if (nntp_resp() != 224) {
	    fprintf(stderr, "%s: xover for group %s failed: %s\n", prg,
		    argv[i], nntp_response);
	    continue;
	}
    
	if ((parts=map_new(no_art*2)) == NULL) {
	    fprintf(stderr, "%s: can't create part map\n", prg);
	    exit(1);
	}
	
	no_file=parse(parts, conin);

	if ((todec=(struct file **)malloc(sizeof(struct file *)*no_file))
	    == NULL) {
	    fprintf(stderr, "%s: malloc failure\n", prg);
	    exit(1);
	}

	no_complete=complete(parts, no_file, todec);

	map_free(parts, 0);

	if (no_complete == 0) {
	    printf("%s: no new files found in %s\n", prg, argv[i]);
	    continue;
	}

	toget=choose(todec, no_complete, argv[i]);
	
	gute=0;
	
	for (j=0; toget[j]!=-1;j++)
	    gute+=decode(todec[toget[j]]);
	no_file = j;

	for (j=0; j<no_complete; j++) {
	    free(todec[j]->comment);
	    free(todec[j]->tag);
	    free(todec[j]->msgid);
	    free(todec[j]);
	}	    
	free(toget);
	free(todec);

	writerc(argv[i]);
	
	if (verbose)
	    printf("%s: %ld found, %ld chosen, %ld decoded\n",
		   argv[i], no_complete, no_file, gute);
    }

    exit(0);
}



int
readrc(char *group, long lower, long upper, long no_art)
{
    FILE *rc;

    if ((rc=fopen(RCNAME, "r")) == NULL) {
	if (errno != ENOENT)
	    fprintf(stderr, "%s: couldn't open %s for reading\n", prg,
		    RCNAME);
	rcmap = range_init(lower, upper, no_art);
	return 0;
    }

    rcmap = NULL;
    
    parserc(group, rc, lower, upper, no_art);

    if (rcmap == NULL) {
	rcmap = range_init(lower, upper, no_art);
    }
    
    fclose(rc);
    return(0);
}




int
writerc(char *group)
{
    FILE *rc, *copy;
    int bool, found;
    char *temp, b[BUFSIZE], *compstr;

    if ((rc=fopen(RCNAME, "r")) == NULL) {
	if (errno != ENOENT)
	    fprintf(stderr, "%s: couldn't open %s for reading\n", prg,
		    RCNAME);
    }

    temp=(char *)xmalloc(strlen(RCNAME)+11);
    sprintf(temp, "%s.%d", RCNAME, (int)getpid());
    
    if ((copy=fopen(temp, "w")) == NULL) {
	fprintf(stderr, "%s: couldn't open %s for writing\n", prg,
		temp);
	if (rc)
	    fclose(rc);
	return 1;
    }

    compstr=xmalloc(strlen(group)+2);
    sprintf(compstr, "%s:", group);

    found=0;
    if (rc)
	while(!ferror(rc) && fgets(b, BUFSIZE, rc) != NULL) {
	    /* is this the group that gets rewritten ? */
	    found+=bool=!strncmp(compstr, b, strlen(compstr));
	
	    while (b[strlen(b)-1] != '\n') {
		if (!bool)
		    fputs(b, copy);
		if (fgets (b, BUFSIZE, rc) != NULL)
		    break;
	    }
	    if (!bool)
		fputs(b, copy);
	    
	    if (found == 1) {
		writegrouptorc(copy, compstr);
	    }
	    
	    if (ferror(copy)) {
		fprintf(stderr, "%s: couldn't write to %s: %s\n",
			prg, temp, strerror(errno));
		fclose(copy);
		fclose(rc);
		free(temp);
		free(compstr);
		unlink(temp);
		return(-1);
	    }
	}

    if (!found)
	writegrouptorc(copy, compstr);
    if (ferror(copy)) {
	fprintf(stderr, "%s: couldn't write to %s: %s\n",
		prg, temp, strerror(errno));
	fclose(copy);
	if (rc)
	    fclose(rc);
	free(temp);
	free(compstr);
	unlink(temp);
	return(-1);
    }
    
    free(compstr);
    
    if (rc && ferror(rc)) {
	fprintf(stderr, "%s: couldn't read from %s: %s\n",
		prg, RCNAME, strerror(errno));
	fclose(copy);
	fclose(rc);
	unlink(temp);
	free(temp);
	return(-1);
    }

    if (rc)
	fclose(rc);
    fclose(copy);

    if (rename(temp, RCNAME) != 0) {
	fprintf(stderr, "%s: couldn't rename %s to %s: %s\n",
		prg, temp, RCNAME, strerror(errno));
	free(temp);
	return -1;
    }
    
    free(temp);

    range_free(rcmap);
    
    return(0);
}



void
writegrouptorc (FILE *copy, char *compstr)
{
    int lower, upper;
    
    fprintf(copy, "%s ", compstr);
    
    lower=upper=0;
    while (range_get(rcmap, &lower, &upper, 1) == 0) {
	if (lower==upper) {
	    fprintf(copy, "%s%d", (lower == 1) ? "" : ",",
		    lower);
	}
	else
	    fprintf(copy, "%s%d-%d", (lower == 1) ? "" : ",",
		    lower, upper);
    }
    putc('\n', copy);
}



int compfile (const void *a, const void *b);

long
complete (map *parts, long no_file, struct file **todec)
{
    long i,j;
    int old;
    map_iter *iterate;
    char *key;
    struct file *value;
    
    if ((iterate=map_start(parts)) == NULL) {
	fprintf(stderr, "%s: can't iterate part map\n", prg);
	exit(1);
    }

    for (i=0; map_next(iterate, (void *)&key, (void *)&value) == 0; ) {
	old=1;
	for (j=0; j<value->npart; j++) {
	    if (value->msgid[j] == -1)
		break;
	    else
		if (range_isin(rcmap, value->msgid[j]) == 0)
		    old=0;
	}
	if ((j == value->npart) && (old == 0)) {
	    todec[i++]=value;
	    for (j=0; j<value->npart; j++) {
		range_set(rcmap, value->msgid[j]);
	    }
	}
	else {
	    free(value->comment);
	    free(value->tag);
	    free(value->msgid);
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
	/* XXX Filesize */
	fprintf(temp,"%5ld %.80s --- %s\n", i+1, todec[i]->tag,
		todec[i]->comment);
    }

    fclose(temp);

    editor=getenv("VISUAL");
    if (editor == NULL)
	editor=getenv("EDITOR");
    if (editor == NULL)
	editor=DEFAULTEDITOR;

    sprintf(b,"%s %s",editor, fname);
    system(b);

    if ((temp=fopen(fname,"r")) == NULL) {
	fprintf(stderr, "%s: couldn't open %s for reading: %s\n",
		prg, fname, strerror(errno));
	return NULL;
    }
    
    i=0;

    chosen=(long *)malloc(sizeof(long)*(no_complete+1));
    
    while (fgets(b, BUFSIZE, temp) != NULL) {
	sscanf(b,"%ld",&j);
	if (j != 0)
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
    int npart, part, i, end, l;
    long msgid, no_file;

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
	msgid = strtol(s, NULL, 10);

	subj = strtok(NULL, "\t");

	/* the rest is egal */

	for (i=0; i<MAX_PATTERNS; i++) {
	    if (regexec(&pattern[i], subj, 6, match, 0) == 0)
		break;
	}
	
	if (i == MAX_PATTERNS) {
	    /* DEBUG */	fprintf(dfile, "%s\n", subj);
	    continue;
	}
	    
	key = extract(subj, match[pat_key[i]]);
	comment = extract(subj, match[pat_comment[i]]);
	s = extract(subj, match[pat_part[i]]);
	part = atoi(s);
	free(s);
	s = extract(subj, match[pat_npart[i]]);
	npart = atoi(s);
	free(s);
	
	if (part == 0) {
	    /* XXX save info */
	    free(key);
	    free(comment);
	    continue;
	}

	if (npart == 0) {
	    /* DEBUG */ fprintf(dfile,"%s: ignored: number of parts zero\n", subj);
	    free(key);
	    free(comment);
	    continue;
	}

	sprintf(b, "%s_%d", key, npart);
	valp = (struct file **)map_insert(parts, b);
	if (*valp == NULL) {
	    no_file++;
	    if ((val=(struct file *)malloc(sizeof(struct file))) == NULL) {
		fprintf(stderr, "%s: malloc failure\n", prg);
		exit(1);
	    }
	    val->tag = key;
	    val->comment = comment;
	    val->npart = npart;
	    if (((val->msgid=(long *)malloc(sizeof(long)*npart))
		 == NULL)) {
		fprintf(stderr, "%s: malloc failure\n", prg);
		exit(1);
	    }
	    for (i=0; i<npart; i++)
		val->msgid[i] = -1;

	    *valp = val;
	}
	else {
	    val = *valp;
	    free(key);
	    free(comment);
	}

	if (val->msgid[part-1] != -1 ) {
	    /* DEBUG */ fprintf(dfile, "%s: ignored: duplicate part %d\n",
		   val->tag, part);
	    continue;
	}

	val->msgid[part-1] = msgid;

    }
    
    return no_file;
}



char *
extract(char *s, regmatch_t m)
{
    char *t;

    if ((t=(char *)malloc(m.rm_eo-m.rm_so+1)) == NULL) {
	fprintf(stderr, "%s: malloc failure\n", prg);
	exit(1);
    }

    strncpy(t, s+m.rm_so, m.rm_eo-m.rm_so);
    t[m.rm_eo-m.rm_so] = '\0';

    return t;
}



int
decode(struct file *val)
{
    FILE *f;
    char b[BUFSIZE], cmd[BUFSIZE*2], **fname, *s;
    int i, state, skip, l;

    printf("decoding `%s'\n", val->tag);

    strcpy(cmd, decoder);
    s = cmd+strlen(cmd);
    if ((fname=(char **)malloc(sizeof(char *)*val->npart)) == NULL) {
	fprintf(stderr, "%s: malloc failure\n", prg);
	exit(1);
    }

    for (i=0; i<val->npart; i++) {
	nntp_put("article %ld", val->msgid[i]);
	if (nntp_resp() != 220) {
	    fprintf(stderr, "%s: article %ld failed: %s\n",
		    prg, val->msgid[i], nntp_response);
	    return 0;
	}

	sprintf(b, ".decode-%ld", val->msgid[i]);
	if ((f=fopen(b, "w")) == NULL) {
	    fprintf(stderr, "%s: can't create %s: %s\n",
		    prg, b, strerror(errno));
	    return 0;
	}
	fname[i] = strdup(b);
	sprintf(s, " %s", b);
	s += strlen(s);
	
	state = 0;
	while (fgets(b, BUFSIZE, conin)) {
	    skip = 0;
	    l=strlen(b);
	    
	    if (b[l-1] == '\n')
		b[--l] = '\0';
	    if (b[l-1] == '\r')
		b[--l] = '\0';

	    if (b[0] == '.') {
		if (b[1] == '\0')
		    break;
		else if (b[1] == '.')
		    skip = 1;
	    }

	    if (state == 0) {
		if (strncasecmp(b, "Subject: ", 8) == 0) {
		    sprintf(b, "Subject: %s (%d/%d)",
			    val->tag, i+1, val->npart);
		}
		else if (b[0] == '\n')
		    state = 1;
	    }

	    fputs(b+skip, f);
	    putc('\n', f);
	}

	fclose(f);
    }

    system(cmd);

    for (i=0; i<val->npart; i++) {
	unlink(fname[i]);
	free(fname[i]);
    }
    free(fname);

    return 1;
}



int
nntp_put(char *fmt, ...)
{
	char buf[BUFSIZE];
	va_list argp;

	if (conout == NULL)
	    return -1;

	va_start(argp, fmt);
	vsprintf(buf, fmt, argp);
	va_end(argp);

	fprintf(conout, "%s\r\n", buf);

	if (fflush(conout) || ferror(conout))
	    return -1;

	return 0;
}	



int
nntp_resp(void)
{
    char line[BUFSIZE];
    int resp;
    
    if (conin == NULL)
	return -1;

    clearerr(conin);
    if (fgets(line, BUFSIZE, conin) == NULL)
	return -1;

    resp = atoi(line);
    free (nntp_response);
    nntp_response = strdup(line);

    return resp;
}
