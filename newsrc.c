/*
  $NiH: newsrc.c,v 1.7 2002/04/10 16:23:29 wiz Exp $

  newsrc.c -- .newsrc handling
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
#include <errno.h>
#include <unistd.h>

#include "util.h"
#include "ranges.h"
#include "globals.h"

extern char *newsrc;

extern int parserc (char *gname, FILE *rc, long lo, long hi, long no_arts);
static void writegrouptorc (FILE *copy, char *compstr);



int
readrc(char *group, long lower, long upper, long no_art)
{
    FILE *rc;

    if ((rc=fopen(newsrc, "r")) == NULL) {
	if (errno != ENOENT)
	    fprintf(stderr, "%s: couldn't open %s for reading\n", prg,
		    newsrc);
	rcmap = range_init(lower, upper, no_art);
	return 0;
    }

    rcmap = NULL;

    parserc(group, rc, lower, upper, no_art);

    if (rcmap == NULL)
	rcmap = range_init(lower, upper, no_art);

    fclose(rc);
    return(0);
}




int
writerc(char *group)
{
    static int modified = 0;

    FILE *rc, *copy;
    int bool, found;
    char *temp, b[BUFSIZE], *compstr;

    if (modified == 0) {
	sprintf(b, "%s~", newsrc);
	remove(b);
	if (link(newsrc, b) < 0 && errno != ENOENT) {
	    fprintf(stderr, "%s: can't backup newsrc `%s' (left unchanged): "
		    "%s\n",
		    prg, newsrc, strerror(errno));
	    return 1;
	}
	modified = 1;
    }

    if ((rc=fopen(newsrc, "r")) == NULL) {
	if (errno != ENOENT)
	    fprintf(stderr, "%s: couldn't open %s for reading\n", prg,
		    newsrc);
    }

    temp=(char *)xmalloc(strlen(newsrc)+11);
    sprintf(temp, "%s.%d", newsrc, (int)getpid());

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
		if (fgets (b, BUFSIZE, rc) == NULL)
		    break;
	    }
	    if (!bool)
		fputs(b, copy);

	    if ((found == 1) && bool) {
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
		prg, newsrc, strerror(errno));
	fclose(copy);
	fclose(rc);
	unlink(temp);
	free(temp);
	return(-1);
    }

    if (rc)
	fclose(rc);
    fclose(copy);

    if (rename(temp, newsrc) != 0) {
	fprintf(stderr, "%s: couldn't rename %s to %s: %s\n",
		prg, temp, newsrc, strerror(errno));
	free(temp);
	return -1;
    }

    free(temp);

    range_free(rcmap);

    return(0);
}



static void
writegrouptorc (FILE *copy, char *compstr)
{
    int lower, upper, first;

    fprintf(copy, "%s ", compstr);

    lower = upper = first = 0;

    while (range_get(rcmap, &lower, &upper, 1) == 0) {
	if (lower==upper) {
	    fprintf(copy, "%s%d", (first == 0) ? "" : ",",
		    lower);
	}
	else
	    fprintf(copy, "%s%d-%d", (first == 0) ? "" : ",",
		    lower, upper);

	if (first == 0)
	    first = 1;
    }
    putc('\n', copy);
}

