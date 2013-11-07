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

// "xetp.cpp"

#if X11
	#ifndef NO_PRAGMAS
	#pragma implementation "xetp.h"
	#endif
#endif

#include "stdafx.h"

#include <iostream>
#if X11
#include <strstream>
#endif
#if WIN32
#include <strstrea.h>
#endif


// Is this include file necessary??
#if WIN32
#include <signal.h>
#endif


#include "utils.h"
#include "intel.h"
#include "streams.h"
#include "physical.h"
#include "world.h"
#include "locator.h"
#include "xetp.h"



void XETP::check_sizes() {
  XETPBasic::check_sizes();

  assert(sizeof(Dir) == sizeof(int));
  assert(sizeof(Health) == sizeof(int));
  assert(sizeof(ColorNum) == sizeof(int));
  assert(sizeof(ITcommand) == sizeof(int));
  assert(sizeof(Mass) == sizeof(int));
  assert(sizeof(Turn) == sizeof(int));
  assert(sizeof(GameStyleType) == sizeof(int));
  assert(sizeof(SoundName) == sizeof(int));
  assert(sizeof(ABBitField) == sizeof(int));

  // Make sure the element fits in a PtrList.
  // >= instead of == because on an Alpha, 
  // void* is 8 while int and TickType are 4.
  assert(sizeof(void *) >= sizeof(TickType));  
  assert(sizeof(void *) >= sizeof(int));
  assert(sizeof(void*) >= sizeof(ClassId));
}



void XETP::send_tcp_connect(OutStreamP out,u_short udpPort,char *humanName,
                            const ViewportInfo &vInfo,int skip,Boolean wantSounds) {
  assert(humanName);  // can still be "".

  u_int len = 
    sizeof(u_short) +                            // udpPort
    Utils::get_string_write_length(humanName) +  // humanName
    ViewportInfo::get_write_length();            // vInfo
  
  if (out->get_protocol() == GenericStream::UDP) {
    ((UDPOutStreamP)out)->prepare_packet(XETP::add_header(len));
  }
  send_header(out,TCP_CONNECT,len);
  out->write_short(udpPort);
  Utils::string_write(out,humanName);
  vInfo.write(out);
  out->write_int(skip);
  out->write_char((char)wantSounds);
  if (out->get_protocol() == GenericStream::UDP) {
    ((UDPOutStreamP)out)->done_packet();
  }
}



void XETP::send_human(OutStreamP out,HumanP human) {
  u_int len = 
    Identifier::get_write_length() + // intelId
    human->get_write_length();       // human data
  
  if (out->get_protocol() == GenericStream::UDP) {
    ((UDPOutStreamP)out)->prepare_packet(XETP::add_header(len));
  }
  
  send_header(out,HUMAN,len);
  IntelId intelId = human->get_intel_id();
  intelId.write(out);
  human->write(out);

  if (out->get_protocol() == GenericStream::UDP) {
    ((UDPOutStreamP)out)->done_packet();
  }
}



void XETP::send_object(OutStreamP out,PhysicalP p,Turn turn,TickType tt) {
  u_int len = compute_object_length(p);

  if (out->get_protocol() == GenericStream::UDP) {
    ((UDPOutStreamP)out)->prepare_packet(XETP::add_header(len));
  }
  
  _send_object(out,p,turn,tt,len);
  
  if (out->get_protocol() == GenericStream::UDP) {
    ((UDPOutStreamP)out)->done_packet();
  }    
}



void XETP::send_objects(OutStreamP out,const PtrList &physicals,
                        Turn turn,const PtrList &tickTypes) {
  assert(physicals.length() == tickTypes.length());
  u_int totalLen = 0;
  int n;
  for (n = 0; n < physicals.length(); n++) {
    totalLen += 
      XETP::add_header(compute_object_length((PhysicalP)physicals.get(n)));
  }

  // length passed into UDP prepare_packet is bigger than the lengths 
  // passed into each XETP packet.
  if (out->get_protocol() == GenericStream::UDP) {
    ((UDPOutStreamP)out)->prepare_packet(totalLen);
  }
  
  // Send each object in turn.
  for (n = 0; n < physicals.length(); n++) {
    PhysicalP p = (PhysicalP)physicals.get(n);
    // computing object_length twice, but who cares.
    _send_object(out,p,turn,(TickType)tickTypes.get(n),
                 compute_object_length(p));
  }
  
  if (out->get_protocol() == GenericStream::UDP) {
    ((UDPOutStreamP)out)->done_packet();
  }      
}



