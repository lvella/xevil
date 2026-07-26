/*
 * XEvil(TM) Copyright (C) 1994,2000 Steve Hardt and Michael Judge
 * http://www.xevil.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.  See gpl.txt.
 */

// "ui.cpp"  SDL port -- one window, one Viewport.

#include <cstdio>
#include <cstring>

#include "utils.h"
#include "ui.h"

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
  viewport = NULL;
  viewportsNum = 0;

  settingsChanges = UInone;
  memset((void *)&settings,0,sizeof(settings));  // POD-ish; only read per mask
  otherInput = False;
  pause = False;
  soundOn = True;
  roleType = rType;
  difficulty = DIFF_NONE;

  humansNumDisplay = 0;
  enemiesNumDisplay = 0;
  quantaDisplay = 40;
  styleDisplay = (GameStyleType)0;

  for (int n = 0; n < Xvars::DISPLAYS_MAX; n++) {
    keysetSet[n] = False;
  }

  // Resolve the integer display scale (same logic as the X11 Ui): -scale wins
  // over the large/small toggle; scale 0 (unset) falls back to the toggle.
  if (scale < 1) {
    scale = (largeViewport ? 2 : 1);
  } else {
    largeViewport = (scale >= 2);
  }
  xvars.stretch = scale;

  Viewport::init_viewport_info(largeViewport,smoothScroll);

  // Open the window (provisional size), create the viewport, then size the
  // window to the viewport's computed layout.
  Size provisional;
  provisional.set(640,480);
  if (!xvars.open_display("XEvil 2.5 (SDL)",provisional)) {
    cerr << "Failed to open SDL display." << endl;
    exit(1);
  }

  add_viewport();
  if (viewport) {
    xvars.resize_window(viewport->get_window_size());
  }
}



Ui::~Ui() {
  delete viewport;
  xvars.close_display();
}



int Ui::add_viewport() {
  // The SDL port supports one viewport (one window).  Additional viewports
  // (local two-player) are a stage-2 feature; reuse viewport 0.
  if (viewport) {
    return 0;
  }
  viewport = new Viewport(xvars,0,world,locator,scale);
  assert(viewport);
  viewportsNum = 1;
  refresh_menu_text();
  return 0;
}



void Ui::del_viewport() {
  // Never remove the primary viewport in the single-window SDL port.
}



void Ui::refresh_menu_text() {
  if (!viewport) {
    return;
  }
  char bottom[256];
  snprintf(bottom,sizeof(bottom),
           "Humans: %d   Enemies: %d   Speed(ms): %d",
           humansNumDisplay,enemiesNumDisplay,(int)quantaDisplay);
  viewport->set_menu_text(
    "Controls  Set Controls  Quit  New Game  Kill  Duel  Extended  Training  "
    "Survival  Boss Rush  Cooperative  Sound  Help",
    bottom);
}



UImask Ui::get_settings(UIsettings &s) {
  s = settings;
  UImask tmp = settingsChanges;
  settingsChanges = UInone;
  return tmp;
}



void Ui::set_humans_num(int v) {humansNumDisplay = v; refresh_menu_text();}
void Ui::set_enemies_num(int v) {enemiesNumDisplay = v; refresh_menu_text();}
void Ui::set_enemies_refill(Boolean) {}
void Ui::set_style(GameStyleType s) {styleDisplay = s;}
void Ui::set_quanta(Quanta q) {quantaDisplay = q; refresh_menu_text();}
void Ui::set_cooperative(Boolean) {}



void Ui::set_humans_playing(int v) {
  if (viewport) {
    viewport->set_humans_playing(v);
  }
}



void Ui::set_enemies_playing(int v) {
  if (viewport) {
    viewport->set_enemies_playing(v);
  }
}



void Ui::set_level(const char *msg) {
  if (viewport) {
    viewport->set_level(msg);
  }
}



void Ui::set_input(int vNum,UIinput input) {
  if (viewport && vNum == 0) {
    viewport->set_input(input);
  }
  // vNum >= 1 (second local player) not modeled in stage 1.
}



void Ui::set_keyset(int dpyNum,UIkeyset) {
  // SDL input mapping is a stage-2 concern; just mark the keyset present so the
  // per-frame assert in the X11 code path is satisfied.
  keysetSet[dpyNum] = True;
}



void Ui::set_keyset(int dpyNum,UIkeyset,KeySym[][2],KeySym[][2]) {
  keysetSet[dpyNum] = True;
}



void Ui::set_pause(Boolean val) {
  pause = val;
  if (viewport) {
    viewport->set_pause_message(val);
  }
}



void Ui::set_prompt_difficulty() {
  // Stage 1: auto-answer with DIFF_NORMAL so "New Game" never hangs waiting for
  // an on-arena difficulty prompt (that prompt is a stage-2 feature).
  difficulty = DIFF_NORMAL;
}



void Ui::unset_prompt_difficulty() {
  difficulty = DIFF_NONE;
}



void Ui::register_intel(int n,IntelP intel) {
  if (viewport && n == 0) {
    viewport->register_intel(n,intel);
  }
}



void Ui::demo_reset() {
  if (viewport) {
    viewport->reset();
  }
}



void Ui::reset() {
  demo_reset();
}



void Ui::set_redraw_arena() {
  if (viewport) {
    viewport->set_redraw_arena();
  }
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

  case SDL_KEYDOWN:
    otherInput = True;
    if (event->key.keysym.sym == SDLK_ESCAPE) {
      settingsChanges |= UIquit;
    }
    // If paused, any key un-pauses (mirrors the X11 behavior).
    if (pause) {
      settingsChanges |= UIpause;
      settings.pause = False;
    }
    break;

  case SDL_MOUSEBUTTONDOWN:
    otherInput = True;
    if (pause) {
      settingsChanges |= UIpause;
      settings.pause = False;
    }
    break;

  default:
    break;
  }
}



void Ui::pre_clock() {
  otherInput = False;

  // Clear the whole window to the chrome background so any area the viewport
  // does not paint is clean.
  xvars.set_target(0);
  Pixel bg = xvars.windowBg[0];
  SDL_SetRenderDrawColor(xvars.renderer,Pixel_r(bg),Pixel_g(bg),Pixel_b(bg),255);
  SDL_RenderClear(xvars.renderer);

  if (!pause) {
    // Route arena messages (level titles, kill notices) to the viewport.
    char *arenaMsg;
    Boolean exclusive;
    do {
      IntelId msgTarget;
      Quanta time;
      Boolean propagate;
      exclusive = locator->arena_message_deq(&arenaMsg,msgTarget,time,propagate);
      (void)exclusive;  // single viewport: all messages route to it (stage 1)
      if (arenaMsg) {
        if (viewport) {
          viewport->set_arena_message(arenaMsg,time);
        }
        delete arenaMsg;
      }
    } while (arenaMsg);
  }

  if (viewport) {
    viewport->pre_clock();
  }

  if (!pause) {
    // One-line message bar.
    char *msg;
    if ((msg = locator->message_deq())) {
      if (viewport) {
        viewport->set_message(msg);
      }
      delete msg;
    }
  }

  SDL_RenderPresent(xvars.renderer);
}



void Ui::post_clock() {
  // Key -> command dispatch is a stage-2 feature (no local human control yet).
}
