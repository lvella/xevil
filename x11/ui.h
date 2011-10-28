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

#ifndef NO_PRAGMAS
#pragma interface
#endif


// Include Files
#include "utils.h"
#include "coord.h"
#include "world.h"
#include "locator.h"
#include "id.h"
#include "intel.h"
#include "game_style.h"
#include "sound.h"
#include "ui_cmn.h"
#include "viewport.h"


#define UI_VIEWPORTS_MAX 6 
#define UI_SHORT_STRING_LENGTH 80


enum UIkeyset {UIsun3, UIdecmips, UIiris, UIncd, UItektronix, UIsun4, UIrsaix,
               UIsun4_sparc,UImac,UIalpha,UIlinux,UIunspecifiedKeyset};

class Ui;
typedef Ui *UiP;

// First index is an IT_COMMAND, sedond is one of two.
typedef KeySym UIkeymap[UI_KEYS_MAX][2];



// Should move all this into ui_cmn.h
class UIsettings {
public:
  int humansNum;
  int enemiesNum;
  Boolean enemiesRefill;
  Boolean pause;
  GameStyleType style;
  Quanta quanta;
  Boolean sound;
  Rooms worldRooms;
  int soundvol;
  int trackvol;
  Boolean cooperative;
  char connectHostname[R_NAME_MAX]; // UIconnectServer
  CMN_PORT connectPort; // UIconnectServer
  char humanName[IT_STRING_LENGTH]; // UIconnectServer, UIrunServer
  CMN_PORT serverPort; // UIrunServer
  Boolean localHuman; // UIrunServer
  char chatReceiver[IT_STRING_LENGTH]; // UIchatRequest
  char chatMessage[UI_CHAT_MESSAGE_MAX + 1]; // UIchatRequest
};



class TextPanel;
class TogglePanel;
class KeyPressPanel;



