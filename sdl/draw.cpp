/*
 * XEvil(TM) Copyright (C) 1994,2000 Steve Hardt and Michael Judge
 * http://www.xevil.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.  See gpl.txt.
 */

// "draw.cpp"  SDL port of every World/Locator/Moving/Fire/Explosion/Protection
// draw+init_x seam.  The X11 original used clip-mask XCopyArea; here the mask is
// the texture's alpha channel, so each blit is a single SDL_RenderCopy.  All
// drawing goes to the "current render target" (the viewport's back-buffer
// texture, passed in as the Drawable buffer) which Xvars::set_target() selects.

#include <cmath>

#include "utils.h"
#include "coord.h"
#include "world.h"
#include "locator.h"
#include "physical.h"
#include "actual.h"

using namespace std;


/* ------------------------------------------------------------------ *
 * SDL blit / primitive helpers.
 * ------------------------------------------------------------------ */

// Copy pixmap (a sub-rect, or the whole texture if srcR==NULL) onto buffer at
// dstR.  Transparency is the texture alpha.
static void sdl_blit(Xvars &xvars,Drawable buffer,Pixmap pixmap,
                     const SDL_Rect *srcR,const SDL_Rect *dstR,
                     Uint8 alpha = 255) {
  if (!pixmap || !pixmap->tex) {
    return;
  }
  xvars.set_target(buffer);
  if (alpha != 255) {
    SDL_SetTextureAlphaMod(pixmap->tex,alpha);
  }
  SDL_RenderCopy(xvars.renderer,pixmap->tex,srcR,dstR);
  if (alpha != 255) {
    SDL_SetTextureAlphaMod(pixmap->tex,255);
  }
}


// Filled ellipse inscribed in {x,y,w,h}, current draw color.
static void sdl_fill_ellipse(SDL_Renderer *ren,int x,int y,int w,int h) {
  if (w <= 0 || h <= 0) {
    return;
  }
  float rx = w * 0.5f, ry = h * 0.5f;
  float cx = x + rx, cy = y + ry;
  for (int row = 0; row < h; row++) {
    float dy = (row + 0.5f - ry) / ry;
    float t = 1.0f - dy * dy;
    if (t < 0) {
      continue;
    }
    int half = (int)(rx * sqrtf(t));
    int mid = (int)cx;
    SDL_RenderDrawLine(ren,mid - half,y + row,mid + half,y + row);
  }
  (void)cy;
}



/* ------------------------------------------------------------------ *
 * World draw functions.
 * ------------------------------------------------------------------ */

