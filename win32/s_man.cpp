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
#include "utils.h"
#include "s_man.h"
#include "xdata.h"



// Manages a set of surfaces that all share the same DirectDraw properties.
class SurfaceSet {
public:
  SurfaceSet(DDCOLORKEY* colorKey,int colHeightMax);
  /* EFFECTS: Create a SurfaceManager to manage surfaces with the given colorKey.
     NULL means don't use a colorKey.  Makes copy of colorKey. */

  ~SurfaceSet();

  void add(void* key,const Size& size);
  /* REQUIRES: size.width > 0 && size.height > 0 */
  /* EFFECTS: Add a request for a rectangle of a given size.  size is in stretched
     coordinates.  The rectangle will be associated with "key".  You may add() the 
     same (key,size) pair multiple times, it will only be allocated one 
     rectangle. */

  const DDCOLORKEY* get_color_key() {return colorKey;}
  /* EFFECTS: Return a pointer to the color key associated with this SurfaceSet
     or NULL if no color key. */

  LPDIRECTDRAWSURFACE lookup(Area& area,void* key);
  /* MODIFIES: area */
  /* EFFECTS: Lookup the surface area for "key".  Return the DirectDrawSurface 
     for that key and set area to be the rectangle on that surface reserved for 
     that key.  Return NULL if not in this set. */

  HRESULT add_complete(Xvars& xvars,int& pixAllc,int& pixUsed);
  /* MODIFIES: pixAllc, pixUsed */
  /* EFFECTS: All rectangle requests have been submitted, generate the surfaces. 
     Set pixAllc and pixUsed to the number of pixels allocated and used, 
     respectively. */

  int debug_pages_num(const Size& size);
  /* EFFECTS: Computes how many debugging pages will be needed to display all 
     the surfaces in this SurfaceSet. */

  void draw_debug_page(Xvars&,LPDIRECTDRAWSURFACE surf,const Size& size,int pageNum);
  /* REQUIRES: 0 <= pageNum < debug_pages_num() */
  /* EFFECTS: Draw one debugging page onto surf.  surf has the given size. */


private:
  enum {
    // Don't use powers of 2 or 10 with our crappy hash function.
    HASHTABLE_ENTRIES = 297, 
  };
  
  // Data for each column.  The surface and the size of the surface.
  struct ColData {
    Size size;
    LPDIRECTDRAWSURFACE surf;
  };

  // The rectangular region associated with a key.
  struct KeyData {
    Size size;
    int tableIndex;
    int colNum;
    int yOffset;
  };

  // Maximum height of each of the columns in an entry in table.
  int colHeightMax;

  // Has add_complete() been called.
  // Redundant data, could have pointer back to containing SurfaceManager.
  Boolean addCompleted;

  // The colorkey for the SurfaceManager.
  DDCOLORKEY *colorKey;

  // Each entry in table has a list of (ColData*).  
  //
  // table[n] is a list of columns of rectangles where 
  // (2^n <= rectangle.width < 2^(n+1)
  // 
  // Before add_complete(), it keeps a running total of the rectangles 
  // added so far.
  //
  // After add_complete(), the surfaces are filled in.
  PtrList table;

  // HashTable from key->KeyData
  IDictionary* hash;
};



SurfaceSet::SurfaceSet(DDCOLORKEY *cKey,int cHeightMax) {  
  addCompleted = False;
  colHeightMax = cHeightMax;

  // Make a copy of the colorKey.
  if (cKey) {
    colorKey = new DDCOLORKEY;
    assert(colorKey);
    *colorKey = *cKey;
  }
  else {
    colorKey = NULL;
  }

  hash = HashTable_factory(HASHTABLE_ENTRIES);
}



SurfaceSet::~SurfaceSet() {
  delete colorKey;
  
  // Wipe out hash table and everything in it.
  IDictIterator* iter = hash->iterate();
  void* key;
  KeyData* kData;
  while (kData = (KeyData*)iter->next(key)) {
    delete kData;
  }
  delete iter;
  delete hash;

  // Kill everything inside table.
  for (int n = 0; n < table.length(); n++) {
    PtrList* cols = (PtrList*)table.get(n);
    if (cols) {
      for (int m = 0; m < cols->length(); m++) {
        ColData* cData = (ColData*)cols->get(m);
        assert(cData);
        delete cData;
        if (addCompleted) {
// Let the surface get released when the IDirectDraw object is killed.
//          assert(cData->surf);
//          cData->surf->Release();
        }
      }
      delete cols;
    }
  }
}



