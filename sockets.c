/*
  $NiH: sockets.c,v 1.5 2002/04/17 17:46:29 dillo Exp $
  (cftp: NiH: sockets.c,v 1.24 2001/12/20 05:44:15 dillo Exp)

  sockets -- auxiliary socket functions
  Copyright (C) 1996, 1997, 1998, 1999, 2000, 2001 Dieter Baron

  This file is part of cftp, a fullscreen ftp client.
  The author can be contacted at <dillo@giga.or.at>.

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
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

#include "config.h"
#include "sockets.h"

extern char *prg;



int
sopen(char *host, char *service, int family)
{
    struct addrinfo hints, *res0, *res;
    int s, err;
    char *cause;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = family;
    hints.ai_socktype = SOCK_STREAM;
    
    if ((err=getaddrinfo(host, service, &hints, &res0)) != 0) {
	fprintf(stderr, "%s: cannot get host/service %s/%s: %s\n",
		prg, host, service, gai_strerror(err));
	return -1;
    }

    s = -1;
    for (res = res0; res; res = res->ai_next) {
	if ((s=socket(res->ai_family, res->ai_socktype,
		      res->ai_protocol)) < 0) {
	    cause = "create socket";
	    continue;
	}

	if (connect(s, res->ai_addr, res->ai_addrlen) < 0) {
	    cause = "connect";
	    close(s);
	    s = -1;
	    continue;
	}

	/* okay we got one */
	break;
    }
    if (s < 0)
	fprintf(stderr, "%s: cannot %s: %s\n",
		prg, cause, strerror(errno));

    freeaddrinfo(res0);

    return s;
}
