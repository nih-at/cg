#include <err.h>
#include <stdio.h>
#include "decode.h"
#include "util.h"

char *prg;

int
main(int argc, char *argv[])
{
    FILE *in, *out;
    int i;
    char buf[100];
    enum enctype type;

    prg = argv[0];

    i = 1;
    while (i < argc) {
	in = fopen(argv[i], "r");
	snprintf(buf, sizeof(buf), "OUTPUT.%03d", i);

	if (in == NULL) {
	    warn("can't open input file `%s'", argv[i]);
	}
	else {
	    /* init */
	    decode_line(buf, NULL, NULL);
	    type = enc_unknown;
	    out = NULL;
	    type = decode_file(in, &out, type);

	    if (out)
		fclose(out);

	    printf("decoded `%s', output: %d\n", argv[i], type);
	}
	i++;
    }

    return 0;
}
