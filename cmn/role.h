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

// "role.h"

#ifndef ROLE_H
#define ROLE_H

#if X11
	#ifndef NO_PRAGMAS
	#pragma interface
	#endif
#endif


#include "utils.h"
#include "world.h"
#include "locator.h"
#include "streams.h"
#include "intel.h"

// Time to show error message if Role fails.
#define ROLE_FAILED_TIME 200
#define FREE_TURN_WINDOW 0xFFFFFFFF


class IGameManager {
public:
  //  virtual void new_world(const Dim&) = 0;

  virtual void world_changed() = 0;
  /* NOTE: Really just used to force redraw. */

  //  virtual void new_world(InStreamP worldStream) = 0;
  /* EFFECTS: Kill all objects and load new world from stream. */

  virtual void manager_reset(GameStyleType) = 0;
  /* EFFECTS: Start a new game.  Set game style to the new value. */

  virtual void register_intel(int num,IntelP intel) = 0;
  /* EFFECTS: Associate intel with viewport num. */
  /* NOTE: No corresponding unregister_intel() needed yet. */

  virtual void humans_num_incremented() = 0;

  virtual void humans_num_decremented(const IntelId &) = 0;

  virtual void quit() = 0;

  virtual void set_humans_playing(int) = 0;

  virtual void set_enemies_playing(int) = 0;

  virtual Quanta manager_get_quanta() = 0;

  virtual GameStyleType get_game_style_type() = 0;
};
typedef IGameManager *IGameManagerP;



class ITurnStarter {
public:
  virtual void start_turn() = 0;
};
typedef ITurnStarter *ITurnStarterP;



class Role {
public:
  Role();
  
  virtual ~Role();

  virtual Boolean ok() = 0;

  virtual RoleType get_type() = 0;  

  virtual int get_humans_num() = 0;

  virtual GameStyleType get_default_game_style();
  /* DEFAULT: LEVELS */

  virtual int num_viewports_needed();
  /* EFFECTS: The first num_viewports_needed() Humans created will have 
     viewports, the rest will not. */
  /* DEFAULT: Return get_humans_num(). */

  virtual void set_humans_num(int) = 0;

  virtual void human_created(IGameManagerP,HumanP human,int num,
                             WorldP,LocatorP);
  /* EFFECTS:  Called for each human created when a new game starts.  num
   gives the order in which the humans are created.  It should not be used 
   inside the Role for index information. */
  /* DEFAULT: do nothing */     

  virtual void clock(IGameManagerP,WorldP,LocatorP);
  /* DEFAULT: Do nothing */

  virtual void yield(CMN_TIME,int quanta,IGameManagerP,ITurnStarterP,WorldP,
                     LocatorP) = 0;
  /* EFFECTS: Throw away time (in ms).  Guaranteed to be called every turn. */
  
  virtual void new_level(IGameManagerP,WorldP,LocatorP);
  /* EFFECTS: Called when a new level is begun. */
  /* DEFAULT: Do nothing. */

  virtual void reset(IGameManagerP);
  /* EFFECTS: Called when a new game is started. */
  /* DEFAULT: Do nothing. */

  virtual void handle_messages(IGameManagerP,LocatorP);
  /* EFFECTS: Give the Role an oppurtunity to deal with messages and 
     arena_messages in the Locator before they get nuked. */
  /* DEFAULT: Do nothing. */

  virtual void set_humans_playing(int);
  virtual void set_enemies_playing(int);
  /* NOTE: Mostly for updating UI. */
  /* DEFAULT: Do nothing. */

  virtual void game_quitting();
  /* EFFECTS: The game is quitting, perform any shutdown operations 
     necessary. */
  /* DEFAULT: Do nothing. */

  virtual void send_chat_request(LocatorP,const char* receiver,
                                 const char* message);
  /* REQUIRES: receiver is non-NULL */
  /* EFFECTS: The Game (really the Ui) is asking to send a chat message
     to the named receiver, or to everyone if receiver is "". */
  /* DEFAULT: Do nothing. */

  static void show_stats() {showStats = True;}

  static void echo_ping_pong() {echoPingPong = True;}

  // error() and message() are now public, so Game can use them.
  void error(const char *msg1,const char *msg2 = NULL,
             const char *msg3 = NULL);
  /* EFFECTS: Report that something went wrong. */

