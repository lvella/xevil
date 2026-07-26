/*
 * XEvil(TM) Copyright (C) 1994,2000 Steve Hardt and Michael Judge
 * http://www.xevil.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.  See gpl.txt.
 */

// "ui.cpp"  SDL port -- one window, one Viewport, real input + widgets.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>

#include "utils.h"
#include "role.h"
#include "l_agreement.h"
#include "dialog.h"
#include "ui.h"
#include "icon_xpm.h"

using namespace std;


// ---- Static option state (set from Game::parse_args via Ui::set_*). ----
Boolean Ui::largeViewport = True;
Boolean Ui::smoothScroll = False;
int Ui::scale = 0;
Boolean Ui::fullscreen = False;
Boolean Ui::reduceDraw = False;
Boolean Ui::useBuffer = True;

char *Ui::keysNames[UI_KEYS_MAX] = {
  "center","right","down_right","down","down_left","left","up_left","up",
  "up_right","weapon_use","weapon_change","weapon_drop","item_use",
  "item_change","item_drop","chat",
};

// ORDER MUST MATCH the enum in viewport.h (menuControls .. stChat).
ViewportCallback Ui::viewportCallbacks[VIEWPORT_CB_NUM] = {
  menu_controls_CB,       // menuControls
  menu_learn_controls_CB, // menuLearnControls
  menu_quit_CB,           // menuQuit
  menu_new_game_CB,       // menuNewGame
  menu_humans_num_CB,     // menuHumansNum
  menu_enemies_num_CB,    // menuEnemiesNum
  menu_enemies_refill_CB, // menuEnemiesRefill
  NULL,                   // menuStyle (just a text label)
  menu_scenarios_CB,      // menuScenarios
  menu_levels_CB,         // menuLevels
  menu_kill_CB,           // menuKill
  menu_duel_CB,           // menuDuel
  menu_extended_CB,       // menuExtended
  menu_training_CB,       // menuTraining
  menu_survival_CB,       // menuSurvival
  menu_bossrush_CB,       // menuBossRush
  menu_quanta_CB,         // menuQuanta
  menu_cooperative_CB,    // menuCooperative
  menu_sound_CB,          // menuSound
  menu_help_CB,           // menuHelp
  status_weapon_CB,       // stWeapon
  status_item_CB,         // stItem
  chat_CB,                // stChat
};


/* ------------------------------------------------------------------ *
 * SDL key -> XEvil virtual key.
 *
 * The bindings are DATA (Ui::keymap), so the in-game "Set Controls" panel can
 * rebind them and persist to ~/.xevil_sdl_keys.  init_keymap_defaults() seeds
 * the classic UIlinux layout (map_render_ui 2.3-2.4): the right-hand player
 * (side UI_KEYS_RIGHT) uses the numeric keypad + arrows + WASD for movement,
 * Insert/Home/PageUp weapon use/change/drop, Delete/End/PageDown item
 * use/change/drop, space chat.  The left-hand player (UI_KEYS_LEFT) uses the
 * l ; / . m,/ k i o p movement cluster with a s d weapons and z x c items.
 * -keys <name> is recorded by Game (keyset_set) but SDL keycodes are
 * platform-independent, so the SDL build uses this table rather than the
 * X-server-specific preset tables.  Diagonals are composed by the cmn
 * KeyDispatcher from the cardinal keys.
 * ------------------------------------------------------------------ */

// Path of the SDL keybinding overrides file ($HOME/.xevil_sdl_keys).  Kept
// SEPARATE from ~/.xevilrc so the shared cmn config writer (which rewrites the
// whole rc on quit) never clobbers it, and the X11 build never sees SDL codes.
static Boolean sdl_keys_path(char *out,int outLen) {
  const char *home = getenv("HOME");
  if (!home || !*home) {
    return False;
  }
  const char *sep = (home[strlen(home) - 1] == '/') ? "" : "/";
  snprintf(out,outLen,"%s%s%s",home,sep,".xevil_sdl_keys");
  return True;
}


// 0-3 (main row or keypad) select a difficulty; anything else is DIFF_NONE.
static int difficulty_from_key(SDL_Keycode sym) {
  switch (sym) {
  case SDLK_0: case SDLK_KP_0: return 0;
  case SDLK_1: case SDLK_KP_1: return 1;
  case SDLK_2: case SDLK_KP_2: return 2;
  case SDLK_3: case SDLK_KP_3: return 3;
  default: return DIFF_NONE;
  }
}



Ui::Ui(int *agc,char **agv,WorldP w,LocatorP l,char **d_names,
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
  for (int n = 0; n < UI_VIEWPORTS_MAX; n++) {
    viewports[n] = NULL;
  }
  viewport = NULL;
  viewportsNum = 0;
  localWindows = 1;

  settingsChanges = UInone;
  memset((void *)&settings,0,sizeof(settings));  // POD-ish; only read per mask
  otherInput = False;
  pause = False;
  soundOn = True;
  promptDifficulty = False;
  roleType = rType;
  difficulty = DIFF_NONE;

  overlay = UIoverlayNone;
  overlayPage = 0;
  setControlsSel = -1;

  init_keymap_defaults();
  load_keymap();               // apply any persisted "Set Controls" rebinds

  for (int n = 0; n < Xvars::DISPLAYS_MAX; n++) {
    keysetSet[n] = False;
  }

  // Open display 0 with a provisional size (resized for the license dialog and
  // then for the game window below).  Must precede the modal license dialog.
  Size provisional;
  provisional.set(640,480);
  if (!xvars.open_display("XEvil 2.5",provisional)) {
    cerr << "Failed to open SDL display." << endl;
    exit(1);
  }

  // First-run license agreement (may exit(1) on Reject).
  run_license_agreement();

  // Resolve the integer display scale (same logic as the X11 Ui): -scale wins
  // over the large/small toggle; scale 0 (unset) falls back to the toggle.
  Boolean scaleWasUnset = (scale < 1);
  if (scale < 1) {
    scale = (largeViewport ? 2 : 1);
  } else {
    largeViewport = (scale >= 2);
  }
  // -fullscreen with no explicit -scale: auto-pick the largest fitting scale.
  if (fullscreen && scaleWasUnset) {
    scale = pick_fullscreen_scale();
  }
  xvars.stretch = scale;

  Viewport::init_viewport_info(largeViewport,smoothScroll);

  // Window chrome (task 5 polish): title + taskbar/window-manager icon.
  xvars.set_window_title(0,"XEvil 2.5");
  xvars.set_window_icon(0,(char **)sdl_icon_xpm);

  // Local two-player: open the SECOND window NOW, before any art loads, so the
  // shared init_x loops (which run once, over xvars.dpyMax) load a copy of every
  // texture onto BOTH renderers.  The viewport OBJECT for it is created later by
  // Game::humans_reset via add_viewport(); until then the window renders blank.
  localWindows = resolve_local_windows();
  if (localWindows >= 2) {
    Size prov;
    prov.set(640,480);
    int d = xvars.add_display("XEvil 2.5 - Player 2",prov);
    if (d >= 0) {
      xvars.set_window_icon(d,(char **)sdl_icon_xpm);
    } else {
      localWindows = 1;   // couldn't open a second window; fall back to one
    }
    xvars.set_active_display(0);
  }

  add_viewport();
  if (viewport && fullscreen) {
    // Honor an initial -fullscreen / config fullscreen=1 (window 0 only).
    xvars.set_fullscreen(0,True,viewport->get_window_size());
  }
}


