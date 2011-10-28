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

// "ui.h"  Header for user interface module.  

/* Overview:
Draws the world.  Manages viewports.  Holds x variables.  Follows an intel
if one is registered.  Sends commands to the intel if it is human.  For weapon
commands, tapping key gives most recently pressed direction or IT_CENTER if 
there is none. */

/* NOTE: A Ui is never destroyed.  Allocates memory for transfering to 
callbacks and event handlers. */

#ifndef UI_H
#define UI_H

#define UI_VIEWPORTS_MAX 2

// Include Files
#include <afxtempl.h>
#include "utils.h"
#include "coord.h"
#include "world.h"
#include "locator.h"
#include "id.h"
#include "intel.h"
#include "game_style.h"
#include "ui_cmn.h"


#include "glowpal.h"
#include "fogpal.h"
#include "palmod.h"
#include "firepal.h"

// Data Structures
enum UIinput {UI_KEYS_RIGHT,UI_KEYS_LEFT};
enum UIkeyset {UIsun3, UIdecmips, UIiris, UIncd, UItektronix, UIsun4, UIrsaix,
	       UIsun4_sparc,UImac,UIalpha,UIpc,UIunspecifiedKeyset};

class Ui;
typedef Ui *UiP;

class Intel;
typedef Intel *IntelP;

class CXEvilWnd;
typedef CXEvilWnd *CXEvilWndP;

class XEvilWindowStatsWnd;
typedef XEvilWindowStatsWnd *XEvilWindowStatsWndP;

class StatusWnd;
typedef StatusWnd *StatusWndP;

class FKeysDlg;

class SoundManager;
typedef SoundManager *SoundManagerP;

// First index is an IT_COMMAND, sedond is one of two.
typedef char UIkeymap[UI_KEYS_MAX][2];


// Should move this into ui_cmn.h
class UIsettings {
public:
  enum SOUNDTRACKTYPE{NONE=0,CD=1,WAV=2,MIDI=3};

  int humansNum;
  int enemiesNum;
  Boolean enemiesRefill;
  Boolean pause;
  GameStyleType style;
  Quanta quanta;
  Boolean monoaural;
  Boolean sound;
  Boolean cooperative;
  SOUNDTRACKTYPE musictype;
  Rooms worldRooms;
  int soundvol;//-10000 <-> 0
  int trackvol;//-10000 <-> 0
  int screenmode;
  int difficulty;
  char connectHostname[R_NAME_MAX]; // UIconnectServer
  CMN_PORT connectPort; // UIconnectServer
  // empty string playerName means unspecified.
  char humanName[IT_STRING_LENGTH]; // UIconnectServer, UIrunServer
  CMN_PORT serverPort; // UIrunServer
  Boolean localHuman; // UIrunServer
  char chatReceiver[IT_STRING_LENGTH]; // UIchatRequest
  char chatMessage[UI_CHAT_MESSAGE_MAX + 1]; // UIchatRequest
};


// Class Declarations
class Ui {
public:
  enum {SOUNDTRACKTIMER_CD=666,SOUNDTRACKTIMER_MIDI=667};
  Ui();  //no display names!
  /* EFFECTS: Create user interface with one viewport (number 0) with world w 
  and locator l.  The memory pointed to by displayNames becomes the
  property of the Ui. */

  virtual ~Ui();

  /* NOTE: Must set_keyset on all displays before first clock. 
  Must manually set_* all desired parameters before first reset. */

  virtual int getWindowList(CTypedPtrList<CPtrList,CWnd *>&p_windowlist);
  virtual int get_viewports_num();
  /* EFFECTS: Total number of viewports in the Ui. */

//  virtual int get_viewports_num_on_dpy(int dpyNum);
  /* EFFECTS: Get viewports and number of viewports on a single display.
  The viewports on a display, dpyNum, range from 0 to 
  get_viewports_num_on(dpyNum) - 1. */

  virtual int get_dpy_max();

  virtual UImask get_settings(UIsettings &s);
  /* MODIFIES: s */
  /* EFFECTS:  Get all of the settings that are stored in the Ui.  Mostly the
  menubar stuff.  settingsChanges gives the fields in s that have changed
  since the last call of get_settings.  Other fields are not guaranteed to
  be meaningful.  Initially, nothing is changed. */


