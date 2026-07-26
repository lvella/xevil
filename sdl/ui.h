/*
 * XEvil(TM) Copyright (C) 1994,2000 Steve Hardt and Michael Judge
 * http://www.xevil.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.  See gpl.txt.
 */

// "ui.h"  SDL port -- user interface module.
//
// Same public surface cmn/ consumes as the X11 Ui (constructed by Game, driven
// by main's pre_clock/post_clock/process_event), but a much simpler internal
// model: one SDL window with one Viewport.  Two-player viewports, the in-game
// menus/dialogs, and live key remapping are stage-2 stubs (documented below).

#ifndef UI_H
#define UI_H

#include "utils.h"
#include "coord.h"
#include "world.h"
#include "locator.h"
#include "id.h"
#include "intel.h"
#include "game_style.h"
#include "sound.h"
#include "ui_cmn.h"
#include "xdata.h"
#include "viewport.h"

#define UI_VIEWPORTS_MAX 6
#define UI_SHORT_STRING_LENGTH 80


enum UIkeyset {UIsun3, UIdecmips, UIiris, UIncd, UItektronix, UIsun4, UIrsaix,
               UIsun4_sparc,UImac,UIalpha,UIlinux,UIunspecifiedKeyset};

class Ui;
typedef Ui *UiP;

// First index is an IT_COMMAND, second is one of two.
typedef KeySym UIkeymap[UI_KEYS_MAX][2];


// Mirrors the X11 UIsettings so cmn/game.cpp's get_settings() reads identical
// fields.  Only the fields the SDL/X11 (non-WIN32) paths touch are meaningful.
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
  char connectHostname[R_NAME_MAX];
  CMN_PORT connectPort;
  char humanName[IT_STRING_LENGTH];
  CMN_PORT serverPort;
  Boolean localHuman;
  char chatReceiver[IT_STRING_LENGTH];
  char chatMessage[UI_CHAT_MESSAGE_MAX + 1];
};



class Ui {
public:
  Ui(int *argc,char **argv,WorldP w,LocatorP l,
     char **displayNames,char *fontName,SoundManager *,
     const DifficultyLevel dLevels[DIFFICULTY_LEVELS_NUM],
     RoleType);
  ~Ui();

  int get_viewports_num() {return viewportsNum;}
  int get_viewports_num_on_dpy(int) {return viewportsNum;}
  int get_viewport_on_dpy(int,int) {return 0;}

  int get_dpy_max() {return xvars.dpyMax;}
  int get_dpy_num(int) {return 0;}

  UImask get_settings(UIsettings &s);
  const char *const *get_keys_names() {return (const char *const *)keysNames;}
  Boolean settings_changed() {return settingsChanges != UInone;}
  Boolean keyset_set(int dpyNum) {return keysetSet[dpyNum];}

  void set_humans_num(int);
  void set_enemies_num(int);
  void set_enemies_refill(Boolean);
  void set_style(GameStyleType);
  void set_quanta(Quanta);
  void set_cooperative(Boolean);

  // Sound UI: no-ops / minimal, as on X11.
  void set_track_volume(int) {}
  void set_sound_volume(int) {}
  void set_sound_onoff(Boolean val) {soundOn = val;}
  void set_world_rooms(const Rooms &) {}
  void set_role_type(RoleType) {}   // window-title change not modeled

  void set_humans_playing(int);
  void set_enemies_playing(int);
  void set_level(const char *);

  static void set_reduce_draw(Boolean val) {reduceDraw = val;}
  static void set_use_buffer(Boolean val) {useBuffer = val;}

  Boolean other_input() {return otherInput;}

  void set_input(int vNum,UIinput input);

  void set_keyset(int dpyNum,UIkeyset keyset);
  void set_keyset(int dpyNum,UIkeyset basis,KeySym right[UI_KEYS_MAX][2],
                  KeySym left[UI_KEYS_MAX][2]);

  void set_difficulty(int) {}
  void set_pause(Boolean);

  void set_prompt_difficulty();
  void unset_prompt_difficulty();
  int get_difficulty() {return difficulty;}

  int add_viewport();
  void del_viewport();

  void register_intel(int n,IntelP intel);

  void demo_reset();
  void reset();
  void set_redraw_arena();

  void process_event(int dpyNum,SDL_Event *event);
  void pre_clock();
  void post_clock();

  static IViewportInfo *get_viewport_info() {return Viewport::get_info();}

  static void set_synchronous() {}

  static void set_large_viewport(Boolean val) {largeViewport = val;}
  static void set_smooth_scroll(Boolean val) {smoothScroll = val;}
  static Boolean get_large_viewport() {return largeViewport;}
  static Boolean get_smooth_scroll() {return smoothScroll;}

  static void set_scale(int val) {scale = val;}
  static int get_scale() {return scale;}

  static void set_fullscreen(Boolean val) {fullscreen = val;}
  static Boolean get_fullscreen() {return fullscreen;}


private:
  void refresh_menu_text();

  static char *keysNames[UI_KEYS_MAX];

  char **argv;
  int argc;
  Xvars xvars;
  char **displayNames;
  char *fontName;

  Viewport *viewport;   // single viewport (two-player is stage 2)
  int viewportsNum;

  WorldP world;
  LocatorP locator;

  Boolean keysetSet[Xvars::DISPLAYS_MAX];

  UIsettings settings;
  UImask settingsChanges;
  Boolean otherInput;
  Boolean pause;
  Boolean soundOn;

  RoleType roleType;
  int difficulty;
  const DifficultyLevel *difficultyLevels;

  // Live menu-bar display values.
  int humansNumDisplay;
  int enemiesNumDisplay;
  Quanta quantaDisplay;
  GameStyleType styleDisplay;

  static Boolean largeViewport;
  static Boolean smoothScroll;
  static int scale;
  static Boolean fullscreen;
  static Boolean reduceDraw;
  static Boolean useBuffer;
};

#endif
