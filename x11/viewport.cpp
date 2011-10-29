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

// "viewport.cpp"

#ifndef NO_PRAGMAS
#pragma implementation "viewport.h"
#endif


#include "utils.h"
#include "xdata.h"
#include "coord.h"
#include "viewport.h"
#include "game_style.h"
#include "physical.h"
#include "locator.h"
#include "role.h"


extern "C" {
#include <X11/Xutil.h>
#ifdef OPENWOUND_XOS_STRLEN_HACK
#define _strings_h
#endif
#include <X11/Xos.h>
#include <X11/keysym.h>
}
#include <strstream>

using namespace std;


#define TICK_BORDER_WIDTH 5 // border width big enough for a tick mark.
#define TICK_WIDTH 3
#define ARENA_BORDER_COLOR "black"
#define SMOOTH_BORDER_WIDTH 2  // Just for decoration.


#define MENU_BG_COLOR "gray80" 

// For viewports with no object.
#define ROW_SHIFT 5
#define COL_SHIFT 4

// Number of characters across for each menu button.
#define QUIT_LINE_LENGTH 5
#define NEW_GAME_LINE_LENGTH 9
#define HUMANS_NUM_LINE_LENGTH 9
#define ENEMIES_NUM_LINE_LENGTH 11
#define ENEMIES_REFILL_LINE_LENGTH 13
#define CONTROLS_LINE_LENGTH 13
#define LEARN_CONTROLS_LINE_LENGTH 12
#define STYLE_LINE_LENGTH 11
#define SCENARIOS_LINE_LENGTH 9
#define LEVELS_LINE_LENGTH 6
#define KILL_LINE_LENGTH 16
#define DUEL_LINE_LENGTH 4
#define EXTENDED_LINE_LENGTH 13
#define TRAINING_LINE_LENGTH 8
#define QUANTA_LINE_LENGTH 13
#define COOPERATIVE_LINE_LENGTH 11
#define HELP_LINE_LENGTH 4


// How far on top and sides viewport extends around the room.
#define SM_EXTRA_COL 2
#define SM_EXTRA_ROW 0
#define LG_EXTRA_COL 2
#define LG_EXTRA_ROW 0

// The inner dim, add LG_EXTRA_COL and LG_EXTRA_ROW to get the
// actual size of the Viewport.
#define LG_COL_MAX 26
#define LG_ROW_MAX 16
#define SM_COL_MAX 30
#define SM_ROW_MAX 16

#define SM_STATUS_LINE_LENGTH 25
#define LG_INTELS_LINE_LENGTH 11
#define LG_LEVEL_LINE_LENGTH SM_STATUS_LINE_LENGTH

// Arena message will be displayed this distance above
// the center of the screen.  This is so the text is not
// right on top of the player.
#define ARENA_MESSAGE_OFF_CENTER 25 // 40



enum {statusName,statusClassName,statusHealth,statusMass,
      statusWeapon,statusItem,statusLivesHKills,statusKillsMKills};



struct PanelClosure {
  Boolean radio;
  Viewport* viewport;
  // Callback passed into Viewport constructor. 
  ViewportCallback callback;
};



KeyState::KeyState() {
  for (int n = 0; n < UI_KEYS_MAX; n++) {
    isDown[n] = False;
  }
}



class VInfoProvider: public IViewportInfo {
public:
  VInfoProvider() {vInfo = NULL;}
  ~VInfoProvider() {delete vInfo;}

  void set_viewport_info(const ViewportInfo& v);
  /* EFFECTS: When the information is available, use this. */

  virtual ViewportInfo get_info();


private:
  ViewportInfo* vInfo;
};



void VInfoProvider::set_viewport_info(const ViewportInfo& v) {
  vInfo = new ViewportInfo(v); 
  assert(vInfo);
}



ViewportInfo VInfoProvider::get_info() {
  // If this assert happens, it means that Viewport::init_viewport_info()
  // was not called early enough.
  assert(vInfo);

  return *vInfo;
}



class BorderTickRenderer: public ITickRenderer {
public:
  BorderTickRenderer(Drawable outer,Xvars&,int dpyNum,
                     const Size& outerSize);

  ~BorderTickRenderer();

  virtual void begin_draw(int dpyNum);
  /* EFFECTS: Clear out the border buffers. */

  virtual void end_draw(int dpyNum);
  /* EFFECTS: Draw the border buffers to window. */

  virtual Boolean draw_tick(TickType tt,CMN_DRAWABLE window,
                            Xvars &xvars,int dpyNum,
                            const Size &arenaSize,
                            Dir tickDir,int offset);
  /* EFFECTS: Record one tick mark. */


private:
  static void init_x(Xvars&,int dpyNum,const Size& outerSize);
  /* EFFECTS: Initialize the static graphics data for one display.
     Can only do one at a time since we need outerSize to init
     borderBuffers. */

  Boolean dir_to_border_or_corner(int &val,Dir);
  /* MODIFIES: val */
  /* REQUIRES: Dir is one of the 8 cardinal directions. */
  /* EFFECTS: Convert dir to an enumerated border or corner.  Return True
     if border, False if corner. */

  enum {BORDER_R,BORDER_DN,BORDER_L,BORDER_UP,BORDER_MAX};
  enum {CORNER_DN_R,CORNER_DN_L,CORNER_UP_L,CORNER_UP_R,CORNER_MAX};

  Drawable outer;
  Xvars& xvars;
  int dpyNum;
  Size outerSize;

  // corners and borderBuffers should more appropriately be non-static,
  // since they are really specific to a single Renderer.  But, we share
  // the buffers for all the renderers on a display as an optimization.
  // This means that we can't have draw_tick() calls interleaved for
  // different renderers on the same display.  Never happens anyway.

  // Static data.
  static XvarsValid xValid[Xvars::DISPLAYS_MAX];
  // The rectangular areas corresponding to each border/corner.
  static Area borderAreas[Xvars::DISPLAYS_MAX][BORDER_MAX];
  static Area cornerAreas[Xvars::DISPLAYS_MAX][CORNER_MAX];
  // Rectangles of width ARENA_BORDER_COLOR.  Small back-buffers for
  // draw_tick().
  static Drawable borderBuffers[Xvars::DISPLAYS_MAX][BORDER_MAX];
  // Most recently drawn color for corner.
  static Pixel corners[Xvars::DISPLAYS_MAX][CORNER_MAX];
  static Pixel tickColors[Xvars::DISPLAYS_MAX][TICK_MAX];
  static Pixel background[Xvars::DISPLAYS_MAX];
  static const char* tickColorNames[TICK_MAX];
};



BorderTickRenderer::BorderTickRenderer(Drawable oter,Xvars& x_vars,
                                       int dpy_num,
                                       const Size& outer_size)
: outer(oter), 
  xvars(x_vars),
  dpyNum(dpy_num),
  outerSize(outer_size) {

  if (!xvars.is_valid(xValid[dpyNum])) {
    init_x(xvars,dpyNum,outerSize);
  }
}



BorderTickRenderer::~BorderTickRenderer() {
}



void BorderTickRenderer::begin_draw(int dpyNum) {
  // Erase the corners.
  for (int corner = 0; corner < CORNER_MAX; corner++) {
    corners[dpyNum][corner] = background[dpyNum];
  }

  // Erase the borderBuffers.
  XSetForeground(xvars.dpy[dpyNum],xvars.gc[dpyNum],background[dpyNum]);
  for (int border = 0; border < BORDER_MAX; border++) {
    Size borderSize = borderAreas[dpyNum][border].get_size();
    XFillRectangle(xvars.dpy[dpyNum],borderBuffers[dpyNum][border],
                   xvars.gc[dpyNum],
                   0,0,borderSize.width,borderSize.height);
  }
  XSetForeground(xvars.dpy[dpyNum],xvars.gc[dpyNum],xvars.black[dpyNum]);
}



void BorderTickRenderer::end_draw(int dpyNum) {
  // Whack corners onto window.
  for (int corner = 0; corner < CORNER_MAX; corner++) {
    XSetForeground(xvars.dpy[dpyNum],xvars.gc[dpyNum],
                   corners[dpyNum][corner]);
    Pos pos = cornerAreas[dpyNum][corner].get_pos();
    Size size = cornerAreas[dpyNum][corner].get_size();
    XFillRectangle(xvars.dpy[dpyNum],outer,xvars.gc[dpyNum],
                   pos.x,pos.y,size.width,size.height);
    // Revert GC to former state.
    XSetForeground(xvars.dpy[dpyNum],xvars.gc[dpyNum],xvars.black[dpyNum]);
  }

  // Whack borderBuffers onto window.
  for (int border = 0; border < BORDER_MAX; border++) {
    Pos pos = borderAreas[dpyNum][border].get_pos();
    Size size = borderAreas[dpyNum][border].get_size();
    XCopyArea(xvars.dpy[dpyNum],borderBuffers[dpyNum][border],outer,
              xvars.gc[dpyNum],
              0,0,size.width,size.height,
              pos.x,pos.y);
  }
}



Boolean BorderTickRenderer::draw_tick(TickType tt,CMN_DRAWABLE window,
                                      Xvars &,int dpyNum,
                                      const Size &arenaSize,
                                      Dir tickDir,int offset) {
  // Sanity check on the sizes passed around.
  assert((arenaSize.width == outerSize.width - 2 * TICK_BORDER_WIDTH) &&
         (arenaSize.height == outerSize.height - 2 * TICK_BORDER_WIDTH));
  assert(window == outer);

  int val;
  Boolean isBorder = dir_to_border_or_corner(val,tickDir);
  if (!isBorder) {
    // Set the value for the given corner.
    corners[dpyNum][val] = tickColors[dpyNum][tt];
  }
  else {
    // Fill in a small line on the apropriate borderBuffer.

    int offset2 = offset - TICK_BORDER_WIDTH - (TICK_WIDTH >> 1);
    XSetForeground(xvars.dpy[dpyNum],xvars.gc[dpyNum],
                   tickColors[dpyNum][tt]);
    switch (val) {
    case BORDER_R:
    case BORDER_L:
      XFillRectangle(xvars.dpy[dpyNum],borderBuffers[dpyNum][val],
                     xvars.gc[dpyNum],
                     0,offset2,TICK_BORDER_WIDTH,TICK_WIDTH);
      break;
    case BORDER_DN:
    case BORDER_UP:
      XFillRectangle(xvars.dpy[dpyNum],borderBuffers[dpyNum][val],
                     xvars.gc[dpyNum],
                     offset2,0,TICK_WIDTH,TICK_BORDER_WIDTH);
      break;
    default:
      assert(0);
    }
    XSetForeground(xvars.dpy[dpyNum],xvars.gc[dpyNum],
                   xvars.black[dpyNum]);
  }
  return True;
}



