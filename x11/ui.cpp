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

// "ui.cpp"

#ifndef NO_PRAGMAS
#pragma implementation "ui.h"
#endif


// Include Files
#include "utils.h"
extern "C" {
#include <string.h>
#include <X11/Xutil.h>
#ifdef OPENWOUND_XOS_STRLEN_HACK
#define _strings_h
#endif
#include <X11/Xos.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>
}

#include <cstdlib>
#include <iostream>
#include <strstream>

#include "coord.h"
#include "area.h"
#include "xdata.h"
#include "ui.h"
#include "world.h"
#include "locator.h"
#include "id.h"
#include "physical.h"
#include "game_style.h"
#include "bitmaps/ui/ui.bitmaps"

#include "sound.h"
#include "l_agreement.h"
#include "panel.h"
#include "viewport.h"

using namespace std;


// Defines
#define DEFAULT_BIG_FONT_NAME "-*-helvetica-*-r-*-*-18-*-*-*-*-*-*-*"
//#define DEFAULT_FONT_NAME "-*-courier-*-r-*-*-17-*-*-*-*-*-*-*"
#define DEFAULT_FONT_NAME "8x13bold"
#define BACKUP_FONT_NAME "9x15"

#define MESSAGE_TIME 9
#define RED_COLOR "red2"
#define GREEN_COLOR "green1" 
#define ARENA_TEXT_COLOR RED_COLOR

#define CONTROLS_COLS 90
#define CONTROLS_ROWS (UI_KEYS_MAX + 5)
#define LEARN_CONTROLS_COLS 72
#define LEARN_CONTROLS_ROWS 10
#define HELP_COLS 70



const static UIkeymap keys_right_sun3 = {
  {XK_R11},{XK_Right},{XK_R15},{XK_Down},{XK_R13},
  {XK_Left},{XK_R7},{XK_Up},{XK_R9},
  {XK_R1},{XK_R2},{XK_R3},
  {XK_R4},{XK_R5},{XK_R6},
  {XK_space},
};



const static UIkeymap keys_right_decmips = {
  {XK_KP_5},{XK_KP_6},{XK_KP_3},{XK_KP_2},{XK_KP_1},
  {XK_KP_4},{XK_KP_7},{XK_KP_8},{XK_KP_9},
  {XK_Find},{XK_Insert},{268500736},
  {XK_Select},{XK_Prior},{XK_Next},
  {XK_space},
}; 



const static UIkeymap keys_right_iris = {
  {XK_KP_5},{XK_KP_6},{XK_KP_3},{XK_KP_2},{XK_KP_1},
  {XK_KP_4},{XK_KP_7},{XK_KP_8},{XK_KP_9},
  {XK_Insert},{XK_Home},{XK_Prior},
  {XK_Delete},{XK_End},{XK_Next},
  {XK_space},
}; 



const static UIkeymap keys_right_ncd = {
  {XK_KP_5},{XK_KP_6},{XK_KP_3},{XK_KP_2},{XK_KP_1},
  {XK_KP_4},{XK_KP_7},{XK_KP_8},{XK_KP_9},
  {XK_Left},{XK_Down},{XK_Right},
  {XK_KP_F1},{XK_KP_F2},{XK_KP_F3},
  {XK_space},
}; 



const static UIkeymap keys_right_sun4 = {
  {XK_KP_5},{XK_KP_6},{XK_KP_3},{XK_KP_2},{XK_KP_1},
  {XK_KP_4},{XK_KP_7},{XK_KP_8},{XK_KP_9},
  {XK_Insert},{XK_Home},{XK_Prior},
  {XK_Delete,XK_BackSpace},{XK_End},{XK_Next},
  {XK_space},
}; 



const static UIkeymap keys_right_sun4_sparc = {
  {XK_F31},{XK_Right},{XK_F35},{XK_Down},{XK_F33},
  {XK_Left},{XK_F27},{XK_Up},{XK_F29},
  {XK_F21},{XK_F22},{XK_F23},
  {XK_F24},{XK_F25},{XK_F26},
  {XK_space},
}; 



const static UIkeymap keys_right_mac = {
  {XK_KP_5},{XK_KP_6},{XK_KP_3},{XK_KP_2},{XK_KP_1},
  {XK_KP_4},{XK_KP_7},{XK_KP_8},{XK_KP_9},
  {XK_Help},{XK_Home},{XK_Prior},
  {XK_Select},{XK_End},{XK_Next},
  {XK_space},
}; 



const static UIkeymap keys_left_all = {
  {XK_l,XK_l},{XK_semicolon,XK_semicolon},{XK_slash,XK_slash},
  {XK_period,XK_period},
  {XK_m,XK_comma},
  {XK_k,XK_k},{XK_i,XK_i},{XK_o,XK_o},{XK_p,XK_bracketleft},
  {XK_a},{XK_s},{XK_d},
  {XK_z},{XK_x},{XK_c},
  {XK_F3},  // Meaningless
};



const static UIkeymap *keymaps_preset[][2] = {
  {&keys_right_sun3, &keys_left_all},         // UIsun3
  {&keys_right_decmips, &keys_left_all},      // UIdecmips
  {&keys_right_iris, &keys_left_all},         // UIiris
  {&keys_right_ncd, &keys_left_all},          // UIncd
  {&keys_right_iris, &keys_left_all},         // UItektronix
  {&keys_right_sun4, &keys_left_all},         // UIsun4
  {&keys_right_sun4, &keys_left_all},         // UIrsaix
  {&keys_right_sun4_sparc, &keys_left_all},   // UIsun4_sparc
  {&keys_right_mac,&keys_left_all},           // UImac
  {&keys_right_decmips,&keys_left_all},       // UIalpha
  {&keys_right_iris,&keys_left_all},          // UIlinux
};  



static int UI_error_handler(Display *dpy,XErrorEvent *evt) {
  char msg[80];

  XGetErrorText(dpy,evt->error_code,msg,80);
  cerr << "X Error: " << msg << endl;

  // Whatever.
  return 0;
}

char* keysym_to_string(KeySym keysym) {
  char* result = XKeysymToString(keysym);
  if (!result) {
    return "";
  }
  return result;
}

Ui::Ui(int *agc, char **agv, WorldP w, LocatorP l,char **d_names,
       char *font_name,SoundManager *,
       const DifficultyLevel dLevels[DIFFICULTY_LEVELS_NUM],
       RoleType rType) {
  argc = *agc;
  argv = agv;
  displayNames = d_names;
  fontName = Utils::strdup(font_name);
  difficultyLevels = dLevels;

  world = w;
  locator = l;

  settingsChanges = UInone;
  otherInput = False;
  pause = False;
  roleType = rType;

  // Bug fix, used to be xvars.dpyMax which isn't initialized yet.
  for (int n = 0; n < Xvars::DISPLAYS_MAX; n++) {
    keysetSet[n] = False;
  }

  for (int dpyNum = 0; dpyNum < Xvars::DISPLAYS_MAX; dpyNum++) {
    vIndexNum[dpyNum] = vIndexMax[dpyNum] = 0;
  }
  
  difficulty = DIFF_NONE; // Not yet set.

  init_x();
  
  // Blatant hack to make sure color is allocated for NShield.
  // More general solution would be, perhaps to force certain classes
  // to ::init_x() first.
  for (int d = 0; d < xvars.dpyMax; d++) {
    xvars.alloc_named_color(d,"steelblue");
  }
  
  // Run the license agreement.
  // Note: The license agreement will not be run if the user runs
  // "xevil -server -no_ui".
  // Must be after init_x().
  Boolean reduceDraw = Viewport::get_reduce_draw();
  Boolean accepted = 
    LAgreement::check_accepted(largeViewport,smoothScroll,reduceDraw,
                               xvars,0,argc,argv);
  Viewport::set_reduce_draw(reduceDraw);
  if (!accepted) {
    // Should probably let game.cpp call exit.
    exit(1);
  }

  // Ick, this should be in init_x(), but we need to call init_x() before
  // running the License Agreement to get this information.
  xvars.stretch = (largeViewport ? 2 : 1);

  // Must be called before the object returned from Ui::get_viewport_info()
  // is used.
  Viewport::init_viewport_info(largeViewport,smoothScroll);

  create_controls();
  create_learn_controls();
  create_help();

  // Add viewport number zero.
  add_viewport();
}



