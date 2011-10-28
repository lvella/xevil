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

// "world.h"

/* Overview:  A world object is a rectangular region composed of WSQUARES.
   Internally, it is divided into rooms of constant size.  It's upper left 
   corner is at WSQUARE (0,0). 
   World should be draw completely once before just drawing with changes. */

#ifndef WORLD_H
#define WORLD_H

#if X11
	#ifndef NO_PRAGMAS
	#pragma interface
	#endif
#endif

// Include Files

#include "xdata.h"
#include "utils.h"
#include "coord.h"
#include "area.h"
#include "id.h"
#include "streams.h"


// Defines.

// WSQUARES in a room, should be an even number, because we require world read
// in from a file to be half-room-size alligned.
#define W_ROOM_COL_MAX 26 // 36
#define W_ROOM_ROW_MAX 16

// The W_ALL_* defines are in coord.h

#define W_BACKGROUNDRATE 2


#define W_MOVER_WIDTH_MAX 32
#define W_MOVER_HEIGHT_MAX 16

/* Maximum rooms in world.  World may actually be smaller.  World starts out 
   at the size of the title screen. */
#define W_ACROSS_MAX_MAX 8
#define W_DOWN_MAX_MAX 5 // Should be odd, for ZIG-ZAG scenario.

// WSQUARES in world.
#define W_COL_MAX_MAX (W_ROOM_COL_MAX * W_ACROSS_MAX_MAX)
#define W_ROW_MAX_MAX (W_ROOM_ROW_MAX * W_DOWN_MAX_MAX)

/* The maximum number of wsquares that can be set changed/unchanged in one 
  clock cycle. */
#define W_CHANGES_MAX 5000

// Only for the "Empty" special map.
#define W_EMPTY_DEAD_ROWS 5

/* The blocks composing the world.  Represented as integers so that the world
   map can be initialized easily.  Don't change order, 
   e.g. World::blockUseTransparent. */
enum { 
  Wempty,
  Wwall,
  Wladder,
  Woutside,
  Wsquanch,
  WupDown,
  WtextSquare,   // Kill me, not used anymoure.
};



// SpecialMap is the way a GameStyle/Scenario tells the world to use 
// something other than the default map.
typedef int RoomMaze;
class SpecialMap;
typedef SpecialMap* SpecialMapP;
class SpecialMap {
public:
  // Possible values for room_maze().
  enum {
    NORMAL,      // Normal maze algorithm.
    HORIZ_ONLY,  // Maze algorithm, then remove all vertical.
    ZIG_ZAG,     // Zig-Zag maze starting at lower left.
    EMPTY,       // No maze whatsoever. It is up to the Scenario to somehow
                 // guarantee that characters can get from one place to 
                 // another.  Can still have horiz/vert "extra" walls, 
                 // if desired.
  };

  virtual ~SpecialMap();

  virtual SpecialMapP clone() = 0;  
  /* EFFECTS: Return a newly allocated copy of this class of SpecialMap. */

  virtual Rooms get_rooms() = 0;
  
  virtual RoomMaze room_maze();
  /* DEFAULT: SpecialMap::NORMAL */

  virtual Boolean horiz_extra_walls();
  /* DEFAULT: True */

  virtual Boolean vert_extra_walls();
  /* NOTE: Also controls whether horizontal extra walls can go up and 
     down a bit. */
  /* DEFAULT: True */
  
  virtual Boolean use_movers();
  /* EFFECTS: Both vertical movers (in place of ladders) and randomly placed
     horizontal movers. */
  /* NOTE: Probably have to change this later to allow choosing horizontal 
     and vertical movers independently. */
  /* DEFAULT: True */

  virtual Boolean big_physicals();
  /* EFFECTS: Return True if world should guarantee that there is enough
     space between walls for big physical objects, e.g. FireDemon. */
  /* DEFAULT: False */
  
  virtual Boolean do_doors();
  /* DEFAULT: True */
};



//W_BLOCKS_NUM     now in coord.h

