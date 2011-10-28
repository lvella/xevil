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
#include "xdata.h"
#include "wheel.h"
#include "resource.h"

#define INDENT_WHEEL 3

// How much alpha is incremented each turn, for the 
// wheel animation.
#define WHEEL_ALPHA_INCR 0.3f



Wheel::Wheel(LocatorP l)
: m_surfacearea(0,0,0,0) {
  // First do static initialization if it hasn't been done already.
  if (numbitmaps == -1) {
    gather_bitmaps(l);
  }

  // Initialize m_values and m_ready.
  m_values = new int[numbitmaps];
  m_ready = new ReadyState[numbitmaps];
  assert(m_values && m_ready);
  for (int i = 0;i < numbitmaps;i ++) {
    m_values[i] = 0;
    m_ready[i] = READY_UNDEFINED;
  }

  m_item = 0;
  m_itemPrev = -1; // no previous value
  m_alpha = 1.0f;
  m_dirty = TRUE;
}



Wheel::~Wheel() {
  if (m_values) {
    delete [] m_values;
  }
  if (m_ready) {
    delete [] m_ready;
  }
}



void Wheel::init_x(LocatorP l,Xvars &xvars,IXCommand command,void*) {
  // General static initialization.
  if (numbitmaps == -1) {
    gather_bitmaps(l);
  }
  
  if (command == IX_ADD) {
    // Add each of the wheel items with no color key.
    for (int n = 0; n < numbitmaps; n++) {
      xvars.m_surfaceManager->add(compute_key(bitmapids[n]),wheelSize,NULL);
    }    
    return;
  }

  assert(command == IX_INIT || command == IX_LIST);
  if (xvars.is_valid(valid)) {
    return;
  }


  // background, always has the wheel background image
  LPDIRECTDRAWSURFACE bgSurf =
    xvars.create_surface(WHEEL_ITEM_WIDTH,WHEEL_ITEM_HEIGHT,NULL);
  if (!bgSurf) {
    DHRESULT(DDERR_GENERIC,return);
  }
  if (!xvars.load_surface(bgSurf,CPoint(0,0),backgroundBits)) {
    COUTRESULT("load wheel background failed");
    DHRESULT(DDERR_GENERIC,return);
  }

  // Surface for loading each item in turn.
  LPDIRECTDRAWSURFACE itemSurf =
    xvars.create_surface(WHEEL_ITEM_WIDTH,WHEEL_ITEM_HEIGHT,
                         &xvars.XVARS_COLORKEY);
  if (!itemSurf) {
    DHRESULT(DDERR_GENERIC,return);
  }
  
  // Offset the source rect a bit because the dest rect
  // is offset from the upper-left.
  CRect indentRect(0,0,
                   WHEEL_ITEM_WIDTH - INDENT_WHEEL,
                   WHEEL_ITEM_HEIGHT - INDENT_WHEEL);

  for (int i = 0; i < numbitmaps; i++) {
    Pos destPos;
    LPDIRECTDRAWSURFACE destSurf = xvars.m_surfaceManager->lookup(destPos,compute_key(bitmapids[i]));

    // Copy bg onto destination.
    HRESULT t_result = xvars.Blt(destSurf,destPos,bgSurf,Pos(0,0),wheelSize);
    DHRESULT(t_result,return);
    
    // Clear bg of itemSurf to colorkey value.
    t_result = 
      xvars.color_fill(itemSurf,CRect(0,0,WHEEL_ITEM_WIDTH,WHEEL_ITEM_HEIGHT),
                      xvars.XVARS_COLORKEY.dwColorSpaceLowValue);
    DHRESULT(t_result,return);

    // Load in item, will probably be smaller than wheelSize, which is fine
    // since we just filled in the surface with the colorkey value.
    if (!xvars.load_surface(itemSurf,CPoint(0,0),bitmapids[i])) {
      ostrstream dbg;
      dbg << "load_ssurface fails on " << i << " which has ID " << bitmapids[i] << ends;
      COUTRESULT(dbg.str());
      delete dbg.str();
      DHRESULT(DDERR_GENERIC,return);
    }

    // Copy t_surfaceitem to m_drawsurface, indented by INDENT_WHEEL from upper-left 
    // corner.
    t_result = 
      xvars.Blt(destSurf,
                destPos.x + INDENT_WHEEL,destPos.y + INDENT_WHEEL,
                itemSurf,indentRect,True);
    DHRESULT(t_result,return);
  }
  itemSurf->Release();
  bgSurf->Release();

  xvars.mark_valid(valid);
}



