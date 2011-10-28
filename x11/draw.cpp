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

// "draw.cpp"
// All init_x(), and draw() functions.  Specific to X11.


// Include Files
#include "utils.h"
#include "coord.h"
#include "world.h"
#include "locator.h"
#include "physical.h"
#include "actual.h"
// Only for Viewport::get_reduce_draw().  Should put get_reduce_draw() 
// in Xvars instead.
#include "viewport.h"  



// Draw functions for World
void World::draw_square(Drawable buffer,Xvars &xvars,int dpyNum,const Loc &l,
                        int x,int y,Boolean reduceDraw) {
  if (!inside(l)) {
    // World::draw_outside() will take care of it.
    return;
  }
  
  int blockNum = themes[themeIndex].blockIndices[map[l.r][l.c]];
  

  // Draw union square, not regular square.
  if (unionSquares[l.r][l.c]) {
    assert(map[l.r][l.c] == Wempty || map[l.r][l.c] == Wwall);
    
    // Draw poster.
    if (unionSquares[l.r][l.c]->type == UN_POSTER) {
      if (!reduceDraw) {
        PosterSquare *pSquare = &unionSquares[l.r][l.c]->pSquare;
        Pixmap pix = xdata.posterPixmaps[dpyNum][pSquare->poster];
        Pixmap mask = xdata.posterMasks[dpyNum][pSquare->poster];
        if (!pix || !mask) {
          return;
        }
        
        // Now draw using clip mask
        XSetClipMask(xvars.dpy[dpyNum],xvars.gc[dpyNum],mask);
        XSetClipOrigin(xvars.dpy[dpyNum],xvars.gc[dpyNum],
                       xvars.stretch_x(x - pSquare->loc.c * WSQUARE_WIDTH),
                       xvars.stretch_y(y - pSquare->loc.r * WSQUARE_HEIGHT));

        XCopyArea(xvars.dpy[dpyNum],pix,
                  buffer,xvars.gc[dpyNum],
                  xvars.stretch_x(pSquare->loc.c * WSQUARE_WIDTH),
                  xvars.stretch_y(pSquare->loc.r * WSQUARE_HEIGHT),
                  xvars.stretch_x(WSQUARE_WIDTH),
                  xvars.stretch_y(WSQUARE_HEIGHT),
                  xvars.stretch_x(x),
                  xvars.stretch_y(y));
        
        // Restore gc to initial state.
        XSetClipMask(xvars.dpy[dpyNum],xvars.gc[dpyNum],None);
        XSetClipOrigin(xvars.dpy[dpyNum],xvars.gc[dpyNum],0,0);
      } // reduceDraw
    }

    // Draw door.
    else if (unionSquares[l.r][l.c]->type == UN_DOOR) {
      int topBottom = unionSquares[l.r][l.c]->dSquare.topBottom;
      int doorNum = themes[themeIndex].doorBase + topBottom;

#ifdef W_DOORS_TRANSPARENT
      if (!reduceDraw) {
        XSetClipMask(xvars.dpy[dpyNum],xvars.gc[dpyNum],
                     xdata.doorMasks[dpyNum][doorNum]);
        XSetClipOrigin(xvars.dpy[dpyNum],xvars.gc[dpyNum],
                       xvars.stretch_x(x),
                       xvars.stretch_y(y));
      }
#endif

      XCopyArea(xvars.dpy[dpyNum],
                xdata.doorPixmaps[dpyNum][doorNum],
                buffer,xvars.gc[dpyNum],0,0,
                xvars.stretch_x(WSQUARE_WIDTH),
                xvars.stretch_y(WSQUARE_HEIGHT),
                xvars.stretch_x(x),
                xvars.stretch_y(y));

#ifdef W_DOORS_TRANSPARENT
      if (!reduceDraw) {
        XSetClipMask(xvars.dpy[dpyNum],xvars.gc[dpyNum],None);
        XSetClipOrigin(xvars.dpy[dpyNum],xvars.gc[dpyNum],0,0);
      }
#endif
    }

    // Draw moverSquare.
    else if (unionSquares[l.r][l.c]->type == UN_MOVER) {
      // If moversquare goes through a wall, draw the wall first.
      if (map[l.r][l.c] == Wwall) {
        // Hack, rip out union square and draw just wall.
        UnionSquare *tmp = unionSquares[l.r][l.c];
        unionSquares[l.r][l.c] = NULL;
        // go recursive
        draw_square(buffer,xvars,dpyNum,l,x,y,reduceDraw);
        unionSquares[l.r][l.c] = tmp;
      }

      int mSquareNum = themes[themeIndex].moverSquareBase + 
        unionSquares[l.r][l.c]->mSquare.orientation;
      
      // Now draw using clip mask
      if (!reduceDraw) {
        XSetClipMask(xvars.dpy[dpyNum],xvars.gc[dpyNum],
                     xdata.moverSquareMasks[dpyNum][mSquareNum]);
        XSetClipOrigin(xvars.dpy[dpyNum],xvars.gc[dpyNum],
                       xvars.stretch_x(x),
                       xvars.stretch_y(y));
      }
      XCopyArea(xvars.dpy[dpyNum],
                xdata.moverSquarePixmaps[dpyNum][mSquareNum],
                buffer,xvars.gc[dpyNum],0,0,
                xvars.stretch_x(WSQUARE_WIDTH),
                xvars.stretch_y(WSQUARE_HEIGHT),
                xvars.stretch_x(x),
                xvars.stretch_y(y));
      // Restore gc to initial state.
      if (!reduceDraw) {
        XSetClipMask(xvars.dpy[dpyNum],xvars.gc[dpyNum],None);
        XSetClipOrigin(xvars.dpy[dpyNum],xvars.gc[dpyNum],0,0);
      }
    }
    // Unknown UnionSquare.
    else {
      assert(0);
    } 
  }

  // Draw regular square, not union square.
  else {
    // World::draw_background() takes care of drawing empty squares.
    if (blockNum != Wempty) {
      // Only use transparent CopyArea for certain bricks.
      Boolean useTransparent = 
        blockUseTransparent[map[l.r][l.c]] && !reduceDraw;
      
      if (useTransparent) {
        XSetClipMask(xvars.dpy[dpyNum],xvars.gc[dpyNum],
                     xdata.blockMasks[dpyNum][blockNum]);
        XSetClipOrigin(xvars.dpy[dpyNum],xvars.gc[dpyNum],
                       xvars.stretch_x(x),
                       xvars.stretch_y(y));
      }
      XCopyArea(xvars.dpy[dpyNum],
                xdata.blockPixmaps[dpyNum][blockNum],
                buffer,xvars.gc[dpyNum],0,0,
                xvars.stretch_x(WSQUARE_WIDTH),
                xvars.stretch_y(WSQUARE_HEIGHT),
                xvars.stretch_x(x),
                xvars.stretch_y(y));
      if (useTransparent) {
        // Restore gc to initial state.
        XSetClipMask(xvars.dpy[dpyNum],xvars.gc[dpyNum],None);
        XSetClipOrigin(xvars.dpy[dpyNum],xvars.gc[dpyNum],0,0);
      }
    }
  }
}



