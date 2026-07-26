/*
 * XEvil(TM) Copyright (C) 1994,2000 Steve Hardt and Michael Judge
 * http://www.xevil.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.  See gpl.txt.
 */

// "viewport.cpp"  SDL port -- arena render + interactive chrome for one player.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>

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

// Viewport scroll increments (no associated intel), from x11/viewport.cpp.
#define ROW_SHIFT 5
#define COL_SHIFT 4


// x11 status enum order (index into statuses[]).
enum {statusName,statusClassName,statusHealth,statusMass,
      statusWeapon,statusItem,statusLivesHKills,statusKillsMKills};


Boolean Viewport::acceptInput = False;


// Menu-callback forwarding record (mirror of the x11 PanelClosure).
struct PanelClosure {
  Boolean radio;
  Viewport *viewport;
  ViewportCallback callback;
  void *uiClosure;
};


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
  Size visible;
  visible.set((LG_COL_MAX + 2 * LG_EXTRA_COL) * WSQUARE_WIDTH,
              (LG_ROW_MAX + 2 * LG_EXTRA_ROW) * WSQUARE_WIDTH);
  ViewportInfo vi(IT_VISION_RANGE,visible);
  p->set_value(vi);
}



/* ------------------------------------------------------------------ */

Viewport::Viewport(Xvars &xv,int dpy,WorldP w,LocatorP l,int sc,
                   ViewportCallback cbs[VIEWPORT_CB_NUM],void *uiClos,
                   RoleType rType,
                   const DifficultyLevel dLevels[DIFFICULTY_LEVELS_NUM])
    : xvars(xv), dpyNum(dpy), world(w), locator(l), scale(sc) {
  intel = NULL;
  humanColorNum = 0;
  input = UI_INPUT_NONE;
  styleType = KILL;
  roleType = rType;
  difficultyLevels = dLevels;
  promptDifficulty = False;
  uiClosure = uiClos;
  focusPanel = NULL;
  arenaMessage = NULL;
  redrawArena = True;
  pauseMessage = False;
  humansPlayingNum = 0;
  enemiesPlayingNum = 0;
  levelMsg[0] = '\0';
  menuRows = 1;
  arenaMessageTimer.set_max(VW_ARENA_MESSAGE_TIME_DEFAULT);

  for (int n = 0; n < VIEWPORT_CB_NUM; n++) {
    callbacks[n] = cbs[n];
    PanelClosure *pc = new PanelClosure;
    pc->radio = (n == menuScenarios || n == menuLevels || n == menuKill ||
                 n == menuDuel || n == menuExtended || n == menuTraining ||
                 n == menuSurvival || n == menuBossRush);
    pc->viewport = this;
    pc->callback = cbs[n];
    pc->uiClosure = uiClos;
    panelClosures[n] = (void *)pc;
  }

  arenaWorld.set((LG_COL_MAX + 2 * LG_EXTRA_COL) * WSQUARE_WIDTH,
                 (LG_ROW_MAX + 2 * LG_EXTRA_ROW) * WSQUARE_HEIGHT);
  arenaPix.set(arenaWorld.width * scale,arenaWorld.height * scale);

  // Start aligned with the upper-left of the world (title screen).
  viewportArea = Area(Pos(0,0),arenaWorld);

  // The back-buffer texture belongs to THIS window's renderer.
  xvars.set_active_display(dpyNum);
  buffer = xvars.create_target_pixmap(arenaPix.width,arenaPix.height);

  for (int n = 0; n < VW_MENUS_NUM; n++) menus[n] = NULL;
  for (int n = 0; n < VW_STATUSES_NUM; n++) statuses[n] = NULL;
  levelPanel = NULL;
  messageBar = NULL;

  layout();
}