Ui::~Ui() {
  for (int n = 0; n < viewports.length(); n++) {
    delete (Viewport*)viewports.get(n);
  }
}



int Ui::add_viewport() {
  assert(viewports.length() < UI_VIEWPORTS_MAX);
  int viewportNum = viewports.length();

  // Update maps between displays and viewports.
  //
  // Figure out the display number of this Viewport.
  // Inefficient, but doesn't really matter here.
  int dpyNum = -1;
  int menusNum;
  for (int dNum = 0; dpyNum == -1 && dNum < xvars.dpyMax; dNum++) {
    for (int vNum = 0; vNum < vIndexMax[dNum]; vNum++) {
      if (vIndex[dNum][vNum] == viewportNum) {
        // Found it.

        // Only the first viewport on any display has any menus.
        if (vNum == 0) {
          // First viewport on first display has all the menus, i.e. primary.
          if (dNum == 0) {
            menusNum = VW_MENUS_PRIMARY_NUM;
          }
          // First viewport on other displays only have a few menus, 
          // i.e. secondary.
          else {
            menusNum = VW_MENUS_SECONDARY_NUM;
          }
        }
        else {
          menusNum = 0;
        }

        dpyNum = dNum;
        break;
      }
    }
  }
  // If this fails, vIndex and the other maps are messed up.
  assert(dpyNum >= 0);

  Viewport* vPort;
  if (xvars.is_stretched()) {
    vPort = new LargeViewport(argc,argv,xvars,dpyNum,world,locator,
                              smoothScroll,menusNum,
                              this,this,this,
                              difficultyLevels,roleType,
                              viewportCallbacks,
                              (void*)this);
  }
  else {
    vPort = new SmallViewport(argc,argv,xvars,dpyNum,world,locator,
                              smoothScroll,menusNum,
                              this,this,this,
                              difficultyLevels,roleType,
                              viewportCallbacks,
                              (void*)this);
  }
  assert(vPort);
  // Must call init() immediately.
  vPort->init();

  // We explicitly copy the level string from the zeroth viewport, since
  // the level strings are the same for all viewportrs.
  //
  // We should have a better way to do this.  In Game::reset(), 
  // humans_reset() comes after new_level(), so the new viewport is added after
  // set_level() has been called for all the viewports.
  if (viewports.length() != 0) {
    Viewport* vPort0 = (Viewport*)viewports.get(0);
    const char* levelMsg = vPort0->get_level();
    vPort->set_level(levelMsg);
  }

  viewports.add((void*)vPort);


  // Increase the count of the number of viewports actually 
  // created on the given display.
  assert(vIndex[dpyNum][vIndexNum[dpyNum]] == viewportNum);
  assert(vIndexNum[dpyNum] < vIndexMax[dpyNum]);
  vIndexNum[dpyNum]++;

  return viewportNum;
}



void Ui::set_pause(Boolean val) {
  if (pause == val) {
    return;
  }
  
  for (int dpyNum = 0; dpyNum < xvars.dpyMax; dpyNum++) {
    // Turn pause on.
    if (val) {
      XClearWindow(xvars.dpy[dpyNum],xdata.controls[dpyNum]);
      
      learnControls[dpyNum]->clear();
      
      for (int v = 0; v < vIndexNum[dpyNum]; v++) {
        Viewport* vPort = (Viewport*)viewports.get(vIndex[dpyNum][v]);
        vPort->clear_all();
      }
    }
    // Turn pause off.
    else {
      controls_redraw(dpyNum);
      
      learnControls[dpyNum]->redraw();
      
      for (int v = 0; v < vIndexNum[dpyNum]; v++) {
        Viewport* vPort = (Viewport*)viewports.get(vIndex[dpyNum][v]);
        vPort->unclear_all();
      }
    }
  } // for dpyNum
  pause = val;
}



void Ui::set_prompt_difficulty() {
  difficulty = DIFF_NONE; // unspecified until user enters it.
  for (int n = 0; n < viewports.length(); n++) {
    Viewport* vPort = (Viewport*)viewports.get(n);
    vPort->set_prompt_difficulty(True);
  }
}



void Ui::unset_prompt_difficulty() {
  for (int n = 0; n < viewports.length(); n++) {
    Viewport* vPort = (Viewport*)viewports.get(n);
    vPort->set_prompt_difficulty(False);
  }
}



void Ui::change_difficulty(int val) {
  difficulty = val;
}



GameStyleType Ui::get_game_style_type() {
  return settings.style;
}



RoleType Ui::get_role_type() {
  return roleType;
}



// Check user-defined keys.
Boolean Ui::key_equals(int dpyNum,int key,
                       int input,int keycode) {
  Boolean ret = 
    ((keycode == (keycodes[dpyNum][input])[key][0]) ||
     (keycode == (keycodes[dpyNum][input])[key][1]));
  return ret;
}

    

void Ui::key_event(int dpyNum,int keycode,Boolean down) {
  Boolean found = False;
    
  // Loop over all the viewports on the display.
  // Target the keypress to whichever one(s) match.
  //
  // Again, the whole point of this is so that a keypress on one viewport
  // can be interpreted as a command on a different viewport on the same
  // display.
  for (int vNum = 0; vNum < vIndexNum[dpyNum]; vNum++) {
    int viewportNum = vIndex[dpyNum][vNum];
    Viewport* vPort = (Viewport*)viewports.get(viewportNum);
    UIinput input = vPort->get_input();

    // Only look at viewports that have an input set, i.e. use the 
    // keys on the right or left side of the keyboard.
    if (input != UI_INPUT_NONE) {
      for (int key = 0; key < UI_KEYS_MAX; key++) {
        if (key_equals(dpyNum,key,input,keycode)) {
          found = True;
          vPort->receive_key(key,down);
        }
      }
    }
  }

  if (down) {
    // Restart game if space pressed.
    // This doesn't always work, the otherInput flag might get cleared 
    // before it is used.
    if (keycode == XKeysymToKeycode(xvars.dpy[dpyNum],XK_space)) {
      otherInput = True;
    }

    if (!found) {
      // Undocumented pause key feature.
      // Don't do pause if XK_F1 is used for something else.
      if (keycode == XKeysymToKeycode(xvars.dpy[dpyNum],XK_F1)) {
        settingsChanges |= UIpause;
        settings.pause = True;
      }
    }
  }
}



