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

#ifndef _UI_PLAYER_H
#define _UI_PLAYER_H

#include "area.h"
#include "ui_cmn.h"
#include "fileman.h"

// Class Prototypes
class AboutDialog;
class CKeySets;
class OptionsDlg;
class CDifficulty;
class CConnectDlg;
class CRunServerDlg;
class CStoryDlg;



enum KeyStyle {UI_NINE_KEY,UI_FOUR_KEY};



class VInfoProvider: public IViewportInfo {
public:
  virtual ViewportInfo get_info();
};



enum RecoveryState {
  // No errors, or after a successful recovery.
  RS_OK,              
  // Attempt error recovery at the soonest possible time.
  RS_MUST_RECOVER,    
  // In the process of attempting to recover, or after recovery but before we
  // know if recovery worked or not.
  RS_IN_RECOVERY,     
  // Error recovery failed, just keep drawing and hope for the best.
  RS_RECOVERY_FAILED,
};



// Class Declarations
class UiPlayer: public Ui, 
                public IKeyState, 
                public IDispatcher, 
                public IErrorRecovery {
public:
  enum {SOUNDTRACKTIMER_CD=666,SOUNDTRACKTIMER_MIDI=667};
  UiPlayer(int *argc,char **argv,WorldP w,LocatorP l,
	  char **,char *,SoundManagerP s,DifficultyLevel dLevels[DIFFICULTY_LEVELS_NUM],
    Xvars::SCREENMODE mode,const char* gameVersion);
  ~UiPlayer();
  /* EFFECTS: Create user interface with one viewport (number 0) with world w 
     and locator l.  The memory pointed to by displayNames becomes the
     property of the Ui. */
  /* NOTE: Must set_keyset on all displays before first clock. 
     Must manually set_* all desired parameters before first reset. */

  int getWindowList(CTypedPtrList<CPtrList,CWnd *>&p_windowlist);
  int get_viewports_num() {return viewportsNum;}
  /* EFFECTS: Total number of viewports in the Ui. */

  int get_dpy_max();

  void set_humans_playing(int);

  void set_enemies_playing(int);

  void set_level(const char *);

  void set_screen_mode(Xvars::SCREENMODE p_mode);

  Boolean other_input();
  /* EFFECTS: Tells whether there has been any keyboard or mouse press other
  than a valid control since the last clock. */

  void set_input(int vNum,UIinput input);
  /* EFFECTS: Set the input device for viewport num. */

  void set_difficulty(int d);

  void set_role_type(RoleType);

  void set_prompt_difficulty();

  int get_difficulty() {return difficulty;} 
  /* EFFECTS: Similar to set_level_title.  Prompts user for difficulty level.
     get_difficulty() returns user entered difficulty since last call to
     set_prompt_difficulty() or, DIFF_NONE if none set. */

  int add_viewport();
  /* EFFECTS: Add another viewport and return its number. */

  void del_viewport();
  /* EFFECTS: Delete the highest numbered viewport. */

  void register_intel(int n, IntelP intel);
  /* REQUIRES: intel is not already registered with a different viewport. */
  /* EFFECTS: Registers the intel on viewport n.  The viewport will now 
  follow the object represented by intel->get_id() and send commands to 
  it if it is human. */

  void unregister_intel(int n);
  /* REQUIRES: n is a valid viewport */
  /* EFFECTS: Unregister the intel associated with viewport n, if any.  
  Otherwise do nothing. */
    
  IntelP get_intel(int n);
  /* REQUIRES: n is a valid viewport */
  /* EFFECTS: none, just returns intel[n] */

  void demo_reset();
  /* EFFECTS: Like reset() but does not effect whether viewport can be
     scrolled. */

  void reset();
  /* EFFECTS: Clear out all registered intels.  Does NOT remove viewports. */

  void reset_graphics(Xvars::SCREENMODE p_newmode);
  /* USED when ui allready displaying. destroys windows and creates new ones.*/

  void set_redraw_arena();
  /* EFFECTS: Next turn, the Ui must redraw everything. */
  Boolean process_event(int,CMN_EVENTDATA eventdata);//return if handled
  /* NOTE: Call callback last so that callback can make changes to self. */

  void pre_clock();
  /* EFFECTS: Follows the object of the registered intel if any and redraws 
  the world and locator. */

  void post_clock();

  virtual Boolean key_down(int key,void*);
  /* NOTE: For implementing IKeyState. */

  virtual void dispatch(ITcommand,void*);
  /* NOTE: For implementing IDispatcher. */

  static void set_generate_xpm(const char* val) {genXPMDir = val;}
  /* REQUIRES: val is in static memory, never deleted */
  /* EFFECTS: Set UiPlayer to auto-generate UNIX XPM images from the windows
     bitmaps when graphics are initialized. */

  static void bmp_match_palette(const char* srcDir,const char* destDir) 
  {bmpSrcDir = srcDir; bmpDestDir = destDir;}
  /* REQUIRES: srcDir and destDir are static memory, never deleted */
  /* EFFECTS: Set UiPlayer to copy the bitmaps in srcDir to destDir matching 
     the palette to the XEvil palette. */

  static IViewportInfo* get_viewport_info() {return &viewInfo;}

  //--------------------- For IErrorRecovery -------------------//
  virtual Boolean attemptRecovery();
  //--------------------- End IErrorRecovery -------------------//


private:
  void set_message(const char *message);
  /* EFECTS: Places message on all the viewports' message bars.  */

  enum DRAWTYPE{
    DRAW_NOTALL,  // objects only
    DRAW_ALL,     // internal redraw all
    DRAW_EXPOSE,  // from expose event
  };

  void full_screen_init_x(HWND p_primarywnd);  //set up for fullscreen mode
  void window_screen_init_x(HWND p_primarywnd);  //set up for windowed mode

  void draw(int viewportNum, DRAWTYPE changedOnly);
  /* REQUIRES: viewportNum is valid. */
  /* EFFECTS: Draw everything in viewport number num. */

  void draw_backbuffer(int viewportNum);
  /* NOTE: Helper for draw(). */

  void interpret_key_state();

  void init_x(HWND);
  /* EFFECTS: Initialize the X Window variables in the Ui. */
  /* NOTE: Also sets vIndex,vIndexMax,meunusNum,dpyIndex. */

  Boolean viewport_to_pos(int n,const Pos &pos);
  /* REQUIRES: n is a valid viewport num.  pos is a Pos in the world. */
  /* EFFECTS: Moves viewport n so that it is centered on pos. Returns whether the
  viewport has moved. */

  void dispatch(int v,ITcommand command);
  /* EFFECTS: Let viewport v deal with the given command.  I.e. Send it to a 
  intel or move the viewport around. */

  int get_viewport_num(HWND window);
  /* EFFECTS: If window is one of the arenas, returns the viewport num.  
  Otherwise, returns -1. */
  CXEvilWndP get_viewport_wnd(int p_viewport);
  //return the HWND associated with viewport p_viewport

  CWnd *findModalessWindow(HWND p_hwnd); /*searches through m_pabout,m_pkeys ect..*/
  void deleteModalessWnd(CWnd *t_wnd); //delete the modalless window m_pabout,m_pkeys  and NULLS them*/

  int get_statsviewport_num(HWND window);
  /* EFFECTS: If window is one of the stats windows, returns the stats num.  
  Otherwise, returns -1. */

  StatusWndP get_stats_wnd(int p_statsport);
  /* EFFECTS: Return the HWND associated with statsport p_statsport. */

  void update_statuses(int viewportNum,BOOL forcestatuschange=FALSE);

  void viewport_expose(int viewportNum); 

  void add_surfaces_all();
  /* EFFECTS: Add all surface requests to the SurfaceManager. */

  void init_x_all();
  /* EFFECTS: Explicitly initialize graphics for World, Locator, all objects,
     and status window. */

  void generate_xpm();
  /* EFFECTS: Generate UNIX XPM images from the windows bitmaps. */

  void display_arena_message_sync(const char* message);
  /* EFFECTS: Synchronously display message as the arena message in all
     viewports. */

  void update_chat_message();
  /* EFFECTS: Reflect the current value of m_chatMessage into the status 
     bar. */

  void chat_process_char(char);
  /* EFFECTS: Deal with a key press when in chat mode.  Possibly committing or
     aborting the chat message. */


  Xvars xvars;

  int viewportsNum;  // Total number of viewports on all displays.

  WorldP world;
  LocatorP locator;
  SoundManagerP soundmanager;

  Area viewportAreas[UI_VIEWPORTS_MAX]; // world coords, unstretched
  Size viewSize; // window coords, stretched
  
  Boolean intelsSet[UI_VIEWPORTS_MAX];
  IntelP intels[UI_VIEWPORTS_MAX];

  // Removed DISPLAYS_MAX dimension, only meaningful on X11.
  // Also removed the 2-keys-per command dimension, we weren't
  // using it on Windows anyway.
  unsigned char keycodes[2][UI_KEYS_MAX]; 
  KeyStyle keyStyle;
  PtrList keyDispatchers; // List of KeyDispatcher

  UIinput inputs[UI_VIEWPORTS_MAX];  // Meaningful iff inputsSet[].
  Boolean inputsSet[UI_VIEWPORTS_MAX];

  Dim roomDim;
  Boolean otherInput;

  Boolean neverReset; /* So can't scroll with title screen. */

  Timer arenaMessageTimer[UI_VIEWPORTS_MAX];
  char *arenaMessage[UI_VIEWPORTS_MAX];  // a message or NULL.

  // Only need one window.
  CTypedPtrArray<CPtrArray,CXEvilWnd *>m_windowarray;
  CTypedPtrArray<CPtrArray,StatusWnd *>m_windowstatsarray;
  CTypedPtrArray<CPtrArray,FKeysDlg *>m_fkeysarray;
  // Only need one clipper.
  CTypedPtrArray<CPtrArray,LPDIRECTDRAWCLIPPER>m_clipperarray;

  Timer messageTimer;
  int difficulty; // may be DIFF_NONE
  DRAWTYPE redrawAll[UI_VIEWPORTS_MAX]; 
  Boolean deactivated;

  void doOptions();
  void doKeys(UINT p_commandid);
  void retrieveKeys();//retrievs keys from m_pkeys
  void retrieveOptions();//retrieves options from m_poptions
  void retrieveDifficulty();//retrievs keys from m_pkeys
  void doDifficulty();
  void doActivate(WORD p_fActive);
  void doConnect();
  void doStory();
  void retrieveConnect();
  void doRunServer();
  void retrieveRunServer();

  AboutDialog *m_pabout;
  CKeySets *m_pkeys;
  CDifficulty *m_pdifficulty;
  OptionsDlg *m_poptions;
  CConnectDlg *m_pconnect;
  CStoryDlg *m_pstory;
  CRunServerDlg *m_prunserver;
  //used to update the window after another ::init_x
  int m_numhumansplaying; 
  int m_numenemiesplaying; 
  CString m_level;
  // Used to recreate menus properly.  E.g. when switching from window to FS mode.
  RoleType m_roletype;  

  const char* m_gameversion;

  // -1 means normal display, else gives the index of the SurfaceManager
  // debugging page we are currently displaying.
  int m_surfMgrPage;

  static LRESULT CALLBACK ViewPortProc(HWND, UINT, WPARAM,LPARAM);

  // The filename of the dest directory for generating XPM images.
  // NULL means don't generate XPM images.
  static const char* genXPMDir;

  // What's the hungarian notation for static members?  Also m_? hardts.
  static const char* bmpSrcDir;
  static const char* bmpDestDir;
  TempFileManager tempfilemanager;

  // For providing information about the viewport.
  static VInfoProvider viewInfo;

  // Where we are in the process of error recovery.
  RecoveryState m_recoveryState;

  // Is the Ui currently in chat mode, the user is typing in a chat message. 
  Boolean m_chatOn;

  // The chat message currently being typed in, may be empty.
  CString m_chatMessage;
};

#endif //_UI_PLAYER_H