Viewport::~Viewport() {
  Utils::freeif(arenaMessage);
  for (int n = 0; n < VW_MENUS_NUM; n++) delete menus[n];
  for (int n = 0; n < VW_STATUSES_NUM; n++) delete statuses[n];
  delete levelPanel;
  delete messageBar;
  for (int n = 0; n < VIEWPORT_CB_NUM; n++) {
    delete (PanelClosure *)panelClosures[n];
  }
  if (buffer) {
    xvars.free_pixmap(buffer);
  }
}



/* ------------------------------------------------------------------ *
 * Layout: menu bar (flow) + arena + statuses + level + message bar.
 * ------------------------------------------------------------------ */

void Viewport::layout() {
  const BitmapFont *f = xvars.font[dpyNum];
  int rowH = TextPanel::get_unit(f,1,1,scale).height;

  create_menus();                 // fills menus[], sets menuRows
  int menuH = menuRows * rowH;

  arenaPos = Pos(0,menuH);

  create_statuses();              // 2 x 4 grid below the arena

  Pixel bg = xvars.windowBg[dpyNum];
  int y0 = menuH + arenaPix.height;

  // Level + play-count info line.
  levelPanel = new TextPanel(xvars,dpyNum,Pos(0,y0 + 2 * rowH),
                             mk_size(arenaPix.width,rowH),scale);
  levelPanel->set_background(bg);

  // Two-line message bar / chat input.
  messageBar = new ChatPanel(xvars,dpyNum,Pos(0,y0 + 3 * rowH),
                             mk_size(arenaPix.width,2 * rowH),scale,"",
                             Viewport::panel_callback,panelClosures[stChat]);
  messageBar->set_background(bg);

  windowSize.set(arenaPix.width,menuH + arenaPix.height + 5 * rowH);
}



// Descriptor for one menu-bar widget.
enum WType {W_LABEL,W_BUTTON,W_TOGGLE,W_WRITE};
struct MenuDef {
  int idx;
  WType type;
  const char *label;
  int lineLen;   // panel width in font cells (== x11 *_LINE_LENGTH)
};

// Visual (flow) order of the menu bar.  Wraps to a new row when a widget would
// overrun the window width.  Every menuControls..menuHelp index appears once.
static const MenuDef g_menuDefs[] = {
  {menuQuit,          W_BUTTON, "Quit",             5},
  {menuNewGame,       W_BUTTON, "New Game",         9},
  {menuHumansNum,     W_WRITE,  "Humans:",          9},
  {menuEnemiesNum,    W_WRITE,  "Enemies:",        11},
  {menuEnemiesRefill, W_TOGGLE, "Regen Enemies",   13},
  {menuLearnControls, W_TOGGLE, "Set Controls",    12},
  {menuControls,      W_TOGGLE, "Show Controls",   13},
  {menuQuanta,        W_WRITE,  "Speed(ms):",      13},
  {menuSound,         W_TOGGLE, "Sound",            5},
  {menuHelp,          W_TOGGLE, "Help",             4},
  {menuStyle,         W_LABEL,  "Game style:",     11},
  {menuLevels,        W_TOGGLE, "Levels",           6},
  {menuScenarios,     W_TOGGLE, "Scenarios",        9},
  {menuKill,          W_TOGGLE, "Kill, Kill, Kill",16},
  {menuDuel,          W_TOGGLE, "Duel",             4},
  {menuExtended,      W_TOGGLE, "Extended Duel",   13},
  {menuTraining,      W_TOGGLE, "Training",         8},
  {menuSurvival,      W_TOGGLE, "Survival",         8},
  {menuBossRush,      W_TOGGLE, "Boss Rush",        9},
  {menuCooperative,   W_TOGGLE, "Cooperative",     11},
};
#define MENU_DEFS_NUM ((int)(sizeof(g_menuDefs) / sizeof(g_menuDefs[0])))


