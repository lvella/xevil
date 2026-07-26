/*
 * XEvil(TM) Copyright (C) 1994,2000 Steve Hardt and Michael Judge
 * http://www.xevil.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.  See gpl.txt.
 */

// "xdata.cpp"  SDL port of the Xvars window-system layer.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

extern "C" {
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
// <sysexits.h> (EX_OK etc.) is POSIX-only; absent on mingw-w64.  The dedicated
// -server -daemon mode that uses it is POSIX-only too (fork/setsid), stubbed
// out for Windows in the Daemon methods below.
#if !defined(_WIN32)
#include <sysexits.h>
#endif
}

#include "utils.h"
#include "coord.h"
#include "area.h"
#include "world.h"
#include "locator.h"
#include "xdata.h"

#include "xpm.h"

using namespace std;


Boolean Xvars::useAveraging = False;

const char *Xvars::humanColorNames[Xvars::HUMAN_COLORS_NUM] = {
  "blue",
  "brown",
  "black",
  "purple",
  "green4",
  "pink3",
};


Xvars::Xvars() {
  stretch = 1;
  dpyMax = 0;
  window = 0;
  renderer = 0;
  activeDpy = 0;
  displayOpen = False;

  for (int d = 0; d < DISPLAYS_MAX; d++) {
    windows[d] = 0;
    renderers[d] = 0;
    currentTarget[d] = 0;
    white[d] = Pixel_rgb(255,255,255);
    black[d] = Pixel_rgb(0,0,0);
    red[d] = Pixel_rgb(255,0,0);
    green[d] = Pixel_rgb(0,255,0);
    arenaTextColor[d] = white[d];
    windowBg[d] = Pixel_rgb(0xB3,0xB3,0xB3);
    windowBorder[d] = Pixel_rgb(0x66,0x66,0x66);
    for (int m = 0; m < HUMAN_COLORS_NUM; m++)
      humanColors[d][m] = white[d];
    font[d] = 0;
    bigFont[d] = 0;
    fontSize[d].set_zero();
    bigFontSize[d].set_zero();
  }
}



void Xvars::resolve_colors_fonts(int d) {
  // Colors are packed RGB ints and fonts are compiled-in glyph bitmaps -- both
  // renderer-independent -- so every display resolves to the same values.
  // (Names mirror x11/ui.cpp RED_COLOR/GREEN_COLOR/ARENA_TEXT.)
  white[d] = alloc_named_color(d,"white");
  black[d] = alloc_named_color(d,"black");
  red[d] = alloc_named_color(d,"red2",black[d]);
  green[d] = alloc_named_color(d,"green1",black[d]);
  arenaTextColor[d] = alloc_named_color(d,"red2",white[d]);
  windowBg[d] = alloc_named_color(d,Xvars_WINDOW_BG_COLOR);
  windowBorder[d] = alloc_named_color(d,Xvars_WINDOW_BORDER_COLOR);
  for (int m = 0; m < HUMAN_COLORS_NUM; m++)
    humanColors[d][m] = alloc_named_color(d,humanColorNames[m],black[d]);

  font[d] = &font::f6x13();
  bigFont[d] = &font::f9x15();
  fontSize[d].set(font::char_width(*font[d]),font::cell_height(*font[d]));
  bigFontSize[d].set(font::char_width(*bigFont[d]),
                     font::cell_height(*bigFont[d]));
}



Boolean Xvars::open_display(const char *title,const Size &windowSize) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    cerr << "SDL_Init failed: " << SDL_GetError() << endl;
    return False;
  }
  // Crisp nearest-neighbor scaling for the pixel art.
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,"0");

  dpyMax = 0;
  int d = add_display(title,windowSize);
  if (d < 0) {
    return False;
  }
  displayOpen = True;
  return True;
}



