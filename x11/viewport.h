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

// "viewport.h"  The window for one player.

#ifndef VIEWPORT_H
#define VIEWPORT_H

#ifndef NO_PRAGMAS
#pragma interface
#endif


// Include Files
#include "utils.h"
#include "coord.h"
#include "xdata.h"
#include "intel.h"
#include "ui_cmn.h"
#include "panel.h"
#include "world.h"
#include "locator.h"



// Kind of a hack, using this enumeration for two separate things.
// Internally in viewport for the menus, externally for the
// list of all the callbacks.
// The external use has three callbacks that do not correspond to menu items,
// stWeapon, stItem, and stChat.
// And, menuStyle is only used internally.  It has no callback since it is 
// just a text label.
//
// We really should split this into two separate enumerations.
enum {
  menuControls,menuLearnControls, // These two MUST be first.
  menuQuit,menuNewGame,menuHumansNum,menuEnemiesNum,menuEnemiesRefill,
  menuStyle,menuScenarios,menuLevels,menuKill,menuDuel,menuExtended,
  menuTraining,menuQuanta,menuCooperative,menuHelp,
  stWeapon,stItem,
  stChat, // Bad naming convention.
  VIEWPORT_CB_NUM, // Must be last.
};
#define VW_MENUS_PRIMARY_NUM 17
#define VW_MENUS_SECONDARY_NUM 2
// Not 9, because ammo is with weapon.
#define VW_STATUSES_NUM 8  




// KeyState handles XEvil virtual keys, i.e. 0 < key <= UI_KEYS_MAX
class KeyState: public IKeyState {
public:
  KeyState();

  virtual Boolean key_down(int key,void*) {return isDown[key];}
  /* EFFECTS: Return whether the given key is currently down. */

  void set(int key,Boolean down) 
  {assert(key < UI_KEYS_MAX); isDown[key] = down;}
  /* EFFECTS: Set the up/down state of a key. */


private:
  Boolean isDown[UI_KEYS_MAX];
};


// Should collapse all these Observer/Callback/Info interfaces into 
// one IViewportManager.  Wouldn't have to pass as many arguments
// around.



// Way of reporting that the user has specified the difficulty level.
class IDifficultyCallback {
public:
  virtual void change_difficulty(int) = 0;
};



// So the Viewport can get the GameStyleType to decide on some of the 
// text for the statuses.
class IStyleInfo {
public:
  virtual GameStyleType get_game_style_type() = 0;

  virtual RoleType get_role_type() = 0;
};



// This is all needed to do the magic where a key pressed on one viewport
// on a display can send a command to a different viewport.
class IKeyObserver {
public:
  virtual void key_event(int dpyNum,int keycode,Boolean down) = 0;
  /* EFFECTS: Report that a key was pressed or released on the specified
     display. */
};



#if 0
// So the Viewport can request that a chat message be sent.
class IChatObserver {
public:
  virtual void send_chat_request(const char* receiver,const char* message) = 0;
  /* REQUIRES: receiver is non-NULL */
  /* EFFECTS: Send message to the named receiver.  Receiver will be the 
     empty string if message should be sent to everyone. */
};
#endif



class Viewport;
typedef void (*ViewportCallback)(void* value,Viewport*,void* closure);



class VInfoProvider;
class IScroller;



class Viewport: public IDispatcher {
public:
  void init();
  /* EFFECTS: Must be called immediately after constructing the Viewport. */
  /* NOTE: Ick.  Hack so that we can call virtual functions from the
     initialization code for Viewport. */

  virtual ~Viewport();
  
  int get_dpy_num() {return dpyNum;}

  IntelP get_intel() {return intel;}
  
  void set_menu_controls(Boolean val);
  
  Boolean get_menu_learn_controls();
  void set_menu_learn_controls(Boolean val);

  void set_menu_humans_num(int);

  void set_menu_enemies_num(int);

  void set_menu_help(Boolean val);
  
  void set_enemies_refill(Boolean);

