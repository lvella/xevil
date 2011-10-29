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

// "panel.cpp"

#ifndef NO_PRAGMAS
#pragma implementation "panel.h"
#endif


#include "utils.h"
extern "C" {
#include <X11/Xutil.h>
#include <X11/keysym.h>
}
#include <strstream>

#include "utils.h"
#include "xdata.h"
#include "panel.h"

using namespace std;


#define PANEL_BORDER 1
#define PANEL_MARGAIN 2
#define PANEL_BG_COLOR Xvars_WINDOW_BG_COLOR
#define KEYSYM_BUFFER 10



Panel::Panel(int dpy_num,const Xvars &x_vars,Window parent,
             const Pos &p,const Size &s,
             PanelCallback cb,void* clsre,
             unsigned int eventMask,
             Boolean border) 
: xvars(x_vars), callback(cb), closure(clsre) {
  dpyNum = dpy_num;
  foreground = xvars.black[dpyNum];
  background = xvars.alloc_named_color(dpyNum,PANEL_BG_COLOR);

  int borderWidth = (border ? PANEL_BORDER : 0);

  size.width = s.width - 2 * borderWidth;
  size.height = s.height - 2 * borderWidth;

  window = XCreateSimpleWindow(xvars.dpy[dpyNum],parent,p.x,p.y,
			       size.width,size.height,borderWidth,
			       xvars.windowBorder[dpyNum],
			       xvars.alloc_named_color(dpyNum,PANEL_BG_COLOR));
  XSelectInput(xvars.dpy[dpyNum],window,eventMask);
  XMapWindow(xvars.dpy[dpyNum],window);
}



Panel::~Panel() {
  XDestroyWindow(xvars.dpy[dpyNum],window);
}



void Panel::set_foreground(Pixel c,Boolean re_draw) {
  foreground = c;
  if (re_draw) {
    redraw();
  }
}



void Panel::set_background(Pixel c,Boolean re_draw) {
  background = c;
  if (re_draw) {
    redraw();
  }
}



Boolean Panel::process_event(int dpy_num,XEvent *event) {
  assert(dpyNum == dpy_num);
  if (event->type == Expose && event->xexpose.window == get_window()) {
    redraw();
    return True;
  }
  
  return False;
}



TextPanel::TextPanel(int dpy_num,const Xvars &xv,Window parent,
                     const Pos &p,const Size &s,
                     PanelCallback callback,void* closure,
                     const char *msg,
                     unsigned int eventMask,
                     Boolean border)
: Panel(dpy_num,xv,parent,p,s,callback,closure,eventMask,border) {
  if (msg != NULL) {
    assert(strlen(msg) < PANEL_STRING_LENGTH);
    strcpy(message,msg);
  }
  else {
    strcpy(message,"");
  }
  sensitive = True;
}



void TextPanel::set_message(const char *msg) {
  assert(msg);
  if (strlen(msg) >= PANEL_STRING_LENGTH) {
    strncpy(message,msg,PANEL_STRING_LENGTH - 1);
  }
  else {
    strcpy(message,msg);
  }
  redraw();
}



Size TextPanel::get_unit(XFontStruct *font,int cols,int rows) {
  Size fontSize;
  fontSize.width = font->max_bounds.width;
  fontSize.height = font->max_bounds.ascent + font->max_bounds.descent;

  Size ret;
  ret.width = fontSize.width * cols + 2 * PANEL_BORDER + 2 * PANEL_MARGAIN;
  ret.height = fontSize.height * rows + 2 * PANEL_BORDER + 2 * PANEL_MARGAIN;
  return ret;
}



void TextPanel::redraw() {
  Size size = get_size();
  const Xvars &xvars = get_xvars();
  int dpyNum = get_dpy_num();
  XSetForeground(xvars.dpy[dpyNum],xvars.gc[dpyNum],get_background());
  XFillRectangle(xvars.dpy[dpyNum],get_window(),xvars.gc[dpyNum],0,0,
		 size.width,size.height);
  XSetForeground(xvars.dpy[dpyNum],xvars.gc[dpyNum],get_foreground());

  // Draw text, line by line.
  char *start = message;
  char *current = message;
  int lineNo = 0;
  while (True) {
    if (*current == '\n' || *current == '\0') {
	  XDrawString(xvars.dpy[dpyNum],get_window(),
                  xvars.gc[dpyNum],PANEL_MARGAIN,
                  PANEL_MARGAIN + xvars.font[dpyNum]->max_bounds.ascent +
                  (xvars.font[dpyNum]->max_bounds.ascent + 
                   xvars.font[dpyNum]->max_bounds.descent) * lineNo,
                  start,current - start);
	  start = current + 1;
	  lineNo++;
	  if (*current == '\0') {
	    break;
      }
	}
    current++;
  }
      

  if (!sensitive) {
    XSetFillStyle(xvars.dpy[dpyNum],xvars.gc[dpyNum],FillStippled);
    XFillRectangle(xvars.dpy[dpyNum],get_window(),xvars.gc[dpyNum],0,0,
                   size.width,size.height);
    XSetFillStyle(xvars.dpy[dpyNum],xvars.gc[dpyNum],FillSolid);
  }
  XSetForeground(xvars.dpy[dpyNum],xvars.gc[dpyNum],xvars.black[dpyNum]);
}



