#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>

#include "stream.h"
#include "output.h"
#include "util.h"



out_state *
output_new()
{
    out_state *out;

    out = xmalloc(sizeof(*out));

    out->infile = out->warned = out->ndata = 0;
    out->do_debug = 1;
    out->do_fdesc = -1;
    out->fout = out->debug = out->fdesc = NULL;

    return out;
}



void
output_free(out_state *out)
{
    if (!out)
	return;

    if (out->fout)
	fclose(out->fout);

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
    char debugfilename[] = ".debug";
    char *outfilename;
    char *fdescname;
    int fd;

    if (t->type == TOK_DATA)
	out->ndata++;
    else {
	if (out->ndata)
	    printf(">data [%d]\n", out->ndata);
	out->ndata = 0;
    }

    switch (t->type) {
    case TOK_FNAME:
	printf(">%s: %s\n", tname[t->type], t->line);

	if (out->infile) {
	    fprintf(stderr, "ERROR: fname while in file (missing EOF)\n");
	    if (out->fout)
		fclose(out->fout);
	}
	outfilename = basename(t->line);
	if ((out->fout=fopen_uniq(&outfilename)) == NULL) {
	    fprintf(stderr, "SYSERR: cannot create output file `%s': %s\n",
		   t->line, strerror(errno));
	}
	out->infile = 1;
	if ((out->do_fdesc != -1) && (outfilename != NULL)) {
	    fdescname = xmalloc(strlen(outfilename)+6);
	    sprintf(fdescname, "%s.desc", outfilename);
	    if (rename(out->tempdesc, fdescname) != 0) {
		fprintf(stderr, "SYSERR: can't rename `%s' to `%s': %s\n",
			out->tempdesc, fdescname, strerror(errno));
		/* XXX: remove(out->tempdesc); */
	    }
	    free(fdescname);
	}
	if (outfilename != basename(t->line))
	    free(outfilename);
	break;

    case TOK_DEBUG:
	if (out->do_debug && out->debug == NULL) {
	    if ((out->debug=fopen(debugfilename, "w")) == NULL) {
		fprintf(stderr, "SYSERR: cannot create debug file `%s': %s\n",
		       debugfilename, strerror(errno));
		out->do_debug = 0;
	    }
	}
	if (out->do_debug) {
	    if (fputs(t->line, out->debug) != 0) {
		fprintf(stderr, "SYSERR: cannot write to debug file `%s': "
			"%s\n", debugfilename, strerror(errno));
		out->do_debug = 0;
	    }
	}
	printf(">%s: %s\n", tname[t->type], t->line);
	break;
	
    case TOK_LINE:
	printf(">%s: %s\n", tname[t->type], t->line);
	if (out->infile) {
	    if (out->fout != NULL) {
		fprintf(out->fout, "%s\n", t->line);
		if (ferror(out->fout)) {
		    fprintf(stderr, "SYSERR: error writing to output file:"
			    " %s\n", strerror(errno));
		    fclose(out->fout);
		    out->fout = NULL;
		}
	    }
	    break;
	}
	
	if ((t->line[0] != '\0') && (out->do_fdesc == -1) && !out->fdesc) {
	    out->do_fdesc = 1;
	    strcpy(out->tempdesc, "./tmpdesc.XXXXXX");
	    if ((fd = mkstemp(out->tempdesc)) == -1) {
		fprintf(stderr, "SYSERR: can't create temporary description "
			"file: %s\n", strerror(errno));
		out->do_fdesc = 0;
	    }
	    else {
		out->fdesc = fdopen(fd, "w");
	    }
	}
	
	if ((out->do_fdesc == 1) && out->fdesc) {
	    if (t->line[0] == '\0') {
		/* save newline for later */
		out->fdescnl++;
	    } else {
		/* print any saved newlines */
		while(out->fdescnl > 0) {
		    fputc('\n', out->fdesc);
		    out->fdescnl--;
		}
		fprintf(out->fdesc, "%s\n", t->line);
		if (ferror(out->fdesc)) {
		    fprintf(stderr, "SYSERR: can't write to description file:"
			    " %s\n", strerror(errno));
		    /* XXX: remove file? */
		    out->do_fdesc = 0;
		    fclose(out->fdesc);
		    out->fdesc = NULL;
		}
	    }
	}
	
	break;

    case TOK_EOP:
	printf(">%s\n", tname[t->type]);
	if (out->fdesc) {
	    fclose(out->fdesc);
	    remove(out->tempdesc);
	    out->fdesc = NULL;
	}
	out->do_fdesc = -1;
	out->fdescnl = 0;
	break;
	
    case TOK_EOF:
	printf(">%s\n", tname[t->type]);

	if (out->fout) {
	    fclose(out->fout);
	}
	out->fout = NULL;
	out->infile = out->warned = 0;
	break;

    case TOK_ERR:
	fprintf(stderr, "Error token type %d: %s\n", t->n, t->line);
	break;

    case TOK_DATA:
	if (!out->infile && !out->warned) {
	    fprintf(stderr, "ERROR: data while not in file (missing fname)");
	    out->warned = 1;
	}
	if (out->fout) {
	    if (fwrite(t->line, 1, t->n, out->fout) < t->n) {
		fprintf(stderr, "SYSERR: can't write to output file: %s\n",
			strerror(errno));
		fclose(out->fout);
		out->fout = NULL;
	    }
	}
	break;

    default:
	printf(">%s\n", tname[t->type]);
	break;
    }


    return 0;
}