  Boolean settings_changed() {return settingsChanges != UInone;}
  /* EFFECTS:  Tells whether the the settings have changed since the last 
  call of get_settings().  Initially is False. */


  virtual void set_humans_num(int);
  virtual void set_enemies_num(int);
  virtual void set_enemies_refill(Boolean);
  virtual void set_style(GameStyleType);
  virtual void set_quanta(Quanta);
  virtual void set_track_volume(int);
  virtual void set_sound_volume(int);
  virtual void set_musictype(UIsettings::SOUNDTRACKTYPE);
  virtual void set_sound_onoff(int);
  virtual void set_world_rooms(const Rooms&);
  virtual void set_humans_playing(int);
  virtual void set_enemies_playing(int);

  virtual void set_level(const char *);

  virtual void set_screen_mode(Xvars::SCREENMODE p_mode);

  virtual Boolean other_input();
  /* EFFECTS: Tells whether there has been any keyboard or mouse press other
  than a valid control since the last clock. */

  virtual void set_input(int vNum,UIinput input);
  /* EFFECTS: Set the input device for viewport num. */

  virtual void set_difficulty(int d);

  virtual void set_keyset(int dpyNum,UIkeyset keyset);
  virtual void set_keyset(int dpyNum,UIkeyset basis,char right[UI_KEYS_MAX][2],
	  char left[UI_KEYS_MAX][2]);
  /* REQUIRES: Must be called at least once before the first clock. */
  /* IMPLEMENTATION NOTE: Sets keycodes[0 and 1] */

  virtual void set_pause(Boolean);
  /* EFFECTS: Pause/unpause. */

  virtual void set_prompt_difficulty();

  virtual void set_cooperative(Boolean);
  /* EFFECTS: Sets the cooperative flag for joint play.*/


  virtual void set_role_type(RoleType);

  
  void unset_prompt_difficulty() {}
  virtual int get_difficulty();
  /* EFFECTS: Similar to set_level_title.  Prompts user for difficulty level.
     get_difficulty() returns user entered difficulty since last call to
     set_prompt_difficulty() or, DIFF_NONE if none set. */

  virtual int add_viewport();
  /* EFFECTS: Add another viewport and return its number. */

  virtual void del_viewport();
  /* EFFECTS: Delete the highest numbered viewport. */

  virtual void register_intel(int n, IntelP intel);
  /* REQUIRES: intel is not already registered with a different viewport. */
  /* EFFECTS: Registers the intel on viewport n.  The viewport will now 
  follow the object represented by intel->get_id() and send commands to 
  it if it is human. */

  virtual void unregister_intel(int n);
  /* REQUIRES: n is a valid viewport */
  /* EFFECTS: Unregister the intel associated with viewport n, if any.  
  Otherwise do nothing. */
    
  virtual IntelP get_intel(int n);
  /* REQUIRES: n is a valid viewport */
  /* EFFECTS: none, just returns intel[n] */

  virtual void demo_reset();
  /* EFFECTS: Like reset() but does not effect whether viewport can be
     scrolled. */

  virtual void reset();
  /* EFFECTS: Clear out all registered intels.  Does NOT remove viewports. */

  virtual void reset_graphics(Xvars::SCREENMODE p_newmode);
  /* USED when ui allready displaying. destroys windows and creates new ones.*/

  virtual void set_redraw_arena();
  /* EFFECTS: Next turn, the Ui must redraw everything. */

  virtual Boolean process_event(int dpyNum,CMN_EVENTDATA eventdata);
  //return True if handled
  /* NOTE: Call callback last so that callback can make changes to self. */

  virtual void pre_clock() = 0;
  /* EFFECTS: Follows the object of the registered intel if any and redraws 
  the world and locator. */

  virtual void post_clock() = 0;

  /* Must use static functions for callbacks because of problems with
  pointers to member functions in some compilers. */

  static IViewportInfo* get_viewport_info();
  /* EFFECTS: Return structure in static memory 
     with info necessary to figure out the viewport around a given point. */

  static void no_init_graphics() {initGraphics = FALSE;}
  /* EFFECTS: If called, don't initialize all graphics at startup. */

  static void check_num_lock();

//  static Boolean get_reduce_draw();

  static void set_reduce_draw(Boolean);


protected:
  UIsettings settings;
  UImask settingsChanges;
  static BOOL initGraphics;
};

#endif