void SurfaceSet::add(void* key,const Size& size) {
  assert(size.width > 0 && size.height > 0);
  assert(size.height <= colHeightMax);

  // If request has already been added, do nothing.
  KeyData* existingKData = (KeyData*)hash->get(key);
  if (existingKData) {
    assert(size == existingKData->size);
    return;
  }
  
  
  unsigned int width = size.width;

  // Poor man's base-2 logarithm
  int lgWidth = 0;
  while ((width >>= 1) > 0) {
    lgWidth++;
  }

  // The log of width gives the index into table.
  PtrList* cols = NULL;
  if (lgWidth < table.length()) {
    cols = (PtrList*)table.get(lgWidth);
  }
  // else it is definitely not in the table.

  // If no list of columns has been created for this range of widths, 
  // create the list now.
  if (!cols) {
    // Make guess that that we will only need one column in cols.
    cols = new PtrList(1);
    assert(cols);

    // Create one empty column in cols.
    ColData* newData = new ColData;
    assert(newData);
    newData->size.set_zero();
    newData->surf = NULL;
    cols->add(newData);
    
    // Store the new column list at lgWidth.
    table.set_and_fill(lgWidth,(void*)cols);
  }

  // cols guaranteed to have length() >= 1 at this point.

  // The data for the newly allocated rectangle.
  KeyData* kData = new KeyData;
  assert(kData);
  kData->size = size;
  kData->tableIndex = lgWidth;

  // Try to add rectangle to the end of the last column.
  ColData* colData = (ColData*)cols->get(cols->length() - 1);
  if (colData->size.height + size.height < colHeightMax) {
    kData->yOffset = colData->size.height;
    colData->size.height += size.height;
    colData->size.width = Utils::maximum(colData->size.width,size.width);
  }
  // Can't fit this rectangle, start a new column in cols.
  else {
    ColData* newData = new ColData;
    assert(newData);
    newData->size = size;
    newData->surf = NULL;
    cols->add(newData);
    kData->yOffset = 0;
  }
  kData->colNum = cols->length() - 1;

  existingKData = (KeyData*)hash->put(key,(void*)kData);
  assert(!existingKData);
}



LPDIRECTDRAWSURFACE SurfaceSet::lookup(Area& area,void* key) {
  KeyData* kData = (KeyData*)hash->get(key);
  if (!kData) {
    // Not in this SurfaceSet.
    return NULL;
  }

  Area areaRet(Pos(0,kData->yOffset),kData->size);
  area = areaRet;

  PtrList* cols = (PtrList*)table.get(kData->tableIndex);
  ColData* cData = (ColData*)cols->get(kData->colNum);
  if (cData->surf == NULL) {
    // Special case where SurfaceManager is calling SurfaceSet::lookup for
    // its own use, not in response to an external call to 
    // SurfaceManager::lookup().
    assert(!addCompleted);
    // Return anything non-NULL.
    return (LPDIRECTDRAWSURFACE)0x1;
  }
  return cData->surf;
}



HRESULT SurfaceSet::add_complete(Xvars& xvars,int& pixAllc,int& pixUsed) {
  // Number of pixels allocated for the surfaces.
  pixAllc = 0;
  // Number of pixels actually used in the allocated surfaces.
  pixUsed = 0;
  
  for (int n = 0; n < table.length(); n++) {
    PtrList* cols = (PtrList*)table.get(n);
    if (cols == NULL) {
      continue;
    }
    // Create a DirectDraw surface for each column.
    for (int m = 0; m < cols->length(); m++) {
      ColData* cData = (ColData*)cols->get(m);
      assert(cData->size.height > 0 && cData->size.width > 0);
      cData->surf = 
        xvars.create_surface(cData->size.width,cData->size.height,colorKey);
      // Fill with red to indicate the wasted pixels for debug drawing.
      HRESULT result = 
        xvars.color_fill(cData->surf,
                         CRect(0,0,cData->size.width,cData->size.height),
                         xvars.m_red);
      if (result != DD_OK) {
        return result;
      }
      pixAllc += cData->size.width * cData->size.height;
    }
  }

  // Compute number of pixels actually used.
  IDictIterator* iter = hash->iterate();
  KeyData* kData;
  void* key;
  while (kData = (KeyData*)iter->next(key)) {
    pixUsed += kData->size.width * kData->size.height;
  }
  delete iter;

  addCompleted = True;

  return DD_OK;
}