void World::draw_square(Drawable buffer,Xvars &xvars,int dpyNum,const Loc &l,
                        int x,int y,Boolean reduceDraw) {
  if (!inside(l)) {
    return;   // World::draw_outside() handles it.
  }

  int blockNum = themes[themeIndex].blockIndices[(unsigned char)map[l.r][l.c]];

  if (unionSquares[l.r][l.c]) {
    assert(map[l.r][l.c] == Wempty || map[l.r][l.c] == Wwall);

    // Poster.
    if (unionSquares[l.r][l.c]->type == UN_POSTER) {
      if (!reduceDraw) {
        PosterSquare *pSquare = &unionSquares[l.r][l.c]->pSquare;
        Pixmap pix = xdata.posterPixmaps[dpyNum][pSquare->poster];
        if (!pix) {
          return;
        }
        SDL_Rect src = {xvars.stretch_x(pSquare->loc.c * WSQUARE_WIDTH),
                        xvars.stretch_y(pSquare->loc.r * WSQUARE_HEIGHT),
                        xvars.stretch_x(WSQUARE_WIDTH),
                        xvars.stretch_y(WSQUARE_HEIGHT)};
        SDL_Rect dst = {xvars.stretch_x(x),xvars.stretch_y(y),
                        xvars.stretch_x(WSQUARE_WIDTH),
                        xvars.stretch_y(WSQUARE_HEIGHT)};
        sdl_blit(xvars,buffer,pix,&src,&dst);
      }
    }

    // Door.
    else if (unionSquares[l.r][l.c]->type == UN_DOOR) {
      int topBottom = unionSquares[l.r][l.c]->dSquare.topBottom;
      int doorNum = themes[themeIndex].doorBase + topBottom;
      SDL_Rect dst = {xvars.stretch_x(x),xvars.stretch_y(y),
                      xvars.stretch_x(WSQUARE_WIDTH),
                      xvars.stretch_y(WSQUARE_HEIGHT)};
      sdl_blit(xvars,buffer,xdata.doorPixmaps[dpyNum][doorNum],NULL,&dst);
    }

    // MoverSquare.
    else if (unionSquares[l.r][l.c]->type == UN_MOVER) {
      if (map[l.r][l.c] == Wwall) {
        // Draw the wall behind the mover square.
        UnionSquare *tmp = unionSquares[l.r][l.c];
        unionSquares[l.r][l.c] = NULL;
        draw_square(buffer,xvars,dpyNum,l,x,y,reduceDraw);
        unionSquares[l.r][l.c] = tmp;
      }
      int mSquareNum = themes[themeIndex].moverSquareBase +
        unionSquares[l.r][l.c]->mSquare.orientation;
      SDL_Rect dst = {xvars.stretch_x(x),xvars.stretch_y(y),
                      xvars.stretch_x(WSQUARE_WIDTH),
                      xvars.stretch_y(WSQUARE_HEIGHT)};
      sdl_blit(xvars,buffer,xdata.moverSquarePixmaps[dpyNum][mSquareNum],
               NULL,&dst);
    }

    else {
      assert(0);
    }
  }

  // Regular square.
  else {
    if (blockNum != Wempty) {   // empty squares come from the background
      SDL_Rect dst = {xvars.stretch_x(x),xvars.stretch_y(y),
                      xvars.stretch_x(WSQUARE_WIDTH),
                      xvars.stretch_y(WSQUARE_HEIGHT)};
      sdl_blit(xvars,buffer,xdata.blockPixmaps[dpyNum][blockNum],NULL,&dst);
    }
  }
}



void World::draw_mover(CMN_DRAWABLE buffer,Xvars &xvars,int dpyNum,
                       MoverP /* mover */,int x,int y) {
  int moverNum = themes[themeIndex].moverIndex;
  SDL_Rect dst = {xvars.stretch_x(x),xvars.stretch_y(y),
                  xvars.stretch_x(moverSize.width),
                  xvars.stretch_y(moverSize.height)};
  sdl_blit(xvars,buffer,xdata.moverPixmaps[dpyNum][moverNum],NULL,&dst);
}



void World::draw_outside_offset(CMN_DRAWABLE dest,Xvars &xvars,int dpyNum,
                                Size sourceOffset,const Area &destArea) {
  if (!xvars.is_valid(xValid)) {
    init_x(xvars,IX_INIT,NULL);
  }

  // Everything is already in window coordinates.
  Pos destPos = destArea.get_pos();
  Size destSize = destArea.get_size();

  SDL_Rect src = {sourceOffset.width,sourceOffset.height,
                  destSize.width,destSize.height};
  SDL_Rect dst = {destPos.x,destPos.y,destSize.width,destSize.height};
  sdl_blit(xvars,dest,xdata.outsidePixmaps[dpyNum][outsideIndex],&src,&dst);
}



