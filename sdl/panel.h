/*
 * XEvil(TM) Copyright (C) 1994,2000 Steve Hardt and Michael Judge
 * http://www.xevil.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.  See gpl.txt.
 */

// "panel.h"  SDL port -- a lean, render-only text widget.
//
// The X11 panels were bordered child Windows that handled their own X events.
// In the SDL single-window port a TextPanel is just a rectangle + label that
// renders into the shared window at an absolute position.  The get_unit()
// metric idiom is preserved 1:1 (border=1, margin=2, sizes derived from font
// max_bounds) so all the viewport layout math ports unchanged.  Click/keyboard
// interaction is deferred to stage 2.

#ifndef PANEL_H
#define PANEL_H

#include "utils.h"
#include "xdata.h"

#define PANEL_STRING_LENGTH 600
#define PANEL_BORDER 1
#define PANEL_MARGAIN 2


class TextPanel {
public:
  TextPanel(Xvars &xvars,int dpyNum,const Pos &pos,const Size &size,
            int textScale = 1,const char *msg = "");

  void set_pos(const Pos &p) {pos = p;}
  Pos get_pos() {return pos;}
  void set_size(const Size &s) {size = s;}
  Size get_size() {return size;}

  void set_message(const char *msg);
  const char *get_message() {return message;}

  void set_foreground(Pixel c) {foreground = c;}
  void set_background(Pixel c) {background = c;}

  void set_border(Boolean b) {border = b;}

  void render();
  /* EFFECTS: Draw the panel (background fill, optional border, text lines) at
     its absolute position onto the current render target. */

  static Size get_unit(const BitmapFont *font,int cols,int rows = 1,
                       int textScale = 1);
  /* EFFECTS: The universal sizing helper (== X11 TextPanel::get_unit, scaled):
     width  = font.cellW * cols * textScale + 2*border + 2*margin*textScale,
     height = font.cellH * rows * textScale + 2*border + 2*margin*textScale. */


private:
  Xvars &xvars;
  int dpyNum;
  Pos pos;
  Size size;
  int textScale;
  Pixel foreground, background;
  Boolean border;
  char message[PANEL_STRING_LENGTH];
};

// Stage-2 interaction (clicks/toggles) not modeled yet; the viewport uses
// TextPanel for every menu/status/message widget.
typedef TextPanel ButtonPanel;
typedef TextPanel TogglePanel;

#endif