void World::draw_mover(CMN_DRAWABLE buffer,Xvars &xvars,int dpyNum,
                       MoverP /* mover */,int x,int y) {
  int moverNum = themes[themeIndex].moverIndex;

  XSetClipMask(xvars.dpy[dpyNum],xvars.gc[dpyNum],
               xdata.moverMasks[dpyNum][moverNum]);
  XSetClipOrigin(xvars.dpy[dpyNum],xvars.gc[dpyNum],
                 xvars.stretch_x(x),
                 xvars.stretch_y(y));
  XCopyArea(xvars.dpy[dpyNum],
            xdata.moverPixmaps[dpyNum][moverNum],
            buffer,xvars.gc[dpyNum],0,0,
            xvars.stretch_x(moverSize.width),
            xvars.stretch_y(moverSize.height),
            xvars.stretch_x(x),
            xvars.stretch_y(y));

  // Restore gc to initial state.
  XSetClipMask(xvars.dpy[dpyNum],xvars.gc[dpyNum],None);
  XSetClipOrigin(xvars.dpy[dpyNum],xvars.gc[dpyNum],0,0);
}


 
void World::draw_outside_offset(CMN_DRAWABLE dest,
                                Xvars &xvars,int dpyNum,Size sourceOffset,
                                const Area &destArea) {
  if (!xvars.is_valid(xValid)) {
    init_x(xvars,IX_INIT,NULL);
  }

  // Everything is already in window coordinates.

  Pos destPos = destArea.get_pos();
  Size destSize = destArea.get_size();

  XCopyArea(xvars.dpy[dpyNum],xdata.outsidePixmaps[dpyNum][outsideIndex],
            dest,xvars.gc[dpyNum],
            sourceOffset.width,sourceOffset.height,
            destSize.width,destSize.height,
            destPos.x,destPos.y);
}



void World::draw_background(CMN_DRAWABLE buffer,Xvars &xvars,int dpyNum,
                            Area area,Boolean background3D) {
  if (!xvars.is_valid(xValid)) {
    init_x(xvars,IX_INIT,NULL);
  }

  // backgrounds.size should really be given in unstretched coordinates 
  // to avoid having to divide by two.
  Size bSize = backgrounds[backgroundIndex].size;

  // Size of the background in stretched coordinates.
  Size bgSize;
  bgSize.set(xvars.stretch_x(bSize.width >> 1),
             xvars.stretch_y(bSize.height >> 1));

  Pos pos = xvars.stretch_pos(area.get_pos());
  Size size = xvars.stretch_size(area.get_size());

  // Move background at half the speed of the foreground.
  if (background3D) {
    pos.x /= W_BACKGROUNDRATE;
    pos.y /= W_BACKGROUNDRATE;
  }

  // The origin to use for tiling with the background.
  Pos tsOrigin(-(pos.x % bgSize.width),
               -(pos.y % bgSize.height));    

  // Set GC to fill with tile of the background.
  XSetTSOrigin(xvars.dpy[dpyNum],xvars.gc[dpyNum],
               tsOrigin.x,tsOrigin.y);
  XSetFillStyle(xvars.dpy[dpyNum],xvars.gc[dpyNum],
                FillTiled);
  XSetTile(xvars.dpy[dpyNum],xvars.gc[dpyNum],
           xdata.backgroundPixmaps[dpyNum][backgroundIndex]);

  // Fill buffer, tiled with the background.
  XFillRectangle(xvars.dpy[dpyNum],buffer,xvars.gc[dpyNum],
                 0,0,size.width,size.height);

  // Revert GC to former state.
  //
  // Not going to bother reverting the tile in the GC, we don't use
  // it anywhere else anyway.  Only applies when fill_style is FillSolid.
  // Tried setting it to "None" or to the value returned from XGetGCValues
  // on the initial GC.  Both gave errors.
  //
  XSetTSOrigin(xvars.dpy[dpyNum],xvars.gc[dpyNum],
               0,0);
  XSetFillStyle(xvars.dpy[dpyNum],xvars.gc[dpyNum],
                FillSolid);
}



// Fill the background of pixmaps[] (as specified by masks[]) with
// the given color. 
void World_fill_background(Xvars& xvars,int dpyNum,
                           Pixmap pixmaps[],Pixmap masks[],
                           const Size& size,int pixNum,
                           Pixel color) {
  Pixmap scratch = 
    XCreatePixmap(xvars.dpy[dpyNum],xvars.root[dpyNum],
                  size.width,size.height,xvars.depth[dpyNum]);
  if (!scratch) {
    return;
  }
  XSetForeground(xvars.dpy[dpyNum],xvars.gc[dpyNum],color);
  for (int n = 0; n < pixNum; n++) {
    // Fill scratch with color.
    XFillRectangle(xvars.dpy[dpyNum],scratch,xvars.gc[dpyNum],0,0,
                   size.width,size.height);

    // Copy pixmaps[n] onto scratch using appropriate mask.
    XSetClipMask(xvars.dpy[dpyNum],xvars.gc[dpyNum],masks[n]);
    XCopyArea(xvars.dpy[dpyNum],pixmaps[n],scratch,xvars.gc[dpyNum],
              0,0,size.width,size.height,0,0);
    XSetClipMask(xvars.dpy[dpyNum],xvars.gc[dpyNum],None);

    // Now copy scratch back onto pixmaps[n] without using mask.
    XCopyArea(xvars.dpy[dpyNum],scratch,pixmaps[n],xvars.gc[dpyNum],
              0,0,size.width,size.height,0,0);
  }

  XSetForeground(xvars.dpy[dpyNum],xvars.gc[dpyNum],xvars.black[dpyNum]);
}