void Ui::del_viewport() {
  assert(viewports.length() > 1);
  Viewport* vPort = (Viewport*)viewports.get(viewports.length() - 1);

  int dpyNum = vPort->get_dpy_num();

  // Delete from list of all viewports.
  viewports.del(viewports.length() - 1);

  // Keep map from display to viewports in synch.
  vIndexNum[dpyNum]--;
  assert(vIndex[dpyNum][vIndexNum[dpyNum]] == viewports.length());

  delete vPort;
}



void Ui::set_humans_num(int val) {
  // Only set for viewport 0.
  Viewport* vPort = (Viewport*)viewports.get(0);

  settings.humansNum = val;
  vPort->set_menu_humans_num(val);
}



void Ui::set_enemies_num(int val) {
  // Only set for viewport 0.
  Viewport* vPort = (Viewport*)viewports.get(0);

  settings.enemiesNum = val;
  vPort->set_menu_enemies_num(val);
}



void Ui::set_enemies_refill(Boolean val) {
  // Only set for viewport 0.
  Viewport* vPort = (Viewport*)viewports.get(0);

  settings.enemiesRefill = val;
  vPort->set_enemies_refill(val);
}



void Ui::set_style(GameStyleType style) {
  // Only set for viewport 0.
  Viewport* vPort = (Viewport*)viewports.get(0);

  // Just used for updating the menu.
  settings.style = style;
  vPort->set_style_and_role_type(style,roleType);
}



void Ui::set_quanta(Quanta quanta) {
  // Only set for viewport 0.
  Viewport* vPort = (Viewport*)viewports.get(0);

  settings.quanta = quanta;
  vPort->set_quanta(quanta);
}



void Ui::set_cooperative(Boolean val) {
  // Only set for viewport 0.
  Viewport* vPort = (Viewport*)viewports.get(0);

  settings.cooperative = val;
  vPort->set_cooperative(val);
}



void Ui::set_humans_playing(int val) {
  for (int n = 0; n < viewports.length(); n++) {
    Viewport* vPort = (Viewport*)viewports.get(n);
    vPort->set_humans_playing(val);
  }
}
  


void Ui::set_enemies_playing(int val) {
  for (int n = 0; n < viewports.length(); n++) {
    Viewport* vPort = (Viewport*)viewports.get(n);
    vPort->set_enemies_playing(val);
  }
}



void Ui::set_level(const char *val) {
  for (int n = 0; n < viewports.length(); n++) {
    Viewport* vPort = (Viewport*)viewports.get(n);
    vPort->set_level(val);
  }
}



void Ui::set_input(int n,UIinput input) {
  assert(n >= 0 && n < viewports.length());
  Viewport* vPort = (Viewport*)viewports.get(n);
  vPort->set_input(input);
}



void Ui::set_keyset(int dpyNum,UIkeyset ks) {
  for (int input = 0; input < 2; input++) {
    for (int n = 0; n < UI_KEYS_MAX; n++) {
      for (int which = 0; which < 2; which++) {
        keycodes[dpyNum][input][n][which] = 
          XKeysymToKeycode(xvars.dpy[dpyNum],
                           (*keymaps_preset[ks][input])[n][which]);
      }
    }
  }
  
  keysetSet[dpyNum] = True;
}



void Ui::set_keyset(int dpyNum,UIkeyset ks,
                    KeySym right[][2],KeySym left[][2]) {
  int n;
  for (n = 0; n < UI_KEYS_MAX; n++) {
    if (right[n][0]) {
      keycodes[dpyNum][UI_KEYS_RIGHT][n][0] = 
        XKeysymToKeycode(xvars.dpy[dpyNum],right[n][0]);
      if (right[n][1]) {
        keycodes[dpyNum][UI_KEYS_RIGHT][n][1] = 
          XKeysymToKeycode(xvars.dpy[dpyNum],right[n][1]);
      }
      else {
        keycodes[dpyNum][UI_KEYS_RIGHT][n][1] = 
          XKeysymToKeycode(xvars.dpy[dpyNum],right[n][0]);
      }
    }
    else {
      keycodes[dpyNum][UI_KEYS_RIGHT][n][0] = 
        XKeysymToKeycode(xvars.dpy[dpyNum],
                         (*keymaps_preset[ks][UI_KEYS_RIGHT])[n][0]);
      keycodes[dpyNum][UI_KEYS_RIGHT][n][1] = 
        XKeysymToKeycode(xvars.dpy[dpyNum],
                         (*keymaps_preset[ks][UI_KEYS_RIGHT])[n][1]);
    }
  }
  
  for (n = 0; n < UI_KEYS_MAX; n++) {
    if (left[n][0]) {
      keycodes[dpyNum][UI_KEYS_LEFT][n][0] = 
        XKeysymToKeycode(xvars.dpy[dpyNum],left[n][0]);
      if (left[n][1]) {
        keycodes[dpyNum][UI_KEYS_LEFT][n][1] = 
          XKeysymToKeycode(xvars.dpy[dpyNum],left[n][1]);
      }
      else {
        keycodes[dpyNum][UI_KEYS_LEFT][n][1] = 
          XKeysymToKeycode(xvars.dpy[dpyNum],left[n][0]);
      }
    }
    else {
      keycodes[dpyNum][UI_KEYS_LEFT][n][0] = 
        XKeysymToKeycode(xvars.dpy[dpyNum],
                         (*keymaps_preset[ks][UI_KEYS_LEFT])[n][0]);
      keycodes[dpyNum][UI_KEYS_LEFT][n][1] = 
        XKeysymToKeycode(xvars.dpy[dpyNum],
                         (*keymaps_preset[ks][UI_KEYS_LEFT])[n][1]);
    }
  }
  keysetSet[dpyNum] = True;
}



int Ui::get_dpy_num(int viewportNum) {
  Viewport* vPort = (Viewport*)viewports.get(viewportNum);
  return vPort->get_dpy_num();
}



UImask Ui::get_settings(UIsettings &s) {
  s = settings; 
  UImask tmp = settingsChanges;
  settingsChanges = UInone;
  return tmp;
}



void Ui::register_intel(int n, IntelP intel) {
  assert(n >= 0 && n < viewports.length());
  Viewport* vPort = (Viewport*)viewports.get(n);

  // Use viewport num as humanColorNum in xvars.
  vPort->register_intel(n,intel);
}



void Ui::demo_reset() {
  for (int n = 0; n < viewports.length(); n++) {
    Viewport* vPort = (Viewport*)viewports.get(n);
    vPort->reset();
  }
}



void Ui::reset() {
  demo_reset();
  // Allow all viewports to accept user input now.
  Viewport::accept_input();
}



void Ui::set_redraw_arena() {
  for (int n = 0; n < viewports.length(); n++) {
    Viewport* vPort = (Viewport*)viewports.get(n);
    vPort->set_redraw_arena();
  }
}



