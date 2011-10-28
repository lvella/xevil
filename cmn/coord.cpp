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

// "coord.cpp"

#if X11
#ifndef NO_PRAGMAS
#pragma implementation "coord.h"
#endif
#endif


// Include Files
#include "stdafx.h"
#include "utils.h"
#include "coord.h"
#include "area.h"


#ifndef NULL
#define NULL 0
#endif


// Functions.
void Stats::add_death(time_t birthTime) {
  if (enabled) {
    time_t lifespan = time(NULL) - birthTime;
    
    if (deaths > 0)
      aveLifespan = (float)((aveLifespan * deaths + lifespan) / (deaths + 1));
    else
      aveLifespan = (float)lifespan;
    
    deaths++;
  }
}



Boolean Stats::enabled = False;



void Pos::read(InStreamP in) {
  x = in->read_signed_short(); 
  y = in->read_signed_short();
}



int Pos::get_write_length() {
  return 2 * sizeof(short);
}



void Pos::write(OutStreamP out) const {
  out->write_signed_short(x); 
  out->write_signed_short(y);
}



int Pos::distance(const Pos &p) const
{
  Size diff = *this - p;
  assert (diff.abs_2() >= 0);
  return (int)sqrt((double)(diff.width * diff.width + diff.height * diff.height)); //stupid MS compiler gets ambiguous call without the recast to int
}



int Pos::distance_2(const Pos &p) const
{
  Size diff = *this - p;
  
  return diff.width * diff.width + diff.height * diff.height;
}



void Vel::read(InStreamP in) {
  dx = in->read_float(); 
  dy = in->read_float();
}



int Vel::get_write_length() {
  return 2 * sizeof(float);
}



void Vel::write(OutStreamP out) const {
  out->write_float(dx); 
  out->write_float(dy);
}



Dir Size::get_dir() {
  if (!width && !height) {
    return CO_air;
  }

  if (height < 0.5 * width) {
    if (height > -2 * width) {
      if (height < -0.5 * width) {
        return CO_UP_R;
      }
      else {
        return CO_R;
      }
    }
    else {
      if (height < 2 * width) {
        return CO_UP;
      }
      else {
        return CO_UP_L;
      } 
    }
  }
  else {
    if (height < -2 * width) {
      if (height < -0.5 * width) {
        return CO_L;
      }
      else {
        return CO_DN_L;
      }
    }
    else {
      if (height < 2 * width) {
        return CO_DN_R;
      }
      else {
        return CO_DN;
      }
    }
  }
}



void Size::get_dirs_4(Dir &d1,Dir &d2) {
  if (!width && !height) {
    d1 = d2 = CO_air;
    return;
  }

  if (width) {
    d1 = (width > 0) ? CO_R : CO_L;
  }
  
  if (height) {
    d2 = (height > 0) ? CO_DN : CO_UP;
  }
  else {
    d2 = d1;
  }

  if (!width) {
    d1 = d2;
  }
}



void Size::read(InStreamP in) {
  width = in->read_char(); 
  height = in->read_char();
}



int Size::get_write_length() {
  return 2 * sizeof(char);
}



void Size::write(OutStreamP out) const {
  assert(width <= UCHAR_MAX && height <= UCHAR_MAX);
  out->write_char((u_char)width); 
  out->write_char((u_char)height);
}



void Size::read_32(InStreamP in) {
  width = in->read_int(); 
  height = in->read_int();
}



int Size::get_write_length_32() {
  return 2 * sizeof(int);
}



void Size::write_32(OutStreamP out) const {
  out->write_int(width); 
  out->write_int(height);
}



float Size::cross(const Vel &vel) {
  return width * vel.dy - height * vel.dx;
}



float Size::dot(const Vel &vel) {
  return width * vel.dx + height * vel.dy;
}



void Loc::read(InStreamP in) {
  r = in->read_short(); 
  c = in->read_short();
}



int Loc::get_write_length() {
  return 2 * sizeof(short);
}



void Loc::write(OutStreamP out) const {
  assert(r <= USHRT_MAX && c <= USHRT_MAX);
  out->write_short((u_short)r); 
  out->write_short((u_short)c);
}



Boolean Box::overlap(const Loc &l) {
  if ((l.c >= loc.c) && (l.c < loc.c + dim.colMax) &&
      (l.r >= loc.r) && (l.r < loc.r + dim.rowMax)) {
    return True;
  }

  return False;
}



