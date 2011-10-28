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

// "area.cpp"  
// Also has class Avoid.

#if X11
#ifndef NO_PRAGMAS
#pragma implementation "area.h"
#endif
#endif


// Include Files
#include "stdafx.h"
extern "C" {
  //#include <stdlib.h> // For the abs() function.
#include <limits.h> // For INT_MAX
}
#include <iostream.h>
#include "utils.h"
#include "coord.h"
#include "area.h"



Size Area::operator - (const Area &other) const {
  assert ((shape == AR_RECT) && (other.shape == AR_RECT));
  return pos - other.pos;
}



Boolean Area::operator == (const Area &other) const {
  assert(shape == AR_RECT && other.shape == AR_RECT);
  
  return pos == other.pos && size == other.size;
}



Area Area::operator + (const Size &shift) const {
  assert(shape == AR_RECT);
  Area ret(AR_RECT,pos + shift,size);
  return ret;
}



Area Area::operator - (const Size &shift) const {
  assert(shape == AR_RECT);
  Area ret(AR_RECT,pos - shift,size);
  return ret;
}



Area::Area(ARshape sh,const Loc &loc,const Dim &dim) {
  assert(sh == AR_RECT);
  
  shape = AR_RECT;
  pos.x = loc.c * WSQUARE_WIDTH;
  pos.y = loc.r * WSQUARE_HEIGHT;
  size.width = dim.colMax * WSQUARE_WIDTH;
  size.height = dim.rowMax * WSQUARE_HEIGHT;
}



Area::Area(InStreamP in) {
  read(in);
}



void Area::read(InStreamP in) {
  shape = AR_RECT;
  pos.read(in);
  size.read(in);
}



void Area::write(OutStreamP out) const {
  pos.write(out);
  size.write(out);
}



Box Area::get_box() const {
  assert (shape == AR_RECT);

  Loc loc;
  loc.r = (int)floor(pos.y * WSQUARE_HEIGHT_INV);
  loc.c = (int)floor(pos.x * WSQUARE_WIDTH_INV);

  // Always has dimension of at least 1x1.
  Loc finish;
  finish.r = (int)ceil((pos.y + size.height) * WSQUARE_HEIGHT_INV);
  finish.c = (int)ceil((pos.x + size.width) * WSQUARE_WIDTH_INV);
  
  Dim dim(finish.r - loc.r,finish.c - loc.c);
  
  Box box(loc,dim);
  return box;
}



void Area::wsquares(Loc list[AR_WSQUARES_MAX],int &nitems) const {
  nitems = 0;

  assert(shape == AR_RECT);
  Loc loc;
  for (loc.r = (int)floor(pos.y * WSQUARE_HEIGHT_INV);
       loc.r * WSQUARE_HEIGHT < pos.y + size.height;
       loc.r++) {
    for (loc.c = (int)floor(pos.x * WSQUARE_WIDTH_INV);
         loc.c * WSQUARE_WIDTH < pos.x + size.width;
         loc.c++) {
      if (nitems >= AR_WSQUARES_MAX) {
	    cerr << "Warning: Area::wsquares: Too many wsquares." << endl;
	    return;
	  }
      list[nitems] = loc;
      nitems++;
    }
  }
}



Boolean Area::overlap(const Area &r) const {
  assert((shape == AR_RECT) && (r.shape == AR_RECT));

  if ((pos.x + size.width <= r.pos.x) ||
      (r.pos.x + r.size.width <= pos.x) ||
      (pos.y + size.height <= r.pos.y) ||
      (r.pos.y + r.size.height <= pos.y)) {
    return False;
  }
  else {
    return True;
  }
}



Boolean Area::overlap(const Loc &loc) const {
  Pos test;
  Boolean ret;

  assert(shape == AR_RECT);
  
  test.x = loc.c * WSQUARE_WIDTH;
  test.y = loc.r * WSQUARE_HEIGHT;

  if ((test.x >= pos.x + size.width) || 
      (test.y >= pos.y + size.height) ||
      (test.x + WSQUARE_WIDTH <= pos.x) || 
      (test.y + WSQUARE_HEIGHT <= pos.y)) {
    ret = False;
  }
  else {
    ret = True;
  }

  return ret;
}