void World::init_x(Xvars &xvars,IXCommand,void*) {
  for (int dpyNum = 0; dpyNum < xvars.dpyMax; dpyNum++) {
    xdata.background[dpyNum] = 
      xvars.alloc_named_color(dpyNum,Xvars_BACKGROUND);
    
    // Regular blocks.
    int n;
    for (n = 0; n < W_ALL_BLOCKS_NUM; n++) {
      Pixmap* mask = NULL;

      // Only create the mask if necessary.  So, check to see if this block is
      // used as a transparent block in any theme.  Inefficient, but doesn't
      // really matter here.
      for (int block = 0; (mask == NULL) && (block < W_BLOCKS_NUM); block++) {
        if (blockUseTransparent[block]) {
          for (int th = 0; th < W_THEME_NUM; th++) {
            if (themes[th].blockIndices[block] == n) {
              mask = &xdata.blockMasks[dpyNum][n];
              break;
            }
          }
        }
      }
      // Load pixmap and possibly mask.
      Boolean val = 
        xvars.load_pixmap(&xdata.blockPixmaps[dpyNum][n],
                          mask,dpyNum,
                          (char**)blocksBits[n]);
      if (!val) {
        cerr << "Failed to load block " << n << endl;
      }
    } // for n
    // For reduce draw, we won't use masks for XCopyArea, so fill background
    // with black.
    if (Viewport::get_reduce_draw()) {
      Size blockSize;
      blockSize.set(xvars.stretch_x(WSQUARE_WIDTH),
                    xvars.stretch_y(WSQUARE_HEIGHT));
      World_fill_background(xvars,dpyNum,xdata.blockPixmaps[dpyNum],
                            xdata.blockMasks[dpyNum],blockSize,
                            W_ALL_BLOCKS_NUM,
                            xvars.black[dpyNum]);
    }


    // Create posterPixmaps, blocks must be created first.
    //
    // Use W_ALL_POSTERS_NUM to get the title poster.
    for (n = 0; n < W_ALL_POSTERS_NUM; n++) {
      Boolean val = 
        xvars.load_pixmap(&xdata.posterPixmaps[dpyNum][n],
                          &xdata.posterMasks[dpyNum][n],
                          dpyNum,(char**)posters[n].id);
      if (!val) {
        cerr << "Failed to load poster " << n << endl;
      }
    }

    
    // Create doorPixmaps.
    for (n = 0; n < W_ALL_DOORS_NUM; n++) {
#ifdef W_DOORS_TRANSPARENT
      Drawable* mask = &xdata.doorMasks[dpyNum][n];
#elif
      Drawable* mask = NULL;
#endif      
      Boolean val = 
        xvars.load_pixmap(&xdata.doorPixmaps[dpyNum][n],
                          mask,dpyNum,
                          (char**)doorPixmapBits[n]);
      if (!val) {
        cerr << "Failed to load door block " << n << endl;
      }
    }


    // Mover squares.
    for (n = 0; n < W_ALL_MOVER_SQUARES_NUM; n++) {
      Boolean val = 
        xvars.load_pixmap(&xdata.moverSquarePixmaps[dpyNum][n],
                          &xdata.moverSquareMasks[dpyNum][n],
                          dpyNum,
                          (char**)moverSquarePixmapBits[n]);
      if (!val) {
        cerr << "Failed to mover square " << n << endl;
      }
    }
    // For reduce draw, we won't use masks for XCopyArea, so fill background
    // with black.
    if (Viewport::get_reduce_draw()) {
      Size blockSize;
      blockSize.set(xvars.stretch_x(WSQUARE_WIDTH),
                    xvars.stretch_y(WSQUARE_HEIGHT));
      World_fill_background(xvars,dpyNum,xdata.moverSquarePixmaps[dpyNum],
                            xdata.moverSquareMasks[dpyNum],blockSize,
                            W_ALL_MOVER_SQUARES_NUM,
                            xvars.black[dpyNum]);
    }


    // Mover.
    for (n = 0; n < W_ALL_MOVERS_NUM; n++) {
      Boolean val = 
        xvars.load_pixmap(&xdata.moverPixmaps[dpyNum][n],
                          &xdata.moverMasks[dpyNum][n],
                          dpyNum,
                          (char**)moverPixmapBits[n]);
      if (!val) {
        cerr << "Failed to load mover " << n << endl;
      }
    }


    // Backgrounds.
    for (n = 0; n < W_ALL_BACKGROUNDS_NUM; n++) {
      Boolean val = 
        xvars.load_pixmap(&xdata.backgroundPixmaps[dpyNum][n],NULL,dpyNum,
                          (char**)backgrounds[n].id);
      if (!val) {
        cerr << "Failed to load background " << n << endl;
      }
    }


    // Outsides.
    for (n = 0; n < W_ALL_OUTSIDES_NUM; n++) {
      Boolean val = 
        xvars.load_pixmap(&xdata.outsidePixmaps[dpyNum][n],NULL,dpyNum,
                          (char**)outsides[n].id);
      if (!val) {
        cerr << "Failed to load outside " << n << endl;
      }
    }
  } // for dpyNum

  
  xvars.mark_valid(xValid);
}