// How many local SDL windows to open: 2 for a stand-alone "-humans 2+", else 1
// (only two input sides exist, and network roles have one local player).
int Ui::resolve_local_windows() {
  if (roleType != R_STAND_ALONE) {
    return 1;
  }
  for (int n = 1; n + 1 < argc; n++) {
    if (!strcmp(argv[n],"-humans") && atoi(argv[n + 1]) >= 2) {
      return 2;
    }
  }
  return 1;
}



void Ui::run_license_agreement() {
  // -accept_agreement, or acceptance recorded in ~/.xevilrc on a prior run,
  // both bypass the dialog.  (Windows/SDL users double-click an exe and pass no
  // flags, so remembering the acceptance is what makes this usable for them.)
  if (LAgreement::is_comm_line_accepted() || sdl_agreement_marker_present()) {
    return;
  }
  Boolean drawBackground = !reduceDraw;
  LicenseResult lr =
    sdl_run_license_dialog(xvars,0,largeViewport,smoothScroll,drawBackground);
  if (!lr.accepted) {
    xvars.close_display();
    exit(1);
  }
  largeViewport = lr.largeViewport;
  smoothScroll = lr.smoothScroll;
  reduceDraw = !lr.drawBackground;
  sdl_agreement_write_marker();
}



int Ui::pick_fullscreen_scale() {
  SDL_DisplayMode dm;
  int sw = 0, sh = 0;
  if (SDL_GetDesktopDisplayMode(0,&dm) == 0) {
    sw = dm.w;
    sh = dm.h;
  }
  if (sw <= 0) {
    return 2;
  }
  // Approximate the SDL game window (arena scale*480 x scale*256 plus ~8 rows
  // of menu/HUD chrome); the fullscreen logical letterbox absorbs any slack.
  for (int s = 4; s >= 1; s--) {
    int fontH = (s >= 3) ? 24 : 13;
    int winW = s * 480 + 2 * 5;
    int winH = s * 256 + 8 * (fontH + 6);
    if (winW <= sw && winH <= sh) {
      return s;
    }
  }
  return 1;
}



void Ui::toggle_fullscreen(int dpyNum) {
  Viewport *vp = (dpyNum >= 0 && dpyNum < viewportsNum) ? viewports[dpyNum]
                                                        : NULL;
  if (!vp) {
    vp = viewport;
  }
  if (!vp) {
    return;
  }
  Boolean now = !xvars.get_window_fullscreen(dpyNum);
  xvars.set_fullscreen(dpyNum,now,vp->get_window_size());
  if (dpyNum == 0) {
    fullscreen = now;   // only window 0's state is the persisted preference
  }
  set_redraw_arena();
}


int Ui::viewport_for_window(Uint32 windowID) {
  for (int i = 0; i < viewportsNum && i < localWindows; i++) {
    if (xvars.get_window_id(i) == windowID) {
      return i;
    }
  }
  return 0;
}



Ui::~Ui() {
  for (int n = 0; n < helpPages.length(); n++) {
    delete (Page *)helpPages.get(n);
  }
  for (int n = 0; n < viewportsNum; n++) {
    delete viewports[n];
  }
  xvars.close_display();
}



int Ui::add_viewport() {
  if (viewportsNum >= UI_VIEWPORTS_MAX) {
    return viewportsNum - 1;
  }
  int idx = viewportsNum;
  // Viewport idx renders on its own window (display idx) when one exists,
  // otherwise it shares window 0 (spectator/no input on the SDL port).
  int dpy = (idx < localWindows) ? idx : 0;

  xvars.set_active_display(dpy);
  Viewport *vp = new Viewport(xvars,dpy,world,locator,scale,
                              viewportCallbacks,(void *)this,
                              roleType,difficultyLevels);
  assert(vp);
  viewports[idx] = vp;
  if (idx == 0) {
    viewport = vp;
  }
  viewportsNum = idx + 1;

  // Seed a secondary viewport's menu bar with the current settings (viewport 0
  // is seeded by Game's set_* calls right after construction).
  if (idx > 0) {
    vp->set_menu_humans_num(settings.humansNum);
    vp->set_menu_enemies_num(settings.enemiesNum);
    vp->set_menu_quanta(settings.quanta);
    vp->set_enemies_refill(settings.enemiesRefill);
    vp->set_cooperative(settings.cooperative);
    vp->set_menu_sound(settings.sound);
    vp->set_style_and_role_type(settings.style,roleType);
  }

  // Size this viewport's window and give it a per-player title.
  if (idx < localWindows) {
    xvars.set_window_title(dpy,idx == 0 ? "XEvil 2.5" : "XEvil 2.5 - Player 2");
    xvars.resize_window(dpy,vp->get_window_size());
    if (idx == 0) {
      xvars.center_window(dpy);
    } else {
      // Offset the second window so both are visible on one screen.
      SDL_DisplayMode dm;
      Size sz = vp->get_window_size();
      if (SDL_GetDesktopDisplayMode(0,&dm) == 0) {
        int x = dm.w - sz.width;   if (x < 0) x = 0;
        int y = dm.h - sz.height;  if (y < 0) y = 0;
        xvars.position_window(dpy,x,y);
      }
    }
  }
  return idx;
}



