/*
 * XEvil(TM) Copyright (C) 1994,2000 Steve Hardt and Michael Judge
 * http://www.xevil.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.  See gpl.txt.
 */

// "xdata.h"  SDL port -- window-system dependent classes.
//
// This is the SDL analogue of x11/xdata.h.  cmn/ includes it by bare name
// (resolved via -I$(DEPTH)/sdl) and consumes only a tiny surface: the
// Xvars stretch helpers (stretch_x/y/area/size), is_valid/mark_valid, and the
// CMN_* / Pixmap / *Xdata types that hold the compiled art.  The whole X11
// pixmap+clip-mask machinery becomes SDL_Texture handles whose ALPHA channel
// is the former clip mask (the XPM parser already turns 'c None' into alpha 0).

#ifndef XDATA_H
#define XDATA_H

#include <SDL2/SDL.h>

#include "utils.h"
#include "coord.h"

#include "font.h"    // BitmapFont (the compiled-in 6x13 / 9x15 fonts)

#define Xvars_WINDOW_BG_COLOR "gray70"
#define Xvars_WINDOW_BORDER_COLOR "grey40"
#define Xvars_BACKGROUND "light grey"


// On UNIX the "graphics valid" check is the identity function; keep that.
typedef Boolean XvarsValid;
#define XVARS_VALID_INIT False;


#define UI_KEYS_MAX IT_WEAPON_R


enum UIinput {UI_KEYS_RIGHT,UI_KEYS_LEFT,UI_INPUT_NONE};


// Only command on UNIX is init graphics.
enum IXCommand {IX_INIT};


// A packed opaque RGB color (0x00RRGGBB).  (Pixel)-1 is the "unset" sentinel
// used by alloc_named_color (never a real color since real colors have the top
// byte clear).
typedef Uint32 Pixel;
inline Pixel Pixel_rgb(Uint8 r,Uint8 g,Uint8 b) {
  return ((Pixel)r << 16) | ((Pixel)g << 8) | (Pixel)b;
}
inline Uint8 Pixel_r(Pixel p) {return (Uint8)((p >> 16) & 0xFF);}
inline Uint8 Pixel_g(Pixel p) {return (Uint8)((p >> 8) & 0xFF);}
inline Uint8 Pixel_b(Pixel p) {return (Uint8)(p & 0xFF);}


// A "pixmap" is an SDL texture handle plus its pixel size.  For art loaded from
// XPM we also retain the decoded RGBA so it can serve as the source of a
// geometric transform (mirroring/rotation) in generate_pixmap_from_transform.
struct SdlPixmap {
  SDL_Texture *tex;
  int w, h;
  int hotx, hoty;
  unsigned char *rgba;   // RGBA8888, or NULL (target/scratch textures)
  Boolean isTarget;      // created with SDL_TEXTUREACCESS_TARGET
};

typedef SdlPixmap *Pixmap;
typedef SdlPixmap *Drawable;
typedef void *Window;          // frontend panels don't use a real child window

typedef Drawable CMN_DRAWABLE;

// For Physical::get_pixmap_mask(): the mask is vestigial (alpha == the mask).
struct CMN_IMAGEDATA {
  Pixmap pixmap;
  Pixmap mask;
};

// Pointer to xpm bits (the compiled-in char* arrays).
typedef char *CMN_BITS_ID;

// The SDL frontend feeds process_event a pointer to the current SDL_Event.
typedef SDL_Event *CMN_EVENTDATA;

typedef char *CMN_COLOR;

// SDL keycode stand-in for the KeySym type used by the (SDL-inert) keyset code.
typedef Uint32 KeySym;


///////// Xvars
class Xvars {
public:
  enum {
    DISPLAYS_MAX = 6,
    DISPLAY_NAME_LENGTH = 80,
    HUMAN_COLORS_NUM = 6
  };

  Xvars();

