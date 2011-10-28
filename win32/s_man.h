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

#ifndef S_MAN_H
#define S_MAN_H

#include "utils.h"
#include "coord.h"
#include "area.h"



class Xvars;



// Should have more general way of generating unique keys.
//
// Keys that use bitmap ids as the last 16 bits and 0x00 as the hightest
// byte don't need to use these values, they are already guaranteed unique,
// e.g. Moving::init_x() and Fire::init_x().
// But, if you want to use any other scheme to generate keys, use a unique
// highest order byte for the scheme.
#define S_MAN_OL_NONCE 0xBB  // Object Locator
#define S_MAN_WHEEL_NONCE 0xCC // Weapon Wheel



// Manages rectangular regions of DirectDraw surfaces.  Each region is 
// associated with an externally supplied void* key.
// 1) Create the SurfaceManager
// 2) Call add() for all desired rectangles
// 3) Call add_complete() to generate the DirectDraw surfaces.  You may not 
//    add more rectangles after calling add_complete().
// 4) Call lookup() as desired to get the allocated for rectangle for a key.
class SurfaceManager {
public:
  SurfaceManager(int heightMax);
  /* EFFECTS: Create a SurfaceManager.  heightMax is the maximum allowed height
     for any surface. */

  ~SurfaceManager();

  Boolean is_add_completed() {return addCompleted;}
  /* EFFECTS: Have all the rectangle requests been added and add_complete() 
     called. */

  void add(void* key,const Size& size,DDCOLORKEY* colorKey);
  /* REQUIRES: size.width > 0 && size.height > 0 
     size.height <= heightMax passed into the constructor.
     Don't add same key twice with a different size or colorKey. */
  /* EFFECTS: Add a request for a rectangle of a given size.  size is in stretched
     coordinates.  The rectangle will be associated with "key".  You may add() the 
     same (key,size,colorKey) triple multiple times, it will only be allocated one 
     rectangle.  Set colorKey=NULL for no colorKey. */

  LPDIRECTDRAWSURFACE lookup(Area& area,const DDCOLORKEY*& colorKey,void* key);
  /* REQUIRES: key was added in the add phase. */
  /* MODIFIES: area, colorKey */
  /* EFFECTS: Lookup the surface area for "key".  Return the DirectDrawSurface 
     for that key and set area to be the rectangle on that surface reserved for 
     that key. */

  LPDIRECTDRAWSURFACE lookup(Pos& pos,void* key);
  /* MODIFIES: pos */
  /* EFFECTS: Convenience method.  Same as other lookup(), except returns less
     data. */

  LPDIRECTDRAWSURFACE lookup(Size& size,void* key);
  /* MODIFIES: size */
  /* EFFECTS: Convenience method.  Same as other lookup(), except the offset is
     returned as a Size instead of as a Pos. */

  HRESULT add_complete(Xvars& xvars);
  /* EFFECTS: All rectangle requests have been submitted, generate the surfaces. 
     DD_OK if success, else an error code. */

  int pixels_allocated() {assert(addCompleted); return pixAllc;}
  int pixels_used() {assert(addCompleted); return pixUsed;}
  /* EFFECTS: Statistics about the layout of the surfaces.  Number of pixels allocated in all
     the direct draw surfaces and the number actually used in the surface requests. */

  int debug_pages_num(const Size& size);
  /* REQUIRES: size.height >= heightMax passed into the constructor.  
     size.width >= max width of all surfaces added to the SurfaceManager. 
     add_complete() must have been called. */
  /* EFFECTS: Computes how many debugging pages will be needed to display all 
     the surfaces in the SurfaceManager. */

  void draw_debug_page(Xvars&,LPDIRECTDRAWSURFACE surf,
                       const Size& size,int pageNum);
  /* REQUIRES: Requirements of debug_pages_num() plus 
     0 <= pageNum < debug_pages_num() */
  /* EFFECTS: Draw one debugging page onto surf.  surf has the given size. */


private:
  LPDIRECTDRAWSURFACE _lookup(Area& area,const DDCOLORKEY*& colorKey,void* key);
  /* EFFECTS: Internal version of lookup().  Can be called before add_complete(). */

  Boolean color_key_equals(const DDCOLORKEY*,const DDCOLORKEY*);
  /* EFFECTS: Are the two color keys the same.  Values may be NULL. */
  
  // Has add_complete() been called.
  Boolean addCompleted;

  // The list of SurfaceSets.
  PtrList sets;

  // Passed on to SurfaceSet constructor.
  int heightMax;

  // Statistics.
  int pixAllc;
  int pixUsed;
};

#endif
