#ifndef _HAD_CHECKGROUP_H
#define _HAD_CHECKGROUP_H

/*
  $NiH: checkgroup.h,v 1.5 2002/04/10 16:23:26 wiz Exp $

  checkgroup.h -- global header
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

struct file {
    char *tag;
    char *comment;
    int npart;
    long *artno;
    int new;
    long size;
};



extern FILE *conin, *conout;



int nntp_put(char *fmt, ...);
int nntp_resp(void);

#endif /* checkgroup.h */
