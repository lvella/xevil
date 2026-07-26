/*
 * XEvil(TM) Copyright (C) 1994,2000 Steve Hardt and Michael Judge
 * http://www.xevil.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.  See gpl.txt.
 */

// "panel.cpp"  SDL port -- render-only text widget.

#include <cstring>

#include "utils.h"
#include "panel.h"
#include "font.h"


TextPanel::TextPanel(Xvars &xv,int dpy,const Pos &p,const Size &s,
                     int tScale,const char *msg)
    : xvars(xv), dpyNum(dpy), pos(p), size(s) {
  textScale = tScale < 1 ? 1 : tScale;
  foreground = xvars.black[dpyNum];
  background = xvars.windowBg[dpyNum];
  border = True;
  message[0] = '\0';
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
        int margin = PANEL_MARGAIN * textScale;
        int baseline = pos.y + margin + f->ascent * textScale
                       + f->cellH * textScale * lineNo;
        font::draw_scaled(ren,*f,pos.x + margin,baseline,line,
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
}