void BorderTickRenderer::init_x(Xvars& xvars,int dpyNum,
                                const Size& outerSize) {
  assert(!xvars.is_valid(xValid[dpyNum]));
  // Static initialization of xValid requires this to be true.
  //  assert(XVARS_VALID_INIT == 0);


  // Allocate colors.
  background[dpyNum] = xvars.alloc_named_color(dpyNum,ARENA_BORDER_COLOR);
  for (TickType tt = 0; tt < TICK_MAX; tt++) {
    tickColors[dpyNum][tt] = 
      xvars.alloc_named_color(dpyNum,tickColorNames[tt]);
  }


  // Compute areas of each border/corner.
  // Should have some helper in area to create areas from internal
  // touching dirs, or something like that.
  {
    Pos pos(outerSize.width - TICK_BORDER_WIDTH,
            TICK_BORDER_WIDTH);
    Size size;
    size.set(TICK_BORDER_WIDTH,
             outerSize.height - 2 * TICK_BORDER_WIDTH);
    Area area(pos,size);
    borderAreas[dpyNum][BORDER_R] = area;
  }
  {
    Pos pos(TICK_BORDER_WIDTH,
            outerSize.height - TICK_BORDER_WIDTH);
    Size size;
    size.set(outerSize.width - 2 * TICK_BORDER_WIDTH,
             TICK_BORDER_WIDTH);
    Area area(pos,size);
    borderAreas[dpyNum][BORDER_DN] = area;
  }
  {
    Pos pos(0,
            TICK_BORDER_WIDTH);
    Size size;
    size.set(TICK_BORDER_WIDTH,
             outerSize.height - 2 * TICK_BORDER_WIDTH);
    Area area(pos,size);
    borderAreas[dpyNum][BORDER_L] = area;
  }
  {
    Pos pos(TICK_BORDER_WIDTH,
            0);
    Size size;
    size.set(outerSize.width - 2 * TICK_BORDER_WIDTH,
             TICK_BORDER_WIDTH);
    Area area(pos,size);
    borderAreas[dpyNum][BORDER_UP] = area;
  }
  // Now corners.
  Size cornerSize;
  cornerSize.set(TICK_BORDER_WIDTH,TICK_BORDER_WIDTH);
  {
    Pos pos(outerSize.width - TICK_BORDER_WIDTH,
            outerSize.height - TICK_BORDER_WIDTH);
    Area area(pos,cornerSize);
    cornerAreas[dpyNum][CORNER_DN_R] = area;
  }
  {
    Pos pos(0,
            outerSize.height - TICK_BORDER_WIDTH);
    Area area(pos,cornerSize);
    cornerAreas[dpyNum][CORNER_DN_L] = area;
  }
  {
    Pos pos(0,
            0);
    Area area(pos,cornerSize);
    cornerAreas[dpyNum][CORNER_UP_L] = area;
  }
  {
    Pos pos(outerSize.width - TICK_BORDER_WIDTH,
            0);
    Area area(pos,cornerSize);
    cornerAreas[dpyNum][CORNER_UP_R] = area;
  }


  // Create borderBuffers.
  for (int border = 0; border < BORDER_MAX; border++) {
    Size size = borderAreas[dpyNum][border].get_size();
    borderBuffers[dpyNum][border] = 
      XCreatePixmap(xvars.dpy[dpyNum],xvars.root[dpyNum],
                    size.width,size.height,
                    xvars.depth[dpyNum]);
  }


  xvars.mark_valid(xValid[dpyNum]);
}



Boolean BorderTickRenderer::dir_to_border_or_corner(int& val,Dir dir) {
  switch (dir) {
  case CO_R:
    val = BORDER_R;
    return True;
  case CO_DN_R:
    val = CORNER_DN_R;
    return False;
  case CO_DN:
    val = BORDER_DN;
    return True;
  case CO_DN_L:
    val = CORNER_DN_L;
    return False;
  case CO_L:
    val = BORDER_L;
    return True;
  case CO_UP_L:
    val = CORNER_UP_L;
    return False;
  case CO_UP:
    val = BORDER_UP;
    return True;
  case CO_UP_R:
    val = CORNER_UP_R;
    return False;
  default:
    assert(0);
  }
  return False;;
}



XvarsValid BorderTickRenderer::xValid[Xvars::DISPLAYS_MAX] = {
  0,
  //  XVARS_VALID_INIT,
  // Assuming that XVARS_VALID_INIT == 0, this will automatically
  // initialize the rest of the values.
};



Area BorderTickRenderer::borderAreas[Xvars::DISPLAYS_MAX][BORDER_MAX];



Area BorderTickRenderer::cornerAreas[Xvars::DISPLAYS_MAX][CORNER_MAX];



Drawable BorderTickRenderer::borderBuffers[Xvars::DISPLAYS_MAX][BORDER_MAX];



Pixel BorderTickRenderer::corners[Xvars::DISPLAYS_MAX][CORNER_MAX];



Pixel BorderTickRenderer::tickColors[Xvars::DISPLAYS_MAX][TICK_MAX];



Pixel BorderTickRenderer::background[Xvars::DISPLAYS_MAX];



const char* BorderTickRenderer::tickColorNames[TICK_MAX] = {
  "yellow",
  "green",
  "red",
  "blue",
};



// Would be better to put this and draw_string() inside of Xvars.
// Size is window coordinates.
static void 
Viewport_draw_string_center(Xvars &xvars,int dpyNum,
                            Drawable drawable,const Size& size,
                            const char *msg) {
  assert(msg);  

  Size textSize;
  textSize.height = xvars.bigFont[dpyNum]->max_bounds.ascent 
    + xvars.bigFont[dpyNum]->max_bounds.descent;
  // get width
  if (xvars.bigFont[dpyNum]->per_char) {
    // variable width, go over all chars.
    textSize.width = 0;
    const char *p = msg;
    while (*p) {
      int index = *p -  xvars.bigFont[dpyNum]->min_char_or_byte2;
      textSize.width +=  xvars.bigFont[dpyNum]->per_char[index].width;
      p++;
    }
  }
  else {
    // Fixed witdth font, just multiply
    textSize.width = xvars.bigFont[dpyNum]->max_bounds.width * strlen(msg);
  }
  
  // Draw in center of the arena.
  Pos pos((size.width - textSize.width) / 2,
          (size.height  - textSize.height) / 2
          - xvars.stretch_y(ARENA_MESSAGE_OFF_CENTER));

  // Draw on black background in big font.
  XSetFont(xvars.dpy[dpyNum],xvars.gc[dpyNum],
           xvars.bigFont[dpyNum]->fid);
  XSetForeground(xvars.dpy[dpyNum],xvars.gc[dpyNum],
                 xvars.black[dpyNum]);
  XDrawString(xvars.dpy[dpyNum],drawable,
              xvars.gc[dpyNum],
              pos.x,pos.y + xvars.bigFont[dpyNum]->max_bounds.ascent,
              msg,strlen(msg));
  XSetForeground(xvars.dpy[dpyNum],xvars.gc[dpyNum],
                 xvars.arenaTextColor[dpyNum]);
  pos.x++;
  pos.y++;  
  XDrawString(xvars.dpy[dpyNum],drawable,
              xvars.gc[dpyNum],
              pos.x,pos.y + xvars.bigFont[dpyNum]->max_bounds.ascent,
              msg,strlen(msg));

  // Set back to default
  XSetForeground(xvars.dpy[dpyNum],xvars.gc[dpyNum],xvars.black[dpyNum]);
  XSetFont(xvars.dpy[dpyNum],xvars.gc[dpyNum],
           xvars.font[dpyNum]->fid);
}



class IScroller {
public:
  virtual ~IScroller();
  
  virtual void init(Xvars&,int dpyNum,Drawable arenaBG,
                    const Size& viewportSize) = 0;
  /* EFFECTS: Initialize any graphics needs of the IScroller. 
     viewportSize is in unstretched coordinates.  arenaBG is the 
     background window behind the arena, used for drawing tricks on its 
     border. */

  virtual Boolean move_to_pos(Area& viewportArea,const Pos& pos,
                              const Dim& innerDim,const Dim& extraDim) = 0;
  /* MODIFIES: vieportArea.set_pos() */
  /* REQUIRES: pos is a location in the world. */
  /* EFFECTS: Set the pos of viewportArea as necessary to follow an object
     centered at pos.  Return whether the viewport moved. */

  virtual void draw_arena(Window arena,Window arenaBG,WorldP,LocatorP,
                          const Area& viewportArea,IntelP intel,
                          Boolean forceDraw,Boolean reduceDraw,
                          const char* arenaMessage) = 0;
  /* REQUIRES: xvars and dpyNum are the same as the ones given to init. */
  /* EFFECTS: Draw the world and locator onto the window arena which 
     represents the area specified by viewportArea.  If forceDraw is true,
     must redraw everything.  If arenaMessage is non-NULL, draw it in the
     center of the arena. */

  virtual int get_border_width() = 0;
  /* NOTE: The Scroller owns this because it may or may want to draw tick
     marks in the arena border. */

  virtual void reset() = 0;
};



IScroller::~IScroller() {
}



class DiscreteScroller: public IScroller {
public:
  DiscreteScroller();

  virtual ~DiscreteScroller();
  
  virtual void init(Xvars&,int dpyNum,Drawable,const Size&);

  virtual Boolean move_to_pos(Area& viewportArea,const Pos& pos,
                              const Dim& innerDim,const Dim& extraDim);

  virtual void draw_arena(Window arena,Window arenaBG,WorldP,LocatorP,
                          const Area& viewportArea,IntelP intel,
                          Boolean forceDraw,Boolean reduceDraw,
                          const char* arenaMessage);
  /* EFFECTS: Just draw what has changed.  Don't scroll the background
     at a different rate than the foreground.  Draws tick marks in the 
     border. */

  virtual int get_border_width() {return TICK_BORDER_WIDTH;}

  virtual void reset();


private:
  enum {FLASH_TIME = 3};
  ITickRenderer* borderRenderer;
  Xvars* xvars;
  int dpyNum;
  Timer flashTimer;
};



DiscreteScroller::DiscreteScroller() {
  borderRenderer = NULL;
  flashTimer.set_max(FLASH_TIME);
  flashTimer.set();
}



DiscreteScroller::~DiscreteScroller() {
  delete borderRenderer;
}



