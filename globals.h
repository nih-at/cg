#ifndef _HAD_GLOBALS_H
#define _HAD_GLOBALS_H

#include "ranges.h"

extern char *prg;                  /* program name */
extern char *newsrc;               /* newsrc file name */
extern struct range *rcmap;        /* bitmap of read articles */

extern int mark_complete;          /* mark complete binary postings as read */

#endif /* globals.h */
