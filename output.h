#ifndef _HAD_OUTPUT_H
#define _HAD_OUTPUT_H

#include "stream.h"

struct out_state {
    int broken;		/* is the file broken (decoding error and similar) */
    int infile;		/* are we currently writing a file */
    int warned;		/* have we warned about data outside a file */
    int ndata;		/* number of data consecutive tokens got */
    int do_fdesc;	/* fdesc state: FDESC_NOTYET: not opened,
			   FDESC_ERROR: given up, FDESC_OPEN: writing */
    char *fdescfilename;	/* name of previous description file */
    char tempdesc[64];	/* temporary name of description file */
    int fdescnl;	/* number of empty lines saved for description file */
    unsigned long size;	/* length of data written */
    FILE *fout;		/* file we're decoding into */
    FILE *fdesc;	/* description file */
    char *filename;	/* name of the output file */
};

typedef struct out_state out_state;



int output(out_state *out, token *t);
void output_free(out_state *out);
out_state *output_new();


#endif /* output.h */