void DiscreteScroller::init(Xvars& x_vars,int dpy_num,Drawable arenaBG,
                            const Size& viewportSize) {
  xvars = &x_vars;
  dpyNum = dpy_num;

  Size outerSize = xvars->stretch_size(viewportSize);
  outerSize.width += 2 * TICK_BORDER_WIDTH;
  outerSize.height += 2 * TICK_BORDER_WIDTH;

  borderRenderer = new BorderTickRenderer(arenaBG,*xvars,dpyNum,outerSize);
  assert(borderRenderer);
}



Boolean DiscreteScroller::move_to_pos(Area& viewportArea,const Pos& pos,
                                      const Dim& innerDim,
                                      const Dim& extraDim) {
  // Center the viewport on a chunk of this size.
  // The entire viewport has size 
  // innerDim + 2 * get_extra_dim()

  // The Loc containing pos.
  Loc l;
  l.set(pos.y / WSQUARE_HEIGHT,pos.x / WSQUARE_WIDTH);

  Pos oldPos  = viewportArea.get_pos();  

  // Scroll by rooms.
  Loc vLoc;
  vLoc.r = 
    innerDim.rowMax * (l.r / innerDim.rowMax) - extraDim.rowMax;
  vLoc.c = 
    innerDim.colMax * (l.c / innerDim.colMax) - extraDim.colMax;
  Pos newPos(vLoc.c * WSQUARE_WIDTH,vLoc.r * WSQUARE_HEIGHT);

  // No change.
  if (oldPos == newPos) {
    return False;
  }
  // Move to new position.
  else {
    viewportArea.set_pos(newPos);
    return True;
  }
}



void DiscreteScroller::draw_arena(Window arena,Window arenaBG,
                                  WorldP world,LocatorP locator,
                                  const Area& viewportArea,IntelP intel,
                                  Boolean forceDraw,
                                  Boolean,
                                  const char* arenaMessage) {
  // Redraw the entire visible portion of the World only if necessary.
  if (forceDraw) {
    // Don't use reduceDraw with Discrete scrolling.  If we wanted it to
    // work, we would have to make locator->draw_buffered take the 
    // reduceDraw argument.
    world->draw(arena,*xvars,dpyNum,viewportArea,False,False); 
  }

  // Get the viewportArea as a Box.
  Pos vPos = viewportArea.get_pos();
  Size vSize = viewportArea.get_size();
  // SmallViewport should never be scrolled in increments other than 
  // wsquares.
  assert(vPos.x % WSQUARE_WIDTH == 0 
         && vPos.y % WSQUARE_HEIGHT == 0);
  assert(vSize.width % WSQUARE_WIDTH == 0 
         && vSize.height % WSQUARE_HEIGHT == 0);
  Loc vLoc;
  vLoc.set(vPos.y / WSQUARE_HEIGHT,vPos.x / WSQUARE_WIDTH);
  Dim vDim(vSize.height / WSQUARE_HEIGHT,vSize.width / WSQUARE_WIDTH);
  Box viewportBox(vLoc,vDim);

  // Draw all visible objects.
  locator->draw_buffered(arena,*xvars,dpyNum,viewportBox);  

  flashTimer.clock();

  // Draw tick marks on the border around the arena.
  if (intel) {
    // Every few turns, don't draw the tick marks.
    if (flashTimer.ready()) {
      // Begin and end draw to erase border.
      borderRenderer->begin_draw(dpyNum);
      borderRenderer->end_draw(dpyNum);
      flashTimer.set();
    }
    else {
      locator->draw_ticks(arenaBG,*xvars,dpyNum,
                          viewportArea,intel->get_id(),
                          borderRenderer);
    }
  }

  if (arenaMessage) {
    Viewport_draw_string_center(*xvars,dpyNum,arena,
                                xvars->stretch_size(vSize),
                                arenaMessage);
  }
}



void DiscreteScroller::reset() {
  // This will clear out any tick marks.
  borderRenderer->begin_draw(dpyNum);
  borderRenderer->end_draw(dpyNum);
}



class SmoothScroller: public IScroller {
public:
  SmoothScroller();

  virtual ~SmoothScroller();

  virtual void init(Xvars&,int dpyNum,Drawable,const Size& viewportSize);

  virtual Boolean move_to_pos(Area& viewportArea,const Pos& pos,
                              const Dim& innerDim,const Dim& extraDim);
  /* EFFECTS: Smooth scrolling. */

  virtual void draw_arena(Window arena,Window arenaBG,WorldP,LocatorP,
                          const Area& viewportArea,IntelP intel,
                          Boolean forceDraw,Boolean reduceDraw,
                          const char* arenaMessage);
  /* EFFECTS: Redraw entire screen.  Scroll the background at a different
     rate than the foreground. */

  virtual int get_border_width() {return SMOOTH_BORDER_WIDTH;}

  virtual void reset() {}


private:
  Xvars* xvars;
  int dpyNum;

  // The double-buffer.
  Pixmap buffer;
};



SmoothScroller::SmoothScroller() {
}



void SmoothScroller::init(Xvars& x_vars,int dpy_num,Drawable,
                          const Size& viewportSize) {
  xvars = &x_vars;
  dpyNum = dpy_num;

  // In window coordinates, does not include border.
  Size arenaSize = xvars->stretch_size(viewportSize);
  
  if (Viewport::get_use_buffer()) {
    // Create back-buffer the size of the arena, without the border.
    buffer = XCreatePixmap(xvars->dpy[dpyNum],
                           xvars->root[dpyNum],
                           arenaSize.width,
                           arenaSize.height,
                           xvars->depth[dpyNum]);
  }
}



SmoothScroller::~SmoothScroller() {
  if (Viewport::get_use_buffer()) {
    XFreePixmap(xvars->dpy[dpyNum],buffer);
  }
}



Boolean SmoothScroller::move_to_pos(Area& viewportArea,const Pos& pos,
                                    const Dim&,const Dim&) {
  Pos posOld = viewportArea.get_pos();
  Pos posNew = pos - 0.5f * viewportArea.get_size();
  viewportArea.set_pos(posNew);

  return !(posOld == posNew);
}



void SmoothScroller::draw_arena(Window arena,Window,
                                WorldP world,LocatorP locator,
                                const Area& viewportArea,IntelP intel,
                                Boolean,Boolean reduceDraw,
                                const char* arenaMessage) {
  Boolean useBuffer = Viewport::get_use_buffer();
  Pixmap dest = (useBuffer ? buffer : arena);

  // Window coordinates.
  Size arenaInner = xvars->stretch_size(viewportArea.get_size());
    
  // Only need to fill with black if not drawing the background.
  if (reduceDraw) {
    XFillRectangle(xvars->dpy[dpyNum],dest,xvars->gc[dpyNum],
                   0,0,arenaInner.width,arenaInner.height);
  }

  // Draw world on the back-buffer.  Will redraw entire world each turn.
  world->draw(dest,*xvars,dpyNum,viewportArea,reduceDraw,True);

  // Draw all objects directly to dest.
  locator->draw_directly(dest,*xvars,dpyNum,viewportArea);

  // Draw tick marks on margins of screen.
  if (intel) {
    // Use locator as the implementation of ITickRenderer.  This uses 
    // the XPM tick mark images.
    locator->draw_ticks(dest,*xvars,dpyNum,
                        viewportArea,intel->get_id(),
                        locator);
  }

  if (arenaMessage) {
    Viewport_draw_string_center(*xvars,dpyNum,dest,
                                arenaInner,
                                arenaMessage);
  }

  // Whack back-buffer onto window.
  if (useBuffer) {
    XCopyArea(xvars->dpy[dpyNum],buffer,arena,xvars->gc[dpyNum],
              0,0,
              arenaInner.width,
              arenaInner.height,
              0,0);
  }
}



Viewport::Viewport(int arg_c,char** arg_v,
                   Xvars& x_vars,int dNum,
                   WorldP wrld,LocatorP lctr,
                   Boolean smoothScroll,int mNum,
                   IDifficultyCallback* diffCB,
                   IStyleInfo* styInfo,
                   IKeyObserver* kObserver,
                   //                   IChatObserver* cObserver,
                   const DifficultyLevel dLevels[DIFFICULTY_LEVELS_NUM],
                   RoleType rType,
                   ViewportCallback callbacks[VIEWPORT_CB_NUM],
                   void* clos)
: argc(arg_c),
  argv(arg_v),
  xvars(x_vars), 
  dpyNum(dNum), 
  world(wrld), 
  locator(lctr), 
  menusNum(mNum) {
  intel = NULL;
  
  roleType = rType;

  input = UI_INPUT_NONE;

  cursorDefined = False;
  arenaMessage = NULL;
  redrawArena = True;
  promptDifficulty = False;

  keyState = new KeyState();
  keyDispatcher = new KeyDispatcher();
  assert(keyDispatcher);  

  diffCallback = diffCB;
  styleInfo = styInfo;
  keyObserver = kObserver;
  //  chatObserver = cObserver;

  difficultyLevels = dLevels;

  // Create PanelClosures
  // NOTE: Creating one for menuStyle even though it is not used.
  for (int n = 0; n < VIEWPORT_CB_NUM; n++) {
    PanelClosure* p = new PanelClosure;
    assert(p);
    // Certain menu items must enforce radio-button behavior.
    if (n == menuScenarios ||
        n == menuLevels || 
        n == menuKill ||
        n == menuDuel ||
        n == menuExtended ||
        n == menuTraining) {
      p->radio = True;
    }
    else {
      p->radio = False;
    }
    p->viewport = this;
    // The viewport callback.
    p->callback = callbacks[n];
    panelClosures.add((void*)p);
  }
  // The one closure for all ViewportCallbacks.
  closure = clos;


  if (smoothScroll) {
    scroller = new SmoothScroller();
  }
  else {
    scroller = new DiscreteScroller();
  }
  assert(scroller);
}
 


void Viewport::init() {
  // Call all the virtual functions here that we aren't allowed to call
  // in the constructor.  Is there a hard-core C++ guru out there who 
  // knows a better way of doing this?

  // Dimensions of viewport are slightly larger than the innerDim.
  Dim viewportDim = get_inner_dim();
  Dim extraDim = get_extra_dim();
  viewportDim.colMax += 2 * extraDim.colMax;
  viewportDim.rowMax += 2 * extraDim.rowMax;

  // Start alligned with upper-left of world.
  Pos pos(0,0);
  Size size;
  size.set(viewportDim);
  Area vArea(pos,size);
  viewportArea = vArea;
  
  if (!sizeValid[dpyNum]) {
    init_sizes(xvars,dpyNum,world);
  }
  
  create_toplevel();

  scroller->init(xvars,dpyNum,arenaBG,viewportArea.get_size());
}



