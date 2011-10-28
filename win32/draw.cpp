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

#include "stdafx.h"
#include "coord.h"
#include "utils.h"
#include "xdata.h"
#include "ui.h"
#include "physical.h"
#include "actual.h"
#include "resource.h"
#include "world.h"



// Key for the Locator scratch buffer in the SurfaceManager.
#define OL_SCRATCH_KEY ((void*)(S_MAN_OL_NONCE << 24))



void Explosion::draw(CMN_DRAWABLE buffer,Xvars &xvars,int,
                     const Area &bufArea) {
  // TODO Change with Explsions animated in their own space
  Pos pos;
  Size size;
  area.get_rect(pos,size);
  Size offset = area - bufArea;
  HDC t_temphdc;
  HRESULT t_result;
  if (!xvars.GetDC(buffer, &t_temphdc)){
    return;
  }

  CDC *t_dc=CDC::FromHandle(t_temphdc);

  CBrush *t_brush=t_dc->SelectObject(&xvars.m_brushes[Xvars::BLACK]);
  CPen *t_pen=t_dc->SelectObject(&xvars.m_pens[Xvars::BLACK]);
  t_dc->Ellipse(xvars.stretch_x(offset.width),xvars.stretch_y(offset.height),
                xvars.stretch_x(offset.width+size.width),
                xvars.stretch_y(offset.height+size.height));
  t_dc->SelectObject(t_brush);
  t_dc->SelectObject(t_pen);
  t_result=buffer->ReleaseDC(t_temphdc);
  if (!xvars.hresultSuccess(t_result)) {
    return;
  }
}



void Fire::draw(CMN_DRAWABLE buffer,Xvars &xvars,int,const Area &bufArea) {
  if (!xvars.is_valid(xdata.valid)) {
    init_x(xvars,IX_INIT,NULL);
  }

  // Location of bitmap on surf.
  Pos surfPos;
  LPDIRECTDRAWSURFACE surf = 
    xvars.m_surfaceManager->lookup(surfPos,(void*)fireBits);
  
  const Area &area = get_area();
  Pos pos;
  Size size;
  area.get_rect(pos,size);
  Size offset = area - bufArea;
  Size sizeMax = Fire::get_size_max();
  HRESULT t_result;
  // Grab a random part of the source bitmap.
  Pos srcUL(Utils::choose(sizeMax.width - size.width + 1),
            Utils::choose(sizeMax.height - size.height + 1));
  CRect t_sourcerect(surfPos.x + xvars.stretch_x(srcUL.x),
                     surfPos.y + xvars.stretch_y(srcUL.y),
                     surfPos.x + xvars.stretch_x(srcUL.x + size.width),
                     surfPos.y + xvars.stretch_y(srcUL.y + size.height));

  t_result = xvars.Blt(buffer,xvars.stretch_x(offset.width),
                       xvars.stretch_y(offset.height),
                       surf,t_sourcerect,True);
  DHRESULT(t_result,return);
}



void Fire::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Size sizeMax = Fire::get_size_max();
  // Add request for surface.
  if (command == IX_ADD) {
    xvars.m_surfaceManager->add((void*)fireBits,xvars.stretch_size(sizeMax),
                                &xvars.XVARS_COLORKEY);    
    return;
  }
  
  if (command == IX_LIST) {
    // For IX_LIST, the argument is a PtrList.
    PtrList* ptrList = (PtrList*)arg;
    IXPixmapInfo* pInfo = new IXPixmapInfo;
    assert(pInfo);
    pInfo->key = (void*)fireBits;
    pInfo->dir = CO_air;
    pInfo->animNum = 0;
    ptrList->add((void*)pInfo);
  }
  // If IX_LIST, still do initialization.

  if (xvars.is_valid(xdata.valid)) {
    return;
  }

  // Load bitmap onto surface.
  Pos pos;
  LPDIRECTDRAWSURFACE surf = 
    xvars.m_surfaceManager->lookup(pos,(void*)fireBits);
  if (!xvars.load_surface(surf,CPoint(pos.x,pos.y),fireBits)) {
    COUTRESULT("Couldn't load Fire surface");
    return;    
  }

  xvars.mark_valid(xdata.valid);
}



void World::draw_outside_offset(LPDIRECTDRAWSURFACE dest,
                               Xvars &xvars,int,Size sourceOffset,
                               const Area &destArea) {
  CMN_BITS_ID srcBits = outsides[outsideIndex].id;

  // Offset into the surface in the SurfaceManager.
  Size smOffset;
  LPDIRECTDRAWSURFACE source = 
    xvars.m_surfaceManager->lookup(smOffset,(void*)srcBits);

  sourceOffset += smOffset;

  // In destination coords.
  Pos destPos = destArea.get_pos();
  Size destSize = destArea.get_size();
  
  CRect t_srcrect(sourceOffset.width,sourceOffset.height,
                  sourceOffset.width + destSize.width,sourceOffset.height + destSize.height);

  HRESULT t_result = xvars.Blt(dest,destPos.x,destPos.y,source,t_srcrect);
  DHRESULT(t_result,return);
}