void Ui::del_viewport() {
  // Never remove the primary viewport in the single-window SDL port.
}



UImask Ui::get_settings(UIsettings &s) {
  s = settings;
  UImask tmp = settingsChanges;
  settingsChanges = UInone;
  return tmp;
}



// The menu-bar value setters mirror to EVERY viewport so a two-player game's
// second window shows the same populated menu bar (each viewport owns its own
// widgets).
void Ui::set_humans_num(int v) {
  settings.humansNum = v;
  for (int i = 0; i < viewportsNum; i++)
    if (viewports[i]) viewports[i]->set_menu_humans_num(v);
}
void Ui::set_enemies_num(int v) {
  settings.enemiesNum = v;
  for (int i = 0; i < viewportsNum; i++)
    if (viewports[i]) viewports[i]->set_menu_enemies_num(v);
}
void Ui::set_enemies_refill(Boolean v) {
  settings.enemiesRefill = v;
  for (int i = 0; i < viewportsNum; i++)
    if (viewports[i]) viewports[i]->set_enemies_refill(v);
}
void Ui::set_style(GameStyleType s) {
  settings.style = s;
  for (int i = 0; i < viewportsNum; i++)
    if (viewports[i]) viewports[i]->set_style_and_role_type(s,roleType);
}
void Ui::set_quanta(Quanta q) {
  settings.quanta = q;
  for (int i = 0; i < viewportsNum; i++)
    if (viewports[i]) viewports[i]->set_menu_quanta(q);
}
void Ui::set_cooperative(Boolean v) {
  settings.cooperative = v;
  for (int i = 0; i < viewportsNum; i++)
    if (viewports[i]) viewports[i]->set_cooperative(v);
}
void Ui::set_sound_onoff(Boolean val) {
  soundOn = val;
  settings.sound = val;
  for (int i = 0; i < viewportsNum; i++)
    if (viewports[i]) viewports[i]->set_menu_sound(val);
}



void Ui::set_humans_playing(int v) {
  for (int i = 0; i < viewportsNum; i++)
    if (viewports[i]) viewports[i]->set_humans_playing(v);
}
void Ui::set_enemies_playing(int v) {
  for (int i = 0; i < viewportsNum; i++)
    if (viewports[i]) viewports[i]->set_enemies_playing(v);
}
void Ui::set_level(const char *msg) {
  for (int i = 0; i < viewportsNum; i++)
    if (viewports[i]) viewports[i]->set_level(msg);
}



void Ui::set_input(int vNum,UIinput input) {
  if (vNum >= 0 && vNum < viewportsNum && viewports[vNum]) {
    viewports[vNum]->set_input(input);
    // Two-player: once the left player exists, drop the WASD movement
    // alternates from the right set so they don't also fire the left player's
    // a/s/d weapons.
    if (input == UI_KEYS_LEFT) {
      strip_right_letters();
    }
  }
}



void Ui::set_keyset(int dpyNum,UIkeyset) {
  // SDL uses a platform-independent, rebindable key table (see key_to_virtual /
  // the keymap); just record that a keyset is present so the per-frame X11-path
  // assert is satisfied and Game skips X-resource remapping.
  keysetSet[dpyNum] = True;
}



void Ui::set_keyset(int dpyNum,UIkeyset,KeySym[][2],KeySym[][2]) {
  keysetSet[dpyNum] = True;
}



void Ui::set_pause(Boolean val) {
  pause = val;
  for (int i = 0; i < viewportsNum; i++)
    if (viewports[i]) viewports[i]->set_pause_message(val);
}



void Ui::set_prompt_difficulty() {
  difficulty = DIFF_NONE;   // unspecified until the user enters it.
  promptDifficulty = True;
  for (int i = 0; i < viewportsNum; i++)
    if (viewports[i]) viewports[i]->set_prompt_difficulty(True);
}



void Ui::unset_prompt_difficulty() {
  promptDifficulty = False;
  for (int i = 0; i < viewportsNum; i++)
    if (viewports[i]) viewports[i]->set_prompt_difficulty(False);
}



void Ui::register_intel(int n,IntelP intel) {
  if (n >= 0 && n < viewportsNum && viewports[n]) {
    // The humanColorNum (n) tints this window's HUD to the player's color.
    viewports[n]->register_intel(n,intel);
  }
}



void Ui::demo_reset() {
  for (int i = 0; i < viewportsNum; i++)
    if (viewports[i]) viewports[i]->reset();
}



void Ui::reset() {
  demo_reset();
  // Allow the viewports to accept user input now (real game, not the demo).
  Viewport::accept_input();
}



void Ui::set_redraw_arena() {
  for (int i = 0; i < viewportsNum; i++)
    if (viewports[i]) viewports[i]->set_redraw_arena();
}



void Ui::key_event(SDL_Keycode sym,Boolean down) {
  // Broadcast to every viewport; each keeps the keys its own input side binds.
  // This is what lets one keyboard drive both local players regardless of which
  // window is focused (map_render_ui 2.2, 7).
  for (int i = 0; i < viewportsNum; i++) {
    Viewport *vp = viewports[i];
    if (!vp) {
      continue;
    }
    UIinput input = vp->get_input();
    if (input == UI_INPUT_NONE) {
      continue;
    }
    int key = key_to_virtual(sym,input);
    if (key >= 0) {
      vp->receive_key(key,down);
    }
  }
}