#define W_TOTAL_PIX_NUM (W_ALL_BLOCKS_NUM + W_ALL_DOORS_NUM + \
  W_ALL_MOVER_SQUARES_NUM + 2 * W_ALL_MOVERS_NUM)
#define W_TOTAL_PIX_NUMDIV2 int(W_TOTAL_PIX_NUM/2.0f +0.5f)
// blocks + doors + moverSquares + mover


enum Wsig {W_NO_SIG, W_CLOSE, W_CLOSE_BAD, W_FAILURE};


enum {W_DOOR_TOP, W_DOOR_BOTTOM};
enum {UN_POSTER,UN_DOOR,UN_MOVER};



// W_POSTERS_NUM now in coord.h for X11.  Does not exist for WIN32.



class Blueprints;
class Locator;
typedef Locator *LocatorP;
class World;
typedef World *WorldP;



///////////////////////////////////////////////////////////////////////////////
// Class Declarations
///////////////////////////////////////////////////////////////////////////////
struct BitmapSpec {
  CMN_BITS_ID id;
  // x2 coordinates, should change it to be unstretched some time.
  Size size;
};



struct Theme {
  int blockIndices[W_BLOCKS_NUM];

  int *backgroundIndices;
  int backgroundsNum;

  int *outsideIndices;
  int outsidesNum;

  int doorBase;
  int moverSquareBase;
  int moverIndex;

  int *posterIndices;
  int postersNum;
};



class Mover {
public:
  Mover() {areaSet = False;}
  /* NOTE: Not really created yet, still need to call init() to finish
     construction. */

  Mover(InStreamP,WorldP,LocatorP);
  void update_from_stream(const Area &); // called by PhysMover
  static int get_write_length();
  void write(OutStreamP);

  void init(WorldP w,LocatorP l,const Area &a,const Size &v,
            MoverId id);
  Boolean is_area_set() {return areaSet;}
  void init_area(const Area &a);
  void init_not_area(WorldP w,LocatorP l,const Size &v,MoverId id);
  /* NOTE: Two ways to do it.  Just call init() or call both init_area() and
     init_not_area(). */
  void set_phys_mover_id(Id id) {physMoverId = id;}
  /* NOTE: Should really be a constructor, must be called first. */
  
  const Area &get_area() {return area;}
  
  const Size &get_vel() {return vel;}
  /* NOTE: Delta to get from prev position to current one. */
  
  const MoverId &get_mover_id() {return moverId;}

  void clock();
  /* EFFECTS: Move to next position. */
  
  
private:
  Boolean check_area(const Area &area);
  /* EFFECTS: True iff all squares covering area are moverSquares for 
     this Mover. */
  
  WorldP world;
  LocatorP locator;
  Boolean areaSet;
  Area area;
  Size vel; // A Size because it is integer.
  Size velStored; // To remember vel when pausing at top or bottom.
  MoverId moverId; // index in list.
  Id physMoverId; // Id of object simulating a Physical for the Locator.
  Timer timer;
  Boolean timerSet;
};
typedef Mover *MoverP;



struct PosterSquare {
  int type;
  int poster;
  Loc loc; // Inside poster.
};



struct DoorSquare {
  int type;
  int topBottom; 
  Loc dest;
};



enum {OR_HORIZ,OR_VERT}; // Orientation
struct MoverSquare {
  int type;
  int orientation;
  Mover *mover;
};



union UnionSquare {
  int type; // UN_POSTER, UN_DOOR, UN_MOVER.
  PosterSquare pSquare;
  DoorSquare dSquare;
  MoverSquare mSquare;

  static UnionSquare *read(InStreamP,WorldP);
  static int get_write_length();
  void write(OutStreamP);
};



class World {
  friend class Blueprints;
  friend class Mover;
  friend class PhysMover;


 public:
  World();
  /* EFFECTS: Create the title screen world.  Fuck politically correct. */

  ~World();
  /* EFFECTS: Destroys the world, heh, heh.  Keep away from children. */