  void set_style_and_role_type(GameStyleType style,RoleType);
  /* NOTE: Only together for efficiency in updating the sensitivity
     of the menu items. */

  void set_quanta(Quanta);

  void set_cooperative(Boolean);

  void set_humans_playing(int);
  
  void set_enemies_playing(int);

  const char* get_level();

  void set_level(const char*);

  UIinput get_input() {return input;}

  void set_input(UIinput);
  /* EFFECTS: Specify whether to use the controls on the right or left 
     side of the keyboard. */

  void set_message(const char*);
  /* EFFECTS: Set the message in the message bar at the bottom of the 
     viewport. */

  void set_arena_message(const char* message,Quanta);
  /* EFFECTS: Display message in the arena area for the given number of 
     turns. */
  /* NOTE: Makes copy of (const char*) argument. */
  
  void set_redraw_arena() {redrawArena = True;}
  /* EFFECTS: Mark the arena portion of the Viewport as dirty. */

  static void accept_input() {acceptInput = True;}
  /* EFFECTS: No Viewports will accept user intput until this is called. */
  /* NOTE: This is so that the viewport can't scroll around while the demo
     is playing. */

  void register_intel(int humanColorNum,IntelP intel);
  /* REQUIRES: intel is non-NULL.  intel is not registered with any other
     Viewport. */
  /* EFFECTS: The viewport will now follow the object represented by 
     intel->get_id() and send commands to it if it is human.  humanColorNum
     gives the index into Xvars::humanColors. */

  void clear_all();
  /* EFFECTS: Clear all windows/subwindows in the Ui. */
  /* NOTE: Used for implementing pause feature. */

  void unclear_all();
  /* EFFECTS: Force everything to get drawn. */
  /* NOTE: Used for recovering when pause is turned off. */

  void set_prompt_difficulty(Boolean val);
  /* EFFECTS: Disable regular drawing of the Viewport and instead prompt the 
     user for the difficulty level. */
     
  void reset();
  /* EFFECTS: Initialize for a new game. */

  Boolean has_window(Window w);
  /* EFFECTS: Is the given window under control of this Viewport.  Used for
     event handling. */

  void process_event(XEvent* e);
  /* REQUIRES: e is an event on a window for which has_window() is True. */
  /* EFFECTS: Process an event that happened on a window in this viewport,
     including subwindows. */

  void pre_clock();
  void post_clock();
  /* NOTE: pre and post refer to before and after pulling all the 
     events out of the queue. */

  static IViewportInfo* get_info();
  /* EFFECTS: Returns an IViewportInfo object that provides information
     about viewports.  This can be called before any Viewport is created
     and before init_viewport_info() is called.
     However, the returned IViewportInfo object may not actually be used 
     until init_viewport_info() is called.  The returned object should
     never be freed. */
  /* NOTE: This complexity is all to deal with the fact that the Role 
     get created before the Ui. */

  static void init_viewport_info(Boolean isStretched,Boolean smoothScroll);
  /* EFFECTS: After this call, the object returned from Viewport::get_info()
     is ready for use. */

  virtual void dispatch(ITcommand,void*);
  /* EFFECTS: Take appropriate action in response to an ITcommand. 
     The void* argument is not used, only there to match the interface
     needed for KeyDispatcher.  (It is used on Windows.) */
  /* NOTE: For implementing IDispatcher. */

  void receive_key(int key,Boolean);
  /* EFFECTS: Ui tells us when a key for this viewport is pressed.  Ui
     does all mapping from keycodes to the key enumeration. */
  /* NOTE: The key event may have come from another Viewport on the same 
     display. */

  static Boolean get_reduce_draw() {return reduceDraw;}

  static void set_reduce_draw(Boolean val) {reduceDraw = val;}

  static Boolean get_use_buffer() {return useBuffer;}