void Ui::process_event(int dpyNum,XEvent *event) {
  int viewportNum = get_viewport_num(dpyNum,event->xany.window);
    
  // Received WM_DELETE_WINDOW, i.e. from pressing the close window
  // button (usually) in the upper-right of the window.
  if (event->type == ClientMessage && event->xclient.format == 32 &&
      event->xclient.data.l[0] == xvars.wmDeleteWindow[dpyNum]) {

    // One of the top-level viewports.  Quit game.
    // Perhaps this logic should be inside Viewport::process_event().
    // 
    // Also, be nice to have a more general way of handling top-level
    // windows.
    if (viewportNum != -1) {
      settingsChanges |= UIquit;
    }

    // Pop down controls window.
    else if (event->xany.window == xdata.controls[dpyNum]) {
      Viewport* vPort = (Viewport*)viewports.get(vIndex[dpyNum][0]);
      // Turn off control button on zeroth viewport on this display.
      vPort->set_menu_controls(False);
      XUnmapWindow(xvars.dpy[dpyNum],event->xany.window);
    }

    // Pop down learn controls window.
    else if (event->xany.window == xdata.learnControls[dpyNum]) {
      Viewport* vPort = (Viewport*)viewports.get(vIndex[dpyNum][0]);
      vPort->set_menu_learn_controls(False);
      XUnmapWindow(xvars.dpy[dpyNum],event->xany.window);
    }

    // Pop down help window.
    else if (event->xany.window == xdata.help[dpyNum]) {
      assert(dpyNum == 0);
      Viewport* vPort = (Viewport*)viewports.get(vIndex[dpyNum][0]);
      vPort->set_menu_help(False);
      XUnmapWindow(xvars.dpy[dpyNum],event->xany.window);
    }
    return;
  }


  // Turn off pause.
  if (pause) {
    if (event->type == KeyPress || event->type == ButtonPress) {
      settingsChanges |= UIpause;
      settings.pause = False;
    }
    return;
  }


  // Give event to the Viewport.
  if (viewportNum != -1) {
    Viewport* vPort = (Viewport*)viewports.get(viewportNum);
    vPort->process_event(event);
    return;
  }

  
  // See if we need to redraw the controls window.
  if (event->xany.window == xdata.controls[dpyNum] && 
      event->type == Expose) {
    controls_expose(dpyNum,event);
    return;
  }

  
  // Give the learnControls window a crack at it.
  if (learnControls[dpyNum]->process_event(dpyNum,event)) {
    return;
  }


  // Give the Help window a crack at it.
  if (help[dpyNum]->process_event(dpyNum,event)) {
    return;
  }
}



void Ui::pre_clock() {
  int dpyNum;
  for (dpyNum = 0; dpyNum < xvars.dpyMax; dpyNum++) {
    assert(keysetSet[dpyNum]);
  }
  
  if (!pause) {
    otherInput = False;
    
    // Display new arena messages.
    char *arenaMsg;
    Boolean exclusive;
    // Loop, empty the queue.
    do {
      IntelId msgTarget;
      Quanta time;
      Boolean propagate; // not used here.
      exclusive = locator->arena_message_deq(&arenaMsg,msgTarget,time,
                                             propagate);
      if (arenaMsg) {
        // Go through all viewports, setting message on appropriate ones.
        for (int n = 0; n < viewports.length(); n++) {
          Viewport* vPort = (Viewport*)viewports.get(n);
          IntelP intel = vPort->get_intel();

          // If message is exclusive to a certain viewport, 
          // intelId must match msgTarget.
          if (!exclusive ||
              (intel && intel->get_intel_id() == msgTarget)
              ) {
            // -1 means use default value.
            if (time == -1) {
              time = UI_ARENA_MESSAGE_TIME;
            }

            // Either this is the target for the exclusive message or 
            // the message goes to all viewports.
            vPort->set_arena_message(arenaMsg,time);
            
            // Dont' need to redraw all, because we'll redraw the message
            // every turn.

            // We found the viewport we're looking for, so stop looking.
            if (exclusive) {
              break;
            }
          }
        }
        delete arenaMsg;
      }
    } while(arenaMsg);
    
    
    // Clock all the Viewports.
    // Drawing happens here.
    for (int n = 0; n < viewports.length(); n++) {
      Viewport* vPort = (Viewport*)viewports.get(n);
      vPort->pre_clock();
    }
    
    // Update message bar.
    if (messageTimer.ready()) {
      char *msg;
      if (msg = locator->message_deq()) {
        set_message(msg);
        delete msg;
        messageTimer.set(MESSAGE_TIME);
      }
    }
    messageTimer.clock();
  } // pause
  

  // Force all drawing requests to finish at the end of each turn.
  for (dpyNum = 0; dpyNum < xvars.dpyMax; dpyNum++) {
    if (vIndexNum[dpyNum] > 0) {
      XSync(xvars.dpy[dpyNum],False);
    }
  }
}



void Ui::post_clock() {
  // Clock all the Viewports.
  // This initiates drawing.
  for (int n = 0; n < viewports.length(); n++) {
    Viewport* vPort = (Viewport*)viewports.get(n);
    vPort->post_clock();
  }
}



void Ui::menu_quit_CB(void*,Viewport*,void* closure) {
  UiP ui = (UiP)closure;
  ui->settingsChanges |= UIquit;
}



void Ui::menu_new_game_CB(void*,Viewport*,void* closure) {
  UiP ui = (UiP)closure;
  ui->settingsChanges |= UInewGame;
}



void Ui::menu_humans_num_CB(void* value,Viewport*,void* closure) {
  UiP ui = (UiP)closure;
  ui->settingsChanges |= UIhumansNum; 
  const char* chPValue = (const char*)value;
  ui->settings.humansNum = atoi(chPValue);
}



void Ui::menu_enemies_num_CB(void* value,Viewport*,void* closure) {
  UiP ui = (UiP)closure;
  ui->settingsChanges |= UIenemiesNum; 
  const char* chPValue = (const char*)value;
  ui->settings.enemiesNum = atoi(chPValue);
}



void Ui::menu_enemies_refill_CB(void* value,Viewport*,void* closure) {
  UiP ui = (UiP)closure;
  ui->settingsChanges |= UIenemiesRefill;
  // SGI compiler says you can't cast void* to Boolean.
  Boolean val = (Boolean)(intptr_t)value;
  ui->settings.enemiesRefill = val;
}



void Ui::menu_controls_CB(void* value,Viewport* vPort,void* closure) {
  UiP ui = (UiP)closure;
  int dpyNum = vPort->get_dpy_num();
  Boolean val = (Boolean)(intptr_t)value;
  if (val) {
    XMapWindow(ui->xvars.dpy[dpyNum],ui->xdata.controls[dpyNum]);
  }
  else {
    XUnmapWindow(ui->xvars.dpy[dpyNum],ui->xdata.controls[dpyNum]);
  }
}



// Button was pressed to pop up the learn controls dialog.
void Ui::menu_learn_controls_CB(void* value,
                                Viewport* vPort,void* closure) {
  UiP ui = (UiP)closure;
  int dpyNum = vPort->get_dpy_num();
  Boolean active = (Boolean)(intptr_t)value;

  if (active) {
    ui->lControls[dpyNum].input = ui->lControls[dpyNum].key = 
      ui->lControls[dpyNum].which = 0;
      
    ostrstream str;
    str << "There are 2 sets of controls for the players on the right and" 
        << "\n"
        << "left sides of the keyboard, respectively." << "\n"
        << "Each player has " << UI_KEYS_MAX << " commands.  "
        << "Two physical keys on the keyboard can" << "\n"
        << "be mapped to each command.  This is useful when two keys "
        << "are physically" 
        << "\n"
        << "close to each other and you would like both of them to perform" 
        << "\n" << "the same action." << "\n"
        << "\n"
        << "Place the cursor in this window and type the prompted key."
        << "\n" << "\n"
        << (ui->lControls[dpyNum].input ? "Left" : "Right") 
        << " player enter the " 
        << (ui->lControls[dpyNum].which ? "second" : "first") 
        << " key for <" << ui->keysNames[ui->lControls[dpyNum].key] 
        << ">." << ends;
    
    ui->learnControls[dpyNum]->set_message(str.str());
    delete str.str();
    XMapWindow(ui->xvars.dpy[dpyNum],ui->xdata.learnControls[dpyNum]);
  }
  else {
    XUnmapWindow(ui->xvars.dpy[dpyNum],ui->xdata.learnControls[dpyNum]);
  }
}



