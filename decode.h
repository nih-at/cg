enum enctype {
    enc_unknown, enc_error, enc_eof, enc_nodata,
    enc_uu, enc_binhex, enc_base64
};

enum enctype decodefile(FILE *fin, FILE **foutp, enum enctype type);

