#ifndef _HAD_DECODE_H
#define _HAD_DECODE_H

#include "stream.h"
#include "output.h"

enum enctype {
    enc_unknown, enc_error, enc_eof, enc_nodata,
    enc_uu, enc_binhex, enc_base64
};

#define DEC_ERRMASK	~0xfff
#define DEC_EOF		0x1000
#define DEC_INCOMPLETE	0x2000
#define DEC_ILLEGAL	0x4000

#define DEC_ILL		-1
#define DEC_END		-2
#define DEC_PAD		-3

extern int decode_table_base64[256];
extern int decode_table_uuencode[256];
extern int decode_table_binhex[256];



int decode(stream *in, out_state *out);

#endif /* decode.h */
