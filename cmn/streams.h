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

// streams.h

#ifndef STREAMS_H
#define STREAMS_H

#if X11
	#ifndef NO_PRAGMAS
	#pragma interface
	#endif
#endif

#include "utils.h"

#define UDP_STREAM_HEADER_LEN 12    // <XETP::versionStr:8> + <checksum:4>
#define UDP_STREAM_BODY_LEN 5000 //2044  should make this some sort of growable array
#define UDP_STREAM_BUFFER_LEN (UDP_STREAM_HEADER_LEN + UDP_STREAM_BODY_LEN)

typedef u_int Checksum;

class GenericStream {
public:
  enum {TCP,UDP};

  GenericStream(Boolean ownSock) {ownSocket = ownSock;}
  /* EFFECTS: ownSock means whether the stream owns the socket or not, i.e
     whether it will take responsibility for closing it when done. */

  virtual ~GenericStream();

  virtual int get_protocol() = 0;

  virtual Boolean alive() = 0;

  static Boolean get_buggy() {return buggy;}
  static void set_buggy(Boolean val) {buggy = val;}
  /* EFFECTS:  Simulate a bad network connection. */
  
  
#ifndef PROTECTED_IS_PUBLIC
protected:
#endif
  Boolean ownSocket;
  
  static Boolean buggy;

  Checksum compute_checksum(u_char *data,int len);
};



class InStream : public GenericStream {
public:
  InStream(Boolean ownSock) : GenericStream(ownSock) {}
  
  virtual Boolean read(void *buf,int size) = 0;
  /* EFFECTS: Read size number of bytes from buf.  Return True if
     successful. */

  virtual u_char read_char() = 0;

  virtual int read_signed_char() = 0;

  virtual u_short read_short() = 0;

  virtual int read_signed_short() = 0;

  virtual u_int read_int() = 0;

  virtual float read_float() = 0;
};
typedef InStream *InStreamP;



class OutStream : public GenericStream {
public:
  OutStream(Boolean ownSock) : GenericStream(ownSock) {}

  virtual Boolean write(void *buf,int size) = 0;
  /* EFFECTS: Write size number of bytes into buf.  Return True if
     successful. */

  virtual void write_char(u_char) = 0;
  
  virtual void write_signed_char(int) = 0;

  virtual void write_short(u_short) = 0;

  virtual void write_signed_short(int) = 0;

  virtual void write_int(u_int) = 0;

  virtual void write_float(float) = 0;
};
typedef OutStream *OutStreamP;



// Non-buffering stream to read from TCP socket.
class NetInStream : public InStream {
public:
  NetInStream(CMN_SOCKET sock,Boolean ownSocket);

  virtual ~NetInStream();
  /* NOTE: Closes the socket */

  virtual int get_protocol();

  virtual Boolean alive();
  
  virtual Boolean read(void *buf,int size);

  virtual u_char read_char();

  virtual int read_signed_char();

  virtual u_short read_short();

  virtual int read_signed_short();

  virtual u_int read_int();

  virtual float read_float();

  static int get_bytes_in() {return bytesIn;}
  static void reset_counter() {bytesIn = 0;}
  /* NOTE: Simple statistics. */


#ifndef PROTECTED_IS_PUBLIC
protected:
#endif
  Boolean isAlive;
  CMN_SOCKET sock;

  static int bytesIn;
};



// Non-buffering stream to write to TCP socket.
class NetOutStream : public OutStream {
public:
  NetOutStream(CMN_SOCKET sock,Boolean ownSocket);

  virtual ~NetOutStream();
  /* NOTE: Closes the socket */
  
  virtual int get_protocol();

  virtual Boolean alive();
  
  virtual Boolean write(void *buf,int size);

  virtual void write_char(u_char);

  virtual void write_signed_char(int);

  virtual void write_short(u_short);

  virtual void write_signed_short(int);

  virtual void write_int(u_int);

  virtual void write_float(float);

  static int get_bytes_out() {return bytesOut;}
  static void reset_counter() {bytesOut = 0;}
  /* NOTE: Simple statistics. */


#ifndef PROTECTED_IS_PUBLIC
protected:
#endif
  Boolean isAlive;
  CMN_SOCKET sock;

  static int bytesOut;
};



class UDPInStream : public NetInStream {
public:
  UDPInStream(CMN_SOCKET sock,Boolean ownSocket);

  virtual ~UDPInStream();

  virtual int get_protocol();

  virtual int prepare_packet(CMN_SOCKADDR_IN *address);
  /* MODIFIES: address */
  /* EFFECTS: Sets the IP address of the UDP packet coming in, return the length of
     the data.  Return -1 if failure. */
  /* NOTE: This one UDP packet may have multiple XETP methods. */

  virtual void done_packet();

  virtual Boolean read(void *buf,int size);

  int bytes_remaining() {return bufLen - bufPtr;}
  /* EFFECTS: Number of bytes that can still be read from this UDP packet. */

  void revive() {isAlive = True;}
  /* EFFECTS: Bring this stream back to life. */

  static int get_bytes_in() {return bytesIn;}
  static void reset_counter() {bytesIn = 0;}
  /* NOTE: Simple statistics. */


private:
  u_char buffer[UDP_STREAM_BUFFER_LEN];
  int bufPtr; // Current location reading in buffer.
  int bufLen;

  static int bytesIn;
};
typedef UDPInStream *UDPInStreamP;



class UDPOutStream : public NetOutStream {
public:
  UDPOutStream(CMN_SOCKET sock,CMN_SOCKADDR_IN *,Boolean ownSocket);
  /* NOTE: Shares the sockaddr_in struct that is passed in. */

  virtual ~UDPOutStream();

  virtual int get_protocol();

  virtual void prepare_packet(int);
  /* NOTE: Should really be called prepare_method(), since caller is about to
     write a single XETP method, not an entire UDP packet. */

  virtual void done_packet();
  /* NOTE: Should be called done_method(). */

  virtual Boolean write(void *buf,int size);

  void flush();
  /* EFFECTS: Force the pending UDP packet to be sent. */

  static float get_average_length_out() 
  { return (float)totalBytesOut / (float)packetCount;}
  static int get_bytes_out() {return bytesOut;}
  static void reset_counter() {bytesOut = 0;}
  /* NOTE: Simple statistics. */


private:
  int buggy_tests();
  /* EFFECTS: Possibly modify data to run tests.  Return the number of
     times to send the data in buffer. */

  void create_header();
  /* MODIFIES: header */
  /* EFFECTS: Compute checksum of data in body and put in header. */

  CMN_SOCKADDR_IN *address;
  u_char buffer[UDP_STREAM_BUFFER_LEN];
  int bufPtr; // Current location writing to buffer.
  int bufLen; // length of the packet to be written

  static int bytesOut;

  // For average packet length
  static int totalBytesOut;
  static int packetCount;
};
typedef UDPOutStream *UDPOutStreamP;

#endif