class Ui: public IDifficultyCallback,
          public IStyleInfo, 
          public IKeyObserver {
  struct UIxdata {
    // Set controls windows.
    Window controls[Xvars::DISPLAYS_MAX];
    
    // The top level shell containing the KeyPressPanel.
    Window learnControls[Xvars::DISPLAYS_MAX]; 

    // Help message.
    Window help[Xvars::DISPLAYS_MAX];
  };
  
  
 public:
  Ui(int *argc,char **argv,WorldP w,LocatorP l,
     char **displayNames,char *fontName,SoundManager *,
     const DifficultyLevel dLevels[DIFFICULTY_LEVELS_NUM],
     RoleType);  
  /* EFFECTS: Create user interface with one viewport (number 0) with world w 
     and locator l.  The memory pointed to by displayNames becomes the
     property of the Ui.  Needs RoleType in the constructor so can
     add info to the window manager title bar. */
  /* NOTE: Must set_keyset on all displays before first clock. 
     Must manually set_* all desired parameters before first reset. */

  ~Ui();
 
  int get_viewports_num() {return viewports.length();}
  /* EFFECTS: Total number of viewports in the Ui. */

  int get_viewports_num_on_dpy(int dpyNum) {return vIndexNum[dpyNum];}
  int get_viewport_on_dpy(int dpyNum,int v) {return vIndex[dpyNum][v];}
  /* EFFECTS: Get viewports and number of viewports on a single display.
     The viewports on a display, dpyNum, range from 0 to 
     get_viewports_num_on(dpyNum) - 1. */

  Display *get_dpy(int dpyNum) {return xvars.dpy[dpyNum];}
  int get_dpy_max() {return xvars.dpyMax;}
  int get_dpy_num(int viewportNum); 
  /* EFFECTS: Display number from viewport Number. */

  UImask get_settings(UIsettings &s);
  /* MODIFIES: s */
  /* EFFECTS:  Get all of the settings that are stored in the Ui.  Mostly the
     menubar stuff.  settingsChanges gives the fields in s that have changed
     since the last call of get_settings.  Other fields are not guaranteed to
     be meaningful.  Initially, nothing is changed. */

  const char * const *get_keys_names() 
  {return (const char * const *)keysNames;}

  Boolean settings_changed() {return settingsChanges != UInone;}
  /* EFFECTS:  Tells whether the the settings have changed since the last 
     call of get_settings().  Initially is False. */

  Boolean keyset_set(int dpyNum) {return keysetSet[dpyNum];}

  void set_humans_num(int);
  void set_enemies_num(int);
  void set_enemies_refill(Boolean);
  void set_style(GameStyleType);
  void set_quanta(Quanta);
  void set_cooperative(Boolean);

  // Do nothing, only place holders for Windows version.
  void set_track_volume(int) {}
  void set_sound_volume(int) {}
  void set_sound_onoff(Boolean){}
  void set_world_rooms(const Rooms &){}
  void set_role_type(RoleType r)
  {assert(0);}
  /* NOTE: Don't have code to change the window title bar after the window
     has been created. */

  void set_humans_playing(int);
  void set_enemies_playing(int);

  void set_level(const char *);

  static void set_reduce_draw(Boolean val) {Viewport::set_reduce_draw(val);}
  /* EFFECTS: Don't draw backgrounds and outside. */

  static void set_use_buffer(Boolean val) {Viewport::set_use_buffer(val);}
  /* EFFECTS: Set whether or not to use a double-buffer for drawing. */

  Boolean other_input() {return otherInput;}
  /* EFFECTS: Tells whether there has been any keyboard or mouse press other
     than a valid control since the last clock. */

  void set_input(int vNum,UIinput input);
  /* EFFECTS: Set the input device for viewport num. */

  void set_keyset(int dpyNum,UIkeyset keyset);
  void set_keyset(int dpyNum,UIkeyset basis,KeySym right[UI_KEYS_MAX][2],
		  KeySym left[UI_KEYS_MAX][2]);
  /* REQUIRES: Must be called at least once before the first clock. */
  /* IMPLEMENTATION NOTE: Sets keycodes[0 and 1] */

  void set_difficulty(int) {}
  /* NOTE: Dummy, could be used to get default value for 
     set_prompt_difficulty. */

  void set_pause(Boolean);

  void set_prompt_difficulty();
  void unset_prompt_difficulty();
  int get_difficulty() {return difficulty;}
  /* EFFECTS: Similar to set_level_title.  Prompts user for difficulty level.
     get_difficulty() returns user entered difficulty since last call to
     set_prompt_difficulty() or, DIFF_NONE if none set. */

  virtual void change_difficulty(int);
  /* NOTE: For implementing IDifficultyCallback. */
  
  virtual GameStyleType get_game_style_type();
  virtual RoleType get_role_type();
  /* NOTE: For implementing IStyleInfo. */

  virtual void key_event(int dpyNum,int keycode,Boolean down);
  /* NOTE: For implementing IKeyObserver. */

#if 0
  virtual void send_chat_request(const char* receiver,const char* message);
  /* NOTE: For implementing IChatObserver. */
#endif

  int add_viewport();
  /* EFFECTS: Add another viewport and return its number. */

  void del_viewport();
  /* EFFECTS: Delete the highest numbered viewport. */

  void register_intel(int n, IntelP intel);
  /* REQUIRES: intel is not already registered with a different viewport. */
  /* EFFECTS: Registers the intel on viewport n.  The viewport will now 
     follow the object represented by intel->get_id() and send commands to 
     it if it is human. */

  void demo_reset();
  /* EFFECTS: Like reset() but does not effect whether viewport can be
     scrolled. */

  void reset();
  /* EFFECTS: Prepare Ui for a new game. Clear out all registered intels.  
     Does NOT remove viewports. */

  void set_redraw_arena();
  /* EFFECTS: Next turn, the Ui must redraw the arena window. */

  void process_event(int dpyNum,XEvent *event);
  /* EFFECTS: Root event handling routine. */

  void pre_clock();
  /* EFFECTS: Follows the object of the registered intel if any and redraws 
     the world and locator.  Ui::pre_clock() is guaranteed to pull
     messages from the queues in this phase.  Drawing happens in 
     pre_clock(). */
  /* NOTE: pre means before reading the events from the queue for this 
     turn. */

  void post_clock();
  /* EFFECTS: Just respond to keyboard events. */
  /* NOTES: post means after reading the events for this turn. */

  // The callbacks exported to the Viewport.
  static void menu_quit_CB(void*,Viewport*,void*);
  static void menu_new_game_CB(void*,Viewport*,void*);
  static void menu_humans_num_CB(void*,Viewport*,void*);
  static void menu_enemies_num_CB(void*,Viewport*,void*);
  static void menu_enemies_refill_CB(void*,Viewport*,void*);
  static void menu_controls_CB(void*,Viewport*,void*);
  static void menu_learn_controls_CB(void*,Viewport*,void*);
  static void menu_scenarios_CB(void*,Viewport*,void*);
  static void menu_levels_CB(void*,Viewport*,void*);
  static void menu_kill_CB(void*,Viewport*,void*);
  static void menu_duel_CB(void*,Viewport*,void*);
  static void menu_extended_CB(void*,Viewport*,void*);
  static void menu_training_CB(void*,Viewport*,void*);
  static void menu_quanta_CB(void*,Viewport*,void*);
  static void menu_cooperative_CB(void*,Viewport*,void*);
  static void menu_help_CB(void*,Viewport*,void*);
  static void status_weapon_CB(void*,Viewport*,void*);
  static void status_item_CB(void*,Viewport*,void*);
  static void chat_CB(void*,Viewport*,void*);

  static void learn_controls_CB(Panel*,void*,void*);

  static IViewportInfo* get_viewport_info();
  /* EFFECTS: Return structure in static memory 
     with info necessary to figure out the viewport around a given point. */

  static void set_synchronous() {synchronous = True;}
  /* EFFECTS: For debugging, make all X requests synchronous. */

  static void set_large_viewport(Boolean val) {largeViewport = val;}
  /* EFFECTS: Sets the initial value for large/small Viewport.  Can be
     overridden by the License Agreement dialog. */

  static void set_smooth_scroll(Boolean val) {smoothScroll = val;}
  /* EFFECTS: Sets the initial value for smooth scroll.  Can be
     overridden by the user in the License Agreement dialog. */
 

 private:
  void set_message(const char *message);
  /* EFECTS: Places message on all the viewports' message bars.  */

  void draw(int viewportNum, Boolean changedOnly = False);
  /* REQUIRES: viewportNum is valid. */
  /* EFFECTS: Draw everything in viewport number num. */

  void init_x();
  /* EFFECTS: Initialize the X Window variables in the Ui. */
  /* NOTE: Also sets vIndex,vIndexMax,meunusNum,dpyIndex. */

  void intern_atoms();

  Boolean viewport_to_loc(int n,const Loc &l);

  int get_viewport_num(int dpyNum,Window window);
  /* EFFECTS: If window is one of the arenas, returns the viewport num.  
     Otherwise, returns -1. */

  void create_controls();

  void create_learn_controls();

  void create_help();

  void controls_redraw(int dpyNum);

  void controls_expose(int dpyNum,XEvent *) {controls_redraw(dpyNum);}

  Boolean key_equals(int dpyNum,int key,int input,int keycode);
  /* EFFECTS: Match the physical keycode on the given display with the
     keys registered for one of the two inputs on that display. */


  // Variables.
  static char *keysNames[UI_KEYS_MAX];

  char **argv;  // Warning: Exposing the rep.
  int argc;
  Xvars xvars;
  UIxdata xdata;
  char **displayNames;  /* Array of [UI_VIEWPORTS_MAX]. 
                           Warning: Exposing the rep. */

  char *fontName;
  PtrList viewports; // List of (Viewport*).

  // Map from displays to viewports, a one to many map.
  int vIndex[Xvars::DISPLAYS_MAX][UI_VIEWPORTS_MAX]; 
  // Current number of viewports on a display.
  int vIndexNum[Xvars::DISPLAYS_MAX]; 
  // Possible number of viewports on a display.
  int vIndexMax[Xvars::DISPLAYS_MAX]; 

  WorldP world;
  LocatorP locator;

  // The keyboard mappings
  // [display num][right or left][key number][2 keys per mapping]
  unsigned int keycodes[Xvars::DISPLAYS_MAX][2][UI_KEYS_MAX][2]; 
  // Just for making an assert.
  Boolean keysetSet[Xvars::DISPLAYS_MAX];

  UIsettings settings;
  UImask settingsChanges;
  Boolean otherInput;
  
  Timer messageTimer;
  Boolean pause;

  RoleType roleType;

  // This value is mofified by the Viewport via giving it th
  int difficulty;
  const DifficultyLevel* difficultyLevels;  // Size DIFFICULTY_LEVELS_NUM.

  // The actual control learning panel.
  KeyPressPanel* learnControls[Xvars::DISPLAYS_MAX];  

  // The panel displaying the help message inside the toplevel window.
  TextPanel* help[Xvars::DISPLAYS_MAX];

  // Would be better if these were stored in the learn controls dialog
  // itself.
  struct LControls {
    int input,key,which;
  };
  LControls lControls[Xvars::DISPLAYS_MAX];

  static Boolean synchronous;

  // From the command line.
  static Boolean largeViewport;

  static Boolean smoothScroll;

  // The callbacks to export to the Viewport.
  static ViewportCallback viewportCallbacks[VIEWPORT_CB_NUM];

  static char* helpMessage;
};

#endif