  void message(const char *msg1,const char *msg2 = NULL,
               const char *msg3 = NULL);
  /* EFFECTS: Generic message to the user. */

  static Boolean uses_humans_num(RoleType);
  static Boolean uses_cooperative(RoleType);
  static Boolean uses_enemies_num(RoleType);
  static Boolean uses_enemies_refill(RoleType);
  static Boolean uses_game_style(RoleType);
  static Boolean uses_difficulty(RoleType);
  static Boolean uses_chat(RoleType);
  /* EFFECTS: These are all so the Ui can decide whether to enable certain 
     items or not depending on the RoleType. */

  
#ifndef PROTECTED_IS_PUBLIC
protected:
#endif
  int compute_remaining(struct timeval &waitTime,
                         CMN_TIME startTime,int quanta);
  /* EFFECTS: Fill waitTime with the amount of time remaining(usec) until 
     quanta
     has expired.  Also return amount remaining in milliseconds. */

  virtual void _error(const char *msg1);
  /* NOTE: Only need to override this function. */
  /* DEFAULT: Print to stdout. */

  virtual void _message(const char *msg);
  /* DEFAULT: Call _error(msg) */
                        
  void check_show_stats(CMN_TIME startTime);
  /* EFFECTS: Show statistics for this turn if show_stats() is enabled. */

  int quantaToMS(Quanta q,IGameManagerP man) ;
  Quanta MSToQuanta(int val,IGameManagerP man);
  /* EFFECTS: Use these for converting between real time and local Quanta. */

  void yield_time(CMN_TIME startTime,int quanta);
  /* EFFECTS: Helper function to throw away time since startTime. */

  void display_chat_message(LocatorP,const char* sender,
                            const char* message);
  /* EFFECTS: Display a chat message to the local user. */
  /* NOTE: The method is in Role so that it can be shared by Client and 
     Server.  Not used for StandAlone. */
  
  static Boolean echoPingPong;

  
private:
  static Boolean showStats;
  static Boolean roleExists; // Only one role should exist at once.
};
typedef Role *RoleP;



class StandAlone : public Role {
public:
  StandAlone();

  Boolean ok();

  virtual RoleType get_type();

  virtual int get_humans_num();

  virtual void set_humans_num(int);
  
  virtual void yield(CMN_TIME,int quanta,IGameManagerP,ITurnStarterP,WorldP,LocatorP);


private:
  int humansNum;
};



class Locator;



class Client : public Role {
public:
  Client(char *serverName,char *portName,CMN_PORT clientPortBase,
         char *humanName,
         IViewportInfo*,int skip,LocatorP errorLocator);
  /* EFFECTS: Create a client that will connect to the given server and port.
     If portName is NULL, connect to XETP::DEFAULT_PORT.
     Give error message to cerr if failure and set error flag. 
     Pass in empty string for humanName to use default. 
     clientPortBase is the first local UDP port, Client will try to 
     bind to.  Pass in 0 for clientPortBase to use default. */
  /* NOTES: Locator is only used for reporting errors. */

  virtual ~Client();

  void connect_server();
  /* EFFECTS: Delay connecting until the rest of XEvil is initialized 
     (especially the graphics), so the server doesn't disconnect for no 
     response. */
  /* NOTE: No other metods of Client should be called until 
     after Client::connectServer(). */

  virtual RoleType get_type();
  
  virtual int get_humans_num();

  virtual void set_humans_num(int);

  virtual Boolean ok();

  virtual void clock(IGameManagerP,WorldP,LocatorP);

  virtual void yield(CMN_TIME,int,IGameManagerP,ITurnStarterP,WorldP,LocatorP);
  /* EFFECTS: Look for data from the server. */

  virtual void game_quitting();

  virtual void send_chat_request(LocatorP,const char* receiver,
                                 const char* message);

  void request_new_human();
  /* EFFECTS: Client will ask the Server to give it a new human.  Does not 
     start a new game on the server. */

  const char *get_server_name() {return serverName;}
  
  static void set_human_reflexes(int val) {humanReflexesTimeMS = val;}

  static void disable_dead_reckoning() {deadReckoning = False;}

  
#ifndef PROTECTED_IS_PUBLIC
protected:
#endif
  virtual void _error(const char *msg1);
  virtual void _message(const char *msg);
  /* NOTE: These actually do something different for Client. */
  
  
private:
  enum {CONN_FAILED = -1,CONN_PARTIALLY = 0,CONN_SUCCESS = 1};