int SurfaceSet::debug_pages_num(const Size& pageSize) {
  assert(addCompleted);

  // Pages needed so far.
  int ret = 0;
  // width used up on the current page.
  // Start at pageSize.width so it is as if we have just finished the
  // -1th page.
  int widthUsed = pageSize.width;

  for (int n = 0; n < table.length(); n++) {
    PtrList* cols = (PtrList*)table.get(n);
    if (cols == NULL) {
      continue;
    }
    for (int m = 0; m < cols->length(); m++) {
      ColData* cData = (ColData*)cols->get(m);
      assert(cData->size.width <= pageSize.width);
      
      // Start new page.
      if (widthUsed + cData->size.width > pageSize.width) {
        ret++;
        widthUsed = 0;
      }
      // Add column width to current page.
      widthUsed += cData->size.width;
    }
  }
  return ret;
}



void SurfaceSet::draw_debug_page(Xvars& xvars,LPDIRECTDRAWSURFACE surf,
                                 const Size& pageSize,int pageNum) {
  assert(addCompleted);

  // Current page number, count until we get to "pageNum".
  int pCount = 0;
  // width used up on the current page.
  int widthUsed = 0;

  // Some code duplication from SurfaceSet::debug_pages_num(), uggh.
  for (int n = 0; n < table.length(); n++) {
    PtrList* cols = (PtrList*)table.get(n);
    if (cols == NULL) {
      continue;
    }
    for (int m = 0; m < cols->length(); m++) {
      ColData* cData = (ColData*)cols->get(m);
      assert(cData->size.width <= pageSize.width);

      // Start new page.
      if (widthUsed + cData->size.width > pageSize.width) {
        if (pCount == pageNum) {
          // We're done drawing all the columns on the target page.
          return;
        }
        pCount++;
        widthUsed = 0;
      }
      // We are on the target page, so draw the column.
      if (pCount == pageNum) {
        // Draw without colorkey even if the surface has one.
        xvars.Blt(surf,Pos(widthUsed,0),
                  cData->surf,Pos(0,0),cData->size,
                  False);      
      }
      // Add column width to current page.
      widthUsed += cData->size.width;
    } // for m
  } // for n
}



SurfaceManager::SurfaceManager(int hMax) {
  addCompleted = False;
  heightMax = hMax;
}



SurfaceManager::~SurfaceManager() {
  for (int n = 0; n < sets.length(); n++) {
    delete (SurfaceSet*)sets.get(n);
  }
}



void SurfaceManager::add(void* key,const Size& size,DDCOLORKEY *colorKey) {
  assert(!addCompleted);
  
  // See if request has already been added.
  // Assert that it was added in exactly the same way to try and detect
  // key collisions errors by the user of SurfaceManager.
  int n;
  for (n = 0; n < sets.length(); n++) {
    Area area;
    SurfaceSet* set = (SurfaceSet*)sets.get(n);
    LPDIRECTDRAWSURFACE surf = set->lookup(area,key);
    if (surf) {
      // Special flag to return before surfaces are generated.
      assert(surf == (LPDIRECTDRAWSURFACE)0x1);
      if (area.get_size() == size && 
          color_key_equals(set->get_color_key(),colorKey)) {
        // Key was added before in the exact same way, don't 
        // need to do anything.
        return;
      }
      // Error: Key was added before, but with a different size and/or 
      // color key.
      assert(0);
    }
  }
  
  int which = -1;
  // Add to existing SurfaceSet if we have one with the right color key.
  for (n = 0; n < sets.length(); n++) {
    SurfaceSet* set = (SurfaceSet*)sets.get(n);
    if (color_key_equals(set->get_color_key(),colorKey)) {
      which = n;
      break;
    }
  }

  // Create new SurfaceSet with the never-before seen color key.
  if (which == -1) {
    SurfaceSet* set = new SurfaceSet(colorKey,heightMax);
    assert(set);
    sets.add((void*)set);
    which = sets.length() - 1;
  }

  SurfaceSet* s = (SurfaceSet*)sets.get(which);
  s->add(key,size);
}



