#include <stdio.h>
#include <stdlib.h>

#include "decode.h"
#include "header.h"
#include "mime.h"
#include "stream_types.h"
#include "util.h"

char *prg;

int
main(int argc, char *argv[])
{
    stream *stm;
    out_state *out;
    int ret;

    prg = argv[0];

    header_init();
    mime_init();

    stm = stream_fcat_open(argc-1, argv+1);
    out = output_new();

    ret = decode(stm, out) ? 1 : 0;

    output_free(out);

    exit(ret);
}
