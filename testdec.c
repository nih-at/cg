/*
  $NiH: testdec.c,v 1.8 2002/04/10 16:21:25 wiz Exp $

  testdec.c -- command line test program
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

#include "decode.h"
#include "header.h"
#include "mime.h"
#include "stream_types.h"
#include "util.h"

char *prg;

int
main(int argc, char *argv[])
{
    stream *stm, *st2;
    out_state *out;
    int ret;
    token t;

    prg = argv[0];

    prdebug_init(DEBUG_ALL^(DEBUG_SUBJ|DEBUG_DEBUG),
		 DEBUG_ALL^(DEBUG_LINE|DEBUG_SUBJ|DEBUG_PART|DEBUG_TOK));

    header_init();
    mime_init();

    stm = stream_fcat_open(argc-1, argv+1);
    st2 = stream_article_open(stm);
    out = output_new();

    ret = decode(st2, out) ? 1 : 0;

    output(out, token_set(&t, TOK_EOP, NULL));

    stream_close(st2);
    stream_close(stm);

    output_free(out);

    exit(ret);
}
