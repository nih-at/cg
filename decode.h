#ifndef _HAD_DECODE_H
#define _HAD_DECODE_H

/*
  $NiH: decode.h,v 1.11 2002/04/10 16:23:27 wiz Exp $

  decode.h -- main decode logic
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

#include "stream.h"
#include "output.h"

enum enctype {
    enc_unknown, enc_error, enc_eof, enc_nodata,
    enc_uu, enc_binhex, enc_base64
};

#define DEC_ERRMASK	~0xfff
#define DEC_EOF		0x1000
#define DEC_INCOMPLETE	0x2000
#define DEC_ILLEGAL	0x4000

#define DEC_ILL		-1
#define DEC_END		-2
#define DEC_PAD		-3

extern int decode_table_base64[256];
extern int decode_table_uuencode[256];
extern int decode_table_binhex[256];



int decode(stream *in, out_state *out);

#endif /* decode.h */
