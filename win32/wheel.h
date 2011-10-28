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

#ifndef WHEEL_H
#define WHEEL_H

#include "utils.h"
#include "coord.h"
#include "locator.h"
#include "xdata.h"



class Wheel {
public:
  // To indicate when a weapon is ready to fire.
  // Will be READY_UNDEFINED for items.
  enum ReadyState {
    READY_YES,
    READY_NO,
    READY_UNDEFINED
  };
  enum {
    WHEEL_ITEM_WIDTH = 64,
    WHEEL_ITEM_HEIGHT = 36,
  };

  Wheel(LocatorP);
  ~Wheel();

  static void init_x(LocatorP,Xvars &xvars,IXCommand,void*);

  void setIndexByClassId(ClassId cId);
  /* EFFECTS: Sets index to the bitmap corresponding to the ClassId. */
  /* REQUIRES: cId is the ClassId of a weapon or item or one of the special
     "non-existent" classes, see coord.h. */

  int getIndex(){return m_item;}
  void setValueAtIndex(int p_value,int p_item);
  int getValueAtIndex(int p_item);
  void setReadyAtIndex(ReadyState,int p_item);
  
  void set_pos(const Pos& p_pos) {m_pos = p_pos;}

  // returns TRUE if actually updated
  BOOL update(LPDIRECTDRAWSURFACE p_buffer,LocatorP,Xvars &xvars,BOOL p_forceredraw=FALSE); 

  // for animation
  void clock();
    

private:
  static void* compute_key(CMN_BITS_ID id);
  /* EFFECTS: Compute the key to store the wheel item for the given bitmap.
     Can't just use id as the key because id is very likely to be a bitmap
     used somewhere else in the game.  E.g. the weapon wheel item for
     IDB_DOPPEL is different than the bitmap for the doppelganer itself. */
  
  static void gather_bitmaps(LocatorP);
  /* EFFECTS: Create the list of all item/weapon bitmaps that can be in the
     Wheel. */

  HRESULT draw(LPDIRECTDRAWSURFACE p_buffer,LocatorP,Xvars &xvars);

  HRESULT drawText(LPDIRECTDRAWSURFACE p_buffer,Xvars &xvars);

  void setIndex(int p_index);
  /* EFFECTS: Set current item by the index into m_bitmapids. */

  // (0,0,WHEEL_ITEM_WIDTH,WHEEL_ITEM_HEIGHT*m_numbitmaps)
  CRect m_surfacearea; 

  // m_alpha is how far we are from displaying previous to displaying current
  // m_alpha == 0 means completely showing m_itemPrev
  // m_alpha == 1.0 means completely showing m_item.
  float m_alpha;
  // The most recently set item.
  int m_item; 
  int m_itemPrev; // Previous item.  -1 means not set.

  // Arrays of length numbitmaps.
  int *m_values;
  ReadyState *m_ready;

  // Location of upper-left of wheel.
  Pos m_pos;

  // Changed since last draw.
  BOOL m_dirty;

  // Convenience.  Equals (WHEEL_ITEM_WIDTH,WHEEL_ITEM_HEIGHT).
  static Size wheelSize;

  static XvarsValid valid;

  // The bitmap IDs for everything in the wheel.
  static int numbitmaps;
  static CMN_BITS_ID bitmapids[A_CLASSES_NUM]; // Could be smaller array.

  // Maps from ClassId to index into bitmapids.
  // Extra one for A_None itself.
  static int bitmapIndex[A_None + 1]; 

  // Name of the bitmap for the Wheel background.
  const static CMN_BITS_ID backgroundBits;
};
#endif //WHEEL_H

