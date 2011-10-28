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

// "streams.cpp"

#if X11
	#ifndef NO_PRAGMAS
	#pragma implementation "streams.h"
	#endif
#endif
#include "stdafx.h"

#include "utils.h"
#include "neth.h" // needed for recv, send, recvfrom, sendto

#include <iostream.h>

#include "streams.h"
#include "xetp.h" // yuck, need this for XETP::versionStr



Checksum GenericStream::compute_checksum(u_char *data,int len) {
  Checksum c = 0;
  for (int n = 0; n < len; n++) {
    u_int byte = data[n];
    c += byte;
  }
  return c;
}



Boolean GenericStream::buggy = False;



GenericStream::~GenericStream() {
}



NetInStream::NetInStream(CMN_SOCKET s,Boolean ownSocket)
: InStream(ownSocket) {
  sock = s;
  isAlive = True;
}



NetInStream::~NetInStream() {
  if (ownSocket) {
    CLOSE_SOCKET(sock);
  }
}


int NetInStream::get_protocol() {
  return TCP;
}


    
Boolean NetInStream::alive() {
  return isAlive;
}
  


Boolean NetInStream::read(void *buf,int size) {
  if (!isAlive) {
    return False;
  }

  //  ssize_t bytesRead = ::read(sock,buf,size);
  int offset = 0;
  while(offset < size) {
    int bytesRead = (int)::recv(sock,(char *)buf + offset,size - offset,0);
    if (bytesRead <= 0) {
      isAlive = False;
      return False;
    }
    offset += bytesRead;
  }

  // Keep stats.
  bytesIn += size;

  return True;
}



u_char NetInStream::read_char() {
  u_char byte;
  if (read(&byte,1)) {
    return byte;
  }
  else {
    return 0;
  }
}


int NetInStream::read_signed_char() {
  u_char val = read_char();
  signed char val2 = (signed char)val;
  return val2;
}



u_short NetInStream::read_short() {
  u_char bytes[2];
  if (read(bytes,2)) {
    u_short ret;
    ret = (((u_short)bytes[0]) << 8) | (((u_short)bytes[1]) << 0);
    return ret;
  }
  else {
    return 0;
  }
}



int NetInStream::read_signed_short() {
  u_short val = read_short();
  signed short val2 = (signed short)val;
  return val2;
}



u_int NetInStream::read_int() {
  u_char bytes[4];
  if (read(bytes,4)) {
    u_int ret;
    ret = (((u_int)bytes[0]) << 24) | (((u_int)bytes[1]) << 16) | 
      (((u_int)bytes[2]) << 8) | (((u_int)bytes[3]) << 0);
    return ret;
  }
  else {
    return 0;
  }
}



float NetInStream::read_float() {
  u_int intVal = read_int();
  float *fp = (float *)&intVal;
  return *fp;
}



int NetInStream::bytesIn = 0;



NetOutStream::NetOutStream(CMN_SOCKET s,Boolean ownSocket) 
: OutStream(ownSocket) {
  sock = s;
  isAlive = True;
}



NetOutStream::~NetOutStream() {
  if (ownSocket) {
    CLOSE_SOCKET(sock);
  }
}



int NetOutStream::get_protocol() {
  return TCP;
}


    
Boolean NetOutStream::alive() {
  return isAlive;
}
  


Boolean NetOutStream::write(void *buf,int size) {
  if (!isAlive) {
    return False;
  }

  //  ssize_t bytesWritten = ::write(sock,buf,size);
  int bytesWritten = (int)::send(sock,(char *)buf,size,0);
  if (bytesWritten != size) {
    isAlive = False;
    return False;
  }

  // Keep stats.
  bytesOut += bytesWritten;

  return True;
}



void NetOutStream::write_char(u_char byte) {
  write(&byte,1);
}



void NetOutStream::write_signed_char(int val) {
  signed char val2 = (signed char)val;
  assert(val == val2);  // Check that data wasn't lost.
  write_char(val2);
}



void NetOutStream::write_short(u_short val) {
  u_char buf[2];
  buf[0] = (u_char)((val & 0xff00) >> 8);
  buf[1] = (u_char)((val & 0x00ff) >> 0);
  write(buf,2);
}



void NetOutStream::write_signed_short(int val) {
  signed short val2 = (signed short)val;
  assert(val == val2);  // Check that data wasn't lost.
  write_short(val2);
}



void NetOutStream::write_int(u_int val) {
  u_char buf[4];
  buf[0] = (u_char)((val & 0xff000000) >> 24);
  buf[1] = (u_char)((val & 0x00ff0000) >> 16);
  buf[2] = (u_char)((val & 0x0000ff00) >> 8);
  buf[3] = (u_char)((val & 0x000000ff) >> 0);
  write(buf,4);
}



void NetOutStream::write_float(float val) {
  u_int *ip = (u_int *)&val;
  u_int i = *ip;
  write_int(i);
}