  void title_map(IViewportInfo*);
  /* EFFECTS: Draw the title screen on the map.  Need the IViewportInfo to 
   make a title screen appropriate for the given viewport. */

  void set_locator(Locator *l) {assert(!locator); locator = l;}
  /* NOTE: Must be set before first level_reset(). */

  Size get_size() {return size;}
  /* EFFECTS: Size of the world in pixels. */

  Dim get_dim() {return dim;}
  /* EFFECTS: Dimensions of the world in wsquares. */

  Dim get_room_dim();
  /* EFFECTS: Return the dimensions of a room in wsquares. */

  Size get_room_size();
  /* EFFECTS: The size of a single room in pixels. */

  Rooms get_rooms() {return rooms;}
  /* EFFECTS: The max number of rooms in across and down direction. */
  /* NOTE: The world is no longer necessarily room-alligned, so this may
     round up. */

  Boolean check_door(const Loc &loc1,Loc &dest);
  /* EFFECTS:  Return whether there is a door at loc1.  If there is, return
     the location of the doors destination in dest. */

  void set_rooms_next(const Rooms &r);
  /* EFFECTS: After the next reset, the world will have as many rooms as r.
   If r is bigger than the maximum, the min of the maximum size and r is 
   used. */

  void set_special_map_next(SpecialMapP map);
  /* EFFECTS: Sets world to have a special map on the next reset.  Implicitly
     sets the dimensions of the world. Call with NULL, to go back to 
     normal map. */
  /* NOTE: World takes ownership of SpecialMap and will delete it when it's 
     done. Do not refer to the SpecialMap after passing it in. */

  void set_file_next(char *filename) 
    {Utils::freeif(worldFile); worldFile = Utils::strdup(filename);}
  /* EFFECTS: Try to read World from given file. */

  void set_map_print(Boolean val) {mapPrint = val;}
  /* EFFECTS: Sets whether to print each new map of the world at reset. */

  Boolean overlap(const Box &box);
  /* EFFECTS: Does any of box overlap with the wsquares of the world. */

  void draw(CMN_DRAWABLE buffer,Xvars &xv,int dpyNum,const Area &area,
            Boolean reduceDraw,Boolean background3D);
  /* EFFECTS: Draw all of area that is in the world.  If background3D is
   true, draw background scrolling at a slower rate than the foreground 
   to give a 3d effect. */

  Boolean inside(const Loc &l)
  {return l.r >= 0 && l.c >= 0 && l.r < dim.rowMax && l.c < dim.colMax;}
  /* EFFECTS: Returns True if loc is a wsquare inside the world, otherwise
   returns False. */

  Boolean inside(int r,int c)
  {Loc loc; loc.set(r,c); return inside(loc);}

  Boolean open(const Area &area,
               Boolean laddersClosed = False,
	             Boolean postersClosed = False,
               Boolean doorsClosed = False,
               Boolean outsideClosed = True);
  /* NOTE: Treats movers as closed. */

  Wsig open_offset(Size &offset,const Area &area,const Vel &vel);
  /* MODIFIES: offset  */
  /* EFFECTS: Returns W_NO_SIG if the portion of the world covered by initial 
     is open.  prev is the previous area occupied before.  If the area is 
     blocked, but a nearby area is not, returns
     W_CLOSE_OK and sets offset to be the offset necessary to unblock initial.
     Retruns W_CLOSE_BLOCKED if a nearby area is blocked.  Stills sets offset
     for the nearby area.  Returns W_FAILURE if initial and all nearby areas 
     are blocked.  offset may be modified in any case. */

  void compute_touching_hanging(Touching &touching, Hanging &hanging, 
          MoverP &touchingMover,const Area &area);
  /* REQUIRES: area is AR_RECT. */
  /* MODIFIES: touching, hanging, touchingMover */
  /* EFFECTS: If area is touching a blocked wsquare, sets touching to be the 
     direction from area to the wsquare.  If the area is touching in both the
     vertical and horizontal direction, the vertical takes precedence.  Sets
     touching to be CO_air if not touching in any direction.  If the area is
     hanging off of the edge of some blocked wsquares, sets hanging 
     appropriately.  Otherwise, sets hanging.corner to be CO_air. 
     If touching a mover, touchingMover is set, else set to NULL. */
  