Boolean Area::overlap(const Box &box) const {
  Pos testPos;
  Size testSize;
  Boolean ret;

  assert(shape == AR_RECT);
  
  testPos.x = box.loc.c * WSQUARE_WIDTH;
  testPos.y = box.loc.r * WSQUARE_HEIGHT;
  testSize.width = box.dim.colMax * WSQUARE_WIDTH;
  testSize.height = box.dim.rowMax * WSQUARE_HEIGHT;

  if ((testPos.x >= pos.x + size.width) || 
      (testPos.y >= pos.y + size.height) ||
      (testPos.x + testSize.width <= pos.x) || 
      (testPos.y + testSize.height <= pos.y)) {
    ret = False;
  }
  else {
    ret = True;
  }
  
  return ret;
}



Boolean Area::overlap(const Pos &p) const {
  assert(shape == AR_RECT);

  if ((pos.x + size.width <= p.x) ||
      (pos.x > p.x) ||
      (pos.y + size.height <= p.y) ||
      (pos.y > p.y)) {
    return False;
  }
  else {
    return True;
  }
}



Loc Area::middle_wsquare() const {
  assert (shape == AR_RECT);

  Loc ret;
  ret.r = (int)((pos.y + size.height * 0.5) * WSQUARE_HEIGHT_INV);
  ret.c = (int)((pos.x + size.width * 0.5) * WSQUARE_WIDTH_INV);
  return ret;
}



ARsig Area::avoid_wsquare(Avoid &avoid,const Loc &loc) const {
  assert(shape == AR_RECT);
  
  if (! overlap(loc)) {
    return AR_NO_SIG;
  }

  avoid.r = -(loc.c * WSQUARE_WIDTH - size.width - pos.x);
  avoid.dn = -(loc.r * WSQUARE_HEIGHT - size.height - pos.y);
  avoid.l = (loc.c + 1) * WSQUARE_WIDTH - pos.x;
  avoid.up = (loc.r + 1) * WSQUARE_HEIGHT - pos.y;
  
  return AR_CLOSE;
}



int Area::avoid_wsquare_dir(const Loc &loc,Dir dir) const {
  assert(shape == AR_RECT);
  assert(overlap(loc));

  switch (dir) {
  case CO_R:
    return (loc.c + 1) * WSQUARE_WIDTH - pos.x;
  case CO_DN:
    return (loc.r + 1) * WSQUARE_HEIGHT - pos.y;
  case CO_L:
    return -(loc.c * WSQUARE_WIDTH - size.width - pos.x);
  case CO_UP:
    return -(loc.r * WSQUARE_HEIGHT - size.height - pos.y);
  };

  assert(0);
  return CO_air;
}



int Area::avoid_area_dir(const Area &other,Dir dir) const {
  assert(shape == AR_RECT && other.shape == AR_RECT);
  assert(overlap(other));

  // Amount this shifted to avoid other.
  switch (dir) {
    case CO_R:
      return other.pos.x + other.size.width - pos.x;
    case CO_DN:
      return other.pos.y + other.size.height - pos.y;
    case CO_L:
      return pos.x + size.width - other.pos.x ;
    case CO_UP:
      return pos.y + size.height - other.pos.y;
  }

  assert(0);
  return CO_air;
}



Size Area::avoid(const Area &other) const {
  assert((shape == AR_RECT) && (other.shape == AR_RECT));

  Avoid avoid;
  avoid.r = pos.x + size.width - other.pos.x;
  avoid.dn = pos.y + size.height - other.pos.y;
  avoid.l = other.pos.x + other.size.width - pos.x;
  avoid.up = other.pos.y + other.size.height - pos.y;

  return avoid.offset_rank();
}



Size Area::avoid_no_up(const Area &other) const {
  assert((shape == AR_RECT) && (other.shape == AR_RECT));

  Avoid avoid;
  avoid.r = -(other.pos.x  - size.width - pos.x);
  avoid.dn = INT_MAX;
  avoid.l = other.pos.x + other.size.width - pos.x;
  avoid.up = other.pos.y + other.size.height - pos.y;

  return avoid.offset_rank();
}



