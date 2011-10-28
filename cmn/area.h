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

// "area.h" 

#ifndef AREA_H
#define AREA_H

#if X11
	#ifndef NO_PRAGMAS
	#pragma interface
	#endif
#endif


// Include Files
#include "utils.h"
#include "coord.h"
#include "streams.h"


// Defines
#define AR_WSQUARES_MAX 20
enum ARshape {AR_RECT,AR_UNDEFINED};
enum ARsig {AR_NO_SIG, AR_CLOSE, AR_FAILURE, AR_BAD_SHAPE};



// Data structures
struct TouchingListItem {
  Loc list[AR_WSQUARES_MAX];
  int num;
};

struct TouchingList {
  TouchingListItem r,dn,l,up; 
};


class Area;
class Avoid;



// Class Declarations
class Area {
 public:
  Size operator - (const Area &) const;
  /* EFFECTS: Subtract areas according to their upper left corners. */
  
  Boolean operator == (const Area &) const;
  
  Area operator + (const Size &) const;
  Area operator - (const Size &) const;
  /* EFFECTS: Return new Area with pos shifted by shift. */
  

  Area() {shape = AR_UNDEFINED;}
  /* EFFECTS: Create undefined area. */

  Area(const Pos &p,const Size &s) {shape = AR_RECT,pos = p,size = s;}
  Area(ARshape sh,const Pos &p,const Size &s) {assert(sh == AR_RECT);  
      shape = AR_RECT; pos = p; size = s; }
  /* REQUIRES: c is AR_RECT. */
  /* EFFECTS: Creates a rectangle area at position p of size s. */

  Area(ARshape c,const Loc &loc,const Dim &dim);
  /* REQUIRES: c is AR_RECT. */
  /* EFFECTS: Creates a rectangle area at loc and dim. */

  Area(InStreamP);
  void read(InStreamP);
  static int get_write_length()
    {return Pos::get_write_length() + Size::get_write_length();}
  void write(OutStreamP) const;

  Boolean is_defined() const {return shape != AR_UNDEFINED;}

  void get_rect(Pos &p,Size &s) const 
    {assert (shape == AR_RECT); p = pos; s = size;}

  /* MODIFIES: p,s */

  Pos get_pos() const 
    {assert(shape == AR_RECT); return pos;}
  Size get_size() const
    {assert(shape == AR_RECT); return size;}
  
  Box get_box() const;
  /* EFFECTS: Returns the box of wsquares that contains the area. */

  void set_pos(const Pos &p) 
	{assert(shape == AR_RECT); pos = p;}
  void set_size(const Size &s)
    {assert(shape == AR_RECT); size = s;}

  void wsquares(Loc list[AR_WSQUARES_MAX],int &nitems) const;
  /* MODIFIES: list, nitems */
  /* EFFECTS: After completion, the first nitems of list contain the locations
     of all the wsquares covered by the area.  The initial values of list 
     and nitems are ignored.  wsquares are not necessarily inside the world. */

  Boolean overlap(const Area &) const;
  /* EFFECTS: Returns True if *this overlaps with r, False otherwise. */

  Boolean overlap(const Loc &loc) const;
  /* EFFECTS: Returns True if loc overlaps with any of the area.  Returns 
     False otherwise. */

  Boolean overlap(const Box &box) const;
  /* EFFECTS: Returns True if box overlaps with any of the area.  Returns 
     False otherwise. */

  Boolean overlap(const Pos &pos) const;

  Loc middle_wsquare() const;
  /* EFFECTS: Returns the location of the middle wsquare in the area. */

  ARsig avoid_wsquare(Avoid &avoid,const Loc &loc) const;
  /* MODIFIES: avoid */
  /* EFFECTS: Returns AR_NO_SIG if the wsquare at loc does not overlap with 
     the area.  If loc overlaps in the area, but not in a nearby area, returns
     AR_CLOSE and sets avoid to be the offsets necessary to make the area no 
     longer touch the wsquare. */