void Locator::draw_buffered(CMN_DRAWABLE window,Xvars &xvars,
                            int dpyNum,const Box &room) {
  // Don't draw if outside the world.
  if (!world->overlap(room)) {
    return;
  }

  if (!xvars.is_valid(xValid)) {
    init_x(xvars,IX_INIT,NULL);
  }
  
  // Draw new stuff
  GLoc gridStart; 
  gridStart.horiz = 
    (int)floor((float)room.loc.c / (float)OL_GRID_COL_MAX) - 1;
  gridStart.vert = 
    (int)floor((float)room.loc.r / (float)OL_GRID_ROW_MAX) - 1;
  
  GLoc gridFinish; 
  gridFinish.horiz = (int)ceil((float)(room.loc.c + room.dim.colMax) / 
                               (float) OL_GRID_COL_MAX) + 1;
  gridFinish.vert = (int)ceil((float)(room.loc.r + room.dim.rowMax) /
                              (float) OL_GRID_ROW_MAX) + 1;
  
  GLoc gloc;
  
  // Don't need to clease the wasDrawn flag if not using the wasDrawn flag.
  if (drawingAlgorithm == DRAW_MERGE_AREAS) {
    // Clear the was_drawn flag for all gridEntries and shadowEntries
    for (gloc.vert = gridStart.vert; gloc.vert < gridFinish.vert; 
         gloc.vert++) {
      for (gloc.horiz = gridStart.horiz; gloc.horiz < gridFinish.horiz; 
           gloc.horiz++)
        if (valid(gloc)) {
          OLshadowEntry *se = shadows[gloc.vert][gloc.horiz]->get_next();
          while (se) {
            se->set_was_drawn(False);
            se = se->get_next();
          }
	      
          OLgridEntry *ge = grid[gloc.vert][gloc.horiz]->get_next();
          while (ge) {
            ge->set_was_drawn(False);
            ge = ge->get_next();
          }
        }
    }
  } // DRAW_MERGE_AREAS
  
  
  // Go through all shadows in room.
  for (gloc.vert = gridStart.vert; gloc.vert < gridFinish.vert; 
       gloc.vert++) {
    for (gloc.horiz = gridStart.horiz; gloc.horiz < gridFinish.horiz; 
         gloc.horiz++) {
      if (valid(gloc)) {
        OLshadowEntry *se = shadows[gloc.vert][gloc.horiz]->get_next();
        
        while (se) {
          if (!se->get_was_drawn()) { // don't draw if already done.
            const Area &shadowArea = se->get_area();

            // Orphaned shadows must always draw themselves, there
            // is no OLgridEntry that might take care of them.
            if (se->get_orphaned()) {
              // Draw the shadow area.
              if (drawingAlgorithm == DRAW_MERGE_AREAS) {
                // Set was_drawn before call to merge_draw_area so
                // that area doesn't waste time merging with itself.
                se->set_was_drawn(True);
                merge_draw_area(window,xvars,dpyNum,room,
                                shadowArea,gloc);
              }
              else {
                draw_area(window,xvars,dpyNum,room,shadowArea,gloc);
              }
            }

            // Not an orphan.  Shadow draws self if it doesn't overlap
            // the current location of the object.
            else  { 
              OLgridEntry *ge = se->get_grid_entry();

              // Regular grid entry will draw shadow.
              if (shadowArea.overlap(ge->get_area())) {
                se->set_draw_self(False);
              }
              // Draw the shadow area.
              else {
                se->set_draw_self(True);
                if (drawingAlgorithm == DRAW_MERGE_AREAS) {
                  se->set_was_drawn(True);
                  merge_draw_area(window,xvars,dpyNum,room,
                                  shadowArea,gloc);
                }
                else {
                  draw_area(window,xvars,dpyNum,room,
                            shadowArea,gloc);
                }
              }
            }
          }
          // se->get_was_drawn() is False.
          else { 
            // Shadow implicitly took care of drawing itself.
            se->set_draw_self(True);
          }
          se = se->get_next();
        } // while se
      } // valid
    } // for gloc.horiz
  } // for gloc.vert
  

  // Go through all grid entries in room.
  for (gloc.vert = gridStart.vert; gloc.vert < gridFinish.vert; 
       gloc.vert++) {
    for (gloc.horiz = gridStart.horiz; gloc.horiz < gridFinish.horiz; 
         gloc.horiz++) {
      if (valid(gloc)) {
        OLgridEntry *ge = grid[gloc.vert][gloc.horiz]->get_next();
        
        while (ge) {
          OLentry *ent = ge->get_entry();
          assert (ent->gloc == gloc);
          
          if ((ge->get_mapped() || ge->get_flash())) {
            OLshadowEntry *se = ge->get_shadow_entry();
            
            const Area &shadowArea = se->get_area();
            if (se->draw_self() ||
                se->get_was_drawn() ||
                // Don't try to combine areas if the shadow is 
                // not in the room.
                // Edge effects taken care of by extra +1 in 
                // looking at grid squares of the room.
                !shadowArea.overlap(room)) {

              // At this point we know we don't need to worry about the shadow.
              
              if (!ge->get_was_drawn()) {
                if (drawingAlgorithm == DRAW_MERGE_AREAS) {
                  ge->set_was_drawn(True);
                  merge_draw_area(window,xvars,dpyNum,room,
                                  ge->get_area(),gloc);
                }
                else {
                  draw_area(window,xvars,dpyNum,room,
                            ge->get_area(),gloc);
                }
              }
            }
            else if (!ge->get_was_drawn()) {
              // Need to draw both grid entry and shadow.
              
              if (drawingAlgorithm == DRAW_MERGE_AREAS) {
                ge->set_was_drawn(True);
                se->set_was_drawn(True);

                // Try to combine areas of gridEntry and shadow.
                Area combined = shadowArea.combine(ge->get_area());
                Size size = combined.get_size();               
                if (size.width <= OL_GRID_WIDTH &&
                    size.height <= OL_GRID_HEIGHT) {
                  // Draw once, combining areas.
                  merge_draw_area(window,xvars,dpyNum,room,
                                  combined,gloc);
                }
                else {
                  // Combined area is too big, so draw areas separately.
                  merge_draw_area(window,xvars,dpyNum,room,
                                  ge->get_area(),gloc);
                  merge_draw_area(window,xvars,dpyNum,room,
                                  shadowArea,gloc);
                }
              }
              else {
                draw_area(window,xvars,dpyNum,room,
                          shadowArea.combine(ge->get_area()),gloc);
              }
            }
            else {
              // gridEntry was drawn, but not shadow, so draw shadow here.
              if (drawingAlgorithm == DRAW_MERGE_AREAS) {
                se->set_was_drawn(True);
                merge_draw_area(window,xvars,dpyNum,room,
                                shadowArea,
                                gloc);
              }
              else {
                draw_area(window,xvars,dpyNum,room,
                          shadowArea,gloc);
              }
            }
          }
          ge = ge->get_next();
        } // while ge
      } // valid 
    } // for gloc.horiz
  } // for gloc.vert
}



Drawable Locator::get_scratch_buffer(Pos& pos,Xvars&,int dpyNum) {
  pos.set_zero();
  return xdata.scratchBuffer[dpyNum];
}



