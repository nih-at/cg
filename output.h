#ifndef _HAD_OUTPUT_H
#define _HAD_OUTPUT_H

/*
  $NiH: output.h,v 1.8 2002/04/10 16:21:17 wiz Exp $

  output.h -- output part of the decoder
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

#include "stream.h"

struct out_state {
    int broken;		/* is the file broken (decoding error and similar) */
    int infile;		/* are we currently writing a file */
    int warned;		/* have we warned about data outside a file */
    int ndata;		/* number of data consecutive tokens got */
    int do_fdesc;	/* fdesc state: FDESC_NOTYET: not opened,
			   FDESC_ERROR: given up, FDESC_OPEN: writing */
    char *fdescfilename;	/* name of previous description file */
    char tempdesc[64];	/* temporary name of description file */
    int fdescnl;	/* number of empty lines saved for description file */
    unsigned long size;	/* length of data written */
    FILE *fout;		/* file we're decoding into */
    FILE *fdesc;	/* description file */
    char *filename;	/* name of the output file */
};

typedef struct out_state out_state;



int output(out_state *out, token *t);
void output_free(out_state *out);
out_state *output_new();


#endif /* output.h */