int Xvars::add_display(const char *title,const Size &windowSize) {
  if (dpyMax >= DISPLAYS_MAX) {
    cerr << "Xvars::add_display: no free display slot." << endl;
    return -1;
  }
  int d = dpyMax;

  SDL_Window *win =
    SDL_CreateWindow(title,SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
                     windowSize.width,windowSize.height,SDL_WINDOW_SHOWN);
  if (!win) {
    cerr << "SDL_CreateWindow failed: " << SDL_GetError() << endl;
    return -1;
  }

  // Prefer accelerated; fall back to software so headless Xvfb always works.
  SDL_Renderer *ren = SDL_CreateRenderer(win,-1,SDL_RENDERER_ACCELERATED |
                                         SDL_RENDERER_TARGETTEXTURE);
  if (!ren)
    ren = SDL_CreateRenderer(win,-1,SDL_RENDERER_SOFTWARE |
                             SDL_RENDERER_TARGETTEXTURE);
  if (!ren) {
    cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << endl;
    SDL_DestroyWindow(win);
    return -1;
  }
  SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_BLEND);

  windows[d] = win;
  renderers[d] = ren;
  currentTarget[d] = 0;
  dpyMax = d + 1;

  resolve_colors_fonts(d);
  set_active_display(d);
  return d;
}



void Xvars::set_active_display(int d) {
  if (d < 0 || d >= dpyMax) {
    return;
  }
  activeDpy = d;
  window = windows[d];
  renderer = renderers[d];
}



void Xvars::resize_window(const Size &windowSize) {
  resize_window(activeDpy,windowSize);
}

void Xvars::resize_window(int d,const Size &windowSize) {
  if (d >= 0 && d < dpyMax && windows[d])
    SDL_SetWindowSize(windows[d],windowSize.width,windowSize.height);
}

void Xvars::center_window(int d) {
  if (d >= 0 && d < dpyMax && windows[d])
    SDL_SetWindowPosition(windows[d],SDL_WINDOWPOS_CENTERED,
                          SDL_WINDOWPOS_CENTERED);
}

void Xvars::position_window(int d,int x,int y) {
  if (d >= 0 && d < dpyMax && windows[d])
    SDL_SetWindowPosition(windows[d],x,y);
}



void Xvars::set_fullscreen(int d,Boolean on,const Size &logical) {
  if (d < 0 || d >= dpyMax || !windows[d]) {
    return;
  }
  if (on) {
    // The letterbox: SDL scales the native-size logical content up to the
    // window with black bars, so every pixel-coordinate draw is unchanged.
    SDL_RenderSetLogicalSize(renderers[d],logical.width,logical.height);
    // Belt-and-suspenders so it also covers the screen on a bare X server with
    // no window manager (headless Xvfb): make the window borderless and the
    // size of the desktop before asking SDL for desktop-fullscreen.
    SDL_DisplayMode dm;
    int di = SDL_GetWindowDisplayIndex(windows[d]);
    if (SDL_GetDesktopDisplayMode(di < 0 ? 0 : di,&dm) == 0) {
      SDL_SetWindowBordered(windows[d],SDL_FALSE);
      SDL_SetWindowSize(windows[d],dm.w,dm.h);
      SDL_SetWindowPosition(windows[d],0,0);
    }
    SDL_SetWindowFullscreen(windows[d],SDL_WINDOW_FULLSCREEN_DESKTOP);
  } else {
    SDL_SetWindowFullscreen(windows[d],0);
    SDL_RenderSetLogicalSize(renderers[d],0,0);
    SDL_SetWindowBordered(windows[d],SDL_TRUE);
    SDL_SetWindowSize(windows[d],logical.width,logical.height);
    SDL_SetWindowPosition(windows[d],SDL_WINDOWPOS_CENTERED,
                          SDL_WINDOWPOS_CENTERED);
  }
}

Boolean Xvars::get_window_fullscreen(int d) {
  if (d < 0 || d >= dpyMax || !windows[d]) {
    return False;
  }
  return (SDL_GetWindowFlags(windows[d]) &
          (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP)) ? True : False;
}



void Xvars::set_window_title(int d,const char *title) {
  if (d >= 0 && d < dpyMax && windows[d])
    SDL_SetWindowTitle(windows[d],title);
}



