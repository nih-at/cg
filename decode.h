enum enctype {
    enc_unknown, enc_error, enc_eof, enc_nodata,
    enc_uu, enc_binhex, enc_base64
};

#define DEC_ERRMASK	~0xfff
#define DEC_EOF		0x1000
#define DEC_INCOMPLETE	0x2000
#define DEC_ILLEGAL	0x4000

#define DEC_ILL		-1
#define DEC_COLON	-2
#define DEC_EQUAL	-3

extern int decode_table_base64[256];
extern int decode_table_uuencode[256];
extern int decode_table_binhex[256];



enum enctype decode_file(FILE *fin, FILE **foutp, enum enctype type);
int decode_line(unsigned char *buf, char *line, int *table);

