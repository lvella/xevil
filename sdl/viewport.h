/*
 * XEvil(TM) Copyright (C) 1994,2000 Steve Hardt and Michael Judge
 * http://www.xevil.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.  See gpl.txt.
 */

// "viewport.h"  SDL port -- the render surface for one player.
//
// A single simplified Viewport draws the arena via the smooth/full-frame path
// (world->draw + locator->draw_directly + draw_ticks into one back-buffer
// texture, then one RenderCopy to the window) and lays out the menu-bar text
// row and the HUD status skeleton with TextPanels.  Menu/status *interaction*
// (clicks, two-player input) is deferred to stage 2.

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


class Viewport {
public:
  Viewport(Xvars &xvars,int dpyNum,WorldP,LocatorP,int scale);
  ~Viewport();

  // ---- Static ViewportInfo factory (cmn consumes it via Ui) ----
  static IViewportInfo *get_info();
  /* EFFECTS: An IViewportInfo whose value is ready after
     init_viewport_info().  Never freed. */

  static void init_viewport_info(Boolean largeViewport,Boolean smoothScroll);

  // ---- Window geometry ----
  Size get_window_size() {return windowSize;}

  // ---- Per-frame ----
  void pre_clock();
  /* EFFECTS: update_statuses + follow_intel + draw. */

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
  void set_style_type(GameStyleType s) {styleType = s;}
  void set_pause_message(Boolean on) {pauseMessage = on;}
  void set_menu_text(const char *top,const char *bottom);


private:
  void layout();
  void draw();
  void draw_arena();
  void update_statuses();
  void follow_intel();
  void draw_string_center(Drawable dest,const Size &arenaPix,const char *msg,
                          Pixel color);

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

  Drawable buffer;           // arena back-buffer (stretched arena pixels)
  Size arenaWorld;           // arena region, world coords (e.g. 480x256)
  Size arenaPix;             // arena, window pixels (arenaWorld * scale)
  Pos arenaPos;              // arena top-left in the window

  Size windowSize;

  // Chrome.
  TextPanel *menuBar;
  TextPanel *statuses[VW_STATUSES_NUM];
  TextPanel *levelPanel;
  TextPanel *messageBarPanel;

  char *arenaMessage;
  Timer arenaMessageTimer;
  char levelMsg[256];
  Boolean redrawArena;
  Boolean pauseMessage;
  int humansPlayingNum;
  int enemiesPlayingNum;
};

typedef Viewport *ViewportP;

#endif
