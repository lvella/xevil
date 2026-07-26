/*
 * XEvil(TM) Copyright (C) 1994,2000 Steve Hardt and Michael Judge
 * http://www.xevil.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.  See gpl.txt.
 */

// "panel.cpp"  SDL port -- interactive widgets (see panel.h).

#include <cstdio>
#include <cstring>

#include "utils.h"
#include "panel.h"
#include "font.h"


/* ------------------------------------------------------------------ *
 * TextPanel (base label + geometry + hit-testing).
 * ------------------------------------------------------------------ */

TextPanel::TextPanel(Xvars &xv,int dpy,const Pos &p,const Size &s,
                     int tScale,const char *msg,
                     PanelCallback cb,void *clos)
    : xvars(xv), dpyNum(dpy), pos(p), size(s) {
  textScale = tScale < 1 ? 1 : tScale;
  foreground = xvars.black[dpyNum];
  background = xvars.windowBg[dpyNum];
  border = True;
  sensitive = True;
  message[0] = '\0';
  callback = cb;
  closure = clos;
  if (msg) {
    set_message(msg);
  }
}



void TextPanel::set_message(const char *msg) {
  if (!msg) {
    message[0] = '\0';
    return;
  }
  if (strlen(msg) >= PANEL_STRING_LENGTH) {
    strncpy(message,msg,PANEL_STRING_LENGTH - 1);
    message[PANEL_STRING_LENGTH - 1] = '\0';
  } else {
    strcpy(message,msg);
  }
}



Boolean TextPanel::hit(const Pos &p) {
  return sensitive &&
         p.x >= pos.x && p.x < pos.x + size.width &&
         p.y >= pos.y && p.y < pos.y + size.height;
}



Size TextPanel::get_unit(const BitmapFont *font,int cols,int rows,
                         int textScale) {
  if (textScale < 1) textScale = 1;
  Size ret;
  ret.width = font->cellW * cols * textScale
              + 2 * PANEL_BORDER + 2 * PANEL_MARGAIN * textScale;
  ret.height = font->cellH * rows * textScale
               + 2 * PANEL_BORDER + 2 * PANEL_MARGAIN * textScale;
  return ret;
}



void TextPanel::render() {
  SDL_Renderer *ren = xvars.renderer;

  // Background fill.
  SDL_Rect r = {pos.x,pos.y,size.width,size.height};
  SDL_SetRenderDrawColor(ren,Pixel_r(background),Pixel_g(background),
                         Pixel_b(background),255);
  SDL_RenderFillRect(ren,&r);

  // Border.
  if (border) {
    Pixel bc = xvars.windowBorder[dpyNum];
    SDL_SetRenderDrawColor(ren,Pixel_r(bc),Pixel_g(bc),Pixel_b(bc),255);
    SDL_RenderDrawRect(ren,&r);
  }

  // Text, line by line (split on '\n'), like X11 TextPanel::redraw.
  const BitmapFont *f = xvars.font[dpyNum];
  const char *start = message;
  const char *cur = message;
  int lineNo = 0;
  while (True) {
    if (*cur == '\n' || *cur == '\0') {
      int len = (int)(cur - start);
      if (len > 0) {
        char line[PANEL_STRING_LENGTH];
        if (len >= PANEL_STRING_LENGTH) {
          len = PANEL_STRING_LENGTH - 1;
        }
        memcpy(line,start,len);
        line[len] = '\0';
        int m = margin();
        int baseline = pos.y + m + f->ascent * textScale
                       + f->cellH * textScale * lineNo;
        font::draw_scaled(ren,*f,pos.x + m,baseline,line,
                          Pixel_r(foreground),Pixel_g(foreground),
                          Pixel_b(foreground),255,textScale);
      }
      start = cur + 1;
      lineNo++;
      if (*cur == '\0') {
        break;
      }
    }
    cur++;
  }

  // Grayed-out (insensitive) overlay -- the SDL analogue of X11's FillStippled.
  if (!sensitive) {
    SDL_SetRenderDrawColor(ren,Pixel_r(background),Pixel_g(background),
                           Pixel_b(background),150);
    SDL_RenderFillRect(ren,&r);
  }
}



