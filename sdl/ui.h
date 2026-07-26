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
// by main's pre_clock/post_clock/process_event), but a simpler internal model:
// one SDL window with one Viewport.  Input, the interactive menu bar / HUD, the
// on-arena difficulty prompt, and chat are now real; live key remapping and
// local two-player viewports remain stage-2+ stubs (documented below).

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

// Data-driven key bindings (so "Set Controls" can rebind and persist them):
// keymap[side][virtual key][alternate].  Two input sides (RIGHT numpad player,
// LEFT a-s-d player), up to UI_KEY_ALTS physical keys each.  0 == unbound.
#define UI_INPUT_SIDES 2
#define UI_KEY_ALTS    3

// Which on-arena overlay panel (Help / Show Controls / Set Controls) is up.
enum UIoverlayMode {UIoverlayNone,UIoverlayHelp,UIoverlayShowControls,
                    UIoverlaySetControls};


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
  void set_sound_onoff(Boolean val);
  void set_world_rooms(const Rooms &) {}
  void set_role_type(RoleType val) {roleType = val;}

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
  void change_difficulty(int val) {difficulty = val;}
  void key_event(SDL_Keycode sym,Boolean down);
  /* EFFECTS: Broadcast the SDL key to EVERY viewport: each maps it through its
     own input side (RIGHT numpad / LEFT a-s-d cluster) and keeps the ones that
     match, so a single keyboard drives both local players regardless of which
     window is focused -- the X11 broadcast model (map_render_ui 2.2, 7). */

  void run_license_agreement();
  /* EFFECTS: First-run modal License Agreement (bypassed by -accept_agreement
     or a prior acceptance recorded in ~/.xevilrc).  exit(1) on Reject. */

  int resolve_local_windows();
  /* EFFECTS: 2 for a stand-alone "-humans 2+", else 1. */

  static int pick_fullscreen_scale();
  /* EFFECTS: Largest integer scale (1..4) whose game window fits the desktop,
     for -fullscreen with no explicit -scale (mirrors x11/ui.cpp). */

  static void clamp_scale_to_desktop();
  /* EFFECTS: For an explicitly-requested -scale whose game window would exceed
     the desktop, step scale down one level at a time until it fits, printing a
     one-line stderr note per step (the screen-fit clamp x11/ui.cpp performs and
     -help promises).  No-op if the desktop size is unknown or scale==1. */

  void toggle_fullscreen(int dpyNum);
  /* EFFECTS: F11 runtime toggle of desktop-fullscreen for one window. */

  // ---- Key bindings (data-driven; editable via Set Controls) ----
  void init_keymap_defaults();
  void load_keymap();          // overlay saved rebinds from ~/.xevil_sdl_keys
  void save_keymap();          // persist the RIGHT (player-1) bindings
  int  key_to_virtual(SDL_Keycode sym,UIinput input) const;
  /* EFFECTS: Scan this side's bindings for sym; returns the IT* virtual key or
     -1.  Replaces the old hard-coded switch. */
  void rebind_key(int side,int virtualKey,SDL_Keycode sym);
  void strip_right_letters();  // drop w/a/s/d from RIGHT (two-player anti-clash)

  // ---- Overlays: Help / Show Controls / Set Controls ----
  void open_overlay(UIoverlayMode mode);
  void close_overlay();
  void draw_overlay();
  Boolean overlay_key(const SDL_Keysym &ks);   // True == consumed
  Boolean overlay_mouse(int button,int x,int y);
  void build_help_pages();
  void sync_overlay_toggles();   // reflect `overlay` in the three menu toggles
  void controls_row_layout(int &listTop,int &rowH);   // Show/Set list geometry

  // ---- Menu / HUD callbacks (registered in viewportCallbacks[]).  Signature
  // matches ViewportCallback: (value, viewport, closure==Ui*). ----
  static void menu_quit_CB(void *,Viewport *,void *);
  static void menu_new_game_CB(void *,Viewport *,void *);
  static void menu_humans_num_CB(void *,Viewport *,void *);
  static void menu_enemies_num_CB(void *,Viewport *,void *);
  static void menu_enemies_refill_CB(void *,Viewport *,void *);
  static void menu_controls_CB(void *,Viewport *,void *);
  static void menu_learn_controls_CB(void *,Viewport *,void *);
  static void menu_scenarios_CB(void *,Viewport *,void *);
  static void menu_levels_CB(void *,Viewport *,void *);
  static void menu_kill_CB(void *,Viewport *,void *);
  static void menu_duel_CB(void *,Viewport *,void *);
  static void menu_extended_CB(void *,Viewport *,void *);
  static void menu_training_CB(void *,Viewport *,void *);
  static void menu_survival_CB(void *,Viewport *,void *);
  static void menu_bossrush_CB(void *,Viewport *,void *);
  static void menu_quanta_CB(void *,Viewport *,void *);
  static void menu_cooperative_CB(void *,Viewport *,void *);
  static void menu_sound_CB(void *,Viewport *,void *);
  static void menu_help_CB(void *,Viewport *,void *);
  static void status_weapon_CB(void *,Viewport *,void *);
  static void status_item_CB(void *,Viewport *,void *);
  static void chat_CB(void *,Viewport *,void *);

  static ViewportCallback viewportCallbacks[VIEWPORT_CB_NUM];
  static char *keysNames[UI_KEYS_MAX];

  char **argv;
  int argc;
  Xvars xvars;
  char **displayNames;
  char *fontName;

  // Local two-player = a second SDL window/renderer (each needs its own texture
  // set, so each is a separate Xvars "display").  viewports[i] renders to
  // display i; `viewport` aliases viewports[0], the primary that owns the menu
  // bar / HUD / overlays.
  Viewport *viewports[UI_VIEWPORTS_MAX];
  Viewport *viewport;
  int viewportsNum;
  int localWindows;     // number of SDL windows opened (1, or 2 for -humans 2)

  int viewport_for_window(Uint32 windowID);  // which viewport a WM event hit
  Viewport *viewport_on_display(int dpyNum);  // viewport rendering to window d

  WorldP world;
  LocatorP locator;

  Boolean keysetSet[Xvars::DISPLAYS_MAX];

  UIsettings settings;
  UImask settingsChanges;
  Boolean otherInput;
  Boolean pause;
  Boolean soundOn;
  Boolean promptDifficulty;

  RoleType roleType;
  int difficulty;
  const DifficultyLevel *difficultyLevels;

  // Data-driven key bindings.
  SDL_Keycode keymap[UI_INPUT_SIDES][UI_KEYS_MAX][UI_KEY_ALTS];

  // Overlay (Help / Show Controls / Set Controls) state.
  UIoverlayMode overlay;
  int overlayPage;             // Help: current page.
  PtrList helpPages;           // Help: paginated text (lazy).
  int setControlsSel;          // Set Controls: armed row (virtual key), or -1.

  static Boolean largeViewport;
  static Boolean smoothScroll;
  static int scale;
  static Boolean fullscreen;
  static Boolean reduceDraw;
  static Boolean useBuffer;
};

#endif
