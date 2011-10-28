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

// "xetp_basic.h"

// A restricted subset of XETP.  Only contains simple methods that require
// no information about Physical, Intel, World, Locator, or any other 
// major XEvil module.  xetp_basic.h is used for utility programs dealing 
// with XETP.


#ifndef XETP_BASIC_H
#define XETP_BASIC_H

#if X11
	#ifndef NO_PRAGMAS
	#pragma interface
	#endif
#endif

#include "utils.h"
#include "streams.h"


class XETPBasic {
public:
  // All methods are proceeded by header of form  
  // <method:1><length:2>
  enum {HEADER_LENGTH = 3}; 

  enum {VERSION_LENGTH = 8};  // streams.cpp depends on this value.
  static char *versionStr;

  // All values are big-endian
  // The ':' syntax gives the size in bytes of each component.  :String means
  // use Utils::string_read(), Utils::string_write(), etc.

  // Methods for XETP
  enum {
#if 0
    CONNECT = 1,     // null
#endif

    TCP_CONNECT = 1,       // <udpPort:2><humanName:String>
                       // <ViewportInfo><skip:4><wantSounds:1>

    // Human must be the same as human after the <IntelId>
    HUMAN,             // <IntelId><human data>

    OBJECT,            // <ClassId:2><Id><Turn:4><TickType:1><object data>

    NEW_TURN,          // null

    NEW_WORLD,         // <worldVersion:4><world data>

    // Start a new game.
    RESET,             // <gameStyleType:4>

    MESSAGE,           // <message>

    ARENA_MESSAGE,     // <time(ms):4><message>

    COMMAND,           // <IntelId><command:1>

    DELETE_OBJECT,     // <Id>

    // Server asking Client to confirm existence
    PING,              // null

    // Client responding to PING
    // Also sent by client everytime it sees a new maximum turn.
    PONG,              // null

    TIMER_PING,        // <turn:4>

    TIMER_PONG,        // <ping_turn:4><local_turn:4>

    // Server explicitly killing Client connection.
    DISCONNECT,        // null

    // Server informing the client of some UI info.
    HUMANS_PLAYING,    // <int>

    // Server informing the client of some UI info.
    ENEMIES_PLAYING,   // <int>

    // Client is begging to start a new game with a new human.
    REQUEST_NEW_HUMAN, // null

    // Server tells client to play a sound effect.
    SOUND_REQUEST,     // <SoundRequest>

    // Ask a server to give information about itself.
    SERVER_PING,       // null

    // Server responding to a SERVER_PING.
    // human_kills is the value after taking soups into account.
    SERVER_PONG,       // <game_style:1><enemies_num:4><humans_num:2>
                       // <version:String>
                       // (humans_num * (<name><clientName><human_kills:4>
                       //                <enemy_kills:4><Id>))

    // A client asks the server to send a CHAT packet to the named receiver.
    // Send to everyone if receiverName is "".
    CHAT_REQUEST,      // <receiverName><message>

    // Server passing on a CHAT message to a specific client.
    // IntelId is that of the sender.
    CHAT,              // <senderName><IntelId><message>


    // Must be last.
    XETP_METHOD_MAX,
  };


  // The default port to use for XEvil servers.
  static const CMN_PORT DEFAULT_PORT;  

  static void check_sizes();
  /* EFFECTS: Runtime check that our assumptions about the sizes of 
     primitive data types are correct. */

  static int add_header(int bodyLen) {return HEADER_LENGTH + bodyLen;}
  /* EFFECTS: Returns the entire size of a packet to be sent. */


  ////////// SEND METHODS
  static void send_server_ping(OutStreamP out) 
  {send_generic(out,SERVER_PING);}


  ////////// RECEIVE METHODS
  static Boolean receive_header(InStreamP inStream,
                                u_short &method,u_int &length);
  /* EFFECTS: Look for XETP header coming in from inStream, 
     return method and length of body. */
  /* MODIFIES: method, length */
  /* NOTE: Does not call prepare_packet() or done_packet(). */


#ifndef PROTECTED_IS_PUBLIC
protected:
#endif
  static void send_header(OutStreamP,u_short method,u_int length);
  /* NOTE: Does not call prepare_packet() or done_packet(). */

  static void send_generic(OutStreamP,u_short method);
  /* EFFECTS: Send a method with no data. */

  static void send_int(OutStreamP,u_short method,u_int val);
  /* EFFECTS: Send a method with an integer data. */

  static void send_string(OutStreamP,u_short method,char *msg);
  /* EFFECTS: Send a method with String data. */
};

#endif