/* ------------------------------------------------------------------ *
 * Key bindings (data-driven; Set Controls edits + persists them).
 * ------------------------------------------------------------------ */

void Ui::init_keymap_defaults() {
  for (int s = 0; s < UI_INPUT_SIDES; s++) {
    for (int k = 0; k < UI_KEYS_MAX; k++) {
      for (int a = 0; a < UI_KEY_ALTS; a++) {
        keymap[s][k][a] = 0;
      }
    }
  }
  // Order follows the ITcommand enum (coord.h): IT_CENTER .. IT_CHAT.
  SDL_Keycode R[UI_KEYS_MAX][UI_KEY_ALTS] = {
    {SDLK_KP_5, 0, 0},                       // IT_CENTER
    {SDLK_KP_6, SDLK_RIGHT, SDLK_d},         // IT_R
    {SDLK_KP_3, 0, 0},                       // IT_DN_R
    {SDLK_KP_2, SDLK_DOWN, SDLK_s},          // IT_DN
    {SDLK_KP_1, 0, 0},                       // IT_DN_L
    {SDLK_KP_4, SDLK_LEFT, SDLK_a},          // IT_L
    {SDLK_KP_7, 0, 0},                       // IT_UP_L
    {SDLK_KP_8, SDLK_UP, SDLK_w},            // IT_UP
    {SDLK_KP_9, 0, 0},                       // IT_UP_R
    {SDLK_INSERT, 0, 0},                     // IT_WEAPON_CENTER
    {SDLK_HOME, 0, 0},                       // IT_WEAPON_CHANGE
    {SDLK_PAGEUP, 0, 0},                     // IT_WEAPON_DROP
    {SDLK_DELETE, 0, 0},                     // IT_ITEM_USE
    {SDLK_END, 0, 0},                        // IT_ITEM_CHANGE
    {SDLK_PAGEDOWN, 0, 0},                   // IT_ITEM_DROP
    {SDLK_SPACE, 0, 0},                      // IT_CHAT
  };
  SDL_Keycode L[UI_KEYS_MAX][UI_KEY_ALTS] = {
    {SDLK_l, 0, 0},                          // IT_CENTER
    {SDLK_SEMICOLON, 0, 0},                  // IT_R
    {SDLK_SLASH, 0, 0},                      // IT_DN_R
    {SDLK_PERIOD, 0, 0},                     // IT_DN
    {SDLK_m, SDLK_COMMA, 0},                 // IT_DN_L
    {SDLK_k, 0, 0},                          // IT_L
    {SDLK_i, 0, 0},                          // IT_UP_L
    {SDLK_o, 0, 0},                          // IT_UP
    {SDLK_p, SDLK_LEFTBRACKET, 0},           // IT_UP_R
    {SDLK_a, 0, 0},                          // IT_WEAPON_CENTER
    {SDLK_s, 0, 0},                          // IT_WEAPON_CHANGE
    {SDLK_d, 0, 0},                          // IT_WEAPON_DROP
    {SDLK_z, 0, 0},                          // IT_ITEM_USE
    {SDLK_x, 0, 0},                          // IT_ITEM_CHANGE
    {SDLK_c, 0, 0},                          // IT_ITEM_DROP
    {0, 0, 0},                               // IT_CHAT (unused on the left set)
  };
  memcpy(keymap[UI_KEYS_RIGHT],R,sizeof(R));
  memcpy(keymap[UI_KEYS_LEFT], L,sizeof(L));
}


int Ui::key_to_virtual(SDL_Keycode sym,UIinput input) const {
  if (sym == 0 || (input != UI_KEYS_RIGHT && input != UI_KEYS_LEFT)) {
    return -1;
  }
  for (int k = 0; k < UI_KEYS_MAX; k++) {
    for (int a = 0; a < UI_KEY_ALTS; a++) {
      if (keymap[input][k][a] == sym) {
        return k;
      }
    }
  }
  return -1;
}


void Ui::rebind_key(int side,int virtualKey,SDL_Keycode sym) {
  if (side < 0 || side >= UI_INPUT_SIDES ||
      virtualKey < 0 || virtualKey >= UI_KEYS_MAX) {
    return;
  }
  // Remove sym anywhere else on this side (a key drives one command), then make
  // it the SOLE binding for virtualKey (so the panel and the persisted file
  // agree: one command, one key).
  for (int k = 0; k < UI_KEYS_MAX; k++) {
    for (int a = 0; a < UI_KEY_ALTS; a++) {
      if (keymap[side][k][a] == sym) {
        keymap[side][k][a] = 0;
      }
    }
  }
  keymap[side][virtualKey][0] = sym;
  keymap[side][virtualKey][1] = 0;
  keymap[side][virtualKey][2] = 0;
  save_keymap();
}


void Ui::strip_right_letters() {
  // Two-player: drop the WASD movement alternates from the RIGHT set so a left
  // player's a/s/d weapon keys don't also drive the right player (the clash the
  // stage-1 note warned about).  Keypad + arrows remain for player 1.
  for (int k = 0; k < UI_KEYS_MAX; k++) {
    for (int a = 0; a < UI_KEY_ALTS; a++) {
      SDL_Keycode c = keymap[UI_KEYS_RIGHT][k][a];
      if (c == SDLK_w || c == SDLK_a || c == SDLK_s || c == SDLK_d) {
        keymap[UI_KEYS_RIGHT][k][a] = 0;
      }
    }
  }
}


void Ui::load_keymap() {
  char path[512];
  if (!sdl_keys_path(path,sizeof(path))) {
    return;
  }
  FILE *fp = fopen(path,"r");
  if (!fp) {
    return;
  }
  // Lines: "right <virtualKey> <keycode>" (only the RIGHT set is user-editable).
  char line[128];
  while (fgets(line,sizeof(line),fp)) {
    char side[16];
    int vk = 0;
    long code = 0;
    if (sscanf(line,"%15s %d %ld",side,&vk,&code) == 3 &&
        !strcmp(side,"right") && vk >= 0 && vk < UI_KEYS_MAX) {
      keymap[UI_KEYS_RIGHT][vk][0] = (SDL_Keycode)code;
      keymap[UI_KEYS_RIGHT][vk][1] = 0;   // an explicit rebind clears alternates
      keymap[UI_KEYS_RIGHT][vk][2] = 0;
    }
  }
  fclose(fp);
}