void TextPanel::clear() {
  const Xvars &xvars = get_xvars();
  int dpyNum = get_dpy_num();
  XClearWindow(xvars.dpy[dpyNum],get_window());
}



void TextPanel::set_sensitive(Boolean s) {
  if (sensitive != s) {
    sensitive = s;
    redraw();
  }
}



WritePanel::WritePanel(int dpy_num,const Xvars &xvars,
                       Window parent,const Pos &pos,const Size &size,
                       PanelCallback cb,void* clos,
                       const char *pmpt,
                       unsigned int eventMask)
: TextPanel(dpy_num,xvars,parent,pos,size,cb,clos,NULL,eventMask) {
  assert(strlen(pmpt) < PANEL_STRING_LENGTH);
  strcpy(prompt,pmpt);
  strcpy(value,"");

  active = False;
  update_message();
}



void WritePanel::set_value(const char *val) {
  assert(strlen(val) < PANEL_STRING_LENGTH);
  strcpy(value,val);
  update_message();
}



Boolean WritePanel::process_event(int dpy_num,XEvent *event) {
  int dpyNum = get_dpy_num();
  assert(dpyNum == dpy_num);
  
  if (get_sensitive()) {
    if (event->xkey.window == get_window()) {
      if (event->type == KeyPress) {
	    char buffer[KEYSYM_BUFFER];
	    KeySym keysym;
	    int len = 
	      XLookupString(&event->xkey,buffer,KEYSYM_BUFFER,&keysym,NULL);
	    buffer[len] = '\0';
	    
	    if ((keysym >= XK_KP_Space && keysym <= XK_KP_9) ||
            (keysym >= XK_space && keysym <= XK_asciitilde)) {
          if (active) {
		    if (strlen(value) + len < PANEL_STRING_LENGTH) {
		      strcat(value,buffer);
            }
		  }
          else {
		    active = True;
		    if (strlen(value) < PANEL_STRING_LENGTH) {
		      strcpy(value,buffer);
            }
		  }
          update_message();
        }
	    else if (keysym == XK_BackSpace || keysym == XK_Delete) {
          if (active) {
		    if (strlen(value) > 0) {
		      value[strlen(value) - 1] = '\0';
            }
		  }
          else {
		    value[0] = '\0';
		    active = True;
		  }
          update_message();
        }
	    else if (keysym == XK_Return || keysym == XK_KP_Enter 
                 || keysym == XK_Linefeed) {
          if (active) {
		    void* closure = get_closure();
            PanelCallback callback = get_callback();
            if (callback) {
              (*callback)(this,(void*)value,closure);
            }
		    active = False;
		  }
          else {
		    value[0] = '\0';
		    active = True;
		  }
          update_message();
        }
	    
	    return True;
	  }
      else if (event->type == ButtonPress) {
	    if (!active) {
          value[0] = '\0';
          active = True;
        }
	    update_message();
	    return True;
	  }
    }
  }
  
  return TextPanel::process_event(dpyNum,event);
}



void WritePanel::update_message() {
  ostrstream tmp;
  if (active) {
    tmp << prompt << value << "_" << ends;
  }
  else {
    tmp << prompt << value << ends;
  }
  set_message(tmp.str());
  delete tmp.str(); 
}



KeyPressPanel::KeyPressPanel(int dpy_num,const Xvars &xvars,
                             Window parent,const Pos &pos,const Size &size,
                             PanelCallback callback,void* closure,
                             const char *msg,
                             unsigned int eventMask)
: TextPanel(dpy_num,xvars,parent,pos,size,callback,closure,msg,eventMask) {
}



Boolean KeyPressPanel::process_event(int dpy_num,XEvent *event) {
  int dpyNum = get_dpy_num();
  assert(dpyNum == dpy_num);

  if (get_sensitive() && event->type == KeyPress && 
      event->xbutton.window == get_window()) {
    PanelCallback callback = get_callback();
    void* closure = get_closure();
    if (callback) {
      (*callback)(this,(void*)event,closure);
    }
    return True;
  }
  else {
    return TextPanel::process_event(dpyNum,event);
  }
}