  Boolean receive_tcp_data(IGameManagerP,WorldP,LocatorP);
  Boolean receive_udp_data(IGameManagerP,WorldP,LocatorP);
  /* EFFECTS:  Read data off socket and return whether we should start a new
     turn. */
  
  void process_new_world(InStreamP,IGameManagerP,WorldP,LocatorP);
  
  void process_reset(InStreamP,IGameManagerP,WorldP,LocatorP);

  void process_human(InStreamP,OutStreamP,IGameManagerP,WorldP,LocatorP);

  void process_object(InStreamP,WorldP,LocatorP);

  void process_delete_object(InStreamP,WorldP,LocatorP);

  void process_message(InStreamP,int length,LocatorP);

  void process_arena_message(InStreamP,int length,IGameManagerP,LocatorP);

  void process_sound_request(InStreamP,LocatorP);

  void process_chat(InStreamP,LocatorP);
  
  void check_alive(IGameManagerP);


  CMN_SOCKET tcpSock;
  CMN_SOCKET udpSock;
  // Server port.  Same for TCP and UDP
  CMN_PORT port; 
  // First port to try, when choosing client UDP port.
  // Probably should also store the actual clientPort chosen.  Don't need it
  // now, though.
  CMN_PORT clientPortBase;
  char hostName[R_NAME_MAX];
  char serverName[R_NAME_MAX];
  CMN_SOCKADDR_IN serverAddr; // Memory shared by UDPOutStream
  InStreamP tcpIn;
  OutStreamP tcpOut;
  UDPInStreamP udpIn;
  UDPOutStreamP udpOut;
  int connected;
  int worldVersion;
  IntelId humanIntelId;
  Timer humanReflexes;
  static int humanReflexesTimeMS;
  char *humanName; // empty string means use default.
  // Has no direct connection to the server's turn.
  // Keep a count of how long we've been running.
  Turn turn;

  // The highest turn we've ever seen from this server.
  Turn turnMax; 

  IViewportInfo* vInfo;
  int skip;
  LocatorP errorLocator;

  static Boolean deadReckoning;
};
typedef Client *ClientP;



class Server;
typedef Server *ServerP;



// Connections coming in to server.
class Connection {
public:
  enum {
    // The magic value for skip that means dynamically adjust the
    // value of skip.
    ADJUST_SKIP = 0, 
  };
  
  Connection(ServerP server,LocatorP errorLocator,
             char *cName,char *hName,
             CMN_SOCKET tcpSock,InStreamP tcpIn,CMN_SOCKET udpSock,
             struct sockaddr_in* udpClientAddr,Turn now,
             const ViewportInfo &,int skip,Boolean wantSounds);
  /* NOTE: Connection now owns the memory for clientAddr. */
  /* TODO: Change all the parameters to a generic structure to be read in from 
           XETP::TCP_CONNECT, say ConnectParameters. */
  /* NOTE: skip may be ADJUST_SKIP */

  ~Connection();

  Boolean alive() 
  {return tcpIn->alive() && tcpOut->alive() && udpOut->alive();}
  /* EFFECTS: True if all the streams are still alive. */

  InStreamP get_tcp_in_stream() {return tcpIn;}
  OutStreamP get_tcp_out_stream() {return tcpOut;}
  UDPOutStreamP get_udp_out_stream() {return udpOut;}

  CMN_SOCKET get_tcp_socket() {return tcpSock;}
  
  const char* get_client_name() {return clientName;}

  char* get_full_client_name();
  /* EFFECTS: For server messages, returns something like
     "billy-bob"@my.host.com". */
  /* NOTE: Caller must free returned memory with delete. */

  struct sockaddr_in *get_udp_client_address() {return udpClientAddr;}
  
  IntelId get_human() {return human;}
  /* EFFECTS: Return the id of the human associated with this connection. */

  // Timer &get_skip_timer() {return skipTimer;}

  Boolean sent_creation_message() {return sentCreationMessage;}
  void creation_message_sent() {sentCreationMessage = True;}
  /* EFFECTS: Have we told everybody that this Client joined the game. */

  void set_human(const IntelId &iId) {human = iId;}

  Turn get_start_turn() {return startTurn;}