int NetOutStream::bytesOut = 0;



UDPInStream::UDPInStream(CMN_SOCKET sock,Boolean ownSocket)
: NetInStream(sock,ownSocket) {
  bufPtr = 0;
  bufLen = 0;
}



UDPInStream::~UDPInStream() {
}



int UDPInStream::get_protocol() {
  return UDP;
}


    
int UDPInStream::prepare_packet(CMN_SOCKADDR_IN *address) {
  // Just to be sure.
  bufLen = 0;
  bufPtr = 0;

  if (!isAlive) {
    return -1;
  }

  CMN_NET_LENGTH addrSize = sizeof(CMN_SOCKADDR_IN);

  // Ignore who it came from for now.
  int n = recvfrom(sock,(char *)buffer,UDP_STREAM_BUFFER_LEN,0,
                   (CMN_SOCKADDR *)address,&addrSize);
  // <= because we never send packets with no data
  if (n <= UDP_STREAM_HEADER_LEN) {  // also checks for < 0
    // For now, don't kill stream.
    //    isAlive = False;
    return -1;
  }

  if (addrSize != sizeof(CMN_SOCKADDR_IN)) {
    cerr << "UDPInStream received invalid size socket address." << endl;
  }

  // Keep stats.
  bytesIn += n;

  // Point to received buffer before the header.
  bufLen = n;
  bufPtr = 0;


  // Check that XETP::versionStr matches
  char data[XETP::VERSION_LENGTH];
  if (!read((u_char *)data,XETP::VERSION_LENGTH)) {
    return -1;
  }

  if (strncmp(data,XETP::versionStr,XETP::VERSION_LENGTH)) {
    // null terminate string.
    char recvdVersion[XETP::VERSION_LENGTH + 1];
    memcpy(recvdVersion,data,XETP::VERSION_LENGTH);
    recvdVersion[XETP::VERSION_LENGTH] = '\0';

    // This error message should really be Role::error().
    cerr << "Received packet with the wrong XETP version." << endl
      << "Expected " << XETP::versionStr << " but received " << recvdVersion
      << endl;
    return False;
  }


  Checksum d = (Checksum)read_int();
  // Now pointing after the checksum.
  
  // Check that checksum matches
  Checksum c = 
    compute_checksum(buffer + UDP_STREAM_HEADER_LEN,n - UDP_STREAM_HEADER_LEN);
  if (c != d) {
    cerr << "UDPInStream checksum does not match, packet gave " << d 
         << " computed value of " << c << endl;
    return -1;
  }

  // Return length of the body.  (not tested)
  return bufLen - UDP_STREAM_HEADER_LEN;
}



void UDPInStream::done_packet() {
  if (!isAlive) {
    return;
  }  

#if 0
  if (bufPtr != bufLen) {
    cerr << "Warning: discarding packet info when only " << bufPtr <<
      " bytes of " << bufLen << " has been read." << endl;
  }
#endif

  bufLen = 0;
  bufPtr = 0;
}



Boolean UDPInStream::read(void *buf,int size) {
  if (!isAlive) {
    return False;
  }

  if (bufPtr + size <= bufLen) {
    memcpy(buf,buffer + bufPtr,size);
    bufPtr += size;
    return True;
  }

  cerr << "ERROR: Trying to read more data than UDP packet contains." << endl;
  //  isAlive = False;

  return False;
}



int UDPInStream::bytesIn = 0;



UDPOutStream::UDPOutStream(CMN_SOCKET sock,CMN_SOCKADDR_IN *addr,
                           Boolean ownSocket)
:  NetOutStream(sock,ownSocket) {
  address = addr;

  // Leave some room for the header.
  bufPtr = UDP_STREAM_HEADER_LEN;
  bufLen = UDP_STREAM_HEADER_LEN;
  // Initialize with meaningless data
  for (int n = 0; n < UDP_STREAM_HEADER_LEN; n++) {
    buffer[n] = (u_char)'@';
  }
}



UDPOutStream::~UDPOutStream() {
  flush();
}



int UDPOutStream::get_protocol() {
  return UDP;
}


    
void UDPOutStream::prepare_packet(int size) 
{
  assert(size < UDP_STREAM_BODY_LEN);
  if (!isAlive) {
    return;
  }
  
  // Send packet when buffer is full.
  // bufLen includes the header length.
  if (bufLen + size > UDP_STREAM_BUFFER_LEN) {
    // Will modify bufLen and bufPtr
    flush();
  }

  // Remove any allocated, but unused space.
  bufLen = bufPtr;

  // Allocate more space.
  bufLen += size;

  // Keep average packet length;
  totalBytesOut += size;
  packetCount++;
}



void UDPOutStream::done_packet() {
  if (bufPtr < bufLen) {
    cerr << "Warning: UDPOutStream only writing " << bufPtr 
         << " bytes when set for "
         << bufLen << endl;
  }
}



