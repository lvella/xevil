/*
 * XEvil 2.5 SDL port -- A1 "proof of pixels" test.
 *
 * Proves the two hardest frontend primitives end to end, headless under Xvfb:
 *   (a) XPM art -> nearest-neighbor SDL_Texture, drawn 1x and 3x, with the
 *       'c None' color coming through as real transparency (no magenta/black
 *       boxes) -- the hero sprite, a frog, and a pistol.
 *   (b) compiled-in bitmap text in both fonts (6x13 and 9x15), including
 *       "XEvil 2.5 SDL -- the quick brown fox".
 *   (c) a filled rect + a tiled blit of a world block XPM.
 *
 * Renders one frame, writes it to a BMP (argv[1], default test_foundation.bmp)
 * via SDL_RenderReadPixels so the result can be screenshotted without any
 * window-manager/grab tooling, then exits.
 */

#include <cstdio>
#include <cstdlib>

#include <SDL2/SDL.h>

#include "font.h"
#include "sdlvars.h"
#include "xpm.h"

/* Compiled-in art (the .bitmaps files pull XPMs in exactly like this). */
#include "../x11/gen_xpm/hero/hero_0.xpm"     /* hero_0   32x56 */
#include "../x11/gen_xpm/frog/frog_0.xpm"     /* frog_0   14x10 */
#include "../x11/gen_xpm/pistol/pistol_4.xpm" /* pistol_4 20x18 */
#include "../x11/gen_xpm/world/block_0.xpm"   /* block_0  32x32 */

static void blit(SDL_Renderer *ren, SDL_Texture *tex, int srcW, int srcH,
                 int x, int y, int scale) {
  SDL_Rect dst;
  dst.x = x;
  dst.y = y;
  dst.w = srcW * scale;
  dst.h = srcH * scale;
  SDL_RenderCopy(ren, tex, NULL, &dst);
}

static bool save_bmp(SDL_Renderer *ren, int w, int h, const char *path) {
  SDL_Surface *shot = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32,
                                                     SDL_PIXELFORMAT_ARGB8888);
  if (!shot) {
    std::fprintf(stderr, "create shot surface failed: %s\n", SDL_GetError());
    return false;
  }
  if (SDL_RenderReadPixels(ren, NULL, SDL_PIXELFORMAT_ARGB8888,
                           shot->pixels, shot->pitch) != 0) {
    std::fprintf(stderr, "RenderReadPixels failed: %s\n", SDL_GetError());
    SDL_FreeSurface(shot);
    return false;
  }
  bool ok = (SDL_SaveBMP(shot, path) == 0);
  if (!ok)
    std::fprintf(stderr, "SaveBMP failed: %s\n", SDL_GetError());
  SDL_FreeSurface(shot);
  return ok;
}

int main(int argc, char **argv) {
  const char *outPath = (argc > 1) ? argv[1] : "test_foundation.bmp";
  const int W = 800, H = 600;

  SDLvars vars;
  if (!vars.init("XEvil 2.5 SDL -- foundation", W, H))
    return 1;
  SDL_Renderer *ren = vars.renderer;

  /* Load the art into textures. */
  int hw, hh, fw, fh, pw, ph, bw, bh;
  SDL_Texture *hero = vars.texture_from_xpm(hero_0, &hw, &hh);
  SDL_Texture *frog = vars.texture_from_xpm(frog_0, &fw, &fh);
  SDL_Texture *pistol = vars.texture_from_xpm(pistol_4, &pw, &ph);
  SDL_Texture *block = vars.texture_from_xpm(block_0, &bw, &bh);
  if (!hero || !frog || !pistol || !block) {
    std::fprintf(stderr, "failed to load one or more XPM textures\n");
    return 1;
  }

  /* --- draw the frame --- */

  /* A distinctive teal background: any missing transparency would show up as
     a magenta/black box against it. */
  SDL_SetRenderDrawColor(ren, 0x10, 0x50, 0x60, 0xFF);
  SDL_RenderClear(ren);

  /* Label strip along the top. */
  font::draw(ren, font::f6x13(), 16, 16 + font::ascent(font::f6x13()),
             "(a) XPM -> texture   1x and 3x, 'c None' == transparency",
             255, 255, 255);

  /* (a) hero at 1x and 3x. */
  blit(ren, hero, hw, hh, 20, 40, 1);
  blit(ren, hero, hw, hh, 80, 40, 3);

  /* (b) frog + pistol at 3x, to the right of the hero. */
  blit(ren, frog, fw, fh, 400, 60, 3);
  font::draw(ren, font::f6x13(), 400, 60 + fh * 3 + 14, "frog x3", 255, 255, 0);
  blit(ren, pistol, pw, ph, 560, 60, 3);
  font::draw(ren, font::f6x13(), 560, 60 + ph * 3 + 14, "pistol x3",
             255, 255, 0);

  /* (c) text in both fonts, including the required string. */
  int ty = 300;
  /* NOTE: real UTF-8 em-dash below; font::draw decodes it and falls back to
     an ASCII '-' since the 6x13/9x15 cells have no em-dash glyph. */
  font::draw(ren, font::f6x13(), 16, ty, "6x13:  XEvil 2.5 SDL \xE2\x80\x94 the quick brown fox",
             255, 255, 255);
  ty += font::cell_height(font::f6x13()) + 8;
  font::draw(ren, font::f9x15(), 16, ty, "9x15:  XEvil 2.5 SDL \xE2\x80\x94 the quick brown fox",
             120, 255, 180);
  ty += font::cell_height(font::f9x15()) + 8;
  font::draw(ren, font::f6x13(), 16, ty,
             "digits/punct: 0123456789 !\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~",
             255, 220, 120);
  ty += font::cell_height(font::f6x13()) + 8;

  /* Exercise the explicit "draw to texture" path too, scaled 2x. */
  int lw, lh;
  SDL_Texture *label = font::make_text_texture(
      ren, font::f9x15(), "make_text_texture() -> scaled 2x", 255, 90, 90, 255,
      &lw, &lh);
  if (label) {
    SDL_Rect d;
    d.x = 16; d.y = ty; d.w = lw * 2; d.h = lh * 2;
    SDL_RenderCopy(ren, label, NULL, &d);
    SDL_DestroyTexture(label);
  }

  /* (d) a filled rect + a tiled blit of a world block XPM. */
  font::draw(ren, font::f6x13(), 16, 430, "(d) filled rect + tiled world block:",
             255, 255, 255);
  SDL_Rect fill;
  fill.x = 16; fill.y = 440; fill.w = 120; fill.h = 96;
  SDL_SetRenderDrawColor(ren, vars.red.r, vars.red.g, vars.red.b, 255);
  SDL_RenderFillRect(ren, &fill);

  int cols = 12, rows = 3;
  int ox = 160, oy = 440;
  for (int ry = 0; ry < rows; ry++)
    for (int rx = 0; rx < cols; rx++)
      blit(ren, block, bw, bh, ox + rx * bw, oy + ry * bh, 1);

  /* Read back the frame and save it before presenting. */
  bool saved = save_bmp(ren, W, H, outPath);
  SDL_RenderPresent(ren);

  /* Brief on-screen presence so the window path is exercised under Xvfb. */
  for (int i = 0; i < 5; i++) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) { /* drain */ }
    SDL_Delay(20);
  }

  SDL_DestroyTexture(hero);
  SDL_DestroyTexture(frog);
  SDL_DestroyTexture(pistol);
  SDL_DestroyTexture(block);
  vars.shutdown();

  std::printf("%s: wrote %s\n", saved ? "OK" : "FAIL", outPath);
  return saved ? 0 : 1;
}