void World::draw_background(CMN_DRAWABLE buffer,Xvars &xvars,int dpyNum,
                            Area area,Boolean background3D) {
  if (!xvars.is_valid(xValid)) {
    init_x(xvars,IX_INIT,NULL);
  }

  Size bSize = backgrounds[backgroundIndex].size;
  Size bgSize;
  bgSize.set(xvars.stretch_x(bSize.width >> 1),
             xvars.stretch_y(bSize.height >> 1));
  if (bgSize.width <= 0 || bgSize.height <= 0) {
    return;
  }

  Pos pos = xvars.stretch_pos(area.get_pos());
  Size size = xvars.stretch_size(area.get_size());

  if (background3D) {
    pos.x /= W_BACKGROUNDRATE;
    pos.y /= W_BACKGROUNDRATE;
  }

  // Tiling origin in (-bgSize,0].
  Pos tsOrigin(-(((pos.x % bgSize.width) + bgSize.width) % bgSize.width),
               -(((pos.y % bgSize.height) + bgSize.height) % bgSize.height));

  Pixmap bg = xdata.backgroundPixmaps[dpyNum][backgroundIndex];
  xvars.set_target(buffer);
  for (int ty = tsOrigin.y; ty < size.height; ty += bgSize.height) {
    for (int tx = tsOrigin.x; tx < size.width; tx += bgSize.width) {
      SDL_Rect dst = {tx,ty,bgSize.width,bgSize.height};
      sdl_blit(xvars,buffer,bg,NULL,&dst);
    }
  }
}



void World::init_x(Xvars &xvars,IXCommand,void*) {
  for (int dpyNum = 0; dpyNum < xvars.dpyMax; dpyNum++) {
    xdata.background[dpyNum] = xvars.alloc_named_color(dpyNum,Xvars_BACKGROUND);

    int n;
    for (n = 0; n < W_ALL_BLOCKS_NUM; n++) {
      if (!xvars.load_pixmap(&xdata.blockPixmaps[dpyNum][n],NULL,dpyNum,
                             (char**)blocksBits[n])) {
        cerr << "Failed to load block " << n << endl;
      }
    }

    // Posters (last poster is the title poster).
    for (n = 0; n < W_ALL_POSTERS_NUM; n++) {
      if (!xvars.load_pixmap(&xdata.posterPixmaps[dpyNum][n],NULL,dpyNum,
                             (char**)posters[n].id)) {
        cerr << "Failed to load poster " << n << endl;
      }
    }

    // Doors.
    for (n = 0; n < W_ALL_DOORS_NUM; n++) {
      if (!xvars.load_pixmap(&xdata.doorPixmaps[dpyNum][n],NULL,dpyNum,
                             (char**)doorPixmapBits[n])) {
        cerr << "Failed to load door block " << n << endl;
      }
    }

    // Mover squares.
    for (n = 0; n < W_ALL_MOVER_SQUARES_NUM; n++) {
      if (!xvars.load_pixmap(&xdata.moverSquarePixmaps[dpyNum][n],NULL,dpyNum,
                             (char**)moverSquarePixmapBits[n])) {
        cerr << "Failed to mover square " << n << endl;
      }
    }

    // Movers.
    for (n = 0; n < W_ALL_MOVERS_NUM; n++) {
      if (!xvars.load_pixmap(&xdata.moverPixmaps[dpyNum][n],NULL,dpyNum,
                             (char**)moverPixmapBits[n])) {
        cerr << "Failed to load mover " << n << endl;
      }
    }

    // Backgrounds.
    for (n = 0; n < W_ALL_BACKGROUNDS_NUM; n++) {
      if (!xvars.load_pixmap(&xdata.backgroundPixmaps[dpyNum][n],NULL,dpyNum,
                             (char**)backgrounds[n].id)) {
        cerr << "Failed to load background " << n << endl;
      }
    }

    // Outsides.
    for (n = 0; n < W_ALL_OUTSIDES_NUM; n++) {
      if (!xvars.load_pixmap(&xdata.outsidePixmaps[dpyNum][n],NULL,dpyNum,
                             (char**)outsides[n].id)) {
        cerr << "Failed to load outside " << n << endl;
      }
    }
  }

  xvars.mark_valid(xValid);
}



/* ------------------------------------------------------------------ *
 * Locator draw functions (only the smooth/full-frame path is used).
 * ------------------------------------------------------------------ */

Drawable Locator::get_scratch_buffer(Pos &pos,Xvars &,int dpyNum) {
  pos.set_zero();
  return xdata.scratchBuffer[dpyNum];
}



