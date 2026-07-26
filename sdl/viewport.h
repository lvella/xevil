/*
 * XEvil(TM) Copyright (C) 1994,2000 Steve Hardt and Michael Judge
 * http://www.xevil.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.  See gpl.txt.
 */

// "viewport.h"  SDL port -- the render surface + interactive chrome for one
// player.
//
// The arena is drawn via the smooth/full-frame path (world->draw +
// locator->draw_directly + draw_ticks into one back-buffer texture, then one
// RenderCopy to the window).  The menu bar and HUD are now REAL widgets
// (panel.h) that hit-test the mouse events the Ui hands them, and keyboard
// input flows through the platform-independent cmn KeyDispatcher exactly as in
// x11/viewport.cpp: physical key -> KeyState bitmap -> one ITcommand per turn
// -> Human::set_command (Viewport implements IDispatcher).

#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "utils.h"
#include "coord.h"
#include "area.h"
#include "xdata.h"
#include "world.h"
#include "locator.h"
#include "intel.h"
#include "ui_cmn.h"
#include "game_style.h"
#include "panel.h"

#define VW_STATUSES_NUM 8


// Menu / callback enumeration.  Mirrors x11/viewport.h so a single
// Ui::viewportCallbacks[] array lines up 1:1 (menuControls .. stChat).  The
// external use has three callbacks that are not menu-bar items: stWeapon and
// stItem (HUD buttons) and stChat (the message bar).  menuStyle is an internal
// text label with no callback.
enum {
  menuControls,menuLearnControls,   // These two MUST be first.
  menuQuit,menuNewGame,menuHumansNum,menuEnemiesNum,menuEnemiesRefill,
  menuStyle,menuScenarios,menuLevels,menuKill,menuDuel,menuExtended,
  menuTraining,menuSurvival,menuBossRush,menuQuanta,menuCooperative,
  menuSound,menuHelp,
  stWeapon,stItem,
  stChat,
  VIEWPORT_CB_NUM,      // Must be last.
};
// Number of menu-bar panels stored in menus[] (menuControls .. menuHelp).
#define VW_MENUS_NUM (menuHelp + 1)


class Viewport;
// Matches x11/viewport.h: (value, viewport, closure).  closure is the Ui.
typedef void (*ViewportCallback)(void *value,Viewport *,void *closure);


// KeyState: the XEvil virtual-key down bitmap (implements IKeyState so the
// shared cmn KeyDispatcher can read it).
class KeyState : public IKeyState {
public:
  KeyState() {for (int n = 0; n < UI_KEYS_MAX; n++) isDown[n] = False;}
  virtual Boolean key_down(int key,void *) {return isDown[key];}
  void set(int key,Boolean down) {
    if (key >= 0 && key < UI_KEYS_MAX) isDown[key] = down;
  }
private:
  Boolean isDown[UI_KEYS_MAX];
};


class Viewport : public IDispatcher {
public:
  Viewport(Xvars &xvars,int dpyNum,WorldP,LocatorP,int scale,
           ViewportCallback callbacks[VIEWPORT_CB_NUM],void *uiClosure,
           RoleType roleType,
           const DifficultyLevel dLevels[DIFFICULTY_LEVELS_NUM]);
  virtual ~Viewport();

  // ---- Static ViewportInfo factory (cmn consumes it via Ui) ----
  static IViewportInfo *get_info();
  static void init_viewport_info(Boolean largeViewport,Boolean smoothScroll);

  // ---- Window geometry ----
  Size get_window_size() {return windowSize;}

  // ---- Per-frame ----
  void pre_clock();
  /* EFFECTS: update_statuses + follow_intel + draw. */
  void post_clock();
  /* EFFECTS: Run the KeyDispatcher (key bitmap -> one ITcommand). */

  // ---- IDispatcher ----
  virtual void dispatch(ITcommand,void *);

