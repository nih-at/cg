/*
  $NiH: mime.c,v 1.16 2002/04/10 16:23:28 wiz Exp $

  mime.c -- MIME header parsing
  Copyright (C) 2002 Dieter Baron and Thomas Klausner

  This file is part of cg, a program to assemble and decode binary Usenet
  postings.  The authors can be contacted at <nih@giga.or.at>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mime.h"

void _mime_lws(char **s);
symbol _mime_token(char **s);
char * _mime_value(char **s);

#define MIME_MAX 14

char *mime_string[MIME_MAX] = {
    "message/partial",
    "message/multipart",
    "multipart/mixed",
    "7bit",
    "8bit",
    "base64",
    "quoted-printable",
    "x-uuencode",
    "filename",
    "name",
    "boundary",
    "id",
    "number",
    "total"
};

symbol mime_sym[MIME_MAX];



void
mime_init(void)
{
    int i;

    for (i=0; i<MIME_MAX; i++)
	mime_sym[i] = intern(mime_string[i]);
}



void
mime_free(struct mime_hdr *m)
{
    int i;

    if (m == NULL)
	return;

    for (i=0; m->option[i].name; i++)
	free(m->option[i].value);

    free(m->option);
    free(m);
}



struct mime_hdr *
mime_parse(char *h)
{
    struct mime_hdr *m;
    struct mime_opt o[256];
    int i;

    if ((m=(struct mime_hdr *)malloc(sizeof(struct mime_hdr))) == NULL)
	return NULL;

    if ((m->type=_mime_token(&h)) == NULL) {
	free(m);
	return NULL;
    }

    i = 0;

    _mime_lws(&h);
    while (*h == ';') {
	h++;

	if ((o[i].name=_mime_token(&h)) == NULL) {
	    break;
	}
	_mime_lws(&h);
	if (*h == '=') {
	    h++;
	    o[i].value = _mime_value(&h);
	}
	else
	    o[i].value = NULL;

        i++;
	/* XXX: handle buffer overrun */
    }

    o[i].name = o[i].value = NULL;
    i++;

    if ((m->option=(struct mime_opt *)malloc(sizeof(struct mime_opt)*i))
	== NULL) {
	free(m);
	while (i)
	    free(o[--i].value);
	return NULL;
    }
    memcpy(m->option, o, sizeof(struct mime_opt)*i);

    return m;
}



symbol
_mime_token(char **s)
{
    symbol m;
    char c, *p;

    _mime_lws(s);
    p = *s+strcspn(*s, " \t()<>@,;:\\\"[]?=");
    c = *p;
    *p = '\0';
    if (strlen(*s) > 0)
	m = intern_lower(*s);
    else
	m = 0;
    *p = c;
    *s = p;
    return m;
}



void
_mime_lws(char **s)
{
    /* XXX: handle () comments */

    *s += strspn(*s, " \t");
}



char *
_mime_value(char **s)
{
    static char b[8192];
    char *p, *r, c;
    int i;

    i=0;
    _mime_lws(s);

    if (**s == '"') {
	/* quoted string */
	p = *s;
	for (;;) {
	    switch (*(++p)) {
	    case '"':
		p++;
		/* fallthrough */
	    case '\0':
		b[i++]='\0';
		*s = p;
		return strdup(b);
	    case '\\':
		if (*p == '\0') {
		    b[i++]='\0';
		    *s = p;
		    return strdup(b);
		}
		else
		    b[i++]=*(++p);
		break;
	    default:
		b[i++]=*p;
		break;
	    }
	}
    }
    else {
	/* token */
	p = *s+strcspn(*s, " \t()<>@,;:\\\"[]?=");
	c = *p;
	*p = '\0';
	if (strlen(*s) > 0)
	    r = strdup(*s);
	else
	    r = NULL;
	*p = c;
	*s = p;
	return r;
    }
}



char *
mime_option_get(struct mime_hdr *m, symbol name)
{
    int i;

    for (i=0; m->option[i].name; i++)
	if (m->option[i].name == name)
	    return m->option[i].value;

    return NULL;
}