  int avoid_wsquare_dir(const Loc &loc,Dir dir) const;
  /* EFFECTS: Returns the amount *this will have to be shited in dir to avoid 
     loc. */

  int avoid_area_dir(const Area &area,Dir dir) const;
  /* EFFECTS: Returns the amount *this will have to be shited in dir to avoid 
      area. */

  Size avoid(const Area &area) const;
  /* EFFECTS: Returns the smallest offset necessary to move *this in order to
     avoid area. */

  Size avoid_no_up(const Area &) const;
  /* EFFECTS: Like avoid(), but will not avoid in the up direction. */

  Pos adjacent_rect(const Size &s,Dir dir) const;
  /* EFFECTS: Return the upper left corner of the rectangle of size s that
     touches *this such that the direction from *this to the rectangle is 
     dir. */

  Pos get_middle() const{  assert(shape == AR_RECT);
              return Pos(pos.x+(size.width>>1),pos.y+(size.height>>1));}
  /* EFFECTS: Returns the middle of the area. */

  Boolean touches(const Area &other,Touching touching) const;
  /* EFFECTS: Does other touch this on this's side specified by touching. */

  void touching_wsquares(TouchingList &list) const;
  /* MODIFIES: list */
  /* EFFECTS: Sets list.{r,dn,l,up} to be the lists of all wsquares that are exactly 
     touching the area.  The directions are set from the area to the touching 
     wsquare.  I.e. A loc listed under list.r would be touching the 
     right side of the area.  The list are ordered such that the smallest 
     row and column indexes come first.  I.e. top->bottom and left->right. */

  void edge_wsquares(TouchingList &list) const;
  /* MODIFIES: list */
  /* EFFECTS: Like touching_wsquares except squares on edge don't have to be 
      exactly touching.  They might just overlap area.  I.e. returns 
      Area::wsquares() - {all squares entirely inside the Area} */
      
  Boolean corner_offset(Size &offset,Loc list[3],Loc &inner,Dir corner) const;
  /* MODIFIES: offset, list, inner */
  /* EFFECTS: If the corner of the Area in the given direction covers 3 wsquares, set offset
	  to be the amount to shift the Area so it is touching on the corner, set list to be the 3 
	  covered wsquares, set inner to be the wsquare of the inside of the corner,
	  and return True.  Else return False.  corner is one of 
	  {CO_DN_R,CO_DN_L,CO_UP_L,CO_UP_R}. */

  void shift(const Size &offset)
  {assert(shape == AR_RECT); pos = pos + offset;}
  /* EFFECTS: Shift the area by offset. */

  void set_middle(const Pos &pos);
  /* EFFECTS: Move the area so that pos will be the middle. */

  Dir dir_to(const Pos &pos) const;
  Dir dir_to(const Area &area) const;
  /* EFFECTS: Returns the direction from this to pos or to area. 
     Will return one of {CO_R .. CO_UP_R, CO_air}.  I.e. 8 directions or 
     CO_air. */

  Area combine(const Area &a) const;
  /* EFFECTS: Returns minimum sized rectangular area that contains a and 
     this. */

  Area wsquare_allign(const Size &offset) const;
  /* EFFECTS: For each dimension of offset that is non-zero, wsquare_allign
	 makes sure that the returned Area is wsquare_alligned in that dimension.
	 I.e. the edge of the Area in that direction will be on a wsquare boundary.
	 The returned area will not cover any wsquares that weren't already 
	 covered. */
  
  Boolean wsquare_alligned() const;
  /* EFFECTS: Is the Area alligned in all four directions with wsquares? */
  
  Area grow_by(int delta) const;
  /* EFFECTS: Increase area by delta on all four sides.  I.e. size will increase by
     2*delta in both directions. */
  
  Size clip_top(int val);
  /* REQUIRES: Don't clip everything away. */
  /* EFFECTS: Clip top to be <= val.  Return offset (width,height) such that height is the
     amount that was clipped away. */
  