ButtonPanel::ButtonPanel(int dpy_num,const Xvars &xvars,
                         Window parent,const Pos &pos,const Size &size,
                         PanelCallback callback,void* closure,
                         const char *msg,
                         unsigned int eventMask)
: TextPanel(dpy_num,xvars,parent,pos,size,callback,closure,msg,eventMask) {
}



Boolean ButtonPanel::process_event(int dpyNum,XEvent *event) {
  if (get_sensitive() && 
      (event->type == ButtonPress) && 
      (event->xbutton.window == get_window())) {
    PanelCallback callback = get_callback();
    void* closure = get_closure();
    if (callback) {
      (*callback)(this,(void*)(int)event->xbutton.button,closure);
    }
    return True;
  }
  else {
    return TextPanel::process_event(dpyNum,event);
  }
}



TogglePanel::TogglePanel(int dpy_num,const Xvars &xvars,
                         Window parent,const Pos &pos,const Size &size,
                         PanelCallback callback,void* closure,
                         const char *msg,
                         unsigned int eventMask)
: TextPanel(dpy_num,xvars,parent,pos,size,callback,closure,msg,eventMask) {
  set = False;
}



void TogglePanel::set_value(Boolean s) {
  if (s != set) {
    Pixel foreground = get_foreground();
    set_foreground(get_background(),False);
    set_background(foreground);
    set = s;
  }
}



Boolean TogglePanel::process_event(int dpyNum,XEvent *event) {
  if (get_sensitive() && 
      (event->type == ButtonPress) && 
      (event->xbutton.window == get_window())) {
    set_value(!set);
    
    PanelCallback callback = get_callback();
    void* closure = get_closure();
    if (callback) {
      (*callback)(this,(void*)set,closure);
    }
    return True;
  }
  else {
    return TextPanel::process_event(dpyNum,event);
  }
}



ChatPanel::ChatPanel(int dpyNum,const Xvars &xvars,
                     Window parent,const Pos &pos,const Size &size,
                     PanelCallback callback,void* closure,
                     const char *msg,
                     unsigned int eventMask)
  : TextPanel(dpyNum,xvars,parent,pos,size,callback,closure,msg,
              eventMask) {
  value[0] = '\0';
  chatOn = False;
}



void ChatPanel::set_chat(Boolean val) {
  // Nothing to do.
  if (val == chatOn) {
    return;
  }

  chatOn = val;

  if (chatOn) {
    update_message();
  }
  else {
    // Erase current chat message.
    value[0] = '\0';
    // Clear the visible area.
    set_message("");
  }
}

  

// Some code copied from WritePanel.
Boolean ChatPanel::process_event(int dpy_num,XEvent *event) {
  int dpyNum = get_dpy_num();
  assert(dpyNum == dpy_num);
  
  // If chat is on grab all KeyPress events even if they are for some
  // other window on the display.
  if (get_sensitive() && chatOn && event->type == KeyPress) {
    char buffer[KEYSYM_BUFFER];
    KeySym keysym;
    int len = 
      XLookupString(&event->xkey,buffer,KEYSYM_BUFFER,&keysym,NULL);
    buffer[len] = '\0';
    
    // Printable character typed.
    if ((keysym >= XK_KP_Space && keysym <= XK_KP_9) ||
        (keysym >= XK_space && keysym <= XK_asciitilde)) {
      if (Utils::strlen(value) + len < PANEL_STRING_LENGTH) {
        Utils::strcat(value,buffer);
      }
      update_message();
    }
    // Delete a character.
    else if (keysym == XK_BackSpace || keysym == XK_Delete) {
      if (Utils::strlen(value) > 0) {
        value[Utils::strlen(value) - 1] = '\0';
      }
      update_message();      
    }
    // Commit the message call callback.
    else if (keysym == XK_Return || keysym == XK_KP_Enter 
             || keysym == XK_Linefeed) {
      void* closure = get_closure();
      PanelCallback callback = get_callback();
      if (callback) {
        (*callback)(this,(void*)value,closure);
      }
      set_chat(False);
    }
    // Cancel chat mode.
    else if (keysym == XK_Escape) {
      set_chat(False);
    }
    return True;
  }
  
  return TextPanel::process_event(dpyNum,event);
}



void ChatPanel::set_message(const char *msg) {
  // Want to disable all set_message() commands if chat mode is engaged.
  if (chatOn) {
    return;
  }
  TextPanel::set_message(msg);
}



void ChatPanel::update_message() {
  if (!chatOn) { 
    return;
  }

  ostrstream tmp;
  tmp << "CHAT <<" << value << "\nEnter to send, Esc to cancel." << ends;
  TextPanel::set_message(tmp.str());
  delete tmp.str(); 
}