  Boolean can_climb(const Loc &loc)
    {return inside(loc) ? (map[loc.r][loc.c] == Wladder) : False;}
  /* EFFECTS: Returns True if loc is a climable square inside the world.  I.e.
     a ladder.  False otherwise. */

  Pos empty_rect(const Size &size);
  /* EFFECTS: Return the upper-left position of a randomly choosen empty 
     rectangle of size s.  Ladders and doors are not considered empty.  
     Posters are considered empty. */

  Pos empty_touching_rect(const Size &size);
  /* EFFECTS: Like empty_rect except the returned rect will be touching the 
     ground. */

  Pos empty_accessible_rect(const Size &size);
  /* EFFECTS: Like empty_rect except that it guarantees that the rectangle can
     be reached by all Creatures.  Also, the returned rectangle will be 
     touching CO_dn. */
  /* REQUIRES: reset() has been called. */

  Pos empty_accessible_rect(const Size &size,const RoomIndex &r);
  /* EFFECTS: Like empty_accessible_rect(const Size &), except restricts 
     search to the given room. */

  Boolean empty_box(Loc &loc,const Dim &dim,Boolean laddersClosed,
		    Boolean postersClosed,Boolean doorsClosed);
  /* MODIFIES: loc */
  /* EFFECTS: Returns True iff an empty box of dimension dim can be found and
     returns its upper-left Loc. */

  Boolean empty_touching_box(Loc &loc,const Dim &dim,Boolean laddersClosed,
			     Boolean postersClosed,Boolean doorsClosed);
  /* MODIFIES: loc */
  /* EFFECTS: Like empty_box, except the returned box will be touching the
     ground. */

  void reset(const Dim *clearDim = NULL,InStreamP inStream = NULL);
  /* EFFECTS:  Prepare the world for a new game.  Create a new map. 
     If InStream is specified, use it to read in a map, overriding any other
     settings.  If clearDim is non-NULL, create an empty world of given 
     dimensions. */

  void demo_reset();
  /* EFFECTS: Prepare the world for a new demo. */

  Boolean just_reset() {return justReset;}
  /* EFFECTS: TRUE if no clock() calls since last reset(). */

  int get_write_length();
  /* EFFECTS: How much data will be written in write_to_stream(). */

  void write(OutStreamP outStream);
  /* EFFECTS: Output all relevant info about the world to outStream. */
  // ???? What about movers, io through Physicals or through World?

  int get_write_length(const RoomIndex &);
  /* EFFECTS: Amount of data to write out one room. */
  
  void write(OutStreamP out,const RoomIndex &);
  /* EFFECTS: Write one room. */

  void read(InStreamP in,const RoomIndex &idx);
  /* EFFECTS: Read in one room of the world. */

  MoverP lookup(const MoverId &moverId);
  /* EFFECTS: Return the Mover or NULL. */

  void init_x(Xvars &,IXCommand,void*);
  /* EFFECTS: Initialize all x dependencies. */

  void clock();
  
  static void disable_movers() {useMovers = False;}

#if WIN32
  Boolean generate_xpm(Xvars& xvars,const char* genDir,Boolean halfSize);
  /* EFFECTS: Output all bitmaps as XPM files to the directory 
     (genDir + "world/").  Return if success. */
#endif


private:
  // Do we use these two versions of World::open() anywhere?
  Boolean open(const Loc &loc,
               Boolean laddersClosed = False,
	             Boolean postersClosed = False,
	             Boolean doorsClosed = False,
               Boolean outsideClosed = True);
  /* EFFECTS: Returns True if loc is an open square inside the world.  Returns
     False otherwise.  Note that wsquares outside the world are treated as if 
     they were closed wsquares. */
  /* NOTE: Doesn't take movers into account. */

