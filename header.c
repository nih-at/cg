/*
  $NiH$
  
  header.c -- RFC 822 header parsing
  Copyright (C) 2002 Dieter Baron and Thomas Klaunser

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

#include "util.h"
#include "header.h"

#define HDR_MAX 5

char *hdr_string[HDR_MAX] = {
    "Subject",
    "Mime-Version",
    "Content-Type",
    "Content-Transfer-Encoding",
    "Content-Disposition"
};

symbol hdr_sym[HDR_MAX];



void
header_init(void)
{
    int i;

    for (i=0; i<HDR_MAX; i++)
	hdr_sym[i] = intern(hdr_string[i]);
}



struct header *
header_read(stream *in, out_state *out)
{
    struct header h, *act;
    token *t;
    char *p;

    act = &h;
    h.next = NULL;
    h.value = NULL;
    h.type = NULL;
    

    while ((t=stream_get(in))->type != TOK_EOH && t->type != TOK_EOF) {
	if (t->type == TOK_LINE) {
	    if (strncmp(t->line, "From ", 5) == 0) {
		/* mbox envelope */
		continue;
	    }
	    else {
		for (p=t->line; (*p > ' ') && (*p < 127); p++) {
		    if (*p == ':') {
			if ((*(p+1) == ' ') || (*(p+1) == '\t')) {
			    *(p++) = '\0';
			    act->next = xmalloc(sizeof(struct header));
			    act = act->next;
			    act->next = NULL;
			    act->value = NULL;
			    if ((act->type=intern_caps(t->line)) == NULL) {
				/* XXX: better handling */
				break;
			    }
			    p = p + strspn (p, " \t");
			    if (strlen(p) > 0)
			    act->value = xstrdup(p);
			    break;
			}
			else {
			    /* not really a header after all */
			    break;
			}
		    }
		}
	    }
	}
	else
	    output(out, t);
    }
	
    return h.next;
}



void
header_free(struct header *h)
{
    struct header *h2;

    while (h) {
	h2 = h;
	h = h->next;
	free(h2->value);
	free(h2);
    }
}



char *
header_get(struct header *h, symbol field)
{
    for (; h; h=h->next) {
	if (h->type == field)
	    return h->value;
    }

    return NULL;
}
