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

#if X11
	#ifndef NO_PRAGMAS
	#pragma implementation "role.h"
	#endif
#endif

#include "stdafx.h"

#include <iostream>
#if X11
#include <strstream>
#include <time.h>
#endif
#if WIN32
#include <strstrea.h>
#endif


#include "utils.h"
#include "neth.h"
#include "role.h"
#include "streams.h"
#include "physical.h"
#include "world.h"
#include "locator.h"
#include "xetp.h"
#include "intel.h"
#include "game_style.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//  Different Roles: StandAlone, Client, and Server
/////////////////////////////////////////////////////////////////////////////

// Warn Client 3 times before disconnecting them.  All in turns.4
#define NO_ACTIVITY_WARN         75    // 3 sec
#define NO_ACTIVITY_WARN_2       100   // 4 sec
#define NO_ACTIVITY_WARN_3       125   // 5 sec
// Always give at least this much time to setup.
#define MINIMUM_CUTOFF           500   // 20 sec

#define NO_ACTIVITY_DISCONNECT_DEFAULT 7000  // in milliseconds

#define CLIENT_OLD_OBJECT_KILL 10 // turns

#define SEND_HUMAN_TIME 25 // turns

// Initial value of skip if dynamically adjusting skip.
#define ADJUST_SKIP_INITIAL_SKIP 4
// Maximum allowed value for skip when adjusting skip.  If this
// doesn't do it, assume nothing will.
// Probably should be kept < Connection::ADJUST_SKIP_TIME
#define ADJUST_SKIP_MAX 10
// Aim at making delay <= to this.
#define DEFAULT_TARGET_DELAY 5

// Number of different UDP ports to try to bind the local UDP address.
#define CLIENT_PORT_TRIES 5


Role::Role() {
  assert(!roleExists);
  roleExists = True;
}



Role::~Role() {
  assert(roleExists);
  roleExists = False;
}



GameStyleType Role::get_default_game_style() {
  return LEVELS;
}



int Role::num_viewports_needed() {
  return get_humans_num();
}



void Role::human_created(IGameManagerP,HumanP,int,WorldP,LocatorP) {
}



void Role::clock(IGameManagerP,WorldP,LocatorP) {
}



void Role::yield_time(CMN_TIME startTime,int quanta) {
  struct timeval waitTime;
  int msec = compute_remaining(waitTime,startTime,quanta);
  
  if (waitTime.tv_usec > 0) {
#if WIN32
	  SleepEx(msec,FALSE);
#endif
#if X11
    if (CMN_SELECT(0,NULL,NULL,NULL,&waitTime) < 0) {
      error("Error with select.");
      // Could force some sort of exit() here.
    }
#endif
  } 
}



void Role::display_chat_message(LocatorP locator,const char* sender,
                                const char* message) {
  int senLen = Utils::strlen(sender);
  int msgLen = Utils::strlen(message);
  // Two for '<', '>' and one for '\0'.
  char *newMsg = new char[senLen + msgLen + 3];  
  assert(newMsg);

  newMsg[0] = '<';
  Utils::strcpy(newMsg + 1,sender);
  newMsg[senLen + 1] = '>';
  Utils::strcpy(newMsg + senLen + 2,message);

  // Display as an arena message, don't propogate message to the Clients.
  // Clients will already receive XETP::CHAT packets.
  locator->arena_message_enq(newMsg,NULL,-1,False);
  // Locator now owns the memory for newMsg.
}



void Role::new_level(IGameManagerP,WorldP,LocatorP) {  
}



void Role::reset(IGameManagerP) {
}



void Role::handle_messages(IGameManagerP,LocatorP) {
}



void Role::set_humans_playing(int) {
}



void Role::set_enemies_playing(int) {
}



void Role::game_quitting() {
}



void Role::send_chat_request(LocatorP,const char*,const char*) {
}



// Return time since startTime in milliseconds.
static int time_since(CMN_TIME startTime) {
#if X11
  struct timeval now;
  if (gettimeofday(&now,NULL) != 0) {
    cerr << "Error with gettimeofday()" << endl;
  }
  // diff in msec.
  assert(sizeof(int) == 4);
  
  long secDiff = now.tv_sec - startTime.tv_sec;
  long usecDiff = now.tv_usec - startTime.tv_usec;  // Could be negative.
  
  int diff = secDiff * 1000 + usecDiff / 1000;
#endif

#if WIN32
  // diff is in msec
  int diff = timeGetTime() - startTime;
#endif

  return diff;
}



int Role::compute_remaining(struct timeval &waitTime,
                            CMN_TIME startTime,int quanta) {
  int msec;
  waitTime.tv_sec = 0;


  int diff = time_since(startTime);

  // Give up remaining time.
  if (diff > 0) {
    msec = quanta - diff;
  }
  // No time has elapsed, so give up all of quanta.
  else {
    msec = quanta;
  }

  // Used up all the time.
  if (msec < 0) {
    msec = 0;
  }
    
  // Clamp at giving up one second of time, shouldn't happen.
  if (msec >= 1000) {
    msec = 999;
  }
  waitTime.tv_usec = 1000 * msec;
  return msec;
}



// All client/server/standalone errors/messages come through here.
void Role::_error(const char *msg) {
#if WIN32
  // In debugger.
  TRACE(msg);
#endif
#if X11
  // Use standard out for both errors and messages on UNIX.
  cout << msg << endl;
#endif
}



void Role::error(const char *msg1,const char *msg2,const char *msg3) {
  ostrstream str;

  if (msg2 == NULL) {
    str << msg1 << ends;
  }
  else if (msg3 == NULL) {
    str << msg1 << msg2 << ends;
  }
  else {
    str << msg1 << msg2 << msg3 << ends;
  }

  // Call one argument version.
  _error(str.str());

  delete str.str();
}



void Role::message(const char *msg1,const char *msg2,const char *msg3) {
  ostrstream str;

  if (msg2 == NULL) {
    str << msg1 << ends;
  }
  else if (msg3 == NULL) {
    str << msg1 << msg2 << ends;
  }
  else {
    str << msg1 << msg2 << msg3 << ends;
  }

  // Call one argument version.
  _message(str.str());

  delete str.str();
}



void Role::_message(const char *msg) {
  _error(msg);
}



void Role::check_show_stats(CMN_TIME startTime) {
  //  static startTime;
  static int counter;

  // Sample every 25 turns.
  if (showStats && (counter % 25 == 0)) {
    int msec = time_since(startTime);
    int sec = msec / 1000;
    
    if (msec > 0) {
      cout << "----- UDPin: " << UDPInStream::get_bytes_in() 
            << " UDPout: " << UDPOutStream::get_bytes_out() 
            << " TCPin: " << NetInStream::get_bytes_in() 
            << " TCPout: " << NetOutStream::get_bytes_out() 
            << " in " << msec << " milliseconds." << endl;
      if (sec > 0) {
        cout << "<BPS> UDPin: " << (UDPInStream::get_bytes_in() / sec)
             << " UDPout: " << (UDPOutStream::get_bytes_out() / sec)
             << " TCPin: " << (NetInStream::get_bytes_in() / sec)
             << " TCPout: " << (NetOutStream::get_bytes_out() / sec)
             << " in " << sec << " seconds, turn " << counter << endl;
      }
      cout << "Average outgoing XETP packet length: " 
           << UDPOutStream::get_average_length_out() << endl;
      cout << endl;
      
      // counter = 0;
      // startTime = now;
    }
  }

  UDPInStream::reset_counter();
  UDPOutStream::reset_counter();
  NetInStream::reset_counter();
  NetOutStream::reset_counter();
  counter++;
}



int Role::quantaToMS(Quanta q,IGameManagerP man) {
  return q * Utils::maximum((int)man->manager_get_quanta(),1);
}



Quanta Role::MSToQuanta(int val,IGameManagerP man) {
  // watch out for division by zero.
  return val / Utils::maximum((int)man->manager_get_quanta(),1);
} 



Boolean Role::uses_humans_num(RoleType r) {
  switch (r) {
    case R_STAND_ALONE:
      return True;
    case R_CLIENT:
    case R_SERVER:
      return False;
    default:
      assert(0);
      return False;
  }
}



Boolean Role::uses_cooperative(RoleType r) {
  switch (r) {
    case R_STAND_ALONE:
    case R_SERVER:
      return True;
    case R_CLIENT:
      return False;
    default:
      assert(0);
      return False;
  }
}



Boolean Role::uses_enemies_num(RoleType r) {
  switch (r) {
    case R_STAND_ALONE:
    case R_SERVER:
      return True;
    case R_CLIENT:
      return False;
    default:
      assert(0);
      return False;
  }
}



Boolean Role::uses_enemies_refill(RoleType r) {
  switch (r) {
    case R_STAND_ALONE:
    case R_SERVER:
      return True;
    case R_CLIENT:
      return False;
    default:
      assert(0);
      return False;
  }
}



Boolean Role::uses_game_style(RoleType r) {
  switch (r) {
    case R_STAND_ALONE:
    case R_SERVER:
      return True;
    case R_CLIENT:
      return False;
    default:
      assert(0);
      return False;
  }
}



Boolean Role::uses_difficulty(RoleType r) {
  switch (r) {
    case R_STAND_ALONE:
    case R_SERVER:
      return True;
    case R_CLIENT:
      return False;
    default:
      assert(0);
      return False;
  }
}



Boolean Role::uses_chat(RoleType r) {
  switch (r) {
    case R_CLIENT:
    case R_SERVER:
      return True;
    case R_STAND_ALONE:
      return False;
    default:
      assert(0);
      return False;
  }
}



Boolean Role::showStats = False;



Boolean Role::echoPingPong = False;



Boolean Role::roleExists = False;



StandAlone::StandAlone() {
  humansNum = 0;
}



Boolean StandAlone::ok() {
  return True;
}



RoleType StandAlone::get_type() {
  return R_STAND_ALONE;
}



int StandAlone::get_humans_num() {
  return humansNum;
}



void StandAlone::set_humans_num(int val) {
  humansNum = val;
}



void StandAlone::yield(CMN_TIME startTime,int quanta,IGameManagerP,
                       ITurnStarterP turnStarter,
                       WorldP,LocatorP l) {
  PhysicalIter iter(*l);

  // Some debugging crap, should probably take it out for a release build.
  PhysicalP p;
  while (p = iter()) {
    const Area &area = p->get_area();
    const Area &areaNext = p->get_area_next();
    if (!(area == areaNext)) {
      cerr << "test failed, area does not equal area next for a " 
           << p->get_class_name()
           << endl;
    }
  }

  // Throw away time.
  Role::yield_time(startTime,quanta);

  // Start timing for next turn.
  turnStarter->start_turn();
}