  Boolean open(const Box &box,
               Boolean laddersClosed = False,
	             Boolean postersClosed = False,
               Boolean doorsClosed = False);
  /* EFFECTS: Is the portion of the world covered by area (or box) blocked? 
     laddersClosed implies that ladders and other non-walls are considered to 
     be blocking. */
  /* NOTE: Doesn't take movers into account. */


  void clear_everything();

  void close_horiz_mover(const Loc &loc,Boolean &inHorizMover);
  void close_vert_movers(const Dim &dim);

  Boolean read_from_file(char *filename);

  void read_from_stream(InStreamP inStream);

  void choose_theme();
  /* EFFECTS: Set themeIndex, backgroundIndex, and outsideIndex. */

  Boolean empty_accessible_rect_one(Pos &pos,const Size &,const RoomIndex &);
  /* MODIFIES: pos */
  /* NOTE: Helper for both versions of empty_accessible_rect(). */

  void draw_outside(CMN_DRAWABLE buffer,Xvars &xvars,int dpyNum,
                    Area area);
  /* EFFECTS: Draw the outside portions of the world that overlap area. */

  void draw_background(CMN_DRAWABLE buffer,Xvars &xvars,int dpyNum,
                       Area area,Boolean background3D);
  /* EFFECTS: Tiles all of buffer with tDraw the background to the buffer. */

  void draw_square(CMN_DRAWABLE buffer,Xvars &xvars,int dpyNum,const Loc &loc,
	             	   int x,int y,Boolean reduceDraw);
  /* EFFECTS: Draw the appropriate square from loc to (x,y) on buffer. */

  void draw_mover(CMN_DRAWABLE buffer,Xvars &xvars,int dpyNum,
                  MoverP mover, int x,int y);
  /* EFFECTS: Draw mover to (x,y) on buffer. */

  void draw_outside_offset(CMN_DRAWABLE dest,Xvars &xvars,int dpyNum,
                           Size sourceOffset,const Area &destArea);
  /* REQUIRES: sourceOffset + destArea.get_size() should fit
     within the source bitmap.  destArea should be completely within the 
     destination bitmap. */
  /* EFFECTS: Draw outside bitmap number outsideIndex onto dest pixmap at 
     destArea.  Grab from sourceOffset on source bitmap.  All coordinates 
     are stretched. */

  void rooms_dim_size_update(const Dim *newDim = 0);
  /* EFFECTS: If newDim is NULL, set this.dim and this.size from the current
     value of this.rooms.  Else, set this.dim, this.rooms, and this.size from 
     newDim. */
  /* NOTE: Use 0 instead of NULL because some compilers don't have NULL defined
     yet. */

  void th_helper(Touching &touching,Hanging &hanging,MoverP &touchingMover,
		 const TouchingListItem &touchingItem,
		 const TouchingListItem &edgeItem,
		 Boolean r_c,
     const Area &area,
		 int coord,int length,
		 int wsquare_length,
		 Touching iftouching,
		 Corner ifsmall, Corner iflarge);
  /* MODIFIES: touching, hanging, touchingMover */
  /* EFFECTS: Helper for World::touching.  Does touching, hanging for one 
     side. */

  Boolean check_wrap_around(Size &offset,const Area &area);
  /* MODIFIES: offset */
  /* EFFECTS: Check whether area should wrap-around to the other side 
     of the world.  If so, set offset and return True. */

  Boolean open_iter(Area &area,int &changed,Dir dir);
  /* MODIFIES: area, changed */
  /* EFFECTS: Move area as much in dir as necessary to avoid all its wsquares.
     Increment changed by the distance moved.  Return True if the original 
     area is open and nothing is changed.*/

  Size open_size(int offset,Dir dir);
  Size open_size(int offset1,Dir dir1,int offset2,Dir dir2);
  /* REQUIRES: dir is in {CO_R, CO_DN, CO_L, CO_UP} */
  /* EFFECTS: Return the Size corresponding to an offset in one or two 
     directions. */