void Locator::init_x(Xvars &xvars,IXCommand command,void*) {
  assert(command == IX_INIT);
  assert(!xvars.is_valid(xValid));

  for (int dpyNum = 0; dpyNum < xvars.dpyMax; dpyNum++) {
    // Floating backbuffer.
    xdata.buffer[dpyNum] = 
      XCreatePixmap(xvars.dpy[dpyNum],xvars.root[dpyNum],
                    xvars.stretch_x(OL_GRID_COL_MAX * WSQUARE_WIDTH),
                    xvars.stretch_y(OL_GRID_ROW_MAX * WSQUARE_HEIGHT),
                    xvars.depth[dpyNum]);
    
    // Scratch buffer, supplied to Physical objects that want to do
    // drawing tricks.
    xdata.scratchBuffer[dpyNum] = 
      XCreatePixmap(xvars.dpy[dpyNum],xvars.root[dpyNum],
                    xvars.stretch_x(OL_GRID_COL_MAX * WSQUARE_WIDTH),
                    xvars.stretch_y(OL_GRID_ROW_MAX * WSQUARE_HEIGHT),
                    xvars.depth[dpyNum]);


    // Tick marks 
    // Much of this code directly from Windows Locator::init_x(), but I don't 
    // see an easy way to share.
    //
    // Load base tick bitmaps onto surface.
    // Also compute max tick size while we're doing a loop anyway.
    TickType tt;
    int hp;
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
          xvars.load_pixmap(&xdata.tickPixmaps[dpyNum][tt][hp],
                            &xdata.tickMasks[dpyNum][tt][hp],
                            dpyNum,(char**)tickPixmapBits[tt][hp]);
        }
      }
    }

    // For efficiency, only create one scratch surface for doing the
    // transformations.  Pass it into generate_pixmap_from_transform().
    Pixmap scratch =
      XCreatePixmap(xvars.dpy[dpyNum],xvars.root[dpyNum],
                    tickMax.width,tickMax.height,
                    xvars.depth[dpyNum]);
    Pixmap maskScratch =
      XCreatePixmap(xvars.dpy[dpyNum],xvars.root[dpyNum],
                    tickMax.width,tickMax.height,
                    1);

    // Load auto-generated ticks.
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

          // Create the empty pixmap.
          xdata.tickPixmaps[dpyNum][tt][hp] = 
            XCreatePixmap(xvars.dpy[dpyNum],xvars.root[dpyNum],
                          xvars.stretch_x(tickSizes[hp].width),
                          xvars.stretch_y(tickSizes[hp].height),
                          xvars.depth[dpyNum]);
          // Transform from base pixmap to auto-generated one.
          xvars.generate_pixmap_from_transform(dpyNum,
                          xdata.tickPixmaps[dpyNum][tt][hp],
                          xdata.tickPixmaps[dpyNum][tt][baseHP],
                          xvars.stretch_size(tickSizes[baseHP]),
                          scratch,
                          transforms,tNum,
                          xvars.depth[dpyNum]);

          // Create the empty mask.
          xdata.tickMasks[dpyNum][tt][hp] = 
            XCreatePixmap(xvars.dpy[dpyNum],xvars.root[dpyNum],
                          xvars.stretch_x(tickSizes[hp].width),
                          xvars.stretch_y(tickSizes[hp].height),
                          1);
          // Transform from base mask to auto-generated one.
          xvars.generate_pixmap_from_transform(dpyNum,
                          xdata.tickMasks[dpyNum][tt][hp],
                          xdata.tickMasks[dpyNum][tt][baseHP],
                          xvars.stretch_size(tickSizes[baseHP]),
                          maskScratch,
                          transforms,tNum,
                          1);                        
        }
      } // hp
    } // tt   

    // Kill scratch pixmaps.
    XFreePixmap(xvars.dpy[dpyNum],scratch);
    XFreePixmap(xvars.dpy[dpyNum],maskScratch);
  } // dpyNum
  xvars.mark_valid(xValid);
}



void Locator::merge_draw_area(Drawable window,Xvars &xvars,int dpyNum,
			      const Box &room,const Area &area,
			      const GLoc &gloc) {
  // Trivial reject.
  if (!area.overlap(room)) {
    return;
  }

  Area newArea = area;
  
  GLoc check;
  for (check.horiz = gloc.horiz - 1; 
       check.horiz <= gloc.horiz + 1; 
       check.horiz++) {
    for (check.vert = gloc.vert - 1; 
         check.vert <= gloc.vert + 1; 
         check.vert++) {
      if (valid(check)&& possible_overlap(area,check)) {
        // Try to merge in gridEntries
        OLgridEntry *checkGe = 
          grid[check.vert][check.horiz]->get_next(); 
        
        while (checkGe) {
          if (!checkGe->get_was_drawn() &&
              newArea.overlap(checkGe->get_area())) {
            // Try to combine areas.
            Area tryMe = newArea.combine(checkGe->get_area());
            Size size = tryMe.get_size();
            if (size.width <= OL_GRID_WIDTH &&
                size.height <= OL_GRID_HEIGHT) {
              // Combine was successful. Rah, rah.
              checkGe->set_was_drawn(True);
              newArea = tryMe;
              
              // Since we merged in the gridEntry, check the associated
              // shadow entry as well.
              OLshadowEntry *se = checkGe->get_shadow_entry();
              if (!se->get_was_drawn()) {
                // Try to combine areas.
                Area tryMe2 = newArea.combine(se->get_area());
                Size size = tryMe2.get_size();
                if (size.width <= OL_GRID_WIDTH &&
                    size.height <= OL_GRID_HEIGHT) {
                  // Combined the shadow as well.
                  se->set_was_drawn(True);
                  newArea = tryMe2;
                }
              }
            }
          }
          checkGe = checkGe->get_next();
        } // while (checkGe)
        
        // Try to merge in shadowEntries
        OLshadowEntry *se = shadows[check.vert][check.horiz]->get_next();
        while (se) {
          if (!se->get_was_drawn() &&
              newArea.overlap(se->get_area())) {
            // Try to combine areas.
            Area tryMe = newArea.combine(se->get_area());
            Size size = tryMe.get_size();
            if (size.width <= OL_GRID_WIDTH &&
                size.height <= OL_GRID_HEIGHT) {
              // Combine was successful. Rah, rah.
              se->set_was_drawn(True);
              newArea = tryMe;
              
              // Since we merged in the shadow, check the associated grid
              // entry as well.
              if (!se->get_orphaned()) {  // else no entry to check.
                OLgridEntry *ge = se->get_grid_entry();
                if (!ge->get_was_drawn()) {
                  // Try to combine areas.
                  Area tryMe2 = newArea.combine(ge->get_area());
                  Size size = tryMe2.get_size();
                  if (size.width <= OL_GRID_WIDTH &&
                      size.height <= OL_GRID_HEIGHT) {
                    // Combined the shadow's gridEntry as well.
                    ge->set_was_drawn(True);
                    newArea = tryMe2;
                  }
                }
              }
            }
          }
          se = se->get_next();
        } // while (se)
      } // if valid
    } // for
  } // for
  
  // Do actual drawing on merged area.
  draw_area(window,xvars,dpyNum,room,newArea,compute_gloc(newArea));
}