  static void set_use_buffer(Boolean val) {useBuffer = val;}


#ifndef PROTECTED_IS_PUBLIC
protected:
#endif
  Viewport(int argc,char** argv,
           Xvars& xvars,int dpyNum,
           WorldP,LocatorP,
           Boolean smoothScroll,int menusNum,
           IDifficultyCallback* diffCB,
           IStyleInfo* styleInfo,
           IKeyObserver* keyObserver,
           //           IChatObserver* chatObserver,
           const DifficultyLevel dLevels[DIFFICULTY_LEVELS_NUM],
           RoleType roleType,
           ViewportCallback callbacks[VIEWPORT_CB_NUM],
           void* closure);
  /* NOTE: Does not take ownership of the callbacks, and closures, arrays. */  

  void create_toplevel();
  /* EFFECTS: Create a top-level window, set the input mask, set standard WM
     properties, and map it.  Creates menus, arena, statuses, intelsPlaying,
     and messageBar.  If viewportNum == 0 then it creates the menus. */

  void create_menus();
  void create_arena();
  virtual void create_statuses() = 0;
  void create_intels_playing();
  void create_message_bar();
  void create_level();

  Dir button_press_to_dir(XEvent *event);

  // BEGIN EVENTS
  void expose(XEvent* event); 
  void key_press(XEvent* event);
  void key_release(XEvent* event);
  void button_press(XEvent* event);
  // END EVENTS

  virtual Dim get_inner_dim() = 0;
  /* EFFECTS: The dimensions of the rectangle to track when following 
     an intel.  The area displayed is actually larger by 
     get_extra_dim() wsquares. */
  
  virtual Dim get_extra_dim() = 0;
  /* EFFECTS: The extra space around the edges of the viewport.  As soon as
     the player gets to this outer margin, the world scrolls to the next 
     screen. */

  virtual void init_sizes(Xvars&,int dpyNum,WorldP world);
  /* EFFECTS: Initialize all the sizes for a given Display. */
  /* NOTE: Viewport::init_sizes() does the sizes common to 
     the children of Viewport, so children should call Viewport::init_sizes()
     explicitly. */

  static void panel_callback(Panel*,void*,void*);
  /* EFFECTS: Just forwards to appropriate callback in panelClosures. */

  void update_statuses();
  /* NOTE: Part of clock(). */

  void follow_intel();
  /* NOTE: Part of clock(). */

  void draw();
  /* NOTE: Part of clock(). */
  /* NOTE: Will draw everything if redrawArena is true, else just what has
     changed. */

  void draw_string(const Pos &pos,char *msg);
  /* EFFECTS: Draw string msg using current GC on arena with upper left of
     character at position pos. */

  static void draw_string_center(char *msg);
  /* EFFECTS: Draw string msg centered on the arena. */
     
  Boolean shift_viewport(int cols,int rows);
  /* EFFECTS: Move the viewport by the given number of rows or cols.  But, 
     always keep the world visible in the viewport.  rows and cols may 
     be negative.  Return whether the viewport moved. */


  // Because some X functions need it.
  int argc;
  char** argv;

  Xvars& xvars;
  int dpyNum;
  WorldP world;
  LocatorP locator;

  // Toplevel XEvil window.
  Window toplevel;
  // Window behind arena, used for drawing tick marks.
  Window arenaBG;
  // Playing field.
  Window arena;

  Boolean cursorDefined;

  // The rectangular box of wsquares that the viewport is showing,
  // unstreched coordinates.
  Area viewportArea;

  IntelP intel;
  KeyState* keyState;
  KeyDispatcher* keyDispatcher;
  UIinput input;
  Boolean promptDifficulty; 

  // Takes advantage of fact that menuControls and menuLearnControls are
  // the first two elements of the enumeration.
  TextPanel *menus[VW_MENUS_PRIMARY_NUM];
  int menusNum; 
  TextPanel *statuses[VW_STATUSES_NUM];
  TextPanel *humansPlaying;
  TextPanel *enemiesPlaying;
  ChatPanel *messageBar;
  // Need better name.  This refers to the status item that displays 
  // level-specific information.
  TextPanel *level;
  Timer arenaMessageTimer;
  // A message or NULL.
  char *arenaMessage;  
  Boolean redrawArena;
  const DifficultyLevel* difficultyLevels;
  RoleType roleType;