void Locator::init_x(Xvars &xvars,IXCommand command,void*) {
  assert(command == IX_INIT);
  assert(!xvars.is_valid(xValid));

  for (int dpyNum = 0; dpyNum < xvars.dpyMax; dpyNum++) {
    xdata.buffer[dpyNum] =
      xvars.create_target_pixmap(xvars.stretch_x(OL_GRID_COL_MAX * WSQUARE_WIDTH),
                                 xvars.stretch_y(OL_GRID_ROW_MAX * WSQUARE_HEIGHT));
    xdata.scratchBuffer[dpyNum] =
      xvars.create_target_pixmap(xvars.stretch_x(OL_GRID_COL_MAX * WSQUARE_WIDTH),
                                 xvars.stretch_y(OL_GRID_ROW_MAX * WSQUARE_HEIGHT));

    // Load base tick pixmaps.
    TickType tt;
    int hp;
    for (hp = 0; hp < CO_DIR_HALF_PURE; hp++) {
      for (tt = 0; tt < TICK_MAX; tt++) {
        Dir dir = Coord::half_pure_to_pure(hp);
        if (Transform2D::is_base(dir,NULL)) {
          assert(tickPixmapBits[tt][hp] != PH_AUTO_GEN);
          xvars.load_pixmap(&xdata.tickPixmaps[dpyNum][tt][hp],NULL,dpyNum,
                            (char**)tickPixmapBits[tt][hp]);
        }
      }
    }

    // Auto-generate the rotated/mirrored ticks.
    for (tt = 0; tt < TICK_MAX; tt++) {
      for (hp = 0; hp < CO_DIR_HALF_PURE; hp++) {
        Dir dir = Coord::half_pure_to_pure(hp);
        if (!Transform2D::is_base(dir,NULL)) {
          assert(tickPixmapBits[tt][hp] == PH_AUTO_GEN);
          Dir base = Transform2D::get_base(dir,NULL);
          int baseHP = Coord::pure_to_half_pure(base);
          int tNum;
          const TransformType *transforms =
            Transform2D::get_transforms(tNum,dir,NULL);

          xdata.tickPixmaps[dpyNum][tt][hp] =
            xvars.create_blank_pixmap(xvars.stretch_x(tickSizes[hp].width),
                                      xvars.stretch_y(tickSizes[hp].height));
          xvars.generate_pixmap_from_transform(dpyNum,
                          xdata.tickPixmaps[dpyNum][tt][hp],
                          xdata.tickPixmaps[dpyNum][tt][baseHP],
                          xvars.stretch_size(tickSizes[baseHP]),
                          NULL,transforms,tNum,32);
        }
      }
    }
  }
  xvars.mark_valid(xValid);
}



Boolean Locator::draw_tick(TickType tt,CMN_DRAWABLE window,Xvars &xvars,
                           int dpyNum,const Size &windowSize,
                           Dir tickDir,int offset) {
  if (!xvars.is_valid(xValid)) {
    init_x(xvars,IX_INIT,NULL);
  }

  int hp = Coord::pure_to_half_pure(tickDir);
  Pos destPos;
  Size tSize = xvars.stretch_size(tickSizes[hp]);

  switch (tickDir) {
  case CO_R: case CO_DN_R: case CO_UP_R:
    destPos.x = windowSize.width - tSize.width; break;
  case CO_DN: case CO_UP:
    destPos.x = offset - (tSize.width >> 1); break;
  case CO_DN_L: case CO_L: case CO_UP_L:
    destPos.x = 0; break;
  default: assert(0);
  }

  switch (tickDir) {
  case CO_R: case CO_L:
    destPos.y = offset - (tSize.height >> 1); break;
  case CO_DN_R: case CO_DN: case CO_DN_L:
    destPos.y = windowSize.height - tSize.height; break;
  case CO_UP_L: case CO_UP: case CO_UP_R:
    destPos.y = 0; break;
  default: assert(0);
  }

  SDL_Rect dst = {destPos.x,destPos.y,tSize.width,tSize.height};
  sdl_blit(xvars,window,xdata.tickPixmaps[dpyNum][tt][hp],NULL,&dst);
  return True;
}



