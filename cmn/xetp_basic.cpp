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

// "xetp_basic.cpp"

#if X11
	#ifndef NO_PRAGMAS
	#pragma implementation "xetp_basic.h"
	#endif
#endif

#include "stdafx.h"


#include "xetp_basic.h"


char *XETPBasic::versionStr = "XETP1.0X";
// 0.10 released with special RedHat version
// 0.12 for 2.0b5
// 0.15 for 2.0b6
// 0.16 for 2.0b7
// 1.00 for 2.0, 2.01, 2.02



void XETPBasic::check_sizes() {
  // If this ever fails, we'll have to put in a bunch of cross-platform 
  // typedefs.
  assert(sizeof(int) == 4);
  assert(sizeof(short) == 2);
  assert(sizeof(char) == 1);
  assert(sizeof(float) == 4);
  assert(sizeof(u_int) == 4);
  assert(sizeof(u_short) == 2);
  assert(sizeof(u_char) == 1);

  // If we get to 256, need more than one byte to determine the method.
  assert(XETP_METHOD_MAX <= 256);
}



Boolean XETPBasic::receive_header(InStreamP inStream,
                             u_short &method,u_int &length) {
  method = (u_short)inStream->read_char();
  length = (u_int)inStream->read_short();

  return inStream->alive();
}



void XETPBasic::send_header(OutStreamP out,u_short method,u_int length) {
  // <method:1><length:4>

  assert(method <= UCHAR_MAX);
  out->write_char((u_char)method);
  assert(length <= USHRT_MAX);
  out->write_short((u_short)length);
}



void XETPBasic::send_generic(OutStreamP out,u_short method) {
  if (out->get_protocol() == GenericStream::UDP) {
    ((UDPOutStreamP)out)->prepare_packet(XETPBasic::add_header(0));
  }
  send_header(out,method,0);
  if (out->get_protocol() == GenericStream::UDP) {
    ((UDPOutStreamP)out)->done_packet();
  }
}



void XETPBasic::send_int(OutStreamP out,u_short method,u_int val) {
  int len = sizeof(int);
  if (out->get_protocol() == GenericStream::UDP) {
    ((UDPOutStreamP)out)->prepare_packet(XETPBasic::add_header(len));
  }
  send_header(out,method,len);
  out->write_int(val);
  if (out->get_protocol() == GenericStream::UDP) {
    ((UDPOutStreamP)out)->done_packet();
  }
}



void XETPBasic::send_string(OutStreamP out,u_short method,char *msg) {
  assert(msg);
  int len = Utils::get_string_write_length(msg);
  if (out->get_protocol() == GenericStream::UDP) {
    ((UDPOutStreamP)out)->prepare_packet(XETPBasic::add_header(len));
  }
  send_header(out,method,len);
  Utils::string_write(out,msg);
  if (out->get_protocol() == GenericStream::UDP) {
    ((UDPOutStreamP)out)->done_packet();
  }
}



const CMN_PORT XETPBasic::DEFAULT_PORT = 6066;