void Locator::draw_area(Drawable window,Xvars &xvars,int dpyNum,
                        const Box &room,const Area &area,const GLoc &gloc) {
  ///// Unnecessary since all calls come from merge_draw_area which already
  ///// does this, and areas don't get smaller.
  // Trivial reject.
  if (drawingAlgorithm != DRAW_MERGE_AREAS) {
    if (!area.overlap(room))
      return;
  }
  
  // reduceDraw is always False for X
  //
  // If we are using Locator::draw_area, we are not doing smooth scrolling.
  // So we don't use the the background3D effect.
  // Locator::draw_directly() doesn't use Locator::draw_area().
  world->draw(xdata.buffer[dpyNum],xvars,dpyNum,area,False,False);

  // Iterate over grid squares + and - 1 in both directions.
  GLoc start,finish;
  start.horiz = gloc.horiz - 1;
  start.vert = gloc.vert - 1;
  finish.horiz = gloc.horiz + 2;
  finish.vert = gloc.vert + 2;

  // Avoid double drawing composite objects.
  clear_already_drawn_flag(start,finish);

  for (int dLevel = 0; dLevel < DRAWING_LEVELS; dLevel++) {
    GLoc check;
    for (check.horiz = start.horiz; 
         check.horiz < finish.horiz; 
         check.horiz++) {
      for (check.vert = start.vert; 
           check.vert < finish.vert; 
           check.vert++) {
        if (valid(check) && possible_overlap(area,check)) {
	      OLgridEntry *checkGe = 
            grid[check.vert][check.horiz]->get_next(); 
	      
	      while (checkGe) {
            if ((checkGe->get_drawing_level() == dLevel) && 
                (checkGe->get_flash() || checkGe->get_mapped())) {
		      PhysicalP checkP = checkGe->get_physical();
		      const Area &checkArea = checkP->get_area();

		      if (checkArea.overlap(area)) {
                draw_chain(xdata.buffer[dpyNum],xvars,dpyNum,area,checkP);
              }
		    }				
            checkGe = checkGe->get_next();
          } // while
	    } // if
      } // for vert
    } // for horiz
  } // for dLevel

  Pos pos;
  Size size;
  area.get_rect(pos,size);
  
  // Copy the backbuffer section onto the actual window.
  XCopyArea(xvars.dpy[dpyNum],xdata.buffer[dpyNum],window,xvars.gc[dpyNum],
            0,0,
            xvars.stretch_x(size.width),
            xvars.stretch_y(size.height),
            xvars.stretch_x(pos.x - room.loc.c * WSQUARE_WIDTH),
            xvars.stretch_x(pos.y - room.loc.r * WSQUARE_HEIGHT));

  // For debugging, if the -draw_rects argument was passed in.
  if (drawRects) {
    XSetForeground(xvars.dpy[dpyNum],xvars.gc[dpyNum],xvars.red[dpyNum]);
    XDrawRectangle(xvars.dpy[dpyNum],window,xvars.gc[dpyNum],
                   xvars.stretch_x(pos.x - room.loc.c * WSQUARE_WIDTH),
                   xvars.stretch_y(pos.y - room.loc.r * WSQUARE_HEIGHT),
                   xvars.stretch_x(size.width) - 1,
                   xvars.stretch_y(size.height) - 1);
    XSetForeground(xvars.dpy[dpyNum],xvars.gc[dpyNum],xvars.black[dpyNum]);
  }
}



Boolean Locator::draw_tick(TickType tt,CMN_DRAWABLE window,Xvars &xvars,
                           int dpyNum,
                           const Size &windowSize,
                           Dir tickDir,int offset) {
  if (!xvars.is_valid(xValid)) {
    init_x(xvars,IX_INIT,NULL);
  }

  int hp = Coord::pure_to_half_pure(tickDir);

  // Stretched coordinates.
  Pos destPos;
  Size tSize = xvars.stretch_size(tickSizes[hp]);

  // Set X coordinate of destPos.
  switch (tickDir) {
  case CO_R:
  case CO_DN_R:
  case CO_UP_R:
    destPos.x = windowSize.width - tSize.width;
    break;
  case CO_DN:
  case CO_UP:
    destPos.x = offset - (tSize.width >> 1);
    break;
  case CO_DN_L:
  case CO_L:
  case CO_UP_L:
    destPos.x = 0;
    break;
  default:
    assert(0);
  }

  // Set Y coordinate of destPos.
  switch (tickDir) {
  case CO_R:
  case CO_L:
    destPos.y = offset - (tSize.height >> 1);
    break;
  case CO_DN_R:
  case CO_DN:
  case CO_DN_L:
    destPos.y = windowSize.height - tSize.height;
    break;
  case CO_UP_L:
  case CO_UP:
  case CO_UP_R:
    destPos.y = 0;
    break;
  default:
    assert(0);
  }

  // Copy tick onto surface.
  XSetClipMask(xvars.dpy[dpyNum],xvars.gc[dpyNum],
               xdata.tickMasks[dpyNum][tt][hp]);
  XSetClipOrigin(xvars.dpy[dpyNum],xvars.gc[dpyNum],
                 destPos.x,destPos.y);
  XCopyArea(xvars.dpy[dpyNum],xdata.tickPixmaps[dpyNum][tt][hp],
            window,xvars.gc[dpyNum],0,0,
            xvars.stretch_x(tickSizes[hp].width),
            xvars.stretch_y(tickSizes[hp].height),
            destPos.x,destPos.y);

  // Restore gc to initial state.
  XSetClipMask(xvars.dpy[dpyNum],xvars.gc[dpyNum],None);
  XSetClipOrigin(xvars.dpy[dpyNum],xvars.gc[dpyNum],0,0);
  return True;
}



void Fire::init_x(Xvars &xvars,IXCommand command,void*) {
  assert(!xvars.is_valid(xdata.valid));
  assert(command == IX_INIT);

  int dpyNum;
  for (dpyNum = 0; dpyNum < xvars.dpyMax; dpyNum++) {
    Boolean val = xvars.load_pixmap(&xdata.pixmap[dpyNum],
                                    &xdata.mask[dpyNum],
                                    dpyNum,(char**)fireBits);
    if (!val) {
      cerr << "Failed to load fire graphics." << endl;
    }
  }

  xvars.mark_valid(xdata.valid);
}