void World::draw_background(LPDIRECTDRAWSURFACE buffer,Xvars &xvars,int,
                            Area area,Boolean background3D) {
  // Always true on Windows.
  assert(background3D);

  if (!xvars.is_valid(xValid)) {
    init_x(xvars,IX_INIT,NULL);
  }
  
  area = xvars.stretch_area(area);
  CRect t_destrect;
  CRect t_srcrect;
  int offsetx = 0;
  int offsety = 0;
  int srcoffsettop;
  int t_int;
  if (area.get_pos().x < 0) {
    t_int = (area.get_pos().x/W_BACKGROUNDRATE)* -1;
    t_int %= backgrounds[backgroundIndex].size.width;
    t_int *= -1;
    t_int += backgrounds[backgroundIndex].size.width;
    t_srcrect.left = t_int;
  }
  else {
    t_srcrect.left = (area.get_pos().x / W_BACKGROUNDRATE) % backgrounds[backgroundIndex].size.width;  //important
  }
  if (area.get_pos().y < 0) {
      t_int = (area.get_pos().y / W_BACKGROUNDRATE) * -1;
      t_int %= backgrounds[backgroundIndex].size.height;
      t_int *= -1;
      t_int += backgrounds[backgroundIndex].size.height;
      srcoffsettop = t_int;
  }
  else {
    srcoffsettop = (area.get_pos().y / W_BACKGROUNDRATE) % backgrounds[backgroundIndex].size.height;  //important
  }
  t_srcrect.top = srcoffsettop;
  t_srcrect.right = backgrounds[backgroundIndex].size.width;
  t_srcrect.bottom = backgrounds[backgroundIndex].size.height;
  HRESULT t_result;
  for(;offsetx < area.get_size().width;) {
    for(;offsety < area.get_size().height;) {
        t_destrect.SetRect(offsetx,offsety,min(offsetx + t_srcrect.Width(),area.get_size().width),
                           min(offsety+t_srcrect.Height(),area.get_size().height));
        t_srcrect.right = min(backgrounds[backgroundIndex].size.width,t_srcrect.left + t_destrect.Width());
        t_srcrect.bottom = min(backgrounds[backgroundIndex].size.height,t_srcrect.top + t_destrect.Height());
        if (t_srcrect.Width() && t_srcrect.Height()) {
          // Now, lookup surface in SurfaceManger.  t_surfrect is t_srcrect adjusted for the 
          // SurfaceManager surface offset.
          Pos t_surfpos;
          LPDIRECTDRAWSURFACE t_surf = 
            xvars.m_surfaceManager->lookup(t_surfpos,(void*)backgrounds[backgroundIndex].id);
          CRect t_surfrect(t_srcrect);
          t_surfrect.OffsetRect(t_surfpos.x,t_surfpos.y);

          t_result = xvars.Blt(buffer,offsetx,offsety,t_surf,t_surfrect);
          DHRESULT(t_result,;)
        }
        offsety += t_srcrect.Height();
        t_srcrect.top = 0;
        t_srcrect.right = backgrounds[backgroundIndex].size.width;
    }
    offsety = 0;
    offsetx += t_srcrect.Width();
    t_srcrect.left = 0;
    t_srcrect.top = srcoffsettop;
    t_srcrect.bottom = backgrounds[backgroundIndex].size.height;
  }
}



void World::draw_square(LPDIRECTDRAWSURFACE buffer,Xvars &xvars,int,
                        const Loc &l,int x,int y,Boolean reduceDraw) {
  if (!inside(l) || map[l.r][l.c] == Woutside) {
    return;
  }

  Size t_blocksize;
  t_blocksize.set(xvars.stretch_x(WSQUARE_WIDTH),xvars.stretch_y(WSQUARE_HEIGHT));
  

  if (unionSquares[l.r][l.c]) {
    if (unionSquares[l.r][l.c]->type != UN_MOVER) {
      assert(map[l.r][l.c] == Wempty);
    }
  
    // Draw poster.
    if (unionSquares[l.r][l.c]->type == UN_POSTER) {
      // No posters in reduced-drawing mode.
      if (!reduceDraw) {
        HRESULT t_result;
        CRect t_srcrect(unionSquares[l.r][l.c]->pSquare.loc.c * WSQUARE_WIDTH,
                        unionSquares[l.r][l.c]->pSquare.loc.r * WSQUARE_HEIGHT,
                        (unionSquares[l.r][l.c]->pSquare.loc.c + 1) * WSQUARE_WIDTH,
                        (unionSquares[l.r][l.c]->pSquare.loc.r + 1) * WSQUARE_HEIGHT);
        CRect t_surfrect = xvars.stretch_rect(t_srcrect);

        int t_index = unionSquares[l.r][l.c]->pSquare.poster;
        Pos t_surfpos;
        LPDIRECTDRAWSURFACE t_surf = 
          xvars.m_surfaceManager->lookup(t_surfpos,
                                         (void*)posters[t_index].id);
        // Adjust t_srcrect for SurfaceManager offset.
        t_surfrect.OffsetRect(t_surfpos.x,t_surfpos.y);

        t_result = xvars.Blt(buffer,xvars.stretch_x(x),xvars.stretch_y(y),
                             t_surf,t_surfrect,True);
        DHRESULT(t_result,return);
      }
    }
    // Draw door.
    else if (unionSquares[l.r][l.c]->type == UN_DOOR) {
      int topBottom = unionSquares[l.r][l.c]->dSquare.topBottom;
      HRESULT t_result;
      int t_doorIndex = themes[themeIndex].doorBase + topBottom;

      Pos t_surfpos;
      LPDIRECTDRAWSURFACE t_surf = 
        xvars.m_surfaceManager->lookup(t_surfpos,(void*)doorPixmapBits[t_doorIndex]);
      t_result = xvars.Blt(buffer,
                           Pos(xvars.stretch_x(x),
                               xvars.stretch_y(y)),
                           t_surf,t_surfpos,t_blocksize
#ifdef W_DOORS_TRANSPARENT
                           ,True
#endif
                           );
      if (!xvars.hresultSuccess(t_result)) {
        return;
      }
    }
        
    // Draw mover square.
    else if (unionSquares[l.r][l.c]->type == UN_MOVER) {
      // Special case for bottom of ladders, just draw the bg wall first.
      if (map[l.r][l.c] == Wwall) {
        // Hack, rip out union square and draw just wall.
        UnionSquare *tmp = unionSquares[l.r][l.c];
        unionSquares[l.r][l.c] = NULL;
        // go recursive
        draw_square(buffer,xvars,0,l,x,y,reduceDraw);
        unionSquares[l.r][l.c] = tmp;
      }

      // Draw moverSquare background
      HRESULT t_result;
      int t_index = themes[themeIndex].moverSquareBase 
        + unionSquares[l.r][l.c]->mSquare.orientation;

      Pos t_surfpos;
      LPDIRECTDRAWSURFACE t_surf = 
        xvars.m_surfaceManager->lookup(t_surfpos,
                                       (void*)moverSquarePixmapBits[t_index]);
      // No trans blt if reduceDraw.
      t_result = xvars.Blt(buffer,
                           Pos(xvars.stretch_x(x),
                               xvars.stretch_y(y)),
                           t_surf,t_surfpos,t_blocksize,
                           !reduceDraw);
      DHRESULT(t_result,return);
    }
    else {
      assert(0);
    }
  }

  else if (map[l.r][l.c] == Wempty) {
    ;  //nothing to do!
  }

  // Not unionSquare, draw regular square.
  else {
    HRESULT t_result;
    int t_index = themes[themeIndex].blockIndices[map[l.r][l.c]];

    Pos t_surfpos;
    LPDIRECTDRAWSURFACE t_surf = 
      xvars.m_surfaceManager->lookup(t_surfpos,(void*)blocksBits[t_index]);


    // Never use transparent BLT for reduceDraw.
    Boolean useTransparent = reduceDraw ? False : blockUseTransparent[t_index];

    t_result = xvars.Blt(buffer,
                         Pos(xvars.stretch_x(x),
                             xvars.stretch_y(y)),
                         t_surf,t_surfpos,t_blocksize,
                         useTransparent);
    DHRESULT(t_result,return);
  }
}