Client::Client(char *sName,char *portName,CMN_PORT clientPrt,char *hName,
               IViewportInfo* vInf,int skp,LocatorP errLocator) {
  assert(vInf);

  turn = 0;
  connected = CONN_FAILED;
  tcpIn = NULL;
  tcpOut = NULL;
  udpIn = NULL;
  udpOut = NULL;
  humanName = Utils::strdup(hName);
  errorLocator = errLocator;

    
  skip = skp;
  vInfo = vInf;

  turnMax = 0;

  XETP::check_sizes();
  
  // An invalid version.
  worldVersion = -1;

  strncpy(serverName,sName,R_NAME_MAX);

  if (portName) {
    port = Utils::atoi(portName);
    if (port <= 0) {
      error("Invalid port ",portName,".");
      return;
    }
  }
  else {
    port = XETP::DEFAULT_PORT;
  }

  // Get client UDP port.
  if (clientPrt <= 0) {
    // Default to server port plus one.
    clientPortBase = port + 1;
  }
  else {
    clientPortBase = clientPrt;
  }

  if (gethostname(hostName,R_NAME_MAX)) {
    error("Unable to get local machine's hostname.");
    return;
  }

  tcpSock = socket(AF_INET, SOCK_STREAM, 0);
  if (tcpSock < 0) {
    error("Error opening client TCP socket.");
    return;
  }
  udpSock = socket(AF_INET, SOCK_DGRAM, 0);
  if (udpSock < 0) {
    error("Error opening client UDP socket.");
    return;
  }

  
  // Display message that we are looking up IP address.
  // Assumes that caller will call Client::connect_server after a turn or two, 
  // so the message will be displayed on the Ui.
  ostrstream str;
  str << "Looking up IP address for server " << serverName << ends;
  // Display for a long time.
  errLocator->arena_message_enq(str.str(),NULL,1000000); 
  errLocator->message_enq(Utils::strdup(str.str()));
  // Probably would be better to use Role::message(), but we want it to stay
  // up for a long time.  Should add argument to Role::message().

  connected = CONN_PARTIALLY;
}



