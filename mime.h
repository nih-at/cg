#ifndef _HAD_MIME_H
#define _HAD_MIME_H

#include "symbol.h"

struct mime_hdr {
    symbol type;
    struct mime_opt *option;
};

struct mime_opt {
    symbol name;
    char *value;
};



#define MIME_CT_MSG_PART	mime_sym[0]
#define MIME_CT_MSG_MULTI	mime_sym[1]
#define MIME_TE_BASE64		mime_sym[2]
#define MIME_CD_FILENAME	mime_sym[3]
#define MIME_CT_NAME		mime_sym[4]

extern symbol mime_sym[];



void mime_init(void);
struct mime_hdr *mime_parse(char *header);
void mime_free(struct mime_hdr *m);

#endif /* mime.h */
