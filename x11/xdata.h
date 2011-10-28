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

// "xdata.h" Window system dependent classes.

#ifndef XDATA_H
#define XDATA_H

#ifndef NO_PRAGMAS
#pragma interface
#endif

// Include Files
extern "C"
{
#include <X11/X.h>
#include <X11/Xlib.h>
}
#include "utils.h"
#include "coord.h"

#define Xvars_WINDOW_BG_COLOR "gray70"
#define Xvars_WINDOW_BORDER_COLOR "grey40"


typedef Boolean XvarsValid;
#define XVARS_VALID_INIT False;


#define UI_KEYS_MAX IT_WEAPON_R


enum UIinput {UI_KEYS_RIGHT,UI_KEYS_LEFT,UI_INPUT_NONE};



// Only command on UNIX is init graphics.
enum IXCommand {IX_INIT};



typedef Drawable CMN_DRAWABLE;



// For Physical::get_pixmap_mask().
struct CMN_IMAGEDATA {
  Pixmap pixmap;
  Pixmap mask;
};



// Pointer to xbm bits (or resource id on win32)
typedef char *CMN_BITS_ID;

typedef XEvent *CMN_EVENTDATA;

typedef char *CMN_COLOR;



///////// Xvars
#define Xvars_BACKGROUND "light grey"
typedef unsigned long Pixel;


class Xvars {
public:
  enum {
    DISPLAYS_MAX = 6,
    DISPLAY_NAME_LENGTH = 80,
	HUMAN_COLORS_NUM = 6
  };

  Window create_toplevel_window(int argc,char** argv,
                                int dpyNum,const Size&,
                                const char* title,
                                long eventMask);
  /* EFFECTS: Create a new top-level window of given size 
     (in window coordinates).  title will be
     displayed in the window manager decoration and when the window is
     iconified.  eventMask gives the set of events the window is interested
     in, NULL for no events.  New window will receive WM_DELETE_WINDOW
     client messages.  The new window will ask the window manager to
     never be resized. */
  /* NOTE: Does not map window. */

  Pixel alloc_named_color(int displayNum,const char *name,
                          Pixel def = (Pixel)-1) const;
  /* EFFECTS: Tries to allocate the named color using the current values
     of the Xvars.  Returns the Pixel if successful.  Otherwise, returns 
     default.  If called with only 2 arguments, default 
     is white.  If black and white display, returns default. */

  Boolean is_valid(XvarsValid check) {return check;}
  /* EFFECTS: Check whether some graphics-user is valid against the current
     Xvars. */
  /* NOTE: On UNIX this is the identity function. */

  void mark_valid(XvarsValid& val) {val = True;}
  /* MODIFIES: val */
  /* EFFECTS: Set val to be valid. */

  void generate_pixmap_from_transform(int dpyNum,Drawable dest,Drawable src,
                                      const Size& srcSize,
                                      Drawable scratch,
                                      const TransformType* transforms,
                                      int tNum,
                                      int depth);
  /* REQUIRES: scratch is at least as big as the max of src and dest.
     depth is the bit depth of src and dest. */
  /* EFFECTS: Generate one pixmap from another based on some 
     transformations.  Use scratch for all scratch calculations. */

  Boolean load_pixmap(Drawable* pixmap,Drawable* mask,
                      int dpyNum,char** xpmBits);
  /* MODIFIES: pixmap, mask */
  /* EFFECTS: Load an XPM pixmap and mask from xpmBits.  Only load mask if 
     non-NULL.  Reduce pixmap size by a factor of 2 if 
     !xvars.is_stretched(). */

  Boolean load_pixmap(Drawable* pixmap,Drawable* mask,
                      int dpyNum,char** xpmBits,Boolean fullSize);
  /* EFFECTS: Same as above load_pixmap() except caller explicitly 
     specifies whether pixmap is fullSize or reduced by a factor of 2. */
  /* NOTE: This method can be called before Xvars::stretch is set. */

  static void set_use_averaging(Boolean val) {useAveraging = val;}
  /* EFFECTS: Set whether or not to use averaging to generate the 
     reduced pixmaps.  If False, use subsampling. */
  

