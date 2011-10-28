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

// "panel.h"  Some simple widgets.


#ifndef PANEL_H
#define PANEL_H

#ifndef NO_PRAGMAS
#pragma interface
#endif

#include "utils.h"

#define PANEL_STRING_LENGTH 600


// Have #defines for these so caller doesn't have to rely on default
// arguments to get the default behavior.
#define PANEL_EVENT_MASK ExposureMask
#define TEXT_PANEL_EVENT_MASK ExposureMask


class Panel;
typedef void (*PanelCallback)(Panel* p,void* value,void* closure);



class Panel {
  // No callback.

 public:
  Panel(int dpyNum,const Xvars &,
        Window,const Pos &,const Size &,
        PanelCallback callback = NULL,void* closure = NULL,
        unsigned int eventMask = PANEL_EVENT_MASK,
        Boolean border = True);
  /* EFFECTS: Creatue a new panel at position pos of size size.  The position
     and size includes the border.  Color is initially black.  Use a 
     viewportNum of -1 if there is no associated viewportNum. */
  /* NOTE: Must explicity set dpyNum as a Panel may be created with 
     viewportNum = -1. */
  virtual ~Panel();

  Pixel get_foreground() {return foreground;}
  Pixel get_background() {return background;}
  Size get_size() {return size;}
  Window get_window() {return window;}
  const Xvars &get_xvars() {return xvars;}

  int get_dpy_num() {return dpyNum;}

  void set_foreground(Pixel c,Boolean re_draw = True);
  void set_background(Pixel c,Boolean re_draw = True);
  /* EFFECTS: Changes the color for the panel.  Redraws iff re_draw. */

  virtual Boolean process_event(int dpyNum,XEvent *event);
  /* EFFECTS: Panel deals with the event if it applies.  Returns True iff it
     used the event. */
  /* NOTE: Only calls up the tree if event is not processed at current 
     level. */

  virtual void redraw() = 0;
  virtual void clear() = 0;


#ifndef PROTECTED_IS_PUBLIC
protected:
#endif
  PanelCallback get_callback() {return callback;}
  void* get_closure() {return closure;}
  /* NOTE: Callback and closure not used by all subclasses of Panel. */


 private:
  int dpyNum;
  const Xvars &xvars;
  Window window;
  Size size; // Inside the border.
  Pixel foreground, background;
  PanelCallback callback;
  void* closure;
};



// Display a text label.
class TextPanel: public Panel {
  // No callback.

 public:
  TextPanel(int dpyNum,const Xvars &xvars,
            Window parent,const Pos &pos,const Size &size,
            PanelCallback callback = NULL,void* closure = NULL,
            const char *msg = NULL,
            unsigned int eventMask = TEXT_PANEL_EVENT_MASK,
            Boolean border = True);

  const char* get_message() {return message;}
  /* EFFECTS: Return the message currently displayed.  Return "" if no
     message displayed.  Never returns NULL. */
  
  virtual void set_message(const char *msg);
  /* REQUIRES: msg is non-NULL */
  /* NOTE:  Makes its own copy.  Redraws. */

  static Size get_unit(XFontStruct *,int cols,int rows = 1);

  virtual void redraw();
  virtual void clear();
  
  void set_sensitive(Boolean val);
  Boolean get_sensitive() {return sensitive;}


 private:
  char message[PANEL_STRING_LENGTH];
  Boolean sensitive;
};



// Accepts text input.  Displays "<prompt><input>_" as its message.
class WritePanel : public TextPanel {
  // Callback value is the (const char*) value of the TextPanel.
  
 public:
  WritePanel(int dpyNum,const Xvars &xvars,
             Window parent,
             const Pos &pos,const Size &size,
             PanelCallback callback = NULL,void* closure = NULL,
             const char *prompt = NULL,
             unsigned int eventMask = 
             ExposureMask | ButtonPressMask | KeyPressMask | KeyReleaseMask);
  /* NOTE: prompt appears before the user's input text. */
  
  const char *get_value() {return value;}
  void set_value(const char *value);

  virtual Boolean process_event(int dpyNum,XEvent *event);

  
 private:
  void update_message();

  Boolean active;
  char prompt[PANEL_STRING_LENGTH];
  char value[PANEL_STRING_LENGTH];
};



class KeyPressPanel: public TextPanel {
  // Callback value is the (XEvent*) event of the key press.

 public:
  KeyPressPanel(int dpyNum,const Xvars &xvars,
                Window parent,
                const Pos &pos,const Size &size,
                PanelCallback callback = NULL,void* closure = NULL,
                const char *msg = NULL,
                unsigned int eventMask = 
                ExposureMask | KeyPressMask | KeyReleaseMask);
  
  virtual Boolean process_event(int dpyNum,XEvent *event);
};



class ButtonPanel: public TextPanel {
  // Callback value is the (int) button number.

 public:
  ButtonPanel(int dpyNum,const Xvars &xvars,
              Window parent,
              const Pos &pos,const Size &size,
              PanelCallback callback = NULL,void* closure = NULL,
              const char *msg = NULL,
              unsigned int eventMask = ExposureMask | ButtonPressMask);
  
  virtual Boolean process_event(int dpyNum,XEvent *event);
};



class TogglePanel: public TextPanel {
  // Callback value is the Boolean state of the TogglePanel.

 public:
  TogglePanel(int dpyNum,const Xvars &xvars,
              Window parent,
              const Pos &pos,const Size &size,
              PanelCallback callback = NULL,void* closure = NULL,
              const char *msg = NULL,
              unsigned int eventMask = ExposureMask | ButtonPressMask);
  
  Boolean get_value() {return set;}
  void set_value(Boolean);

  virtual Boolean process_event(int dpyNum,XEvent *event);


 private:
  Boolean set;
};



// Act like a regular TextPanel unless chat is turned on.  If
// chat is on, ChatPanel will capture all key events to the
// display regardless of the target window.  Will call
// callback when the chat message is finished.  No callback
// if the message is canceled. 
class ChatPanel: public TextPanel {
  // Callback value is the (const char*) value of the ChatPanel.
  
public:
  ChatPanel(int dpyNum,const Xvars &xvars,
            Window parent,const Pos &pos,const Size &size,
            PanelCallback callback = NULL,void* closure = NULL,
            const char *msg = NULL,
            unsigned int eventMask = 
            ExposureMask | KeyPressMask | KeyReleaseMask);
  
  Boolean get_chat() {return chatOn;}

  void set_chat(Boolean val);
  /* NOTE: Will redraw if necessary. */

  const char* get_value() {return value;}
  /* EFFECTS: Return the chat message itself, not including any prompts
     or instructions to the user. */

  virtual Boolean process_event(int dpyNum,XEvent *event);
  /* NOTE: May grab events for windows other than this->get_window(). */

  virtual void set_message(const char *msg);
  /* NOTE: To override default behavior of TextPanel. */


private:
  void update_message();

  Boolean chatOn;
  char value[PANEL_STRING_LENGTH];
};


#endif
