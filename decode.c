#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "stream.h"
#include "decode.h"
#include "header.h"
#include "mime.h"
#include "util.h"
#include "stream_types.h"

#define BINHEX_TAG "(This file must be converted with BinHex 4.0)"

int decode_binhex(stream *in, out_state *out);
int decode_file(stream *in, out_state *out, int *tbl);
int decode_mime(stream *in, out_state *out, struct header *h,
		struct mime_hdr *ct, struct mime_hdr *te);




int
decode(stream *in, out_state *out)
{
    stream *stm, *st2, *st3;
    struct header *h;
    struct mime_hdr *m, *m2;
    token *t, tok;
    char *s;
    

    while (!stream_eof(in)) {
	if ((h=header_read(in, out)) == NULL) {
	    /* XXX: error: no header found */
	    skip_to(in, TOK_EOA);
	    continue;
	}
	
	if ((s=header_get(h, HDR_CONTENT_TYPE)) && (m=mime_parse(s))) {
	    if (m->type == MIME_CT_MSG_PART) {
		debug(out, "found: MIME message/partial");
		
		stm = stream_msg_partial_open(in, m);
		st2 = stream_article_open(stm);
		decode(st2, out);
		stream_close(st2);
		stream_close(stm);
	    }
	    else if (m->type == MIME_CT_MULTI_MIX) {
		debug(out, "found: MIME multipart/mixed");
		
		stm = stream_msg_multi_open(in, m);

		while (!stream_eof(stm)) {
		    st2 = stream_section_open(stm, NULL);
		    st3 = stream_article_open(st2);
		    decode(st3, out);
		    stream_close(st3);
		    stream_close(st2);
		}

		stream_close(stm);
		output(out, TOKEN_EOF);
	    }
	    else if ((s=header_get(h, HDR_CONTENT_TRENC))
		     && (m2=mime_parse(s))) {
		debug(out, "found: MIME (single part)");
		
		decode_mime(in, out, h, m, m2);
		mime_free(m2);
	    }
	    mime_free(m);
	}
	    	
	header_free(h);

	/* non mime */

	while ((t=stream_get(in))->type == TOK_LINE) {
	    if (strncmp(t->line, "begin ", 6) == 0) {
		s = t->line+6 + strspn(t->line+6, "01234567");
		if (s != t->line+6 && *s == ' ') {
		    debug(out, "found: uuencoded");
		    
		    output(out, token_set(&tok, TOK_FNAME, s+1));
		    stm = stream_uuextract_open(in);
		    decode_file(stm, out, decode_table_uuencode);
		    output(out, TOKEN_EOF);
		    stream_close(stm);
		}
	    }
	    else if (strcmp(t->line, BINHEX_TAG) == 0) {
		debug(out, "found: binhex");
		
		decode_binhex(in, out);
	    }
	    else {
		/* XXX: ignore BEGIN, CUT HERE, etc. */
		if (strncmp(t->line, "BEGIN --- CUT HERE", 18) == 0) {
		    /* skip -- line is not interesting */
		    continue;
		}

		output(out, t);
	    }
	}
    }

    return 0;
}  



int
decode_mime(stream *in, out_state *out, struct header *h,
	    struct mime_hdr *ct, struct mime_hdr *te)
{
    token t;
    char *s, *filename;
    struct mime_hdr *cd;

    filename = NULL;
    cd = NULL;

    if ((s=header_get(h, HDR_CONTENT_DISP)) && ((cd=mime_parse(s))))
	filename = mime_option_get(cd, MIME_CD_FILENAME);
    if (filename == NULL)
	filename = mime_option_get(ct, MIME_CT_NAME);

    if (te->type == MIME_TE_BASE64) {
	debug(out, "found: MIME base64");

	output(out, token_set(&t, TOK_FNAME, filename));
	decode_file(in, out, decode_table_base64);
	output(out, TOKEN_EOF);
    }
    else if (filename) {
	debug(out, "found: MIME %s", te->type);

	output(out, token_set(&t, TOK_FNAME, filename));
	copy_stream(in, out);
	output(out, TOKEN_EOF);
    }
    else
	debug(out, "no filename, unknown MIME transfer encoding: %s",
	      te->type);

    mime_free(cd);

    return 0;
}



int
decode_file(stream *in, out_state *out, int *tbl)
{
    stream *stm;

    stm = stream_decode_open(in, tbl);
    copy_stream(stm, out);
    stream_close(stm);

    return 0;
}



int
decode_binhex(stream *in, out_state *out)
{
    token tok;
    
    /* XXX: extract file name */
    /* XXX: handle multi-part */

    output(out, token_set(&tok, TOK_FNAME, NULL));
    decode_file(in, out, decode_table_binhex);
    output(out, TOKEN_EOF);

    return 0;
}