  // Forwarding info for calling the registered callbacks.
  // A list of (PanelClosure*).
  PtrList panelClosures;
  // Passed into the Viewport constructor.
  void* closure;

  IDifficultyCallback* diffCallback;
  IStyleInfo* styleInfo;
  IKeyObserver* keyObserver;
  //  IChatObserver* chatObserver;

  IScroller* scroller;

  static Boolean acceptInput;

  // For supplying ViewportInfo lazily.
  // (Note: Only used by Role right now.)
  static VInfoProvider* vInfoProvider;

  static Boolean reduceDraw;
  static Boolean useBuffer;


  /////////// All following sizes are in window coordinates. ////////////

  // Have the sizes been initialized for the display.
  static Boolean sizeValid[Xvars::DISPLAYS_MAX];

  // Size of the entire menu rectangle.
  static Size menusSize[Xvars::DISPLAYS_MAX];

  // Size of the inner area, the area being tracked on the world.
  static Size innerSize[Xvars::DISPLAYS_MAX];

  // arenaSize includes border
  static Size arenaSize[Xvars::DISPLAYS_MAX];

  // Size of the rectangle of statuses for the active intel
  static Size statusesSize[Xvars::DISPLAYS_MAX];

  // Info about humans/machines still remaining.
  static Pos intelsPlayingPos[Xvars::DISPLAYS_MAX];
  static Size intelsPlayingSize[Xvars::DISPLAYS_MAX];

  // The 2-line message bar at the bottom of the window.
  static Pos messageBarPos[Xvars::DISPLAYS_MAX];
  static Size messageBarSize[Xvars::DISPLAYS_MAX];

  // Displays level-specific information.
  static Pos levelPos[Xvars::DISPLAYS_MAX];
  static Size levelSize[Xvars::DISPLAYS_MAX];

  // Size of the entire viewport, does not include any titling the 
  // Window Manager puts on.
  static Size viewportSize[Xvars::DISPLAYS_MAX];
};



// Low resolution viewport, unstretched coordinates, small pixmaps.
class SmallViewport: public Viewport {
public:
  SmallViewport(int argc,char** argv,
                Xvars& xvars,int dpyNum,
                WorldP,LocatorP,
                Boolean smoothScroll,int menusNum,
                IDifficultyCallback* diffCB,
                IStyleInfo* styleInfo,
                IKeyObserver* keyObserver,
                //                IChatObserver* chatObserver,
                const DifficultyLevel dLevels[DIFFICULTY_LEVELS_NUM],
                RoleType roleType,
                ViewportCallback callbacks[VIEWPORT_CB_NUM],
                void* closure);

  virtual Dim get_inner_dim();

  virtual Dim get_extra_dim();

  virtual void init_sizes(Xvars&,int dpyNum,WorldP);
  /* EFFECTS: Initialize sizes appropriate for the small viewport. */

  virtual void create_statuses();

  static ViewportInfo get_info(Boolean smoothScroll);
};



// High resolution viewport, stretched coordinates, large pixmaps.
class LargeViewport: public Viewport {
public:
  LargeViewport(int argc,char** argv,
                Xvars& xvars,int dpyNum,
                WorldP,LocatorP,
                Boolean smoothScroll,int menusNum,
                IDifficultyCallback* diffCB,
                IStyleInfo* styleInfo,
                IKeyObserver* keyObserver,
                //                IChatObserver* chatObserver,
                const DifficultyLevel dLevels[DIFFICULTY_LEVELS_NUM],
                RoleType roleType,
                ViewportCallback callbacks[VIEWPORT_CB_NUM],
                void* closure);  

  virtual Dim get_inner_dim();

  virtual Dim get_extra_dim();

  virtual void init_sizes(Xvars&,int dpyNum,WorldP);
  /* EFFECTS: Initialize sizes appropriate for the large viewport. */

  virtual void create_statuses();

  static ViewportInfo get_info(Boolean smoothScroll);
};
#endif