  Turn get_last_activity() {return lastActivity;}
  void set_last_activity(Turn val) 
  {assert(val >= lastActivity); lastActivity = val;}
  /* EFFECTS: Get/set the last turn when something came in from this 
     connection. */
  
  char *get_human_name() {return humanName;}
  /* NOTE: Empty string means it wasn't set. */

  Timer &send_human_timer() {return humanTimer;}
  /* EFFECTS: Send_human whenever this timer is ready. */

  const ViewportRegionP get_viewport() {return &viewport;}
  void set_viewport(const ViewportRegion &v) {viewport = v;}
  /* EFFECTS: Get/set the viewport the client is looking at. */

  const ViewportInfoP get_viewport_info() {return &vInfo;}

  Boolean want_sounds() {return wantSounds;}
  /* EFFECTS: Does this client want sounds sent to it? */

  Boolean allocate_turn_window(Turn);
  /* EFFECTS: Try to get a window to send the data for the given Turn. */

  void free_turn_windows(Turn turn);
  /* EFFECTS: Free all turn windows for all turns <= turn. */

  static void enable_echo_turn_windows() {echoTurnWindows = True;}  

  static Boolean echo_turn_windows() {return echoTurnWindows;}  

  void process_timer_pong(UDPInStreamP);

  void clock(Boolean turnWindowAllocated);

  void borrow_skip_time();
  /* EFFECTS: Try to borrow some skip time so that
     data will be sent next turn. */

  static void set_target_delay(Turn val) {targetDelay = val;}


private:
  enum {
    TURN_WINDOWS_MAX = 1,
    // Deal with measuring and compensating for network delays.  
    // Unit is turns.
    ADJUST_SKIP_TIME = 75,  // Every 3 seconds.
    // Max number of delays that we will measure in the period of 
    // HANDLE_DELAYS_TIME.
    MEASURED_DELAYS_MAX = 2 * ADJUST_SKIP_TIME,
  };

  ServerP server;

  // For reporting errors.
  LocatorP errorLocator; 

  char clientName[R_NAME_MAX];
  struct sockaddr_in *udpClientAddr; // Address of client
  IntelId human;
  Turn lastActivity;
  Turn startTurn;
  
  // streams
  InStreamP tcpIn;
  OutStreamP tcpOut;
  // udpIn is shared by all connections.
  UDPOutStreamP udpOut;
  CMN_SOCKET tcpSock;

  // Name to be used for the Human associated with the Client.
  char *humanName;

  // So can follow intel properly.
  ViewportInfo vInfo;

  // Variables for implementing skip.
  int skip; // Always > 0.
  Timer skipTimer;
  // Did we borrow some skip time from the next turn.
  Boolean borrowedSkip; 
  
  Boolean wantSounds;

  Boolean sentCreationMessage;

  Timer humanTimer;
  ViewportRegion viewport; // the current viewport.

  // Can be FREE_TURN_WINDOW
  Turn turnWindows[TURN_WINDOWS_MAX];

  static Boolean echoTurnWindows;

  // Variables for dynamically adjusting skip.
  Boolean adjustSkip;
  Turn delays[MEASURED_DELAYS_MAX];
  int delaysNum;
  Timer adjustSkipTimer;
  static Turn targetDelay;
};



class Server : public Role {
  // So can call message() and error().  Maybe we should just make them
  // public.
  friend class Connection; 


public:
  Server(Boolean localHuman,char *portName,LocatorP);
  /* EFFECTS: Create a server that listens for connections on given port.
     If NULL is passed in, use XETP::DEFAULT_PORT. */

  ~Server();

  void run();
  /* EFFECTS: Start the server running.  Should be called after the 
     server is constructed, before any other calls.  But not until the 
     game is ready to start clocking the main loop. */

  virtual Boolean ok();

  virtual RoleType get_type();

  virtual int get_humans_num();

  Turn get_turn() {return turn;}
  // Perhaps should move turn up into role.

  virtual GameStyleType get_default_game_style();

  virtual int num_viewports_needed();

  virtual void set_humans_num(int);

  virtual void human_created(IGameManagerP,HumanP,int,WorldP,LocatorP);

  virtual void clock(IGameManagerP,WorldP,LocatorP);
  /* EFFECTS: Update all clients. */

  virtual void yield(CMN_TIME startTime,int quanta,
                     IGameManagerP,ITurnStarterP,WorldP,LocatorP);