void Xvars::set_window_icon(int d,char **xpmBits) {
  if (d < 0 || d >= dpyMax || !windows[d] || !xpmBits) {
    return;
  }
  XpmImage img;
  if (!xpm::parse((const char *const *)xpmBits,img)) {
    return;
  }
  // Copy into a surface SDL owns (img.rgba is freed when img goes out of scope).
  SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormat(
      0,img.width,img.height,32,SDL_PIXELFORMAT_RGBA32);
  if (surf) {
    for (int y = 0; y < img.height; y++) {
      memcpy((Uint8 *)surf->pixels + (size_t)y * surf->pitch,
             img.rgba + (size_t)y * img.width * 4,(size_t)img.width * 4);
    }
    SDL_SetWindowIcon(windows[d],surf);
    SDL_FreeSurface(surf);
  }
}



Uint32 Xvars::get_window_id(int d) {
  if (d < 0 || d >= dpyMax || !windows[d]) {
    return 0;
  }
  return SDL_GetWindowID(windows[d]);
}



void Xvars::close_display() {
  for (int d = 0; d < dpyMax; d++) {
    if (renderers[d]) {
      SDL_DestroyRenderer(renderers[d]);
      renderers[d] = 0;
    }
    if (windows[d]) {
      SDL_DestroyWindow(windows[d]);
      windows[d] = 0;
    }
  }
  renderer = 0;
  window = 0;
  if (displayOpen) {
    SDL_Quit();
    displayOpen = False;
  }
}



Pixel Xvars::alloc_named_color(int,const char *name,Pixel def) const {
  unsigned char r,g,b;
  if (xpm::lookup_named_color(name,r,g,b)) {
    return Pixel_rgb(r,g,b);
  }
  cerr << "Warning:: unable to allocate color " << (name ? name : "(null)")
       << "." << endl;
  return (def == (Pixel)-1) ? Pixel_rgb(255,255,255) : def;
}



void Xvars::set_target(Drawable d) {
  // Target state is per-renderer, so memoize per active display.
  if (d == currentTarget[activeDpy]) {
    return;
  }
  SDL_SetRenderTarget(renderer,d ? d->tex : NULL);
  currentTarget[activeDpy] = d;
}



/* ------------------------------------------------------------------ *
 * Pixmap creation helpers.
 * ------------------------------------------------------------------ */

static SDL_Texture *make_static_texture(SDL_Renderer *ren,
                                        const unsigned char *rgba,
                                        int w,int h) {
  SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormatFrom(
      (void *)rgba,w,h,32,w * 4,SDL_PIXELFORMAT_RGBA32);
  if (!surf) {
    cerr << "SDL_CreateRGBSurfaceWithFormatFrom: " << SDL_GetError() << endl;
    return 0;
  }
  SDL_Texture *tex = SDL_CreateTextureFromSurface(ren,surf);
  SDL_FreeSurface(surf);
  if (!tex) {
    cerr << "SDL_CreateTextureFromSurface: " << SDL_GetError() << endl;
    return 0;
  }
  SDL_SetTextureBlendMode(tex,SDL_BLENDMODE_BLEND);
  SDL_SetTextureScaleMode(tex,SDL_ScaleModeNearest);
  return tex;
}



Pixmap Xvars::create_target_pixmap(int w,int h) {
  SdlPixmap *p = new SdlPixmap;
  p->w = w;
  p->h = h;
  p->hotx = p->hoty = 0;
  p->rgba = 0;
  p->isTarget = True;
  p->tex = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_RGBA32,
                             SDL_TEXTUREACCESS_TARGET,w,h);
  if (p->tex) {
    SDL_SetTextureBlendMode(p->tex,SDL_BLENDMODE_BLEND);
    SDL_SetTextureScaleMode(p->tex,SDL_ScaleModeNearest);
  }
  return p;
}



Pixmap Xvars::create_blank_pixmap(int w,int h) {
  SdlPixmap *p = new SdlPixmap;
  p->w = w;
  p->h = h;
  p->hotx = p->hoty = 0;
  p->isTarget = False;
  p->rgba = (unsigned char *)calloc((size_t)w * h * 4,1);
  p->tex = 0;
  return p;
}



void Xvars::upload_pixmap(Pixmap p) {
  if (!p || !p->rgba) {
    return;
  }
  if (p->tex) {
    SDL_DestroyTexture(p->tex);
    p->tex = 0;
  }
  p->tex = make_static_texture(renderer,p->rgba,p->w,p->h);
}



