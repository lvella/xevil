/*
 * XEvil(TM) Copyright (C) 1994,2000 Steve Hardt and Michael Judge
 * http://www.xevil.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.  See gpl.txt.
 */

// "dialog.cpp"  SDL port -- the modal first-run License Agreement dialog.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>

#include "utils.h"
#include "xdata.h"
#include "panel.h"
#include "font.h"
#include "l_agreement.h"
#include "dialog.h"

using namespace std;


// Mirror of cmn/game.cpp's XEVIL_RC_MARKER (line 1 of ~/.xevilrc; kept
// byte-identical so a file written by either path is recognized by both).
#define SDL_XEVIL_RC_MARKER "XEvil is your friend.  Trust XEvil."

#define DLG_COLS 70
#define DLG_ROWS 24
#define DLG_PAD  6


static Size mk_size_dlg(int w,int h) {
  Size s;
  s.set(w,h);
  return s;
}


static Boolean rc_path(char *out,int outLen) {
  const char *home = getenv("HOME");
  if (!home || !*home) {
    return False;
  }
  const char *sep = (home[strlen(home) - 1] == '/') ? "" : "/";
  snprintf(out,outLen,"%s%s%s",home,sep,".xevilrc");
  return True;
}


Boolean sdl_agreement_marker_present() {
  char path[512];
  if (!rc_path(path,sizeof(path))) {
    return False;
  }
  FILE *fp = fopen(path,"r");
  if (!fp) {
    return False;
  }
  char line[256];
  Boolean found = False;
  while (fgets(line,sizeof(line),fp)) {
    if (strstr(line,SDL_XEVIL_RC_MARKER)) {
      found = True;
      break;
    }
  }
  fclose(fp);
  return found;
}


void sdl_agreement_write_marker() {
  char path[512];
  if (!rc_path(path,sizeof(path))) {
    return;
  }
  // If a config already exists it already carries the marker as line 1 (the cmn
  // config writer guarantees that); don't clobber the saved settings.
  FILE *fp = fopen(path,"r");
  if (fp) {
    fclose(fp);
    if (sdl_agreement_marker_present()) {
      return;
    }
  }
  fp = fopen(path,"w");
  if (fp) {
    fprintf(fp,"%s\n",SDL_XEVIL_RC_MARKER);
    fclose(fp);
  }
}


/* ------------------------------------------------------------------ */

enum {DLG_NONE,DLG_PREV,DLG_NEXT,DLG_ACCEPT,DLG_REJECT};

struct DlgState {
  int action;
};

struct BtnClosure {
  int action;
  DlgState *st;
};

static void dlg_button_CB(TextPanel *,void *,void *closure) {
  BtnClosure *bc = (BtnClosure *)closure;
  bc->st->action = bc->action;
}


// Pick the largest dialog text scale (1..2) whose window fits the desktop.
static int pick_dialog_scale(const BitmapFont *f) {
  SDL_DisplayMode dm;
  int screenW = 0, screenH = 0;
  if (SDL_GetDesktopDisplayMode(0,&dm) == 0) {
    screenW = dm.w;
    screenH = dm.h;
  }
  for (int s = 2; s >= 1; s--) {
    int w = DLG_COLS * f->cellW * s + 2 * DLG_PAD;
    int h = DLG_ROWS * f->cellH * s + 8 * f->cellH * s + 6 * DLG_PAD;
    if (screenW <= 0 || (w <= screenW && h <= screenH)) {
      return s;
    }
  }
  return 1;
}


