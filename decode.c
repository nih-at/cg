enum enctype
decode(FILE *fin, FILE **foutp, enum enctype type)
{
    FILE *fdesc;
    struct header *h;
    struct mime_hdr *m;
    char *s, *line, *descname, b[8192];
    int nel;

    if ((h=header_read(fin, 1)) == NULL)
	return enc_eof;

    descname = NULL;
    fdesc = NULL;
    
    if (type == enc_unknown) {
	if ((s=header_get(h, HDR_CONTENT_TYPE))) {
	    if ((m=mime_parse(s)) != NULL) {
		if (m->type == MIME_CT_MSG_PART) {
		    /* XXX: message/partial */
		    mime_free(m);
		    header_free(h);
		    if ((h=header_read(fin, 1)) == NULL) {
			error("mime message/partial, but no second header");
			return enc_error;
		    }
		    type = roteier(fin,foutp,&filename,h);
		    if ((s=header_get(h, HDR_CONTENT_TYPE))) {
			if ((m=mime_parse(s)) != NULL) {
			    if (m->
		    
		    
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
		    
		    if (s=header_get(h, HDR_CONTENT_DISP)) {
			if ((m=mime_parse(s)) != NULL) {
			    for (i=0; m->option[i].name != NULL; i++)
				if (m->option[i].name == MIME_CD_FILENAME) {
				    filename = m->option[i].val;
				    break;
				}
			}
		    }
		    
		    if (filename == NULL) {
			mime_free(m);
			if (s=header_get(h, HDR_CONTENT_TYPE)) {
			    if ((m=mime_parse(s)) != NULL) {
				for (i=0; m->option[i].name != NULL; i++)
				    if (m->option[i].name == MIME_CT_NAME) {
					filename = m->option[i].val;
					break;
				    }
			    }
			}
		    }

		    if ((*foutp=fopen_uniq(&filename)) == NULL) {
			error("can't create %s: %s", filename,
			      strerror(errno));
			return enc_error;
		    }
		    else
			type = decode_base64(fin, *fout);
		    
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
		    if ((fdesc=fopen_uniq(&descname)) == NULL) {
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