void Viewport::create_menus() {
  const BitmapFont *f = xvars.font[dpyNum];
  Pixel menuBg = xvars.windowBg[dpyNum];
  int rowH = TextPanel::get_unit(f,1,1,scale).height;
  int width = arenaPix.width;

  Boolean dbg = (getenv("XEVIL_UI_DEBUG") != NULL);

  int x = 0, y = 0;
  int rows = 1;
  for (int i = 0; i < MENU_DEFS_NUM; i++) {
    const MenuDef &d = g_menuDefs[i];
    Size unit = TextPanel::get_unit(f,d.lineLen,1,scale);
    if (x > 0 && x + unit.width > width) {
      x = 0;
      y += rowH;
      rows++;
    }
    Pos p(x,y);
    TextPanel *panel = NULL;
    switch (d.type) {
    case W_BUTTON:
      panel = new ButtonPanel(xvars,dpyNum,p,unit,scale,d.label,
                              Viewport::panel_callback,panelClosures[d.idx]);
      break;
    case W_TOGGLE:
      panel = new TogglePanel(xvars,dpyNum,p,unit,scale,d.label,
                              Viewport::panel_callback,panelClosures[d.idx]);
      break;
    case W_WRITE:
      panel = new WritePanel(xvars,dpyNum,p,unit,scale,d.label,
                             Viewport::panel_callback,panelClosures[d.idx]);
      break;
    case W_LABEL:
    default:
      panel = new TextPanel(xvars,dpyNum,p,unit,scale,d.label);
      break;
    }
    panel->set_background(menuBg);
    menus[d.idx] = panel;
    if (dbg) {
      fprintf(stderr,"UIWIDGET menu \"%s\" %d %d %d %d\n",
              d.label,p.x,p.y,unit.width,unit.height);
    }
    x += unit.width;
  }
  menuRows = rows;
}



void Viewport::create_statuses() {
  const BitmapFont *f = xvars.font[dpyNum];
  int rowH = TextPanel::get_unit(f,1,1,scale).height;
  Pixel bg = xvars.windowBg[dpyNum];
  Boolean dbg = (getenv("XEVIL_UI_DEBUG") != NULL);

  int cellW = arenaPix.width / 4;
  int y0 = arenaPos.y + arenaPix.height;
  int order[2][4] = {
    {statusWeapon,statusName,statusHealth,statusLivesHKills},
    {statusItem,statusClassName,statusMass,statusKillsMKills},
  };
  const char *dbgName[VW_STATUSES_NUM] = {
    "name","className","health","mass","weapon","item","lives","kills"
  };
  for (int row = 0; row < 2; row++) {
    for (int col = 0; col < 4; col++) {
      int idx = order[row][col];
      int w = (col == 3) ? (arenaPix.width - 3 * cellW) : cellW;
      Pos p(col * cellW,y0 + row * rowH);
      Size sz = mk_size(w,rowH);
      // Weapon and item are clickable HUD buttons (fire/change/drop).
      if (idx == statusWeapon) {
        statuses[idx] = new ButtonPanel(xvars,dpyNum,p,sz,scale,"",
                                        Viewport::panel_callback,
                                        panelClosures[stWeapon]);
      } else if (idx == statusItem) {
        statuses[idx] = new ButtonPanel(xvars,dpyNum,p,sz,scale,"",
                                        Viewport::panel_callback,
                                        panelClosures[stItem]);
      } else {
        statuses[idx] = new TextPanel(xvars,dpyNum,p,sz,scale);
      }
      statuses[idx]->set_background(bg);
      if (dbg) {
        fprintf(stderr,"UIWIDGET status \"%s\" %d %d %d %d\n",
                dbgName[idx],p.x,p.y,sz.width,sz.height);
      }
    }
  }
}



/* ------------------------------------------------------------------ *
 * Menu-callback forwarding (radio + dispatch to the Ui static callback).
 * ------------------------------------------------------------------ */

