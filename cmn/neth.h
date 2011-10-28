/* 
 * XEvil(TM) Copyright (C) 1994,2000 Steve Hardt and Michael Judge
 * http://www.xevil.com
 * satan@xevil.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program, the file "gpl.txt"; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA, or visit http://www.gnu.org.
 */

// "role.cpp"


#ifndef NETH_H
#define NETH_H

#include "utils.h"

// Just used to get all the network headers.
// Should only be included by .cpp files.

#if X11
#include <unistd.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/signal.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

// For recvfrom() and accept, fucking AIX 2.4
#ifdef USE_ULONG_NET_LENGTH
typedef unsigned long CMN_NET_LENGTH;
// Red Hat Linux likes unsigned int
#elif defined USE_UINT_NET_LENGTH
typedef unsigned int CMN_NET_LENGTH;
#else
typedef int CMN_NET_LENGTH;
#endif


#if X11
#ifdef SELECT_NEEDS_PROTOTYPES
extern "C" {
int select(int,
	   fd_set *,
	   fd_set *,
	   fd_set *,
	   struct timeval *);
}
#endif

#ifdef USE_SELECT_H
extern "C" {
#include <sys/select.h>
}
#endif

#endif


#if WIN32
#include <sys/utime.h>
#include <signal.h>
#endif


// For Solaris 8
#ifndef INADDR_NONE
#define INADDR_NONE -1
#endif


// Stuff that might be useful
#if 0
struct sockaddr_in {
        short   sin_family;
        u_short sin_port;
        struct  in_addr sin_addr;
        char    sin_zero[8];
};
#endif



#ifdef SELECT_TAKES_INTP
  #define CMN_SELECT(sockNum,readSock,writeSock,exceptSock,timeout) \
  select(sockNum,((int *)readSock),((int *)writeSock),((int *)exceptSock),timeout)
#else
  #define CMN_SELECT(sockNum,readSock,writeSock,exceptSock,timeout) \
  select(sockNum,readSock,writeSock,exceptSock,timeout)
#endif



#if X11
#define CLOSE_SOCKET(sck) close(sck)
#endif
#if WIN32
#define CLOSE_SOCKET(sck) closesocket(sck)
#endif

#endif