  Size clip_left(int val);
  /* REQUIRES: Don't clip everything away. */
  /* EFFECTS: Clip left to be <= val.  Return offset (width,height) such that width is the
     amount that was clipped away. */

  Size clip(const Size &size);
  /* REQUIRES: Don't clip everything away. */
  /* EFFECTS: Clip this to clipper.  Return False if nothing left. 
     In the second version, clipper is an area of size size with pos (0,0). */


 private:
  ARshape shape;
  Pos pos;
  Size size;
};



class Avoid {
friend class Area;

public:    
  void maximize(const Avoid &avoid);
  /* EFFECTS: Set all directions of *this to be to max of *this and avoid. */

  Size offset_rank(int rank = 0) const;
  /* REQUIRES: 0 <= rank < 4 */
  /* EFFECTS: Return the offset corresponding to the rankth minimum direction
     of *this using one dimensions.  E.g. rank = 0 gives the minimum.  rank
     = 1 gives the second minimum, etc. */
  
  Size offset_dir(Dir d) const;
  /* REQUIRES: d in {CO_R, CO_DN, CO_L, CO_UP} */
  /* EFFECTS: Return the offset corresponding to d. */

  int get_dir(Dir d) const;
  /* REQUIRES: d in {CO_R, CO_DN, CO_L, CO_UP} */
  /* EFFECTS: Returns the offset in the specified direction. */


private:
  /* Direction from area to wsquare.  All are positive.  r and dn are 
     therefore the negative of the corresponding offsets. */
  int r,dn,l,up; 
  // Note: A bit counter-intuitive.
};



// All values are world (unstretched) coordinates.
class ViewportRegion {
  friend class ViewportInfo;

public:
  ViewportRegion() {};
  /* NOTE: Make the fucking compiler happy. */

  Boolean visibleOverlap(const Area &) const;
  /* EFFECTS: Does the area overlap the visible region of the  viewport? */

  Boolean peripheralOverlap(const Area &) const;
  /* EFFECTS: Does the area overlap the peripheral region of the viewport, 
     including the visible subset of that. */     

  void grow_by(int val);
  /* EFFECTS: Grow the ViewportRegion by val, visible and peripheral. */


private:
  ViewportRegion(const Area &a); 
  /* EFFECTS: V_AREA constructor. */

  ViewportRegion(const Pos &periphCenter,int periphRadius,
                 const Area &visibleArea);
  /* EFFECTS: V_DISK constructor. */


  enum {V_AREA,V_DISK};
  char type;
  Area area;   // both, for V_DISK this gives the inner,visible area
  Pos pos;     // for V_DISK
  int radius;  // for V_DISK
};
typedef ViewportRegion *ViewportRegionP;



// A Factory for generating ViewportRegions, given a Physical to follow.
//
// All values are world (unstretched) coordinates.
class ViewportInfo {
public:
  ViewportInfo() {}
  /* EFFECTS: So we can create an uninitialized object to call
     ViewportInfo::read(). */

  ViewportInfo(int radius,const Size &visibleSize);
  /* EFFECTS: Create smooth scroll ViewportInfo. */
  
  ViewportInfo(int radius,const Size &logicalSize,const Size &extraSize);
  /* EFFECTS: Create non-smooth scroll ViewportInfo. */

  void read(InStreamP in);
  static int get_write_length();
  void write(OutStreamP out) const;

  ViewportRegion get_viewport_for(const Pos &pos) const;
  /* EFFECTS: The factory's creation method.  pos is the center of the
     Physical to follow. */

  Dim get_viewport_dim() const;
  /* REQUIRES: Viewport dimensions must be multiples of wsquares. */
  /* EFFECTS: Get the dimensions of a viewport in wsquares. */


private:
  Boolean smoothScroll; // both
  Size logicalSize;     // both
  Size extraSize;       // if !smoothScroll
  int radius;           // both
};
typedef ViewportInfo *ViewportInfoP;



// An interface to lazily provide a ViewportInfo object.
class IViewportInfo {
public:
  virtual ViewportInfo get_info() = 0;
};

#endif