void Ui::save_keymap() {
  char path[512];
  if (!sdl_keys_path(path,sizeof(path))) {
    return;
  }
  FILE *fp = fopen(path,"w");
  if (!fp) {
    return;
  }
  fprintf(fp,"# XEvil SDL key bindings (player 1 / right set).  "
             "virtualKey keycode.\n");
  for (int k = 0; k < UI_KEYS_MAX; k++) {
    fprintf(fp,"right %d %ld\n",k,(long)keymap[UI_KEYS_RIGHT][k][0]);
  }
  fclose(fp);
}


/* ------------------------------------------------------------------ *
 * Overlays: Help / Show Controls / Set Controls (drawn over window 0).
 * ------------------------------------------------------------------ */

// Same content as the X11 Ui::helpMessage (x11/ui.cpp).
static const char *ui_help_message =
"XEvil 2.5 -- native SDL build.\n"
"\n"
"For full instructions, including NETWORK PLAY, see "
"http://www.xevil.com/docs/instructions.html\n"
"\n"
"Run 'xevil-sdl -help' for usage and basic network-play options.\n"
"\n"
"MENU BAR: New Game restarts; type into Humans:/Enemies:/Speed(ms): and press "
"Return; the Game style toggles (Levels, Scenarios, Kill Kill Kill, Duel, ...) "
"choose the mode; Regen Enemies, Cooperative and Sound are toggles.\n"
"\n"
"Use the \"Set Controls\" and \"Show Controls\" buttons to view and rebind the "
"keyboard.  Player 1 defaults to the numeric keypad (and the arrow keys) for "
"movement, Insert/Home/PageUp for weapon use/change/drop, Delete/End/PageDown "
"for item use/change/drop, and space to chat.\n"
"\n"
"Two players on one machine: run with -humans 2.  A second window opens and the "
"left-hand a-s-d cluster (l ; / . k i o p move, a s d weapons, z x c items) "
"drives player 2.\n"
"\n"
"Press F1 to pause the game; press any key to resume.  Press F11 to toggle "
"fullscreen.  The Sound toggle turns sound on/off.  Settings are remembered in "
"~/.xevilrc between sessions.\n"
"\n"
"In this Help panel: PageDown/Space and PageUp/Backspace turn the page; Esc "
"closes.\n"
"\n"
"XEvil(TM) Copyright(C) 1994,2000  Steve Hardt and Michael Judge\n"
"http://www.xevil.com   satan@xevil.com";


void Ui::build_help_pages() {
  for (int n = 0; n < helpPages.length(); n++) {
    delete (Page *)helpPages.get(n);
  }
  while (helpPages.length() > 0) {
    helpPages.del(0);
  }
  if (!viewport) {
    return;
  }

  int dpy = 0;
  const BitmapFont *f = xvars.font[dpy];
  int s = (scale >= 1) ? scale : 1;
  Size ws = viewport->get_window_size();
  int pad = 8;
  int cols = (ws.width - 2 * pad) / (f->cellW * s);
  int rows = (ws.height - 6 * f->cellH * s) / (f->cellH * s);
  if (cols < 20) cols = 20;
  if (rows < 6)  rows = 6;

  Line::set_text_columns(cols);
  Page::set_text_rows(rows);
  const char *p = ui_help_message;
  while (*p) {
    helpPages.add(new Page(&p,p));
  }
  if (helpPages.length() == 0) {
    const char *empty = "";
    helpPages.add(new Page(&empty,empty));
  }
}


void Ui::sync_overlay_toggles() {
  if (!viewport) {
    return;
  }
  viewport->set_menu_help(overlay == UIoverlayHelp);
  viewport->set_menu_controls(overlay == UIoverlayShowControls);
  viewport->set_menu_learn_controls(overlay == UIoverlaySetControls);
}


void Ui::open_overlay(UIoverlayMode mode) {
  overlay = mode;
  overlayPage = 0;
  setControlsSel = -1;
  if (mode == UIoverlayHelp) {
    build_help_pages();
  }
  sync_overlay_toggles();
}


void Ui::close_overlay() {
  overlay = UIoverlayNone;
  setControlsSel = -1;
  sync_overlay_toggles();
  set_redraw_arena();
}


// Human-readable names of the keys bound to a virtual key, e.g. "Keypad 8, Up".
static void binding_names(const SDL_Keycode alts[UI_KEY_ALTS],char *out,int n) {
  out[0] = '\0';
  int written = 0;
  for (int a = 0; a < UI_KEY_ALTS; a++) {
    if (alts[a] == 0) {
      continue;
    }
    const char *name = SDL_GetKeyName(alts[a]);
    if (!name || !name[0]) {
      name = "?";
    }
    int add = snprintf(out + written,n - written,"%s%s",
                       written ? ", " : "",name);
    if (add > 0) {
      written += add;
      if (written >= n - 1) break;
    }
  }
  if (written == 0) {
    snprintf(out,n,"(unbound)");
  }
}


// Row layout for the Show/Set Controls list (shared by draw + hit-test).
void Ui::controls_row_layout(int &listTop,int &rowH) {
  const BitmapFont *f = xvars.font[0];
  int s = (scale >= 1) ? scale : 1;
  rowH = f->cellH * s;
  listTop = 8 + 2 * rowH;
}