Pos Area::adjacent_rect(const Size &otherSize,Dir dir) const {
  assert (shape == AR_RECT);

  Pos middle = get_middle();
  Pos ret;

  switch (dir) {
  case CO_UP_R_R:
  case CO_R:
  case CO_DN_R_R:
    ret.x = pos.x + size.width;
    ret.y = middle.y - (int)(0.5 * otherSize.height);
    break;
  case CO_DN_R:
    ret = pos + size;
    break;
  case CO_DN_DN_R:
  case CO_DN:
  case CO_DN_DN_L:
    ret.x = middle.x - (int)(0.5 * otherSize.width);
    ret.y = pos.y + size.height;
    break;
  case CO_DN_L:
    ret.x = pos.x - otherSize.width;
    ret.y = pos.y + size.height;
    break;
  case CO_DN_L_L:
  case CO_L:
  case CO_UP_L_L:
    ret.x = pos.x - otherSize.width;
    ret.y = middle.y - (int)(0.5 * otherSize.height);
    break;
  case CO_UP_L:
    ret = pos - otherSize;
    break;
  case CO_UP_UP_L:
  case CO_UP:
  case CO_UP_UP_R:
    ret.x = middle.x - (int)(0.5 * otherSize.width);
    ret.y = pos.y - otherSize.height;
    break;
  case CO_UP_R:
    ret.x = pos.x + size.width;
    ret.y = pos.y - otherSize.height;
    break;
  default: 
    assert(0);
    break;
  }

  return ret;
}



Boolean Area::touches(const Area &oth,Touching touching) const {
  assert(shape == AR_RECT && oth.shape == AR_RECT);

  switch (touching) {
    case CO_r:
      return pos.x + size.width == oth.pos.x &&
                pos.y < oth.pos.y + oth.size.height &&
                pos.y + size.height > oth.pos.y;      

    case CO_dn:
      return pos.y + size.height == oth.pos.y &&
                pos.x < oth.pos.x + oth.size.width &&
                pos.x + size.width > oth.pos.x;      

    case CO_l:
      return pos.x == oth.pos.x + oth.size.width &&
                pos.y < oth.pos.y + oth.size.height &&
                pos.y + size.height > oth.pos.y;      

    case CO_up:
      return pos.y == oth.pos.y + oth.size.height &&
                pos.x < oth.pos.x + oth.size.width &&
                pos.x + size.width > oth.pos.x;      

  default:
    assert(0);
    return False;
  }
}



void Area::touching_wsquares(TouchingList &list) const {
  assert(shape == AR_RECT);
  
  list.r.num = list.dn.num = list.l.num = list.up.num = 0;
  
  // Right side.
  if ((pos.x + size.width) % WSQUARE_WIDTH == 0) {
    Loc loc;
    loc.c = (pos.x + size.width) / WSQUARE_WIDTH;
    
    for (loc.r = (int)floor(pos.y * WSQUARE_HEIGHT_INV);
         loc.r * WSQUARE_HEIGHT < pos.y + size.height;
         loc.r++) {
	  list.r.list[list.r.num] = loc;
	  list.r.num++;
	}
  }
  
  // Bottom.
  if ((pos.y + size.height) % WSQUARE_HEIGHT == 0) {
    Loc loc;
    loc.r = (pos.y + size.height) / WSQUARE_HEIGHT;
    
    for (loc.c = (int)floor(pos.x * WSQUARE_WIDTH_INV);
         loc.c * WSQUARE_WIDTH < pos.x + size.width;
         loc.c++) {
	  list.dn.list[list.dn.num] = loc;
	  list.dn.num++;
	}
  }
  
  // Left side.
  if (pos.x % WSQUARE_WIDTH == 0) {
    Loc loc;
    loc.c = pos.x / WSQUARE_WIDTH - 1;
    
    for (loc.r = (int)floor(pos.y * WSQUARE_HEIGHT_INV);
         loc.r * WSQUARE_HEIGHT < pos.y + size.height;
         loc.r++) {
	  list.l.list[list.l.num] = loc;
	  list.l.num++;
	}
  }
  
  // Top.
  if (pos.y % WSQUARE_HEIGHT == 0) {
    Loc loc;
    loc.r = pos.y / WSQUARE_HEIGHT - 1;
    
    for (loc.c = (int)floor(pos.x * WSQUARE_WIDTH_INV);
         loc.c * WSQUARE_WIDTH < pos.x + size.width;
         loc.c++) {
	  list.up.list[list.up.num] = loc;
	  list.up.num++;
	}
  }
}