/* ------------------------------------------------------------------ *
 * ButtonPanel.
 * ------------------------------------------------------------------ */

ButtonPanel::ButtonPanel(Xvars &xv,int dpy,const Pos &p,const Size &s,
                         int tScale,const char *msg,
                         PanelCallback cb,void *clos)
    : TextPanel(xv,dpy,p,s,tScale,msg,cb,clos) {}



Boolean ButtonPanel::button_press(int button,const Pos &at) {
  if (!sensitive || !hit(at)) {
    return False;
  }
  PanelCallback cb = get_callback();
  if (cb) {
    cb(this,(void *)(intptr_t)button,get_closure());
  }
  return True;
}



/* ------------------------------------------------------------------ *
 * TogglePanel (checked state swaps fg/bg, as on X11).
 * ------------------------------------------------------------------ */

TogglePanel::TogglePanel(Xvars &xv,int dpy,const Pos &p,const Size &s,
                         int tScale,const char *msg,
                         PanelCallback cb,void *clos)
    : TextPanel(xv,dpy,p,s,tScale,msg,cb,clos) {
  set = False;
}



void TogglePanel::set_value(Boolean s) {
  if (s != set) {
    Pixel fg = foreground;
    foreground = background;
    background = fg;
    set = s;
  }
}



Boolean TogglePanel::button_press(int button,const Pos &at) {
  (void)button;
  if (!sensitive || !hit(at)) {
    return False;
  }
  set_value(!set);
  PanelCallback cb = get_callback();
  if (cb) {
    cb(this,(void *)(intptr_t)set,get_closure());
  }
  return True;
}



/* ------------------------------------------------------------------ *
 * WritePanel (editable text field).
 * ------------------------------------------------------------------ */

WritePanel::WritePanel(Xvars &xv,int dpy,const Pos &p,const Size &s,
                       int tScale,const char *pmpt,
                       PanelCallback cb,void *clos)
    : TextPanel(xv,dpy,p,s,tScale,NULL,cb,clos) {
  strncpy(prompt,pmpt ? pmpt : "",PANEL_STRING_LENGTH - 1);
  prompt[PANEL_STRING_LENGTH - 1] = '\0';
  value[0] = '\0';
  active = False;
  update_message();
}



void WritePanel::set_value(const char *v) {
  strncpy(value,v ? v : "",PANEL_STRING_LENGTH - 1);
  value[PANEL_STRING_LENGTH - 1] = '\0';
  update_message();
}



Boolean WritePanel::button_press(int button,const Pos &at) {
  (void)button;
  if (!sensitive || !hit(at)) {
    return False;
  }
  // Clicking starts a fresh entry (mirrors X11 WritePanel).
  value[0] = '\0';
  active = True;
  update_message();
  return True;
}



Boolean WritePanel::key_press(const SDL_Keysym &ks) {
  if (!sensitive) {
    return False;
  }
  SDL_Keycode sym = ks.sym;
  char c = panel_printable_char(ks);
  if (c) {
    if (active) {
      size_t len = strlen(value);
      if (len + 1 < PANEL_STRING_LENGTH) {
        value[len] = c;
        value[len + 1] = '\0';
      }
    } else {
      active = True;
      value[0] = c;
      value[1] = '\0';
    }
    update_message();
    return True;
  }
  if (sym == SDLK_BACKSPACE || sym == SDLK_DELETE) {
    if (active) {
      size_t l = strlen(value);
      if (l > 0) {
        value[l - 1] = '\0';
      }
    } else {
      value[0] = '\0';
      active = True;
    }
    update_message();
    return True;
  }
  if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER || sym == SDLK_RETURN2) {
    if (active) {
      PanelCallback cb = get_callback();
      if (cb) {
        cb(this,(void *)value,get_closure());
      }
      active = False;
    } else {
      value[0] = '\0';
      active = True;
    }
    update_message();
    return True;
  }
  if (sym == SDLK_ESCAPE) {
    // Cancel editing (consume so ESC does not also quit the game).
    active = False;
    update_message();
    return True;
  }
  return False;
}