void
Wheel::setIndex(int p_item) {
  if (p_item < 0 || p_item >= numbitmaps) {
    ASSERT(0);
    ostrstream dbg;
    dbg << "Wheel::setIndex invalid index " << p_item << ends;
    COUTRESULT(dbg.str());
    delete dbg.str();
    m_item = 0;
  }
  // Only set m_dirty if changed.
  else if (m_item != p_item) {
    // If we are not animating, start a new animation.
    if (m_itemPrev == -1) {
      m_itemPrev = m_item;
      m_alpha = 0.0f;
      m_item = p_item;
    }
    // We were already animating, replace current, 
    // leave prev alone.
    else {
      ASSERT(m_alpha < 1.0f);
      m_item = p_item;
      // Don't start the animation over, keep
      // m_alpha the same.
    }

    m_dirty = TRUE;
  }
}



void
Wheel::setIndexByClassId(ClassId p_cid) {
  if (!(p_cid >= 0 && p_cid <= A_None)) {
    ostrstream dbg;
    dbg << "Wheel::setIndexByClassId, invalid cid " << p_cid << ends;
    COUTRESULT(dbg.str());
    delete dbg.str();
    setIndex(bitmapIndex[A_None]);
  }
  else {
    setIndex(bitmapIndex[p_cid]);
  }
}



void
Wheel::setValueAtIndex(int p_value, int p_index) {
  if ((p_index < numbitmaps) && (p_index > -1)) {
    if (p_value != m_values[p_index]) {
      m_values[p_index]= p_value;
    }
  }
  m_dirty = TRUE;
}



void
Wheel::setReadyAtIndex(Wheel::ReadyState p_state,int p_index) {
  if (p_index < numbitmaps && p_index > -1) {
    m_ready[p_index] = p_state;
  }
  else {
    ostrstream dbg;
    dbg << "setReadyAtIndex to " << p_index 
        << " when max value is " << numbitmaps << ends;
    COUTRESULT(dbg.str());
    delete dbg.str();
    ASSERT(0);
  }
  m_dirty = TRUE;
}



int
Wheel::getValueAtIndex(int p_index) {
  if ((p_index < numbitmaps) && (p_index > -1)) {
    return m_values[p_index];
  }
  return -1;
}



BOOL Wheel::update(LPDIRECTDRAWSURFACE p_buffer,LocatorP l,Xvars &xvars,
                   BOOL p_forceredraw) {
  if (m_dirty || p_forceredraw) {
    draw(p_buffer,l,xvars);
    m_dirty = FALSE;
    return TRUE;
  }

  return FALSE;
}



void Wheel::clock() {
  // If we are in the middle of an animation.
  if (m_itemPrev != -1) {
    ASSERT(m_alpha < 1.0f);
    m_alpha += WHEEL_ALPHA_INCR;

    // Completely onto the next animation frame.
    // m_itemPrev is now meaningless.
    if (m_alpha >= 1.0f) {
      m_alpha = 1.0f;
      m_itemPrev = -1;      
    }

    m_dirty = TRUE;
  }
}



void* Wheel::compute_key(CMN_BITS_ID id) {
  // Make sure id fits in 16 bits.
  assert((0xffff0000 & id) == 0x0);

  return (void*)((S_MAN_WHEEL_NONCE << 24) | id);
}