/* ------------------------------------------------------------------ *
 * Fire / Protection / Explosion / Moving.
 * ------------------------------------------------------------------ */

void Fire::init_x(Xvars &xvars,IXCommand command,void*) {
  assert(!xvars.is_valid(xdata.valid));
  assert(command == IX_INIT);

  for (int dpyNum = 0; dpyNum < xvars.dpyMax; dpyNum++) {
    if (!xvars.load_pixmap(&xdata.pixmap[dpyNum],NULL,dpyNum,(char**)fireBits)) {
      cerr << "Failed to load fire graphics." << endl;
    }
  }
  xvars.mark_valid(xdata.valid);
}



void Protection::draw(Drawable buffer,Xvars &xvars,int dpyNum,
                      const Area &bufArea) {
  if (!xvars.is_valid(pXdata->valid)) {
    init_x(xvars,IX_INIT,NULL,*prc,*pXdata);
  }

  Pos pos;
  Size size;
  area.get_rect(pos,size);
  Size offset = area - bufArea;

  xvars.set_target(buffer);
  xvars.set_draw_color(pXdata->color[dpyNum]);
  SDL_Rect r = {xvars.stretch_x(offset.width),xvars.stretch_y(offset.height),
                xvars.stretch_x(size.width - 1),xvars.stretch_y(size.height - 1)};
  SDL_RenderDrawRect(xvars.renderer,&r);
  xvars.set_draw_color(xvars.black[dpyNum]);
}



void Moving::draw(Drawable buffer,Xvars &xvars,int dpyNum,
                  const Area &bufArea) {
  if (!xvars.is_valid(movingXdata->valid)) {
    init_x(xvars,IX_INIT,NULL,*mc,*movingXdata);
  }

  Pos pos;
  Size size;
  area.get_rect(pos,size);
  Size offset = area - bufArea;

  CMN_IMAGEDATA imageData;
  get_pixmap_mask(xvars,dpyNum,imageData,dir,movingAnimNum);
  Pixmap pixmap = imageData.pixmap;

  // Invisibility: the X11 version did a screen-distortion wrap trick; here we
  // render the sprite at reduced alpha (25%), which reads as a translucent
  // shimmer and avoids a render-target read-back.
  Uint8 alpha = is_invisible() ? 64 : 255;

  SDL_Rect dst = {xvars.stretch_x(offset.width),xvars.stretch_y(offset.height),
                  xvars.stretch_x(size.width),xvars.stretch_y(size.height)};
  sdl_blit(xvars,buffer,pixmap,NULL,&dst,alpha);
}



void Moving::get_pixmap_mask(Xvars &,int dpyNum,CMN_IMAGEDATA &imageData,
                             Dir dir,int animNum) {
  imageData.pixmap = movingXdata->pixmaps[dpyNum][dir][animNum];
  imageData.mask = movingXdata->masks[dpyNum][dir][animNum];
}



void Explosion::draw(Drawable buffer,Xvars &xvars,int dpyNum,
                     const Area &bufArea) {
  Pos pos;
  Size size;
  area.get_rect(pos,size);
  Size offset = area - bufArea;

  xvars.set_target(buffer);
  xvars.set_draw_color(xvars.black[dpyNum]);
  sdl_fill_ellipse(xvars.renderer,
                   xvars.stretch_x(offset.width),xvars.stretch_y(offset.height),
                   xvars.stretch_x(size.width),xvars.stretch_y(size.height));
}