void Xvars::free_pixmap(Pixmap p) {
  if (!p) {
    return;
  }
  for (int d = 0; d < dpyMax; d++) {
    if (p == currentTarget[d]) {
      int save = activeDpy;
      set_active_display(d);
      set_target(0);
      set_active_display(save);
    }
  }
  if (p->tex) {
    SDL_DestroyTexture(p->tex);
  }
  if (p->rgba) {
    free(p->rgba);
  }
  delete p;
}



/* ------------------------------------------------------------------ *
 * XPM loading (with nearest-neighbor scaling by numer/denom).
 * ------------------------------------------------------------------ */

Boolean Xvars::load_pixmap_scaled(Drawable *pixmap,Drawable *mask,
                                  char **xpmBits,int numer,int denom) {
  XpmImage img;
  if (!xpm::parse((const char *const *)xpmBits,img)) {
    return False;
  }

  int destW = img.width * numer / denom;
  int destH = img.height * numer / denom;
  if (destW < 1) destW = 1;
  if (destH < 1) destH = 1;

  unsigned char *dst = (unsigned char *)malloc((size_t)destW * destH * 4);
  if (!dst) {
    return False;
  }

  // Nearest-neighbor resample (subsample when reducing, pixel-replicate when
  // enlarging).  Matches the X11 subsample reducer for denom==2,numer==1.
  for (int dy = 0; dy < destH; dy++) {
    int sy = dy * denom / numer;
    if (sy >= img.height) sy = img.height - 1;
    for (int dx = 0; dx < destW; dx++) {
      int sx = dx * denom / numer;
      if (sx >= img.width) sx = img.width - 1;
      const unsigned char *s = img.rgba + ((size_t)sy * img.width + sx) * 4;
      unsigned char *d = dst + ((size_t)dy * destW + dx) * 4;
      d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; d[3] = s[3];
    }
  }

  SdlPixmap *p = new SdlPixmap;
  p->w = destW;
  p->h = destH;
  p->hotx = img.hotx * numer / denom;
  p->hoty = img.hoty * numer / denom;
  p->rgba = dst;
  p->isTarget = False;
  p->tex = make_static_texture(renderer,dst,destW,destH);
  if (!p->tex) {
    free(dst);
    delete p;
    return False;
  }

  *pixmap = p;
  // Masks are vestigial in the SDL port -- transparency is the alpha channel.
  // Alias the mask handle to the pixmap so callers that read imageData.mask are
  // satisfied; nothing frees it separately.
  if (mask) {
    *mask = p;
  }
  return True;
}



Boolean Xvars::load_pixmap(Drawable *pixmap,Drawable *mask,
                           int dpyNum,char **xpmBits) {
  // Textures belong to a single renderer, so create them on THIS display's
  // renderer (two-player loads a full copy of the art per window).
  set_active_display(dpyNum);
  // stretch==1 reduces the native-2x art by half; stretch==2 uses it as-is;
  // stretch>=3 enlarges it (nearest) to stretch/2 of native.
  if (stretch >= 3) {
    return load_pixmap_scaled(pixmap,mask,xpmBits,stretch,2);
  }
  return load_pixmap(pixmap,mask,dpyNum,xpmBits,stretch == 2);
}



Boolean Xvars::load_pixmap(Drawable *pixmap,Drawable *mask,
                           int dpyNum,char **xpmBits,Boolean fullSize) {
  set_active_display(dpyNum);
  if (fullSize) {
    return load_pixmap_scaled(pixmap,mask,xpmBits,1,1);
  }
  return load_pixmap_scaled(pixmap,mask,xpmBits,1,2);
}



/* ------------------------------------------------------------------ *
 * Geometric transforms (mirror/rotate), on the retained RGBA.
 * ------------------------------------------------------------------ */