void Client::connect_server() {  
  if (connected != CONN_PARTIALLY) {
    connected = CONN_FAILED;
    return;
  }


  // Create server address.
  memset((void *)&serverAddr,'\0',sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  u_long IPAddr = inet_addr(serverName);
  // serverName is in dotted IP address notation, just copy it in.
  if (IPAddr != INADDR_NONE) {
    memcpy((void *)&serverAddr.sin_addr.s_addr,(void *)&IPAddr,sizeof(u_long)); 
  }
  // serverName is a hostname, perform DNS lookup.
  else {
    struct hostent *hp = gethostbyname(serverName);
    if (hp == 0) {
      error("Invalid server name ",serverName);
      return;
    }
    memcpy((void *)&serverAddr.sin_addr,(void *)hp->h_addr,hp->h_length);
  }
  serverAddr.sin_port = htons((u_short)port);


  // Bind local address for UDP
  // Try all ports in the range 
  // [clientPortBase...(clientPortBase+CLIENT_PORT_TRIES)).
  // Makes it easier for users to run multiple clients on the same machine,
  // only really meaningful on UNIX.
  int n;
  for (n = 0; n < CLIENT_PORT_TRIES; n++) {
    CMN_SOCKADDR_IN client;
    memset((void *)&client,'\0',sizeof(client));
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = htonl(INADDR_ANY);
    client.sin_port = htons((u_short)(clientPortBase + n));
    if (bind(udpSock,(CMN_SOCKADDR *)&client,sizeof(client)) >= 0) {
      // Success.
      break;
    }
  }
  if (n == CLIENT_PORT_TRIES) {
    ostrstream str;
    str << "Could not bind local UDP port to any of " 
        << clientPortBase << "-" << (clientPortBase + CLIENT_PORT_TRIES - 1) 
        << ends;
    error(str.str());
    delete str.str();
    return;    
  }
  //  cout << "UDP port seems to be" << client.sin_port << endl;

  
  // Connect TCP to server
  if (connect(tcpSock,
              (CMN_SOCKADDR *)&serverAddr,sizeof(serverAddr)) >= 0) {
    // Send initial connection
    tcpOut = new NetOutStream(tcpSock,True);
    assert(tcpOut);
    // Client tells server if it should send sounds or not.
#if WIN32
    Boolean wantSounds = True;
#endif
#if X11
    Boolean wantSounds = False;
#endif

    // Note: This is the only time we use vInfo.
    // Ui must have called Viewport::init_viewport_info() before now or 
    // this will fail.
    XETP::send_tcp_connect(tcpOut,clientPortBase + n,humanName,
                           vInfo->get_info(),
                           skip,wantSounds);

    // Mark as OK.
    connected = CONN_SUCCESS;
  }

  // Could test that UDP connection works before continuing.

  // Inform user of failed connection
  if (!ok()) {
    strstream msg;
    msg << "Could not connect to " << serverName << " on port " <<
      port << "." << ends;
    error(msg.str());
    delete(msg.str());
    delete tcpOut;
    return;
  }

  // Inform user of successful connection
  strstream msg;
  msg << hostName << " connected to " << serverName << " on port " <<
    port << ends;
  message(msg.str());
  delete msg.str();

  
  // Create TCP and UDP streams.
  tcpIn = new NetInStream(tcpSock,False);
  assert(tcpIn);
  udpIn = new UDPInStream(udpSock,True);
  assert(udpIn);
  udpOut = new UDPOutStream(udpSock,&serverAddr,False);
  assert(udpOut);

  // Used so that multiple update_from_stream()s don't animate multiple times.
  NetData::enable_clocked_flag();
}



Client::~Client() {
  // Perhaps we should use ok()
  if (connected == CONN_SUCCESS) {
    // Explicitly tell server to disconnect.
    XETP::send_disconnect(udpOut);
    udpOut->flush();    
  }
  
  Utils::freeif(humanName);
  delete tcpIn;
  delete tcpOut;
  delete udpIn;
  delete udpOut;
}



RoleType Client::get_type() {
  return R_CLIENT;
}



int Client::get_humans_num() {
  return 1;
}



void Client::set_humans_num(int) {
}



Boolean Client::ok() {
  return connected == CONN_SUCCESS;
}



void Client::clock(IGameManagerP manager,WorldP,LocatorP locator) {
  if (!ok()) {
    return;
  }

  // Only send human command every few turns.
  if (humanReflexes.ready()) {

    IntelP intel = locator->lookup(humanIntelId);
    if (intel) {
      assert(intel->is_human());
      HumanP human = (HumanP)intel;
      if (human->get_command() != IT_NO_COMMAND) {
        XETP::send_command(udpOut,humanIntelId,human->get_command());
        // Want to send this immediately.
//        udpOut->flush();
        human->set_command(IT_NO_COMMAND);
        
        // Set timer.
        int reflexTime = MSToQuanta(humanReflexesTimeMS,manager);
        humanReflexes.set(reflexTime);
      }
    }
  }

  humanReflexes.clock();
  // Turn is not incremented until yield() is done.
}



void Client::yield(CMN_TIME startTime,int quanta,IGameManagerP manager,
                   ITurnStarterP turnStarter,
                   WorldP w,LocatorP l) {
  if (!ok()) {
    // Still want to throw away time properly.
    Role::yield_time(startTime,quanta);
    // Start timing for next turn.
    turnStarter->start_turn();
    return;
  }

  check_alive(manager);

  // Should we modify quanta???

  // Decide how much time to throw away.
  struct timeval waitTime;
  compute_remaining(waitTime,startTime,quanta);

  fd_set fdvar;
  FD_ZERO(&fdvar);
  // Listen to server.
  FD_SET(tcpSock,&fdvar);
  FD_SET(udpSock,&fdvar);

  // Before any changes made to objects.
  l->client_pre_clock();
  NetData::toggle_clocked_flag();

  // Loop until all time for this turn thrown away or server starts new turn.
  do {
    check_alive(manager);

    int sockMax = Utils::maximum(tcpSock,udpSock) + 1;
    int cond = CMN_SELECT(sockMax,&fdvar,NULL,NULL,&waitTime);
    if (cond > 0) {
      // TCP data coming in.
      if (FD_ISSET(tcpSock,&fdvar)) {
        Boolean newTurn = receive_tcp_data(manager,w,l);
        if (newTurn) {
          waitTime.tv_usec = 0;
        }
      }
      
      // UDP data coming in.
      if (FD_ISSET(udpSock,&fdvar)) {
        Boolean newTurn = receive_udp_data(manager,w,l);
        if (newTurn) {
          waitTime.tv_usec = 0;
        }
      }
      
      // See how much more, if any, time we still need to throw away.
      compute_remaining(waitTime,startTime,quanta);
    }

    // Error or time expired.
    else {
      // To be sure.
      waitTime.tv_usec = 0;
    }
  } while (waitTime.tv_usec > 0);


  // Start the timer for the next turn here because we are doing 
  // significant computation AFTER calling select to throw time away.
  // So, the following time will be added to the next turn.
  turnStarter->start_turn();


  // Two things:
  // 1) Dead reckoning and animation clock
  //
  // 2)  Now check to see if any objects are old and need to be killed off.
  // We do this by comparing the turn of this object with the highest turn
  // we've ever seen.
  PtrList dieList;
  PhysicalIter iter(*l);
  PhysicalP p;
  while (p = iter()) {
    NetDataP netData = p->get_net_data();
    assert(turnMax >= netData->get_last_modified());
    if (turnMax - netData->get_last_modified() >= CLIENT_OLD_OBJECT_KILL) {
#if 0
      ostrstream str;
      str << "killing old object " << p->get_class_name()
          << " turn=" << turn << " turnMax=" << turnMax << ends;
      message(str.str());
      delete str.str();
#endif
      dieList.add((void*)p);
    }
    
    // Do dead reckoning for p if we aren't going to kill it.
    else if (deadReckoning) {
      // Only do dead reckoning if we didn't receive new data this turn.
      NetDataP netData = p->get_net_data();
      if (!netData->clocked_this_turn()) {
        p->dr_clock();
        netData->set_clocked_this_turn();
      }
    }
  }
  // Don't call explicit_die inside the iteration because it screws up the
  // iterator.
  int n;
  for (n = 0; n < dieList.length(); n++) {
    l->explicit_die((PhysicalP)dieList.get(n));
  }
  
  // After changes made to objects.
  // Also called above
  l->client_post_clock();

  // Flush any commands/pings to the server.
  udpOut->flush();

  turn++;

  check_show_stats(startTime);
}



void Client::game_quitting() {
  if (!ok()) {
    return;
  }

  // Explicitly tell server to disconnect.
  //// Why do we need this.  Don't we already do it in Client::~Client??
  XETP::send_disconnect(udpOut);
  udpOut->flush();
}



void Client::send_chat_request(LocatorP,const char* receiver,
                               const char* message) {
  if (!ok()) {
    return;
  }

  XETP::send_chat_request(udpOut,receiver,message);
  udpOut->flush();
}



void Client::request_new_human() {
  if (!ok()) {
    return;
  }
  XETP::send_request_new_human(udpOut);
}



void Client::_error(const char *msg) {
  // Also print to stdout.
  Role::_error(msg);

  if (errorLocator) {
    ostrstream str2;
    str2 << "ERROR: " << msg << ends;
    errorLocator->arena_message_enq(str2.str(),NULL,ROLE_FAILED_TIME);
    // Don't delete str2.str(), give memory to the Locator.
    errorLocator->message_enq(Utils::strdup(str2.str()));
  }
#if WIN32
  // Only the client can afford to block on error messages.
//  AfxMessageBox(msg);
#endif
}



void Client::_message(const char *msg) {
  // Also print to stdout.
  Role::_error(msg);

  if (errorLocator) {
    ostrstream str2;
    str2 << msg << ends;
    errorLocator->arena_message_enq(str2.str());
    // Don't delete str2.str(), give memory to the Locator.
    errorLocator->message_enq(Utils::strdup(str2.str()));
  }
}



Boolean Client::receive_tcp_data(IGameManagerP manager,
                                 WorldP world,LocatorP locator) {
  if (!ok()) {
    return False;
  }

  Boolean ret = False;  // not a new turn
  u_short method = 0;
  u_int length = 0;
  if (!XETP::receive_header(tcpIn,method,length)) {
    message("Received invalid TCP header.");
    return ret;
  }
  
  switch (method) {
  case XETP::HUMAN:
    // Use udpOut no matter what
    process_human(tcpIn,udpOut,manager,world,locator);
    break;
    
    // Can't receive via TCP, because sometimes we throw them away.
#if 0
  case XETP::OBJECT:
    process_object(tcpIn,world,locator);
    break;
#endif
    
  case XETP::NEW_TURN:
    ret = True;
    break;
    
  case XETP::NEW_WORLD:
    //    message("new_world received");
    process_new_world(tcpIn,manager,world,locator);
    break;

  case XETP::RESET:
    process_reset(tcpIn,manager,world,locator);
    break;
    
  case XETP::MESSAGE:
    process_message(tcpIn,length,locator);
    break;

  case XETP::ARENA_MESSAGE:
    process_arena_message(tcpIn,length,manager,locator);
    break;
    
  default:
    error("Received unknown XETP header.");
  }
  
  return ret;
}



static const char *lastObject = NULL; // class name
static u_short lastMethod = 333;
static u_int lastLength = 0;

Boolean Client::receive_udp_data(IGameManagerP manager,
                                 WorldP world,LocatorP locator) {
  if (!ok()) {
    return False;
  }

  Boolean ret = False;  // not a new turn

  // Prepare to read from packet.
  CMN_SOCKADDR_IN addr;
  if (udpIn->prepare_packet(&addr) < 0) {
    error("Failed to read UDP packet.");
    return False;
  }
  // Should check that addr == serverAddr.


  // Keep reading XETP packets until the UDP packet is empty, or we hit 
  // an error.  If we hit an error, just discard the packet.
  int bytesRemaining;
  Boolean ok = True;
  while (ok && (bytesRemaining = udpIn->bytes_remaining()) > 0) {
    u_short method = 0;
    u_int length = 0;
    if (!XETP::receive_header(udpIn,method,length)) {
      error("Received invalid UDP header.");
      ok = False;
    }

    if (ok) {
      lastLength = length;
      lastMethod = method;
      
      switch (method) {
      case XETP::HUMAN:
        process_human(udpIn,udpOut,manager,world,locator);
        break;

      case XETP::OBJECT:
        process_object(udpIn,world,locator);
        break;
      
      case XETP::DELETE_OBJECT:
        process_delete_object(udpIn,world,locator);
        break;
      
      case XETP::NEW_TURN:
        ret = True;
        break;
      
      case XETP::NEW_WORLD:
        process_new_world(udpIn,manager,world,locator);
        break;
      
      case XETP::PING:
        if (echoPingPong) {
          message("Received PING from Server, send back a PONG.");
        }
        XETP::send_pong(udpOut,turnMax);
        // Tell server NOW, or we're fucking history.
//        udpOut->flush();
        break;

      case XETP::TIMER_PING: {
        Turn pingTurn = (Turn)udpIn->read_int();
        XETP::send_timer_pong(udpOut,pingTurn,turn);                              
//        udpOut->flush();
      }
        break;

      case XETP::MESSAGE:
        process_message(udpIn,length,locator);
        break;

      case XETP::ARENA_MESSAGE:
        process_arena_message(udpIn,length,manager,locator);
        break;

      case XETP::DISCONNECT:
        error("Server has disconnected connection");
        connected = CONN_FAILED;
        break;

      case XETP::HUMANS_PLAYING:
        manager->set_humans_playing(udpIn->read_int());
        break;
      
      case XETP::ENEMIES_PLAYING:
        manager->set_enemies_playing(udpIn->read_int());
        break;

      case XETP::SOUND_REQUEST:
        process_sound_request(udpIn,locator);
        break;

      case XETP::CHAT:
        process_chat(udpIn,locator);
        break;

      default:
        error("Received unknown XETP header.");
      }
    }
  } // while bytes remaining


  // Done reading packet.
  udpIn->done_packet();

  // Should we start a new turn.
  return ret;
}



void Client::process_new_world(InStreamP in,IGameManagerP manager,
                               WorldP world,LocatorP) {
  // Don't reset the locator.

  // Don't do anything with the version for now.
  int worldVersion = in->read_int();

  //  manager->new_world(in);

  world->reset(NULL,in);
  manager->world_changed();
}



void Client::process_reset(InStreamP in,IGameManagerP manager,
                           WorldP,LocatorP) {
  GameStyleType style = (GameStyleType)in->read_int();

  // Perhaps should also call Locator::reset().
  manager->manager_reset(style);
}



void Client::process_human(InStreamP in,OutStreamP,IGameManagerP manager,
                           WorldP world,LocatorP locator) {
  IntelId intelId(in);
  IntelP intel = locator->lookup(intelId);

  // If Intel exists, just update it.
  if (intel) {
    intel->update_from_stream(in);

    // Just to be sure.
    manager->register_intel(0,intel);
  }
  // Else add a new human to the locator.
  else {
    // Clear out any existing humans.
    int hNum = locator->humans_registered();
    assert(hNum <= 1);
    if (hNum == 1) {
      HumanP old = locator->get_human(0);
      // Be sure to kill the Physical pointing to the human if it exists,
      // o.w. it will have a reference to freed memory.
      PhysicalP p = locator->lookup(old->get_id());
      if (p) {
        locator->explicit_die(p);
      }
      // This will delete the Human.
      locator->unregister_human(old->get_intel_id());
    }
    
    // Give Human outStream so that it can talk back to the Server.
    HumanP human = new Human(in,world,locator);
    if (!human->creation_ok()) {
      delete human;
      return;
    }
    
    locator->register_human_as(human);
    humanIntelId = human->get_intel_id();

    // Perhaps should call even if human is NULL to make the UI point at 
    // nothing.  (Although, Viewport currently disallows 
    // register_intel() with NULL intel.)
    manager->register_intel(0,human);
  }
}



void Client::process_object(InStreamP inStream,WorldP world,LocatorP locator) {
  // Read classId and Id
  ClassId classId = (ClassId)inStream->read_short();
  Id id(inStream);
  Turn turn = (Turn)inStream->read_int();
  TickType tt = (TickType)inStream->read_char();

  // The highest turn we've ever seen from the server.
  if (turn > turnMax) {
    turnMax = turn;
    if (echoPingPong) {
      ostrstream str;
      str << "PONG the server with turn " << turnMax << ends;
      message(str.str());
      delete str.str();
    }
    // Tell server so it can free turn windows to send more data.
    XETP::send_pong(udpOut,turnMax);
//    udpOut->flush();
  }
  
  // Don't bother creating an object that is out of date.
  assert(inStream->get_protocol() == GenericStream::UDP);
  // Must be UDP or we need to flush the rest of the data from the stream.
  if (turnMax - turn >= CLIENT_OLD_OBJECT_KILL) {
    return;
  }

  if (inStream->alive()) {
    PhysicalP pExisting = locator->lookup(id);
    // If object exists, update the object from the information in the stream.
    if (pExisting) {
      // Don't use data from old packets, if we already have newer data.
      NetDataP netData = pExisting->get_net_data();
      if (netData->get_last_modified() > turn) {
        return;
      }

      pExisting->update_from_stream(inStream);
      const char *className = pExisting->get_class_name();

      // Don't make these calls on a garbage object read in.
      if (pExisting->creation_ok() && (pExisting->get_id() == id)) {

        // Patch differences in size from client and server.
        if (pExisting->is_moving()) {
          ((MovingP)pExisting)->patch_size_and_size_next();      
        }

        // Mark object as being updated.
        netData->touch(turn);

        // Mark object as being clocked this turn, must do this after the call
        // to update_from_stream().
        netData->set_clocked_this_turn();

        // Set tick type if it was specified from the stream, TICK_MAX
        // means unspecified.
        netData->set_tick_type(tt);
        
        // debugging hack
        lastObject = className;
      }

      // Something is wrong with the object we just read, kill it.
      if (!inStream->alive() || !pExisting->creation_ok()) {
        error("Bad data read when updating ",(char *)className,
              ", destroy object.");
        locator->explicit_die(pExisting);
        
        // Bring stream back to life.
        if (!inStream->alive() && 
            inStream->get_protocol() == GenericStream::UDP) {
          ((UDPInStreamP)inStream)->revive();
        }
      }
    }
    // Create a new object from the data in the stream.
    else {
      const PhysicalContext *cx = locator->get_context(classId);
      if (cx) {
        if (cx->create_from_stream) {
          PhysicalP p = cx->create_from_stream(inStream,world,locator);
          if (p) {
            if (p->get_id() == id) {
              // Patch differences in size from client and server.
              if (p->is_moving()) {
                ((MovingP)p)->patch_size_and_size_next();      
              }
              
              // Mark object as being updated.
              NetDataP netData = p->get_net_data();
              netData->touch(turn);
              
              // Set tick type if it was specified from the stream, TICK_MAX
              // means unspecified.
              netData->set_tick_type(tt);
              
              // Id is redundant, too lazy to recompile now.
              locator->add_as(p,id);
              
              // debugging hack
              lastObject = p->get_class_name();
            }
            // Packet read in is inconsistent, id in header does not match
            // that inside the object.
            else {
              error("Bad data read creating object from stream, id's are inconsistent");
              locator->explicit_die(p);
            }
          }
          else {
            error("Failed to create object from stream.");
          }
        }
        else {
          error("No create_from_stream() for ",cx->className);
        }
      }
      else {
        ostrstream str;
        str << "No context for classId " << classId << ends;
        error(str.str());
        delete str.str();
      }
    }
  }
}



void Client::process_delete_object(InStreamP in,WorldP,LocatorP locator) {
  Id id(in);
  
  if (!in->alive()) {
    return;
  }


  // Kill the object.
  PhysicalP p = locator->lookup(id);
  if (p) {
    locator->explicit_die(p);

    //    p->set_quiet_death();
    //p->kill_self();
  }
}



void Client::process_message(InStreamP in,int length,LocatorP locator) {
  if (length <= 0) {
    return;
  }

  char *msg = new char[length + 1];
  in->read(msg,length);
  msg[length] = '\0';
  if (in->alive()) {
    locator->message_enq(msg);
  }
  else {
    delete [] msg;
  }
}



void Client::process_arena_message(InStreamP in,int length,
                                   IGameManagerP manager,LocatorP locator) {
  length -= sizeof(int); // take of the space for time(ms)

  if (length <= 0) {
    return;
  }

  int timeMS = in->read_int();
  Quanta timeQuanta = MSToQuanta(timeMS,manager);

  char *msg = new char[length + 1];
  in->read(msg,length);
  msg[length] = '\0';
  if (in->alive()) {
    // Don't need to bother specifying an Intel.
    locator->arena_message_enq(msg,NULL,timeQuanta);
  }
  else {
    delete [] msg;
  }
}



void Client::process_sound_request(InStreamP in,LocatorP locator) {
  SoundRequest request;
  request.read(in);
  if (in->alive()) {
    locator->submitSoundRequest(request);
  }
}



void Client::process_chat(InStreamP in,LocatorP locator) {
  char sender[IT_STRING_LENGTH];  
  Utils::string_read(in,sender,IT_STRING_LENGTH);

  // We don't use senderId, only exists for future use.
  IntelId senderId;
  senderId.read(in);

  char* message = Utils::string_read(in);

  display_chat_message(locator,sender,message);

  delete message;
}



void Client::check_alive(IGameManagerP /*manager*/) {
  if (ok() && 
      (!tcpIn->alive() || !tcpOut->alive() ||
       !udpIn->alive() || !udpOut->alive())) {
    connected = CONN_FAILED;
    error("Lost connection to server ",serverName);
#if 0
    strstream str;
    str << "tcpIn: " << (int)tcpIn->alive() 
        << " tcpOut: " << (int)tcpOut->alive() 
        << " udpIn: " << (int)udpIn->alive()
        << " udpOut: " << (int)udpOut->alive()
        << ends;
    error(str.str());
    delete str.str();
#endif
    //    manager->quit();
  }
}



int Client::humanReflexesTimeMS = 20; // In milliseconds.



Boolean Client::deadReckoning = True; // On by default.



Connection::Connection(ServerP srvr,
                       LocatorP eLocator,
                       char *cName,char *hName,
                       CMN_SOCKET tcp,InStreamP tcp_in,
                       CMN_SOCKET udp,CMN_SOCKADDR_IN* cAddr,
                       Turn now,
                       const ViewportInfo &v_info,int skp,Boolean wSounds) 
: vInfo(v_info) {
  server = srvr;
  errorLocator = eLocator;
  strncpy(clientName,cName,R_NAME_MAX); // why not dynamic alloc string?

  humanName = Utils::strdup(hName);
  udpClientAddr = cAddr; // We now own the memory.

  tcpIn = tcp_in;
  assert(tcpIn);
  tcpOut = new NetOutStream(tcp,False);
  assert(tcpOut);

  tcpSock = tcp;

  udpOut = new UDPOutStream(udp,udpClientAddr,False);
  assert(udpOut);

  // humanId takes care of itself.

  lastActivity = now;
  startTurn = now;

  sentCreationMessage = False;

  // Start out in the middle of the upper-left room.
  Pos pos((W_ROOM_COL_MAX * WSQUARE_WIDTH) / 2,
          (W_ROOM_ROW_MAX * WSQUARE_HEIGHT) / 2);
  viewport = vInfo.get_viewport_for(pos);

  // skipTimer starts out ready.

  // Decide whether to dynamically adjust skip or not.
  if (skp == ADJUST_SKIP) {
    adjustSkip = True;
    skip = ADJUST_SKIP_INITIAL_SKIP;
  }
  else {
    adjustSkip = False;
    skip = skp;
  }
  assert(skip > 0);
  borrowedSkip = False;
  wantSounds = wSounds;

  // Starts out set to max time, don't do anything until 
  // we have collected some data.
  adjustSkipTimer.set_max(ADJUST_SKIP_TIME);
  adjustSkipTimer.set();

  // Don't have any data yet.
  delaysNum = 0;

  // All turn windows start unallocated.
  int n;
  for (n = 0; n < TURN_WINDOWS_MAX; n++) {
    turnWindows[n] = FREE_TURN_WINDOW;
  }
}



Connection::~Connection() {
  delete tcpIn;
  delete tcpOut;
  delete udpOut;
  delete udpClientAddr;
  delete humanName; // should use delete[] ??
}



char* Connection::get_full_client_name() {
  IntelP intel = errorLocator->lookup(human);

  if (!intel) {
    return Utils::strdup(clientName);
  }

  ostrstream str;
  str << '\"' << intel->get_name() << "\"@" << clientName << ends;
  return str.str();
}



Boolean Connection::allocate_turn_window(Turn /*turn*/) {
  if (skipTimer.ready()) {
    skipTimer.set(skip);
    return True;
  }

  if (borrowedSkip) {
    return True;
  }
  
#if 0
  assert(turn != FREE_TURN_WINDOW);

  for (int n = 0; n < TURN_WINDOWS_MAX; n++) {
    if (turnWindows[n] == FREE_TURN_WINDOW) {
      turnWindows[n] = turn;

      // print allocated turn windows.
      if (echoTurnWindows) {
        cout << clientName << " [";
        for (int m = 0; m < TURN_WINDOWS_MAX; m++) {
          if (turnWindows[m] != FREE_TURN_WINDOW) {
            cout << turnWindows[m] << ",";
          }
        }
        cout << "]" << endl;
      }

      return True;
    }
  }
  
  if (echoTurnWindows) {
    cout << "FAILED to allocate window" << endl;
  }

  // No free windows.
#endif
  return False;
}



void Connection::free_turn_windows(Turn turn) {
  assert(turn != FREE_TURN_WINDOW);

  // Free all windows <= turn.
  for (int n = 0; n < TURN_WINDOWS_MAX; n++) {
    if (turnWindows[n] != FREE_TURN_WINDOW &&
        turnWindows[n] <= turn) {
      if (echoTurnWindows) {
        cout << clientName << " freed turnWindow " << turnWindows[n] << endl;
      }
      turnWindows[n] = FREE_TURN_WINDOW;
    }
  }
}



// To figure out how client and server clocks relate to each other.
static Turn lastServerTurn = 0;
static Turn lastClientTurn = 0;



void Connection::process_timer_pong(UDPInStreamP in) {
  Turn pingTurn = (Turn)in->read_int();
  lastClientTurn = (Turn)in->read_int(); // Ignore clientTurn for now.
  // don't allow negative
  Turn delay = Utils::maximum(server->get_turn() - pingTurn,0);  

  // Assume server turn is average of start and end time.
  lastServerTurn = (server->get_turn() + pingTurn) >> 1;

  if (delaysNum < MEASURED_DELAYS_MAX) {
    delays[delaysNum] = delay;
    delaysNum++;
  }
  else {
    server->error("Throwing away delay info.");
  }
}



void Connection::clock(Boolean turnWindowAllocated) {
  if (adjustSkip) {
    assert(skip > 0);
    
    // Only send timer_ping if we are allowed to send data at all 
    // this turn.
    if (turnWindowAllocated) {
      XETP::send_timer_ping(udpOut,server->get_turn());
    }

    if (adjustSkipTimer.ready()) {
      float avg = 0.0f;
      for (int n = 0; n < delaysNum; n++) {
        avg += delays[n];
      }
      if (delaysNum) {
        avg /= delaysNum;
      }
    
#ifdef SKIP_MESSAGES
      ostrstream msg;
      msg << "Average delay is " << avg << " out of " 
        << delaysNum << " samples." << ends;
      server->message(msg.str());
      delete msg.str();
#endif

#if 0
      ostrstream msg2;
      msg2 << "clientTurn=" << lastClientTurn 
        << " serverTurn=" << lastServerTurn 
        << " diff=" << (lastServerTurn - lastClientTurn)
        << ends;
      server->message(msg2.str());
      delete msg2.str();
#endif
      

      // Do the adjustment.
      // Latency is too high, send less data.
      if (delaysNum == 0 || avg > targetDelay) {
        if (skip < ADJUST_SKIP_MAX) {
          skip++;
        }
//          else {
//            server->error("Client has reached maximum skip value.");
//          }
#ifdef SKIP_MESSAGES
        ostrstream msg;
        msg << "Increasing skip to " << skip << ends;
        server->message(msg.str());
        delete msg.str();
#endif
      }
      // We have moved back into our target range, can send more data now.
      else if (delaysNum > 0 && avg <= targetDelay && skip > 1) {
        skip--;

#ifdef SKIP_MESSAGES
        ostrstream msg;
        msg << "Decreasing skip to " << skip << ends;
        server->message(msg.str());
        delete msg.str();
#endif
      }

      // reset for next time.
      delaysNum = 0;
      adjustSkipTimer.set();
    }

    adjustSkipTimer.clock();
  } // adjustSkip


  borrowedSkip = False;
  skipTimer.clock();
}



void Connection::borrow_skip_time() {
  // skipTimer.get_remaining() == skip only happpens here if we just borrowed last
  // turn.
  // For each Server turn, allocate(), clock(), borrow().
  // Also, don't bother borrowing if we will send next turn anyway.
  if (skipTimer.get_remaining() < skip && !skipTimer.ready()) {
    skipTimer.set(skipTimer.get_remaining() + skip);
    borrowedSkip = True;
  }
}



Turn Connection::targetDelay = DEFAULT_TARGET_DELAY;



Boolean Connection::echoTurnWindows = False;



Server::Server(Boolean lHuman,char *portName,LocatorP errLocator) {
  // Hack, using errLocator for more than reporting errors.
  errLocator->set_remember_deleted(True);
  errLocator->set_remember_sounds(True); 
  
  turn = 0;
  running = False;
  errorLocator = errLocator;
  udpIn = NULL;
  localHuman = lHuman;
  tcpSock = 0;
  udpSock = 0;
  worldVersion = 0; // first valid version
  XETP::check_sizes();

  if (portName) {
    port = Utils::atoi(portName);
    if (port <= 0) {
      error("Invalid port ",portName,".");
      return;
    }
  }
  else {
    port = XETP::DEFAULT_PORT;
  }

  // Don't do anything that could cause an error until Server::run().
}



void Server::run() {
  // All actual network calls, creating and binding sockets, listen, etc.
  //
  // Server object must still be usable even if we return from an error.

#if X11
  signal(SIGPIPE,SIG_IGN);
#endif

  if (gethostname(hostName,R_NAME_MAX)) {
    error("Unable to get hostname of local machine.");
    return;
  }


  tcpSock = socket(AF_INET, SOCK_STREAM, 0);
  if (tcpSock < 0) {
    error("Error opening TCP socket for incoming connections.");
    return;
  }
  udpSock = socket(AF_INET, SOCK_DGRAM, 0);
  if (udpSock < 0) {
    error("Error opening UDP socket for incoming data.");
    return;
  }

  // Bind server addresses
  CMN_SOCKADDR_IN serverAddr;
  memset((void *)&serverAddr,'\0',sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serverAddr.sin_port = htons(port);

  // Give address to both the TCP and UDP sockets.
  if (bind(tcpSock, (CMN_SOCKADDR *)&serverAddr, sizeof(serverAddr)) < 0) {
    ostrstream str;
    str << "Couldn't bind socket name to TCP socket on port " 
        << port << "."  << ends;
    error(str.str());
    delete str.str();
    return;
  }
  if (bind(udpSock, (CMN_SOCKADDR *)&serverAddr, sizeof(serverAddr)) < 0) {
    ostrstream str;
    str << "Couldn't bind socket name to UDP socket on port " 
        << port << "."  << ends;
    error(str.str());
    delete str.str();
    return;
  }

  // Listen for connections on the TCP socket.
  listen(tcpSock,5);
  
  
  // UDP in is shared by all connections.
  udpIn = new UDPInStream(udpSock,True);
  assert(udpIn);

  // Message to server log.
  strstream msg;
  msg << "Set up server on port " << port << ends;
  message(msg.str());
  delete(msg.str());

  running = True;
}



Server::~Server() {
  message("Exiting.");

  // Hack, using errLocator for more than reporting errors.
  errorLocator->set_remember_deleted(False);
  errorLocator->set_remember_sounds(False); 

  for (int n = 0; n < connections.length(); n++) {
    delete get_connection(n);
  }
  delete udpIn;
  CLOSE_SOCKET(tcpSock);
}



Boolean Server::ok() {
  return running;
}



RoleType Server::get_type() {
  return R_SERVER;
}



GameStyleType Server::get_default_game_style() {
  return EXTENDED;
}



int Server::get_humans_num() {
  return connections.length() + (localHuman ? 1 : 0);
}



int Server::num_viewports_needed() {
  if (localHuman) {
    return 1;
  }
  else {
    return 0;
  }
}



void Server::set_humans_num(int) {
}



void Server::human_created(IGameManagerP manager,
                           HumanP human,int num,
                           WorldP,LocatorP locator) {
  if (!running) {
    return;
  }

  // human should already be registered.
  IntelId hId = human->get_intel_id();
  assert(locator->lookup(hId));

  // Use first human created as the local human.
  // Don't need to push this data to any of the clients.
  if (localHuman && num == 0) {
    localHumanId = hId;

    // Log the human's name.
    strstream msg;
    msg << "Player 0 \"" << human->get_name() << '\"' << "@SERVER" << ends;
    message(msg.str());
    delete(msg.str());

    return;
  }

  // Find a connection to assign this human to.
  int n;
  Connection* cn = NULL;
  for (n = 0; n < connections.length(); n++) {
    cn = get_connection(n);
    if (locator->lookup(cn->get_human()) == NULL) {
      // Done.
      break;
    }
  }
  if (n == connections.length()) {
    error("Trying to assign a human when all connections already have one.");
    return;
  }
  assert(cn);

  // If client provided us with a name, set it on the newly created Human.
  if (Utils::strlen(cn->get_human_name())) {
    human->set_name(cn->get_human_name());
  }
  
  // Assign this human to connection n and send the info to the client.
  cn->set_human(human->get_intel_id());

  // Log all the players in the game, some code duplication for the
  // local human.
  ostrstream str;
  char* fullName = cn->get_full_client_name();
  str << "Player " << num << " " << fullName << ends;
  message(str.str());
  delete fullName;
  delete str.str();
  
  // Send assign_intel via TCP
  OutStreamP out = cn->get_tcp_out_stream();
  XETP::send_human(out,human);


  // Tell all connections that a new player has joined if we haven't already.
  // Have to do it here because we didn't know the player's name until now.
  if (!cn->sent_creation_message()) {
    int m;
    int timeMS = quantaToMS(2 * UI_ARENA_MESSAGE_TIME,manager);

    // Don't bother sending message to new player, it'll probably get eaten
    // anyway.
    for (m = 0; m < connections.length() && m != n; m++) {
      char* fullName = cn->get_full_client_name();
      ostrstream str;
      str << fullName << " has joined the game" << ends;
      delete fullName;

      OutStreamP out = cn->get_udp_out_stream();
      XETP::send_arena_message(out,timeMS,str.str());
      delete str.str();
    }
    cn->creation_message_sent();
  }
}



void Server::clock(IGameManagerP manager,WorldP world,LocatorP locator) {
  if (!check_alive()) {
    return;
  }
  
  delete_dead_connections(manager,world,locator);

  // Try to allocate turn window for each connection.
  Boolean* turnWindowAllocated = NULL;
  if (connections.length()) {
    turnWindowAllocated = new Boolean[connections.length()];
  }
  int n;
  for (n = 0; n < connections.length(); n++) {
    Connection* cn = get_connection(n);
    turnWindowAllocated[n] = cn->allocate_turn_window(turn);
  }
  
  send_pings(turnWindowAllocated);

  send_objects(locator,turnWindowAllocated); // calls send_sounds()
  
  send_recently_deleted(locator,turnWindowAllocated);
  
  // Doesn't use turnWindowAllocated, only sends when changed
  // or every SEND_HUMAN_TIME turns anyway.
  send_humans(locator);
  
  // Clock the connections.
  // If you move this, check that borrow_skip_time() is still correct.
  for (n = 0; n < connections.length(); n++) {
    Connection* cn = get_connection(n);
    cn->clock(turnWindowAllocated[n]);
  }

  flush_connections();

  delete [] turnWindowAllocated;

  // Turn is not incremented until yield() is done.
}



Boolean Server::check_alive() {
  if (!running) {
    return False;
  }
  
  if (!udpIn->alive()) {
    error("UDP input stream is dead.  Shutting down server.");
    running = False;
    //    manager->quit();
    return False;
  }

  return True;
}



// Caller should use Server::message() to log the reason for deleting the
// connection.  This function will inform the clients, however.
void Server::delete_dead_connections(IGameManagerP manager,WorldP world,
                                     LocatorP locator) {
  // Kill off dead connections.  May reorder list.
  int n = 0;
  while (n < connections.length()) {
    // Connection has not responded to pings, so kill it.
    Boolean tooOld = False;
    Connection* cn = get_connection(n);

    // Kill off connections that don't respond to our pings.
    if (autoDisconnect) {
      assert(turn >= cn->get_last_activity());
      Quanta diff = (Quanta)(turn - cn->get_last_activity());
      if (diff >= MSToQuanta(noActivityDisconnect,manager) &&
          // Give the connection some slack when it is first starting up.
          turn - cn->get_start_turn() >= MINIMUM_CUTOFF) {
        char* fullName = cn->get_full_client_name();
        ostrstream str;
        str << fullName
            << " has not responded in "
            << diff << " turns.  Disconnect." << ends;
        message(str.str());
        delete fullName;
        delete str.str();
        tooOld = True;
      }
    }
    
    Boolean isDead = False;
    if (!cn->alive()) {
      char* fullName = cn->get_full_client_name();
      message("Lost connection to ",fullName,".");
      delete fullName;
      isDead = True;
    }

    // Kill off a connection.
    if (isDead || tooOld) {

      // Try to tell connection that it is about to die.
      XETP::send_disconnect(cn->get_udp_out_stream());

      delete_connection(manager,world,locator,n);
    }
    else {
      n++;
    }
  }
}



void Server::send_pings(Boolean turnWindowAllocated[]) {
  // If we haven't heard from a Client in a while, send a PING.
  for (int n = 0; n < connections.length(); n++) {
    if (turnWindowAllocated[n]) {
      Connection* cn = get_connection(n);

      Turn diff = turn - cn->get_last_activity();
      if (diff == NO_ACTIVITY_WARN ||
          diff == NO_ACTIVITY_WARN_2 ||
          diff == NO_ACTIVITY_WARN_3) {
        if (echoPingPong) {
          ostrstream str;
          str << "Haven't heard from " << cn->get_client_name()
              << " in " << diff << " turns, sending PING." << ends;
          message(str.str());
          delete str.str();
        }
        XETP::send_ping(cn->get_udp_out_stream());
      }

      // Timer pings sent in Connection::clock().
    }
  }
}



// Send all objects to all clients via UDP
void Server::send_objects(LocatorP locator,Boolean turnWindowAllocated[]) {
  // Keep track so we can clear the NetData::sent_flag.
  PtrList alreadySent;

  int n;
  for (n = 0; n < connections.length(); n++) {
#if 0
    // kept around for send_recently_deleted()
    turnWindowAllocated[n] = connections[n]->allocate_turn_window(turn);
#endif
    Connection* cn = get_connection(n);

    OutStreamP stream = cn->get_udp_out_stream();

    // Prepare for use.
    alreadySent.clear();

    // Find Physical of Human associated with the viewport.
    PhysicalP humanPhysical = NULL;
    IntelP human = locator->lookup(cn->get_human());
    if (human) {
      humanPhysical = locator->lookup(human->get_id());
    }

    // Update the viewport for this connection.
    if (humanPhysical) {
      Area area = humanPhysical->get_area();
      Pos pos = area.get_middle();
    
      const ViewportInfoP vInfo = cn->get_viewport_info();
      ViewportRegion viewport = vInfo->get_viewport_for(pos);

      // Grow viewport by a bit to help with edge cases.
      viewport.grow_by(OL_GRID_SIZE_MAX);
      cn->set_viewport(viewport);

      // HumanPhysical not special anymore.
    }


    // Call to allocate_turn_window() is above.
    if (turnWindowAllocated[n]) {
      // Could put this in its own phase.
      send_sounds(locator,n);

      // Send all objects that overlap the viewport.
      const ViewportRegionP viewport = cn->get_viewport();

      // First pass, send all Creatures and their followers.
      //
      // WARNING: Making the assumption that only Creatures can have 
      // followers and that Creatures are never followers.  
      // If this is broken, we will still send everything, but
      // followers may be sent in different packets that their followees.
      // <<<<not true anymore>>>>
      PhysicalP p;
      PhysicalIter iter(*locator);
      while (p = iter()) {
        if (p->is_creature()) {
          NetDataP netData = p->get_net_data();
          assert(!netData->get_sent_flag());  // Should have cleared it last turn.
          
          // Uses should_send_object().
          check_send_object_and_followers(locator,stream,viewport,
                                          p,turn,alreadySent,
                                          human);
        }
      }

      // Second pass, send all other objects that are not Creatures or their
      // followers.
      PhysicalIter iter2(*locator);
      while (p = iter2()) {
        NetDataP netData = p->get_net_data();
        if (!netData->get_sent_flag()) {
          // Former bug, weren't checking on the second pass.
          if (should_send_object(locator,viewport,p,human)) {
            XETP::send_object(stream,p,turn,locator->compute_tick_type(p,human));
            // Don't bother setting and clearing the sent flag at this point.
          }
        }
      }
    } // if turnWindowAllocated

    // Now clear the sent flag for next time.
    // Must do this even if not turnWindowAllocated, still have the 
    // humanPhysical and its followers.
    int n;
    for (n = 0; n < alreadySent.length(); n++) {
      PhysicalP p = (PhysicalP)alreadySent.get(n);
      NetDataP netData = p->get_net_data();
      if (!netData->get_sent_flag()) {
        cerr << "WARNING: Server::send_objects(): sentFlag is False for an object on the "
             << "alreadySent list." << endl;
      }
      netData->set_sent_flag(False);
    }
  } // for connections.length()
}



void Server::check_send_object_and_followers(LocatorP locator,OutStreamP out,
                                             const ViewportRegionP viewport,
                                             PhysicalP p,Turn turn,
                                             PtrList &alreadySent,IntelP relativeTo) {
  // If we don't send the main object, don't worry about the followers here,
  // they'll be taken care of in the second pass.
  if (!should_send_object(locator,viewport,p,relativeTo)) {
    return;
  }


  PtrList followers;
  p->get_followers(followers);

  // Apply the should_send_object() criteria to all followers.
  int m;
  for (m = 0; m < followers.length();) {
    PhysicalP follower = (PhysicalP)followers.get(m);
    if (!follower->is_creature() && 
        // For composite objects, we may have a Creature follower, 
        // don't send it in the same packet.
        should_send_object(locator,viewport,follower,relativeTo)) {
      // This one is ok, go to next element.
      m++;
    }
    else {
      followers.del(m);
      // copies last element to element m, so we don't want to
      // increment m
    }
  }


  // Record all objects that we send in alreadySent, so we can clear the
  // sent flag.
  alreadySent.add(p);
  alreadySent.append(followers);

  // Mark all objects that we send with the sent_flag.
  NetDataP netData = p->get_net_data();
  assert(!netData->get_sent_flag());
  netData->set_sent_flag(True);
  for (m = 0; m < followers.length(); m++) {
    PhysicalP q = (PhysicalP)followers.get(m);
    netData = q->get_net_data();
    assert(!netData->get_sent_flag());
    netData->set_sent_flag(True);
  }

  // p has some followers, send p and all followers in the same packet.
  if (followers.length() > 0) {
    PtrList tickTypes(followers.length() + 1);
    for (int n = 0; n < followers.length(); n++) {
      PhysicalP follower = (PhysicalP)followers.get(n);
      TickType tt = locator->compute_tick_type(follower,relativeTo);
      tickTypes.add((void *)tt);
    }
    // Reuse followers list to be the list of {p and all followers}.
    followers.add(p);
    tickTypes.add((void *)locator->compute_tick_type(p,relativeTo));
    XETP::send_objects(out,followers,turn,tickTypes);
  }
  // Just send single object, don't have to allocate any data.
  else {
    XETP::send_object(out,p,turn,locator->compute_tick_type(p,relativeTo));
  }
}



Boolean Server::should_send_object(LocatorP locator,const ViewportRegionP viewport,
                                   PhysicalP p,IntelP relativeTo) {
  // Don't bother with unmapped objects.
  if (!(p->get_mapped() || p->get_flash())) {
    return False;
  }

  // Send object iff it is in the viewport.
  Area area = p->get_area();
  // Object must at least be in the viewport's periphery.
  if (viewport->peripheralOverlap(area)) {
    // We send tickType for all objects, let client decide whether to
    // draw actual object, or just a tick.
    TickType tt = locator->compute_tick_type(p,relativeTo);

    // Object must either be in the visible region of the viewport or
    // be in the periphery and show up as a tick mark.
    if (tt != TICK_MAX || 
        viewport->visibleOverlap(area)) {
      return True;
    }
  }
  return False;
}



void Server::send_sounds(LocatorP locator,int n) {
  Connection* cn = get_connection(n);

  // Send recent sounds.
  if (cn->want_sounds()) {
    int m;
    for (m = 0; m < locator->get_sounds_num(); m++) {
      SoundRequest req;
      locator->get_sound(req,m);
      OutStreamP stream = cn->get_udp_out_stream();
      XETP::send_sound_request(stream,req);
    }
  }
}



void Server::send_recently_deleted(LocatorP locator,Boolean turnWindowAllocated[]) {
  // Send any objects that were recently deleted (or recently unmapped).
  int m;
  for (m = 0; m < locator->get_recently_deleted_num(); m++) {
    Id id = locator->get_recently_deleted(m);
    int n;
    for (n = 0; n < connections.length(); n++) {
      Connection* cn = get_connection(n);

      // Only send delete message if we have permission to send this turn.
      if (turnWindowAllocated[n]) {
        OutStreamP stream = cn->get_udp_out_stream();
        PhysicalP p = locator->lookup(id);
        if (p) {
          // p is still around, must have just been unmapped.
          assert(!p->get_mapped());
          XETP::send_object(stream,p,turn,TICK_MAX);
        }
        else {
          XETP::send_delete_object(stream,id);
        }
      }
    }
  }
  // Would be nice to do this on a per-connection basis, so could still send
  // delete message even if turn is skipped.  Not worth the cost right now.
  locator->reset_recently_deleted();
  // Same for sounds.
  locator->reset_sounds();
}



void Server::send_humans(LocatorP locator) {
  // Send each human to the corresponding client via UDP
  // connection number n is human number n.
  int n;
  for (n = 0; n < connections.length(); n++) {
    Connection* cn = get_connection(n);
    HumanP human = (HumanP)locator->lookup(cn->get_human());
    
    Timer &humanTimer = cn->send_human_timer();
    if (human && (human->intel_status_changed() || humanTimer.ready())) {
      OutStreamP out = cn->get_udp_out_stream();
      XETP::send_human(out,human);
      
      // just to reset intel_status_changed.
      // We should probably just be sending the intelStatus.
      human->get_intel_status();
      humanTimer.set(SEND_HUMAN_TIME);
    }
    humanTimer.clock();
  }
}



void Server::flush_connections() {
  // Flush each connection.
  int n;
  for (n = 0; n < connections.length(); n++) {
    Connection* cn = get_connection(n);
    UDPOutStreamP udpOut = cn->get_udp_out_stream();
    udpOut->flush();
  }
}



void Server::yield(CMN_TIME startTime,int quanta,
                   IGameManagerP manager,ITurnStarterP turnStarter,
                   WorldP w,LocatorP l) {
  if (!running) {
    Role::yield_time(startTime,quanta);
    
    // Start timing for next turn.
    turnStarter->start_turn();
    return;
  }

  // Decide how much time to throw away.
  struct timeval waitTime;
  compute_remaining(waitTime,startTime,quanta);
  

  // Right now, we don't check for data coming in on any of the tcp input
  // streams.

  // Loop until all time for this turn thrown away. Call at least once.
  do {
    fd_set fdvar;
    FD_ZERO(&fdvar);
    FD_SET(tcpSock,&fdvar); // acceptor socket
    FD_SET(udpSock,&fdvar); // common UDP socket
    u_int sockMax = Utils::maximum(tcpSock,udpSock);

#if 0
    int n;
    // individual TCP sockets.
    for (n = 0; n < connections.length(); n++) {
      sockMax = Utils::maximum(sockMax,connections[n]->get_tcp_socket());
      FD_SET(connections[n]->get_tcp_socket(),&fdvar);
    }
#endif
    sockMax++; // select() claims to want 1 greater than max number.

    int cond = CMN_SELECT(sockMax,&fdvar,NULL,NULL,&waitTime);
    if (cond > 0) {
      // If there is ANY data, be sure to read ALL of it before starting a 
      // new turn.
      Boolean moreToRead;
      do {
        moreToRead = False;

        // TCP connection coming in
        if (FD_ISSET(tcpSock,&fdvar)) {
          accept_connection(manager,w,l);
        }

        // UDP data coming in
        if (FD_ISSET(udpSock,&fdvar)) {
          receive_udp_data(manager,w,l);
        }


        // See if we should keep reading data.
        FD_ZERO(&fdvar);
        FD_SET(tcpSock,&fdvar); // acceptor socket
        FD_SET(udpSock,&fdvar); // common UDP socket
        // sockMax should still be the same

        // Don't block for any time.          
        struct timeval pollTime;
        pollTime.tv_sec = 0;
        pollTime.tv_usec = 0;              
        int cond2 = CMN_SELECT(sockMax,&fdvar,NULL,NULL,&pollTime);
        if (cond2 > 0) {
          moreToRead = True;
        }
      } while (moreToRead);

      // See how much more, if any, time we still need to throw away.
      compute_remaining(waitTime,startTime,quanta);
    }
    // Error or time expired.
    else {
      // Be sure to drop out of the outer while loop.
      waitTime.tv_usec = 0;
    }
  } while (waitTime.tv_usec > 0);

  // Now we increment turn, right before drawing.
  turn++;

  check_show_stats(startTime);

  turnStarter->start_turn();
}



void Server::new_level(IGameManagerP manager,
                       WorldP world,LocatorP locator) {  
  if (!running) {
    return;
  }

  // Flush all messages to the clients.  So, the client users know what the
  // fuck is going on when a new level starts.
  // The clients will receive the messages and arena_messages twice in
  // this case, but who cares.
  handle_messages(manager,locator);


  // At this point, world has already been updated to the new map.
  
  // Mark new version of world.
  worldVersion++;

  // Send new world to all connections via TCP
  int n;
  for (n = 0; n < connections.length(); n++) {
    Connection* cn = get_connection(n);
    if (cn->alive()) {
      // Give the new client the current world via TCP.
      OutStreamP out = cn->get_tcp_out_stream();
      XETP::send_new_world(out,world,worldVersion);

      UDPOutStreamP udpOut = cn->get_udp_out_stream();
      udpOut->flush();
    }
  }
}



void Server::reset(IGameManagerP manager) {
  if (!running) {
    return;
  }

  message("New game (",
          Utils::game_style_to_string(manager->get_game_style_type()),
          ").");

  // Disassociate all humans with the connections.
  // This is done automatically because locator has been reset.
  
  // Tell all connections that game has reset via TCP.
  int n;
  for (n = 0; n < connections.length(); n++) {
    Connection* cn = get_connection(n);
    if (cn->alive()) {
      OutStreamP stream = cn->get_tcp_out_stream();
      XETP::send_reset(stream,manager->get_game_style_type());

      UDPOutStreamP udpOut = cn->get_udp_out_stream();
      udpOut->flush();
    }
  }
}



void Server::handle_messages(IGameManagerP manager,LocatorP l) {
  // Careful changing this, it is also used internally by 
  // Server::new_level() to flush messages to the clients.

  if (!running) {
    return;
  }

  // We don't need to worry about sending duplicates because Game will
  // clear out all messages every turn.

  // Regular messages
  char *msg = l->peek_most_recent_message();
  if (msg) {
    for (int n = 0; n < connections.length(); n++) {
      Connection* cn = get_connection(n);
      UDPOutStreamP out = cn->get_udp_out_stream();
      XETP::send_message(out,msg);
    }
  }

  // Arena messages
  for (int n = 0; n < connections.length(); n++) {
    Connection* cn = get_connection(n);
    ArenaMessageIter iter(*l,cn->get_human());
    char *msg;
    Boolean propagate = False;
    while (msg = iter.next(propagate)) {
      // Careful to send only messages that are marked to be propagated.
      if (propagate) {
        UDPOutStreamP out = cn->get_udp_out_stream();
        
        // Send number of milliseconds to display message.
        int timeMS = quantaToMS(UI_ARENA_MESSAGE_TIME,manager);
        XETP::send_arena_message(out,timeMS,msg);
      }
    }
  }
}



void Server::send_chat_request(LocatorP locator,const char* receiver,const char* message) {
  if (localHuman) {
    IntelP sender = locator->lookup(localHumanId);
    if (!sender) {
      // If localHuman isn't around for some reason, don't send the message.
      return;
    }
    _process_chat_request(locator,sender,receiver,message);
  }
  else {
    // Send message from the SERVER.
    _process_chat_request(locator,NULL,receiver,message);
  }
}



void Server::set_humans_playing(int num) {
  if (!running) {
    return;
  }
  for (int n = 0; n < connections.length(); n++) {
    Connection* cn = get_connection(n);
    XETP::send_humans_playing(cn->get_udp_out_stream(),num);
  }
}



void Server::set_enemies_playing(int num) {
  if (!running) {
    return;
  }
  for (int n = 0; n < connections.length(); n++) {
    Connection* cn = get_connection(n);
    XETP::send_enemies_playing(cn->get_udp_out_stream(),num);
  }
}



// Errors and messages come through here.
void Server::_error(const char *msg) {
  // Put to stdout to assist logging.
  // Use standard out for both errors and messages on UNIX, not stderr.
#if X11
  // If we can't get the timestamp for any reason, just don't put it out.
  time_t calTime = time(NULL);
  if (calTime != -1) {
    char *timeStr = Utils::strdup(ctime(&calTime));
    if (timeStr) {
      int len = strlen(timeStr);
      if (len > 0) {
        // Kill the new line if it exists.
        if (timeStr[len - 1] == '\n') {
          timeStr[len - 1] = '\0';
        }
        cout << "[" << timeStr << "] ";
      }
    }
  }
  cout << msg << endl;
#endif

  // Careful to set the propagate flag to False so the message doesn't get
  // sent to the clients.
  if (errorLocator) {
    ostrstream str2;
    str2 << "SERVER: " << msg << ends;
    errorLocator->arena_message_enq(str2.str(),NULL,ROLE_FAILED_TIME,False);
    // Don't delete str2.str(), give memory to the Locator.

    // Could do a regular message_enq(), but would have to deal with the
    // propagate problem.
  }
}



void Server::display_chat_message(LocatorP l,const char* sender,
                                  const char* msg) {
  // Log the chat message, then let Role handle it.
  strstream logMsg;
  logMsg << '<' << sender << '>' << msg << ends;
  message(logMsg.str());
  delete logMsg.str();

  Role::display_chat_message(l,sender,msg);
}



void Server::accept_connection(IGameManagerP manager,WorldP world,LocatorP l) {
  // There is no longer a limit on the max number of connections.  Could put 
  // user-specified one here.
#if 0
  if (connections.length() >= CONNECTIONS_MAX) {
    error("Already at maximum number of connections.");
    return;
  }
#endif
  

  //--------------- First accept TCP connection -----------//
  
  CMN_SOCKADDR_IN tcpAddr;
  memset((void *)&tcpAddr,'\0',sizeof(tcpAddr));
  CMN_NET_LENGTH length = sizeof(tcpAddr);
  int msgsock = accept(tcpSock,(CMN_SOCKADDR *)&tcpAddr,
                       &length);

  if (length != sizeof(CMN_SOCKADDR_IN)) {
    error("Received client address that is not proper length.");
  }
  
  if (msgsock < 0) {
    error("Error in accept().");
    return;
  }
  // tcpAddr should now be filled in

  
  /* Given it's network address, attempt to lookup the hostname of 
     the connecting machine. */
  char *clientName;
  struct hostent *hostptr = 
    gethostbyaddr((char *)&tcpAddr.sin_addr,
                  sizeof(CMN_SOCKADDR_IN),AF_INET);
  
  if (hostptr) {
    clientName = (char *)hostptr->h_name;
  }
  else {
    error("Warning: Could not look up hostname of connecting machine.");
    // So, use IP address.
    clientName = inet_ntoa(tcpAddr.sin_addr);
  }

  

  //--------------- Now get info from the TCP_CONNECT -----------//
  InStreamP tcpIn = new NetInStream(msgsock,True);
  u_short udpPort = 0;
  char humanName[IT_STRING_LENGTH];
  Boolean udpPortSet = False;
  ViewportInfo vInfo;
  int skip;
  Boolean wantSounds;
  while (!udpPortSet) {
    // Really should have a timeout here.

    message("Waiting for connection to complete from ",clientName);
    u_short method;
    u_int length;
    if (XETP::receive_header(tcpIn,method,length)) {
      if (method != XETP::TCP_CONNECT) {
        error("Received incorrect XETP method when expecting TCP_CONNECT.");
        // Throw away incoming data.
        u_char *buf = new u_char[length];
        tcpIn->read(buf,length);
        delete []buf;
        continue;
      }
      udpPort = tcpIn->read_short();
      udpPortSet = True;
      Utils::string_read(tcpIn,humanName,IT_STRING_LENGTH);
      vInfo.read(tcpIn);
      skip = tcpIn->read_int();
      wantSounds = (Boolean)tcpIn->read_char();
    }
    else {
      // XETP header didn't come in
      error("Bad XETP header when connecting.  Abort connection.");
      delete tcpIn;
      return;
    }
  }

  // If we already have a connection to this machine, kill the old one.
  int n;
  for (n = 0; n < connections.length(); n++) {
    Connection* cn = get_connection(n);
    CMN_SOCKADDR_IN *addr = cn->get_udp_client_address();
    if (addr->sin_addr.s_addr == tcpAddr.sin_addr.s_addr &&
        addr->sin_port == udpPort) {
      message("New connection from ",cn->get_client_name(),
              ", kill the old one.");
      
      delete_connection(manager,world,l,n);
      // to counteract the increment, delete_connection changes 
      // connections.length()
      n--; 

      // Could break here, but might as well check 'em all.
    }
  }


  // Create memory for new client address.
  CMN_SOCKADDR_IN *udpAddr = new CMN_SOCKADDR_IN;
  assert(udpAddr);
  // Copy TCP address.
  memcpy(udpAddr,&tcpAddr,sizeof(CMN_SOCKADDR_IN));
  // Use port given by client.
  udpAddr->sin_port = htons(udpPort);

  // Log the connection.
  strstream msg;
  msg << clientName <<
	" connected (TCP port " << ntohs(tcpAddr.sin_port) << 
    ", UDP port " << udpPort << ")" << ends;
  message(msg.str());
  delete(msg.str());

  Connection* cn =   
    new Connection(this,errorLocator,
                   clientName,humanName,msgsock,
                   tcpIn,udpSock,udpAddr,turn,
                   vInfo,skip,wantSounds);
  connections.add(cn);

  
  // Give the new client the current world via TCP.
  OutStreamP out = cn->get_tcp_out_stream();
  XETP::send_new_world(out,world,worldVersion);

  // Start a new game.
  XETP::send_reset(out,manager->get_game_style_type());

  // Say hello to the new client.
  ostrstream greeting;
  greeting << "Welcome to " << hostName << ".";

  if (get_humans_num() == 1) {
    greeting << " You are the only player.";
  }
  else {
    greeting << " There are now " 
             << get_humans_num() << " players.";
  }
  greeting << ends;
  int timeMS = quantaToMS(UI_ARENA_MESSAGE_TIME,manager);  
  XETP::send_arena_message(out,timeMS,greeting.str());
  delete greeting.str();

  manager->humans_num_incremented();
}



void Server::delete_connection(IGameManagerP manager,WorldP,LocatorP l,int n) {
  assert(n < connections.length());
  Connection* cn = get_connection(n);
  IntelP intel = l->lookup(cn->get_human());
  int kills = 0;
  if (intel) {
    assert(intel->is_human());
    HumanP human = (HumanP)intel;
    // If the human has infinite lives, take soups into account.
    if (human->get_lives() == IT_INFINITE_LIVES) {
      kills = human->get_human_kills() - human->get_soups();
    }
    else {
      kills = human->get_human_kills();
    }
  }

  // Tell everyone that a connection has been removed.
  int timeMS = quantaToMS(2 * UI_ARENA_MESSAGE_TIME,manager);
  for (int m = 0; m < connections.length(); m++) {
    if (m != n) {
      Connection* cm = get_connection(m);

      char* fullName = cn->get_full_client_name();
      ostrstream str;
      str << fullName << " has disconnected";
      if (intel) {
        str << ", " << kills << " human kills";
      }
      str << ends;
      XETP::send_arena_message(cm->get_udp_out_stream(),
                               timeMS,str.str());


      delete fullName;
      delete str.str();
    }
  }

  manager->humans_num_decremented(cn->get_human());

  delete cn;
  connections.del(n);
}



void Server::receive_udp_data(IGameManagerP manager,
                              WorldP world,LocatorP locator) {
  CMN_SOCKADDR_IN clientAddr;
  udpIn->prepare_packet(&clientAddr);


  // Keep reading XETP packets until the UDP packet is empty.
  int bytesRemaining;
  while ((bytesRemaining = udpIn->bytes_remaining()) > 0) {
    // Get header from client
    u_short method;
    u_int bodyLen;
    if (!XETP::receive_header(udpIn,method,bodyLen)) {
      error("Received bad header.");
      return;
    }

    int n = find_connection(&clientAddr);
    
    // If there is no connection corresponding to the incoming UDP packet.
    if (n == -1) {
      // SERVER_PING is the only method that can come from an address other
      // than a connection.
      if (method == XETP::SERVER_PING) {
        udpIn->done_packet();
        send_udp_server_pong(manager,locator,udpSock,&clientAddr);        
        return;
      }

      error("Received data from unknown client.  Tell it to DISCONNECT.");
      udpIn->done_packet();

      // Create a temp stream to write DISCONNECT to the client.
      UDPOutStream out(udpSock,&clientAddr,False);
      XETP::send_disconnect(&out);
      return;
    }


    // Remember last time something came from a client.
    // Should probably use the Client's idea of what turn it is.
    Connection* cn = get_connection(n);
    cn->set_last_activity(turn);

    switch (method) {
    case XETP::COMMAND:
      process_command(udpIn,n,world,locator);
      break;
    
    case XETP::PONG:
      process_pong(udpIn,n);
      break;

    case XETP::DISCONNECT: {
      // Client is explicitly requesting to be disconnected.
      char* fullName = cn->get_full_client_name();
      message(fullName," has disconnected.");
      delete fullName;
      delete_connection(manager,world,locator,n);
      break;
    }

    case XETP::REQUEST_NEW_HUMAN:
      process_request_new_human(manager,n);
      break;

    case XETP::TIMER_PING: {
      Turn pingTurn = (Turn)udpIn->read_int();
      UDPOutStreamP udpOut = cn->get_udp_out_stream();
      XETP::send_timer_pong(udpOut,pingTurn,turn);
//      udpOut->flush();
    }
      break;

    case XETP::TIMER_PONG:
      cn->process_timer_pong(udpIn);      
      break;

    case XETP::CHAT_REQUEST:
      process_chat_request(udpIn,n,locator);
      break;

    default: 
      error("Server can't process method from UDP packet.");
    }
  } // while bytes remaining

  udpIn->done_packet();
}



#if 0
void Server::receive_tcp_data(IGameManagerP,WorldP,LocatorP,int n) {
  InStreamP in = connections[n]->get_tcp_in_stream();

  u_short method = 0;
  u_int length = 0;
  if (!XETP::receive_header(in,method,length)) {
    message("Received invalid TCP header.");
    return;
  }

  switch (method) {
  case XETP::NAME:
    // Mark the name in the connection, will be used in 
    // Server::human_created().
    Utils::string_read(in,connections[n]->human_name_buffer(),
                       connections[n]->human_name_buffer_length());
    break;

  default: 
      error("Server can't process method from TCP packet.");
  }

}
#endif



void Server::process_command(InStreamP in,int n,
                             WorldP,LocatorP locator) {
  IntelId iId(in);
  ITcommand command = (ITcommand)in->read_char();

  if (Connection::echo_turn_windows()) {
    cout << "Received command " << command << endl;
  }


  if (!in->alive()) {
    return;
  }
  
  Connection* cn = get_connection(n);
  if (iId != cn->get_human()) {
    error("Received command from invalid human id.");
    return;
  }

  IntelP intel = locator->lookup(iId);
  if (!intel) {
    return;
  }

  if (!intel->is_human()) {
    error("Intel associated with a client is non-Human.");
    return;
  }

  // Actually set the command on the human.
  ((HumanP)intel)->set_command(command);

  // Try to borrow some time so that we send the results as soon as possible.
  cn->borrow_skip_time();
}



void Server::process_pong(InStreamP in,int n) {
  Turn clientTurn = (Turn)in->read_int();
  // set_last_activity() should probably go through this, but would have
  // to get turn from every packet from the client.

  Connection* cn = get_connection(n);
  cn->free_turn_windows(clientTurn);

  // Already called set_last_activity() which is the important thing.
  if (echoPingPong) {
    message("Received PONG from ",cn->get_client_name());
  }
}



void Server::process_request_new_human(IGameManagerP manager,int n) {
  Connection* cn = get_connection(n);

  // Weird shit happens if REQUEST_NEW_HUMAN comes in on the same 
  // turn the Connection was created.  Something to do with Locator
  // not being clocked once to add everything.
  if (turn - cn->get_start_turn() > 0) {
    manager->humans_num_decremented(cn->get_human());
    manager->humans_num_incremented();
  }
}



// Only called for CHAT_REQUESTs from clients.
void Server::process_chat_request(InStreamP in,int senderNum,
                                  LocatorP locator) {
  char receiver[IT_STRING_LENGTH];  
  Utils::string_read(in,receiver,IT_STRING_LENGTH);
  char* message = Utils::string_read(in);


  // If the sender doesn't exist anymore, ignore the message.
  Connection* cn = get_connection(senderNum);
  IntelId senderId = cn->get_human(); 
  IntelP sender = locator->lookup(senderId);
  if (!sender) {
    delete message;
    return;
  }
  assert(sender->is_human());

  _process_chat_request(locator,sender,receiver,message);

  delete message;
}



// May be called from CHAT_REQUEST from client, or local request.
void Server::_process_chat_request(LocatorP locator,IntelP sender,
                                   const char* receiver,const char* message) {
  // If a receiver name is specified, only send to that player.
  int rIndex = -1; // Means send to everyone.
  int n;
  if (Utils::strlen(receiver)) {
    for (n = 0; n < connections.length(); n++) {
      Connection* cn = get_connection(n);
      IntelP intel = locator->lookup(cn->get_human());
      if (intel && !Utils::strcmp(intel->get_name(),receiver)) {
        rIndex = n;
        break;
      }
    }
    if (localHuman) {
      IntelP intel = locator->lookup(localHumanId);
      if (intel && !Utils::strcmp(intel->get_name(),receiver)) {
        // Special flag to mean display on Server only.
        rIndex = connections.length();
      }      
    }

    // If we can't find the one unique receiver, don't send to anybody.
    // We could send an error message to the sender.
    if (rIndex == -1) {
      return;
    }
  }


  // Don't send empty messages.
  if (Utils::strlen(message) > 0) {
    const char* senderName;
    IntelId senderId;
    if (sender) {
      senderName = sender->get_name();
      senderId = sender->get_intel_id();
    }
    else {
      senderName = "SERVER";
      // senderId will just be an invalid Id.
    }

    // Special flag to just display on the server.
    if (rIndex == connections.length()) {
      display_chat_message(locator,senderName,message);
      return;
    }

    // Send message to all players between start and end.
    int start = 0;
    int end = connections.length();
    // If message is targeted, just send to that Client.
    if (rIndex != -1) {
      start = rIndex;
      end = rIndex + 1;
    }

    // Maybe we shouldn't send the message back to the sender.
    for (n = start; n < end; n++) {
      Connection* cn = get_connection(n);
      UDPOutStreamP out = cn->get_udp_out_stream();
      XETP::send_chat(out,senderName,senderId,message);
      out->flush();
    }

    // If message goes to everybody display it on the server.
    if (rIndex == -1) {
      display_chat_message(locator,senderName,message);
    }
  }
}



// NOTE: This only works for UDP right now.
int Server::find_connection(CMN_SOCKADDR_IN *clientAddr) {
  for (int n = 0; n < connections.length(); n++) {
    Connection* cn = get_connection(n);
    CMN_SOCKADDR_IN* addr = cn->get_udp_client_address();
    // Check IP address and port.
    if (addr->sin_addr.s_addr == clientAddr->sin_addr.s_addr &&
        addr->sin_port == clientAddr->sin_port) {
      return n;
    }
  }
  return -1;
}



int Server::find_connection(const IntelId& iId) {
  for (int n = 0; n < connections.length(); n++) {
    Connection* cn = get_connection(n);
    if (cn->get_human() == iId) {
      return n;
    }
  }
  return -1;
}



void Server::send_udp_server_pong(IGameManager* manager,LocatorP locator,
                                  CMN_SOCKET udpSock,
                                  CMN_SOCKADDR_IN* destAddr) {
  static char* unknownString = "<unknown>";
  static char* serverString = "<server>";


  // Temporary stream.
  UDPOutStream out(udpSock,destAddr,False);
  
  // Allocate some temporary arrays to pass as arguments to send_server_pong().
  int humansNum = locator->humans_registered();
  const char** names = new constCharP[humansNum];
  const char** clientNames = new constCharP[humansNum];
  int* humanKills = new int[humansNum];
  int* enemyKills = new int[humansNum];
  Id* ids = new Id[humansNum];
  assert(names && clientNames && humanKills && enemyKills && ids);

  char* version = Utils::get_OS_info();

  // Fill the parallel arrays with info about each Human.
  for (int n = 0; n < humansNum; n++) {
    HumanP human = locator->get_human(n);
    assert(human);
    names[n] = human->get_name();

    // Inefficient, we could store the association between humans_registered()
    // and connections as an index.  That would be more prone to error, though.
    int cNum = find_connection(human->get_intel_id());
    if (cNum == -1) {
      clientNames[n] = unknownString;
      if (localHuman && (human->get_intel_id() == localHumanId)) {
        clientNames[n] = serverString;
      }
    }
    else {
      Connection* cn = get_connection(cNum);
      clientNames[n] = cn->get_client_name();
    }

    // If the human has infinite lives, take soups into account.
    if (human->get_lives() == IT_INFINITE_LIVES) {
      humanKills[n] = human->get_human_kills() - human->get_soups();
    }
    else {
      humanKills[n] = human->get_human_kills();
    }
    enemyKills[n] = human->get_enemy_kills();
    // May be an invalid id.
    ids[n] = human->get_id();
  }

  XETP::send_server_pong(&out,manager->get_game_style_type(),
                         locator->enemies_alive(),humansNum,
                         version,
                         names,clientNames,
                         humanKills,enemyKills,ids);
  delete version;
  delete [] names;
  delete [] humanKills;
  delete [] enemyKills;
  delete [] ids;                          

  // Temp UDP stream will close itself here.
}



Boolean Server::autoDisconnect = True;



int Server::noActivityDisconnect = NO_ACTIVITY_DISCONNECT_DEFAULT;