void World::draw_mover(LPDIRECTDRAWSURFACE buffer,Xvars &xvars,int,
                       MoverP,int x,int y) {
  int t_index = themes[themeIndex].moverIndex;

  Pos t_surfpos;
  LPDIRECTDRAWSURFACE t_surf = 
    xvars.m_surfaceManager->lookup(t_surfpos,(void*)moverPixmapBits[t_index]);

  HRESULT t_result = 
    xvars.Blt(buffer,
              Pos(xvars.stretch_x(x),xvars.stretch_y(y)),
              t_surf,t_surfpos,xvars.stretch_size(moverSize),True);
  DHRESULT(t_result,return);
}



// Helper to both add surfaces to the surface manager and load bitmaps
// onto the surfaces.
static void World_init_x_helper(Xvars& xvars,BitmapSpec* specs,int bitsNum,
                                const char* errorMsg,IXCommand command,void*,
                                Boolean srcBlt = False) {
  for (int n = 0; n < bitsNum; n++) {
    // Just add to surface Manager.
    if (command == IX_ADD) {
      DDCOLORKEY* cKey = srcBlt ? &xvars.XVARS_COLORKEY : NULL;
      xvars.m_surfaceManager->add((void*)specs[n].id,specs[n].size,cKey);
      continue;
    }
    
    // Do initialization if IX_INIT or IX_LIST.
    assert(command == IX_INIT || command == IX_LIST);
    Pos surfPos;
    LPDIRECTDRAWSURFACE surf = 
      xvars.m_surfaceManager->lookup(surfPos,(void*)specs[n].id);
    if (!xvars.load_surface(surf,CPoint(surfPos.x,surfPos.y),specs[n].id)) {
      ostrstream str;
      str << "World::init_x: Couldn't load " << errorMsg << ends;
      COUTRESULT(str.str());
      delete str.str();
      return;
    }    
  }
}



// Wrapper for the other World_init_x_helper.
static void World_init_x_helper(Xvars& xvars,CMN_BITS_ID* bits,Size size,int bitsNum,
                                const char* errorMsg,IXCommand command,void* arg,
                                Boolean srcBlt = False) {
  // Allocating and destroying some extra memory, but fuck it.
  // Worth it to avoid duplicate code.
  BitmapSpec* specs = new BitmapSpec[bitsNum];
  assert(specs);
  for (int n = 0; n < bitsNum; n++) {
    specs[n].id = bits[n];
    specs[n].size = size;
  }
  World_init_x_helper(xvars,specs,bitsNum,errorMsg,command,arg,srcBlt);
  delete [] specs;
}



/* EFFECTS: Replace the transparent portions of all the pixmaps pointed to by
   bits with the given color.  Only do anything for the entries for which
   doIt[] is True.  All pixmaps must be of the given (stretched) size. */
void World_fill_background(Xvars& xvars,CMN_BITS_ID* bits,
                           const Size& size,int bitsNum,
                           DWORD color) {
  LPDIRECTDRAWSURFACE scratch = 
    xvars.create_surface(size.width,size.height,NULL);
  if (!scratch) {
    return;
  }

  CRect blockRect(0,0,size.width,size.height);
  for (int n = 0; n < bitsNum; n++) {
    // Fill scratch with color.
    xvars.color_fill(scratch,blockRect,False);

    // Copy bits[n] onto scratch using trans Blt.
    Pos surfPos;
    LPDIRECTDRAWSURFACE surf = xvars.m_surfaceManager->lookup(surfPos,(void*)bits[n]);
    xvars.Blt(scratch,0,0,surf,
              CRect(surfPos.x,surfPos.y,surfPos.x + size.width,surfPos.y + size.height),
              True);

    // Now copy scratch back onto surf without trans Blt.
    xvars.Blt(surf,surfPos.x,surfPos.y,scratch,
              blockRect,
              False);
  }
  scratch->Release();
}