void Viewport::panel_callback(TextPanel *panel,void *value,void *closure) {
  PanelClosure *pc = (PanelClosure *)closure;
  assert(pc);

  // Radio-button behavior: can't uncheck the active style toggle.  The logic
  // to uncheck the previously-active toggle lives in set_style_and_role_type().
  if (pc->radio) {
    Boolean bValue = (Boolean)(intptr_t)value;
    if (!bValue) {
      ((TogglePanel *)panel)->set_value(True);
    }
  }

  if (pc->callback) {
    pc->callback(value,pc->viewport,pc->uiClosure);
  }
}



/* ------------------------------------------------------------------ *
 * Input.
 * ------------------------------------------------------------------ */

void Viewport::receive_key(int key,Boolean down) {
  keyState.set(key,down);
}



void Viewport::post_clock() {
  if (input != UI_INPUT_NONE) {
    keyDispatcher.clock(&keyState,this,NULL);
  }
}



// Implement IDispatcher.
void Viewport::dispatch(ITcommand command,void *) {
  if (!acceptInput) {
    return;
  }

  // Optional evidence hook: XEVIL_INPUT_DEBUG logs each dispatched command and
  // whether it reached a controllable human (used to prove key rebinding).
  if (command != IT_NO_COMMAND && command != IT_CENTER &&
      getenv("XEVIL_INPUT_DEBUG")) {
    Boolean toHuman = (intel && intel->is_playing() && intel->is_human());
    fprintf(stderr,"INPUTDBG vp_dpy=%d input=%d cmd=%d -> %s\n",
            dpyNum,(int)input,(int)command,toHuman ? "human" : "viewport");
  }

  if (command == IT_CHAT) {
    // Unlike X11 (which gates on Role::uses_chat and so disables chat in
    // stand-alone), the single-window SDL port always lets the local player
    // open the chat bar -- handy for a local message and required so the
    // ChatPanel is exercisable in single player.
    if (messageBar) {
      messageBar->set_chat(True);
    }
    return;
  }

  // If there is an intel associated with the viewport, give command to it.
  if (intel && intel->is_playing()) {
    if (intel->is_human()) {
      ((HumanP)intel)->set_command(command);
    }
    return;
  }

  // No associated intel: scroll the viewport with the keyset.
  Boolean changed = False;
  switch (command) {
  case IT_R:    changed = shift_viewport(COL_SHIFT,0); break;
  case IT_DN_R: changed = shift_viewport(COL_SHIFT,ROW_SHIFT); break;
  case IT_DN:   changed = shift_viewport(0,ROW_SHIFT); break;
  case IT_DN_L: changed = shift_viewport(-COL_SHIFT,ROW_SHIFT); break;
  case IT_L:    changed = shift_viewport(-COL_SHIFT,0); break;
  case IT_UP_L: changed = shift_viewport(-COL_SHIFT,-ROW_SHIFT); break;
  case IT_UP:   changed = shift_viewport(0,-ROW_SHIFT); break;
  case IT_UP_R: changed = shift_viewport(COL_SHIFT,-ROW_SHIFT); break;
  default:      changed = False;
  }
  if (changed) {
    redrawArena = True;
  }
}



Boolean Viewport::shift_viewport(int cols,int rows) {
  Pos p = viewportArea.get_pos();
  Pos np(p.x + cols * WSQUARE_WIDTH,p.y + rows * WSQUARE_HEIGHT);
  if (np.x < 0) np.x = 0;
  if (np.y < 0) np.y = 0;
  if (np.x == p.x && np.y == p.y) {
    return False;
  }
  viewportArea.set_pos(np);
  return True;
}



TextPanel *Viewport::find_panel_at(const Pos &at) {
  for (int n = 0; n < VW_MENUS_NUM; n++) {
    if (menus[n] && menus[n]->hit(at)) {
      return menus[n];
    }
  }
  for (int n = 0; n < VW_STATUSES_NUM; n++) {
    if (statuses[n] && statuses[n]->hit(at)) {
      return statuses[n];
    }
  }
  return NULL;
}