Viewport::~Viewport() {
  assert (panelClosures.length() == VIEWPORT_CB_NUM);
  int n;
  for (n = 0; n < panelClosures.length(); n++) {
    PanelClosure* p = (PanelClosure*)panelClosures.get(n);
    delete p;
  }
  
  Utils::freeif(arenaMessage);
  delete keyDispatcher;
  delete keyState;
  
  for (n = 0; n < menusNum; n++) {
    delete menus[n];
  }
  for (n = 0; n < VW_STATUSES_NUM; n++) {
    delete statuses[n];
  }
  delete humansPlaying;
  delete enemiesPlaying;
  delete messageBar;
  delete level;

  delete scroller;
  
  XDestroyWindow(xvars.dpy[dpyNum],toplevel);
}
 
 

void Viewport::set_menu_controls(Boolean val) {
  assert(menusNum > menuControls);
  ((TogglePanel*)menus[menuControls])->set_value(val);
}



Boolean Viewport::get_menu_learn_controls() {
  assert(menusNum > menuLearnControls);
  return ((TogglePanel*)menus[menuLearnControls])->get_value();  
}



void Viewport::set_menu_learn_controls(Boolean val) {
  assert(menusNum > menuLearnControls);
  ((TogglePanel*)menus[menuLearnControls])->set_value(val);
}
 


void Viewport::set_menu_humans_num(int val) {
  assert(menusNum > menuHumansNum);
  ostrstream str;
  str << val << ends;
  ((WritePanel *)menus[menuHumansNum])->set_value(str.str());
  delete str.str();
}
 


void Viewport::set_menu_enemies_num(int val) {
  assert(menusNum > menuEnemiesNum);
  ostrstream str;
  str << val << ends;
  ((WritePanel *)menus[menuEnemiesNum])->set_value(str.str());
  delete str.str();
}



void Viewport::set_menu_help(Boolean val) {
  assert(menusNum > menuHelp);
  ((TogglePanel*)menus[menuHelp])->set_value(val);
}



void Viewport::set_enemies_refill(Boolean val) {
  ((TogglePanel *)menus[menuEnemiesRefill])->set_value(val);
}



void Viewport::set_style_and_role_type(GameStyleType style,RoleType roleType) {
  assert(menusNum == VW_MENUS_PRIMARY_NUM);

  // Set value of radio buttons.
  ((TogglePanel *)menus[menuScenarios])->set_value(style == SCENARIOS);
  ((TogglePanel *)menus[menuLevels])->set_value(style == LEVELS);
  ((TogglePanel *)menus[menuKill])->set_value(style == KILL);
  ((TogglePanel *)menus[menuDuel])->set_value(style == DUEL);
  ((TogglePanel *)menus[menuExtended])->set_value(style == EXTENDED);
  ((TogglePanel *)menus[menuTraining])->set_value(style == TRAINING);

  // EnemiesNum
  menus[menuEnemiesNum]->
    set_sensitive(Role::uses_enemies_num(roleType) && 
                  GameStyle::uses_enemies_num(style));

  // EnemiesRefill
  menus[menuEnemiesRefill]->
    set_sensitive(Role::uses_enemies_refill(roleType) && 
                  GameStyle::uses_enemies_refill(style));

  // GameStyle
  Boolean enabled = Role::uses_game_style(roleType);
  menus[menuLevels]->set_sensitive(enabled);
  menus[menuScenarios]->set_sensitive(enabled);
  menus[menuKill]->set_sensitive(enabled);
  menus[menuDuel]->set_sensitive(enabled);
  menus[menuExtended]->set_sensitive(enabled);
  menus[menuTraining]->set_sensitive(enabled);
  
  // HumansNum
  menus[menuHumansNum]->set_sensitive(Role::uses_humans_num(roleType));

  // Cooperative
  menus[menuCooperative]->set_sensitive(Role::uses_cooperative(roleType));
}



void Viewport::set_quanta(Quanta quanta) {
  assert(menusNum > menuQuanta);
  ostrstream str;
  str << quanta << ends;
  ((WritePanel *)menus[menuQuanta])->set_value(str.str());
  delete str.str();  
} 



void Viewport::set_cooperative(Boolean val) {
  assert(menusNum > menuCooperative);
  ((TogglePanel *)menus[menuCooperative])->set_value(val);
}



void Viewport::set_humans_playing(int val) {  
  ostrstream msg;
  if (val == 1) {
    msg << "1 Human" << ends;
  }
  else {
    msg << val << " Humans" << ends;
  }
  char *msg_str = msg.str();
  
  humansPlaying->set_message(msg_str);
  delete msg_str;
}



void Viewport::set_enemies_playing(int val) {  
  ostrstream msg;
  if (val == 1) {
    msg << "1 Enemy" << ends; 
  }
  else {
    msg << val << " Enemies" << ends; 
  }
  char *msg_str = msg.str();
  
  enemiesPlaying->set_message(msg_str);
  delete msg_str;
}



const char* Viewport::get_level() {
  return level->get_message();
}



void Viewport::set_level(const char* val) {  
  level->set_message(val);
}



void Viewport::set_input(UIinput inpt) {
  assert(inpt != UI_INPUT_NONE);
  input = inpt; 
}



void Viewport::set_message(const char* message) {
  messageBar->set_message(message);
}



void Viewport::set_arena_message(const char* msg,Quanta time) {
  Utils::freeif(arenaMessage);
  arenaMessage = Utils::strdup(msg);
  arenaMessageTimer.set(time);
  redrawArena = True;
}



void Viewport::register_intel(int humanColorNum,IntelP intl) {  
  assert(intl);  
  intel = intl;

  // Set colors of status panels to be those of the human.
  if (intel->is_human()) {
    Pixel pixel = xvars.humanColors[dpyNum][humanColorNum];
    for (int num = 0; num < VW_STATUSES_NUM; num++) {
      statuses[num]->set_foreground(pixel);
    }
  }

}



void Viewport::clear_all() {
  XClearWindow(xvars.dpy[dpyNum],arena);
  for (int mm = 0; mm < menusNum; mm++) {
    menus[mm]->clear();
  }
  for (int s = 0; s < VW_STATUSES_NUM; s++) {
    statuses[s]->clear();
  }
  humansPlaying->clear();
  enemiesPlaying->clear();
  messageBar->clear();
  level->clear();
}



void Viewport::unclear_all() {
  redrawArena = True;
  for (int mm = 0; mm < menusNum; mm++) {
    menus[mm]->redraw();
  }
  for (int s = 0; s < VW_STATUSES_NUM; s++) {
    statuses[s]->redraw();
  }
  humansPlaying->redraw();
  enemiesPlaying->redraw();
  messageBar->redraw();
  level->redraw();
}



void Viewport::set_prompt_difficulty(Boolean val) {
  promptDifficulty = val;
  redrawArena = True;
}



Boolean Viewport::has_window(Window w) {
  // Check toplevel and arena.
  if (toplevel == w || arena == w) {
    return True;
  }

  // Check menus.
  int m;
  for (m = 0; m < menusNum; m++) {
    if (menus[m]->get_window() == w) {
      return True;
    }
  }
    
  // Check statuses.
  for (m = 0; m < VW_STATUSES_NUM; m++) {
    if (statuses[m]->get_window() == w) {
      return True;
    }
  }
  
  if (humansPlaying->get_window() == w) {
    return True;
  }
  
  if (enemiesPlaying->get_window() == w) {
    return True;
  }
  
  if (messageBar->get_window() == w) {
    return True;
  }
    
  if (level->get_window() == w) {
    return True;
  }
  
  return False;
}



void Viewport::reset() {
  intel = NULL;
  redrawArena = True;
  promptDifficulty = False;
  Utils::freeif(arenaMessage);  // Actual message being displayed.
  for (int m = 0; m < VW_STATUSES_NUM; m++) {
    statuses[m]->set_message("");
    statuses[m]->set_foreground(xvars.black[dpyNum]);
  }
  Dim extraDim = get_extra_dim();

  messageBar->set_chat(False);

  Pos zeroPos;
  viewportArea.set_pos(zeroPos);

  scroller->reset();
}



void Viewport::process_event(XEvent* event) {
  assert(has_window(event->xany.window));
  
  // Put this first becase messageBar wants to capture all key events
  // for chat mode.
  if (messageBar->process_event(dpyNum,event)) {
    return;
  }
  
  // Events on the toplevel or arena.
  if (event->xany.window == toplevel ||
      event->xany.window == arena) {
    switch(event->type) {
    case Expose:
      expose(event);
      break;
    case KeyPress:
      key_press(event);
      break;
    case KeyRelease:
      key_release(event);
      break;
    case ButtonPress:
      button_press(event);
      break;
    default:
      cerr << "Warning: Unexpected event of type " << event->type 
           << endl;
      break;
    }
    return;
  }


  // Send to all menus/statuses/etc.
  int m;
  for (m = 0; m < menusNum; m++) {
    if (menus[m]->process_event(dpyNum,event)) {
      return;
    }
  }
  
  for (m = 0; m < VW_STATUSES_NUM; m++) {
    if (statuses[m]->process_event(dpyNum,event)) {
      return;
    }
  }
  
  if (humansPlaying->process_event(dpyNum,event)) {
    return;
  }
    
  if (enemiesPlaying->process_event(dpyNum,event)) {
    return;
  }
    
  if (level->process_event(dpyNum,event)) {
    return;
  }
}



IViewportInfo* Viewport::get_info() {
  if (!vInfoProvider) {
    vInfoProvider = new VInfoProvider();
    assert(vInfoProvider);
  }
  return vInfoProvider;
}



void Viewport::init_viewport_info(Boolean isStretched,Boolean smoothScroll) {
  // Make sure provider has been created.
  VInfoProvider* provider = (VInfoProvider*)get_info();

  if (isStretched) {
    provider->set_viewport_info(LargeViewport::get_info(smoothScroll));
  }
  else {
    provider->set_viewport_info(SmallViewport::get_info(smoothScroll));
  }
}