// Produce a transformed copy of srcRgba (RGBA8888).  Caller frees *outRgba.
static void transform_rgba(const unsigned char *srcRgba,const Size &srcSize,
                           TransformType tt,
                           unsigned char **outRgba,Size &outSize) {
  outSize = Transform2D::apply(tt,srcSize);
  size_t bytes = (size_t)outSize.width * outSize.height * 4;
  unsigned char *out = (unsigned char *)malloc(bytes);

  Pos srcPos;
  for (srcPos.y = 0; srcPos.y < srcSize.height; srcPos.y++) {
    for (srcPos.x = 0; srcPos.x < srcSize.width; srcPos.x++) {
      Pos destPos = Transform2D::apply(tt,srcPos,srcSize);
      const unsigned char *s =
          srcRgba + ((size_t)srcPos.y * srcSize.width + srcPos.x) * 4;
      unsigned char *d =
          out + ((size_t)destPos.y * outSize.width + destPos.x) * 4;
      d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; d[3] = s[3];
    }
  }
  *outRgba = out;
}



void Xvars::gen_pix_from_trans(Drawable dest,Drawable src,const Size &srcSize,
                               TransformType transform) {
  if (!src || !src->rgba) {
    cerr << "gen_pix_from_trans: source has no pixels." << endl;
    return;
  }
  unsigned char *out;
  Size outSize;
  transform_rgba(src->rgba,srcSize,transform,&out,outSize);

  // Install as dest's pixels (dest was created blank, but resize to be safe).
  if (dest->rgba) {
    free(dest->rgba);
  }
  dest->rgba = out;
  dest->w = outSize.width;
  dest->h = outSize.height;
}



void Xvars::generate_pixmap_from_transform(int dpyNum,Drawable dest,Drawable src,
                                           const Size &srcSize,Drawable,
                                           const TransformType *transforms,
                                           int tNum,int) {
  // upload_pixmap (re)creates dest's texture on the active renderer.
  set_active_display(dpyNum);
  assert(tNum <= 2);
  switch (tNum) {
  case 0:
    gen_pix_from_trans(dest,src,srcSize,TR_NONE);
    break;
  case 1:
    gen_pix_from_trans(dest,src,srcSize,transforms[0]);
    break;
  case 2: {
    // src -> temp -> dest.
    unsigned char *tmp;
    Size tmpSize;
    transform_rgba(src->rgba,srcSize,transforms[0],&tmp,tmpSize);
    SdlPixmap tempPix;
    tempPix.rgba = tmp;
    tempPix.w = tmpSize.width;
    tempPix.h = tmpSize.height;
    tempPix.tex = 0;
    gen_pix_from_trans(dest,&tempPix,tmpSize,transforms[1]);
    free(tmp);
    break;
  }
  default:
    assert(0);
  }
  upload_pixmap(dest);
}



Area Xvars::stretch_area(const Area &area) {
  Area ret(stretch_pos(area.get_pos()),stretch_size(area.get_size()));
  return ret;
}



/* ------------------------------------------------------------------ *
 * Daemon -- POSIX server logging (ported verbatim from x11/xdata.cpp).
 * ------------------------------------------------------------------ */

Daemon::Daemon(const char *filename) {
  fname = Utils::strdup(filename);
  fd = 0;
}

Daemon::~Daemon() {
  delete fname;
  if (fd != 0) {
    ::close(fd);
  }
}

const char *Daemon::get_file_name() {
  return fname;
}

void Daemon::daemonize() {
#if defined(_WIN32)
  // fork()/setsid() do not exist on Windows; -daemon is a POSIX-only feature.
  cerr << "Background daemon mode is not supported on Windows." << endl;
#else
  pid_t new_pid = fork();
  if (new_pid < 0) {
    cerr << "Could not fork background process." << endl;
  } else if (new_pid == 0) {
    close(0);
    setsid();
    chdir("/tmp");
  } else {
    cout << "Started [pid " << new_pid << ']' << endl;
    exit(EX_OK);
  }
#endif
}

void Daemon::go() {
#if defined(_WIN32)
  cerr << "Background daemon logging is not supported on Windows." << endl;
#else
  int fDesc = ::open(fname,O_WRONLY | O_CREAT | O_APPEND,
                     S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
  if (fDesc == -1) {
    cerr << "Could not open log file " << fname << endl;
    return;
  }
  cout << "Logging all output to " << fname << endl;

  daemonize();

  int val1 = dup2(fDesc,1);
  int val2 = dup2(fDesc,2);
  if (val1 == -1 || val2 == -1) {
    cerr << "Unable to redirect output to log file " << fname << endl;
    return;
  }
  fd = fDesc;
#endif
}