Boolean Viewport::handle_mouse(int button,int wx,int wy) {
  Pos at(wx,wy);
  TextPanel *clicked = find_panel_at(at);

  // Clicking anything other than the active field drops keyboard focus.
  if (focusPanel && clicked != focusPanel) {
    focusPanel->deactivate();
    focusPanel = NULL;
  }
  if (!clicked) {
    return False;
  }
  Boolean consumed = clicked->button_press(button,at);
  if (consumed && clicked->has_focus()) {
    focusPanel = clicked;
  }
  return consumed;
}



Boolean Viewport::handle_text_key(const SDL_Keysym &ks) {
  // Chat grabs every key while engaged.
  if (messageBar && messageBar->grabs_keys()) {
    return messageBar->key_press(ks);
  }
  // Otherwise the focused WritePanel (if any) gets first crack.
  if (focusPanel) {
    Boolean consumed = focusPanel->key_press(ks);
    if (!focusPanel->has_focus()) {
      focusPanel = NULL;
    }
    return consumed;
  }
  return False;
}



/* ------------------------------------------------------------------ *
 * Per-frame state pushed in by the Ui.
 * ------------------------------------------------------------------ */

void Viewport::register_intel(int humanColorNumArg,IntelP intl) {
  intel = intl;
  humanColorNum = humanColorNumArg;
  if (intel && intel->is_human()) {
    Pixel pixel = xvars.humanColors[dpyNum][humanColorNum %
                                            Xvars::HUMAN_COLORS_NUM];
    for (int n = 0; n < VW_STATUSES_NUM; n++) {
      statuses[n]->set_foreground(pixel);
    }
  }
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
  if (messageBar && msg) {
    messageBar->set_message(msg);
  }
}



void Viewport::reset() {
  intel = NULL;
  Utils::freeif(arenaMessage);
  arenaMessage = NULL;
  pauseMessage = False;
  promptDifficulty = False;
  redrawArena = True;
  focusPanel = NULL;
  if (messageBar) {
    messageBar->set_chat(False);
  }
  for (int n = 0; n < VW_STATUSES_NUM; n++) {
    statuses[n]->set_message("");
    statuses[n]->set_foreground(xvars.black[dpyNum]);
  }
  viewportArea.set_pos(Pos(0,0));
}



/* ------------------------------------------------------------------ *
 * Menu-bar value setters (Game drives these through the Ui).
 * ------------------------------------------------------------------ */

void Viewport::set_style_and_role_type(GameStyleType style,RoleType rType) {
  styleType = style;
  roleType = rType;

  ((TogglePanel *)menus[menuScenarios])->set_value(style == SCENARIOS);
  ((TogglePanel *)menus[menuLevels])->set_value(style == LEVELS);
  ((TogglePanel *)menus[menuKill])->set_value(style == KILL);
  ((TogglePanel *)menus[menuDuel])->set_value(style == DUEL);
  ((TogglePanel *)menus[menuExtended])->set_value(style == EXTENDED);
  ((TogglePanel *)menus[menuTraining])->set_value(style == TRAINING);
  ((TogglePanel *)menus[menuSurvival])->set_value(style == SURVIVAL);
  ((TogglePanel *)menus[menuBossRush])->set_value(style == BOSS_RUSH);
}



void Viewport::set_menu_humans_num(int val) {
  char buf[32];
  snprintf(buf,sizeof(buf),"%d",val);
  ((WritePanel *)menus[menuHumansNum])->set_value(buf);
}



void Viewport::set_menu_enemies_num(int val) {
  char buf[32];
  snprintf(buf,sizeof(buf),"%d",val);
  ((WritePanel *)menus[menuEnemiesNum])->set_value(buf);
}



void Viewport::set_menu_quanta(Quanta val) {
  char buf[32];
  snprintf(buf,sizeof(buf),"%d",(int)val);
  ((WritePanel *)menus[menuQuanta])->set_value(buf);
}



void Viewport::set_cooperative(Boolean val) {
  ((TogglePanel *)menus[menuCooperative])->set_value(val);
}



