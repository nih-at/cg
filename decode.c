#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "decode.h"
#include "header.h"
#include "mime.h"
#include "util.h"

#define BINHEX_TAG "(This file must be converted with BinHex 4.0)"

enum enctype decode_mime(FILE *fin, FILE **foutp, char **fnamep,
			 struct header *h);
enum enctype decode_uu(FILE *fin, FILE *fout, int inp);
void decode_uu_line(FILE *fout, unsigned char *line);
enum enctype decode_base64(FILE *fin, FILE *fout);
enum enctype decode_binhex(FILE *fin, FILE **foutp, char **fn);



enum enctype
decodefile(FILE *fin, FILE **foutp, enum enctype type)
{
    FILE *fdesc;
    struct header *h;
    struct mime_hdr *m;
    char *s, *line, *descname, b[8192], *filename;
    int nel, i;

    if ((h=header_read(fin, 1)) == NULL)
	return enc_eof;

    filename = descname = NULL;
    fdesc = NULL;
    
    if (type == enc_unknown) {
	if ((s=header_get(h, HDR_CONTENT_TYPE))) {
	    if ((m=mime_parse(s)) != NULL) {
		if (m->type == MIME_CT_MSG_PART) {
		    /* message/partial */
		    
		    mime_free(m);
		    header_free(h);
		    if ((h=header_read(fin, 1)) == NULL) {
			prerror("mime message/partial, but no second header");
			return enc_error;
		    }
		    type = decode_mime(fin, foutp, &filename, h);
		    free(filename);
		    return type;
		}
		else if (m->type == MIME_CT_MSG_MULTI) {
		    /* XXX: message/multipart */
		}
		mime_free(m);
	    }
	}
	else if ((s=header_get(h, HDR_CONTENT_TRENC))) {
	    if ((m=mime_parse(s)) != NULL) {
		if (m->type == MIME_TE_BASE64) {
                     /* mime base64 (single part) */
		    
		    mime_free(m);

		    type = decode_mime(fin, foutp, &filename, h);
		    free(filename);
		    
		    if (type == enc_base64)
			return enc_eof;
		    return type;
		}
		mime_free(m);
	    }
	}

	/* non mime */

	nel = 0;
	while (type == enc_unknown) {
	    if ((line=getline(fin)) == NULL) {
		/* no encoded data found */
		type = enc_nodata;
		break;
	    }
	    if (strncasecmp(line, "begin ", 6) == 0) {
		s = line+6 + strspn(line+6, "01234567");
		if (s != line+6 && *s == ' ') {
		    /* found uuencoded data */
		    
		    type = enc_uu;
		    s = strdup(s+1);
		    filename = s;
		    if ((*foutp = fopen_uniq(&filename)) == NULL) {
			prerror("can't create %s: %s", filename,
			      strerror(errno));
			type = enc_error;
		    }
		    else
			type = decode_uu(fin, *foutp, 1);
		    if (filename != s)
			free(s);
		}
	    }
	    else if (strcmp(line, BINHEX_TAG) == 0) {
		/* found binhexed data */
		
		type = decode_binhex(fin, foutp, &filename);
	    }
	    else if (line[0] == 0) {
		nel++;
	    }
	    else {
		/* XXX: ignore BEGIN, CUT HERE, etc. */
		if (descname == NULL) {
		    descname = ".desc";
		    if ((fdesc=fopen_uniq(&descname)) == NULL)
			prerror("can't create %s: %s", descname,
			      strerror(errno));
		}
		else {
		    for (i=0; i<nel; i++)
			putc('\n', fdesc);
		    nel = 0;
		}
		fprintf(fdesc, "%s\n", line);
	    }
	}
	if (fdesc) {
	    if (fclose(fdesc) != 0)
		prerror("can't close %s: %s", descname, strerror(errno));
	    switch (type) {
	    case enc_error:
	    case enc_nodata:
		remove(descname);
		break;
	    default:
		sprintf(b, "%s.desc", filename);
		if (rename(descname, b) != 0) {
		    prerror("can't rename %s to %s: %s", descname, b,
			  strerror(errno));
		}
		break;
	    }
	}
	return type;
    }	
    else {
	/* not first part */
	switch (type) {
	case enc_uu:
	    type = decode_uu(fin, *foutp, 0);
	    return type;
	case enc_binhex:
	    type = decode_binhex(fin, foutp, NULL);
	    return type;
	case enc_base64:
	    type = decode_base64(fin, *foutp);
	    return type;
	default:
	    prerror("can't happen: decode called with invalid encoding type %d",
		  type);
	    return enc_error;
	}
    }
}  



enum enctype
decode_mime(FILE *fin, FILE **foutp, char **fnamep, struct header *h)
{
    char *s, *filename;
    struct mime_hdr *m;
    int i;

    filename = NULL;

    if ((s=header_get(h, HDR_CONTENT_DISP))) {
	if ((m=mime_parse(s)) != NULL) {
	    for (i=0; m->option[i].name != NULL; i++)
		if (m->option[i].name == MIME_CD_FILENAME) {
		    filename = strdup(m->option[i].value);
		    break;
		}
	    mime_free(m);
	}
    }
		    
    if (filename == NULL) {
	if ((s=header_get(h, HDR_CONTENT_TYPE))) {
	    if ((m=mime_parse(s)) != NULL) {
		for (i=0; m->option[i].name != NULL; i++)
		    if (m->option[i].name == MIME_CT_NAME) {
			filename = strdup(m->option[i].value);
			break;
		    }
		mime_free(m);
	    }
	}
    }

    *fnamep = filename;
    if ((*foutp=fopen_uniq(fnamep)) == NULL) {
	prerror("can't create %s: %s", filename,
	      strerror(errno));
	return enc_error;
    }

    if (*fnamep != filename)
	free(filename);

    return decode_base64(fin, *foutp);
}



