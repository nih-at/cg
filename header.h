#include "symbol.h"

struct header {
    symbol type;
    char *value;
};



#define HDR_SUBJECT		hdr_sym[0]
#define HDR_MIME_VERSION	hdr_sym[1]
#define HDR_CONTENT_TYPE	hdr_sym[2]
#define HDR_CONTENT_TRENC	hdr_sym[3]
#define HDR_CONTENT_DISP	hdr_sym[4]

#define HDR_MAX 5

char *hdr_string[HDR_MAX] = {
    "Subject",
    "Mime-Version",
    "Content-Type",
    "Content-Transfer-Encoding",
    "Content-Disposition"
};

symbol hdr_sym[HDR_MAX];



int header_init(void);

struct header *header_read(FILE *f, int dotp);
int header_print(FILE *f, struct header *header);
char *header_get(struct header *header, symbol field);