void WritePanel::deactivate() {
  if (active) {
    active = False;
    update_message();
  }
}



void WritePanel::update_message() {
  char buf[2 * PANEL_STRING_LENGTH];   // oversized; set_message() truncates
  if (active) {
    snprintf(buf,sizeof(buf),"%s%s_",prompt,value);
  } else {
    snprintf(buf,sizeof(buf),"%s%s",prompt,value);
  }
  TextPanel::set_message(buf);
}



/* ------------------------------------------------------------------ *
 * ChatPanel (message bar; grabs all keys while chat is engaged).
 * ------------------------------------------------------------------ */

ChatPanel::ChatPanel(Xvars &xv,int dpy,const Pos &p,const Size &s,
                     int tScale,const char *msg,
                     PanelCallback cb,void *clos)
    : TextPanel(xv,dpy,p,s,tScale,msg,cb,clos) {
  value[0] = '\0';
  chatOn = False;
}



void ChatPanel::set_chat(Boolean val) {
  if (val == chatOn) {
    return;
  }
  chatOn = val;
  if (chatOn) {
    value[0] = '\0';
    update_message();
  } else {
    value[0] = '\0';
    TextPanel::set_message("");
  }
}



Boolean ChatPanel::key_press(const SDL_Keysym &ks) {
  if (!sensitive || !chatOn) {
    return False;
  }
  SDL_Keycode sym = ks.sym;
  char c = panel_printable_char(ks);
  if (c) {
    size_t len = strlen(value);
    if (len + 1 < PANEL_STRING_LENGTH) {
      value[len] = c;
      value[len + 1] = '\0';
    }
    update_message();
    return True;
  }
  if (sym == SDLK_BACKSPACE || sym == SDLK_DELETE) {
    size_t l = strlen(value);
    if (l > 0) {
      value[l - 1] = '\0';
    }
    update_message();
    return True;
  }
  if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER || sym == SDLK_RETURN2) {
    PanelCallback cb = get_callback();
    if (cb) {
      cb(this,(void *)value,get_closure());
    }
    set_chat(False);
    return True;
  }
  if (sym == SDLK_ESCAPE) {
    set_chat(False);
    return True;
  }
  return True;   // swallow every key while chatting
}



void ChatPanel::set_message(const char *msg) {
  // Disable all set_message() commands while chat mode is engaged, so the
  // live "CHAT <<..." prompt is not clobbered (matches X11 ChatPanel).
  if (chatOn) {
    return;
  }
  TextPanel::set_message(msg);
}



void ChatPanel::update_message() {
  if (!chatOn) {
    return;
  }
  char buf[2 * PANEL_STRING_LENGTH];   // oversized; set_message() truncates
  snprintf(buf,sizeof(buf),"CHAT <<%s\nEnter to send, Esc to cancel.",value);
  TextPanel::set_message(buf);
}



/* ------------------------------------------------------------------ *
 * Shared keysym -> printable-char helper.
 * ------------------------------------------------------------------ */

char panel_printable_char(const SDL_Keysym &ks) {
  SDL_Keycode sym = ks.sym;
  Boolean shift = (ks.mod & KMOD_SHIFT) != 0;

  // Letters (with shift -> uppercase).
  if (sym >= SDLK_a && sym <= SDLK_z) {
    return shift ? (char)('A' + (sym - SDLK_a)) : (char)('a' + (sym - SDLK_a));
  }
  // Main-row digits / space / basic punctuation share their code with ASCII.
  if (sym >= SDLK_SPACE && sym < 127) {
    return (char)sym;
  }
  // Numeric keypad digits.
  switch (sym) {
  case SDLK_KP_0: return '0';
  case SDLK_KP_1: return '1';
  case SDLK_KP_2: return '2';
  case SDLK_KP_3: return '3';
  case SDLK_KP_4: return '4';
  case SDLK_KP_5: return '5';
  case SDLK_KP_6: return '6';
  case SDLK_KP_7: return '7';
  case SDLK_KP_8: return '8';
  case SDLK_KP_9: return '9';
  default: break;
  }
  return 0;
}