  // Stretch methods.
  Boolean is_stretched() {return stretch == 2;}
  /* EFFECTS:  If true, use the stretched coordinate system, and large 
     pixmaps. Else, use unstretched coordinate system and small 
     pixmaps. */

  int stretch_x(int val) {return stretch * val;}
  int stretch_y(int val) {return stretch * val;}
  inline Pos stretch_pos(const Pos& pos);
  inline Size stretch_size(const Size& size);
  Area stretch_area(const Area& area);

  int stretch;

  // Total number of allocated displays.
  int dpyMax; 
  Display *dpy[DISPLAYS_MAX];
  Screen *scr_ptr[DISPLAYS_MAX];
  int scr_num[DISPLAYS_MAX];
  Window root[DISPLAYS_MAX];
  Visual *visual[DISPLAYS_MAX];
  int depth[DISPLAYS_MAX];
  Colormap cmap[DISPLAYS_MAX];
  // background/foreground. 
  Pixel white[DISPLAYS_MAX],black[DISPLAYS_MAX]; 
  Pixel red[DISPLAYS_MAX],
    green[DISPLAYS_MAX],
    arenaTextColor[DISPLAYS_MAX]; /* Default to black. */
  Pixel humanColors[DISPLAYS_MAX][HUMAN_COLORS_NUM];
  Pixel windowBg[DISPLAYS_MAX];
  Pixel windowBorder[DISPLAYS_MAX];
  /* Default gc except foreground is black, background is white, no
     graphics exposures, font is set, stipple is suitable for drawing
     insensitive areas, fill style is FillSolid */
  GC gc[DISPLAYS_MAX]; 
  XFontStruct *font[DISPLAYS_MAX];
  Size fontSize[DISPLAYS_MAX];
  XFontStruct *bigFont[DISPLAYS_MAX];
  Size bigFontSize[DISPLAYS_MAX];

  static const char *humanColorNames[HUMAN_COLORS_NUM];

  // For when the windows are iconified
  Pixmap iconPixmap[Xvars::DISPLAYS_MAX];
  Pixmap iconMask[Xvars::DISPLAYS_MAX];
  
  // Used to get the WM_DELETE message from the window manager.
  Atom wmProtocols[Xvars::DISPLAYS_MAX];
  Atom wmDeleteWindow[Xvars::DISPLAYS_MAX];

  // Empty cursor so it doesn't distract the user.
  Cursor arenaCursor[Xvars::DISPLAYS_MAX];


private:
  void gen_pix_from_trans(int dpyNum,Drawable dest,Drawable src,
                          const Size& srcSize,TransformType transform,
                          int depth);
  /* REQUIRES: dest is large enough for the transformed version of src. */
  /* EFFECTS: Helper for generate_pixmap_from_transform.  Do one 
     transformation, from source to dest. */

  void unstretch_image(int dpyNum,XImage* dest,XImage* src,
                       Pixel* pixels,int pixelsNum);
  /* REQUIRES: dest is an image exactly half the width and height of src.
     src != dest */
  /* EFFECTS: Reduce size of src image, copying it to dest.  src is 
     unchanged.  pixels is the list of pixels in src, has length nPixels. 
     Can pass in NULL,0 for pixels,nPixels for a mask. */

  int color_match(XColor* color,XColor* palette,int paletteNum);
  /* EFFECTS: Match RGB values in color to the list of colors in palette.
     Return index into palette of closest color. */

  char* new_bytes_for_image(const Size&,int depth,int bitmap_pad);
  /* EFFECTS: Allocate (with new) the appropriate number of bytes needed 
     for an X Image. */

  void destroy_image(XImage*);
  /* EFFECTS: Destroy an XImage and the memory that was allocated with
     new_bytes_for_image().  Don't use this to destroy an image whose data 
     came from XpmCreateImageFromData() or anything else other than
     new_bytes_for_image(). */

  void put_image(int dpyNum,Drawable dest,XImage* src,const Size&);
  /* EFFECTS: XPutImage src up to dest.  Will work properly even if
     src is a mask. */


  static Boolean useAveraging;
};