void Wheel::gather_bitmaps(LocatorP l) {
  // Not really part of gather_bitmaps(), but part of general static
  // class-initialization.
  wheelSize.set(WHEEL_ITEM_WIDTH,WHEEL_ITEM_HEIGHT);
  
// bitmapsids is now static sized array.
#if 0
  // First, just count the number of bitmaps.
  numbitmaps = 1;  // Always have a slot for none.
  for (cId = 0; cId < A_None; cId++) {
    Boolean isItem;
    Boolean isWeapon;
    UINT bitmapId;
    l->get_item_info(isItem,isWeapon,bitmapId,cId);
    if (isWeapon || isItem) {
      numbitmaps++;
    }  
  }
  bitmapids = new CMN_BITS_ID[numbitmaps];
  assert(bitmapids);
#endif

  // Now make bitmapIndex, the map from ClassId to bitmapids,
  // and fill bitmapids with the appropriate bitmap IDs.
  numbitmaps = 1; // Always have a slot for none.
  bitmapIndex[A_None] = 0; // "None" is always the first one.
  bitmapids[0] = IDB_NONE;
  // Loop over all the classIds.
  ClassId cId;
  for (cId = 0; cId < A_None; cId++) {
    bitmapIndex[cId] = -1; // means not there.
    Boolean isItem;
    Boolean isWeapon;
    UINT bitmapId;
    l->get_item_info(isItem,isWeapon,bitmapId,cId);
    if (isWeapon || isItem) {
      // Store this one.
      bitmapids[numbitmaps] = bitmapId;
      bitmapIndex[cId] = numbitmaps;
      numbitmaps++;
    }
  }
  // numbitmaps is now equal to the number of bitmaps found
}


  
HRESULT Wheel::draw(LPDIRECTDRAWSURFACE p_buffer,LocatorP l,Xvars &xvars) {
  if (!xvars.is_valid(valid)) {
    init_x(l,xvars,IX_INIT,NULL);
  }         
  ASSERT(m_item >= 0 && m_item < numbitmaps);

  // No animation, just draw current item
  if (m_itemPrev == -1) {
    Pos surfPos;
    LPDIRECTDRAWSURFACE surf = 
      xvars.m_surfaceManager->lookup(surfPos,compute_key(bitmapids[m_item]));

    HRESULT t_result = xvars.Blt(p_buffer,m_pos,surf,surfPos,wheelSize);
    DHRESULT(t_result,return t_result);

    t_result = drawText(p_buffer,xvars);
    return t_result;
  }
  // Draw some of the current item and some of the previous item.
  else {
    ASSERT(m_alpha < 1.0f && m_alpha >= 0.0f);

    // Convert [0,1] to pixels.
    int pixAlpha = (int)(m_alpha * WHEEL_ITEM_HEIGHT);

    // Draw current item in top of weapon wheel.
    if (pixAlpha > 0) {
      Pos surfPos;
      LPDIRECTDRAWSURFACE surf = 
        xvars.m_surfaceManager->lookup(surfPos,compute_key(bitmapids[m_item]));
      
      // Draw rect of height pixAlpha.
      Size size;
      size.set(WHEEL_ITEM_WIDTH,pixAlpha);
      HRESULT t_result = 
        xvars.Blt(p_buffer,m_pos,
                  surf,Pos(surfPos.x,surfPos.y + WHEEL_ITEM_HEIGHT - pixAlpha),
                  size);
      DHRESULT(t_result,return t_result);
    }

    // Draw previous item in bottom of weapon wheel.
    if (pixAlpha < WHEEL_ITEM_HEIGHT) {
      Pos surfPos;
      LPDIRECTDRAWSURFACE surf = 
        xvars.m_surfaceManager->lookup(surfPos,compute_key(bitmapids[m_itemPrev]));

      // Draw rect of height (WHEEL_ITEM_HEIGHT - pixAlpha).
      Size size;
      size.set(WHEEL_ITEM_WIDTH,WHEEL_ITEM_HEIGHT - pixAlpha);
      HRESULT t_result = 
        xvars.Blt(p_buffer,Pos(m_pos.x,m_pos.y + pixAlpha),
                  surf,surfPos,
                  size);
      DHRESULT(t_result,return t_result);
    }

    // Don't draw text if we are animating.
  }

  return DD_OK;
}



HRESULT Wheel::drawText(LPDIRECTDRAWSURFACE p_buffer,Xvars &xvars) {
  // Draw m_value in upper-right of wheel.
  //
  HDC t_pixhdc;
  if (!xvars.GetDC(p_buffer,&t_pixhdc)){
    return DD_OK; //nothing to see here..
  }

  CDC *t_dc = CDC::FromHandle(t_pixhdc);
  COLORREF t_cref;
  switch(m_ready[m_item]) {
    case READY_YES:
      // Green, weapon ready to shoot
      t_cref = RGB(0x00,0x7f,0x00);
      break;
    case READY_NO:
      // Red, weapon is not ready
      t_cref = RGB(0xff,0x00,0x00);
      break;
    case READY_UNDEFINED:
      // Black, for items
      t_cref = RGB(0x00,0x00,0x00);
      break;
    default:
      ASSERT(0);
  }
  CPen t_pen;
  COLORREF t_prevcolor = t_dc->SetTextColor(t_cref);
  t_dc->SelectStockObject(ANSI_VAR_FONT);
  t_dc->SelectStockObject(WHITE_PEN);
  t_dc->SetBkMode(TRANSPARENT);

  CString t_string;
  if (m_values[m_item] == PH_AMMO_UNLIMITED) {
    t_string = "*";
  }
  else if (m_values[m_item] >= 0) {
    itoa(m_values[m_item],t_string.GetBuffer(25),10);
    t_string.ReleaseBuffer();
  }
  else {
    ASSERT(0);
  }

  RECT t_textrect;
  SetRect(&t_textrect,
          m_pos.x,m_pos.y,
          m_pos.x + WHEEL_ITEM_WIDTH - 2,m_pos.y + WHEEL_ITEM_HEIGHT);
  t_dc->DrawText(
      t_string,	// pointer to string to draw 
      t_string.GetLength(),	// string length, in characters 
      &t_textrect, // pointer to structure with formatting dimensions  
      DT_RIGHT // text-drawing flags 
  );
  t_dc->SetTextColor(t_prevcolor);
  HRESULT t_result = p_buffer->ReleaseDC(t_pixhdc);

  return t_result;
}



Size Wheel::wheelSize;



XvarsValid Wheel::valid = XVARS_VALID_INIT;



// Means not yet initialized.
int Wheel::numbitmaps = -1;



CMN_BITS_ID Wheel::bitmapids[A_CLASSES_NUM];



int Wheel::bitmapIndex[A_None + 1];



const CMN_BITS_ID Wheel::backgroundBits = IDB_WHEELBACKGROUND;
