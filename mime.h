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
#define MIME_CT_MULTI_MIX	mime_sym[2]
#define MIME_TE_7BIT		mime_sym[3]
#define MIME_TE_8BIT		mime_sym[4]
#define MIME_TE_BASE64		mime_sym[5]
#define MIME_TE_QUOT_PRINT	mime_sym[6]
#define MIME_TE_X_UUENCODE	mime_sym[7]
#define MIME_CD_FILENAME	mime_sym[8]
#define MIME_CT_NAME		mime_sym[9]
#define MIME_CT_BOUNDARY	mime_sym[10]
#define MIME_CT_ID		mime_sym[11]
#define MIME_CT_NUMBER		mime_sym[12]
#define MIME_CT_TOTAL		mime_sym[13]

extern symbol mime_sym[];



void mime_init(void);
struct mime_hdr *mime_parse(char *header);
void mime_free(struct mime_hdr *m);
char *mime_option_get(struct mime_hdr *m, symbol name);

#endif /* mime.h */
