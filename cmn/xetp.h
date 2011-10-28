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

// "xetp.h"

#ifndef XETP_H
#define XETP_H

#if X11
	#ifndef NO_PRAGMAS
	#pragma interface
	#endif
#endif

#include "utils.h"
#include "coord.h"
#include "world.h"
#include "locator.h"
#include "streams.h"
#include "sound.h"
#include "intel.h"
#include "xetp_basic.h"


// Static class with info for reading/writing XETP (XEvil Transport Protocol)
class XETP: public XETPBasic {
public:
  static void check_sizes();
  /* EFFECTS: Runtime check that our assumptions about the sizes of 
     primitive data types are correct. */

  ////////// SEND METHODS
  static void send_tcp_connect(OutStreamP,u_short udpPort,char *humanName,
                               const ViewportInfo &,int skip,
                               Boolean wantSounds);
  /* EFFECTS: Send by client immediately after connecting to server to give
     startup information.  Connection is not complete until TCP_CONNECT
     received.  humanName should be empty string for default. */

  static void send_human(OutStreamP,HumanP);
  /* EFFECTS: Update human or tell the client which intelId it is 
     associated with. */

  static void send_object(OutStreamP,PhysicalP,Turn turn,TickType tt);

  static void send_objects(OutStreamP,const PtrList &physicals,
                           Turn turn,const PtrList &tickTypes);
  /* EFFECTS: Like send object, but guarantees to send all the objects in
     the same UDP packet. */

  static void send_new_turn(OutStreamP out) {send_generic(out,NEW_TURN);}

  static void send_new_world(OutStreamP,WorldP,int worldVersion);

  static void send_reset(OutStreamP out,GameStyleType t) 
  {send_int(out,RESET,(u_int)t);}
  
  static void send_world_room(OutStreamP,WorldP,
                              const RoomIndex &,int worldVersion);

  static void send_message(OutStreamP,char *msg);

  static void send_arena_message(OutStreamP,int timeMS,char *msg);

  static void send_command(OutStreamP,const IntelId &,ITcommand);

  static void send_delete_object(OutStreamP,const Id &);

  static void send_ping(OutStreamP out) {send_generic(out,PING);}

  static void send_pong(OutStreamP out,Turn turn) 
  {send_int(out,PONG,(int)turn);}

  static void send_timer_ping(OutStreamP out,Turn turn)
  {send_int(out,TIMER_PING,(int)turn);}

  static void send_timer_pong(OutStreamP out,Turn pingTurn,Turn localTurn);

  static void send_disconnect(OutStreamP out) {send_generic(out,DISCONNECT);}

  static void send_humans_playing(OutStreamP out,u_int num) 
  {send_int(out,HUMANS_PLAYING,num);}
  
  static void send_enemies_playing(OutStreamP out,u_int num) 
  {send_int(out,ENEMIES_PLAYING,num);}

  static void send_request_new_human(OutStreamP out) 
  {send_generic(out,REQUEST_NEW_HUMAN);}

  static void send_sound_request(OutStreamP,const SoundRequest &);

  static void send_server_pong(OutStreamP out,GameStyleType,int enemiesNum,
                               int humansNum,
                               const char* version,
                               const char* names[],
                               const char* clientNames[],
                               int humanKills[],
                               int enemyKills[],
                               const Id[]);
  /* NOTE: Arrays are all of length humansNum. */
  
  static void send_chat_request(OutStreamP out,const char* receiver,
                                const char* message);

  static void send_chat(OutStreamP out,const char* sender,
                        const IntelId& senderId,const char* message);


private:
  static u_int compute_object_length(PhysicalP);
  static void _send_object(OutStreamP,PhysicalP,
                           Turn turn,TickType tt,u_int len);
  /* EFFECTS: Helpers for send_object() and send_objects(). */
};

#endif