Vel Vel::shrink(float k) const {
  Vel ret = *this;

  if (ret.dx > k) {
    ret.dx -= k;
  }
  else if (ret.dx < -k) {
    ret.dx += k;
  }
  else {
    ret.dx = 0.0f;
  }

  if (ret.dy > k) {
    ret.dy -= k;
  }
  else if (ret.dy < -k) {
    ret.dy += k;
  }
  else {
    ret.dy = 0.0f;
  }

  return ret;
}



void Vel::damp(float k) {
  if (dx > k) {
    dx -= k;
  }
  else if (dx < -k) {
    dx += k;
  }
  else {
    dx = 0.0f;
  }

  if (dy > k) {
    dy -= k;
  }
  else if (dy < -k) {
    dy += k;
  }
  else {
    dy = 0.0f;
  }
}



Boolean Vel::is_zero() const {
  Boolean ret;
  if ((dx == 0) && (dy == 0)) {
    ret = True;
  }
  else {
    ret = False;
  }
  return ret;
}



Dir Vel::get_dir() const {
  if (!dx && !dy) {
    return CO_air;
  }

  if (dy < 0.5 * dx) {
    if (dy > -2 * dx) {
      if (dy < -0.5 * dx) {
        return CO_UP_R;
      }
      else {
        return CO_R;
      }
    }
    else {
      if (dy < 2 * dx) {
        return CO_UP;
      }
      else {
        return CO_UP_L;
      }
    }
  }
  else {
    if (dy < -2 * dx) {
      if (dy < -0.5 * dx) {
        return CO_L;
      }
      else {
        return CO_DN_L;
      }
    }
    else {
      if (dy < 2 * dx) {
        return CO_DN_R;
      }
      else {
        return CO_DN;
      }
    }
  }
}



void Vel::limit(float k) {
  assert (k >= 0);

  if (dx > k) {
    dx = k;
  }
  if (dx < -k) {
    dx = -k;
  }
  if (dy > k) {
    dy = k;
  }
  if (dy < -k) {
    dy = -k;
  }
}



void Vel::get_dirs_4(Dir in[4],Dir out[4],int &inNum,int &outNum) {
  inNum = 0;
  outNum = 0;

  if (dx > 0) {
    in[inNum] = CO_R;
    inNum++;
  }
  else {
    out[outNum] = CO_R;
    outNum++;
  }

  if (dy > 0) {
    in[inNum] = CO_DN;
    inNum++;
  }
  else {
    out[outNum] = CO_DN;
    outNum++;
  }

  if (dx < 0) {
    in[inNum] = CO_L;
    inNum++;
  }
  else {
    out[outNum] = CO_L;
    outNum++;
  }

  if (dy < 0) {
    in[inNum] = CO_UP;
    inNum++;
  }
  else {
    out[outNum] = CO_UP;
    outNum++;
  }

  assert(inNum + outNum == 4);
}



void Dim::read(InStreamP in) {
  rowMax = in->read_short(); 
  colMax = in->read_short();
}



int Dim::get_write_length() {
  return 2 * sizeof(short);
}



void Dim::write(OutStreamP out) const {
  assert(rowMax <= USHRT_MAX && colMax <= USHRT_MAX);
  out->write_short((u_short)rowMax); 
  out->write_short((u_short)colMax);
}



Boolean operator == (const Loc &l1, const Loc &l2) {
  return l1.r == l2.r && l1.c == l2.c;
}



Boolean operator == (const Pos &p1, const Pos &p2) {
  return p1.x == p2.x && p1.y == p2.y;
}



Boolean operator != (const Pos &p1, const Pos &p2) {
  return p1.x != p2.x || p1.y != p2.y;
}



Boolean operator == (const Vel &v1, const Vel &v2) {
  return v1.dx == v2.dx && v1.dy == v2.dy;
}



Boolean operator == (const Size &s1, const Size &s2) {
  if ((s1.width == s2.width) && (s1.height == s2.height))
    return True;
  else
    return False;
}



Boolean operator == (const GLoc &g1,const GLoc &g2) {
  return (g1.horiz == g2.horiz) && (g1.vert == g2.vert);
}



Boolean operator == (const Dim &d1,const Dim &d2) {
  return d1.rowMax == d2.rowMax && d1.colMax == d2.colMax;
}



Boolean operator == (const Rooms &r1,const Rooms &r2) {
  return r1.acrossMax == r2.acrossMax && r1.downMax == r2.downMax;
}



Boolean operator != (const GLoc &g1,const GLoc &g2) {
  return (g1.horiz != g2.horiz) || (g1.vert != g2.vert);
}