void World::init_x(Xvars &xvars,IXCommand command,void* arg) {
  // IX_LIST also does initialization.
  if ((command == IX_INIT || command == IX_LIST) && xvars.is_valid(xValid)) {
    return;
  }


  // Blocks
  Size blockSize;
  blockSize.set(xvars.stretch_x(WSQUARE_WIDTH),
                xvars.stretch_y(WSQUARE_HEIGHT));    
  // Don't really need to store all the world blocks with key-src blitting, only
  // certain ones.
  World_init_x_helper(xvars,blocksBits,blockSize,W_ALL_BLOCKS_NUM,
                      "world block",command,arg,True);
  // Want block backgrounds to be black
  if (xvars.get_reduce_draw() && command == IX_INIT) {
    World_fill_background(xvars,blocksBits,blockSize,W_ALL_BLOCKS_NUM,
                          xvars.m_black);
  }

  // Door blocks
  World_init_x_helper(xvars,doorPixmapBits,blockSize,W_ALL_DOORS_NUM,
                     "door block",command,arg
#ifdef W_DOORS_TRANSPARENT
                      ,True
#endif
                      );

  // Mover square
  World_init_x_helper(xvars,moverSquarePixmapBits,blockSize,
                      W_ALL_MOVER_SQUARES_NUM,
                      "mover square",command,arg,True);
  // Want mover square backgrounds to be black
  if (xvars.get_reduce_draw() && command == IX_INIT) {
    World_fill_background(xvars,moverSquarePixmapBits,blockSize,
                          W_ALL_MOVER_SQUARES_NUM,
                          xvars.m_black);
  }

  // Mover
  World_init_x_helper(xvars,moverPixmapBits,xvars.stretch_size(moverSize),
                      W_ALL_MOVERS_NUM,
                      "mover",command,arg,True);

  // Don't allocate any of the following, since we don't draw them in 
  // reduceDraw mode.
  if (!xvars.get_reduce_draw()) {
    // Posters, uses key src blitting 
    World_init_x_helper(xvars,posters,W_ALL_POSTERS_NUM,
                        "poster",command,arg,True);

    // Outsides
    World_init_x_helper(xvars,outsides,W_ALL_OUTSIDES_NUM,
                        "outside bitmap",command,arg);

    // Backgrounds
    World_init_x_helper(xvars,backgrounds,W_ALL_BACKGROUNDS_NUM,
                        "background bitmap",command,arg);
  }

  if (command == IX_INIT || command == IX_LIST) {
    xvars.mark_valid(xValid);
  }
}



// Helper function to generate an XPM image for a bitmap in the World.
// if animNum == -1, don't add suffix numbering the bitmap.
Boolean World_generate_xpm_helper(Xvars& xvars,CMN_BITS_ID bits,int animNum,
                               const char* xpmDir,const char* varRoot,
                               Boolean halfSize) {
  // Find the pixmap in the SurfaceManager.
  Area area;
  const DDCOLORKEY* cKey;
  LPDIRECTDRAWSURFACE surf = 
    xvars.m_surfaceManager->lookup(area,cKey,(void*)bits);
  
  // Name of variable, e.g. "block_1".
  ostrstream varName;
  varName << varRoot;
  if (animNum != -1) {
    varName << '_' << animNum;
  }
  varName << ends;

  // Name of file.
  CString fullName(xpmDir);
  fullName += varName.str();
  fullName += ".xpm";

  // Finally.  Write the bastard.
  Boolean ret = xvars.write_xpm_file(fullName,varName.str(),
                                     surf,area,cKey,halfSize);
  delete varName.str();

  return ret;
}



Boolean World::generate_xpm(Xvars& xvars,const char* genDir,Boolean halfSize) {
  if (!xvars.is_valid(xValid)) {
    init_x(xvars,IX_INIT,NULL);
  }

  CString dir(genDir);
  dir += "world/";
  if (!Utils::is_dir(dir)) {
    Boolean ret = Utils::mkdir(dir);
    if (!ret) {
      return False;
    }
  }

  // The blocks in the world.
  int n;
  for (n = 0; n < W_ALL_BLOCKS_NUM; n++) {
    if (!World_generate_xpm_helper(xvars,blocksBits[n],
                                   n,dir,"block",halfSize)) {
      return False;
    }
  }

  // Backgrounds.
  for (n = 0; n < W_ALL_BACKGROUNDS_NUM; n++) {
    if (!World_generate_xpm_helper(xvars,backgrounds[n].id,
                                   n,dir,"background",halfSize)) {
      return False;
    }
  }

  // Outsides.
  for (n = 0; n < W_ALL_OUTSIDES_NUM; n++) {
    if (!World_generate_xpm_helper(xvars,outsides[n].id,
                                   n,dir,"outside",halfSize)) {
      return False;
    }
  }

  // Door.
  for (n = 0; n < W_ALL_DOORS_NUM; n++) {
    if (!World_generate_xpm_helper(xvars,doorPixmapBits[n],
                                   n,dir,"door",halfSize)) {
      return False;
    }
  }

  // Mover square.
  for (n = 0; n < W_ALL_MOVER_SQUARES_NUM; n++) {
    if (!World_generate_xpm_helper(xvars,moverSquarePixmapBits[n],
                                   n,dir,"mover_square",halfSize)) {
      return False;
    }
  }

  // Mover.
  for (n = 0; n < W_ALL_MOVERS_NUM; n++) {
    if (!World_generate_xpm_helper(xvars,moverPixmapBits[n],
                                   n,dir,"mover",halfSize)) {
      return False;
    }
  }

  // Posters.
  for (n = 0; n < W_ALL_POSTERS_NUM; n++) {
    if (!World_generate_xpm_helper(xvars,posters[n].id,
                                   n,dir,"poster",halfSize)) {
      return False;
    }
  }

  return True;
}



