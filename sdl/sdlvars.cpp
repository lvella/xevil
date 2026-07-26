/*
 * XEvil 2.5 SDL port -- SDLvars implementation.
 */

#include "sdlvars.h"

#include <cstdio>

#include "xpm.h"

const char *SDLvars::humanColorNames[SDLvars::HUMAN_COLORS_NUM] = {
  "blue",
  "brown",
  "black",
  "purple",
  "green4",
  "pink3",
};

SDLvars::SDLvars()
    : window(0), renderer(0), font(0), bigFont(0) {
  white = make_color(255, 255, 255);
  black = make_color(0, 0, 0);
  red = make_color(255, 0, 0);
  green = make_color(0, 255, 0);
  for (int i = 0; i < HUMAN_COLORS_NUM; i++)
    humanColors[i] = white;
  windowBg = make_color(0xB3, 0xB3, 0xB3);
  windowBorder = make_color(0x66, 0x66, 0x66);
  background = make_color(0xD3, 0xD3, 0xD3);
}

SDLColor SDLvars::alloc_named_color(const char *name, SDLColor def) const {
  unsigned char r, g, b;
  if (xpm::lookup_named_color(name, r, g, b))
    return make_color(r, g, b);
  std::fprintf(stderr, "SDLvars::alloc_named_color: unknown color \"%s\"\n",
               name);
  return def;
}

bool SDLvars::init(const char *title, int w, int h) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
    return false;
  }
  /* Global default: crisp nearest-neighbor scaling for the pixel art. */
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

  window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_SHOWN);
  if (!window) {
    std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
    return false;
  }

  /* Prefer accelerated; fall back to software so headless Xvfb always works. */
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer)
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
  if (!renderer) {
    std::fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
    return false;
  }

  /* Resolve colors and fonts. */
  white = alloc_named_color("white");
  black = alloc_named_color("black");
  red = alloc_named_color("red");
  green = alloc_named_color("green1", make_color(0, 255, 0));
  for (int i = 0; i < HUMAN_COLORS_NUM; i++)
    humanColors[i] = alloc_named_color(humanColorNames[i]);
  windowBg = alloc_named_color("gray70");
  windowBorder = alloc_named_color("grey40");
  background = alloc_named_color("light grey");

  font = &font::f6x13();
  bigFont = &font::f9x15();
  return true;
}

void SDLvars::shutdown() {
  if (renderer) {
    SDL_DestroyRenderer(renderer);
    renderer = 0;
  }
  if (window) {
    SDL_DestroyWindow(window);
    window = 0;
  }
  SDL_Quit();
}

SDL_Texture *SDLvars::texture_from_xpm(const char *const *xpmData,
                                       int *wOut, int *hOut) const {
  XpmImage img;
  if (!xpm::parse(xpmData, img))
    return 0;

  SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormatFrom(
      img.rgba, img.width, img.height, 32, img.width * 4,
      SDL_PIXELFORMAT_RGBA32);
  if (!surf) {
    std::fprintf(stderr, "SDL_CreateRGBSurfaceWithFormatFrom failed: %s\n",
                 SDL_GetError());
    return 0;
  }
  SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
  SDL_FreeSurface(surf);   /* pixels were copied into the texture */
  if (!tex) {
    std::fprintf(stderr, "SDL_CreateTextureFromSurface failed: %s\n",
                 SDL_GetError());
    return 0;
  }
  SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
  SDL_SetTextureScaleMode(tex, SDL_ScaleModeNearest);
  if (wOut) *wOut = img.width;
  if (hOut) *hOut = img.height;
  return tex;
  /* img frees its rgba buffer here */
}
