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
    out->fout = NULL;

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
	"EOF", "line", "data", "EOS", "EOH", "EOA", "fname", "debug", "error"
    };
    static char **tname = _foo+1;

    if (t->type == TOK_DATA)
	out->ndata++;
    else {
	if (out->ndata)
	    printf(">data [%d]\n", out->ndata);
	out->ndata = 0;
	printf(">%s", tname[t->type]);
    }

    switch (t->type) {
    case TOK_FNAME:
	printf(": %s\n", t->line);

	if (out->infile) {
	    printf("ERROR: fname while in file (missing EOF)\n");
	    if (out->fout)
		fclose(out->fout);
	}
	if ((out->fout=fopen(basename(t->line), "wb")) == NULL) {
	    printf("SYSERR: cannot create `%s': %s\n",
		   t->line, strerror(errno));
	}
	out->infile = 1;
	break;

    case TOK_DEBUG:
    case TOK_LINE:
	printf(": %s\n", t->line);
	break;

    case TOK_EOF:
	putc('\n', stdout);

	if (out->fout) {
	    fclose(out->fout);
	}
	out->fout = NULL;
	out->infile = out->warned = 0;
	break;

    case TOK_ERR:
	printf(" %d: %s\n", t->n, t->line);
	break;

    case TOK_DATA:
	if (!out->infile && !out->warned) {
	    printf("ERROR: data while not in file (missing fname)");
	    out->warned = 1;
	}
	if (out->fout)
	    fwrite(t->line, 1, t->n, out->fout);
	break;

    default:
	putc('\n', stdout);
	break;
    }


    return 0;
}
