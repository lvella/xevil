/*
 * XEvil(TM) Copyright (C) 1994,2000 Steve Hardt and Michael Judge
 * http://www.xevil.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.  See gpl.txt.
 */

// "viewport.cpp"  SDL port -- arena render + chrome for one player.

#include <cstdio>
#include <cstring>

#include "utils.h"
#include "coord.h"
#include "area.h"
#include "xdata.h"
#include "world.h"
#include "locator.h"
#include "physical.h"
#include "intel.h"
#include "viewport.h"

using namespace std;


// Geometry (mirrors x11/viewport.cpp LargeViewport constants).  The SDL port
// always renders the "large", full-frame (smooth) visible region.
#define LG_COL_MAX 26
#define LG_ROW_MAX 16
#define LG_EXTRA_COL 2
#define LG_EXTRA_ROW 0
#define ARENA_MESSAGE_OFF_CENTER 25

#define VW_ARENA_MESSAGE_TIME_DEFAULT 40


// x11 status enum order (index into statuses[]).
enum {statusName,statusClassName,statusHealth,statusMass,
      statusWeapon,statusItem,statusLivesHKills,statusKillsMKills};


static Size mk_size(int w,int h) {
  Size s;
  s.set(w,h);
  return s;
}


/* ------------------------------------------------------------------ *
 * The lazy ViewportInfo provider (cmn consumes this via Ui).
 * ------------------------------------------------------------------ */
class VInfoProvider: public IViewportInfo {
public:
  VInfoProvider() {vInfo = NULL;}
  void set_value(const ViewportInfo &v) {
    delete vInfo;
    vInfo = new ViewportInfo(v);
  }
  virtual ViewportInfo get_info() {
    assert(vInfo);
    return *vInfo;
  }
private:
  ViewportInfo *vInfo;
};

static VInfoProvider *g_vInfoProvider = NULL;


IViewportInfo *Viewport::get_info() {
  if (!g_vInfoProvider) {
    g_vInfoProvider = new VInfoProvider();
  }
  return g_vInfoProvider;
}


void Viewport::init_viewport_info(Boolean /*largeViewport*/,
                                  Boolean /*smoothScroll*/) {
  VInfoProvider *p = (VInfoProvider *)get_info();
  // Always the large, full-frame visible region.
  Size visible;
  visible.set((LG_COL_MAX + 2 * LG_EXTRA_COL) * WSQUARE_WIDTH,
              (LG_ROW_MAX + 2 * LG_EXTRA_ROW) * WSQUARE_WIDTH);
  ViewportInfo vi(IT_VISION_RANGE,visible);
  p->set_value(vi);
}



/* ------------------------------------------------------------------ */

Viewport::Viewport(Xvars &xv,int dpy,WorldP w,LocatorP l,int sc)
    : xvars(xv), dpyNum(dpy), world(w), locator(l), scale(sc) {
  intel = NULL;
  humanColorNum = 0;
  input = UI_INPUT_NONE;
  styleType = (GameStyleType)0;
  arenaMessage = NULL;
  redrawArena = True;
  pauseMessage = False;
  humansPlayingNum = 0;
  enemiesPlayingNum = 0;
  levelMsg[0] = '\0';
  arenaMessageTimer.set_max(VW_ARENA_MESSAGE_TIME_DEFAULT);

  arenaWorld.set((LG_COL_MAX + 2 * LG_EXTRA_COL) * WSQUARE_WIDTH,
                 (LG_ROW_MAX + 2 * LG_EXTRA_ROW) * WSQUARE_HEIGHT);
  arenaPix.set(arenaWorld.width * scale,arenaWorld.height * scale);

  // Start aligned with the upper-left of the world (title screen).
  viewportArea = Area(Pos(0,0),arenaWorld);

  buffer = xvars.create_target_pixmap(arenaPix.width,arenaPix.height);

  menuBar = NULL;
  levelPanel = NULL;
  messageBarPanel = NULL;
  for (int n = 0; n < VW_STATUSES_NUM; n++) {
    statuses[n] = NULL;
  }

  layout();
}



Viewport::~Viewport() {
  Utils::freeif(arenaMessage);
  delete menuBar;
  delete levelPanel;
  delete messageBarPanel;
  for (int n = 0; n < VW_STATUSES_NUM; n++) {
    delete statuses[n];
  }
  if (buffer) {
    xvars.free_pixmap(buffer);
  }
}



