/*
 * XEvil 2.5 SDL port -- compiled-in bitmap font rendering.
 */

#include "font.h"

/* Only this TU pulls in the generated glyph blobs, so their internal-linkage
   `static const` definitions never collide across the build. */
#include "fonts/font6x13.h"
#include "fonts/font9x15.h"

namespace font {

const BitmapFont &f6x13() { return font6x13; }
const BitmapFont &f9x15() { return font9x15; }

/* ---- minimal UTF-8 decode ----
   Returns the next codepoint and advances *pp past its bytes.  Only the 1/2/3
   byte forms we actually need are handled; malformed bytes decode as-is. */
static unsigned decode_utf8(const unsigned char **pp) {
  const unsigned char *p = *pp;
  unsigned c = *p++;
  if (c < 0x80) {
    ;                                  /* ASCII */
  } else if ((c & 0xE0) == 0xC0 && (p[0] & 0xC0) == 0x80) {
    c = ((c & 0x1F) << 6) | (p[0] & 0x3F);
    p += 1;
  } else if ((c & 0xF0) == 0xE0 && (p[0] & 0xC0) == 0x80 &&
             (p[1] & 0xC0) == 0x80) {
    c = ((c & 0x0F) << 12) | ((p[0] & 0x3F) << 6) | (p[1] & 0x3F);
    p += 2;
  }
  *pp = p;
  return c;
}

/* Map a codepoint to a glyph index in the font, or -1 to draw nothing. */
static int glyph_index(const BitmapFont &f, unsigned cp) {
  if (cp == 0x2014 || cp == 0x2013)   /* em / en dash -> ASCII hyphen  */
    cp = '-';
  if (cp < (unsigned)f.firstChar || cp >= (unsigned)(f.firstChar + f.numChars)) {
    if (cp == ' ')
      return -1;
    cp = '?';                          /* unknown -> visible fallback   */
  }
  if (cp == ' ')
    return -1;                         /* space: advance only, no pixels */
  return (int)cp - f.firstChar;
}

int text_width(const BitmapFont &f, const char *s) {
  int n = 0;
  const unsigned char *p = (const unsigned char *)s;
  while (*p) {
    decode_utf8(&p);
    n++;
  }
  return n * f.cellW;
}

/* Collect the set pixels of one glyph as renderer points at (penX, topY). */
static int glyph_points(const BitmapFont &f, int gi, int penX, int topY,
                        SDL_Point *pts, int cap) {
  const unsigned char *g = f.bits + (size_t)gi * f.cellH * f.bytesPerRow;
  int n = 0;
  for (int row = 0; row < f.cellH; row++) {
    const unsigned char *rb = g + (size_t)row * f.bytesPerRow;
    for (int col = 0; col < f.cellW; col++) {
      int byte = col >> 3;
      int bit = 7 - (col & 7);
      if (rb[byte] & (1 << bit)) {
        if (n < cap) {
          pts[n].x = penX + col;
          pts[n].y = topY + row;
          n++;
        }
      }
    }
  }
  return n;
}

void draw(SDL_Renderer *ren, const BitmapFont &f, int x, int baselineY,
          const char *s, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
  Uint8 or_, og, ob, oa;
  SDL_GetRenderDrawColor(ren, &or_, &og, &ob, &oa);
  SDL_SetRenderDrawColor(ren, r, g, b, a);

  /* Worst case every pixel of a cell is set. */
  SDL_Point pts[16 * 32];
  const int cap = (int)(sizeof(pts) / sizeof(pts[0]));
  int topY = baselineY - f.ascent;
  int pen = x;
  const unsigned char *p = (const unsigned char *)s;
  while (*p) {
    unsigned cp = decode_utf8(&p);
    int gi = glyph_index(f, cp);
    if (gi >= 0) {
      int n = glyph_points(f, gi, pen, topY, pts, cap);
      if (n > 0)
        SDL_RenderDrawPoints(ren, pts, n);
    }
    pen += f.cellW;
  }
  SDL_SetRenderDrawColor(ren, or_, og, ob, oa);
}

void draw_scaled(SDL_Renderer *ren, const BitmapFont &f, int x, int baselineY,
                 const char *s, Uint8 r, Uint8 g, Uint8 b, Uint8 a, int scale) {
  if (scale <= 1) {
    draw(ren, f, x, baselineY, s, r, g, b, a);
    return;
  }
  Uint8 or_, og, ob, oa;
  SDL_GetRenderDrawColor(ren, &or_, &og, &ob, &oa);
  SDL_SetRenderDrawColor(ren, r, g, b, a);

  int topY = baselineY - f.ascent * scale;
  int pen = x;
  const unsigned char *p = (const unsigned char *)s;
  while (*p) {
    unsigned cp = decode_utf8(&p);
    int gi = glyph_index(f, cp);
    if (gi >= 0) {
      const unsigned char *g = f.bits + (size_t)gi * f.cellH * f.bytesPerRow;
      for (int row = 0; row < f.cellH; row++) {
        const unsigned char *rb = g + (size_t)row * f.bytesPerRow;
        for (int col = 0; col < f.cellW; col++) {
          int byte = col >> 3;
          int bit = 7 - (col & 7);
          if (rb[byte] & (1 << bit)) {
            SDL_Rect blk = {pen + col * scale, topY + row * scale, scale, scale};
            SDL_RenderFillRect(ren, &blk);
          }
        }
      }
    }
    pen += f.cellW * scale;
  }
  SDL_SetRenderDrawColor(ren, or_, og, ob, oa);
}

SDL_Texture *make_text_texture(SDL_Renderer *ren, const BitmapFont &f,
                               const char *s, Uint8 r, Uint8 g, Uint8 b,
                               Uint8 a, int *wOut, int *hOut) {
  int w = text_width(f, s);
  int h = f.cellH;
  if (w <= 0)
    w = 1;
  SDL_Texture *tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA32,
                                       SDL_TEXTUREACCESS_STREAMING, w, h);
  if (!tex)
    return NULL;
  SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
  SDL_SetTextureScaleMode(tex, SDL_ScaleModeNearest);

  void *pixels = NULL;
  int pitch = 0;
  if (SDL_LockTexture(tex, NULL, &pixels, &pitch) != 0) {
    SDL_DestroyTexture(tex);
    return NULL;
  }
  /* Clear to transparent. */
  for (int y = 0; y < h; y++)
    SDL_memset((Uint8 *)pixels + (size_t)y * pitch, 0, (size_t)w * 4);

  Uint32 fg = ((Uint32)r) | ((Uint32)g << 8) | ((Uint32)b << 16) |
              ((Uint32)a << 24);  /* RGBA32 is byte order R,G,B,A on LE */
  int pen = 0;
  const unsigned char *p = (const unsigned char *)s;
  while (*p) {
    unsigned cp = decode_utf8(&p);
    int gi = glyph_index(f, cp);
    if (gi >= 0) {
      const unsigned char *gl = f.bits + (size_t)gi * f.cellH * f.bytesPerRow;
      for (int row = 0; row < f.cellH; row++) {
        Uint32 *dst = (Uint32 *)((Uint8 *)pixels + (size_t)row * pitch);
        const unsigned char *rb = gl + (size_t)row * f.bytesPerRow;
        for (int col = 0; col < f.cellW; col++) {
          int byte = col >> 3;
          int bit = 7 - (col & 7);
          if (rb[byte] & (1 << bit))
            dst[pen + col] = fg;
        }
      }
    }
    pen += f.cellW;
  }
  SDL_UnlockTexture(tex);
  if (wOut) *wOut = w;
  if (hOut) *hOut = h;
  return tex;
}

}  // namespace font