Boolean Locator::generate_xpm(Xvars& xvars,const char* genDir,
                              Boolean halfSize) {
  if (!xvars.is_valid(xValid)) {
    init_x(xvars,IX_INIT,NULL);
  }

  CString xpmDir(genDir);
  xpmDir += "locator/";
  if (!Utils::is_dir(xpmDir)) {
    Boolean ret = Utils::mkdir(xpmDir);
    if (!ret) {
      return False;
    }
  }

  // Tick marks.
  for (TickType tt = 0; tt < TICK_MAX; tt++) {
    for (int hp = 0; hp < CO_DIR_HALF_PURE; hp++) {
      // Only write out the bases, X11 will auto-generate the rest.
      Dir dir = Coord::half_pure_to_pure(hp);
      if (!Transform2D::is_base(dir,NULL)) {
        continue;
      }

      // Some of this code lifted from World_generate_xpm_helper, maybe merge
      // the two.

      // Find the pixmap in the SurfaceManager.
      void* key = compute_tick_key(tt,dir);
      Area area;
      const DDCOLORKEY* cKey;
      LPDIRECTDRAWSURFACE surf = 
        xvars.m_surfaceManager->lookup(area,cKey,key);
  
      // Name of variable, e.g. "tick_0_7".
      ostrstream varName;
      varName << "tick_" << tt << '_' << hp << ends;

      // Name of file.
      CString fullName(xpmDir);
      fullName += varName.str();
      fullName += ".xpm";

      // Finally.  Write the bastard.
      Boolean val = xvars.write_xpm_file(fullName,varName.str(),
                                         surf,area,cKey,halfSize);
      delete varName.str();

      if (!val) {
        ostrstream err;
        err << "Failed to write tick tt=" << tt << " hp=" << hp << " fullName=" << fullName << ends;
        COUTRESULT(err.str());
        delete err.str();
        return False;
      }
    }
  }

  return True;
}



void* Locator::compute_tick_key(int tickType,Dir dir) {
  assert(tickType < TICK_MAX);
  assert(dir >= CO_R && dir < CO_DIR_MAX);
  assert((dir - CO_R) % 2 == 0); // Must be convertable to a "half-pure" direction.
  u_int ret = (S_MAN_OL_NONCE << 24);
  ret |= (u_char)tickType << 8;
  ret |= (u_char)dir;
  return (void*)ret;
}



// Returns whether successful.
Boolean Locator::draw_tick(TickType tt,LPDIRECTDRAWSURFACE window,
                           Xvars &xvars,int,
                           const Size &windowSize,
                           Dir tickDir,int offset) {
  int n = Coord::pure_to_half_pure(tickDir);
  HRESULT t_result;
  CRect t_destrect;

  // Lookup tick bitmap in SurfaceManager.
  Pos srcPos;
  LPDIRECTDRAWSURFACE surf = 
    xvars.m_surfaceManager->lookup(srcPos,compute_tick_key(tt,tickDir));
  CRect t_srcrect;
  t_srcrect.SetRect(srcPos.x,srcPos.y,
                    srcPos.x + xvars.stretch_x(tickSizes[n].width),
                    srcPos.y + xvars.stretch_y(tickSizes[n].height));


  int offset2;  // Takes width/height of tick into account.  
  switch (tickDir) {
    case CO_R:
      offset2 = offset - (xvars.stretch_y(tickSizes[n].height) >> 1);
      t_destrect.SetRect(windowSize.width 
                         - xvars.stretch_x(tickSizes[n].width),offset2,
                         windowSize.width,offset2 
                         + xvars.stretch_y(tickSizes[n].height));
      break;
    case CO_DN_R:
      t_destrect.SetRect(windowSize.width 
                         - xvars.stretch_x(tickSizes[n].width),
                         windowSize.height 
                         - xvars.stretch_y(tickSizes[n].height),
                         windowSize.width,windowSize.height);
      break;
    case CO_DN:
      offset2 = offset - (xvars.stretch_x(tickSizes[n].width) >> 1);
      t_destrect.SetRect(offset2,windowSize.height 
                         - xvars.stretch_y(tickSizes[n].height),
                         offset2 + xvars.stretch_x(tickSizes[n].width),
                         windowSize.height);
      break;
    case CO_DN_L:
      t_destrect.SetRect(0,windowSize.height 
                         - xvars.stretch_y(tickSizes[n].height),
                         xvars.stretch_x(tickSizes[n].width),
                         windowSize.height);
      break;
    case CO_L:
      offset2 = offset - (xvars.stretch_y(tickSizes[n].height) >> 1);
      t_destrect.SetRect(0,offset2,
                         xvars.stretch_x(tickSizes[n].width),
                         offset2 + xvars.stretch_y(tickSizes[n].height));
      break;
    case CO_UP_L:
      t_destrect.SetRect(0,0,xvars.stretch_x(tickSizes[n].width),
                         xvars.stretch_y(tickSizes[n].height));
      break;
    case CO_UP:
      offset2 = offset - (xvars.stretch_x(tickSizes[n].width) >> 1);
      t_destrect.SetRect(offset2,0,
                         offset2 + xvars.stretch_x(tickSizes[n].width),
                         xvars.stretch_y(tickSizes[n].height));
      break;
    case CO_UP_R:      
      t_destrect.SetRect(windowSize.width 
                         - xvars.stretch_x(tickSizes[n].width),0,
                         windowSize.width,
                         xvars.stretch_y(tickSizes[n].height));
      break;
    default:
      assert(0);
      
  }
  
  t_result = xvars.Blt(window,t_destrect.left,t_destrect.top,
                       surf,t_srcrect,True);
  DHRESULT(t_result,return False);
  
  return True;
}



