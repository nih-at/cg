/*
  $NiH: getaddrinfo.c,v 1.10 2001/12/11 14:37:33 dillo Exp $

  getaddrinfo -- nodename-to-address translation in protocol-independent manner
  Copyright (C) 2000, 2001 Dieter Baron

  This file is a replacement for the following library functions:
    getaddrinfo, freeaddrinfo, gai_strerror.
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



#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>	
#include <netdb.h>
#include <stdlib.h>

#include "config.h"
#include "sockets.h"

#ifndef HAVE_DECL_H_ERRNO
extern int h_errno;
#endif



int
getaddrinfo(const char *nodename, const char *servname,
	    const struct addrinfo *hints, struct addrinfo **res)
{
    struct addrinfo *ai, *airet, *aiprev;
    struct hostent *hp;
    struct sockaddr_in sa;
    struct servent *serv;
    u_short port;
    char addr[4], *s_addr_list[2], **addr_list;
    int addr_len;
    int i;

    /* XXX: check arguments */
    /* XXX: get proto */

    if (hints && hints->ai_family != PF_UNSPEC
	&& hints->ai_family != PF_INET)
	return EAI_FAMILY;

    if ((serv = getservbyname(servname, "tcp")) == NULL) {
	if ((port=atoi(servname))==0
	    && (servname[0]!='0' || servname[1]!='\0')) {
	    return EAI_SERVICE;
	}
	port = htons(port);
    }
    else
	port = (u_short)serv->s_port;

    sa.sin_family = AF_INET;
    sa.sin_port = port;
    memset(sa.sin_zero, 0, 8);

    addr_list = s_addr_list;
    addr_list[0] = addr;
    addr_list[1] = NULL;
    addr_len = 4;

    if (hints->ai_flags & AI_PASSIVE && nodename == NULL) {
	sa.sin_addr.s_addr = INADDR_ANY;
	memcpy(addr_list[0], &sa.sin_addr.s_addr, addr_len);
    }
    else if (inet_aton(nodename, &sa.sin_addr) == 1) {
	memcpy(addr_list[0], &sa.sin_addr.s_addr, addr_len);
    }
    else {
	if ((hp = gethostbyname(nodename)) == NULL) {
	    switch (h_errno) {
	    case TRY_AGAIN:
		return EAI_AGAIN;
	    case HOST_NOT_FOUND:
	    case NO_DATA:
		return EAI_NODATA;
	    case NO_RECOVERY:
	    default:
		return EAI_FAIL;
	    }
	}
	addr_list = hp->h_addr_list;
	addr_len = hp->h_length;
    }

    airet = aiprev = NULL;
    for (i=0; addr_list[i]; i++) {
	if ((ai=malloc(sizeof(struct addrinfo))) == NULL) {
	    freeaddrinfo(airet);
	    return EAI_MEMORY;
	}
	if (airet == NULL)
	    airet = ai;
	if (aiprev)
	    aiprev->ai_next = ai;
	aiprev = ai;

	ai->ai_next = NULL;
	ai->ai_family = AF_INET;
	ai->ai_socktype = SOCK_STREAM; /* XXX: from hints */
	ai->ai_protocol = IPPROTO_TCP; /* XXX: from hints */
	memcpy(&sa.sin_addr, addr_list[i], addr_len);
	if ((ai->ai_addr=malloc(sizeof(struct sockaddr_in))) == NULL) {
	    freeaddrinfo(airet);
	    return EAI_MEMORY;
	}
	memcpy(ai->ai_addr, &sa, sizeof(struct sockaddr_in));
	ai->ai_addrlen = sizeof(struct sockaddr_in);
    }

    *res = airet;
    return 0;
}



void
freeaddrinfo(struct addrinfo *ai)
{
    struct addrinfo *ai2;

    while (ai) {
	ai2 = ai->ai_next;
	free(ai->ai_addr);
	free(ai);
	ai = ai2;
    }
}



char *
gai_strerror(int ecode)
{
    static char *estr[] = {
	"Success",
	"Address family for hostname not supported",
	"Temporary failure in name resolution",
	"Invalid value for ai_flags",
	"Non-recoverable failure in name resolution",
	"ai_family not supported",
	"Memory allocation failure",
	"No address associated with hostname",
	"hostname nor servname provided, or not known",
	"servname not supported for ai_socktype",
	"ai_socktype not supported",
	"System error returned in errno"
    };
    static const int nestr = sizeof(estr)/sizeof(estr[0]);

    return (int)ecode < nestr ? estr[ecode] : "Unknown error";
}