u_int XETP::compute_object_length(PhysicalP p) {
  u_int len = sizeof(u_short) +         // classId
    Identifier::get_write_length() +  // id
    sizeof(Turn) +                    // turn
    sizeof(char) +                    // TickType
    p->get_write_length();            // object data
  return len;
}



void XETP::_send_object(OutStreamP out,PhysicalP p,Turn turn,
                        TickType tt,u_int len) {
  // len does not include the XETP header.

  const PhysicalContext *cx = p->get_context();
  // Only send objects that can be read back in.
  assert(cx->create_from_stream);
  Id id = p->get_id();

  send_header(out,OBJECT,len);
  assert(p->get_class_id() <= USHRT_MAX);
  out->write_short((u_short)p->get_class_id());
  id.write(out);
  out->write_int((int)turn);
  assert(tt <= UCHAR_MAX);
  out->write_char((char)tt);
  p->write(out);
}



void XETP::send_new_world(OutStreamP out,WorldP world,int worldVersion) {
  u_int len = sizeof(int) +       // version
    world->get_write_length();  // world data
  if (out->get_protocol() == GenericStream::UDP) {
    ((UDPOutStreamP)out)->prepare_packet(XETP::add_header(len));
  }

  send_header(out,NEW_WORLD,len);
  out->write_int(worldVersion);
  world->write(out);

  if (out->get_protocol() == GenericStream::UDP) {
    ((UDPOutStreamP)out)->done_packet();
  }
}



void XETP::send_message(OutStreamP out,char *msg) {
  assert(msg);
  u_int len = strlen(msg);
  
  if (out->get_protocol() == GenericStream::UDP) {
    ((UDPOutStreamP)out)->prepare_packet(XETP::add_header(len));
  }

  send_header(out,MESSAGE,len);
  // signed/unsigned shouldn't matter here, ASCII only.
  out->write((u_char *)msg,len);
  
  if (out->get_protocol() == GenericStream::UDP) {
    ((UDPOutStreamP)out)->done_packet();
  }
}



void XETP::send_arena_message(OutStreamP out,int time,char *msg) {
  assert(msg);
  u_int len = sizeof(int) +  // time
    strlen(msg);             // string body
  
  if (out->get_protocol() == GenericStream::UDP) {
    ((UDPOutStreamP)out)->prepare_packet(XETP::add_header(len));
  }

  send_header(out,ARENA_MESSAGE,len);
  out->write_int(time);
  // signed/unsigned shouldn't matter here.
  out->write(msg,len - sizeof(int));
  
  if (out->get_protocol() == GenericStream::UDP) {
    ((UDPOutStreamP)out)->done_packet();
  }
}



void XETP::send_command(OutStreamP out,
                        const IntelId &iId,ITcommand command) {
  u_int len = 
    Identifier::get_write_length() + // intelId
    sizeof(char);                     // command
  
  if (out->get_protocol() == GenericStream::UDP) {
    ((UDPOutStreamP)out)->prepare_packet(XETP::add_header(len));
  }
  
  send_header(out,COMMAND,len);
  iId.write(out);
  assert(command <= UCHAR_MAX);
  out->write_char((u_char)command);
  
  if (out->get_protocol() == GenericStream::UDP) {
    ((UDPOutStreamP)out)->done_packet();
  }
}



void XETP::send_delete_object(OutStreamP out,const Id &id) {
  u_int len = 
    Identifier::get_write_length();  // Id
  
  if (out->get_protocol() == GenericStream::UDP) {
    ((UDPOutStreamP)out)->prepare_packet(XETP::add_header(len));
  }
  
  send_header(out,DELETE_OBJECT,len);
  id.write(out);
  
  if (out->get_protocol() == GenericStream::UDP) {
    ((UDPOutStreamP)out)->done_packet();
  }
}