void Locator::init_x(Xvars &xvars,IXCommand command,void* arg) {
  int hp;  // The "half-pure" direction.
  TickType tt;
  
  if (command == IX_ADD) {
    // Tick marks.
    for (tt = 0; tt < TICK_MAX; tt++) {
      for (hp = 0; hp < CO_DIR_HALF_PURE; hp++) {
        Dir dir = Coord::half_pure_to_pure(hp);
        void* key = compute_tick_key(tt,dir);
        xvars.m_surfaceManager->add(key,xvars.stretch_size(tickSizes[hp]),
                                    &xvars.XVARS_COLORKEY);
      }
    }

    // Scratch buffer.
    Size scratchSize;
    scratchSize.set(xvars.stretch_x(OL_GRID_COL_MAX * WSQUARE_WIDTH),
                    xvars.stretch_y(OL_GRID_ROW_MAX * WSQUARE_HEIGHT));
    xvars.m_surfaceManager->add(OL_SCRATCH_KEY,scratchSize,
                                &xvars.XVARS_COLORKEY);
    return;
  }

  // Ok to call multiple times.
  if (xvars.is_valid(xValid)) {
    return;
  }

  // Never called with IX_LIST, at least not for now.
  assert(command == IX_INIT);

  // Load base tick bitmaps onto surface.
  // Also compute max tick size while we're doing a loop anyway.
  Size tickMax; // stretched coordinates.
  tickMax.set_zero();
  for (hp = 0; hp < CO_DIR_HALF_PURE; hp++) {
    tickMax.width = 
      Utils::maximum(tickMax.width,xvars.stretch_x(tickSizes[hp].width));
    tickMax.height = 
      Utils::maximum(tickMax.height,xvars.stretch_y(tickSizes[hp].height));

    for (tt = 0; tt < TICK_MAX; tt++) {
      Dir dir = Coord::half_pure_to_pure(hp);
      if (Transform2D::is_base(dir,NULL)) {
        assert(tickPixmapBits[tt][hp] != PH_AUTO_GEN);
        void* key = compute_tick_key(tt,dir);
        Pos pos;
        LPDIRECTDRAWSURFACE surf = 
          xvars.m_surfaceManager->lookup(pos,key);
        xvars.load_surface(surf,CPoint(pos.x,pos.y),tickPixmapBits[tt][hp]);
      }
    }
  }


  // Load auto-generated ticks.
  LPDIRECTDRAWSURFACE scratch = 
    xvars.create_surface(tickMax.width,tickMax.height,NULL);
  for (tt = 0; tt < TICK_MAX; tt++) {
    for (hp = 0; hp < CO_DIR_HALF_PURE; hp++) {
      // Need "pure" dire for Transform2D, not 
      Dir dir = Coord::half_pure_to_pure(hp);
      if (!Transform2D::is_base(dir,NULL)) {
        assert(tickPixmapBits[tt][hp] == PH_AUTO_GEN);
        Dir base = Transform2D::get_base(dir,NULL);
        int baseHP = Coord::pure_to_half_pure(base);
        int tNum;
        const TransformType* transforms = 
          Transform2D::get_transforms(tNum,dir,NULL);
    
        void* srcKey = compute_tick_key(tt,base);
        void* destKey = compute_tick_key(tt,dir);
        Pos srcPos, destPos;
        LPDIRECTDRAWSURFACE srcSurf = 
          xvars.m_surfaceManager->lookup(srcPos,srcKey);
        LPDIRECTDRAWSURFACE destSurf = 
          xvars.m_surfaceManager->lookup(destPos,destKey);

        xvars.generate_pixmap_from_transform(destSurf,destPos,
                                             srcSurf,srcPos,     
                                             xvars.stretch_size(tickSizes[baseHP]),
                                             scratch,
                                             transforms,tNum);
      }
    }
  }   
  scratch->Release();


  // Don't need to do anything to initialize scratch buffer.

  xvars.mark_valid(xValid);
}



LPDIRECTDRAWSURFACE Locator::get_scratch_buffer(Pos& pos,Xvars& xvars,int) {
  return xvars.m_surfaceManager->lookup(pos,OL_SCRATCH_KEY);
}



void Protection::init_x(Xvars &xvars,IXCommand command,void* arg,
                        const ProtectionContext &prc,ProtectionXdata &pXdata) {
  if (command == IX_ADD) {
    return;
  }
  
  if (xvars.is_valid(pXdata.valid)) {
    return;
  }
  assert(command == IX_INIT || command == IX_LIST);
  if (!pXdata.color.m_hObject) {
    pXdata.color.CreatePen(PS_SOLID,1,prc.colorName); 
  }
  xvars.mark_valid(pXdata.valid);
}