  // ---- Window-system lifecycle (frontend-owned) ----
  Boolean open_display(const char *title,const Size &windowSize);
  /* EFFECTS: Create the one SDL window+renderer of the given pixel size,
     resolve all named colors and fonts.  dpyMax becomes 1.  Nearest-neighbor
     scaling (crisp pixel art). */

  void resize_window(const Size &windowSize);
  /* EFFECTS: Resize the single window to the given pixel size. */

  void close_display();

  // ---- Colors ----
  Pixel alloc_named_color(int dpyNum,const char *name,
                          Pixel def = (Pixel)-1) const;
  /* EFFECTS: Resolve an X11 color name via the baked table.  Returns def if
     unknown; if def is the (Pixel)-1 sentinel, returns white. */

  // ---- "graphics valid" identity check (as on UNIX) ----
  Boolean is_valid(XvarsValid check) {return check;}
  void mark_valid(XvarsValid &val) {val = True;}

  // ---- Pixmap loading / transforms ----
  Boolean load_pixmap(Drawable *pixmap,Drawable *mask,
                      int dpyNum,char **xpmBits);
  /* EFFECTS: Load an XPM into an SDL texture (alpha == the old mask).  Reduce
     by 2 if !is_stretched(), enlarge by scale/2 if stretch>=3.  *mask is set
     to the same handle (masks are vestigial). */

  Boolean load_pixmap(Drawable *pixmap,Drawable *mask,
                      int dpyNum,char **xpmBits,Boolean fullSize);

  static void set_use_averaging(Boolean val) {useAveraging = val;}

  void generate_pixmap_from_transform(int dpyNum,Drawable dest,Drawable src,
                                      const Size &srcSize,Drawable scratch,
                                      const TransformType *transforms,
                                      int tNum,int depth);
  /* EFFECTS: Build dest's pixels from src by geometric transform(s), on the
     retained RGBA, then (re)upload dest's texture. */

  // ---- Frontend-internal pixmap helpers (used by draw.cpp) ----
  Pixmap create_target_pixmap(int w,int h);
  /* EFFECTS: A render-target texture of the given pixel size (for back
     buffers and scratch).  No retained rgba. */

  Pixmap create_blank_pixmap(int w,int h);
  /* EFFECTS: A pixmap with a zeroed rgba buffer of the given size and a texture
     to be (re)uploaded later (for auto-generated transform destinations). */

  void upload_pixmap(Pixmap p);
  /* EFFECTS: (Re)create p->tex from p->rgba (static, blended, nearest). */

  void free_pixmap(Pixmap p);

  // ---- Render-target tracking (draw.cpp draws to the current buffer) ----
  void set_target(Drawable d);
  /* EFFECTS: SDL_SetRenderTarget(renderer, d ? d->tex : NULL), memoized. */

  void set_draw_color(Pixel p,Uint8 a = 255) {
    SDL_SetRenderDrawColor(renderer,Pixel_r(p),Pixel_g(p),Pixel_b(p),a);
  }

  // ---- Stretch (the built-in integer scaler, now 1..4) ----
  Boolean is_stretched() {return stretch >= 2;}
  int stretch_x(int val) {return stretch * val;}
  int stretch_y(int val) {return stretch * val;}
  inline Pos stretch_pos(const Pos &pos);
  inline Size stretch_size(const Size &size);
  Area stretch_area(const Area &area);

  int stretch;

  // Total number of allocated displays (always 1 for the SDL port).
  int dpyMax;

  SDL_Window *window;
  SDL_Renderer *renderer;

  // Colors (indexed by display, mirroring the X11 layout so ported draw code
  // like xvars.black[dpyNum] is untouched).
  Pixel white[DISPLAYS_MAX],black[DISPLAYS_MAX];
  Pixel red[DISPLAYS_MAX],green[DISPLAYS_MAX],
        arenaTextColor[DISPLAYS_MAX];
  Pixel humanColors[DISPLAYS_MAX][HUMAN_COLORS_NUM];
  Pixel windowBg[DISPLAYS_MAX];
  Pixel windowBorder[DISPLAYS_MAX];