Pos operator + (const Pos &pos,const Size &size) {
  Pos ret;
  ret.x = pos.x + size.width;
  ret.y = pos.y + size.height;
  return ret;
}



Pos operator - (const Pos &pos,const Size &size) {
  Pos ret;
  ret.x = pos.x - size.width;
  ret.y = pos.y - size.height;
  return ret;
}



Size operator - (const Pos &p1,const Pos &p2) {
  Size ret;
  ret.width = p1.x - p2.x;
  ret.height = p1.y - p2.y;
  return ret;
}



Pos operator + (const Pos &pos, const Vel &vel) {
  Pos ret;
  
  /* We want it to round towards zero so that it does the same thing for 
     something going to the right as to the left. */
  ret.x = pos.x + (int)vel.dx;
  ret.y = pos.y + (int)vel.dy;
  /*  ret.x = pos.x + (int)trunc(vel.dx);
      ret.y = pos.y + (int)trunc(vel.dy);
      */
  return ret;
}



Size operator * (float k,const Size &size) {
  Size ret;
  ret.width = (int)floor(k * size.width);
  ret.height = (int)floor(k * size.height);
  return ret;
}



Vel operator + (const Vel &v1,const Vel &v2) {
  Vel ret(v1.dx + v2.dx,v1.dy + v2.dy);
  return ret;
}



Vel operator + (const Vel &vel, const Acc &acc) {
  Vel ret(vel.dx + acc.ddx,vel.dy + acc.ddy);
  return ret;
}

     

Vel operator + (const Vel &vel, const Size &size) {
  Vel ret(vel.dx + size.width,vel.dy + size.height);
  return ret;
}

     

Vel operator * (float k,const Vel &vel) {
  Vel ret(k * vel.dx, k * vel.dy);
  return ret;
}



Vel operator / (float k,const Vel &vel) {
  Vel ret(k / vel.dx, k / vel.dy);
  return ret;
}



Vel operator + (float k,const Vel &vel) {
  Vel ret(k + vel.dx,k + vel.dy);
  return ret;
}



Acc operator * (int k,const Acc &acc) {
  Acc ret;
  ret.ddx = k * acc.ddx;
  ret.ddy = k * acc.ddy;
  return ret;
}



Size operator + (const Size& s1,const Size& s2) {
  Size ret;
  ret.set(s1.width + s2.width,s1.height + s2.height);
  return ret;
}



#if 0
Size& operator += (Size& s1,const Size& s2) {
  s1.width += s2.width;
  s1.height += s2.height;
  return s1;
}
#endif



Boolean Coord::is_dir_pure(Dir dir) {
  return dir >= CO_R && dir < (CO_R + CO_DIR_PURE);
}



Dir Coord::dir_opposite(Dir dir) {
  if (dir == CO_air)
    return CO_air;

  assert(dir >= CO_R && dir < CO_DIR_MAX);

  return ((dir - CO_R + CO_DIR_PURE / 2) % CO_DIR_PURE) + CO_R;
}



Dir Coord::movement_dir_4(Dir dir) {
  switch(dir) {
  case CO_center:
  case CO_air:
  case CO_climb:
  case CO_r:
  case CO_dn:
  case CO_l:
  case CO_up:
    return CO_air;

  case CO_center_R:
  case CO_dn_R:
  case CO_up_R:
  case CO_air_R:
  case CO_climb_R:
  case CO_UP_R:
  case CO_UP_R_R:
  case CO_R:
  case CO_DN_R_R:
  case CO_DN_R:
    return CO_R;

  case CO_air_DN:
  case CO_r_DN:
  case CO_l_DN:
  case CO_climb_DN:
  case CO_DN_DN_R:
  case CO_DN:
  case CO_DN_DN_L:
    return CO_DN;
    
  case CO_center_L:
  case CO_dn_L:
  case CO_up_L:
  case CO_air_L:
  case CO_climb_L:
  case CO_UP_L:
  case CO_UP_L_L:
  case CO_L:
  case CO_DN_L_L:
  case CO_DN_L:
    return CO_L;

  case CO_air_UP:
  case CO_r_UP:
  case CO_l_UP:
  case CO_climb_UP:
  case CO_UP_UP_R:
  case CO_UP:
  case CO_UP_UP_L:
    return CO_UP;

  default:
    assert(0);
    return CO_air;
  }
}



Dir Coord::parallel_dir(Stance stance) {
  switch (stance) {
    case CO_r:
    case CO_l:
    case CO_climb:
      return CO_DN;
    case CO_up:
    case CO_dn:
    case CO_center:
      return CO_R;
    case CO_air:
      return CO_air;
    default:
      assert(0);
      return CO_air;
  }
}