void Ui::draw_overlay() {
  if (!viewport || overlay == UIoverlayNone) {
    return;
  }
  int dpy = 0;
  xvars.set_active_display(dpy);
  SDL_Renderer *ren = xvars.renderer;
  const BitmapFont *f = xvars.font[dpy];
  int s = (scale >= 1) ? scale : 1;
  int rowH = f->cellH * s;
  int pad = 8;
  Size ws = viewport->get_window_size();
  Pixel bg = xvars.windowBg[dpy];
  Pixel fg = xvars.black[dpy];
  Pixel hi = xvars.humanColors[dpy][0];   // highlight = player-1 color
  Pixel red = xvars.red[dpy];

  xvars.set_target(0);
  SDL_SetRenderDrawColor(ren,Pixel_r(bg),Pixel_g(bg),Pixel_b(bg),255);
  SDL_Rect full = {0,0,ws.width,ws.height};
  SDL_RenderFillRect(ren,&full);

  char buf[PANEL_STRING_LENGTH];
  int y = pad;

  if (overlay == UIoverlayHelp) {
    if (helpPages.length() == 0) build_help_pages();
    int pageMax = helpPages.length();
    if (overlayPage >= pageMax) overlayPage = pageMax - 1;
    snprintf(buf,sizeof(buf),"XEvil Help   (Page %d/%d)",overlayPage + 1,pageMax);
    font::draw_scaled(ren,*f,pad,y + f->ascent * s,buf,
                      Pixel_r(fg),Pixel_g(fg),Pixel_b(fg),255,s);
    y += 2 * rowH;
    Page *page = (Page *)helpPages.get(overlayPage);
    const PtrList &lines = page->get_lines();
    for (int n = 0; n < lines.length(); n++) {
      char *text = ((Line *)lines.get(n))->alloc_text();
      if (text && text[0]) {
        font::draw_scaled(ren,*f,pad,y + f->ascent * s + rowH * n,text,
                          Pixel_r(fg),Pixel_g(fg),Pixel_b(fg),255,s);
      }
      delete [] text;
    }
    snprintf(buf,sizeof(buf),
             "PageDown/Space, PageUp/Backspace to page   -   Esc closes");
    font::draw_scaled(ren,*f,pad,ws.height - rowH - pad + f->ascent * s,buf,
                      Pixel_r(red),Pixel_g(red),Pixel_b(red),255,s);
    return;
  }

  // Show / Set Controls.
  Boolean setMode = (overlay == UIoverlaySetControls);
  snprintf(buf,sizeof(buf),"%s (Player 1)",
           setMode ? "Set Controls" : "Show Controls");
  font::draw_scaled(ren,*f,pad,y + f->ascent * s,buf,
                    Pixel_r(fg),Pixel_g(fg),Pixel_b(fg),255,s);

  int listTop, rH;
  controls_row_layout(listTop,rH);
  for (int k = 0; k < UI_KEYS_MAX; k++) {
    int ry = listTop + k * rH;
    Pixel rowColor = fg;
    if (setMode && k == setControlsSel) {
      // Armed row: fill with the highlight color, draw text in bg for contrast.
      SDL_SetRenderDrawColor(ren,Pixel_r(hi),Pixel_g(hi),Pixel_b(hi),255);
      SDL_Rect rr = {pad / 2,ry,ws.width - pad,rH};
      SDL_RenderFillRect(ren,&rr);
      rowColor = bg;
    }
    char names[128];
    binding_names(keymap[UI_KEYS_RIGHT][k],names,sizeof(names));
    snprintf(buf,sizeof(buf),"%-14s  %s",keysNames[k],names);
    font::draw_scaled(ren,*f,pad,ry + f->ascent * s,buf,
                      Pixel_r(rowColor),Pixel_g(rowColor),Pixel_b(rowColor),
                      255,s);
  }

  if (setMode) {
    if (setControlsSel >= 0) {
      snprintf(buf,sizeof(buf),
               "Press a key to bind \"%s\"   -   Esc cancels",
               keysNames[setControlsSel]);
    } else {
      snprintf(buf,sizeof(buf),
               "Click a row, then press a key.  Saved to ~/.xevil_sdl_keys.  "
               "Esc closes.");
    }
  } else {
    snprintf(buf,sizeof(buf),"Esc closes.");
  }
  font::draw_scaled(ren,*f,pad,ws.height - rowH - pad + f->ascent * s,buf,
                    Pixel_r(red),Pixel_g(red),Pixel_b(red),255,s);
}


Boolean Ui::overlay_key(const SDL_Keysym &ks) {
  if (overlay == UIoverlayNone) {
    return False;
  }
  SDL_Keycode sym = ks.sym;

  if (overlay == UIoverlayHelp) {
    switch (sym) {
    case SDLK_ESCAPE: close_overlay(); break;
    case SDLK_PAGEDOWN: case SDLK_SPACE: case SDLK_RIGHT: case SDLK_RETURN:
      if (overlayPage < helpPages.length() - 1) overlayPage++;
      break;
    case SDLK_PAGEUP: case SDLK_BACKSPACE: case SDLK_LEFT:
      if (overlayPage > 0) overlayPage--;
      break;
    default: break;
    }
    return True;
  }

  if (overlay == UIoverlaySetControls) {
    if (sym == SDLK_ESCAPE) {
      if (setControlsSel >= 0) {
        setControlsSel = -1;    // cancel the armed rebind, stay in the panel
      } else {
        close_overlay();
      }
      return True;
    }
    if (setControlsSel >= 0) {
      // Bind the pressed key to the armed virtual key (player-1 / right set).
      rebind_key(UI_KEYS_RIGHT,setControlsSel,sym);
      setControlsSel = -1;
    }
    return True;
  }

  // Show Controls: Esc closes; everything else swallowed.
  if (sym == SDLK_ESCAPE) {
    close_overlay();
  }
  return True;
}


Boolean Ui::overlay_mouse(int button,int x,int y) {
  if (overlay == UIoverlayNone) {
    return False;
  }
  if (overlay == UIoverlaySetControls && button == 1) {
    int listTop, rH;
    controls_row_layout(listTop,rH);
    if (y >= listTop) {
      int row = (y - listTop) / rH;
      if (row >= 0 && row < UI_KEYS_MAX) {
        setControlsSel = row;    // arm this row for the next key press
      }
    }
  }
  return True;
}