  const BitmapFont *font[DISPLAYS_MAX];
  Size fontSize[DISPLAYS_MAX];
  const BitmapFont *bigFont[DISPLAYS_MAX];
  Size bigFontSize[DISPLAYS_MAX];

  static const char *humanColorNames[HUMAN_COLORS_NUM];


private:
  void gen_pix_from_trans(Drawable dest,Drawable src,const Size &srcSize,
                          TransformType transform);
  Boolean load_pixmap_scaled(Drawable *pixmap,Drawable *mask,
                             char **xpmBits,int numer,int denom);
  /* EFFECTS: Load an XPM and scale its native pixels by numer/denom
     (nearest-neighbor).  numer/denom == 1/1 full, 1/2 reduce, 3/2 or 2/1
     enlarge. */

  Drawable currentTarget;
  Boolean displayOpen;

  static Boolean useAveraging;
};


inline Pos Xvars::stretch_pos(const Pos &pos) {
  Pos ret(pos.x * stretch,pos.y * stretch);
  return ret;
}

inline Size Xvars::stretch_size(const Size &size) {
  Size ret;
  ret.set(size.width * stretch,size.height * stretch);
  return ret;
}



struct Wxdata {
  Pixel background[Xvars::DISPLAYS_MAX];
  Pixmap blockPixmaps[Xvars::DISPLAYS_MAX][W_ALL_BLOCKS_NUM];
  Pixmap blockMasks[Xvars::DISPLAYS_MAX][W_ALL_BLOCKS_NUM];
  Pixmap posterPixmaps[Xvars::DISPLAYS_MAX][W_ALL_POSTERS_NUM];
  Pixmap posterMasks[Xvars::DISPLAYS_MAX][W_ALL_POSTERS_NUM];
  Pixmap doorPixmaps[Xvars::DISPLAYS_MAX][2];
#ifdef W_DOORS_TRANSPARENT
  Pixmap doorMasks[Xvars::DISPLAYS_MAX][2];
#endif
  Pixmap moverSquarePixmaps[Xvars::DISPLAYS_MAX][W_ALL_MOVER_SQUARES_NUM];
  Pixmap moverSquareMasks[Xvars::DISPLAYS_MAX][W_ALL_MOVER_SQUARES_NUM];
  Pixmap moverPixmaps[Xvars::DISPLAYS_MAX][W_ALL_MOVERS_NUM];
  Pixmap moverMasks[Xvars::DISPLAYS_MAX][W_ALL_MOVERS_NUM];
  Pixmap backgroundPixmaps[Xvars::DISPLAYS_MAX][W_ALL_BACKGROUNDS_NUM];
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
  MovingXdata() {
    valid = XVARS_VALID_INIT;
    offsetsValid = False;
    // Zero the (large) handle arrays so unused direction/frame slots are
    // never mistaken for real textures.
    for (int d = 0; d < Xvars::DISPLAYS_MAX; d++)
      for (int i = 0; i < CO_DIR_MAX; i++)
        for (int m = 0; m < PH_ANIM_MAX; m++) {
          pixmaps[d][i][m] = 0;
          masks[d][i][m] = 0;
        }
  }

  XvarsValid valid;
  Pixmap pixmaps[Xvars::DISPLAYS_MAX][CO_DIR_MAX][PH_ANIM_MAX],
         masks[Xvars::DISPLAYS_MAX][CO_DIR_MAX][PH_ANIM_MAX];

  // Independent of init_x().
  Size offsets[CO_DIR_MAX];
  Boolean offsetsValid;
};



// UNIX Daemon helper (server logging).  Reused verbatim from the X11 port; no
// window-system dependency.
class Daemon {
public:
  Daemon(const char *fName);
  ~Daemon();
  const char *get_file_name();
  void go();

private:
  void daemonize();
  const char *fname;
  int fd;
};

#endif
