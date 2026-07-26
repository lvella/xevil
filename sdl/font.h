/*
 * XEvil 2.5 SDL port -- compiled-in bitmap fonts.
 *
 * NO SDL_ttf / freetype dependency: the classic public-domain X11 fonts
 * 6x13 and 9x15 are baked into C arrays (see fonts/gen_font.py and the
 * generated fonts/font6x13.h / fonts/font9x15.h).
 *
 * The metric idiom mirrors X11's XFontStruct->max_bounds so the panel layout
 * math ports 1:1:
 *     width  = char_width(f) * cols        (== font->max_bounds.width * cols)
 *     height = cell_height(f) * rows        (== (ascent + descent) * rows)
 * and draw() takes a BASELINE y, exactly like XDrawString.
 */

#ifndef SDL_FONT_H
#define SDL_FONT_H

#include <SDL2/SDL.h>

/* A fixed-cell bitmap font.  Every glyph occupies cellW x cellH pixels; `bits`
   holds numChars glyphs starting at codepoint firstChar, each glyph cellH rows
   of bytesPerRow bytes, MSB-first, row 0 == top of cell (baseline - ascent). */
struct BitmapFont {
  int cellW;        /* fixed advance / max_bounds.width  */
  int cellH;        /* == ascent + descent               */
  int ascent;       /* max_bounds.ascent                 */
  int descent;      /* max_bounds.descent                */
  int firstChar;    /* codepoint of glyph 0 (32 == ' ')  */
  int numChars;     /* number of glyphs                  */
  int bytesPerRow;  /* (cellW + 7) / 8                   */
  const unsigned char *bits;
};

namespace font {

/* The two shipped fonts (defined in font.cpp). */
const BitmapFont &f6x13();   /* the default HUD/menu font (X11 "6x13")  */
const BitmapFont &f9x15();   /* the bigger font        (X11 "9x15")     */

/* ---- metric idioms (mirror XFontStruct->max_bounds) ---- */
inline int char_width (const BitmapFont &f) { return f.cellW; }
inline int cell_height(const BitmapFont &f) { return f.cellH; }
inline int ascent     (const BitmapFont &f) { return f.ascent; }
inline int descent    (const BitmapFont &f) { return f.descent; }

/* Pixel width of a UTF-8 string (fixed-pitch: glyph count * cellW). */
int text_width(const BitmapFont &f, const char *s);

/* Draw a UTF-8 string onto the renderer's CURRENT target (window backbuffer,
   or a texture if one is set via SDL_SetRenderTarget -- i.e. this IS the
   "draw to texture" primitive).  (x, baselineY) is the pen position with y on
   the baseline, exactly like XDrawString.  Bytes >= 0x80 are UTF-8 decoded;
   em/en dashes map to '-', anything else out of range to '?'.  Restores the
   renderer draw color on return. */
void draw(SDL_Renderer *ren, const BitmapFont &f, int x, int baselineY,
          const char *s, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);

/* Same as draw() but magnifies every glyph pixel into a scale x scale block
   (integer nearest-neighbor), so the compiled bitmap fonts scale crisply with
   the -scale option.  scale<=1 behaves exactly like draw().  The pen advances
   by cellW*scale per glyph and baselineY/ascent are in the scaled metric. */
void draw_scaled(SDL_Renderer *ren, const BitmapFont &f, int x, int baselineY,
                 const char *s, Uint8 r, Uint8 g, Uint8 b, Uint8 a, int scale);

/* Bake a UTF-8 string into a brand-new RGBA streaming texture (transparent
   background, given text color).  Caller owns the texture (SDL_DestroyTexture).
   Writes the texture size into wOut/hOut when non-NULL.  Nearest scaled. */
SDL_Texture *make_text_texture(SDL_Renderer *ren, const BitmapFont &f,
                               const char *s, Uint8 r, Uint8 g, Uint8 b,
                               Uint8 a, int *w, int *h);

}  // namespace font

#endif