void Ui::process_event(int,SDL_Event *event) {
  switch (event->type) {
  case SDL_QUIT:
    settingsChanges |= UIquit;
    break;

  case SDL_WINDOWEVENT:
    if (event->window.event == SDL_WINDOWEVENT_CLOSE) {
      settingsChanges |= UIquit;
    }
    break;

  case SDL_MOUSEBUTTONDOWN: {
    otherInput = True;
    // Overlays (Help / Show / Set Controls) grab the mouse while up.
    if (overlay != UIoverlayNone) {
      overlay_mouse(event->button.button,event->button.x,event->button.y);
      break;
    }
    if (promptDifficulty) {
      change_difficulty(DIFF_NORMAL);
      break;
    }
    if (pause) {
      settingsChanges |= UIpause;
      settings.pause = False;
      break;
    }
    // Route the click to the viewport whose window it landed in (window 0 owns
    // the menu bar; each window owns its own player's HUD buttons).
    int vi = viewport_for_window(event->button.windowID);
    if (viewports[vi]) {
      viewports[vi]->handle_mouse(event->button.button,
                                  event->button.x,event->button.y);
    }
    break;
  }

  case SDL_KEYDOWN: {
    otherInput = True;
    SDL_Keycode sym = event->key.keysym.sym;

    // F11 toggles desktop-fullscreen at any time (safe under SDL, unlike raw
    // X11); route to the window the key came from.
    if (sym == SDLK_F11) {
      toggle_fullscreen(viewport_for_window(event->key.windowID));
      break;
    }

    // Overlays (Help / Show / Set Controls) grab all keys while up.
    if (overlay != UIoverlayNone) {
      overlay_key(event->key.keysym);
      break;
    }

    if (promptDifficulty) {
      int d = difficulty_from_key(sym);
      if (d != DIFF_NONE) {
        change_difficulty(d);
      } else if (sym == SDLK_SPACE) {
        change_difficulty(DIFF_NORMAL);
      }
      break;
    }

    if (pause) {
      settingsChanges |= UIpause;
      settings.pause = False;
      break;
    }

    // Text widgets (chat bar / focused WritePanel) get first crack.
    if (viewport && viewport->handle_text_key(event->key.keysym)) {
      break;
    }

    // Undocumented pause key.
    if (sym == SDLK_F1) {
      settingsChanges |= UIpause;
      settings.pause = True;
      break;
    }

    // ESC quits (only when no text widget swallowed it above).
    if (sym == SDLK_ESCAPE) {
      settingsChanges |= UIquit;
      break;
    }

    key_event(sym,True);
    break;
  }

  case SDL_KEYUP: {
    if (promptDifficulty) {
      break;
    }
    // Key-up never feeds a text widget; just clear the command bitmap.
    key_event(event->key.keysym.sym,False);
    break;
  }

  default:
    break;
  }
}



// The viewport that renders to window `d` (NULL when the window exists but its
// viewport has not been created yet, e.g. player 2's window during the demo).
Viewport *Ui::viewport_on_display(int d) {
  if (d >= 0 && d < viewportsNum && d < localWindows) {
    return viewports[d];
  }
  return NULL;
}


void Ui::pre_clock() {
  otherInput = False;

  // Route arena messages (level titles, kill notices) to ALL viewports so both
  // local players see them.
  if (!pause) {
    char *arenaMsg;
    do {
      IntelId msgTarget;
      Quanta time;
      Boolean propagate;
      locator->arena_message_deq(&arenaMsg,msgTarget,time,propagate);
      if (arenaMsg) {
        for (int i = 0; i < viewportsNum; i++) {
          if (viewports[i]) viewports[i]->set_arena_message(arenaMsg,time);
        }
        delete arenaMsg;
      }
    } while (arenaMsg);
  }

  // Render every window in turn (each on its own renderer / texture set).
  for (int d = 0; d < xvars.dpyMax; d++) {
    xvars.set_active_display(d);
    Pixel bg = xvars.windowBg[d];
    SDL_SetRenderDrawColor(xvars.renderer,Pixel_r(bg),Pixel_g(bg),Pixel_b(bg),
                           255);
    SDL_RenderClear(xvars.renderer);

    Viewport *vp = viewport_on_display(d);
    if (vp) {
      vp->pre_clock();
    }
    // Overlays (Help / Show / Set Controls) paint over the primary window only.
    if (d == 0 && overlay != UIoverlayNone) {
      draw_overlay();
    }
    SDL_RenderPresent(xvars.renderer);
  }

  if (!pause) {
    // One-line message bar -> all viewports.
    char *msg;
    if ((msg = locator->message_deq())) {
      for (int i = 0; i < viewportsNum; i++) {
        if (viewports[i]) viewports[i]->set_message(msg);
      }
      delete msg;
    }
  }
}



void Ui::post_clock() {
  // Turn each viewport's key bitmap into (at most) one ITcommand this turn,
  // exactly as x11/viewport.cpp does.
  for (int i = 0; i < viewportsNum; i++) {
    if (viewports[i]) {
      viewports[i]->post_clock();
    }
  }
}



/* ------------------------------------------------------------------ *
 * Menu / HUD callbacks.  These set a UI* mask bit + a field in `settings`;
 * Game::ui_settings_check polls settings_changed() each turn (map 4.1).
 * ------------------------------------------------------------------ */

void Ui::menu_quit_CB(void *,Viewport *,void *closure) {
  ((UiP)closure)->settingsChanges |= UIquit;
}

void Ui::menu_new_game_CB(void *,Viewport *,void *closure) {
  ((UiP)closure)->settingsChanges |= UInewGame;
}

void Ui::menu_humans_num_CB(void *value,Viewport *,void *closure) {
  UiP ui = (UiP)closure;
  ui->settingsChanges |= UIhumansNum;
  ui->settings.humansNum = atoi((const char *)value);
}

void Ui::menu_enemies_num_CB(void *value,Viewport *,void *closure) {
  UiP ui = (UiP)closure;
  ui->settingsChanges |= UIenemiesNum;
  ui->settings.enemiesNum = atoi((const char *)value);
}