  virtual void new_level(IGameManagerP,WorldP,LocatorP);

  virtual void reset(IGameManagerP);

  virtual void handle_messages(IGameManagerP,LocatorP);

  virtual void send_chat_request(LocatorP,const char* receiver,
                                 const char* message);

  virtual void set_humans_playing(int);

  virtual void set_enemies_playing(int);

  static void disable_auto_disconnect() {autoDisconnect = False;}

  static void set_no_activity_disconnect(int val) 
  {if (val > 0) noActivityDisconnect = val;}
  /* EFFECTS: Set time until disconnecting client to val 
     milliseconds.  Does nothing if disable_auto_disconnect() 
     was called. */


#ifndef PROTECTED_IS_PUBLIC
protected:
#endif
  virtual void _error(const char *msg1);
  /* NOTE: Calls up the tree. */

  void display_chat_message(LocatorP,const char* sender,
                            const char* message);
  /* EFFECTS: Overrides, so can also log the chat message. */
  
  
private:
  Connection* get_connection(int n) {return (Connection*)connections.get(n);}
  /* NOTE: Just to reduce some typing. */

  void accept_connection(IGameManagerP,WorldP world,LocatorP);

  void delete_connection(IGameManagerP,WorldP,LocatorP,int);
  
  void receive_udp_data(IGameManagerP,WorldP,LocatorP);

  void process_command(InStreamP in,int connectionNum,WorldP,LocatorP);

  void process_pong(InStreamP in,int connectionNum);

  void process_timer_pong(InStreamP in,int connectionNum);

  void process_request_new_human(IGameManagerP,int h);

  void process_chat_request(InStreamP,int connectionNum,LocatorP);

  void _process_chat_request(LocatorP,IntelP sender,
                             const char* receiver,const char* message);
  /* EFFECTS: The guts of process_chat_request(), but after reading from
     the stream.  Is reused when Server's localHuman "sends" a 
     chat request to itself. i.e. there is no actual XETP::CHAT_REQUEST 
     message sent, just do everything as if we just received one from 
     a client.  sender may be NULL, in that case send a message from
     <SERVER>. */      

  int find_connection(CMN_SOCKADDR_IN* clientAddr);
  /* EFFECTS: Return index of connection matching clientAddr or -1 if not
     found. */

  int find_connection(const IntelId&);
  /* EFFECTS: Return index of connection with given Human IntelId, or -1
     if not found. */

  void send_udp_server_pong(IGameManager*,LocatorP,
                            CMN_SOCKET udpSock,CMN_SOCKADDR_IN* destAddr);
  /* EFFECTS: Send all the information of a SERVER_PONG to the given address. 
     Use udpSock which is already open and bound. */


  /****** BEGIN PHASES OF CLOCK() ********/
  Boolean check_alive();

  void delete_dead_connections(IGameManagerP,WorldP,LocatorP);

  void send_pings(Boolean []);

  void send_objects(LocatorP,Boolean []);

  void check_send_object_and_followers(LocatorP,OutStreamP,
                                       const ViewportRegionP viewport,
                                       PhysicalP p,Turn,
                                       PtrList &alreadySent,
                                       IntelP relativeTo);
  /* NOTE: Uses should_send_object() to decide for p and all its 
     followers. */
  /* NOTE: Add any objects we send to alreadySent. */

  Boolean should_send_object(LocatorP,const ViewportRegionP viewport,
                             PhysicalP p,IntelP relativeTo);

  void send_sounds(LocatorP,int connectionNum);

  void send_recently_deleted(LocatorP,Boolean []);

  void send_humans(LocatorP);

  void flush_connections();
  /****** END PHASES OF CLOCK() ********/

  
  Boolean running;
  char hostName[R_NAME_MAX];
  CMN_PORT port; // same for TCP and UDP
  CMN_SOCKET tcpSock; // for new incoming connections only
  CMN_SOCKET udpSock; // for all udp data, in and out
  
  UDPInStreamP udpIn; // owns udpSock

  PtrList connections;

  int worldVersion;
  Turn turn;

  // For reporting errors.
  LocatorP errorLocator; 

  // Whether there is a human on the server.
  Boolean localHuman;
  IntelId localHumanId;

  static Boolean autoDisconnect;
  static int noActivityDisconnect; // Number of msec until disconnect.
};

#endif