void Viewport::layout() {
  const BitmapFont *f = xvars.font[dpyNum];
  Size menuUnit = TextPanel::get_unit(f,1,1,scale);
  int menuH = 2 * menuUnit.height;
  int rowH = menuUnit.height;
  int statusH = 2 * rowH;
  int messageH = rowH;

  arenaPos = Pos(0,menuH);
  windowSize.set(arenaPix.width,menuH + arenaPix.height + statusH + messageH);

  Pixel bg = xvars.windowBg[dpyNum];

  menuBar = new TextPanel(xvars,dpyNum,Pos(0,0),
                          mk_size(arenaPix.width,menuH),scale);
  menuBar->set_background(bg);
  set_menu_text(
    "Controls  Set Controls  Quit  New Game  Kill  Duel  Extended  Training",
    "Survival  Boss Rush  Cooperative  Sound  Help");

  // 8 status panels: 2 rows x 4 cols below the arena.
  int cellW = arenaPix.width / 4;
  int y0 = menuH + arenaPix.height;
  int order[2][4] = {
    {statusWeapon,statusName,statusHealth,statusLivesHKills},
    {statusItem,statusClassName,statusMass,statusKillsMKills},
  };
  for (int row = 0; row < 2; row++) {
    for (int col = 0; col < 4; col++) {
      int idx = order[row][col];
      int w = (col == 3) ? (arenaPix.width - 3 * cellW) : cellW;
      statuses[idx] =
        new TextPanel(xvars,dpyNum,Pos(col * cellW,y0 + row * rowH),
                      mk_size(w,rowH),scale);
      statuses[idx]->set_background(bg);
    }
  }

  messageBarPanel =
    new TextPanel(xvars,dpyNum,Pos(0,y0 + statusH),
                  mk_size(arenaPix.width,messageH),scale);
  messageBarPanel->set_background(bg);
}



void Viewport::set_menu_text(const char *top,const char *bottom) {
  if (!menuBar) {
    return;
  }
  char both[PANEL_STRING_LENGTH];
  snprintf(both,sizeof(both),"%s\n%s",top ? top : "",bottom ? bottom : "");
  menuBar->set_message(both);
}



void Viewport::register_intel(int humanColorNumArg,IntelP intl) {
  intel = intl;
  humanColorNum = humanColorNumArg;
}



void Viewport::set_level(const char *msg) {
  if (!msg) {
    levelMsg[0] = '\0';
    return;
  }
  strncpy(levelMsg,msg,sizeof(levelMsg) - 1);
  levelMsg[sizeof(levelMsg) - 1] = '\0';
}



void Viewport::set_arena_message(const char *msg,Quanta time) {
  Utils::freeif(arenaMessage);
  arenaMessage = Utils::strdup(msg);
  arenaMessageTimer.set(time < 0 ? VW_ARENA_MESSAGE_TIME_DEFAULT : time);
  redrawArena = True;
}



void Viewport::set_message(const char *msg) {
  if (messageBarPanel && msg) {
    messageBarPanel->set_message(msg);
  }
}



void Viewport::reset() {
  Utils::freeif(arenaMessage);
  arenaMessage = NULL;
  pauseMessage = False;
  redrawArena = True;
}



void Viewport::follow_intel() {
  if (intel && intel->is_playing()) {
    PhysicalP p = locator->lookup(intel->get_id());
    if (p) {
      const Area &a = p->get_area();
      Pos posNew = a.get_middle() - 0.5f * viewportArea.get_size();
      viewportArea.set_pos(posNew);
    }
  }
}



