#ifndef _HAD_OUTPUT_H
#define _HAD_OUTPUT_H

#include "stream.h"

struct out_state {
    int infile;		/* are we currently writing a file */
    int warned;		/* have we warned about data outside a file */
    int ndata;		/* number of data consecutive tokens got */
    int do_fdesc;	/* fdesc state: -1: not opened,
			   0: given up, 1: writing */
    char *prev_fdesc;	/* name of previous description file */
    char tempdesc[64];	/* temporary name of description file */
    int fdescnl;	/* number of empty lines saved for description file */
    FILE *fout;		/* file we're decoding into */
    FILE *fdesc;	/* description file */
};

typedef struct out_state out_state;



int output(out_state *out, token *t);
void output_free(out_state *out);
out_state *output_new();


#endif /* output.h */
