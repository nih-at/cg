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
    stream *stm, *st2;
    out_state *out;
    int ret;

    prg = argv[0];

    header_init();
    mime_init();

    stm = stream_fcat_open(argc-1, argv+1);
    st2 = stream_article_open(stm);
    out = output_new();

    ret = decode(st2, out) ? 1 : 0;

    stream_close(st2);
    stream_close(stm);

    output_free(out);

    exit(ret);
}