void Area::edge_wsquares(TouchingList &list) const {
  assert(shape == AR_RECT);
  
  list.r.num = list.dn.num = list.l.num = list.up.num = 0;
  
  // Right side.
  {
    Loc loc;
    loc.c = (pos.x + size.width) / WSQUARE_WIDTH;
    
    for (loc.r = (int)floor(pos.y * WSQUARE_HEIGHT_INV);
         loc.r * WSQUARE_HEIGHT < pos.y + size.height;
         loc.r++) {
      list.r.list[list.r.num] = loc;
      list.r.num++;
    }
  }
  
  // Bottom.
  {
    Loc loc;
    loc.r = (pos.y + size.height) / WSQUARE_HEIGHT;
    
    for (loc.c = (int)floor(pos.x * WSQUARE_WIDTH_INV);
         loc.c * WSQUARE_WIDTH < pos.x + size.width;
         loc.c++) {
      list.dn.list[list.dn.num] = loc;
      list.dn.num++;
    }
  }
  
  // Left side.
  {
    Loc loc;
    loc.c = (int)ceil(pos.x * WSQUARE_WIDTH_INV) - 1;
    
    for (loc.r = (int)floor(pos.y * WSQUARE_HEIGHT_INV);
         loc.r * WSQUARE_HEIGHT < pos.y + size.height;
         loc.r++) {
      list.l.list[list.l.num] = loc;
      list.l.num++;
    }
  }

  // Top.
  {
    Loc loc;
    loc.r = (int)ceil(pos.y * WSQUARE_HEIGHT_INV) - 1;
    
    for (loc.c = (int)floor(pos.x * WSQUARE_WIDTH_INV);
         loc.c * WSQUARE_WIDTH < pos.x + size.width;
         loc.c++) {
      list.up.list[list.up.num] = loc;
      list.up.num++;
    }
  }
}



Boolean Area::corner_offset(Size &offset,Loc list[],Loc &inner,
                            Dir corner) const {
  assert(shape == AR_RECT);
  Pos opp = pos + size; // lower right of area.
  
  // the 0,1,2 in the comments tell which wsquares will be returned in list.
  // x is the one that will be returned in inner.
  switch (corner) {
    // down right corner.
    // x0
    // 21
  case CO_DN_R:
    if (Utils::mod(opp.x,WSQUARE_WIDTH) > 0 &&
        Utils::mod(opp.y,WSQUARE_HEIGHT) > 0) {
      list[0].c = 
        list[1].c = Utils::div(opp.x,WSQUARE_WIDTH);
      list[2].c = 
        inner.c = list[0].c - 1;
      list[1].r = 
        list[2].r = Utils::div(opp.y,WSQUARE_HEIGHT);
      list[0].r = 
        inner.r = list[1].r - 1;
      offset.width = -Utils::mod(opp.x,WSQUARE_WIDTH);
      offset.height = -Utils::mod(opp.y,WSQUARE_HEIGHT);
      return True;
    }
    break;		
    
    // down left corner
    // 0x
    // 12
  case CO_DN_L:
    if (Utils::mod(pos.x,WSQUARE_WIDTH) > 0 &&
        Utils::mod(opp.y,WSQUARE_WIDTH) > 0) {
      list[0].c = 
        list[1].c = Utils::div(pos.x,WSQUARE_WIDTH);
      list[2].c = 
        inner.c = list[0].c + 1;
      list[1].r =
        list[2].r = Utils::div(opp.y,WSQUARE_HEIGHT);
      list[0].r = 
        inner.r = list[1].r - 1;
      offset.width = WSQUARE_WIDTH - Utils::mod(pos.x,WSQUARE_WIDTH);
      offset.height = -Utils::mod(opp.y,WSQUARE_HEIGHT);
      return True;
    }
    break;
    
    // upper left corner
    // 01
    // 2x
  case CO_UP_L:
    if (Utils::mod(pos.x,WSQUARE_WIDTH) > 0 &&
        Utils::mod(pos.y,WSQUARE_HEIGHT) > 0) {
      list[0].c = 
        list[2].c = Utils::div(pos.x,WSQUARE_WIDTH);
      list[1].c = 
        inner.c = list[0].c + 1;
      list[0].r =
        list[1].r = Utils::div(pos.y,WSQUARE_HEIGHT);
      list[2].r = 
        inner.r = list[1].r + 1;
      offset.width = WSQUARE_WIDTH - Utils::mod(pos.x,WSQUARE_WIDTH);
      offset.height = WSQUARE_HEIGHT - Utils::mod(pos.y,WSQUARE_HEIGHT);
      return True;
    }
    break;
    
    // upper right corner
    // 01
    // x2
  case CO_UP_R:
    if (Utils::mod(opp.x,WSQUARE_WIDTH) > 0 &&
        Utils::mod(pos.y,WSQUARE_HEIGHT) > 0) {
      list[1].c = 
        list[2].c = Utils::div(opp.x,WSQUARE_WIDTH);
      list[0].c = 
        inner.c = list[1].c - 1;
      list[0].r =
        list[1].r = Utils::div(pos.y,WSQUARE_HEIGHT);
      list[2].r = 
        inner.r = list[0].r + 1;
      offset.width = - Utils::mod(opp.x,WSQUARE_WIDTH);
      offset.height = WSQUARE_HEIGHT - Utils::mod(pos.y,WSQUARE_HEIGHT);
      return True;
    }
    break;
    
  default:
    assert(0);
  }
  return False;
}