void Ui::menu_scenarios_CB(void* value,Viewport*,void* closure) {
  UiP ui = (UiP)closure;
  Boolean val = (Boolean)(intptr_t)value;
  if (val) {
    ui->settingsChanges |= UIstyle;
    ui->settings.style = SCENARIOS;
  }
}



void Ui::menu_levels_CB(void* value,Viewport*,void* closure) {
  UiP ui = (UiP)closure;
  Boolean val = (Boolean)(intptr_t)value;
  if (val) {
    ui->settingsChanges |= UIstyle;
    ui->settings.style = LEVELS;
  }
}



void Ui::menu_kill_CB(void* value,Viewport*,void* closure) {
  UiP ui = (UiP)closure;
  Boolean val = (Boolean)(intptr_t)value;
  if (val) {
    ui->settingsChanges |= UIstyle;
    ui->settings.style = KILL;
  }
}



void Ui::menu_duel_CB(void* value,Viewport*,void* closure) {
  UiP ui = (UiP)closure;
  Boolean val = (Boolean)(intptr_t)value;
  if (val) {
    ui->settingsChanges |= UIstyle;
    ui->settings.style = DUEL;
  }
}



void Ui::menu_extended_CB(void* value,Viewport*,void* closure) {
  UiP ui = (UiP)closure;
  Boolean val = (Boolean)(intptr_t)value;
  if (val) {
    ui->settingsChanges |= UIstyle;
    ui->settings.style = EXTENDED;
  }
}



void Ui::menu_training_CB(void* value,Viewport*,void* closure) {
  UiP ui = (UiP)closure;
  Boolean val = (Boolean)(intptr_t)value;
  if (val) {
    ui->settingsChanges |= UIstyle;
    ui->settings.style = TRAINING;
  }
}



void Ui::menu_quanta_CB(void* value,Viewport*,void* closure) {
  UiP ui = (UiP)closure;
  ui->settingsChanges |= UIquanta;
  const char* chPValue = (const char*)value;
  ui->settings.quanta = atoi(chPValue);
}



void Ui::menu_cooperative_CB(void* value,Viewport*,void* closure) {
  UiP ui = (UiP)closure;
  ui->settingsChanges |= UIcooperative;
  Boolean val = (Boolean)(intptr_t)value;
  ui->settings.cooperative = val;
}



void Ui::menu_help_CB(void* value,Viewport* vPort,void* closure) {
  UiP ui = (UiP)closure;
  int dpyNum = vPort->get_dpy_num();
  Boolean active = (Boolean)(intptr_t)value;

  if (active) {
    XMapWindow(ui->xvars.dpy[dpyNum],ui->xdata.help[dpyNum]);
  }
  else {
    XUnmapWindow(ui->xvars.dpy[dpyNum],ui->xdata.help[dpyNum]);
  }
}



void Ui::status_weapon_CB(void* value,Viewport* vPort,void*) {
  // Some mouse controls.
  //  UiP ui = (UiP)closure;
  intptr_t button = (intptr_t)value;

  switch (button) {
  case Button1:
    vPort->dispatch(IT_WEAPON_CENTER,NULL);
    break;
  case Button2:
    vPort->dispatch(IT_WEAPON_CHANGE,NULL);
    break;
  case Button3:
    vPort->dispatch(IT_WEAPON_DROP,NULL);
    break;
  }
}



void Ui::status_item_CB(void* value,Viewport* vPort,void*) {
  //  UiP ui = (UiP)closure;
  intptr_t button = (intptr_t)value;

  switch (button) {
  case Button1:
    vPort->dispatch(IT_ITEM_USE,NULL);
    break;
  case Button2:
    vPort->dispatch(IT_ITEM_CHANGE,NULL);
    break;
  case Button3:
    vPort->dispatch(IT_ITEM_DROP,NULL);
    break;
  }
}



void Ui::chat_CB(void* value,Viewport* vPort,void* closure) {
  const char* message = (const char*)value;

  UiP ui = (UiP)closure;
  ui->settingsChanges |= UIchatRequest;

  // No UI for specifying the receiver yet.
  ui->settings.chatReceiver[0] = '\0';

  // Copy over the message.
  strncpy(ui->settings.chatMessage,message,UI_CHAT_MESSAGE_MAX);
  ui->settings.chatMessage[UI_CHAT_MESSAGE_MAX] = '\0';
}



void Ui::learn_controls_CB(Panel* panel,void* value,void* closure) {
  UiP ui = (UiP)closure;
  int dpyNum = panel->get_dpy_num();
  
  XEvent* event = (XEvent*)value;
  if (event->type != KeyPress) {
    return;
  }

  // Viewport containing the [Learn Controls] button.  panel has no viewport.
  int vNum = ui->vIndex[dpyNum][0]; 
  Viewport* vPort = (Viewport*)ui->viewports.get(vNum);
  

  // If the learnControls button isn't pressed, don't do anything with
  // the learn Controls dialog.
  // This shouldn't happen, but does for some reason on an Athena SGI.
  if (!vPort->get_menu_learn_controls()) {
    return;
  }

  ostrstream str;
  str << (ui->lControls[dpyNum].input ? "Left" : "Right") 
      << " player: The "
      << (ui->lControls[dpyNum].which ? "second" : "first") << " key for <" 
      << ui->keysNames[ui->lControls[dpyNum].key] << "> is "
      << keysym_to_string(XKeycodeToKeysym(ui->xvars.dpy[dpyNum],
					  event->xkey.keycode,0))
      << "." << "\n" << "\n";
  ui->keycodes[dpyNum][ui->lControls[dpyNum].input]
    [ui->lControls[dpyNum].key][ui->lControls[dpyNum].which] =
      event->xkey.keycode;
  

#if 0
  // Some special code to only store one key for chat.
  if (ui->lControls[dpyNum].key == IT_CHAT) {
    assert(ui->lControls[dpyNum].which == 0);
    ui->keycodes[dpyNum][1] = ui->keycodes[dpyNum][0];
    ui->lControls[dpyNum].which++;
  }
#endif
  // Need something so that only does chat for first player.

  assert(IT_CHAT == UI_KEYS_MAX - 1);

  // "Carry" bits for the counter
  //
  // Increment "which" of the two keys.
  if (ui->lControls[dpyNum].which < 1) {
    ui->lControls[dpyNum].which++;
  }
  // Increment the key being set.
  else if (ui->lControls[dpyNum].key < (UI_KEYS_MAX - 1)
           // Don't set chat key for second player.
           && (!(ui->lControls[dpyNum].key == (IT_CHAT - 1)
                 && ui->lControls[dpyNum].input > 0))) {
    ui->lControls[dpyNum].key++;
    ui->lControls[dpyNum].which = 0;
  }
  // Increment between right and left players
  else if (ui->lControls[dpyNum].input < 1) {
    ui->lControls[dpyNum].input++;
    ui->lControls[dpyNum].key = 0;
    ui->lControls[dpyNum].which = 0;
  }
  // Done
  else {
    vPort->set_menu_learn_controls(False);
    XUnmapWindow(ui->xvars.dpy[dpyNum],ui->xdata.learnControls[dpyNum]);
  }
  
  str << (ui->lControls[dpyNum].input ? "Left" : "Right") 
      << " player enter the " 
      << (ui->lControls[dpyNum].which ? "second" : "first") 
      << " key for <" << keysNames[ui->lControls[dpyNum].key] 
	<< ">." << ends;

  // Add message in front of str.str().
  ostrstream fullStr;
  if (ui->lControls[dpyNum].input == UI_KEYS_LEFT) {
    fullStr << "You may dismiss this dialog now if you only want to" << "\n"
           << "set the controls for one player." 
            << "\n" << "\n";
  }
  fullStr << str.str() << ends;
  delete str.str();
 

  ui->learnControls[dpyNum]->set_message(fullStr.str());
  delete fullStr.str();

  ui->controls_redraw(dpyNum);
}