void Ui::menu_enemies_refill_CB(void *value,Viewport *,void *closure) {
  UiP ui = (UiP)closure;
  ui->settingsChanges |= UIenemiesRefill;
  ui->settings.enemiesRefill = (Boolean)(intptr_t)value;
}

void Ui::menu_controls_CB(void *value,Viewport *,void *closure) {
  UiP ui = (UiP)closure;
  if ((Boolean)(intptr_t)value) {
    ui->open_overlay(UIoverlayShowControls);
  } else if (ui->overlay == UIoverlayShowControls) {
    ui->close_overlay();
  }
}

void Ui::menu_learn_controls_CB(void *value,Viewport *,void *closure) {
  UiP ui = (UiP)closure;
  if ((Boolean)(intptr_t)value) {
    ui->open_overlay(UIoverlaySetControls);
  } else if (ui->overlay == UIoverlaySetControls) {
    ui->close_overlay();
  }
}

void Ui::menu_scenarios_CB(void *value,Viewport *,void *closure) {
  if ((Boolean)(intptr_t)value) {
    UiP ui = (UiP)closure;
    ui->settingsChanges |= UIstyle;
    ui->settings.style = SCENARIOS;
  }
}

void Ui::menu_levels_CB(void *value,Viewport *,void *closure) {
  if ((Boolean)(intptr_t)value) {
    UiP ui = (UiP)closure;
    ui->settingsChanges |= UIstyle;
    ui->settings.style = LEVELS;
  }
}

void Ui::menu_kill_CB(void *value,Viewport *,void *closure) {
  if ((Boolean)(intptr_t)value) {
    UiP ui = (UiP)closure;
    ui->settingsChanges |= UIstyle;
    ui->settings.style = KILL;
  }
}

void Ui::menu_duel_CB(void *value,Viewport *,void *closure) {
  if ((Boolean)(intptr_t)value) {
    UiP ui = (UiP)closure;
    ui->settingsChanges |= UIstyle;
    ui->settings.style = DUEL;
  }
}

void Ui::menu_extended_CB(void *value,Viewport *,void *closure) {
  if ((Boolean)(intptr_t)value) {
    UiP ui = (UiP)closure;
    ui->settingsChanges |= UIstyle;
    ui->settings.style = EXTENDED;
  }
}

void Ui::menu_training_CB(void *value,Viewport *,void *closure) {
  if ((Boolean)(intptr_t)value) {
    UiP ui = (UiP)closure;
    ui->settingsChanges |= UIstyle;
    ui->settings.style = TRAINING;
  }
}

void Ui::menu_survival_CB(void *value,Viewport *,void *closure) {
  if ((Boolean)(intptr_t)value) {
    UiP ui = (UiP)closure;
    ui->settingsChanges |= UIstyle;
    ui->settings.style = SURVIVAL;
  }
}

void Ui::menu_bossrush_CB(void *value,Viewport *,void *closure) {
  if ((Boolean)(intptr_t)value) {
    UiP ui = (UiP)closure;
    ui->settingsChanges |= UIstyle;
    ui->settings.style = BOSS_RUSH;
  }
}

void Ui::menu_quanta_CB(void *value,Viewport *,void *closure) {
  UiP ui = (UiP)closure;
  ui->settingsChanges |= UIquanta;
  ui->settings.quanta = atoi((const char *)value);
}

void Ui::menu_cooperative_CB(void *value,Viewport *,void *closure) {
  UiP ui = (UiP)closure;
  ui->settingsChanges |= UIcooperative;
  ui->settings.cooperative = (Boolean)(intptr_t)value;
}

void Ui::menu_sound_CB(void *value,Viewport *,void *closure) {
  UiP ui = (UiP)closure;
  ui->settingsChanges |= UIsound;
  ui->settings.sound = (Boolean)(intptr_t)value;
}

void Ui::menu_help_CB(void *value,Viewport *,void *closure) {
  UiP ui = (UiP)closure;
  if ((Boolean)(intptr_t)value) {
    ui->open_overlay(UIoverlayHelp);
  } else if (ui->overlay == UIoverlayHelp) {
    ui->close_overlay();
  }
}

void Ui::status_weapon_CB(void *value,Viewport *vPort,void *) {
  intptr_t button = (intptr_t)value;
  switch (button) {
  case 1: vPort->dispatch(IT_WEAPON_CENTER,NULL); break;   // fire
  case 2: vPort->dispatch(IT_WEAPON_CHANGE,NULL); break;   // change
  case 3: vPort->dispatch(IT_WEAPON_DROP,NULL);   break;   // drop
  }
}

void Ui::status_item_CB(void *value,Viewport *vPort,void *) {
  intptr_t button = (intptr_t)value;
  switch (button) {
  case 1: vPort->dispatch(IT_ITEM_USE,NULL);    break;
  case 2: vPort->dispatch(IT_ITEM_CHANGE,NULL); break;
  case 3: vPort->dispatch(IT_ITEM_DROP,NULL);   break;
  }
}

void Ui::chat_CB(void *value,Viewport *,void *closure) {
  UiP ui = (UiP)closure;
  const char *message = (const char *)value;

  ui->settingsChanges |= UIchatRequest;
  ui->settings.chatReceiver[0] = '\0';
  strncpy(ui->settings.chatMessage,message,UI_CHAT_MESSAGE_MAX);
  ui->settings.chatMessage[UI_CHAT_MESSAGE_MAX] = '\0';

  // In stand-alone play the role's send_chat_request is a no-op (nobody to
  // send to), so echo the message locally into the bottom message bar -- this
  // is what makes the ChatPanel visibly work in single player.
  if (!Role::uses_chat(ui->roleType) && strlen(message) > 0) {
    char buf[UI_CHAT_MESSAGE_MAX + 16];
    snprintf(buf,sizeof(buf),"You: %s",message);
    ui->locator->message_enq(Utils::strdup(buf));
  }
}
