enum enctype
decode(FILE *fin, FILE *fout, enum enctype type)
{
    struct header *h;
    struct mime_hdr *m;
    char *s, *line;
    int nel;

    if ((h=header_read(fin, 1)) == NULL)
	return enc_eof;

    if (type == enc_unknown) {
	if ((s=header_get(h, HDR_CONTENT_TYPE))) {
	    if ((m=mime_parse(s)) != NULL) {
		if (m->type == MIME_CT_MSG_PART) {
		    /* XXX: message/partial */
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
		    /* XXX: mime base64 (single part) */
		    
		    mime_free(m);
		    return enc_eof;
		}
		mime_free(m);
	    }
	}

	/* non mime */

	nel = 0;
	while (type == enc_unknown) {
	    if ((line=getline(fin)) == NULL) {
		/* XXX: no encoded data found */
	    }
	    if (strncasecmp(line, "begin ", 6) == 0) {
		s = line+6 + strspn(line+6, "01234567");
		if (s != line+6 && *s == ' ') {
		    /* found uuencoded data */
		    
		    s = strdup(s+1);
		    filename = s;
		    fout = fopen_uniq(&filename);
		    if (filename != s)
			free(s);
		    type = enc_uu;
		    decode_uu(fin, fout);
		}
	    }
	    else if (strcmp(line, BINHEX_TAG) == 0) {
		/* found binhexed data */
		type = enc_binhex;
	    }
	    else if (line[0] == 0) {
		nel++;
	    }
	    else {
		/* XXX: ignore BEGIN, CUT HERE, etc. */
		if (fdesc == NULL) {
		    /* XXX: open desc file */
		}
		else {
		    for (i=0; i<nel; i++)
			putc('\n', fdesc);
		    nel = 0;
		}
		fprints(fdesc, "%s\n", line);
	    }
	}
	if (fdesc) {
	    /* XXX: check for errors */
	    fclose(fdesc);
	}
    }
    else {
	/* XXX: not first part */
    }
}
