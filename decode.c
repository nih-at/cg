enum enctype
decode(FILE *fin, FILE **foutp, enum enctype type)
{
    FILE *fdesc;
    struct header *h;
    struct mime_hdr *m;
    char *s, *line, *descname, b[8192], *filename;
    int nel;

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
			error("mime message/partial, but no second header");
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
	    }
	    if (strncasecmp(line, "begin ", 6) == 0) {
		s = line+6 + strspn(line+6, "01234567");
		if (s != line+6 && *s == ' ') {
		    /* found uuencoded data */
		    
		    type = enc_uu;
		    s = strdup(s+1);
		    filename = s;
		    if ((fout = fopen_uniq(&filename)) == NULL) {
			error("can't create %s: %s", filename,
			      strerror(errno));
			type = enc_error;
		    }
		    else
			type = decode_uu(fin, *foutp);
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
			error("can't create %s: %s", descname,
			      strerror(errno));
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
	    if (fclose(fdesc) != 0)
		error("can't close %s: %s", descname, strerror(errno));
	    switch (type) {
	    case enc_error:
	    case enc_nodata:
		remove(descname);
		break;
	    default:
		sprintf(b, "%s.desc", filename);
		if (rename(descname, b) != 0) {
		    error("can't rename %s to %s: %s", descname, b,
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
	    type = decode_uu(fin, *foutp);
	    return type;
	case enc_binhex:
	    type = decode_binhex(fin, foutp, NULL);
	    return type;
	case enc_base64:
	    type = decode_base64(fin, *foutp);
	    return type;
	default:
	    error("can't happen: decode called with invalid encoding type %d",
		  type);
	    return enc_error;
	}
    }
}  



enum enctype
decode_mime(FILE *fin, FILE **foutp, char **fnamep, struct header *h)
{
    char *s, *filename;
    struct mime *m;
    int i;

    filename = NULL;

    if (s=header_get(h, HDR_CONTENT_DISP)) {
	if ((m=mime_parse(s)) != NULL) {
	    for (i=0; m->option[i].name != NULL; i++)
		if (m->option[i].name == MIME_CD_FILENAME) {
		    filename = strdup(m->option[i].val);
		    break;
		}
	    mime_free(m);
	}
    }
		    
    if (filename == NULL) {
	if (s=header_get(h, HDR_CONTENT_TYPE)) {
	    if ((m=mime_parse(s)) != NULL) {
		for (i=0; m->option[i].name != NULL; i++)
		    if (m->option[i].name == MIME_CT_NAME) {
			filename = strdup(m->option[i].val);
			break;
		    }
		mime_free(m);
	    }
	}
    }

    *fnamep = filename;
    if ((*foutp=fopen_uniq(fnamep)) == NULL) {
	error("can't create %s: %s", filename,
	      strerror(errno));
	return enc_error;
    }

    if (*fnamep != filename)
	free(filename);

    return decode_base64(fin, *fout);
}



enum enctype
decode_uu(FILE *fin, FILE *fout, int inp)
{
    char *line, b[2][62], b[45], *s;
    int len0, len1, end, i, j;
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
		    strcpy(b[1], line);
		    inp = 1;
		}
		else {
		    strcpy(b[0], line);
		    len0 = len1;
		    len1 = 0;
		}
	    }
	    len0 = 0;
	}
    }

    if (len0) {
	decode_uu_line(fout, b[0]);
	if (len1)
	    decode_uu_line(fout, b[1]);
    }

    if (end)
	return enc_eof;

    for (;;) {
	if ((line=getline(fin)) == NULL)
	    return enc_uu;

	if (line[0] == 'M') {
	    if (strlen(line) != 61) {
		/* XXX: wrong line */
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
		return enc_uu;
	    }
	}
	if (line[0] == ' ' || line[0] == '`')
	    end = 1;
	else if (strcmp(line, "end") == 0) {
	    if (end)
		return enc_eof;
	    else {
		/* XXX: wrong line */
		return enc_uu;
	    }
	}

	for (j=1,i=0; i<len; j+=4) {
	    l = ((line[j]&0x3f)<<18)
		| ((line[j+1]&0x3f)<<12)
		| ((line[j+2]&0x3f)<<6)
		| line[j+3]&0x3f;
	    b[i++] = l >> 16;
	    b[i++] = (l>>8) & 0xff;
	    b[i++] = l & 0xff;
	}
	fwrite(b, 1, 3, fout);
    }
}



decode_uu_line(FILE *fout, char *line)
{
    int len, i, j;
    long l;
    char b[45];
    
    len = line[0] - ' ';

    for (j=1,i=0; i<len; j+=4) {
	l = ((line[j]&0x3f)<<18)
	    | ((line[j+1]&0x3f)<<12)
	    | ((line[j+2]&0x3f)<<6)
	    | line[j+3]&0x3f;
	b[i++] = l >> 16;
	b[i++] = (l>>8) & 0xff;
	b[i++] = l & 0xff;
    }
    fwrite(b, 1, 3, fout);
}