// Draw functions for Physcial.
void Protection::draw(Drawable buffer,Xvars &xvars,int dpyNum,
                      const Area &bufArea) {
  if (!xvars.is_valid(pXdata->valid)) {
    init_x(xvars,IX_INIT,NULL,*prc,*pXdata);
  }

  Pos pos;
  Size size;
  area.get_rect(pos,size);
  Size offset = area - bufArea;

  XSetForeground(xvars.dpy[dpyNum],xvars.gc[dpyNum],pXdata->color[dpyNum]);
  XDrawRectangle(xvars.dpy[dpyNum],buffer,xvars.gc[dpyNum],
                 xvars.stretch_x(offset.width),
                 xvars.stretch_y(offset.height),
                 xvars.stretch_x(size.width - 1),
                 xvars.stretch_y(size.height - 1));
  XSetForeground(xvars.dpy[dpyNum],xvars.gc[dpyNum],xvars.black[dpyNum]);
}



void Moving::draw(Drawable buffer,Xvars &xvars,int dpyNum,
                  const Area &bufArea) {    
  // Initialize X variables if necessary.
  if (!xvars.is_valid(movingXdata->valid)) {
    init_x(xvars,IX_INIT,NULL,*mc,*movingXdata);
  }

  Pos pos;
  Size size;
  area.get_rect(pos,size);
  Size offset = area - bufArea;

  Pixmap scratch;
  if (is_invisible()) {
    LocatorP locator = get_locator();
    Pos dummy;
    scratch = locator->get_scratch_buffer(dummy,xvars,dpyNum);
    
    // Which way to distort the image.
    // We will draw the image twice to get the wrap-around.
    // Copy portion of area into scratch buffer.
    XCopyArea(xvars.dpy[dpyNum],buffer,scratch,xvars.gc[dpyNum],
              xvars.stretch_x(offset.width),
              xvars.stretch_y(offset.height),
              xvars.stretch_x(size.width),
              xvars.stretch_y(size.height),
              xvars.stretch_x(0),
              xvars.stretch_y(size.height - 3));
    XCopyArea(xvars.dpy[dpyNum],buffer,scratch,xvars.gc[dpyNum],
              xvars.stretch_x(offset.width),
              xvars.stretch_y(offset.height),
              xvars.stretch_x(size.width),
              xvars.stretch_y(size.height),
              xvars.stretch_x(0),
              xvars.stretch_y(-3));
  }
  
  CMN_IMAGEDATA imageData;
  get_pixmap_mask(xvars,dpyNum,imageData,dir,movingAnimNum);
  Pixmap pixmap = imageData.pixmap;
  Pixmap mask = imageData.mask;
  
  XSetClipMask(xvars.dpy[dpyNum],xvars.gc[dpyNum],mask);
  XSetClipOrigin(xvars.dpy[dpyNum],xvars.gc[dpyNum],
                 xvars.stretch_x(offset.width),
                 xvars.stretch_y(offset.height));
  
  Drawable srcPix;
  if (is_invisible()) {
    // Draw from the scratch surface.
    srcPix = scratch;
  }
  else {
    // Use supplied pixmap.
    srcPix = pixmap;
  }

  XCopyArea(xvars.dpy[dpyNum],srcPix,
            buffer,xvars.gc[dpyNum],0,0,
            xvars.stretch_x(size.width),
            xvars.stretch_y(size.height),
            xvars.stretch_x(offset.width),
            xvars.stretch_y(offset.height));
  
  // Restore gc to initial state.
  XSetClipMask(xvars.dpy[dpyNum],xvars.gc[dpyNum],None);
  XSetClipOrigin(xvars.dpy[dpyNum],xvars.gc[dpyNum],0,0);
  

// Haven't fixed this for stretching yet.
#if 0
  ColorNum colorNum;
  if (draw_outline(colorNum) &&
      // Don't draw for XPM images.
      !mc->useXPM
      ) {
    XSetForeground(xvars.dpy[dpyNum],xvars.gc[dpyNum],
                   xvars.humanColors[dpyNum][colorNum]);
    XPoint points[12];
    
    points[0].x = offset.width;
    points[0].y = offset.height;
    
    points[1].x = offset.width + 1;
    points[1].y = offset.height;
    
    points[2].x = offset.width;
    points[2].y = offset.height + 1;
    
    
    points[3].x = offset.width + size.width - 1;
    points[3].y = offset.height;
    
    points[4].x = offset.width + size.width - 2;
    points[4].y = offset.height;
    
    points[5].x = offset.width + size.width - 1;
    points[5].y = offset.height + 1;
    
    
    points[6].x = offset.width;
    points[6].y = offset.height + size.height - 1;
    
    points[7].x = offset.width;
    points[7].y = offset.height + size.height - 2;
    
    points[8].x = offset.width + 1;
    points[8].y = offset.height + size.height - 1;
    
    
    points[9].x = offset.width + size.width - 1;
    points[9].y = offset.height + size.height - 1;
    
    points[10].x = offset.width + size.width - 2;
    points[10].y = offset.height + size.height - 1;
    
    points[11].x = offset.width + size.width - 1;
    points[11].y = offset.height + size.height - 2;
    
    XDrawPoints(xvars.dpy[dpyNum],buffer,xvars.gc[dpyNum],points,
                12,CoordModeOrigin);
    XSetForeground(xvars.dpy[dpyNum],xvars.gc[dpyNum],
                   xvars.black[dpyNum]);
  }
#endif
}



void Moving::get_pixmap_mask(Xvars &,int dpyNum,CMN_IMAGEDATA &imageData,
                             Dir dir,int animNum) {
  imageData.pixmap = movingXdata->pixmaps[dpyNum][dir][animNum];
  imageData.mask = movingXdata->masks[dpyNum][dir][animNum];
}



// Draw functions for Actual
void Explosion::draw(Drawable buffer,Xvars &xvars,int dpyNum,
		     const Area &bufArea) {
  Pos pos;
  Size size;
  area.get_rect(pos,size);
  Size offset = area - bufArea;

  XFillArc(xvars.dpy[dpyNum],buffer,xvars.gc[dpyNum],
           xvars.stretch_x(offset.width),
           xvars.stretch_y(offset.height),
           xvars.stretch_x(size.width),
           xvars.stretch_y(size.height),
           0,23040);
}