void Area::set_middle(const Pos &mid) {
  assert(shape == AR_RECT);

  pos.x = mid.x - (int)floor(size.width * 0.5);
  pos.y = mid.y - (int)floor(size.height * 0.5);
}



Dir Area::dir_to(const Pos &other) const {
  assert(shape == AR_RECT);

  int xdir,ydir;

  if (other.x >= pos.x + size.width) {
    xdir = 1;
  }
  else if (other.x < pos.x) {
    xdir = -1;
  }
  else {
    xdir = 0;
  }

  if (other.y >= pos.y + size.height) {
    ydir = 1;
  }
  else if (other.y < pos.y) {
    ydir = -1;
  }
  else {
    ydir = 0;
  }

  switch (xdir) {
  case 1:
    switch (ydir) {
    case -1:
      return CO_UP_R;
    case 0:
      return CO_R;
    case 1:
      return CO_DN_R;
    };
  case 0:
    switch (ydir) {
    case -1:
      return CO_UP;
    case 0:
      return CO_air;
    case 1:
      return CO_DN;
    };
  case -1:
    switch (ydir) {
    case -1:
      return CO_UP_L;
    case 0:
      return CO_L;
    case 1:
      return CO_DN_L;
    };
  };

  assert(0);
  return CO_air;
}



Dir Area::dir_to(const Area &other) const
{
  assert((shape == AR_RECT) && (other.shape == AR_RECT));

  int xdir,ydir;

  if (other.pos.x >= pos.x + size.width)
    xdir = 1;
  else if (other.pos.x + other.size.width <= pos.x)
    xdir = -1;
  else
    xdir = 0;

  if (other.pos.y >= pos.y + size.height)
    ydir = 1;
  else if (other.pos.y + other.size.height <= pos.y)
    ydir = -1;
  else
    ydir = 0;

  switch (xdir) {
  case 1:
    switch (ydir) {
    case -1:
      return CO_UP_R;
    case 0:
      return CO_R;
    case 1:
      return CO_DN_R;
    };
  case 0:
    switch (ydir) {
    case -1:
      return CO_UP;
    case 0:
      return CO_air;
    case 1:
      return CO_DN;
    };
  case -1:
    switch (ydir) {
    case -1:
      return CO_UP_L;
    case 0:
      return CO_L;
    case 1:
      return CO_DN_L;
    };
  };

  assert(0);
  return CO_air;
}



Area Area::combine(const Area &a) const
{
  assert((shape == AR_RECT) && (a.shape == AR_RECT));
  Pos pos1;
  pos1.x = Utils::minimum(pos.x,a.pos.x);
  pos1.y = Utils::minimum(pos.y,a.pos.y);

  Pos pos2;
  pos2.x = Utils::maximum(pos.x + size.width,a.pos.x + a.size.width);
  pos2.y = Utils::maximum(pos.y + size.height,a.pos.y + a.size.height);

  Size s = pos2 - pos1;

  Area ret(AR_RECT,pos1,s);
  return ret;
}