LPDIRECTDRAWSURFACE SurfaceManager::_lookup(Area& area,const DDCOLORKEY*& colorKey,
                                           void* key) {
  // Do not assert addCompleted, this is the internal version of lookup() that
  // may be called before add_complete().

  for (int n = 0; n < sets.length(); n++) {
    SurfaceSet* set = (SurfaceSet*)sets.get(n);
    LPDIRECTDRAWSURFACE ret = set->lookup(area,key);
    if (ret) {
      colorKey = set->get_color_key();
      return ret;
    }
  }
  assert(0);
  return NULL;
}



LPDIRECTDRAWSURFACE SurfaceManager::lookup(Area& area,const DDCOLORKEY*& colorKey,void* key) {
  assert(addCompleted); 
  return _lookup(area,colorKey,key);
}

  
  
LPDIRECTDRAWSURFACE SurfaceManager::lookup(Pos& pos,void* key) {
  Area area;
  const DDCOLORKEY* cKey;
  LPDIRECTDRAWSURFACE ret = lookup(area,cKey,key);
  pos = area.get_pos();
  return ret;
}



LPDIRECTDRAWSURFACE SurfaceManager::lookup(Size& size,void* key) {
  Pos pos;
  LPDIRECTDRAWSURFACE ret = lookup(pos,key);
  size.width = pos.x;
  size.height = pos.y;
  return ret;
}



HRESULT SurfaceManager::add_complete(Xvars& xvars) {
  pixAllc = 0;
  pixUsed = 0;

  for (int n = 0; n < sets.length(); n++) {
    int pAllc, pUsed;
    SurfaceSet* set = (SurfaceSet*)sets.get(n);
    HRESULT result = set->add_complete(xvars,pAllc,pUsed);
    if (result != DD_OK) {
      return result;
    }
    pixAllc += pAllc;
    pixUsed += pUsed;

    // Print stats.
    strstream str;
    str << "SurfaceSet: used " << pUsed << " out of " 
      << pAllc << " allocated." << ends;
    DebugInfo::print(str.str());
    delete str.str();
  }
  addCompleted = True;

  // Print stats.
  strstream str;
  str << "SurfaceManager: used " << pixUsed << " out of " 
    << pixAllc << " allocated." << ends;
  DebugInfo::print(str.str());
  delete str.str();

  return DD_OK;
}



Boolean SurfaceManager::color_key_equals(const DDCOLORKEY* cKey1,const DDCOLORKEY* cKey2) {
  if (cKey1 == NULL || cKey2 == NULL) {
    return (cKey1 == NULL && cKey2 == NULL);
  }
  // Safe to dereference both cKey1 and cKey2.
  if (cKey1->dwColorSpaceLowValue == cKey2->dwColorSpaceLowValue &&
     cKey1->dwColorSpaceHighValue == cKey2->dwColorSpaceHighValue) {
    return True;
  }
  return False;
}



int SurfaceManager::debug_pages_num(const Size& size) {
  assert(addCompleted);
  
  int ret = 0;
  for (int s = 0; s < sets.length(); s++) {
    SurfaceSet* set = (SurfaceSet*)sets.get(s);
    ret += set->debug_pages_num(size);
  }
  return ret;
}



void SurfaceManager::draw_debug_page(Xvars& xvars,LPDIRECTDRAWSURFACE surf,
                                     const Size& size,int pageNum) {
  // Fill background with green to indicate pixels that do not correspond to 
  // a SurfaceSet.
  xvars.color_fill(surf,CRect(0,0,size.width,size.height),xvars.m_green);
  
  // Not necessarily the most efficient in terms of counting pages multiple 
  // times, but who cares.  Worth it for simplicity.
  for (int s = 0; s < sets.length(); s++) {
    SurfaceSet* set = (SurfaceSet*)sets.get(s);
    int pNum = set->debug_pages_num(size);
    if (pageNum < pNum) {
      // The page is on this set, draw it.
      set->draw_debug_page(xvars,surf,size,pageNum);
      return;
    }
    else {
      // Count pageNum down by the number of pages in this set.
      pageNum -= pNum;
    }
  }
}