enum enctype
decode_uu(FILE *fin, FILE *fout, int inp)
{
    unsigned char *line, b0[62], b1[62], b[45];
    int len, len0, len1, end, i, j;
    long l;
    

    end = len0 = 0;
    while (!inp) {
	if ((line=getline(fin)) == NULL)
	    return enc_nodata;
	
	if (line[0] == ' ' || line[0] == '`') {
	    end = 1;
	}
	else if (strcmp(line, "end") == 0) {
	    if (end == 1) {
		inp = 1;
	    }
	    else
		len0 = 0;
	}
	else if (line[0] < '!' || line[0] > 'M') {
	    end = 0;
	    len0 = 0;
	}
	else {
	    end = 0;
	    len1 = (line[0] - ' ');
	    if (strlen(line)-1 == ((len1+2)/3)*4) {
		if (len0 == 45) {
		    strcpy(b1, line);
		    inp = 1;
		}
		else {
		    strcpy(b0, line);
		    len0 = len1;
		    len1 = 0;
		}
	    }
	    else
		len0 = 0;
	}
    }

    if (len0) {
	decode_uu_line(fout, b0);
	if (len1)
	    decode_uu_line(fout, b1);
    }

    if (end) {
	skip_rest(fin);
	return enc_eof;
    }
    for (;;) {
	if ((line=getline(fin)) == NULL)
	    return enc_uu;

	if (line[0] == 'M') {
	    if (strlen(line) != 61) {
		/* XXX: wrong line */
		skip_rest(fin);
		return enc_uu;
	    }
	    len = 45;
	    end = 0;
	}
	else if (line[0] > ' ' && line[0] < 'M') {
	    len = line[0] - ' ';
	    end = 0;
	    if (strlen(line)-1 != ((len+2)/3)*4) {
		/* XXX: wrong line */
		skip_rest(fin);
		return enc_uu;
	    }
	}
	else if (line[0] == ' ' || line[0] == '`') {
	    end = 1;
	    continue;
	}
	else if (strcmp(line, "end") == 0) {
	    if (end) {
		skip_rest(fin);
		return enc_eof;
	    }
	    else {
		/* XXX: wrong line */
		skip_rest(fin);
		return enc_uu;
	    }
	}
	else {
	    /* XXX: wrong line */
	    skip_rest(fin);
	    return enc_uu;
	}

	for (j=1,i=0; i<len; j+=4) {
	    l = (((line[j]-' ')&0x3f)<<18)
		| (((line[j+1]-' ')&0x3f)<<12)
		| (((line[j+2]-' ')&0x3f)<<6)
		| ((line[j+3]-' ')&0x3f);
	    b[i++] = l >> 16;
	    b[i++] = (l>>8) & 0xff;
	    b[i++] = l & 0xff;
	}
	fwrite(b, 1, len, fout);
    }
}



void
decode_uu_line(FILE *fout, unsigned char *line)
{
    int len, i, j;
    long l;
    unsigned char b[45];
    
    len = line[0] - ' ';

    for (j=1,i=0; i<len; j+=4) {
	l = (((line[j]-' ')&0x3f)<<18)
	    | (((line[j+1]-' ')&0x3f)<<12)
	    | (((line[j+2]-' ')&0x3f)<<6)
	    | ((line[j+3]-' ')&0x3f);
	b[i++] = l >> 16;
	b[i++] = (l>>8) & 0xff;
	b[i++] = l & 0xff;
    }
    fwrite(b, 1, len, fout);
}



enum enctype
decode_base64(FILE *fin, FILE *fout)
{
    skip_rest(fin);
    return enc_base64;
}



enum enctype
decode_binhex(FILE *fin, FILE **foutp, char **fn)
{

    if (!foutp) {
	/* first part */
    }
    else {
	/* other part */
    }

    skip_rest(fin);
    return enc_binhex;
}



int
decode_line(unsigned char *buf, char *line, int *table)
{
    static int rest, no;
    int i;

    if (line == NULL) {
	if (no != 0)
	    i =  DEC_INCOMPLETE;
	else
	    i =  0;
	no = rest = 0;
	return i;
    }
    
    i = 0;

    while (*line) {
	b = table[*(line++)];
	if (b < 0) {
	    switch (b) {
	    case DEC_COLON:
		if (no == 0) 
		    return i | DEC_EOF;
		else
		    return i | DEC_EOF | DEC_ILL;
	    case DEC_EQUAL:
		switch (no) {
		case 0:
		case 1:
		    return i | DEC_ILL;
		case 2:
		    buf[i++] = rest >> 4;
		    break;
		case 3:
		    buf[i++] = rest >> 10;
		    buf[i++] = (rest>>2) & 0xff;
		}
		return i | DEC_EOF;
	    default:
		return i | DEC_ILL;
	    }
	}

	rest = (rest << 6) | b;
	no++;

	if (no == 4) {
	    buf[i++] = rest >> 16;
	    buf[i++] = (rest>>8) & 0xff;
	    buf[i++] = (rest & 0xff);
	    rest = no = 0;
	}
	
    }

    return i;
}