void Ui::set_message(const char *message) {
  for (int n = 0; n < viewports.length(); n++) {
    Viewport* vPort = (Viewport*)viewports.get(n);
    vPort->set_message(message);
  }
}



void Ui::init_x() {
  // Map from viewports to displays.
  // Now just a local variable instead of a member, the info
  // is recomputed when the Viewport object is actually created.
  int dpyIndex[UI_VIEWPORTS_MAX]; 

  // Initialize Xvars.
  xvars.dpyMax = 0;

  // Assign a display to all viewports.
  for (int vNum = 0; vNum < UI_VIEWPORTS_MAX; vNum++) {
    Boolean found = False;
    for (int m = 0; m < vNum && !found; m++) {
      // Use already opened display.
      if (!strcmp(displayNames[m],displayNames[vNum])) {
        int theDisplay = dpyIndex[m];
        dpyIndex[vNum] = theDisplay;
        vIndex[theDisplay][vIndexMax[theDisplay]] = vNum;
        vIndexMax[theDisplay]++;
        found = True;
      }
    }

    // Open new display.
    if (!found) {
      if (xvars.dpyMax >= Xvars::DISPLAYS_MAX) {
        cerr << "Can only open " << Xvars::DISPLAYS_MAX <<
          " different displays." << endl;
        cerr << "If more displays are desired, recompile with "
             << "a higher value for Xvars::DISPLAYS_MAX." << endl;
        exit(1);
      }
      if (!(xvars.dpy[xvars.dpyMax] = 
            XOpenDisplay(strlen(displayNames[vNum]) ?
                         displayNames[vNum] :(char *) NULL))) {
        cerr << "Could not open X display " << displayNames[xvars.dpyMax] 
             << endl;
        if (strlen(displayNames[xvars.dpyMax])) {
          cerr << "Make sure you have used xhost on " 
               << displayNames[xvars.dpyMax] 
               << " to allow this machine to connect to it." << endl;
        }
        exit(1);
      }
      // Synchronous, for debugging.
      if (synchronous) {
        XSynchronize(xvars.dpy[xvars.dpyMax],True);
      }

      // Initialize a bunch of useful X variables.
      xvars.scr_ptr[xvars.dpyMax] = 
        DefaultScreenOfDisplay(xvars.dpy[xvars.dpyMax]);
      xvars.scr_num[xvars.dpyMax] = 
        DefaultScreen(xvars.dpy[xvars.dpyMax]);
      xvars.root[xvars.dpyMax] = 
        RootWindowOfScreen(xvars.scr_ptr[xvars.dpyMax]);
      xvars.visual[xvars.dpyMax] = 
        DefaultVisual(xvars.dpy[xvars.dpyMax],xvars.scr_num[xvars.dpyMax]);
      xvars.depth[xvars.dpyMax] = 
        DefaultDepthOfScreen(xvars.scr_ptr[xvars.dpyMax]);
      xvars.cmap[xvars.dpyMax] = 
        DefaultColormap(xvars.dpy[xvars.dpyMax],xvars.scr_num[xvars.dpyMax]);
      xvars.white[xvars.dpyMax] = 
        WhitePixel(xvars.dpy[xvars.dpyMax],xvars.scr_num[xvars.dpyMax]);
      xvars.black[xvars.dpyMax] = 
        BlackPixel(xvars.dpy[xvars.dpyMax],xvars.scr_num[xvars.dpyMax]);
      
      
      // Get font.  Use user-specified font if given.
      // regular size font.
      const char *theFont = fontName ? fontName : DEFAULT_FONT_NAME;
      xvars.font[xvars.dpyMax] = 
        XLoadQueryFont(xvars.dpy[xvars.dpyMax],theFont);
      if (!xvars.font[xvars.dpyMax]) {
        cerr << "Could not load font " << theFont << " trying backup font "
             << BACKUP_FONT_NAME << endl;
        // Try backup font, should always be there.
        xvars.font[xvars.dpyMax] = 
          XLoadQueryFont(xvars.dpy[xvars.dpyMax],BACKUP_FONT_NAME);
      }
      if (!xvars.font[xvars.dpyMax]) {
        // Really failed.
        cerr << "Could not load " << BACKUP_FONT_NAME;
        if (strlen(displayNames[vNum]))
          cerr << " on " << displayNames[vNum];
        cerr << endl;
        exit (1);
      }
      
      // BigFont
      theFont = DEFAULT_BIG_FONT_NAME;
      xvars.bigFont[xvars.dpyMax] = 
        XLoadQueryFont(xvars.dpy[xvars.dpyMax],theFont);
      if (!xvars.bigFont[xvars.dpyMax]) {
        cerr << "Could not load font " << theFont << " trying backup font "
         << BACKUP_FONT_NAME << endl;
        // Try backup font, should always be there.
        xvars.bigFont[xvars.dpyMax] = 
          XLoadQueryFont(xvars.dpy[xvars.dpyMax],BACKUP_FONT_NAME);
      }
      if (!xvars.bigFont[xvars.dpyMax]) {
        // Really failed.
        cerr << "Could not load " << BACKUP_FONT_NAME;
        if (strlen(displayNames[vNum]))
          cerr << " on " << displayNames[vNum];
        cerr << endl;
        exit (1);
      }
      
      // For convenience, compute sizes of fonts.
      xvars.fontSize[xvars.dpyMax].width = 
        xvars.font[xvars.dpyMax]->max_bounds.width;
      xvars.fontSize[xvars.dpyMax].height = 
        xvars.font[xvars.dpyMax]->max_bounds.ascent 
        + xvars.font[xvars.dpyMax]->max_bounds.descent;
            
      xvars.bigFontSize[xvars.dpyMax].width = 
        xvars.bigFont[xvars.dpyMax]->max_bounds.width;
      xvars.bigFontSize[xvars.dpyMax].height = 
        xvars.bigFont[xvars.dpyMax]->max_bounds.ascent 
        + xvars.bigFont[xvars.dpyMax]->max_bounds.descent;
      
      // Create graphics context.
      XGCValues values;
      values.graphics_exposures = False;
      values.font = xvars.font[xvars.dpyMax]->fid;
      values.foreground = xvars.black[xvars.dpyMax];
      values.background = xvars.white[xvars.dpyMax];
      values.stipple = 
        XCreateBitmapFromData(xvars.dpy[xvars.dpyMax],
                  xvars.root[xvars.dpyMax],INSENSITIVE_BITS,
                  INSENSITIVE_WIDTH,INSENSITIVE_HEIGHT);
      xvars.gc[xvars.dpyMax] = 
        XCreateGC(xvars.dpy[xvars.dpyMax],xvars.root[xvars.dpyMax],
              GCGraphicsExposures | GCFont | GCForeground | 
              GCBackground | GCStipple,
              &values);

      // Some colors.
      xvars.red[xvars.dpyMax] = 
        xvars.alloc_named_color(xvars.dpyMax,RED_COLOR,
                  xvars.black[xvars.dpyMax]);
      xvars.green[xvars.dpyMax] = 
        xvars.alloc_named_color(xvars.dpyMax,GREEN_COLOR,
                  xvars.black[xvars.dpyMax]);
      xvars.arenaTextColor[xvars.dpyMax] = 
        xvars.alloc_named_color(xvars.dpyMax,ARENA_TEXT_COLOR,
                  xvars.white[xvars.dpyMax]);
      xvars.windowBg[xvars.dpyMax] = 
        xvars.alloc_named_color(xvars.dpyMax,Xvars_WINDOW_BG_COLOR);
      xvars.windowBorder[xvars.dpyMax] = 
        xvars.alloc_named_color(xvars.dpyMax,Xvars_WINDOW_BORDER_COLOR);
      
      for (int m = 0; m < Xvars::HUMAN_COLORS_NUM; m++) {
        xvars.humanColors[xvars.dpyMax][m] =
          xvars.alloc_named_color(xvars.dpyMax,Xvars::humanColorNames[m],
                    xvars.black[xvars.dpyMax]);
      }
      
      
      // Initialize xdata.  I.e. values local to the Ui.
      XColor color;
      color.red = color.green = color.blue = 0;
      color.flags = DoRed | DoGreen | DoBlue;

      Pixmap pixmap = 
        XCreateBitmapFromData(xvars.dpy[xvars.dpyMax],
                              xvars.root[xvars.dpyMax],CURSOR_BITS,
                              CURSOR_WIDTH,CURSOR_HEIGHT);
      Pixmap mask = 
        XCreateBitmapFromData(xvars.dpy[xvars.dpyMax],
                              xvars.root[xvars.dpyMax],CURSOR_MASK_BITS,
                              CURSOR_WIDTH,CURSOR_HEIGHT);
      xvars.arenaCursor[xvars.dpyMax] = 
        XCreatePixmapCursor(xvars.dpy[xvars.dpyMax],pixmap,mask,
                            &color,&color,CURSOR_HOT_X,CURSOR_HOT_Y);
      XFreePixmap(xvars.dpy[xvars.dpyMax],pixmap);
      XFreePixmap(xvars.dpy[xvars.dpyMax],mask);
      

      xvars.iconPixmap[xvars.dpyMax] = 0;
      xvars.iconMask[xvars.dpyMax] = 0;

// (??)Doesn't work, pixmap must be 1-bit deep when specified this way.

      // Use color XEvil icon when program is iconified.
      //
      // A little funny, calling load_pixmap() before xvars is fully 
      // initialized.
      xvars.load_pixmap(&xvars.iconPixmap[xvars.dpyMax],
                        &xvars.iconMask[xvars.dpyMax],
                        xvars.dpyMax,XPM_ICON_BITS,True);
#if 0
      xvars.iconPixmap[xvars.dpyMax] = 
        XCreateBitmapFromData(xvars.dpy[xvars.dpyMax],
                              xvars.root[xvars.dpyMax],ICON_BITS,
                              ICON_WIDTH,ICON_HEIGHT);
#endif
      
      // Maps between displays and viewports.
      dpyIndex[vNum] = xvars.dpyMax;
      vIndex[xvars.dpyMax][vIndexNum[xvars.dpyMax]] = vNum;
      vIndexMax[xvars.dpyMax]++;
      xvars.dpyMax++;
    }
  }

  intern_atoms();
  XSetErrorHandler(&UI_error_handler);
}