LicenseResult sdl_run_license_dialog(Xvars &xvars,int dpyNum,
                                     Boolean largeViewport,
                                     Boolean smoothScroll,
                                     Boolean drawBackground) {
  LicenseResult result;
  result.accepted = False;
  result.largeViewport = largeViewport;
  result.smoothScroll = smoothScroll;
  result.drawBackground = drawBackground;

  xvars.set_active_display(dpyNum);
  const BitmapFont *f = xvars.font[dpyNum];
  int s = pick_dialog_scale(f);
  int rowH = f->cellH * s;

  // Paginate the agreement text (reusing the platform-independent cmn parser).
  Line::set_text_columns(DLG_COLS);
  Page::set_text_rows(DLG_ROWS);
  PtrList pages;
  const char *p = LAgreement::get_text();
  while (*p) {
    pages.add(new Page(&p,p));
  }
  int pageNum = 0;
  int pageMax = pages.length();

  // Geometry.
  int textTop = DLG_PAD;
  int textH = DLG_ROWS * rowH;
  int togTop = textTop + textH + DLG_PAD;
  int togH = TextPanel::get_unit(f,1,1,s).height;
  int btnTop = togTop + togH + DLG_PAD;
  int btnH = togH;
  int winW = DLG_COLS * f->cellW * s + 2 * DLG_PAD;
  int winH = btnTop + btnH + DLG_PAD;

  Pixel bg = xvars.windowBg[dpyNum];
  Pixel fg = xvars.black[dpyNum];

  xvars.resize_window(dpyNum,mk_size_dlg(winW,winH));

  // Three render toggles (mirror x11 optionToggleLabels), left to right.
  const char *togLabels[3] = {"Large Viewport","Smooth Scroll","Draw Background"};
  Boolean togInit[3] = {largeViewport,smoothScroll,drawBackground};
  TogglePanel *toggles[3];
  int tx = DLG_PAD;
  for (int i = 0; i < 3; i++) {
    Size u = TextPanel::get_unit(f,(int)strlen(togLabels[i]),1,s);
    toggles[i] = new TogglePanel(xvars,dpyNum,Pos(tx,togTop),u,s,togLabels[i],
                                 NULL,NULL);
    toggles[i]->set_background(bg);
    toggles[i]->set_value(togInit[i]);
    tx += u.width + 2 * f->cellW * s;
  }

  // Four buttons: Prev, Next, Accept, Reject.
  DlgState st;
  st.action = DLG_NONE;
  const char *btnLabels[4] = {"Prev","Next","Accept","Reject"};
  int btnActions[4] = {DLG_PREV,DLG_NEXT,DLG_ACCEPT,DLG_REJECT};
  ButtonPanel *buttons[4];
  BtnClosure closures[4];
  int bx = DLG_PAD;
  for (int i = 0; i < 4; i++) {
    Size u = TextPanel::get_unit(f,(int)strlen(btnLabels[i]) + 2,1,s);
    closures[i].action = btnActions[i];
    closures[i].st = &st;
    buttons[i] = new ButtonPanel(xvars,dpyNum,Pos(bx,btnTop),u,s,btnLabels[i],
                                 dlg_button_CB,&closures[i]);
    buttons[i]->set_background(bg);
    bx += u.width + 2 * f->cellW * s;
  }

  Boolean running = True;
  while (running) {
    // ---- events ----
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_QUIT ||
          (ev.type == SDL_WINDOWEVENT &&
           ev.window.event == SDL_WINDOWEVENT_CLOSE)) {
        st.action = DLG_REJECT;
      } else if (ev.type == SDL_MOUSEBUTTONDOWN) {
        Pos at(ev.button.x,ev.button.y);
        for (int i = 0; i < 4; i++) {
          buttons[i]->button_press(ev.button.button,at);
        }
        for (int i = 0; i < 3; i++) {
          toggles[i]->button_press(ev.button.button,at);
        }
      } else if (ev.type == SDL_KEYDOWN) {
        switch (ev.key.keysym.sym) {
        case SDLK_ESCAPE:                       st.action = DLG_REJECT; break;
        case SDLK_RIGHT: case SDLK_PAGEDOWN:
        case SDLK_SPACE:                        st.action = DLG_NEXT;   break;
        case SDLK_LEFT:  case SDLK_PAGEUP:
        case SDLK_BACKSPACE:                    st.action = DLG_PREV;   break;
        case SDLK_RETURN: case SDLK_KP_ENTER:   st.action = DLG_ACCEPT; break;
        default: break;
        }
      }
    }

    // ---- act ----
    switch (st.action) {
    case DLG_PREV: if (pageNum > 0) pageNum--; break;
    case DLG_NEXT: if (pageNum < pageMax - 1) pageNum++; break;
    case DLG_ACCEPT: result.accepted = True; running = False; break;
    case DLG_REJECT: result.accepted = False; running = False; break;
    default: break;
    }
    st.action = DLG_NONE;
    if (!running) {
      break;
    }

    // ---- render ----
    xvars.set_target(0);
    SDL_SetRenderDrawColor(xvars.renderer,Pixel_r(bg),Pixel_g(bg),Pixel_b(bg),255);
    SDL_RenderClear(xvars.renderer);

    // Current page text.
    Page *page = (Page *)pages.get(pageNum);
    const PtrList &lines = page->get_lines();
    for (int n = 0; n < lines.length(); n++) {
      char *text = ((Line *)lines.get(n))->alloc_text();
      if (text && text[0]) {
        int baseline = textTop + f->ascent * s + rowH * n;
        font::draw_scaled(xvars.renderer,*f,DLG_PAD,baseline,text,
                          Pixel_r(fg),Pixel_g(fg),Pixel_b(fg),255,s);
      }
      delete [] text;
    }

    // Page counter (right-aligned on the button row).
    char counter[64];
    snprintf(counter,sizeof(counter),"Page %d/%d",pageNum + 1,pageMax);
    int cw = f->cellW * s * (int)strlen(counter);
    font::draw_scaled(xvars.renderer,*f,winW - DLG_PAD - cw,
                      btnTop + f->ascent * s + (btnH - rowH) / 2,counter,
                      Pixel_r(fg),Pixel_g(fg),Pixel_b(fg),255,s);

    // Separators.
    Pixel bc = xvars.windowBorder[dpyNum];
    SDL_SetRenderDrawColor(xvars.renderer,Pixel_r(bc),Pixel_g(bc),Pixel_b(bc),255);
    SDL_Rect sep1 = {0,togTop - DLG_PAD / 2,winW,1};
    SDL_Rect sep2 = {0,btnTop - DLG_PAD / 2,winW,1};
    SDL_RenderFillRect(xvars.renderer,&sep1);
    SDL_RenderFillRect(xvars.renderer,&sep2);

    for (int i = 0; i < 3; i++) toggles[i]->render();
    for (int i = 0; i < 4; i++) buttons[i]->render();

    SDL_RenderPresent(xvars.renderer);
    SDL_Delay(16);
  }

  result.largeViewport  = toggles[0]->get_value();
  result.smoothScroll   = toggles[1]->get_value();
  result.drawBackground = toggles[2]->get_value();

  for (int i = 0; i < 3; i++) delete toggles[i];
  for (int i = 0; i < 4; i++) delete buttons[i];
  for (int i = 0; i < pages.length(); i++) delete (Page *)pages.get(i);

  return result;
}