  Boolean open_try_dirs(Size &offset,const Area &area,const Dir dirs[4],
			       int dirsNum);
  /* MODIFIES: offset */
  /* EFFECTS: Sets offset to be the minimum necessary to shift area to an
     open area in one of the directions of dirs.  Returns whether it 
     succeeded.  offset may be changed in any case. */
  
  Boolean open_try_corners(Size &offset,const Area &area);
  /* MODIFIES: offset */
  /* EFFECTS: Check if area is wedged in a corner (overlapping 3 closed 
     wsquares). If so, set offset to push the Area inside the corner and 
     return True. */

  Boolean open_try_diagonals(Size &offset,const Area &area);
  /* MODIFIES: offset */
  /* EFFECTS: Set offset to shift area to a nearby open one.  Max of all two 
     direction combinations for each diagonal direction.  Returns whether
     it succeeded.  offset may be changed in any case. */
  
  
  /* NOTES: Helpers for World::reset. */
  void add_posters();
  void add_doors();
  void add_movers();

  Boolean add_horiz_mover();
  Boolean add_vert_mover();
  /* NOTE: Helpers for add_movers().  Try to add one mover, return whether 
     successful. */

  void delete_movers();
  /* EFFECTS: Clear out all movers. */
  
  void mover_list_add(MoverP list[],int &nItems,const Loc &loc);
  /* MODIFIES: list, nItems */
  /* EFFECTS: If there is a mover square overlapping loc, add the mover to 
     list, removing duplicates. loc may be outside world. */

  
  enum {MOVERS_MAX = 20};

  Rooms rooms,roomsNext;
  SpecialMapP specialMap, specialMapNext;
  char *worldFile;
  Size size;  /* For convenience only. */
  Dim dim;    /* For convenience only. */

  Locator *locator;
  char map[W_ROW_MAX_MAX][W_COL_MAX_MAX];
  XvarsValid xValid;
  Wxdata xdata;
  Boolean mapPrint;
  UnionSquare *unionSquares[W_ROW_MAX_MAX][W_COL_MAX_MAX];
  Blueprints *blueprints;
  MoverP movers[MOVERS_MAX];
  int moversNum;
  int uniqueGen; // For generating MoverIds.
  Boolean justReset;
  // World will leave enough space for objects of this max size.  
  Dim objectDimMax; 

  Dim posterDims[W_ALL_POSTERS_NUM]; 

  // Used on X, but just to keep things in sync as a server.
  int themeIndex;
  int backgroundIndex; // 0 <= backgroundIndex < W_ALL_BACKGROUNDS_NUM
  int outsideIndex; // 0 <= outsideIndex < W_ALL_OUTSIDES_NUM

  static Boolean useMovers;

  // Defined in world.bitmaps.
  static CMN_BITS_ID *blocksBits; // [W_ALL_BLOCKS_NUM];
  static BitmapSpec *backgrounds; // [W_ALL_BACKGROUNDS_NUM];
  static BitmapSpec *outsides; // [W_ALL_OUTSIDES_NUM];
  static CMN_BITS_ID *doorPixmapBits; //[W_ALL_DOORS_NUM];
  static CMN_BITS_ID *moverSquarePixmapBits; //[W_ALL_MOVER_SQUARES_NUM];
  static CMN_BITS_ID *moverPixmapBits; //[W_ALL_MOVERS_NUM];
  static BitmapSpec *posters; //[W_ALL_POSTERS_NUM];


  // Needed by X11 to act as a server.
  Theme *themes;
  // Just a pointer to World_themes in world.bitmaps.
  // Don't use "static Theme themes[W_THEME_NUM]", because then we'd
  // need to recompile everytime W_THEME_NUM changed.
  // Really should separate the World interface from implementation.

  // Unstretched coordinates.
  static const Size moverSize;

  // Which blocks to draw using transparent background.
  static Boolean blockUseTransparent[W_BLOCKS_NUM];
};

			   
#endif

