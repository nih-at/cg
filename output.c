/*
  $NiH: output.c,v 1.18 2002/04/10 16:23:30 wiz Exp $

  output.c -- output part of the decoder
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

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>

#include "stream.h"
#include "output.h"
#include "util.h"

static void close_fdesc(out_state *out);
static void rename_fdesc(out_state *out);

#define FDESC_NOTYET	0
#define FDESC_OPEN	1
#define FDESC_ERROR	-1



out_state *
output_new()
{
    out_state *out;

    out = xmalloc(sizeof(*out));

    out->broken = out->infile = out->warned = out->ndata = 0;
    out->do_fdesc = FDESC_NOTYET;
    out->fdescnl = 0;
    out->size = 0;
    out->fdescfilename = NULL;
    out->fout = out->fdesc = NULL;
    out->filename = NULL;

    return out;
}



void
output_close(out_state *out)
{

    if (out->fout)
	fclose(out->fout);
    out->fout = NULL;
}



void
output_free(out_state *out)
{
    if (!out)
	return;

    output_close(out);
    close_fdesc(out);
    free(out->fdescfilename);
    free(out->filename);

    free(out);
}



int
output(out_state *out, token *t)
{
    static char *_foo[] = {
	"EOF", "line", "data", "EOS", "EOH", "EOA", "EOP", "fname", "debug",
	"error"
    };
    static char **tname = _foo+1;
    char *outfilename;
    char *brokenfilename;
    int fd;

    if (t->type == TOK_DATA)
	out->ndata++;
    else {
	if (out->ndata)
	    prdebug(DEBUG_TOK, ">data [%d]", out->ndata);
	out->ndata = 0;
    }

    switch (t->type) {
    case TOK_FNAME:
	prdebug(DEBUG_TOK, ">%s: %s",
		tname[t->type], t->line ? t->line : "(null)");

	if (out->infile) {
	    fprintf(stderr, "ERROR: fname while in file (missing EOF)\n");
	    output_close(out);
	}
	outfilename = our_basename(t->line);
	prdebug(DEBUG_FNAM, "writing `%s'", outfilename);

	if ((out->fout=fopen_uniq(&outfilename)) == NULL) {
	    fprintf(stderr, "SYSERR: cannot create output file `%s': %s\n",
		   t->line, strerror(errno));
	    /* discard description */
	    if (out->do_fdesc == FDESC_OPEN) {
		out->do_fdesc = FDESC_ERROR;
		remove(out->tempdesc);
		out->tempdesc[0] = '\0';
	    }
	}
	else {
	    free(out->filename);
	    out->filename = outfilename;

	    if (out->do_fdesc != FDESC_NOTYET)
		rename_fdesc(out);
	}
	close_fdesc(out);
	out->broken = 0;
	out->size = 0;
	out->infile = 1;
	break;

    case TOK_DEBUG:
	prdebug(DEBUG_TOK, ">debug: %s", t->line);
	prdebug(DEBUG_DEBUG, "%s", t->line);
	break;

    case TOK_LINE:
	if (out->infile) {
	    if (out->fout != NULL) {
		fprintf(out->fout, "%s\n", t->line);
		if (ferror(out->fout)) {
		    fprintf(stderr, "SYSERR: error writing to output file:"
			    " %s\n", strerror(errno));
		    output_close(out);
		}
		else
		    out->size += strlen(t->line)+1;
	    }
	}
	else {
	    prdebug(DEBUG_LINE, ">%s: %s", tname[t->type], t->line);
	    if (t->line[0] != '\0' && out->do_fdesc == FDESC_NOTYET) {
		out->do_fdesc = FDESC_OPEN;
		strcpy(out->tempdesc, "./tmpdesc.XXXXXX");
		if ((fd = mkstemp(out->tempdesc)) == -1) {
		    fprintf(stderr, "SYSERR: can't create "
			    "temporary description file: %s\n",
			    strerror(errno));
		    out->do_fdesc = FDESC_ERROR;
		}
		else {
		    if ((out->fdesc=fdopen(fd, "w")) == NULL) {
			fprintf(stderr, "SYSERR: cannot fdopen "
				"temporary description file `%s': %s\n",
				out->tempdesc, strerror(errno));
			out->tempdesc[0] = '\0';
			out->do_fdesc = FDESC_ERROR;
		    }
		}
	    }

	    if (out->do_fdesc == FDESC_OPEN) {
		if (t->line[0] == '\0') {
		    /* save newline for later */
		    out->fdescnl++;
		}
		else {
		    /* print any saved newlines */
		    while (out->fdescnl > 0) {
			fputc('\n', out->fdesc);
			out->fdescnl--;
		    }

		    fprintf(out->fdesc, "%s\n", t->line);
		    if (ferror(out->fdesc)) {
			fprintf(stderr, "SYSERR: can't write to "
				"description file: %s\n",
				strerror(errno));
			/* XXX: remove file? */
			out->do_fdesc = FDESC_ERROR;
			fclose(out->fdesc);
			out->fdesc = NULL;
		    }
		}
	    }
	}
	break;

    case TOK_EOP:
	if (out->do_fdesc == FDESC_OPEN) {
	    if (out->fdescfilename) {
		close_fdesc(out);

		append_file(out->fdescfilename, out->tempdesc,
			    "\n[DATA]\n\n");
		remove(out->tempdesc);
		out->tempdesc[0] = '\0';
	    }
	    else {
		rename_fdesc(out);
	    }
	}
	free(out->fdescfilename);
	out->fdescfilename = NULL;
	close_fdesc(out);

	/* fallthrough */

    case TOK_EOF:
	if (out->infile)
	    prdebug(DEBUG_SIZE, "[%lu bytes]", out->size);
	prdebug(DEBUG_TOK, ">%s", tname[t->type]);

	output_close(out);
	out->infile = out->warned = 0;
	break;

    case TOK_ERR:
	fprintf(stderr, "Error token type %d: %s\n", t->n, t->line);
	prdebug(DEBUG_ERROR, ">error: type %d: %s", t->n, t->line);
	if (out->infile && out->broken == 0) {
	    /* rename file to .broken */
	    brokenfilename = malloc(strlen(out->filename)+8);
	    sprintf(brokenfilename, "%s.broken", out->filename);

	    outfilename = brokenfilename;
	    if (rename_uniq(out->filename, &brokenfilename) == -1) {
		fprintf(stderr, "SYSERR: error renaming broken output "
			"file from `%s' to `%s': %s\n", out->filename,
			outfilename, strerror(errno));
	    }
	    else {
		free(outfilename);
		free(out->filename);
		out->filename = brokenfilename;

		/* rename description file */
		if (out->fdescfilename != NULL) {
		    brokenfilename = malloc(strlen(out->filename)+6);
		    sprintf(brokenfilename, "%s.desc", out->filename);

		    if (rename(out->fdescfilename, brokenfilename) != 0) {
			fprintf(stderr, "SYSERR: can't rename `%s' "
				"to `%s': %s\n", out->fdescfilename,
				brokenfilename, strerror(errno));
			/* XXX: remove(out->fdescfilename); */
		    }
		    else {
			free(out->fdescfilename);
			out->fdescfilename = brokenfilename;
		    }
		}
	    }
	    out->broken = 1;

	}
	break;

    case TOK_DATA:
	if (!out->infile) {
	    if (!out->warned) {
		fprintf(stderr, "ERROR: data while not in file "
			"(missing fname)");
		out->warned = 1;
	    }
	}
	else if (out->fout) {
	    if (fwrite(t->line, 1, t->n, out->fout) < t->n) {
		fprintf(stderr, "SYSERR: can't write to output file: %s\n",
			strerror(errno));
		output_close(out);
	    }
	    else
		out->size += t->n;
	}
	break;

    default:
	prdebug(DEBUG_TOK, ">%s", tname[t->type]);
	break;
    }

    return 0;
}



static void
close_fdesc(out_state *out)
{
    if (out->do_fdesc == FDESC_OPEN)
	fclose(out->fdesc);

    out->fdescnl = 0;
    out->do_fdesc = FDESC_NOTYET;
}



static void
rename_fdesc(out_state *out)
{
    char *fdescname;

    if (out->filename == NULL)
	return;

    fdescname = xmalloc(strlen(out->filename)+6);
    sprintf(fdescname, "%s.desc", out->filename);
    if (rename(out->tempdesc, fdescname) != 0) {
	fprintf(stderr, "SYSERR: can't rename `%s' to `%s': %s\n",
		out->tempdesc, fdescname, strerror(errno));
	/* XXX: remove(out->tempdesc); */
    }

    out->tempdesc[0] = '\0';
    free(out->fdescfilename);
    out->fdescfilename = fdescname;

    return;
}