// Implement IDispatcher.
void Viewport::dispatch(ITcommand command,void*) {
  if (!acceptInput) {
    return;
  }
  
  // Can chat even if no associated human.  For Server in observer mode.
  if (command == IT_CHAT) {
    // Only enable Chat mode if the current Role supports it.
    if (Role::uses_chat(styleInfo->get_role_type())) {
      messageBar->set_chat(True);
    }
    return;
  }

  // If there is an intel associated with the viewport, give command to it.
  if (intel && intel->is_playing()) {  
    // Only give the command to a human.
    if (intel->is_human()) {
      ((HumanP)intel)->set_command(command);
    }
    return;
  }


  // No associated intel, so scroll with the keyset. 
  Boolean changed;
  switch (command) {
  // Right.
  case IT_R:
    changed = shift_viewport(COL_SHIFT,0);
    break;
  // Down right.
  case IT_DN_R:
    changed = shift_viewport(COL_SHIFT,ROW_SHIFT);
    break;
  // Down.
  case IT_DN:
    changed = shift_viewport(0,ROW_SHIFT);
    break;
  // Down left.
  case IT_DN_L:
    changed = shift_viewport(-COL_SHIFT,ROW_SHIFT);
    break;
  // Left.
  case IT_L:
    changed = shift_viewport(-COL_SHIFT,0);
    break;
  // Up left.
  case IT_UP_L:
    changed = shift_viewport(-COL_SHIFT,-ROW_SHIFT);
    break;
  // Up.
  case IT_UP:
    changed = shift_viewport(0,-ROW_SHIFT);
    break;
  // Up right.
  case IT_UP_R:
    changed = shift_viewport(COL_SHIFT,-ROW_SHIFT);
    break;
  default:
    changed = False;
  }
  
  if (changed) {
    redrawArena = True;
  }
}



void Viewport::pre_clock() {
  // Before check for death in follow_intel().
  update_statuses();
  
  follow_intel();

  draw();
      
  arenaMessageTimer.clock();

  // Clear arenaMessage being displayed if time is up.
  if (arenaMessage && arenaMessageTimer.ready()) {
    Utils::freeif(arenaMessage);
    // Clean up the message next time.
    redrawArena = True;
  }
}



void Viewport::post_clock() {
  // Want this immediately after the events have been pulled from the 
  // queue and before Locator::clock().
  if (input != UI_INPUT_NONE) {
    keyDispatcher->clock(keyState,this,NULL);
  }
}



void Viewport::expose(XEvent*) {  
  if (!cursorDefined) {
    XDefineCursor(xvars.dpy[dpyNum],arena,xvars.arenaCursor[dpyNum]);
    cursorDefined = True;
  }
  // Mark dirty to force a redraw.
  redrawArena = True;
  draw();
}