void Fire::draw(Drawable buffer,Xvars &xvars,int dpyNum,const Area &bufArea) {
  if (!xvars.is_valid(xdata.valid)) {
    init_x(xvars,IX_INIT,NULL);
  }

  const Area &area = get_area();
  Pos pos;
  Size size;
  area.get_rect(pos,size);
  Size offset = area - bufArea;
  Size sizeMax = Fire::get_size_max();

  // Grab a random part of the source bitmap (world coordinates).
  Pos srcPos(Utils::choose(sizeMax.width - size.width + 1),
             Utils::choose(sizeMax.height - size.height + 1));

  SDL_Rect src = {xvars.stretch_x(srcPos.x),xvars.stretch_y(srcPos.y),
                  xvars.stretch_x(size.width),xvars.stretch_y(size.height)};
  SDL_Rect dst = {xvars.stretch_x(offset.width),xvars.stretch_y(offset.height),
                  xvars.stretch_x(size.width),xvars.stretch_y(size.height)};
  sdl_blit(xvars,buffer,xdata.pixmap[dpyNum],&src,&dst);
}



void Protection::init_x(Xvars &xvars,IXCommand,void*,
                        const ProtectionContext &prc,ProtectionXdata &pXdata) {
  assert(!xvars.is_valid(pXdata.valid));
  for (int dpyNum = 0; dpyNum < xvars.dpyMax; dpyNum++) {
    pXdata.color[dpyNum] =
      xvars.alloc_named_color(dpyNum,prc.colorName,xvars.black[dpyNum]);
  }
  xvars.mark_valid(pXdata.valid);
}



void Moving::init_x(Xvars &xvars,IXCommand command,void*,
                    const MovingContext &mc,MovingXdata &movingXdata) {
  assert(!xvars.is_valid(movingXdata.valid));
  assert(command == IX_INIT);

  int dpyNum;
  // Load all non-autogenerated base pixmaps.
  for (dpyNum = 0; dpyNum < xvars.dpyMax; dpyNum++) {
    for (int n = 0; n < CO_DIR_MAX; n++) {
      if (mc.animMax[n] == 0 || mc.pixmapBits[n][0] == PH_AUTO_GEN) {
        continue;
      }
      for (int m = 0; m < mc.animMax[n]; m++) {
        assert(mc.useXPM);
        if (!xvars.load_pixmap(&movingXdata.pixmaps[dpyNum][n][m],NULL,dpyNum,
                               (char**)mc.pixmapBits[n][m])) {
          cerr << "load_pixmap() failed for "
               << mc.physicalContext.className
               << " dir=" << n << " animNum=" << m << endl;
        }
        // Masks are vestigial (alpha is the mask); alias for get_pixmap_mask.
        movingXdata.masks[dpyNum][n][m] = movingXdata.pixmaps[dpyNum][n][m];
      }
    }
  }

  // Auto-generate the PH_AUTO_GEN pixmaps by transforming a base direction.
  for (dpyNum = 0; dpyNum < xvars.dpyMax; dpyNum++) {
    for (int n = 0; n < CO_DIR_MAX; n++) {
      if (mc.animMax[n] == 0 || mc.pixmapBits[n][0] != PH_AUTO_GEN) {
        continue;
      }
      Dir base = Transform2D::get_base(n,mc.transformOverride);
      int tNum;
      const TransformType *transforms =
        Transform2D::get_transforms(tNum,n,mc.transformOverride);

      for (int m = 0; m < mc.animMax[n]; m++) {
        movingXdata.pixmaps[dpyNum][n][m] =
          xvars.create_blank_pixmap(xvars.stretch_x(mc.sizes[n].width),
                                    xvars.stretch_y(mc.sizes[n].height));
        xvars.generate_pixmap_from_transform(dpyNum,
                        movingXdata.pixmaps[dpyNum][n][m],
                        movingXdata.pixmaps[dpyNum][base][m],
                        xvars.stretch_size(mc.sizes[base]),
                        NULL,transforms,tNum,32);
        movingXdata.masks[dpyNum][n][m] = movingXdata.pixmaps[dpyNum][n][m];
      }
    }
  }

  xvars.mark_valid(movingXdata.valid);
}
