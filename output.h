#ifndef _HAD_OUTPUT_H
#define _HAD_OUTPUT_H

#include "stream.h"

struct out_state {
    int infile;
    int warned;
    FILE *fout;
};

typedef struct out_state out_state;



int output(out_state *out, token *t);
void output_free(out_state *out);
out_state *output_new();


#endif /* output.h */