Area Area::wsquare_allign(const Size &offset) const
{
	assert(shape == AR_RECT);

	Pos lr = pos + size; // lower right of area.
	Pos retPos = pos;
	Size retSize = size;

  // allign right side
	if (offset.width > 0 && lr.x % WSQUARE_WIDTH != 0)
		retPos.x += ((lr.x / WSQUARE_WIDTH) + 1) * WSQUARE_WIDTH - lr.x;

  // allign bottom side
	if (offset.height > 0 && lr.y % WSQUARE_HEIGHT != 0)
		retPos.y += ((lr.y / WSQUARE_HEIGHT) + 1) * WSQUARE_HEIGHT - lr.y;

	// allign left side.
	if (offset.width < 0 && pos.x % WSQUARE_WIDTH != 0)
		retPos.x = (pos.x / WSQUARE_WIDTH) * WSQUARE_WIDTH;

	// allign top side.
	if (offset.height < 0 && pos.y % WSQUARE_HEIGHT != 0)
		retPos.y = (pos.y / WSQUARE_HEIGHT) * WSQUARE_HEIGHT;

	Area ret(AR_RECT,retPos,retSize);
	return ret;
}



Boolean Area::wsquare_alligned() const
{
	assert(shape == AR_RECT);
	return pos.x % WSQUARE_WIDTH == 0 &&
				 pos.y % WSQUARE_HEIGHT == 0 &&
				 (pos.x + size.width) % WSQUARE_WIDTH == 0 &&
				 (pos.y + size.height) % WSQUARE_HEIGHT == 0;
}



Area Area::grow_by(int delta) const
{
	assert(shape == AR_RECT);
  Pos p(pos.x - delta,pos.y - delta);
  Size s;
  s.set(size.width + 2 * delta,size.height + 2 * delta);
  Area ret(AR_RECT,p,s);
  return ret;
}



Size Area::clip_top(int val) {
	assert(shape == AR_RECT);
  if (val > pos.y) {
    int diff = val - pos.y;
    pos.y += diff;
    size.height -= diff;

    // Clipped everything away.
    assert(size.height > 0);
    Size ret;
    ret.set(0,diff);
    return ret;
  }

  Size ret;
  ret.set_zero();
  return ret;
}



Size Area::clip_left(int val) {
	assert(shape == AR_RECT);
  if (val > pos.x) {
    int diff = val - pos.x;
    pos.x += diff;
    size.width -= diff;

    // Clipped everything away.
    assert(size.width > 0);
    Size ret;
    ret.set(diff,0);
    return ret;
  }

  Size ret;
  ret.set_zero();
  return ret;
}



Size Area::clip(const Size &clipper) {
  assert(shape == AR_RECT);
  Size ret;
  ret.set_zero();

  if (pos.x < 0) {
    ret.width = -pos.x;
    pos.x = 0;
    size.width -= ret.width;
  }

  if (pos.y < 0) {
    ret.height = -pos.y;
    pos.y = 0;
    size.height -= ret.height;
  }

  if (pos.x + size.width > clipper.width) {
    size.width = clipper.width - pos.x;
  }

  if (pos.y + size.height > clipper.height) {
    size.height = clipper.height - pos.y;
  }

  assert(size.width > 0 && size.height > 0);
  return ret;
}



/////////////////////////////// Avoid ///////////////////////////////////////
void Avoid::maximize(const Avoid &avoid) {
  r = Utils::maximum(r,avoid.r);
  dn = Utils::maximum(dn,avoid.dn);
  l = Utils::maximum(l,avoid.l);
  up = Utils::maximum(up,avoid.up);
}



Size Avoid::offset_rank(int rank) const {
  assert ((rank >= 0) && (rank < 4));

  int dists[4];
  dists[0] = r;
  dists[1] = dn;
  dists[2] = l;
  dists[3] = up;
  Utils::insertion_sort(dists,4);
  int m = dists[rank];      
  
  Size ret;
  if (m == r)
    {
      ret.width = -r;
      ret.height = 0;
      return ret;
    }
  if (m == dn)
    {
      ret.width = 0;
      ret.height = -dn;
      return ret;
    }
  if (m == l)
    {
      ret.width = l;
      ret.height = 0;
      return ret;
    }
  if (m == up)
    {
      ret.width = 0;
      ret.height = up;
      return ret;
    }
  assert(0);
  ret.width = ret.height = 0;
  return ret;
}



Size Avoid::offset_dir(Dir dir) const {
  Size ret;
  switch(dir)
    {
    case CO_R:
      ret.width = -r;
      ret.height = 0;
      break;
    case CO_DN:
      ret.width = 0;
      ret.height = -dn;
      break;
    case CO_L:
      ret.width = l;
      ret.height = 0;
      break;
    case CO_UP:
      ret.width = 0;
      ret.height = up;
      break;
    default:
      assert(0);
    }
  return ret;
}