void XETP::send_timer_pong(OutStreamP out,Turn pingTurn,Turn localTurn) {
  u_int len = 
    2 * sizeof(Turn);
  
  if (out->get_protocol() == GenericStream::UDP) {
    ((UDPOutStreamP)out)->prepare_packet(XETP::add_header(len));
  }
  
  send_header(out,TIMER_PONG,len);
  out->write_int((int)pingTurn);
  out->write_int((int)localTurn);
  
  if (out->get_protocol() == GenericStream::UDP) {
    ((UDPOutStreamP)out)->done_packet();
  }
}



void XETP::send_sound_request(OutStreamP out,const SoundRequest &request) {
  u_int len = 
    SoundRequest::get_write_length();
  
  if (out->get_protocol() == GenericStream::UDP) {
    ((UDPOutStreamP)out)->prepare_packet(XETP::add_header(len));
  }
  
  send_header(out,SOUND_REQUEST,len);
  request.write(out);

  if (out->get_protocol() == GenericStream::UDP) {
    ((UDPOutStreamP)out)->done_packet();
  }
}



void XETP::send_server_pong(OutStreamP out,GameStyleType gameStyle,
                            int enemiesNum,int humansNum,
                            const char* version,
                            const char* names[],const char* clientNames[],
                            int humanKills[],
                            int enemyKills[],const Id ids[]) {
  u_int len = 
    sizeof(u_char) +                          // gameStyle
    sizeof(u_int) +                           // enemiesNum
    sizeof(u_short) +                         // humansNum
    Utils::get_string_write_length(version);  // version
  int n;
  for (n = 0; n < humansNum; n++) {
    len += Utils::get_string_write_length(names[n]);       // name
    len += Utils::get_string_write_length(clientNames[n]); // name
    len += sizeof(u_int);                                  // humanKills
    len += sizeof(u_int);                                  // enemyKills
    len += Id::get_write_length();                         // Id
  }

  if (out->get_protocol() == GenericStream::UDP) {
    ((UDPOutStreamP)out)->prepare_packet(XETP::add_header(len));
  }

  assert(gameStyle < 256 && enemiesNum >= 0 && humansNum >= 0 && 
         (((u_int)humansNum & 0xffff0000) == 0));
  send_header(out,SERVER_PONG,len);
  out->write_char((u_char)gameStyle);
  out->write_int((u_int)enemiesNum);
  out->write_short((u_short)humansNum);
  Utils::string_write(out,version);
  // Write out data for each human.
  for (n = 0; n < humansNum; n++) {
    Utils::string_write(out,names[n]);
    Utils::string_write(out,clientNames[n]);
    out->write_int((u_int)humanKills[n]);
    out->write_int((u_int)enemyKills[n]);
    ids[n].write(out);
  }

  if (out->get_protocol() == GenericStream::UDP) {
    ((UDPOutStreamP)out)->done_packet();
  }
}



void XETP::send_chat_request(OutStreamP out,const char* receiver,
                             const char* message) {
  u_int len = 
    Utils::get_string_write_length(receiver) +    // receiverName
    Utils::get_string_write_length(message);      // message
  
  if (out->get_protocol() == GenericStream::UDP) {
    ((UDPOutStreamP)out)->prepare_packet(XETP::add_header(len));
  }
  
  send_header(out,CHAT_REQUEST,len);
  Utils::string_write(out,receiver);
  Utils::string_write(out,message);

  if (out->get_protocol() == GenericStream::UDP) {
    ((UDPOutStreamP)out)->done_packet();
  }
}



void XETP::send_chat(OutStreamP out,const char* sender,
                     const IntelId& senderId,const char* message) {
  u_int len = 
    Utils::get_string_write_length(sender) +      // sender
    IntelId::get_write_length() +                 // senderId
    Utils::get_string_write_length(message);      // message
  
  if (out->get_protocol() == GenericStream::UDP) {
    ((UDPOutStreamP)out)->prepare_packet(XETP::add_header(len));
  }
  
  send_header(out,CHAT,len);
  Utils::string_write(out,sender);
  senderId.write(out);
  Utils::string_write(out,message);

  if (out->get_protocol() == GenericStream::UDP) {
    ((UDPOutStreamP)out)->done_packet();
  }
}