void Viewport::update_statuses() {
  if (!intel || !intel->intel_status_changed()) {
    return;
  }
  const IntelStatus *status = intel->get_intel_status();
  char buf[256];

  statuses[statusName]->set_message(status->name);
  statuses[statusClassName]->set_message(status->className);

  if (status->health == -1) {
    statuses[statusHealth]->set_message("Dead");
  } else {
    snprintf(buf,sizeof(buf),"%d Health",status->health);
    statuses[statusHealth]->set_message(buf);
  }

  snprintf(buf,sizeof(buf),"%d Mass",status->mass);
  statuses[statusMass]->set_message(buf);

  if (status->weaponClassId == A_None) {
    snprintf(buf,sizeof(buf),"No Weapon");
  } else if (status->ammo != PH_AMMO_UNLIMITED) {
    snprintf(buf,sizeof(buf),"%s (%d)",status->weapon,status->ammo);
  } else {
    snprintf(buf,sizeof(buf),"%s",status->weapon);
  }
  statuses[statusWeapon]->set_foreground(status->weaponReady ?
                                         xvars.green[dpyNum] : xvars.red[dpyNum]);
  statuses[statusWeapon]->set_message(buf);

  if (status->itemClassId == A_None) {
    snprintf(buf,sizeof(buf),"No Item");
  } else {
    snprintf(buf,sizeof(buf),"%s (%d)",status->item,status->itemCount);
  }
  statuses[statusItem]->set_message(buf);

  // (SDL stage 1: always uses the generic Lives/Kills wording; the EXTENDED
  // style's "Human Kills"/"Machine Kills" split is deferred to stage 2.)
  if (status->lives == IT_INFINITE_LIVES) {
    snprintf(buf,sizeof(buf),"Unlimited Lives");
  } else if (status->lives == 1) {
    snprintf(buf,sizeof(buf),"1 Life");
  } else {
    snprintf(buf,sizeof(buf),"%d Lives",status->lives);
  }
  statuses[statusLivesHKills]->set_message(buf);

  int kills = status->humanKills + status->enemyKills;
  if (kills == 1) {
    snprintf(buf,sizeof(buf),"1 Kill");
  } else {
    snprintf(buf,sizeof(buf),"%d Kills",kills);
  }
  statuses[statusKillsMKills]->set_message(buf);
}



void Viewport::draw_string_center(Drawable dest,const Size &arenaPixSize,
                                  const char *msg,Pixel color) {
  const BitmapFont *bf = xvars.bigFont[dpyNum];
  int textW = bf->cellW * scale * (int)strlen(msg);
  int textH = bf->cellH * scale;

  int px = (arenaPixSize.width - textW) / 2;
  int py = (arenaPixSize.height - textH) / 2
           - xvars.stretch_y(ARENA_MESSAGE_OFF_CENTER);
  int baseline = py + bf->ascent * scale;

  xvars.set_target(dest);
  // Black shadow, then the text color offset by one scaled pixel.
  font::draw_scaled(xvars.renderer,*bf,px,baseline,msg,
                    Pixel_r(xvars.black[dpyNum]),Pixel_g(xvars.black[dpyNum]),
                    Pixel_b(xvars.black[dpyNum]),255,scale);
  font::draw_scaled(xvars.renderer,*bf,px + scale,baseline + scale,msg,
                    Pixel_r(color),Pixel_g(color),Pixel_b(color),255,scale);
}



void Viewport::draw_arena() {
  // Full-frame smooth path: render the whole visible world into the buffer.
  xvars.set_target(buffer);
  xvars.set_draw_color(xvars.black[dpyNum]);
  SDL_RenderClear(xvars.renderer);

  world->draw(buffer,xvars,dpyNum,viewportArea,False /*reduceDraw*/,
              True /*background3D*/);
  locator->draw_directly(buffer,xvars,dpyNum,viewportArea);
  if (intel) {
    locator->draw_ticks(buffer,xvars,dpyNum,viewportArea,intel->get_id(),
                        locator);
  }

  if (arenaMessage) {
    draw_string_center(buffer,arenaPix,arenaMessage,
                       xvars.arenaTextColor[dpyNum]);
  }
  if (pauseMessage) {
    draw_string_center(buffer,arenaPix,"PAUSED",xvars.arenaTextColor[dpyNum]);
  }

  // Blit the arena buffer into the window.
  xvars.set_target(0);
  SDL_Rect dst = {arenaPos.x,arenaPos.y,arenaPix.width,arenaPix.height};
  SDL_RenderCopy(xvars.renderer,buffer->tex,NULL,&dst);
}



void Viewport::draw() {
  draw_arena();

  // Chrome renders into the window (target already NULL after draw_arena).
  if (menuBar) {
    menuBar->render();
  }
  for (int n = 0; n < VW_STATUSES_NUM; n++) {
    if (statuses[n]) {
      statuses[n]->render();
    }
  }

  // Compose the bottom bar: level + play counts.
  if (messageBarPanel) {
    char bar[PANEL_STRING_LENGTH];
    snprintf(bar,sizeof(bar),"%s%sHumans: %d   Enemies: %d",
             levelMsg,levelMsg[0] ? "   " : "",
             humansPlayingNum,enemiesPlayingNum);
    messageBarPanel->set_message(bar);
    messageBarPanel->render();
  }
}



void Viewport::pre_clock() {
  update_statuses();
  follow_intel();
  draw();

  arenaMessageTimer.clock();
  if (arenaMessage && arenaMessageTimer.ready()) {
    Utils::freeif(arenaMessage);
    arenaMessage = NULL;
    redrawArena = True;
  }
}