int Avoid::get_dir(Dir dir) const {
  switch (dir) {
  case CO_R:
    return r;
  case CO_DN:
    return dn;
  case CO_L:
    return l;
  case CO_UP:
    return up;
  };
  assert(0);
  return CO_air;
}



//////////////////////////// ViewportRegion and ViewportInfo //////////////////////////
ViewportRegion::ViewportRegion(const Area &a) {
  // Not used anymore.
  assert(0);

  type = V_AREA; 
  area = a;
}



ViewportRegion::ViewportRegion(const Pos &p,int rad,const Area &visibleArea) {
  type = V_DISK; 
  pos = p; 
  radius = rad;
  area = visibleArea;
}



Boolean ViewportRegion::visibleOverlap(const Area &pArea) const {
  if (type == V_AREA) {
    return area.overlap(pArea);
  }
  if (type == V_DISK) {
    // Use inner,visible area
    return area.overlap(pArea);
  }
  assert(0);
  return False;
}



Boolean ViewportRegion::peripheralOverlap(const Area &pArea) const {
  if (type == V_AREA) {
    return area.overlap(pArea);
  }
  if (type == V_DISK) {
    // Use outer,peripheral disk.
    int dist_2 = pos.distance_2(pArea.get_middle());
    int radius_2 = radius * radius;
    return (dist_2 <= radius_2);
  }
  assert(0);
  return False;
}



void ViewportRegion::grow_by(int val) {
  if (type == V_AREA) {
    area.grow_by(val);
  }
  else if (type == V_DISK) {
    // grow visible area
    area.grow_by(val);
    // grow peripheral
    radius += val;
  }
  else {
    assert(0);
  }
}



ViewportInfo::ViewportInfo(int r,const Size &visibleSize) {
  smoothScroll = True; 
  radius = r; 
  logicalSize = visibleSize;
  extraSize.set(0,0); // Actually not used, but we do read/write it.  
}



ViewportInfo::ViewportInfo(int r,const Size &lSize,const Size &exSize) {
  smoothScroll = False; 
  radius = r;
  logicalSize = lSize; 
  extraSize = exSize;
}



void ViewportInfo::read(InStreamP in) {
  smoothScroll = (Boolean)in->read_char();
  logicalSize.read_32(in);
  extraSize.read(in);
  radius = in->read_int();
}



int ViewportInfo::get_write_length() {
  return 
    sizeof(char) +                   // smoothScroll
    Size::get_write_length_32() +    // logicalSize
    Size::get_write_length() +       // extraSize
    sizeof(int);                     // radius
}



void ViewportInfo::write(OutStreamP out) const {
  out->write_char((char)smoothScroll);
  logicalSize.write_32(out);
  extraSize.write(out);
  out->write_int(radius);
}



ViewportRegion ViewportInfo::get_viewport_for(const Pos &pos) const {
  // This simulates the code in Ui::viewport_to_loc on UNIX and
  // Ui::viewport_to_pos on Windows.

  // Windows
  if (smoothScroll) {
    Pos vPos = pos - 0.5f * logicalSize;
    Area visible(AR_RECT,vPos,logicalSize);    
    ViewportRegion ret(pos,radius,visible);
    return ret;
  }
  // UNIX
  else {
    Pos vPos((pos.x / logicalSize.width) * logicalSize.width 
             - extraSize.width,
             (pos.y / logicalSize.height) * logicalSize.height 
             - extraSize.height);
    Size vSize;
    vSize.set(logicalSize.width + 2 * extraSize.width,
              logicalSize.height + 2 * extraSize.height);
    
    Area visible(AR_RECT,vPos,vSize);
    ViewportRegion ret(pos,radius,visible);
    return ret;
  }
}



Dim ViewportInfo::get_viewport_dim() const {
  Size size;
  if (smoothScroll) {
    size = logicalSize;
  }
  else {
    size.set(logicalSize.width + 2 * extraSize.width,
             logicalSize.height + 2 * extraSize.height);
  }
  assert(size.width % WSQUARE_WIDTH == 0 &&
         size.height % WSQUARE_HEIGHT == 0);
  Dim dim(size.height / WSQUARE_HEIGHT,
          size.width / WSQUARE_WIDTH);
  return dim;
}