// Helper macros for Viewport::key_press().
#define NUM_KEY_EQ(keySym) \
  ((event->xkey.keycode == XKeysymToKeycode(xvars.dpy[dpyNum],XK_ ## keySym)) || \
   (event->xkey.keycode == XKeysymToKeycode(xvars.dpy[dpyNum],XK_KP_ ## keySym)))

void Viewport::key_press(XEvent *event) {
  // If waiting for user to enter difficulty, don't accept anything else.
  if (promptDifficulty) {
    // Should be DIFFICULTY_LEVELS_NUM of these.  It'd be nice to 
    // abstract this more.
    if (NUM_KEY_EQ(0)) {
      diffCallback->change_difficulty(0);
    }
    else if (NUM_KEY_EQ(1)) {
      diffCallback->change_difficulty(1);
    }
    else if (NUM_KEY_EQ(2)) {
      diffCallback->change_difficulty(2);
    }
    else if (NUM_KEY_EQ(3)) {
      diffCallback->change_difficulty(3);
    }
    
    if (event->xkey.keycode == 
        XKeysymToKeycode(xvars.dpy[dpyNum],XK_space)) {
      diffCallback->change_difficulty(DIFF_NORMAL);
    }
    return;
  }

  // The observer will translate the keycode into a key and send it
  // back to Viewport::receive_key.  Note that it may send it to 
  // receive_key on a different viewport.
  keyObserver->key_event(dpyNum,event->xkey.keycode,True);
}



void Viewport::key_release(XEvent *event) {     
  if (promptDifficulty) {
    return;
  }
  keyObserver->key_event(dpyNum,event->xkey.keycode,False);
}



void Viewport::receive_key(int key,Boolean down) {
  keyState->set(key,down);
}



void Viewport::button_press(XEvent *event) {
  if (promptDifficulty) {
    diffCallback->change_difficulty(DIFF_NORMAL);
    return;
  }

// Disable the rudimentary mouse controls, they just confuse the user.
#if 0  
  // Button1 moves your character.
  if (event->xbutton.button == Button1) {
    dispatch(Intel::dir_to_command(button_press_to_dir(event)),NULL);
  }
  // Button2 fires weapon.
  else if (event->xbutton.button == Button2) {
    dispatch(Intel::dir_to_command_weapon(button_press_to_dir(event)),NULL);
  }
  // Button3 changes weapon.
  else if (event->xbutton.button == Button3) {
    dispatch(IT_WEAPON_CHANGE,NULL);
  }
#endif
}



void Viewport::init_sizes(Xvars& xvars,int dpyNum,WorldP) {
  Dim innerDim = get_inner_dim();
  Dim extraDim = get_extra_dim();
  int border = scroller->get_border_width();

  innerSize[dpyNum].set(xvars.stretch_x(innerDim.colMax * WSQUARE_WIDTH),
                        xvars.stretch_y(innerDim.rowMax * WSQUARE_HEIGHT));

  arenaSize[dpyNum].width = innerSize[dpyNum].width 
    + xvars.stretch_x(2 * WSQUARE_WIDTH * extraDim.colMax)
    + 2 * border;
  arenaSize[dpyNum].height = innerSize[dpyNum].height 
    + xvars.stretch_y(2 * WSQUARE_HEIGHT * extraDim.rowMax)
    + 2 * border;
}



void Viewport::panel_callback(Panel* panel,void* value,void* closure) {
  PanelClosure* pClosure = (PanelClosure*)closure;
  assert(pClosure);

  // Enforce radio button behavior for certain panels.
  // Make sure you can't uncheck the active button.
  // The logic to unselect the active button when another one is set is
  // in Viewport::set_style().
  //
  // Would be much better to put the radio-button logic in the Panel classes.
  if (pClosure->radio) {
    // SGI compiler says you can't cast void* to Boolean.
    Boolean bValue = (Boolean)(int)value;
    if (!bValue) {
      ((TogglePanel*)panel)->set_value(True);
    }
  }

  // Call the appropriate callback registered in the Viewport constructor
  // with the closure given to the Viewport constructor.
  pClosure->callback(value,pClosure->viewport,
                     pClosure->viewport->closure);
}



Dir Viewport::button_press_to_dir(XEvent *event) {
  assert(event->type == ButtonPress);
  
  if (event->xbutton.x < arenaSize[dpyNum].width * 0.3333) {
    if (event->xbutton.y < arenaSize[dpyNum].height * 0.3333) {
      return CO_UP_L;
    }
    else if (event->xbutton.y < arenaSize[dpyNum].height * 0.6666) {
      return CO_L;
    }
    else {
      return CO_DN_L;
    }
  }
  else if (event->xbutton.x < arenaSize[dpyNum].width * .6666) {
    if (event->xbutton.y < arenaSize[dpyNum].height * 0.3333) {
      return CO_UP;
    }
    else if (event->xbutton.y < arenaSize[dpyNum].height * 0.6666) {
      return CO_air;
    }
    else {
      return CO_DN;
    }
  }
  else {
    if (event->xbutton.y < arenaSize[dpyNum].height * 0.3333) {
      return CO_UP_R;
    }
    else if (event->xbutton.y < arenaSize[dpyNum].height * 0.6666) {
      return CO_R;
    }
    else {
      return CO_DN_R;
    }
  }
}


 
void Viewport::draw() {
  // Just draw some text in the middle of the arena.
  if (promptDifficulty) {
    if (redrawArena) {
      XClearWindow(xvars.dpy[dpyNum],arena);
      
      // prompt user for level of difficulty.
      XSetForeground(xvars.dpy[dpyNum],xvars.gc[dpyNum],
                     xvars.red[dpyNum]);
      
      Pos pos(xvars.fontSize[dpyNum].width,xvars.fontSize[dpyNum].height);
      draw_string(pos,"Enter level of difficulty:");
      pos.y += xvars.fontSize[dpyNum].height;
      
      for (int n = 0; n < DIFFICULTY_LEVELS_NUM; n++) {
        ostrstream str;
        str << "[" << n;
        if (n == DIFF_NORMAL) {
          str << ",space]  ";
        }
        else {
          str << "]        ";
        }
        str << difficultyLevels[n].name << ends;
        pos.y += xvars.fontSize[dpyNum].height;
        draw_string(pos,str.str());
        delete str.str();
      }
      
      XSetForeground(xvars.dpy[dpyNum],xvars.gc[dpyNum],
                     xvars.black[dpyNum]);
    }
  }

  // Normal drawing.
  else {
    // Draw world and all objects.
    scroller->draw_arena(arena,arenaBG,world,locator,viewportArea,intel,
                         redrawArena,reduceDraw,arenaMessage);
  }  
  
  // Mark as not dirty.  We just brought everything up to date.
  redrawArena = False;
}



void Viewport::draw_string(const Pos &pos,char *msg) {
  XDrawString(xvars.dpy[dpyNum],arena,
              xvars.gc[dpyNum],
              pos.x,pos.y + xvars.font[dpyNum]->max_bounds.ascent,
              msg,strlen(msg));
}



Boolean Viewport::shift_viewport(int cols,int rows) {
  Pos shift(cols * WSQUARE_WIDTH,rows * WSQUARE_HEIGHT);

  const Dim worldDim = world->get_dim();

  Pos pos = viewportArea.get_pos();
  Size size = viewportArea.get_size();
  if (shift.x > 0) {
    if (pos.x + shift.x < 
        worldDim.colMax * WSQUARE_WIDTH) {
      pos.x += shift.x;
    }
  }
  if (shift.x < 0) {
    if (pos.x + size.width + shift.x > 0) {
      pos.x += shift.x;
    }
  }
  if (shift.y > 0) {
    if (pos.y + shift.y < 
        worldDim.rowMax * WSQUARE_HEIGHT) {
      pos.y += shift.y;
    }
  }
  if (shift.y < 0) {
    if (pos.y + size.height + shift.y > 0) {
      pos.y += shift.y;
    }
  }

  // See if we actually changed pos.
  if (!(pos == viewportArea.get_pos())) {
    viewportArea.set_pos(pos);
    return True;
  }

  return False;
}



void Viewport::create_toplevel() {
  const char* title = NULL;
  switch (roleType) {
  case R_STAND_ALONE:
    title = "XEvil";
    break;
  case R_CLIENT:
    title = "XEvil [Client]";
    break;
  case R_SERVER:
    title = "XEvil [Server]";
    break;
  default:
    assert(0);
  }
  
  toplevel = 
    xvars.create_toplevel_window(argc,argv,dpyNum,viewportSize[dpyNum],
                                 title,KeyPressMask | KeyReleaseMask);

#if 0
  toplevel = 
    XCreateSimpleWindow(xvars.dpy[dpyNum],xvars.root[dpyNum],0,0,
                        viewportSize[dpyNum].width,
                        viewportSize[dpyNum].height,
                        0,xvars.windowBorder[dpyNum],
                        xvars.windowBg[dpyNum]);

  XSizeHints size_hints;
  size_hints.flags = PPosition | PSize | PMinSize | PMaxSize;
  size_hints.min_width = viewportSize[dpyNum].width;
  size_hints.min_height = viewportSize[dpyNum].height;
  size_hints.max_width = viewportSize[dpyNum].width;
  size_hints.max_height = viewportSize[dpyNum].height;

  XTextProperty windowName, iconName;
  char *window_name = "XEvil";  // Will appear on window.
  char *icon_name = "XEvil";
  Status stat = XStringListToTextProperty(&window_name,1,&windowName);
  assert(stat);
  stat = XStringListToTextProperty(&icon_name,1,&iconName);
  assert(stat);
  
  XWMHints wm_hints;
  wm_hints.initial_state = NormalState;
  wm_hints.input = True;
  wm_hints.icon_pixmap = xvars.icon[dpyNum];
  wm_hints.flags = StateHint | IconPixmapHint | InputHint;

  XClassHint class_hints;
  class_hints.res_name = argv[0];
  class_hints.res_class = "XEvil";

  XSetWMProperties(xvars.dpy[dpyNum],toplevel,
                   &windowName,&iconName,argv,argc,
                   &size_hints,&wm_hints,&class_hints);

  XSelectInput(xvars.dpy[dpyNum],toplevel,
               KeyPressMask | KeyReleaseMask);
  
  // Add WM_DELETE_WINDOW protocol
  XChangeProperty(xvars.dpy[dpyNum],toplevel,
                  xvars.wmProtocols[dpyNum],XA_ATOM,
                  32,PropModePrepend,
                  (unsigned char *)&xvars.wmDeleteWindow[dpyNum],
                  1);
#endif

  create_menus();
  create_arena();
  create_statuses();
  create_intels_playing();
  create_message_bar();
  create_level();

  XMapWindow(xvars.dpy[dpyNum],arena);
  XMapWindow(xvars.dpy[dpyNum],arenaBG);
  XMapWindow(xvars.dpy[dpyNum],toplevel);
}



void Viewport::create_menus() {
  if (!menusNum) {
    return;
  }
  Pixel menuBg = xvars.alloc_named_color(dpyNum,MENU_BG_COLOR);
  Panel *p = NULL;

  
  // Compute sizes for all menus.
  Size quitUnit = ButtonPanel::get_unit(xvars.font[dpyNum],QUIT_LINE_LENGTH);
  Size newGameUnit = 
    ButtonPanel::get_unit(xvars.font[dpyNum],NEW_GAME_LINE_LENGTH);
  Size humansNumUnit = 
    WritePanel::get_unit(xvars.font[dpyNum],HUMANS_NUM_LINE_LENGTH);
  Size enemiesNumUnit = 
    WritePanel::get_unit(xvars.font[dpyNum],ENEMIES_NUM_LINE_LENGTH);
  Size enemiesRefillUnit = 
    TogglePanel::get_unit(xvars.font[dpyNum],ENEMIES_REFILL_LINE_LENGTH);
  Size controlsUnit =
    TogglePanel::get_unit(xvars.font[dpyNum],CONTROLS_LINE_LENGTH);
  Size learnControlsUnit =
    TogglePanel::get_unit(xvars.font[dpyNum],LEARN_CONTROLS_LINE_LENGTH);

  Size styleUnit = 
    TogglePanel::get_unit(xvars.font[dpyNum],STYLE_LINE_LENGTH);
  Size scenariosUnit = 
    TogglePanel::get_unit(xvars.font[dpyNum],SCENARIOS_LINE_LENGTH);
  Size levelsUnit = 
    TogglePanel::get_unit(xvars.font[dpyNum],LEVELS_LINE_LENGTH);
  Size killUnit = TogglePanel::get_unit(xvars.font[dpyNum],KILL_LINE_LENGTH);
  Size duelUnit = TogglePanel::get_unit(xvars.font[dpyNum],DUEL_LINE_LENGTH);
  Size extendedUnit = 
    TogglePanel::get_unit(xvars.font[dpyNum],EXTENDED_LINE_LENGTH);
  Size trainingUnit = 
    TogglePanel::get_unit(xvars.font[dpyNum],TRAINING_LINE_LENGTH);
  Size quantaUnit = 
    WritePanel::get_unit(xvars.font[dpyNum],QUANTA_LINE_LENGTH);
  Size cooperativeUnit = 
    WritePanel::get_unit(xvars.font[dpyNum],COOPERATIVE_LINE_LENGTH);
  Size helpUnit = 
    TextPanel::get_unit(xvars.font[dpyNum],HELP_LINE_LENGTH);


  // Create actual menu panels.
  // Menus created at (0,0) for Large and Small Viewports.
  Pos pos(0,0);

  if (menusNum == VW_MENUS_PRIMARY_NUM) {
    assert(dpyNum == 0);
    
    // Quit button.
    p = menus[menuQuit] = 
      new ButtonPanel(dpyNum,xvars,toplevel,
                      pos,quitUnit,
                      Viewport::panel_callback,panelClosures.get(menuQuit),
                      "Quit");
    assert(p);
    p->set_background(menuBg,False);
    pos.x += quitUnit.width;
    
    // New Game button.
    p = menus[menuNewGame] = 
      new ButtonPanel(dpyNum,xvars,toplevel,
                      pos,newGameUnit,
                      Viewport::panel_callback,panelClosures.get(menuNewGame),
                      "New Game");
    assert(p);
    p->set_background(menuBg,False);
    pos.x += newGameUnit.width;
    
    // Humans WritePanel
    p = menus[menuHumansNum] = 
      new WritePanel(dpyNum,xvars,toplevel,
                     pos,humansNumUnit,
                     Viewport::panel_callback,panelClosures.get(menuHumansNum),
                     "Humans:");
    assert(p);
    p->set_background(menuBg,False);
    pos.x += humansNumUnit.width;
    
    // Enemies WritePanel
    p = menus[menuEnemiesNum] = 
      new WritePanel(dpyNum,xvars,toplevel,
                     pos,enemiesNumUnit,
                     Viewport::panel_callback,panelClosures.get(menuEnemiesNum),
                     "Enemies:");
    assert(p);
    p->set_background(menuBg,False);
    pos.x += enemiesNumUnit.width;
    
    // Enemies Refill TogglePanel
    p = menus[menuEnemiesRefill] = 
      new TogglePanel(dpyNum,xvars,toplevel,
                      pos,enemiesRefillUnit,
                      Viewport::panel_callback,
                      panelClosures.get(menuEnemiesRefill),
                      "Regen Enemies");
    assert(p);
    p->set_background(menuBg,False);
    pos.x += enemiesRefillUnit.width;
  }
  else {
    assert(menusNum == VW_MENUS_SECONDARY_NUM);
  }
  
  // Learn Controls ButtonPanel
  p = menus[menuLearnControls] = 
    new TogglePanel(dpyNum,xvars,toplevel,
                    pos,learnControlsUnit,
                    Viewport::panel_callback,
                    panelClosures.get(menuLearnControls),
                    "Set Controls");
  assert(p);
  p->set_background(menuBg,False);
  pos.x += learnControlsUnit.width;
  
  // Controls TogglePanel
  p = menus[menuControls] = 
    new TogglePanel(dpyNum,xvars,toplevel,
                    pos,controlsUnit,
                    Viewport::panel_callback,
                    panelClosures.get(menuControls),
                    "Show Controls");
  assert(p);
  p->set_background(menuBg,False);
  pos.x += controlsUnit.width;
  
  
  // speed now in first row.
  if (menusNum == VW_MENUS_PRIMARY_NUM) {
    p = menus[menuQuanta] = 
      new WritePanel(dpyNum,xvars,toplevel,
                     pos,quantaUnit,
                     Viewport::panel_callback,panelClosures.get(menuQuanta),
                     "Speed(ms):");
    assert(p);
    p->set_background(menuBg,False);
    pos.x += quantaUnit.width;
  }

  
  // Second row.
  pos.x = 0;
  pos.y += quitUnit.height;
  
  if (menusNum == VW_MENUS_PRIMARY_NUM) {
    // Game stlye header.
    p = menus[menuStyle] = 
      new TextPanel(dpyNum,xvars,toplevel,
                    pos,styleUnit,
                    NULL,NULL,
                    "Game style:");
    assert(p);
    //      p->set_background(menuBg,False);
    pos.x += styleUnit.width;
      
    // Levels game style.
    p = menus[menuLevels] = 
      new TogglePanel(dpyNum,xvars,toplevel,
                      pos,levelsUnit,
                      Viewport::panel_callback,panelClosures.get(menuLevels),
                      "Levels");
    assert(p);
    p->set_background(menuBg,False);
    pos.x += levelsUnit.width;
    
    // Scenarios game style.
    p = menus[menuScenarios] = 
      new TogglePanel(dpyNum,xvars,toplevel,
                      pos,scenariosUnit,
                      Viewport::panel_callback,
                      panelClosures.get(menuScenarios),
                      "Scenarios");
    assert(p);
    p->set_background(menuBg,False);
    pos.x += scenariosUnit.width;
    
    // Kill, Kill, Kill game style.
    p = menus[menuKill] = 
      new TogglePanel(dpyNum,xvars,toplevel,
                      pos,killUnit,
                      Viewport::panel_callback,panelClosures.get(menuKill),
                      "Kill, Kill, Kill");
    assert(p);
    p->set_background(menuBg,False);
    pos.x += killUnit.width;
    
    // Duel game style.
    p = menus[menuDuel] = 
      new TogglePanel(dpyNum,xvars,toplevel,
                      pos,duelUnit,
                      Viewport::panel_callback,panelClosures.get(menuDuel),
                      "Duel");
    assert(p);
    p->set_background(menuBg,False);
    pos.x += duelUnit.width;
    
    // Extended duel game style.
    p = menus[menuExtended] = 
      new TogglePanel(dpyNum,xvars,toplevel,
                      pos,extendedUnit,
                      Viewport::panel_callback,panelClosures.get(menuExtended),
                      "Extended Duel");
    assert(p);
    p->set_background(menuBg,False);
    pos.x += extendedUnit.width;
    
    // Training game style.
    p = menus[menuTraining] = 
      new TogglePanel(dpyNum,xvars,toplevel,
                      pos,trainingUnit,
                      Viewport::panel_callback,
                      panelClosures.get(menuTraining),
                      "Training");
    assert(p);
    p->set_background(menuBg,False);
    pos.x += trainingUnit.width;
    
    // Cooperative mode.
    pos.x = viewportSize[dpyNum].width - cooperativeUnit.width 
      - helpUnit.width;
    p = menus[menuCooperative] = 
      new TogglePanel(dpyNum,xvars,toplevel,
                      pos,cooperativeUnit,
                      Viewport::panel_callback,
                      panelClosures.get(menuCooperative),
                      "Cooperative");
    assert(p);
    p->set_background(menuBg,False);

    // Help button.
    pos.x = viewportSize[dpyNum].width - helpUnit.width;
    p = menus[menuHelp] = 
      new TogglePanel(dpyNum,xvars,toplevel,
                      pos,helpUnit,
                      Viewport::panel_callback,
                      panelClosures.get(menuHelp),
                      "Help");
    assert(p);
    p->set_background(menuBg,False);
  }
}



void Viewport::create_arena() {
  // The background window behind arena, used for drawing tick marks.
  arenaBG =
    XCreateSimpleWindow(xvars.dpy[dpyNum],
                        toplevel,
                        0,menusSize[dpyNum].height,
                        arenaSize[dpyNum].width,
                        arenaSize[dpyNum].height,
                        0,
                        xvars.alloc_named_color(dpyNum,
                                                ARENA_BORDER_COLOR,
                                                xvars.black[dpyNum]),
                        xvars.alloc_named_color(dpyNum,
                                                ARENA_BORDER_COLOR,
                                                xvars.black[dpyNum]));
  // arenaBG doesn't really need any events.


  int border = scroller->get_border_width();

  // Create arena just inside arenaBG.
  arena =
    XCreateSimpleWindow(xvars.dpy[dpyNum],
                        arenaBG,
                        border,border,
                        arenaSize[dpyNum].width - 2 * border,
                        arenaSize[dpyNum].height - 2 * border,
                        0,
                        xvars.alloc_named_color(dpyNum,
                                                Xvars_BACKGROUND),
                        xvars.alloc_named_color(dpyNum,
                                                Xvars_BACKGROUND));
  XSelectInput(xvars.dpy[dpyNum],arena,
               ExposureMask | ButtonPressMask);
}



void Viewport::create_intels_playing() {
  Pos pos = intelsPlayingPos[dpyNum];
  Size oneIntelPlaying = intelsPlayingSize[dpyNum];
  oneIntelPlaying.height /= 2;

  humansPlaying = 
    new TextPanel(dpyNum,xvars,toplevel,
                  pos,oneIntelPlaying);
  assert(humansPlaying);

  pos.y += oneIntelPlaying.height;
  enemiesPlaying = 
    new TextPanel(dpyNum,xvars,toplevel,
                  pos,oneIntelPlaying);
  assert(enemiesPlaying);
}



void Viewport::create_message_bar() {
  messageBar = new ChatPanel(dpyNum,xvars,toplevel,
                             messageBarPos[dpyNum],messageBarSize[dpyNum],
                             Viewport::panel_callback,
                             panelClosures.get(stChat));
  assert(messageBar);
}



// Need better name.  This refers to the status item that displays 
// level-specific information.
void Viewport::create_level() {
  level = new TextPanel(dpyNum,xvars,toplevel,
                        levelPos[dpyNum],levelSize[dpyNum]);
  assert(level);
}



void Viewport::update_statuses() {
  // See if we actually need to do anything.
  if (!intel || !intel->intel_status_changed()) {
    return;
  }

  const IntelStatus *status = intel->get_intel_status();
  
  ostrstream name;
  name << status->name << ends;
  statuses[statusName]->set_message(name.str());
  delete name.str();

  ostrstream className;
  className << status->className << ends;
  statuses[statusClassName]->set_message(className.str());
  delete className.str();
  
  ostrstream health;
  if (status->health == -1) {
    health << "Dead" << ends;
  }
  else {
    health << status->health << " Health" << ends; 
  }
  statuses[statusHealth]->set_message(health.str());
  delete health.str();
  
  ostrstream mass;
  mass << status->mass << " Mass" << ends;
  statuses[statusMass]->set_message(mass.str());
  delete mass.str();
  
  ostrstream weapon;
  if (status->weaponClassId == A_None) {
    weapon << "No Weapon";
  }
  else {
    weapon << status->weapon;
  }
  if (status->ammo != PH_AMMO_UNLIMITED) {
    weapon << " (" << status->ammo << ")";
  }
  weapon << ends;
  statuses[statusWeapon]->
    set_foreground(status->weaponReady ? 
                   xvars.green[dpyNum] : xvars.red[dpyNum],False);
  statuses[statusWeapon]->set_message(weapon.str());
  delete weapon.str();
  
  ostrstream item;
  if (status->itemClassId == A_None) {
    item << "No Item";
  }
  else {
    item << status->item << " (" << status->itemCount << ")";
  }
  item << ends;
  statuses[statusItem]->set_message(item.str());
  delete item.str();
  
  ostrstream livesHKills;
  if (styleInfo->get_game_style_type() == EXTENDED) {
    // Takes soups into account.
    livesHKills << (status->humanKills - status->soups) 
                << " Human Kills" << ends;
  }
  else {
    if (status->lives == IT_INFINITE_LIVES) {
      // Does this case ever happen?  I don't think so.
      livesHKills << "Unlimited Lives" << ends;
    }
    else {
      if (status->lives == 1) {
        livesHKills << "1 Life" << ends;
      }
      else {
        livesHKills << status->lives << " Lives" << ends;
      }
    }
  }
  statuses[statusLivesHKills]->set_message(livesHKills.str());
  delete livesHKills.str();
  
  ostrstream killsMKills;
  if (styleInfo->get_game_style_type() == EXTENDED) {
    killsMKills << status->enemyKills << " Machine Kills" << ends;
  }
  else {
    int kills = status->humanKills + status->enemyKills;
    if (kills == 1) {
      killsMKills << "1 Kill" << ends;
    }
    else {
      killsMKills << kills << " Kills" << ends;
    }
  }
  statuses[statusKillsMKills]->set_message(killsMKills.str());
  delete killsMKills.str();
}



void Viewport::follow_intel() {
  if (intel) {
    // Only follow if playing.
    if (intel->is_playing()) {
      PhysicalP p;
      if (p = locator->lookup(intel->get_id())) {
        const Area &a = p->get_area();
        if (scroller->move_to_pos(viewportArea,a.get_middle(),
                                  get_inner_dim(),get_extra_dim())) {
          redrawArena = True;
        }
      }
    }
#if 0
    // As good a place as any to check if the intel is dead.
    else {
      // Unregister the Intel.
      intel = NULL;
    }
#endif
  }
}



Boolean Viewport::acceptInput = False;



VInfoProvider* Viewport::vInfoProvider = NULL;



Boolean Viewport::reduceDraw = False;



Boolean Viewport::useBuffer = True;



Boolean Viewport::sizeValid[Xvars::DISPLAYS_MAX] = {
  0,
  // Compiler will initialize the rest of the values to zero.
};



Size Viewport::menusSize[Xvars::DISPLAYS_MAX];



Size Viewport::innerSize[Xvars::DISPLAYS_MAX];



Size Viewport::arenaSize[Xvars::DISPLAYS_MAX];



Size Viewport::statusesSize[Xvars::DISPLAYS_MAX];



Pos Viewport::intelsPlayingPos[Xvars::DISPLAYS_MAX];



Size Viewport::intelsPlayingSize[Xvars::DISPLAYS_MAX];



Pos Viewport::messageBarPos[Xvars::DISPLAYS_MAX];



Size Viewport::messageBarSize[Xvars::DISPLAYS_MAX];



Pos Viewport::levelPos[Xvars::DISPLAYS_MAX];



Size Viewport::levelSize[Xvars::DISPLAYS_MAX];



Size Viewport::viewportSize[Xvars::DISPLAYS_MAX];



SmallViewport::SmallViewport(int argc,char** argv,
                             Xvars& xvars,int dpyNum,
                             WorldP w,LocatorP l,
                             Boolean smoothScroll,int menusNum,
                             IDifficultyCallback* diffCB,
                             IStyleInfo* styleInfo,
                             IKeyObserver* keyObserver,
                             //                             IChatObserver* chatObserver,
                             const DifficultyLevel 
                             dLevels[DIFFICULTY_LEVELS_NUM],
                             RoleType roleType,
                             ViewportCallback callbacks[VIEWPORT_CB_NUM],
                             void* closure) 
: Viewport(argc,argv,xvars,dpyNum,w,l,
           smoothScroll,menusNum,diffCB,styleInfo,
           keyObserver,dLevels,roleType,callbacks,closure) {
}



Dim SmallViewport::get_inner_dim() {
  Dim ret(SM_ROW_MAX,SM_COL_MAX);
  return ret;
}



Dim SmallViewport::get_extra_dim() {
  Dim ret(SM_EXTRA_ROW,SM_EXTRA_COL);
  return ret;
}



void SmallViewport::init_sizes(Xvars& xvars,int dpyNum,WorldP world) {
  Viewport::init_sizes(xvars,dpyNum,world);

  // Statuses first to get width of viewport.
  statusesSize[dpyNum] = 
    TextPanel::get_unit(xvars.font[dpyNum],SM_STATUS_LINE_LENGTH);
  // Minus 1 because health and mass on the same line and plus .5 to
  // leave a little space.
  statusesSize[dpyNum].height = 
    (int) (statusesSize[dpyNum].height * (VW_STATUSES_NUM - .75));

  // Menus next, needed for intelsPlayingPos.
  Size oneMenuUnit = 
    ButtonPanel::get_unit(xvars.font[dpyNum],1);
  menusSize[dpyNum].height = 2 * oneMenuUnit.height;
  menusSize[dpyNum].width = arenaSize[dpyNum].width
    + statusesSize[dpyNum].width;
  
  // Intels Playing, two of them.
  intelsPlayingPos[dpyNum].x = arenaSize[dpyNum].width;
  intelsPlayingPos[dpyNum].y = 
    menusSize[dpyNum].height + statusesSize[dpyNum].height;
  intelsPlayingSize[dpyNum] = 
    TextPanel::get_unit(xvars.font[dpyNum],SM_STATUS_LINE_LENGTH);
  intelsPlayingSize[dpyNum].height *= 2;

  // Level-specific info, flush with bottom of arena.
  levelSize[dpyNum] = 
    TextPanel::get_unit(xvars.font[dpyNum],SM_STATUS_LINE_LENGTH,2);
  levelPos[dpyNum].x = arenaSize[dpyNum].width;
  levelPos[dpyNum].y = 
    menusSize[dpyNum].height + arenaSize[dpyNum].height 
    - levelSize[dpyNum].height;
    
  // Message bar at bottom.
  messageBarPos[dpyNum].x = 0;
  messageBarPos[dpyNum].y = 
    menusSize[dpyNum].height + arenaSize[dpyNum].height;
  messageBarSize[dpyNum] = TextPanel::get_unit(xvars.font[dpyNum],1,2);
  messageBarSize[dpyNum].width = arenaSize[dpyNum].width 
    + statusesSize[dpyNum].width;

  // Entire size of viewport.
  viewportSize[dpyNum].width = arenaSize[dpyNum].width
    + statusesSize[dpyNum].width;
  viewportSize[dpyNum].height = menusSize[dpyNum].height 
    + arenaSize[dpyNum].height + messageBarSize[dpyNum].height;
}



void SmallViewport::create_statuses() {
  Pos pos(arenaSize[dpyNum].width,menusSize[dpyNum].height);
  Size statusUnit;

  for (int n = 0; n < VW_STATUSES_NUM; n++) {
    if (n == statusWeapon) {
	    statusUnit = 
	      TextPanel::get_unit(xvars.font[dpyNum],SM_STATUS_LINE_LENGTH);
	    statuses[n] = 
	      new ButtonPanel(dpyNum,xvars,toplevel,
                          pos,statusUnit,
                          Viewport::panel_callback,
                          panelClosures.get(stWeapon));
	  }
    else if (n == statusItem) {
	    statusUnit = 
	      TextPanel::get_unit(xvars.font[dpyNum],SM_STATUS_LINE_LENGTH);
	    statuses[n] = 
	      new ButtonPanel(dpyNum,xvars,toplevel,
                          pos,statusUnit,
                          Viewport::panel_callback,
                          panelClosures.get(stItem));
	  }
    else if (n == statusHealth) {
	    statusUnit = 
	      TextPanel::get_unit(xvars.font[dpyNum],
                              SM_STATUS_LINE_LENGTH / 2);
	    statuses[n] = 
	      new TextPanel(dpyNum,xvars,
                        toplevel,
                        pos,statusUnit);
	  }
    else if (n == statusMass) {
	    statusUnit = 
	      TextPanel::get_unit(xvars.font[dpyNum],SM_STATUS_LINE_LENGTH);
	    Size otherStatusUnit = 
	      TextPanel::get_unit(xvars.font[dpyNum],SM_STATUS_LINE_LENGTH / 2);
	    Pos p(pos.x + otherStatusUnit.width,pos.y);
	    Size s;
	    s.width = statusUnit.width - otherStatusUnit.width;
	    s.height = statusUnit.height;
	    statuses[n] = 
	      new TextPanel(dpyNum,xvars,
                        toplevel,p,s);
	  }
    else {
	    statusUnit = 
	      TextPanel::get_unit(xvars.font[dpyNum],SM_STATUS_LINE_LENGTH);
	    statuses[n] = 
	      new TextPanel(dpyNum,xvars,toplevel,
                        pos,statusUnit);
	  }
    
    assert(statuses[n]);
    if (n != statusHealth) {
      pos.y += statusUnit.height;
    }
  }
}


 
ViewportInfo SmallViewport::get_info(Boolean smoothScroll) {
  if (smoothScroll) {
    Size visible;
    visible.set((SM_COL_MAX + 2 * SM_EXTRA_COL) * WSQUARE_WIDTH,
                (SM_ROW_MAX + 2 * SM_EXTRA_ROW) * WSQUARE_WIDTH);
    ViewportInfo ret(IT_VISION_RANGE,visible);
    return ret;
  }
  else {
    Size logicalSize;
    logicalSize.set(SM_COL_MAX * WSQUARE_WIDTH,
                    SM_ROW_MAX * WSQUARE_HEIGHT);
    Size extraSize;
    extraSize.set(SM_EXTRA_COL * WSQUARE_WIDTH,
                  SM_EXTRA_ROW * WSQUARE_HEIGHT);
    
    // Non-smooth scrolling.
    ViewportInfo ret(IT_VISION_RANGE,logicalSize,extraSize);
    return ret;
  }
}



LargeViewport::LargeViewport(int argc,char** argv,
                             Xvars& xvars,int dpyNum,
                             WorldP w,LocatorP l,
                             Boolean smoothScroll,int menusNum,
                             IDifficultyCallback* diffCB,
                             IStyleInfo* styleInfo,
                             IKeyObserver* keyObserver,
                             //                             IChatObserver* chatObserver,
                             const DifficultyLevel 
                             dLevels[DIFFICULTY_LEVELS_NUM],
                             RoleType roleType,
                             ViewportCallback callbacks[VIEWPORT_CB_NUM],
                             void* closure) 
: Viewport(argc,argv,xvars,dpyNum,w,l,smoothScroll,
           menusNum,diffCB,styleInfo,
           keyObserver,dLevels,roleType,callbacks,closure) {
}



Dim LargeViewport::get_inner_dim() {
  Dim ret(LG_ROW_MAX,LG_COL_MAX);
  return ret;
}



Dim LargeViewport::get_extra_dim() {
  Dim ret(LG_EXTRA_ROW,LG_EXTRA_COL);
  return ret;
}



void LargeViewport::init_sizes(Xvars& xvars,int dpyNum,WorldP world) {
  Viewport::init_sizes(xvars,dpyNum,world);

  // Menus.
  Size oneMenuUnit = 
    ButtonPanel::get_unit(xvars.font[dpyNum],1);
  menusSize[dpyNum].width = arenaSize[dpyNum].width;
  menusSize[dpyNum].height = 2 * oneMenuUnit.height;

  // Get height of 2 statuses.
  Size statusUnit = 
    TextPanel::get_unit(xvars.font[dpyNum],1,1);
  statusUnit.height *= 2;

  // Level-specific info.
  levelSize[dpyNum] = 
    TextPanel::get_unit(xvars.font[dpyNum],LG_LEVEL_LINE_LENGTH,2);
  levelSize[dpyNum].height = statusUnit.height;
  levelPos[dpyNum].x = arenaSize[dpyNum].width - levelSize[dpyNum].width;
  levelPos[dpyNum].y = menusSize[dpyNum].height + arenaSize[dpyNum].height;

  // Status rectangles.
  statusesSize[dpyNum].height = statusUnit.height;
  statusesSize[dpyNum].width = arenaSize[dpyNum].width 
    - levelSize[dpyNum].width;

  // Intels playing.
  intelsPlayingSize[dpyNum] = 
    TextPanel::get_unit(xvars.font[dpyNum],LG_INTELS_LINE_LENGTH);
  intelsPlayingSize[dpyNum].height *= 2;
  intelsPlayingPos[dpyNum].x = 
    arenaSize[dpyNum].width - intelsPlayingSize[dpyNum].width;
  intelsPlayingPos[dpyNum].y = levelPos[dpyNum].y + levelSize[dpyNum].height;

  // Message bar at bottom.
  messageBarPos[dpyNum].x = 0;
  messageBarPos[dpyNum].y = intelsPlayingPos[dpyNum].y;
  messageBarSize[dpyNum].width = intelsPlayingPos[dpyNum].x;
  messageBarSize[dpyNum].height = intelsPlayingSize[dpyNum].height;

  viewportSize[dpyNum].width = arenaSize[dpyNum].width;
  viewportSize[dpyNum].height = 
    menusSize[dpyNum].height 
    + arenaSize[dpyNum].height 
    + levelSize[dpyNum].height
    + intelsPlayingSize[dpyNum].height;
}



void LargeViewport::create_statuses() {
  Pos pos(0,menusSize[dpyNum].height + arenaSize[dpyNum].height);  
  Size statusUnit[4];
  int n;
  for (n = 0; n < 4; n++) {
    statusUnit[n].height = statusesSize[dpyNum].height / 2;
  }
  statusUnit[0].width = (int)(0.29 * statusesSize[dpyNum].width);
  statusUnit[1].width = (int)(0.29 * statusesSize[dpyNum].width);
  statusUnit[2].width = (int)(0.18 * statusesSize[dpyNum].width);
  statusUnit[3].width = statusesSize[dpyNum].width - statusUnit[0].width
    - statusUnit[1].width - statusUnit[2].width;
                 

  // First row.
  n = 0;
  statuses[statusWeapon] = 
    new ButtonPanel(dpyNum,xvars,toplevel,
                    pos,statusUnit[n],
                    Viewport::panel_callback,
                    panelClosures.get(stWeapon));
  pos.x += statusUnit[n++].width;

  statuses[statusName] = 
    new TextPanel(dpyNum,xvars,toplevel,
                  pos,statusUnit[n]);
  pos.x += statusUnit[n++].width;

  statuses[statusHealth] = 
    new TextPanel(dpyNum,xvars,toplevel,
                  pos,statusUnit[n]);
  pos.x += statusUnit[n++].width;

  statuses[statusLivesHKills] = 
    new TextPanel(dpyNum,xvars,toplevel,
                  pos,statusUnit[n]);
  pos.x = 0;
  n = 0;
  pos.y += statusUnit[n].height;


  // Second row.
  statuses[statusItem] = 
    new ButtonPanel(dpyNum,xvars,toplevel,
                    pos,statusUnit[n],
                    Viewport::panel_callback,
                    panelClosures.get(stItem));
  pos.x += statusUnit[n++].width;
  
  statuses[statusClassName] = 
    new TextPanel(dpyNum,xvars,toplevel,
                  pos,statusUnit[n]);
  pos.x += statusUnit[n++].width;

  statuses[statusMass] = 
    new TextPanel(dpyNum,xvars,toplevel,
                  pos,statusUnit[n]);
  pos.x += statusUnit[n++].width;

  statuses[statusKillsMKills] = 
    new TextPanel(dpyNum,xvars,toplevel,
                  pos,statusUnit[n]);
}



ViewportInfo LargeViewport::get_info(Boolean smoothScroll) {
  if (smoothScroll) {
    Size visible;
    visible.set((LG_COL_MAX + 2 * LG_EXTRA_COL) * WSQUARE_WIDTH,
                (LG_ROW_MAX + 2 * LG_EXTRA_ROW) * WSQUARE_WIDTH);
    ViewportInfo ret(IT_VISION_RANGE,visible);
    return ret;
  }
  else {
    Size logicalSize;
    logicalSize.set(LG_COL_MAX * WSQUARE_WIDTH,
                    LG_ROW_MAX * WSQUARE_HEIGHT);
    Size extraSize;
    extraSize.set(LG_EXTRA_COL * WSQUARE_WIDTH,
                  LG_EXTRA_ROW * WSQUARE_HEIGHT);
    
    // Non-smooth scrolling.
    ViewportInfo ret(IT_VISION_RANGE,logicalSize,extraSize);
    return ret;
  }
}