Pos Coord::shot_initial_pos(const Area &area,Touching touching,
			                      const Size &shotSize,Dir shotDir) {
  // Return value;
  Pos ret;

  Pos pos;
  Size size;
  area.get_rect(pos,size);
  
  // Set x coord.
  switch (shotDir) {
  case CO_UP_R:
  case CO_UP_R_R:
  case CO_R:
  case CO_DN_R_R:
  case CO_DN_R:
    if (shotSize.width <= size.width)
      ret.x = pos.x + size.width - shotSize.width;
    else
      ret.x = pos.x;
    break;

  case CO_UP_L:
  case CO_UP_L_L:
  case CO_L:
  case CO_DN_L_L:
  case CO_DN_L:
    if (shotSize.width <= size.width)
      ret.x = pos.x;
    else
      ret.x = pos.x + size.width - shotSize.width;
    break;
    
  case CO_UP_UP_L:
  case CO_UP:
  case CO_UP_UP_R:
  case CO_DN_DN_L:
  case CO_DN:
  case CO_DN_DN_R:
  case CO_air:
    ret.x = (int)(pos.x + size.width * .5 - shotSize.width * .5);
    if (touching == CO_l) {
      ret.x = Utils::maximum(ret.x,pos.x);
    }
    else if (touching == CO_r) {
      ret.x = Utils::minimum(ret.x + shotSize.width,pos.x + size.width) - shotSize.width;
    }
    break;
    
  default:
    assert(0);
  }

  // Set y coord.
  switch (shotDir) {
  case CO_DN_L:
  case CO_DN_DN_L:
  case CO_DN:
  case CO_DN_DN_R:
  case CO_DN_R:
    if (shotSize.height <= size.height)
      ret.y = pos.y + size.height - shotSize.height;
    else
      ret.y = pos.y;
    break;
    
  case CO_UP_L:
  case CO_UP_UP_L:
  case CO_UP:
  case CO_UP_UP_R:
  case CO_UP_R:
    if (shotSize.height <= size.height)
      ret.y = pos.y;
    else
      ret.y = pos.y + size.height - shotSize.height;
    break;

  case CO_UP_R_R:
  case CO_R:
  case CO_DN_R_R:
  case CO_UP_L_L:
  case CO_L:
  case CO_DN_L_L:
  case CO_air:
    ret.y = (int)(pos.y + size.height * .5 - shotSize.height * .5);
    if (touching == CO_up) {
      ret.y = Utils::maximum(ret.y,pos.y);
    }
    else if (touching == CO_dn) {
      ret.y = Utils::minimum(ret.y + shotSize.height,pos.y + size.height) - shotSize.height;
    }
    break;
    
  default:
    assert(0);
  }

  return ret;
}



NetData::NetData() {
  lastModified = 0;
  fClocked = False;
  sent = False;

  // means is unspecified.
  tickType = TICK_MAX;
}



Touching Coord::dir_to_touching(Dir dir) {
  switch (dir) {
    case CO_r:
    case CO_r_DN:
    case CO_r_UP:
      return CO_r;
    case CO_dn:
    case CO_dn_R:
    case CO_dn_L:
    case CO_center:
    case CO_center_R:
    case CO_center_L:
      return CO_dn;
    case CO_l:
    case CO_l_DN:
    case CO_l_UP:
      return CO_l;
    case CO_up:
    case CO_up_R:
    case CO_up_L:
      return CO_up;
    default:
      return CO_air;
  }
}



Touching Coord::zero_offset_generator(Dir) {
  // See Coord::generate_offsets.
  return CO_UP_L;
}



void Coord::generate_offsets(Size offsets[CO_DIR_MAX],
                             const Size sizes[CO_DIR_MAX],const Size &sizeMax,
                             Touching (*dirToTouching)(Dir)) {                             
  Dir dir;
  for (dir = 0; dir < CO_DIR_MAX; dir++) {
    Touching touching = dirToTouching(dir);
    assert(touching == CO_r || touching == CO_dn || touching == CO_l ||
           touching == CO_up || touching == CO_air || touching == CO_UP_L);

    // CO_UP_L is a flag that means the offsets should both be zero.
    if (touching == CO_UP_L) {
      offsets[dir].width = 0;
      offsets[dir].height = 0;
    }
    // Normal touching values.
    else {
      if (touching == CO_r) {
        offsets[dir].width = sizeMax.width - sizes[dir].width;
      }
      else if (touching == CO_l) {
        offsets[dir].width = 0;
      }
      else {
        offsets[dir].width = (sizeMax.width - sizes[dir].width) >> 1;      
      }
      if (touching == CO_dn) {
        offsets[dir].height = sizeMax.height - sizes[dir].height;
      }
      else if (touching == CO_up) {
        offsets[dir].height = 0;
      }
      else {
        offsets[dir].height = (sizeMax.height - sizes[dir].height) >> 1;
      }   
    } // not CO_UP_L
  }
}