void Ui::intern_atoms() {
  for (int dpyNum = 0; dpyNum < xvars.dpyMax; dpyNum++) {
    xvars.wmProtocols[dpyNum] = 
      XInternAtom(xvars.dpy[dpyNum],"WM_PROTOCOLS",False);
    xvars.wmDeleteWindow[dpyNum] = 
      XInternAtom(xvars.dpy[dpyNum],"WM_DELETE_WINDOW",False);
  }
}



int Ui::get_viewport_num(int dpyNum,Window window) {
  // Lame-ass linear search.
  // Could use the HashTable class now that we have it.
  for (int v = 0; v < vIndexNum[dpyNum]; v++) {
    int viewportNum = vIndex[dpyNum][v];
    Viewport* vPort = (Viewport*)viewports.get(viewportNum);
    if (vPort->has_window(window)) {
      return viewportNum;
    }
  }
  
  return -1;
}



void Ui::create_controls() {
  // Need to move all this window-creation code into a common method, 
  // probably on Xvars.
  for (int dpyNum = 0; dpyNum < xvars.dpyMax; dpyNum++) {
    Size size;
    size.width = xvars.font[dpyNum]->max_bounds.width * CONTROLS_COLS;
    size.height = 
      (xvars.font[dpyNum]->max_bounds.ascent 
       + xvars.font[dpyNum]->max_bounds.descent) 
	  * CONTROLS_ROWS;

    xdata.controls[dpyNum] = 
      xvars.create_toplevel_window(argc,argv,dpyNum,size,
                                   "XEvil Controls",ExposureMask);
  } // dpyNum
}



void Ui::create_learn_controls() {
  for (int dpyNum = 0; dpyNum < xvars.dpyMax; dpyNum++) {
    Size size = 
      TextPanel::get_unit(xvars.font[dpyNum],
                          LEARN_CONTROLS_COLS,LEARN_CONTROLS_ROWS);
    xdata.learnControls[dpyNum] = 
      xvars.create_toplevel_window(argc,argv,dpyNum,size,
                                   "Set Controls", 
                                   ExposureMask | KeyPressMask);
    
    Pos pos;  // Initialized to (0,0)
    learnControls[dpyNum] = 
      new KeyPressPanel(dpyNum,xvars,xdata.learnControls[dpyNum],
                        pos,size,Ui::learn_controls_CB,(void*)this);
    assert(learnControls[dpyNum]);    
  } // dpyNum
}



void Ui::create_help() {
  // Parse the helpMessage and create a string with appropriate line breaks
  // for the TextPanel.
  Line::set_text_columns(HELP_COLS);
  Page page(NULL,helpMessage);
  const PtrList& lines = page.get_lines();
  ostrstream message;
  for (int n = 0; n < lines.length(); n++) {
    Line* line = (Line*)lines.get(n);
    char* text = line->alloc_text();
    // Careful to use '\n' for line break.
    message << text << '\n';
    delete [] text;
  }
  message << ends;


  for (int dpyNum = 0; dpyNum < xvars.dpyMax; dpyNum++) {
    // Create the top-level window.
    Size size = 
      TextPanel::get_unit(xvars.font[dpyNum],
                          HELP_COLS,lines.length());
    xdata.help[dpyNum] = 
      xvars.create_toplevel_window(argc,argv,dpyNum,size,
                                   "XEvil Help", 
                                   ExposureMask);
    
    // Create the TextPanel inside.
    Pos pos;  // Initialized to (0,0)
    help[dpyNum] = 
      new TextPanel(dpyNum,xvars,xdata.help[dpyNum],pos,size);
    assert(help[dpyNum]);    

    help[dpyNum]->set_message(message.str());
  } // dpyNum

  delete message.str();
}