void Fire::draw(Drawable buffer,Xvars &xvars,int dpyNum,
                const Area &bufArea) {
  // Initialize X variables if necessary.
  if (!xvars.is_valid(xdata.valid)) {
    init_x(xvars,IX_INIT,NULL);
  }

  const Area &area = get_area();
  Pos pos;
  Size size;
  area.get_rect(pos,size);
  Size offset = area - bufArea;
  Size sizeMax = Fire::get_size_max();

  // Grab a random part of the source bitmap.
  // World coordinates.
  Pos srcPos(Utils::choose(sizeMax.width - size.width + 1),
             Utils::choose(sizeMax.height - size.height + 1));

  
  XSetClipMask(xvars.dpy[dpyNum],xvars.gc[dpyNum],xdata.mask[dpyNum]);
  XSetClipOrigin(xvars.dpy[dpyNum],xvars.gc[dpyNum],
                 xvars.stretch_x(offset.width),
                 xvars.stretch_y(offset.height));

  XCopyArea(xvars.dpy[dpyNum],xdata.pixmap[dpyNum],buffer,
            xvars.gc[dpyNum],
      xvars.stretch_x(srcPos.x),xvars.stretch_y(srcPos.y),
            xvars.stretch_x(size.width),xvars.stretch_y(size.height),
            xvars.stretch_x(offset.width),xvars.stretch_y(offset.height));

  // Restore gc to initial state.
  XSetClipMask(xvars.dpy[dpyNum],xvars.gc[dpyNum],None);
  XSetClipOrigin(xvars.dpy[dpyNum],xvars.gc[dpyNum],0,0);


#if 0
  XSetForeground(xvars.dpy[dpyNum],xvars.gc[dpyNum],xvars.red[dpyNum]);
  XFillArc(xvars.dpy[dpyNum],buffer,xvars.gc[dpyNum],
           xvars.stretch_x(offset.width),
           xvars.stretch_y(offset.height),
           xvars.stretch_x(size.width),
           xvars.stretch_y(size.height),
           0,23040);
  XSetForeground(xvars.dpy[dpyNum],xvars.gc[dpyNum],xvars.black[dpyNum]);
#endif
}



void Protection::init_x(Xvars &xvars,IXCommand,void*,
                        const ProtectionContext &prc,
                        ProtectionXdata &pXdata) {
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

  // Load all non-autogenerated bitmaps.
  // This should load ALL the base bitmaps.
  int dpyNum;
  for (dpyNum = 0; dpyNum < xvars.dpyMax; dpyNum++) {
    for (int n = 0; n < CO_DIR_MAX; n++) {
      if (mc.animMax[n] == 0 || mc.pixmapBits[n][0] == PH_AUTO_GEN) {
        continue;
      }

      for (int m = 0; m < mc.animMax[n]; m++) {
        assert(mc.useXPM);
        // Load an XPM image
        Boolean val = 
          xvars.load_pixmap(&movingXdata.pixmaps[dpyNum][n][m],
                            &movingXdata.masks[dpyNum][n][m],
                            dpyNum,(char**)mc.pixmapBits[n][m]);
        if (!val) {
          cerr << "load_pixmap() failed for "
               << mc.physicalContext.className 
               << " dir=" << n << " animNum=" << m << endl;
        }
      } // for m
    } // for n
  } // for dpyNum

  
  // Auto generate the PH_AUTO_GEN pixmaps and masks.
  for (dpyNum = 0; dpyNum < xvars.dpyMax; dpyNum++) {
    // For efficiency, only create one scratch surface for doing the
    // transformations.  Pass it into generate_pixmap_from_transform().
    Pixmap scratch =
      XCreatePixmap(xvars.dpy[dpyNum],xvars.root[dpyNum],
                    xvars.stretch_x(mc.physicalContext.sizeMax.width),
                    xvars.stretch_y(mc.physicalContext.sizeMax.height),
                    xvars.depth[dpyNum]);
    Pixmap maskScratch =
      XCreatePixmap(xvars.dpy[dpyNum],xvars.root[dpyNum],
                    xvars.stretch_x(mc.physicalContext.sizeMax.width),
                    xvars.stretch_y(mc.physicalContext.sizeMax.height),
                    1);
    
    // Loop over all dirs.
    for (int n = 0; n < CO_DIR_MAX; n++) {
      if (mc.animMax[n] == 0 || mc.pixmapBits[n][0] != PH_AUTO_GEN) {
        continue;
      }

      // Lookup the necessary transformations.
      Dir base = Transform2D::get_base(n,mc.transformOverride);
      int tNum;
      const TransformType* transforms = 
        Transform2D::get_transforms(tNum,n,mc.transformOverride);


      // Loop over all animation frames.
      for (int m = 0; m < mc.animMax[n]; m++) {
        // Create the empty pixmap.
        movingXdata.pixmaps[dpyNum][n][m] = 
          XCreatePixmap(xvars.dpy[dpyNum],xvars.root[dpyNum],
                        xvars.stretch_x(mc.sizes[n].width),
                        xvars.stretch_y(mc.sizes[n].height),
                        xvars.depth[dpyNum]);
        // Transform from base pixmap to auto-generated one.
        xvars.generate_pixmap_from_transform(dpyNum,
                        movingXdata.pixmaps[dpyNum][n][m],
                        movingXdata.pixmaps[dpyNum][base][m],
                        xvars.stretch_size(mc.sizes[base]),
                        scratch,
                        transforms,tNum,
                        xvars.depth[dpyNum]);

        // Create the empty mask.
        movingXdata.masks[dpyNum][n][m] = 
          XCreatePixmap(xvars.dpy[dpyNum],xvars.root[dpyNum],
                        xvars.stretch_x(mc.sizes[n].width),
                        xvars.stretch_y(mc.sizes[n].height),
                        1);
        // Transform from base mask to auto-generated one.
        xvars.generate_pixmap_from_transform(dpyNum,
                        movingXdata.masks[dpyNum][n][m],
                        movingXdata.masks[dpyNum][base][m],
                        xvars.stretch_size(mc.sizes[base]),
                        maskScratch,
                        transforms,tNum,
                        1);                        
      } // for m
    } // for n

    // Kill scratch pixmaps.
    XFreePixmap(xvars.dpy[dpyNum],scratch);
    XFreePixmap(xvars.dpy[dpyNum],maskScratch);
  } // for dpyNum


  xvars.mark_valid(movingXdata.valid);
}
