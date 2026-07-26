/*
 * XEvil(TM) Copyright (C) 1994,2000 Steve Hardt and Michael Judge
 * http://www.xevil.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.  See gpl.txt.
 */

// "panel.h"  SDL port -- real (interactive) widgets.
//
// The X11 panels were bordered child Windows that handled their own X events.
// In the SDL single-window port every panel is a rectangle + label positioned
// in absolute window coordinates; it is repainted each frame by the Viewport
// (there are no Expose events) and it hit-tests mouse/keyboard events the
// Viewport hands to it.  The get_unit() metric idiom is preserved 1:1
// (border=1, margin=2, sizes derived from font cell metrics) so all the
// viewport layout math ports unchanged from x11/panel.cpp.
//
// Callback semantics match x11/panel.h exactly:
//   ButtonPanel  -- value is the (intptr_t) mouse button number (1/2/3).
//   TogglePanel  -- value is the (Boolean) new state; checked swaps fg/bg.
//   WritePanel   -- value is the (const char*) typed string, on Return.
//   ChatPanel    -- value is the (const char*) chat message, on Return.

#ifndef PANEL_H
#define PANEL_H

#include "utils.h"
#include "xdata.h"

#define PANEL_STRING_LENGTH 600
#define PANEL_BORDER 1
#define PANEL_MARGAIN 2


class TextPanel;
// Matches the X11 PanelCallback shape: (panel, value, closure).
typedef void (*PanelCallback)(TextPanel *p,void *value,void *closure);


class TextPanel {
public:
  TextPanel(Xvars &xvars,int dpyNum,const Pos &pos,const Size &size,
            int textScale = 1,const char *msg = "",
            PanelCallback callback = NULL,void *closure = NULL);
  virtual ~TextPanel() {}

  void set_pos(const Pos &p) {pos = p;}
  Pos get_pos() {return pos;}
  void set_size(const Size &s) {size = s;}
  Size get_size() {return size;}
  int get_dpy_num() {return dpyNum;}

  virtual void set_message(const char *msg);
  const char *get_message() {return message;}

  void set_foreground(Pixel c) {foreground = c;}
  Pixel get_foreground() {return foreground;}
  void set_background(Pixel c) {background = c;}
  Pixel get_background() {return background;}

  void set_border(Boolean b) {border = b;}
  void set_sensitive(Boolean b) {sensitive = b;}
  Boolean get_sensitive() {return sensitive;}

  Boolean hit(const Pos &p);
  /* EFFECTS: Is window-coordinate point p inside this panel's rectangle. */

  virtual void render();
  /* EFFECTS: Draw the panel (background fill, optional border, text lines,
     grayed stipple when insensitive) at its absolute position onto the
     current render target. */

  // ---- Event hooks (base class: no-ops).  Return True iff consumed. ----
  virtual Boolean button_press(int button,const Pos &at) {
    (void)button; (void)at; return False;
  }
  virtual Boolean key_press(const SDL_Keysym &ks) {(void)ks; return False;}
  virtual Boolean grabs_keys() {return False;}   // ChatPanel when chat is on
  virtual Boolean has_focus() {return False;}    // WritePanel while editing
  virtual void deactivate() {}                    // lost keyboard focus

  static Size get_unit(const BitmapFont *font,int cols,int rows = 1,
                       int textScale = 1);
  /* EFFECTS: The universal sizing helper (== X11 TextPanel::get_unit, scaled):
     width  = font.cellW * cols * textScale + 2*border + 2*margin*textScale,
     height = font.cellH * rows * textScale + 2*border + 2*margin*textScale. */


protected:
  PanelCallback get_callback() {return callback;}
  void *get_closure() {return closure;}
  int margin() {return PANEL_MARGAIN * textScale;}

  Xvars &xvars;
  int dpyNum;
  Pos pos;
  Size size;
  int textScale;
  Pixel foreground, background;
  Boolean border;
  Boolean sensitive;
  char message[PANEL_STRING_LENGTH];
  PanelCallback callback;
  void *closure;
};



// Momentary button.  Callback value is the (intptr_t) mouse button number.
class ButtonPanel : public TextPanel {
public:
  ButtonPanel(Xvars &xvars,int dpyNum,const Pos &pos,const Size &size,
              int textScale = 1,const char *msg = "",
              PanelCallback callback = NULL,void *closure = NULL);
  virtual Boolean button_press(int button,const Pos &at);
};



// Checkbox.  Checked state swaps fg/bg (as on X11).  Callback value is the
// (Boolean) new state.
class TogglePanel : public TextPanel {
public:
  TogglePanel(Xvars &xvars,int dpyNum,const Pos &pos,const Size &size,
              int textScale = 1,const char *msg = "",
              PanelCallback callback = NULL,void *closure = NULL);
  Boolean get_value() {return set;}
  void set_value(Boolean);
  virtual Boolean button_press(int button,const Pos &at);
private:
  Boolean set;
};



// Editable field.  Displays "<prompt><value>_" while active.  Callback value
// is the (const char*) typed string, fired on Return.
class WritePanel : public TextPanel {
public:
  WritePanel(Xvars &xvars,int dpyNum,const Pos &pos,const Size &size,
             int textScale = 1,const char *prompt = "",
             PanelCallback callback = NULL,void *closure = NULL);
  const char *get_value() {return value;}
  void set_value(const char *v);
  virtual Boolean button_press(int button,const Pos &at);
  virtual Boolean key_press(const SDL_Keysym &ks);
  virtual Boolean has_focus() {return active;}
  virtual void deactivate();
private:
  void update_message();
  Boolean active;
  char prompt[PANEL_STRING_LENGTH];
  char value[PANEL_STRING_LENGTH];
};



// The bottom message bar.  Acts like a plain label unless chat is turned on,
// in which case it grabs all keystrokes and echoes "CHAT <<...".  Callback
// value is the (const char*) chat message, fired on Return.
class ChatPanel : public TextPanel {
public:
  ChatPanel(Xvars &xvars,int dpyNum,const Pos &pos,const Size &size,
            int textScale = 1,const char *msg = "",
            PanelCallback callback = NULL,void *closure = NULL);
  Boolean get_chat() {return chatOn;}
  void set_chat(Boolean);
  const char *get_value() {return value;}
  virtual Boolean key_press(const SDL_Keysym &ks);
  virtual Boolean grabs_keys() {return chatOn;}
  virtual void set_message(const char *msg);   // no-op while chat is engaged
private:
  void update_message();
  Boolean chatOn;
  char value[PANEL_STRING_LENGTH];
};


// Convert a KEYDOWN keysym into the printable character it types, or 0.
// Handles the ASCII range plus shift (uppercase letters) and the numeric
// keypad digits, which is all the widgets need.  (Full IME/layout text entry
// via SDL_TEXTINPUT is a later refinement.)
char panel_printable_char(const SDL_Keysym &ks);


#endif
