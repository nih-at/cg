#ifndef _HAD_SYMBOL_H
#define _HAD_SYMBOL_H

/*
  $NiH: symbol.h,v 1.5 2002/04/10 16:23:38 wiz Exp $

  symbol.h -- simple symbol table
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

typedef char *symbol;

symbol intern(char *);
symbol intern_lower(char *);
symbol intern_caps(char *);

#endif /* symbol.h */