void Protection::draw(LPDIRECTDRAWSURFACE window,Xvars &xvars,int,
                      const Area &bufArea) {
  if (!xvars.is_valid(pXdata->valid)) {
    init_x(xvars,IX_INIT,NULL,*prc,*pXdata);
  }
  Pos pos;
  Size size;
  area.get_rect(pos,size);
  Size offset = area - bufArea;
  HDC t_temphdc;
  if (!xvars.GetDC(window,&t_temphdc)){
    return;
  }
  CDC *t_dc=CDC::FromHandle(t_temphdc);
  t_dc->SelectStockObject(NULL_BRUSH);//clear
  CPen *t_oldpen;
  t_oldpen=t_dc->SelectObject(&pXdata->color);
  t_dc->Rectangle(xvars.stretch_x(offset.width),xvars.stretch_y(offset.height),xvars.stretch_x(offset.width+size.width),xvars.stretch_y(offset.height+size.height));
  t_dc->SelectObject(t_oldpen);
  HRESULT t_result=window->ReleaseDC(t_temphdc);
  if (!xvars.hresultSuccess(t_result))
    return;
}



void Moving::get_pixmap_mask(Xvars &xvars,int,CMN_IMAGEDATA &pixmap,
                             Dir dir,int animNum) {
  void* key = compute_key(dir,animNum,False,mc,movingXdata);
  
  // Look up surfaces in the SurfaceManager.
  pixmap.pixmap = xvars.m_surfaceManager->lookup(pixmap.pixmapOffset,key);
  if (mc->invisibility) {
    void* maskKey = compute_key(dir,animNum,True,mc,movingXdata);
    pixmap.mask = xvars.m_surfaceManager->lookup(pixmap.maskOffset,maskKey);
  }
  else {
    pixmap.mask = NULL;
    pixmap.maskOffset.set_zero();
  }
}



void Moving::init_x(Xvars &xvars,IXCommand command,void* arg,
                    const MovingContext &mc,MovingXdata &movingXdata) {
  check_auto_generated(mc);

  int m,n;

  // Just add the surfaces to the SurfaceManager.
  if (command == IX_ADD) {
    // Sanity check.
    assert(xvars.m_surfaceManager && 
           !xvars.m_surfaceManager->is_add_completed());
    
    // Compute hash key and add surface request to SurfaceManager for each 
    // anim frame and mask.
    for (n = 0; n < CO_DIR_MAX; n++) { 
      for (m = 0; m < mc.animMax[n]; m++) {
        void* key = compute_key(n,m,False,&mc,&movingXdata);
        xvars.m_surfaceManager->add(key,xvars.stretch_size(mc.sizes[n]),
                                    &xvars.XVARS_COLORKEY);
        // Only generate masks if this object supports invisibility.
        if (mc.invisibility) {
          void* maskKey = compute_key(n,m,True,&mc,&movingXdata);
          xvars.m_surfaceManager->add(maskKey,xvars.stretch_size(mc.sizes[n]),
                                      &xvars.XVARS_COLORKEY_MASKS);
        }
      }
    }

    // Done adding surfaces.
    return;
  }


  // List all pixmaps in the SurfaceManager.
  // This code is before the xvars.is_valid() check so that it is always 
  // run, even if the graphics have been initialized.  But, we fall through
  // to the IX_INIT code so that graphics are always initialized by the 
  // time IX_LIST returns.
  if (command == IX_LIST) {
    // For IX_LIST, the argument is a PtrList.
    PtrList* ptrList = (PtrList*)arg;

    // Loop over all dirs.
    for (n = 0; n < CO_DIR_MAX; n++) {
      // Don't add auto-generated pixmaps.
      if (mc.pixmapBits[n][0] == PH_AUTO_GEN) {
        continue;
      }

      // Loop over all animation frames.
      for (m = 0; m < mc.animMax[n]; m++) {
        // Create IXPixmapInfo for this pixmap.
        void* key = compute_key(n,m,False,&mc,&movingXdata);
        IXPixmapInfo* pInfo = new IXPixmapInfo;
        assert(pInfo);
        pInfo->key = key;
        pInfo->dir = n;
        pInfo->animNum = m;
        ptrList->add((void*)pInfo);

        // Don't add masks.
      } // for m
    } // for n
  } // if IX_LIST


  if (xvars.is_valid(movingXdata.valid)) {
    return;
  }
  assert(command == IX_INIT || command == IX_LIST);
  assert(xvars.m_surfaceManager->is_add_completed());


  // Load all non-autogenerated bitmaps onto surfaces, possibly stretching.	
  // This should load ALL the base bitmaps.
  for (n = 0; n < CO_DIR_MAX; n++) {
    if (mc.animMax[n] > 0 && mc.pixmapBits[n][0] != PH_AUTO_GEN) {
      for (m = 0; m < mc.animMax[n]; m++) {
        void* key = compute_key(n,m,False,&mc,&movingXdata);
        Pos pos;
        LPDIRECTDRAWSURFACE surf = xvars.m_surfaceManager->lookup(pos,key);
        CPoint point(pos.x,pos.y);

        // Don't stretch if already hiRes bitmaps.
        if (mc.hiRes) {
          xvars.load_surface(surf,point,mc.pixmapBits[n][m]);
        }
        else {
          // Shouldn't be any of these left.
          assert(0);
          xvars.load_stretch_surface(surf,point,mc.pixmapBits[n][m]);
        }
      }
    }
  }


  // For efficiency, only create one scratch surface for doing the
  // transformations.  Pass it into generate_pixmap_from_transform().
  LPDIRECTDRAWSURFACE t_scratch = 
    xvars.create_surface(xvars.stretch_x(mc.physicalContext.sizeMax.width),
                         xvars.stretch_y(mc.physicalContext.sizeMax.height),
                         NULL);
  // Auto generate the PH_AUTO_GEN bitmaps.
  for (n = 0; n < CO_DIR_MAX; n++) {
    if (mc.animMax[n] > 0 && mc.pixmapBits[n][0] == PH_AUTO_GEN) {
      Dir base = Transform2D::get_base(n,mc.transformOverride);
      int tNum;
      const TransformType* transforms = 
        Transform2D::get_transforms(tNum,n,mc.transformOverride);
      for (m = 0; m < mc.animMax[n]; m++) {
        void* srcKey = compute_key(base,m,False,&mc,&movingXdata);
        void* destKey = compute_key(n,m,False,&mc,&movingXdata);
        Area srcArea, destArea;
        const DDCOLORKEY* dummy;
        LPDIRECTDRAWSURFACE srcSurf = 
          xvars.m_surfaceManager->lookup(srcArea,dummy,srcKey);
        LPDIRECTDRAWSURFACE destSurf = 
          xvars.m_surfaceManager->lookup(destArea,dummy,destKey);

        xvars.generate_pixmap_from_transform(destSurf,destArea.get_pos(),
                                             srcSurf,srcArea.get_pos(),     
                                             srcArea.get_size(),
                                             t_scratch,
                                             transforms,tNum);
      }
    }
  }
  t_scratch->Release();


  // Create masks by copying color-key data from the pixmaps
  // 
  // Only generate masks if this object supports invisibility.
  if (mc.invisibility) {
    for (n = 0; n < CO_DIR_MAX; n++) {
      for (m = 0; m < mc.animMax[n]; m++) {
        void* key = compute_key(n,m,False,&mc,&movingXdata);
        void* maskKey = compute_key(n,m,True,&mc,&movingXdata);
        Area area, maskArea;
        const DDCOLORKEY* dummy;
        LPDIRECTDRAWSURFACE surf = xvars.m_surfaceManager->lookup(area,dummy,key);
        LPDIRECTDRAWSURFACE maskSurf = xvars.m_surfaceManager->lookup(maskArea,dummy,maskKey);
        assert(area.get_size() == maskArea.get_size());

        xvars.generate_mask_from_pixmap(maskSurf,maskArea.get_pos(),
                                        surf,area.get_pos(),                                      
                                        area.get_size());
      } // for m
    } // for n
  } // invisibility


  // Mark Xdata as valid for this version.
  xvars.mark_valid(movingXdata.valid);
}



