#ifndef _HAD_STREAM_TYPES_H
#define _HAD_STREAM_TYPES_H

/*
  $NiH: stream_types.h,v 1.5 2002/04/10 16:21:23 wiz Exp $

  stream_types.h -- stream interface
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

#include "stream.h"
#include "checkgroup.h"
#include "mime.h"

stream *stream_article_open(struct stream *source);
stream *stream_cat_open(struct file *file);
stream *stream_decode_open(stream *source, int *tbl);
stream *stream_fcat_open(int n, char **names);
stream *stream_file_open(FILE *f, int is_nntp);
stream *stream_msg_partial_open(stream *source, struct mime_hdr *m);
stream *stream_msg_multi_open(stream *source, struct mime_hdr *m);
stream *stream_quot_print_open(stream *source);
stream *stream_section_open(stream *source, char *boundary);
stream *stream_uuextract_open(stream *source);
stream *stream_yenc_open(struct stream *source, char *ybegin);


#endif /* stream_types.h */
