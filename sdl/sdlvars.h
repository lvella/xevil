/*
 * XEvil 2.5 SDL port -- SDLvars: window/renderer/color/font state.
 *
 * This is the SDL analogue of x11/xdata.h's `Xvars` (the object that carries
 * all the window-system state the frontend needs: the display connection,
 * allocated colors, the fonts, and the pixmap loader).  For the A1 foundation
 * it carries just enough for the proof-of-pixels test, but the surface is
 * shaped to grow into the full Xvars replacement:
 *   - Xvars::dpy/scr/root/cmap        -> SDL_Window* + SDL_Renderer*
 *   - Xvars::alloc_named_color        -> SDLvars::alloc_named_color
 *   - Xvars::white/black/red/green/humanColors/windowBg/windowBorder
 *                                     -> same-named SDLColor fields
 *   - Xvars::font/bigFont (XFontStruct)-> const BitmapFont* font/bigFont
 *   - Xvars::load_pixmap (XPM->Pixmap+mask) -> texture_from_xpm (XPM->texture,
 *                                              alpha == the old clip mask)
 * Multi-display (Xvars::DISPLAYS_MAX) is intentionally NOT modeled yet.
 */

#ifndef SDL_SDLVARS_H
#define SDL_SDLVARS_H

#include <SDL2/SDL.h>

#include "font.h"

struct SDLColor {
  Uint8 r, g, b, a;
};

class SDLvars {
 public:
  enum { HUMAN_COLORS_NUM = 6 };

  static SDLColor make_color(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255) {
    SDLColor c;
    c.r = r; c.g = g; c.b = b; c.a = a;
    return c;
  }

  SDL_Window *window;
  SDL_Renderer *renderer;

  /* Common colors (mirror the Xvars pixel fields). */
  SDLColor white, black, red, green;
  SDLColor humanColors[HUMAN_COLORS_NUM];
  SDLColor windowBg;       /* gray70    (Xvars_WINDOW_BG_COLOR)     */
  SDLColor windowBorder;   /* grey40    (Xvars_WINDOW_BORDER_COLOR) */
  SDLColor background;     /* light grey (Xvars_BACKGROUND)         */

  /* Fonts (mirror Xvars::font / Xvars::bigFont). */
  const BitmapFont *font;     /* the "6x13" HUD/menu font */
  const BitmapFont *bigFont;  /* the "9x15" big font      */

  SDLvars();

  /* Create the window + renderer (nearest-neighbor scaling) and resolve all
     the named colors and fonts.  Returns false (with an SDL_GetError print)
     on failure. */
  bool init(const char *title, int w, int h);
  void shutdown();

  /* Mirror Xvars::alloc_named_color: resolve an X11 color name via the baked
     table; returns `def` (default white) when the name is unknown. */
  SDLColor alloc_named_color(const char *name,
                             SDLColor def = make_color(255, 255, 255)) const;

  /* Mirror Xvars::load_pixmap: build a nearest-neighbor SDL_Texture straight
     from compiled-in XPM art (the alpha channel is the former clip mask).
     Writes the pixel size into wOut/hOut when non-NULL.  NULL on failure. */
  SDL_Texture *texture_from_xpm(const char *const *xpmData,
                                int *w = 0, int *h = 0) const;

  /* Human player color names (matches x11 Xvars::humanColorNames). */
  static const char *humanColorNames[HUMAN_COLORS_NUM];
};

#endif
