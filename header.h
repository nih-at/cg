#ifndef _HAD_HEADER_H
#define _HAD_HEADER_H

#include "output.h"
#include "stream.h"
#include "symbol.h"

struct header {
    struct header *next;
    symbol type;
    char *value;
};



#define HDR_SUBJECT		hdr_sym[0]
#define HDR_MIME_VERSION	hdr_sym[1]
#define HDR_CONTENT_TYPE	hdr_sym[2]
#define HDR_CONTENT_TRENC	hdr_sym[3]
#define HDR_CONTENT_DISP	hdr_sym[4]

extern symbol hdr_sym[];



struct header *header_read(stream *stm, out_state *out);
void header_free(struct header *h);
char *header_get(struct header *header, symbol field);
void header_init(void);
int header_print(FILE *f, struct header *header);

#endif /* header.h */
