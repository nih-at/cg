#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>

#include "stream.h"
#include "output.h"
#include "util.h"

static void close_fdesc(out_state *out);



out_state *
output_new()
{
    out_state *out;

    out = xmalloc(sizeof(*out));

    out->infile = out->warned = out->ndata = 0;
    out->do_fdesc = -1;
    out->fdescnl = 0;
    out->size = 0;
    out->prev_fdesc = NULL;
    out->fout = out->fdesc = NULL;

    return out;
}



void
output_free(out_state *out)
{
    if (!out)
	return;

    if (out->infile && out->fout)
	fclose(out->fout);
    close_fdesc(out);
    free(out->prev_fdesc);

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
    char *fdescname;
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
	    if (out->fout)
		fclose(out->fout);
	}
	outfilename = our_basename(t->line);
	prdebug(DEBUG_FNAM, "writing `%s'", outfilename);

	if ((out->fout=fopen_uniq(&outfilename)) == NULL) {
	    fprintf(stderr, "SYSERR: cannot create output file `%s': %s\n",
		   t->line, strerror(errno));
	    /* discard description */
	    if (out->do_fdesc == 1)
		remove(out->tempdesc);
	}
	else {
	    /* rename description file */
	    if (out->do_fdesc != -1) {
		fdescname = xmalloc(strlen(outfilename)+6);
		sprintf(fdescname, "%s.desc", outfilename);
		if (rename(out->tempdesc, fdescname) != 0) {
		    fprintf(stderr, "SYSERR: can't rename `%s' to `%s': %s\n",
			    out->tempdesc, fdescname, strerror(errno));
		    /* XXX: remove(out->tempdesc); */
		}
		if (out->prev_fdesc)
		    free(out->prev_fdesc);
		out->prev_fdesc = fdescname;
	    }
	    free(outfilename);
	}
	close_fdesc(out);
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
		    fclose(out->fout);
		    out->fout = NULL;
		}
		else
		    out->size += strlen(t->line)+1;
	    }
	}
	else {
	    prdebug(DEBUG_LINE, ">%s: %s", tname[t->type], t->line);
	    if (t->line[0] != '\0' && out->do_fdesc == -1) {
		out->do_fdesc = 1;
		strcpy(out->tempdesc, "./tmpdesc.XXXXXX");
		if ((fd = mkstemp(out->tempdesc)) == -1) {
		    fprintf(stderr, "SYSERR: can't create "
			    "temporary description file: %s\n",
			    strerror(errno));
		    out->do_fdesc = 0;
		}
		else {
		    if ((out->fdesc=fdopen(fd, "w")) == NULL) {
			fprintf(stderr, "SYSERR: cannot fdopen "
				"temporary description file `%s': %s\n",
				out->tempdesc, strerror(errno));
			out->do_fdesc = 0;
		    }
		}
	    }
	
	    if (out->do_fdesc == 1) {
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
			out->do_fdesc = 0;
			fclose(out->fdesc);
			out->fdesc = NULL;
		    }
		}
	    }
	}
	break;

    case TOK_EOP:
	if (out->do_fdesc == 1) {
	    if (out->prev_fdesc) {
		close_fdesc(out);

		append_file(out->prev_fdesc, out->tempdesc,
			    "\n[DATA]\n\n");
	    }
	    remove(out->tempdesc);
	}
	free(out->prev_fdesc);
	out->prev_fdesc = NULL;
	close_fdesc(out);
	
	/* fallthrough */
	
    case TOK_EOF:
	if (out->infile)
	    prdebug(DEBUG_SIZE, "[%lu bytes]", out->size);
	prdebug(DEBUG_TOK, ">%s", tname[t->type]);

	if (out->infile && out->fout)
	    fclose(out->fout);
	out->infile = out->warned = 0;
	break;

    case TOK_ERR:
	fprintf(stderr, "Error token type %d: %s\n", t->n, t->line);
	prdebug(DEBUG_ERROR, ">error: type %d: %s", t->n, t->line);
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
		fclose(out->fout);
		out->fout = NULL;
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
    if (out->do_fdesc == 1)
	fclose(out->fdesc);

    out->fdescnl = 0;
    out->do_fdesc = -1;
}