void Moving::draw(LPDIRECTDRAWSURFACE window,Xvars &xvars,int,
                  const Area &bufArea) {    
  // Initialize graphics if necessary.
  if (!xvars.is_valid(movingXdata->valid)) {
    init_x(xvars,IX_INIT,NULL,*mc,*movingXdata);
  }

  Pos pos;
  Size size;
  area.get_rect(pos,size);
  Size offset = area - bufArea;
  LPDIRECTDRAWSURFACE t_surface(NULL);
  Pos t_offset;
  CRect t_destrect;

  // Fills pixmap and mask
  CMN_IMAGEDATA imagedata;
  get_pixmap_mask(xvars,0,imagedata,dir,movingAnimNum);
  HRESULT t_result;

  // BUG?: This looks like it will fail if size.height <= 3.
  if (is_invisible()) {
    // We should never have been made invisible if we don't support it.
    assert(mc->invisibility);    
    
    LocatorP locator = get_locator();
    t_surface = locator->get_scratch_buffer(t_offset,xvars,0);
    if (!t_surface) {
      assert(FALSE);
      return;
    }
    // Which way to distort the image.
    // We will draw the image twice to get the wrap-around.
    // Copy portion of area into scratch buffer.

    // Draw top 3 pixel rows of window into bottom 3 of t_surface.
    Area stretchedArea;
    //bufArea
    CRect bltRect(offset.width, offset.height,
                  offset.width + size.width, offset.height + 3);
    bltRect = xvars.contain_rect(bltRect,bufArea.get_size().width,bufArea.get_size().height);

    if (bltRect.Height() && bltRect.Width()){
      t_result = xvars.Blt(t_surface, t_offset.x, t_offset.y + xvars.stretch_y(size.height - 3) ,
                         window,xvars.stretch_rect(bltRect));
      DHRESULT(t_result,return);
    }

    // Draw all but top 3 rows of window into t_surface
    bltRect.SetRect(offset.width,offset.height + 3,
                  offset.width + size.width,offset.height + size.height);
    bltRect = xvars.contain_rect(bltRect,bufArea.get_size().width,bufArea.get_size().height);
    if (bltRect.Height() && bltRect.Width()){
      t_result = xvars.Blt(t_surface,t_offset.x,t_offset.y,window, xvars.stretch_rect(bltRect));
      DHRESULT(t_result,return);
    }
    // Whack the mask on t_surface to draw the outside portion pink.
    t_result = xvars.Blt(t_surface,t_offset,
                         imagedata.mask,imagedata.maskOffset,
                         xvars.stretch_size(size),
                         True);
    DHRESULT(t_result,return);
  }
  else {
    t_surface = imagedata.pixmap;
    t_offset = imagedata.pixmapOffset;
  }

  // Copy appropriate bitmap onto window.
  Pos destPos(xvars.stretch_x(offset.width),xvars.stretch_y(offset.height));
  t_result = xvars.Blt(window,destPos,
                       t_surface,
                       t_offset,xvars.stretch_size(size),
                       True);
  DHRESULT(t_result,return);
}