signed char NetData::gClocked = -1;  // disabled by default.



unsigned int OneTransform::compute_key(const TransformType* transforms,int tNum) {
  assert(TRANSFORMS_MAX == 2); // Or we need to change this code.
  assert(tNum <= TRANSFORMS_MAX);

  if (tNum == 0 || (tNum == 1 && transforms[0] == TR_NONE)) {
    return 0x0;
  }
  if (tNum == 1) {
    switch (transforms[0]) {
    case TR_RT_1:
      return 0x1;
    case TR_RT_2:
      return 0x2;
    case TR_RT_3:
      return 0x3;
    case TR_RF_X:
      return 0x4;
    }
  }
  if (tNum == 2) {
    // Reflection always goes before rotation in canonical order.
    assert(transforms[0] == TR_RF_X);

    switch (transforms[1]) {
    case TR_RT_1:
      return 0x5;
    case TR_RT_2:
      return 0x6;
    case TR_RT_3:
      return 0x7;
    }
  }

  // Shouldn't get here.
  assert(0);
  return 0x0;
}



Boolean Transform2D::is_base(Dir d,TransformMap* override) {
  // Check whether to use override or default.
  OneTransform* trans;
  if (override && (*override)[d].transformsNum != TR_USE_DEFAULT) {
    trans = &(*override)[d];
  }
  else {
    trans = &transforms[d];
  }

  return trans->transformsNum == 0;
}



Dir Transform2D::get_base(Dir d,TransformMap* override) {
  assert(!is_base(d,override)); 

  // Check whether to use override or default.
  OneTransform* trans;
  if (override && (*override)[d].transformsNum != TR_USE_DEFAULT) {
    trans = &(*override)[d];
  }
  else {
    trans = &transforms[d];
  }

  return trans->base;
}



const TransformType* Transform2D::get_transforms(int &tNum,Dir d,
                                                 TransformMap* override) {
  assert(!is_base(d,override)); 

  // Check whether to use override or default.
  OneTransform* trans;
  if (override && (*override)[d].transformsNum != TR_USE_DEFAULT) {
    trans = &(*override)[d];
  }
  else {
    trans = &transforms[d];
  }

  // Special check for the identity transformation.
  if (trans->transforms[0] == TR_NONE) {
    assert(trans->transformsNum == 1);
    tNum = 0;
    return NULL;
  }
  
  tNum = trans->transformsNum; 
  return trans->transforms;
}



Pos Transform2D::apply(TransformType tt,const Pos &pos,const Size &size) {
  switch (tt) {
  case TR_NONE:
    return pos;
  case TR_RT_1:
    return Pos(size.height - 1 - pos.y,pos.x);
  case TR_RT_2:
    return Pos(size.width - 1 - pos.x,size.height - 1 - pos.y);
  case TR_RT_3:
    return Pos(pos.y,size.width - 1 - pos.x);
  case TR_RF_X:
    return Pos(size.width - 1 - pos.x,pos.y);    
  default:
    assert(0);
    // Will all compilers accept this syntax?
    return Pos();
  }
}



Size Transform2D::apply(TransformType tt,const Size &size) {
  switch (tt) {
  case TR_NONE:
    return size;
  case TR_RT_1:
  case TR_RT_3: {
    Size ret;
    ret.set(size.height,size.width);
    return ret;
  }
  case TR_RT_2:
  case TR_RF_X:
    return size;
  default:
    assert(0);
    // Will all compilers accept this syntax?
    return size;
  }
}



Pos Transform2D::apply_all(Dir d,const Pos &pos,const Size &size,
                           TransformMap* override) {
  int tNum;
  const TransformType* transforms = 
    Transform2D::get_transforms(tNum,d,override);
  Pos ret = pos;
  for (int m = 0; m < tNum; m++) {
    ret = Transform2D::apply(transforms[m],ret,size);
  }
  return ret;
}

// Transform2D::transforms is defined in transform.bitmaps included in 
// physical.cpp



