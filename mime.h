#ifndef _HAD_MIME_H
#define _HAD_MIME_H

/*
  $NiH: mime.h,v 1.9 2002/04/10 16:21:16 wiz Exp $

  mime.h -- MIME header parsing
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

#include "symbol.h"

struct mime_hdr {
    symbol type;
    struct mime_opt *option;
};

struct mime_opt {
    symbol name;
    char *value;
};



#define MIME_CT_MSG_PART	mime_sym[0]
#define MIME_CT_MSG_MULTI	mime_sym[1]
#define MIME_CT_MULTI_MIX	mime_sym[2]
#define MIME_TE_7BIT		mime_sym[3]
#define MIME_TE_8BIT		mime_sym[4]
#define MIME_TE_BASE64		mime_sym[5]
#define MIME_TE_QUOT_PRINT	mime_sym[6]
#define MIME_TE_X_UUENCODE	mime_sym[7]
#define MIME_CD_FILENAME	mime_sym[8]
#define MIME_CT_NAME		mime_sym[9]
#define MIME_CT_BOUNDARY	mime_sym[10]
#define MIME_CT_ID		mime_sym[11]
#define MIME_CT_NUMBER		mime_sym[12]
#define MIME_CT_TOTAL		mime_sym[13]

extern symbol mime_sym[];



void mime_init(void);
struct mime_hdr *mime_parse(char *header);
void mime_free(struct mime_hdr *m);
char *mime_option_get(struct mime_hdr *m, symbol name);

#endif /* mime.h */