  // ---- Input the Ui routes in ----
  void receive_key(int key,Boolean down);
  /* EFFECTS: Ui maps a physical key to a virtual key and pushes it here. */
  Boolean handle_mouse(int button,int wx,int wy);
  /* EFFECTS: Hit-test the menu/HUD widgets; returns True iff consumed. */
  Boolean handle_text_key(const SDL_Keysym &ks);
  /* EFFECTS: Give a key to the chat bar / focused WritePanel.  Returns True
     iff a text widget consumed it (so it must NOT become a command). */

  static void accept_input() {acceptInput = True;}
  static void no_input() {acceptInput = False;}

  // ---- State the Ui pushes in ----
  void reset();
  void set_redraw_arena() {redrawArena = True;}
  void register_intel(int humanColorNum,IntelP intel);
  IntelP get_intel() {return intel;}
  void set_input(UIinput in) {input = in;}
  UIinput get_input() {return input;}
  void set_level(const char *);
  const char *get_level() {return levelMsg;}
  void set_arena_message(const char *message,Quanta);
  void set_message(const char *);
  void set_humans_playing(int n) {humansPlayingNum = n;}
  void set_enemies_playing(int n) {enemiesPlayingNum = n;}
  void set_pause_message(Boolean on) {pauseMessage = on;}
  void set_prompt_difficulty(Boolean on) {promptDifficulty = on; redrawArena = True;}

  // ---- Menu-bar value setters (Game drives these through the Ui) ----
  void set_style_and_role_type(GameStyleType,RoleType);
  void set_menu_humans_num(int);
  void set_menu_enemies_num(int);
  void set_menu_quanta(Quanta);
  void set_cooperative(Boolean);
  void set_enemies_refill(Boolean);
  void set_menu_sound(Boolean);
  void set_menu_controls(Boolean);
  void set_menu_learn_controls(Boolean);
  void set_menu_help(Boolean);
  GameStyleType get_style_type() {return styleType;}
  RoleType get_role_type() {return roleType;}


private:
  void layout();
  void create_menus();
  void create_statuses();
  void draw();
  void draw_arena();
  void draw_difficulty_prompt();
  void update_statuses();
  void follow_intel();
  Boolean shift_viewport(int cols,int rows);
  void draw_string_center(Drawable dest,const Size &arenaPix,const char *msg,
                          Pixel color);
  TextPanel *find_panel_at(const Pos &at);

  static void panel_callback(TextPanel *,void *value,void *closure);

  Xvars &xvars;
  int dpyNum;
  WorldP world;
  LocatorP locator;
  int scale;

  Area viewportArea;         // world region shown (unstretched world coords)
  IntelP intel;
  int humanColorNum;
  UIinput input;
  GameStyleType styleType;
  RoleType roleType;
  const DifficultyLevel *difficultyLevels;
  Boolean promptDifficulty;

  // Input plumbing (reused cmn dispatcher).
  KeyState keyState;
  KeyDispatcher keyDispatcher;

  // Menu-callback forwarding (mirror of x11 PanelClosure/callbacks/closure).
  ViewportCallback callbacks[VIEWPORT_CB_NUM];
  void *panelClosures[VIEWPORT_CB_NUM];   // opaque PanelClosure* per callback
  void *uiClosure;

  Drawable buffer;           // arena back-buffer (stretched arena pixels)
  Size arenaWorld;           // arena region, world coords
  Size arenaPix;             // arena, window pixels (arenaWorld * scale)
  Pos arenaPos;              // arena top-left in the window

  Size windowSize;

  // Chrome.
  TextPanel *menus[VW_MENUS_NUM];
  TextPanel *statuses[VW_STATUSES_NUM];
  TextPanel *levelPanel;
  ChatPanel *messageBar;
  TextPanel *focusPanel;     // active WritePanel, or NULL

  char *arenaMessage;
  Timer arenaMessageTimer;
  char levelMsg[256];
  Boolean redrawArena;
  Boolean pauseMessage;
  int humansPlayingNum;
  int enemiesPlayingNum;
  int menuRows;

  static Boolean acceptInput;
};

typedef Viewport *ViewportP;

#endif