void Ui::controls_redraw(int dpyNum) {
  XClearWindow(xvars.dpy[dpyNum],xdata.controls[dpyNum]);

  // Temporary list of keysyms.
  //
  // Indexed by 
  // [right or left]
  // [key number]
  // [which of the two possible keys]
  // [possibly multiple names, e.g. 'M' and 'm', for same keycode]
  KeySym keymaps[2][UI_KEYS_MAX][2][2];  
  for (int input = 0; input < 2; input++) {
    for (int nn = 0; nn < UI_KEYS_MAX; nn++) {
      for (int which = 0; which < 2; which++) {
        for (int i = 0; i < 2; i++) {
          unsigned int keycode = keycodes[dpyNum][input][nn][which];
          keymaps[input][nn][which][i] = 
            XKeycodeToKeysym(xvars.dpy[dpyNum],keycode,i);
        }
      }
    }
  }


  // Line number, I have no idea why I called it "p".
  int p = 0;


  // Fluff at the top
  char player[] = 
    "Left Side                             >>>>>> Right Side (DEFAULT) <<<<<<";
  XDrawString(xvars.dpy[dpyNum],xdata.controls[dpyNum],xvars.gc[dpyNum],
	      xvars.fontSize[dpyNum].width,
	      xvars.font[dpyNum]->max_bounds.ascent 
	      + (p++) * xvars.fontSize[dpyNum].height,
	      player,strlen(player));

  char line[] =  
    "------------------------------------------------------------------------------------------";
  XDrawString(xvars.dpy[dpyNum],xdata.controls[dpyNum],xvars.gc[dpyNum],
	      xvars.fontSize[dpyNum].width,
	      xvars.font[dpyNum]->max_bounds.ascent 
	      + (p++) * xvars.fontSize[dpyNum].height,
	      line,strlen(line));


  // // Loop through all keys.
  for (int n = 0; n < UI_KEYS_MAX; n++) {
    // String for right side.
    strstream str0;
    str0 << keysNames[n] << ":  " << keysym_to_string(keymaps[0][n][0][0]);
    if (keymaps[0][n][0][1] && strlen(keysym_to_string(keymaps[0][n][0][1])))
      str0 << ", " << keysym_to_string(keymaps[0][n][0][1]);
    if (keymaps[0][n][0][0] != keymaps[0][n][1][0] && keymaps[0][n][1][0]) {
	  str0 << ", " << keysym_to_string(keymaps[0][n][1][0]);
	  if (keymaps[0][n][1][1]
	      && strlen(keysym_to_string(keymaps[0][n][1][1])))
	    str0 << ", " << keysym_to_string(keymaps[0][n][1][1]);
	}
    str0 << ends;
    
    // String for left side.
    strstream str1;
    str1 << keysNames[n] << ":  " << keysym_to_string(keymaps[1][n][0][0]);
    if (keymaps[1][n][0][1] && strlen(keysym_to_string(keymaps[1][n][0][1]))) {
      str1 << ", " << keysym_to_string(keymaps[1][n][0][1]);
    }
    if (keymaps[1][n][0][0] != keymaps[1][n][1][0] && keymaps[1][n][1][0]) {
	  str1 << ", " << keysym_to_string(keymaps[1][n][1][0]);
	  if (keymaps[1][n][1][1]
	      && strlen(keysym_to_string(keymaps[1][n][1][1])))
	    str1 << ", " << keysym_to_string(keymaps[1][n][1][1]);
	}
    str1 << ends;
    
    // Draw key on left side.
    if (n != IT_CHAT) {  // Chat meaningless for left player.
      XDrawString(xvars.dpy[dpyNum],xdata.controls[dpyNum],xvars.gc[dpyNum],
                  xvars.fontSize[dpyNum].width,
                  xvars.font[dpyNum]->max_bounds.ascent 
                  + p * xvars.fontSize[dpyNum].height,
                  str1.str(),strlen(str1.str()));
    }
    // Draw on right side.
    XDrawString(xvars.dpy[dpyNum],xdata.controls[dpyNum],xvars.gc[dpyNum],
                xvars.fontSize[dpyNum].width * (CONTROLS_COLS / 2 + 1),
                xvars.font[dpyNum]->max_bounds.ascent 
                + (p++) * xvars.fontSize[dpyNum].height,
                str0.str(),strlen(str0.str()));
    delete str0.str();
    delete str1.str();
  }
  p++;
  

  // Fluff at the bottom.
  char line2[] =  
    "Keyboard controls can be set with the -keys command line option";
  XDrawString(xvars.dpy[dpyNum],xdata.controls[dpyNum],xvars.gc[dpyNum],
              xvars.fontSize[dpyNum].width,
              xvars.font[dpyNum]->max_bounds.ascent
              + (p++) * xvars.fontSize[dpyNum].height,
              line2,strlen(line2));
  char line3[] =  
    "the [Set controls] button or with the appropriate X resources.";
  XDrawString(xvars.dpy[dpyNum],xdata.controls[dpyNum],xvars.gc[dpyNum],
              xvars.fontSize[dpyNum].width,
              xvars.font[dpyNum]->max_bounds.ascent 
              + (p++) * xvars.fontSize[dpyNum].height,
              line3,strlen(line3));
  p++;
}



IViewportInfo* Ui::get_viewport_info() {
  return Viewport::get_info();
}



char *Ui::keysNames[UI_KEYS_MAX] = {
  "center",
  "right",
  "down_right",
  "down",
  "down_left",
  "left",
  "up_left",
  "up",
  "up_right",
  "weapon_use",
  "weapon_change",
  "weapon_drop",
  "item_use",
  "item_change",
  "item_drop",
  "chat",
};



Boolean Ui::synchronous = False;



// The default value.
Boolean Ui::largeViewport = True;



Boolean Ui::smoothScroll = False;



ViewportCallback Ui::viewportCallbacks[VIEWPORT_CB_NUM] = {
  menu_controls_CB,
  menu_learn_controls_CB,
  menu_quit_CB,
  menu_new_game_CB,
  menu_humans_num_CB,
  menu_enemies_num_CB,
  menu_enemies_refill_CB,
  NULL,
  menu_scenarios_CB,
  menu_levels_CB,
  menu_kill_CB,
  menu_duel_CB,
  menu_extended_CB,
  menu_training_CB,
  menu_quanta_CB,
  menu_cooperative_CB,
  menu_help_CB,
  status_weapon_CB,
  status_item_CB,
  chat_CB,
};



char* Ui::helpMessage = 
"For full instructions, including NETWORK PLAY, see "
"http://www.xevil.com/docs/instructions.html\n"
"\n"
"Run 'xevil -help' for usage and basic NETWORK PLAY options.\n"
"\n"
"Use the \"Set Controls\" and \"Show Controls\" buttons to configure the "
"keyboard.\n"
"\n"
"XEvil(TM) Copyright(C) 1994,1999  Steve Hardt and Michael Judge\n"
"http://www.xevil.com   satan@xevil.com"
;

