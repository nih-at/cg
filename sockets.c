/*
  $NiH: sockets.c,v 1.3 2002/04/10 16:23:31 wiz Exp $

  sockets.c -- auxiliary socket functions
  Copyright (C) 1996, 1997 Dieter Baron

  This file is part of cftp, a fullscreen ftp client.
  The author can be contacted at <dillo@giga.or.at>

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
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#include "config.h"

extern char *prg;

#ifndef H_ERRNO_DECLARED
extern int h_errno;
#endif

#ifndef HAVE_HSTRERROR
char *hstrerror(int);
#endif



int
sopen(char *host, char *service)
{
    int s;
    struct hostent *hp;
    struct sockaddr_in sa;
    u_short port;
    struct servent *serv;

    if ((serv = getservbyname(service, "tcp")) == NULL) {
	if ((port=atoi(service)) == 0 && service[0] != '0') {
	    fprintf(stderr, "%s: unknown service: %s\n",
		    prg, service);
	    return -1;
	}
	else
	    port = htons(port);
    }
    else
	port = (u_short)serv->s_port;

    if ((hp = gethostbyname(host)) == NULL) {
	fprintf(stderr, "%s: can't get host %s: %s\n",
		prg, host, hstrerror(h_errno));
	return -1;
    }
    memcpy(&sa.sin_addr, hp->h_addr, hp->h_length);
    sa.sin_family = hp->h_addrtype;
    sa.sin_port = port;
    if ((s = socket(hp->h_addrtype, SOCK_STREAM, 0)) < 0) {
	fprintf(stderr, "%s: can't allocate socket: %s\n",
		prg, strerror(errno));
	return -1;
    }
    if (connect(s, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
	fprintf(stderr, "%s: can't connect: %s\n",
		prg, strerror(errno));
	return -1;
    }

    return s;
}