inline Pos Xvars::stretch_pos(const Pos& pos) {
  Pos ret(pos.x * stretch,pos.y * stretch);
  return ret;
}



inline Size Xvars::stretch_size(const Size& size) {
  Size ret;
  ret.set(size.width * stretch,size.height * stretch);
  return ret;
}



struct Wxdata {
  // Background color.
  Pixel background[Xvars::DISPLAYS_MAX]; 
  // Blocks.
  Pixmap blockPixmaps[Xvars::DISPLAYS_MAX][W_ALL_BLOCKS_NUM];
  Pixmap blockMasks[Xvars::DISPLAYS_MAX][W_ALL_BLOCKS_NUM]; // not all used.
  // Posters.
  Pixmap posterPixmaps[Xvars::DISPLAYS_MAX][W_ALL_POSTERS_NUM];  
  Pixmap posterMasks[Xvars::DISPLAYS_MAX][W_ALL_POSTERS_NUM];  
  // Door, top and bottom.
  Pixmap doorPixmaps[Xvars::DISPLAYS_MAX][2];
#ifdef W_DOORS_TRANSPARENT
  Pixmap doorMasks[Xvars::DISPLAYS_MAX][2];
#endif
  // Horizontal and vertical mover squares.
  Pixmap moverSquarePixmaps[Xvars::DISPLAYS_MAX][W_ALL_MOVER_SQUARES_NUM];
  Pixmap moverSquareMasks[Xvars::DISPLAYS_MAX][W_ALL_MOVER_SQUARES_NUM];
  // Mover itself.
  Pixmap moverPixmaps[Xvars::DISPLAYS_MAX][W_ALL_MOVERS_NUM];
  Pixmap moverMasks[Xvars::DISPLAYS_MAX][W_ALL_MOVERS_NUM];
  // The backgrounds, behind the players and world blocks.
  Pixmap backgroundPixmaps[Xvars::DISPLAYS_MAX][W_ALL_BACKGROUNDS_NUM];
  // The outsides, graphics drawn outside the world.
  Pixmap outsidePixmaps[Xvars::DISPLAYS_MAX][W_ALL_OUTSIDES_NUM];  
};



struct OLxdata {
  Pixmap buffer[Xvars::DISPLAYS_MAX];
  Pixmap scratchBuffer[Xvars::DISPLAYS_MAX];
  Pixmap tickPixmaps[Xvars::DISPLAYS_MAX][TICK_MAX][CO_DIR_HALF_PURE];
  Pixmap tickMasks[Xvars::DISPLAYS_MAX][TICK_MAX][CO_DIR_HALF_PURE];
};



class FireXdata {
public:
  FireXdata() {valid = XVARS_VALID_INIT;}

  Pixmap pixmap[Xvars::DISPLAYS_MAX];
  Pixmap mask[Xvars::DISPLAYS_MAX];
  XvarsValid valid;
}; 



class ProtectionXdata {
public:
  ProtectionXdata() {valid = XVARS_VALID_INIT;}

  Pixel color[Xvars::DISPLAYS_MAX];
  XvarsValid valid;
};



class MovingXdata {
public:
  MovingXdata() {valid = XVARS_VALID_INIT; offsetsValid = False;}
  
  XvarsValid valid;
  Pixmap pixmaps[Xvars::DISPLAYS_MAX][CO_DIR_MAX][PH_ANIM_MAX],
  masks[Xvars::DISPLAYS_MAX][CO_DIR_MAX][PH_ANIM_MAX];

  // Hack, This snippet of data is completely independent of init_x(), etc.
  Size offsets[CO_DIR_MAX];  
  Boolean offsetsValid;
};



// Handle the black magic to be a UNIX Daemon.
class Daemon {
public:
  // First, just record the file name.
  Daemon(const char* fName);

  // Close out anything necessary.
  ~Daemon();

  const char* get_file_name();

  // 1) fork new process
  // 2) New session id
  // 3) change working directory to /tmp
  // 4) redirect all output from stdout and stderr to logfile
  void go();


private:
  void daemonize();

  const char* fname;
  int fd;
};

#endif