void UDPOutStream::flush() {
  if (!isAlive) {
    return;
  }
  // Write bufPtr bytes, not bufLen.

  // Only send if some data has been written to the stream.
  if (bufPtr > UDP_STREAM_HEADER_LEN) {
//    cout << "Flushing " << bufPtr << " bytes." << endl;

    // Wait until we are about to send before creating the header, so we 
    // can compute the checksum.
    create_header();

    // Careful to do this after computing the checksum in create_header.
    int count = buggy_tests();

    int n;
    for (n = 0; n < count; n++) {
      if (bufPtr != sendto(sock,(char *)buffer,bufPtr,0,
                           (CMN_SOCKADDR *)address,
                           sizeof(CMN_SOCKADDR_IN))) {
        cerr << "Failed to send UDP packet." << endl;
        // Don't kill it for now.
        // isAlive = False;
      }
    }

    // Keep stats.
    bytesOut += bufPtr;
  }
  
  // Leave some room for the next header.
  bufPtr = UDP_STREAM_HEADER_LEN;
  bufLen = UDP_STREAM_HEADER_LEN;
  // Initialize with meaningless data
  for (int n = 0; n < UDP_STREAM_HEADER_LEN; n++) {
    buffer[n] = (u_char)'@';
  }
}



Boolean UDPOutStream::write(void *buf,int size) {
  if (!isAlive) {
    return False;
  }

  if (bufPtr + size <= bufLen) {
    memcpy(buffer + bufPtr,buf,size);
    bufPtr += size;
    return True;
  }

  cerr << "Writing more data to UDPOutStream than amount specified." << endl;
  // isAlive = False;    
  return False;
}



int UDPOutStream::buggy_tests() {
  int count = 1;
  if (buggy) {
    // Randomly drop a packet.
    if (Utils::choose(2) == 0) {
      cerr << "Test: Drop a packet." << endl;
      count = 0;
    }
    // Duplicate packet
    else if (Utils::choose(30) == 0) {
      count = 2 + Utils::choose(5);
      cerr << "Testing: Send " << count << " copies of a packet." << endl;
    }
    // Send random packet of random length
    else if (Utils::choose(100) == 0) {
      bufPtr = Utils::choose(UDP_STREAM_BUFFER_LEN) + 1;
      for (int n = 0; n < bufPtr; n++) {
        buffer[n] = (u_char)Utils::choose(256);
      }
      cerr << "Testing: Send garbage packet of length " << bufPtr << endl;
    }
    // Send truncated packet
    else if (Utils::choose(100) == 0) {
      int oldLen = bufPtr;
      if (bufPtr > 0) {
        bufPtr = Utils::choose(bufPtr);
      }
      cerr << "Testing: Truncate " << oldLen << " byte packet to " 
           << bufPtr << " bytes" << endl;
    }
    // Send packet with extra crap at end
    else if (Utils::choose(100) == 0) {
      int extraLen = Utils::choose(20) + 1;
      int newLen = bufPtr + extraLen;
      if (newLen <= UDP_STREAM_BUFFER_LEN) {
        for (; bufPtr < newLen; bufPtr++) {
          buffer[bufPtr] = (u_char)Utils::choose(256);
        }
      }
      cerr << "Testing: Add " << extraLen 
           << " bytes to a packet formerly of length "
           << (newLen - extraLen) << endl;
    }
    // Randomly twiddle a few bytes.
    else if (Utils::choose(100) == 0) {
      if (bufPtr > 0) {
        int changes = Utils::choose(5) + 1;
        for (int n = 0; n < changes; n++) {
          int which = Utils::choose(bufPtr);
          buffer[which] = (u_char)Utils::choose(256);
        }
        cerr << "Testing: Randomly change " << changes
             << " bytes in a packet." << endl;
      }
    }
  }
  return count;
}



void UDPOutStream::create_header() {
  // Check that we didn't accidentally write data into the header's space.
  if (!buggy) { // in buggy mode we might want to write garbage.
    int n;
    for (n = 0; n < UDP_STREAM_HEADER_LEN; n++) {
      assert(buffer[n] == (u_char)'@');
    }
  }

  // Copy checksum into the header bytes.
  Checksum c = compute_checksum(buffer + UDP_STREAM_HEADER_LEN,bufPtr - UDP_STREAM_HEADER_LEN);

  // Little funny here, moving bufPtr back to the beginning to write the header bytes.
  // then put it back at the end.
  int bufPtrSave = bufPtr;
  bufPtr = 0;
  write(XETP::versionStr,XETP::VERSION_LENGTH);
  write_int((u_int)c);
  bufPtr = bufPtrSave;
}



int UDPOutStream::bytesOut = 0;



int UDPOutStream::totalBytesOut = 0;



int UDPOutStream::packetCount = 0;