void Viewport::set_enemies_refill(Boolean val) {
  ((TogglePanel *)menus[menuEnemiesRefill])->set_value(val);
}



void Viewport::set_menu_sound(Boolean val) {
  ((TogglePanel *)menus[menuSound])->set_value(val);
}



void Viewport::set_menu_controls(Boolean val) {
  ((TogglePanel *)menus[menuControls])->set_value(val);
}



void Viewport::set_menu_learn_controls(Boolean val) {
  ((TogglePanel *)menus[menuLearnControls])->set_value(val);
}



void Viewport::set_menu_help(Boolean val) {
  ((TogglePanel *)menus[menuHelp])->set_value(val);
}



/* ------------------------------------------------------------------ *
 * Drawing.
 * ------------------------------------------------------------------ */

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
  font::draw_scaled(xvars.renderer,*bf,px,baseline,msg,
                    Pixel_r(xvars.black[dpyNum]),Pixel_g(xvars.black[dpyNum]),
                    Pixel_b(xvars.black[dpyNum]),255,scale);
  font::draw_scaled(xvars.renderer,*bf,px + scale,baseline + scale,msg,
                    Pixel_r(color),Pixel_g(color),Pixel_b(color),255,scale);
}



void Viewport::draw_arena() {
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

  xvars.set_target(0);
  SDL_Rect dst = {arenaPos.x,arenaPos.y,arenaPix.width,arenaPix.height};
  SDL_RenderCopy(xvars.renderer,buffer->tex,NULL,&dst);
}



void Viewport::draw_difficulty_prompt() {
  const BitmapFont *f = xvars.font[dpyNum];

  // Black arena background.
  xvars.set_target(0);
  SDL_Rect dst = {arenaPos.x,arenaPos.y,arenaPix.width,arenaPix.height};
  xvars.set_draw_color(xvars.black[dpyNum]);
  SDL_RenderFillRect(xvars.renderer,&dst);

  Pixel red = xvars.red[dpyNum];
  int lineH = f->cellH * scale;
  int x = arenaPos.x + f->cellW * scale;
  int y = arenaPos.y + lineH;

  font::draw_scaled(xvars.renderer,*f,x,y + f->ascent * scale,
                    "Enter level of difficulty:",
                    Pixel_r(red),Pixel_g(red),Pixel_b(red),255,scale);
  y += lineH;

  for (int n = 0; n < DIFFICULTY_LEVELS_NUM; n++) {
    char buf[128];
    if (n == DIFF_NORMAL) {
      snprintf(buf,sizeof(buf),"[%d,space]  %s",n,difficultyLevels[n].name);
    } else {
      snprintf(buf,sizeof(buf),"[%d]        %s",n,difficultyLevels[n].name);
    }
    y += lineH;
    font::draw_scaled(xvars.renderer,*f,x,y + f->ascent * scale,buf,
                      Pixel_r(red),Pixel_g(red),Pixel_b(red),255,scale);
  }
}



void Viewport::draw() {
  if (promptDifficulty) {
    draw_difficulty_prompt();
  } else {
    draw_arena();
  }

  // Chrome renders into the window (target already NULL after the arena).
  for (int n = 0; n < VW_MENUS_NUM; n++) {
    if (menus[n]) {
      menus[n]->render();
    }
  }
  for (int n = 0; n < VW_STATUSES_NUM; n++) {
    if (statuses[n]) {
      statuses[n]->render();
    }
  }

  if (levelPanel) {
    char bar[PANEL_STRING_LENGTH];
    snprintf(bar,sizeof(bar),"%s%sHumans: %d   Enemies: %d",
             levelMsg,levelMsg[0] ? "   " : "",
             humansPlayingNum,enemiesPlayingNum);
    levelPanel->set_message(bar);
    levelPanel->render();
  }

  if (messageBar) {
    messageBar->render();
  }

  redrawArena = False;
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
