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

// "world.cpp"

#if X11
#ifndef NO_PRAGMAS
#pragma implementation "world.cpp"
#endif
#endif

// Include Files.
#include "stdafx.h"

#if WIN32
#include "resource.h"
#endif

// Include Files.
#include <iostream>

#include "utils.h"
#include "coord.h"
#include "xdata.h"
#include "area.h"

#include "world.h"
#include "bitmaps/world/world.bitmaps"

// For PhysMover.
#include "actual.h"

using namespace std;

// Defines.
#define HANGING_PERCENT 0.40 // The % of the edge hanging off the corner.
#define STRAND_PERCENT .50 // Gives number of boundaries between rooms.
#define STRAND_LENGTH 8 // Length of strands in Blueprints::fill_random.
#define EMPTY_TRIES_MAX 5000
#define WALL_LENGTH 30
#define WALLS_PERCENT .01
#define WALLS_HORIZ_CHANCE 4 // 2 gives equal chance
#define LADDER_CHANCE 10
#define MAP_PRINT_DEFAULT False
#define UP_DOWN_CHANCE 4
#define OPEN_ITERATIONS 2


#define POSTERS_ACTUAL_PERCENT .0003  // Percentage of dim.
#define DOORS_ACTUAL_PERCENT .1  // Pairs of doors as percentage of rooms.
// Percentage of rooms that can possibly have movers.

// both are multipliers on number of rooms.
#define MOVERS_PERCENT .4
// Tries to find a space for the mover.
#define MOVER_TRIES 6

// Min number of movers across a horizontal track must be.
#define MOVERS_HORIZ_MIN_TRACK 6 // moverwidths
#define MOVERS_HORIZ_TRACK_LENGTH 25 // wsquares

// Used to determine how much space to leave between walls.
// Only the default, actual variable is objectMinDim.
#define OBJECT_COL_MAX 2
#define OBJECT_ROW_MAX 2
#define OBJECT_BIG_COL_MAX 2
#define OBJECT_BIG_ROW_MAX 3



// Blueprints class.  A helper for World.
class Blueprints {
 public:
  Blueprints(WorldP world,char map[W_ROW_MAX_MAX][W_COL_MAX_MAX],
	           const Dim &objectDimMax,SpecialMapP specialMap);
  /* EFFECTS: Create a blueprints object with room boundaries defined. */

  int get_middle_row(int down);
  int get_middle_col(int across);
  /* NOTES: Offset from edge of room, in WSQUARES.  Floor at middleRow.
     Ladder at [middleCol ... middleCol + objectDimMax.colMax). */


  // NOTE: The following four methods replace Blueprints::fill_map.
  void implement_rooms();
  /* MODIFIES: this->map */
  /* EFFECTS: Using the defined room boundaries, create a map of the world. 
     Create walls between rooms, and guarantee access between adjacent rooms
     using walls and ladders. */

  void implement_edges_only();
  /* EFFECTS: Like implement_rooms(), but only create walls for the
     outer edges of the world. */  

  void extra_walls();
  /* MODIFIES: map */

  void up_downs();
  /* MODIFIES: map */
  /* EFFECTS: Change all walls on edge of ladders to up_downs. */


  void print();
  /* EFFECTS: Print the room boundaries to cout. */

  Boolean blocked_right(const RoomIndex &ri);
  Boolean blocked_bottom(const RoomIndex &ri);
  Boolean blocked_left(const RoomIndex &ri);
  Boolean blocked_top(const RoomIndex &ri);
  /* EFFECTS: Returns whether ri has a blocking wall on the edge of the
     room. */


 private:
  void fill_random();
  /* EFFECTS: Snake algorithm to define boundries between rooms. */

  void special_map_adjust();
  /* EFFECTS: Adjust map for any requirements imposed by the
     special map specification. */

  Boolean open(const RoomIndex &ri);
  void has_right(const RoomIndex &ri,const Loc &rl);
  void missing_right(const RoomIndex &ri,const Loc &rl);
  void has_bottom(const RoomIndex &ri,const Loc &rl);
  void missing_bottom(const RoomIndex &ri,const Loc &rl);
  void has_top(const RoomIndex &ri,const Loc &rl);
  void missing_top(const RoomIndex &ri,const Loc &rl);
  void has_left(const RoomIndex &ri,const Loc &rl);
  void missing_left(const RoomIndex &ri,const Loc &rl);

  void ladder(const Loc &loc);
  /* EFFECTS: Draw a ladder going up and down from loc until a wall is hit. */

  Boolean wall_below(int rTop,int cTop,int num);
  /* EFFECTS: Return whether there is a Wwall or WupDown in the vertical
     column of num blocks whose top is (rTop,cTop). */

  WorldP world;
  Rooms worldRooms;
  Dim worldDim;
  char (* map)[W_COL_MAX_MAX];
  int middleRows[W_DOWN_MAX_MAX];
  int middleCols[W_ACROSS_MAX_MAX];
  Boolean horiz[W_DOWN_MAX_MAX + 1][W_ACROSS_MAX_MAX];
  Boolean vert[W_DOWN_MAX_MAX][W_ACROSS_MAX_MAX + 1];
  SpecialMapP specialMap; // Owned by world.
  Dim objectDimMax;
};



Blueprints::Blueprints(WorldP w,char m[W_ROW_MAX_MAX][W_COL_MAX_MAX],
		                   const Dim &o_dim_max,SpecialMapP special) {
  objectDimMax = o_dim_max;
  world = w;
  worldRooms = world->get_rooms();
  worldDim = world->get_dim();
  map = m;
  specialMap = special;

  // The Blueprints object assumes that the world is exactly an
  // integer multiple of rooms.
  assert(worldDim.colMax % W_ROOM_COL_MAX == 0 &&
         worldDim.rowMax % W_ROOM_ROW_MAX == 0);

  // Boundaries only at the edges of the world.
  RoomIndex ri;
  for (ri.across = 0; ri.across < worldRooms.acrossMax + 1; ri.across++) {
    for (ri.down = 0; ri.down < worldRooms.downMax + 1; ri.down++) {
      if (ri.across < worldRooms.acrossMax) {
	      if ((ri.down == 0) || (ri.down == worldRooms.downMax)) {
	        horiz[ri.down][ri.across] = True;
        }
	      else {
	        horiz[ri.down][ri.across] = False;
        }
    	}
      if (ri.down < worldRooms.downMax) {
	      if ((ri.across == 0) || (ri.across == worldRooms.acrossMax)) {
	        vert[ri.down][ri.across] = True;
        }
	      else {
	        vert[ri.down][ri.across] = False;
        }
    	}
    }
  }

  // The maze algorithm.
  if (specialMap == NULL || 
      specialMap->room_maze() == SpecialMap::NORMAL ||
      specialMap->room_maze() == SpecialMap::HORIZ_ONLY) {
    fill_random();
  }

  // Any special adjustment for special world maps.
  special_map_adjust();


  assert(W_ROOM_ROW_MAX - 2 * (objectDimMax.rowMax + 1) > 0);
  assert(W_ROOM_COL_MAX - 2 * (objectDimMax.colMax + 1) > 0);

  // Compute middles
  for (int riR = 0; riR < worldRooms.downMax; riR++) {
    middleRows[riR] = objectDimMax.rowMax + 1 +
      Utils::choose(W_ROOM_ROW_MAX - 2 * (objectDimMax.rowMax + 1));
  }

  for (int riC = 0; riC < worldRooms.acrossMax; riC++) {
    middleCols[riC] = objectDimMax.colMax + 1 +
      Utils::choose(W_ROOM_COL_MAX - 2 * (objectDimMax.colMax + 1));
  }
}



int Blueprints::get_middle_row(int down) {
  assert (0 <= down && down < worldRooms.downMax);
  return middleRows[down];
}



int Blueprints::get_middle_col(int across) {
  assert (0 <= across && across < worldRooms.acrossMax);
  return middleCols[across];
}



void Blueprints::implement_rooms() {
  RoomIndex ri;
  Loc rl;

  // Walls between rooms.
  for (ri.across = 0, rl.c = 0;
       ri.across < worldRooms.acrossMax;
       ri.across++, rl.c += W_ROOM_COL_MAX) {
    for (ri.down = 0, rl.r = 0;
	       ri.down < worldRooms.downMax;
	       ri.down++, rl.r += W_ROOM_ROW_MAX) {
      if (! horiz[ri.down][ri.across]) {
	      missing_top(ri,rl);
      }
      else {
	      has_top(ri,rl);
      }
      if (! horiz[ri.down + 1][ri.across]) {
	      missing_bottom(ri,rl);
      }
      else {
	      has_bottom(ri,rl);
      }
      if (! vert[ri.down][ri.across]) {
	      missing_left(ri,rl);
      }
      else {
	      has_left(ri,rl);
      }
      if (! vert[ri.down][ri.across + 1]) {
	      missing_right(ri,rl);
      }
      else {
	      has_right(ri,rl);
      }
    }
  }
}



void Blueprints::implement_edges_only() {
  RoomIndex r1,r2;
  Loc l1,l2;

  // Ceiling, floor
  l1.r = 0;
  r1.down = 0;
  l2.r = worldDim.rowMax - W_ROOM_ROW_MAX;
  r2.down = worldRooms.downMax - 1;
  for (r1.across = 0, r2.across = 0, l1.c = 0, l2.c = 0;
       r1.across < worldRooms.acrossMax;
       r1.across++, r2.across++, 
       l1.c += W_ROOM_COL_MAX, l2.c += W_ROOM_COL_MAX) {
    has_top(r1,l1);
    has_bottom(r2,l2);
  }

  // Left, right walls.
  l1.c = 0;
  r1.across = 0;
  l2.c = worldDim.colMax - W_ROOM_COL_MAX;
  r2.across = worldRooms.acrossMax - 1;
  for (r1.down = 0, r2.down = 0, l1.r = 0, l2.r = 0;
	     r1.down < worldRooms.downMax;
	     r1.down++, r2.down++, 
       l1.r += W_ROOM_ROW_MAX, l2.r += W_ROOM_ROW_MAX) {
    has_left(r1,l1);
    has_right(r2,l2);    
  }
}



void Blueprints::extra_walls() {
  for (int walls = 0;
       walls < worldDim.rowMax * worldDim.colMax * WALLS_PERCENT;
       walls ++) {
    Boolean ok = True;
    Loc loc;
    loc.r = Utils::choose(worldDim.rowMax);
    loc.c = Utils::choose(worldDim.colMax);
    int delta = Utils::coin_flip() ? 1 : -1;
    int horiz = Utils::choose(WALLS_HORIZ_CHANCE);

    // See if the SpecialMap disallows certain types of walls.
    if (specialMap) {
      if (!specialMap->vert_extra_walls()) {
        if (!specialMap->horiz_extra_walls()) {
          // Shouldn't ever go here.  You'll end up with a map with no
          // extra walls at all.
          return;
        }
        horiz = 1;
      }
      else if (!specialMap->horiz_extra_walls()) {
        horiz = 0;
      }
    }

    Loc check;
    for (check.c = loc.c - objectDimMax.colMax;
	       check.c <= loc.c + objectDimMax.colMax;
	       check.c++) {
	    for (check.r = loc.r - objectDimMax.rowMax;
	         check.r <= loc.r + objectDimMax.rowMax;
	         check.r++) {
	      if (!world->open(check,True)) {
	        ok = False;
        }
      }
    }

    // Horizontal extra wall.
    if (horiz) {
	    while (ok && Utils::choose(WALL_LENGTH)) {
	      for (check.c = loc.c;
		         check.c != loc.c + delta * (objectDimMax.colMax + 1);
		         check.c += delta) {
	        for (check.r = loc.r - objectDimMax.rowMax;
                check.r <= loc.r + objectDimMax.rowMax;
                check.r++) {
		        if (!world->open(check,True)) {
		          ok = False;
            }
          }
        }

	      if (ok) {
		      if (! Utils::choose(LADDER_CHANCE)) {
		        ladder(loc);
          }
		      else {
		        map[loc.r][loc.c] = Wwall;
          }

    		  loc.c += delta;

          // Don't perturb floors up/down if special map disallows it.
		      if (specialMap == NULL ||
              specialMap->vert_extra_walls()) {
            if (Utils::choose(UP_DOWN_CHANCE) == 0) {
		          loc.r += (Utils::coin_flip() ? 1 : -1);
            }
          }
        }
      }
    }
    // Vertical wall, horiz == 0
    else {
	    while (ok && Utils::choose(WALL_LENGTH)) {
        for (check.r = loc.r;
             check.r != loc.r + delta * (objectDimMax.rowMax + 1);
             check.r += delta) {
          for (check.c = loc.c - objectDimMax.colMax;
               check.c <= loc.c + objectDimMax.colMax;
               check.c++) {
            if (!world->open(check,True)) {
	            ok = False;
            }
          }
        }
        if (ok) {
          map[loc.r][loc.c] = Wwall;
          loc.r += delta;
        }
      }
    }
  } // for walls.
}



void Blueprints::up_downs() {
  Loc loc;

  // Change Wwall to WupDown if a ladder is immediately to left or right.
  for (loc.c = 0; loc.c < worldDim.colMax; loc.c++) {
    for (loc.r = 0; loc.r < worldDim.rowMax; loc.r++) {
      if ((map[loc.r][loc.c] == Wwall) &&
          (((loc.c + 1 < worldDim.colMax) &&
	          (map[loc.r][loc.c + 1] == Wladder)) ||
           ((loc.c - 1 >= 0) &&
	          (map[loc.r][loc.c - 1] == Wladder)))) {
	      map[loc.r][loc.c] = WupDown;
      }
    }
  }


  // Change Wwall to WupDown if doesn't have a wall to right AND left.
  for (loc.c = 0; loc.c < worldDim.colMax; loc.c++) {
    for (loc.r = 0; loc.r < worldDim.rowMax; loc.r++) {
      if (map[loc.r][loc.c] == Wwall &&
          !((wall_below(loc.r - 1,loc.c - 1,3) &&
	           wall_below(loc.r - 1,loc.c + 1,3)) ||
	          wall_below(loc.r - 1,loc.c,1) ||
	          wall_below(loc.r + 1,loc.c,1))) {
        map[loc.r][loc.c] = WupDown;
      }
    }
  }
}



void Blueprints::print() {
  RoomIndex ri;

  for (ri.down = 0; ri.down <= worldRooms.downMax; ri.down++) {
    // Horizontal boundaries.
    for (ri.across = 0; ri.across < worldRooms.acrossMax; ri.across++) {
      cout << ((horiz[ri.down][ri.across]) ? "**" : "*-");
      cout << "*" << endl;

      // Vertical boundaries.
      if (ri.down < worldRooms.downMax) {
        for (ri.across = 0; ri.across <= worldRooms.acrossMax; ri.across++) {
          cout << ((vert[ri.down][ri.across]) ? "* " : "| ");
        }
      }

      cout << endl;
    }
  }
}



Boolean Blueprints::blocked_right(const RoomIndex &ri) {
  assert(ri.down < worldRooms.downMax && ri.down >= 0);
  assert(ri.across < worldRooms.acrossMax && ri.across >= 0);

  return vert[ri.down][ri.across + 1];
}



Boolean Blueprints::blocked_bottom(const RoomIndex &ri) {
  assert(ri.down < worldRooms.downMax && ri.down >= 0);
  assert(ri.across < worldRooms.acrossMax && ri.across >= 0);

  return horiz[ri.down + 1][ri.across];
}



Boolean Blueprints::blocked_left(const RoomIndex &ri) {
  assert(ri.down < worldRooms.downMax && ri.down >= 0);
  assert(ri.across < worldRooms.acrossMax && ri.across >= 0);

  return vert[ri.down][ri.across];
}



Boolean Blueprints::blocked_top(const RoomIndex &ri) {
  assert(ri.down < worldRooms.downMax && ri.down >= 0);
  assert(ri.across < worldRooms.acrossMax && ri.across >= 0);

  return horiz[ri.down][ri.across];
}



void Blueprints::fill_random() {
  // Don't do maze algorithm if the world is too small.
  if ((worldRooms.acrossMax < 2) || (worldRooms.downMax < 2)) {
    return;
  }

  int strandsNum = 
    (int)(worldRooms.acrossMax * worldRooms.downMax * STRAND_PERCENT);
  for (int n = 0; n < strandsNum; n++) {
    // Starting position.
    RoomIndex ri;
    ri.across = Utils::choose(worldRooms.acrossMax - 1) + 1;
    ri.down = Utils::choose(worldRooms.downMax - 1) + 1;

    Boolean ok = open(ri);
    // open implies that ri must not be on outer boundaries.
    while (Utils::choose(STRAND_LENGTH) && ok) {
      // Move horiz.
	    if (Utils::coin_flip()) {
        // Increase.
        if (Utils::coin_flip()) {
          horiz[ri.down][ri.across] = True;
          ri.across++;
          ok = open(ri);
        }
        // Decrease.
        else {
          horiz[ri.down][ri.across - 1] = True;
          ri.across--;
          ok = open(ri);
        }
  	  }
      // Move vert.
      else {
        // Increase.
        if (Utils::coin_flip()) {
          vert[ri.down][ri.across] = True;
          ri.down++;
          ok = open(ri);
        }
        // Decrease.
        else {
          vert[ri.down - 1][ri.across] = True;
          ri.down--;
          ok = open(ri);
        }
      }
		}	  
  }
}



void Blueprints::special_map_adjust() {
  if (specialMap == NULL) {
    return;
  }

  switch (specialMap->room_maze()) {
    case SpecialMap::NORMAL:
    case SpecialMap::EMPTY:
      break;

    case SpecialMap::HORIZ_ONLY: {
      // remove all interior vertical walls.
      RoomIndex ri;
      for (ri.down = 0; ri.down < worldRooms.downMax; ri.down++) {
        for (ri.across = 1; ri.across < worldRooms.acrossMax; ri.across++) {
          vert[ri.down][ri.across] = False;
        }
      }
    }
    break;

    case SpecialMap::ZIG_ZAG: {
      RoomIndex ri;
      // Loop over all but top and bottom rows.
      for (ri.down = worldRooms.downMax - 1; ri.down > 0; ri.down--) {
        for (ri.across = 0; ri.across < worldRooms.acrossMax; ri.across++) {
          // Every row alternates having right and left side open
          // First row on bottom should have right open.
          Boolean rightOpen = ((worldRooms.downMax - ri.down) % 2 == 1);

          if ((rightOpen &&
	             ri.across != worldRooms.acrossMax - 1) ||
	            (!rightOpen &&
	             ri.across != 0)) {
	          horiz[ri.down][ri.across] = True;
          }
        }
      }
    }
    break;

    default:
      assert(0);
    break;
  } // switch
}



Boolean Blueprints::open(const RoomIndex &ri) {
  assert((ri.across >= 0) && 
         (ri.across <= worldRooms.acrossMax) &&
	       (ri.down >= 0) && 
         (ri.down <= worldRooms.downMax));

  if ((ri.across > 0) && horiz[ri.down][ri.across - 1]) {
    return False;
  }

  if ((ri.across < worldRooms.acrossMax) && horiz[ri.down][ri.across]) {
    return False;
  }

  if ((ri.down > 0) && vert[ri.down - 1][ri.across]) {
    return False;
  }

  if ((ri.down < worldRooms.downMax) && vert[ri.down][ri.across]) {
    return False;
  }

  return True;
}



void Blueprints::has_top(const RoomIndex &,const Loc &roomLoc) {
  for (int c = roomLoc.c; c < roomLoc.c + W_ROOM_COL_MAX; c++) {
    map[roomLoc.r][c] = Wwall;
  }
}



void Blueprints::missing_top(const RoomIndex &roomIndex,
				                     const Loc &roomLoc) {
  for (int c = roomLoc.c + middleCols[roomIndex.across];
       c < roomLoc.c + middleCols[roomIndex.across] + objectDimMax.colMax;
       c++) {
    for (int r = roomLoc.r; r < roomLoc.r + W_ROOM_ROW_MAX - 1; r++) {
      map[r][c] = Wladder;
    }
  }
}



void Blueprints::has_bottom(const RoomIndex &,
			                      const Loc &roomLoc) {
  for (int c = roomLoc.c; c < roomLoc.c + W_ROOM_COL_MAX; c++) {
    map[roomLoc.r + W_ROOM_ROW_MAX - 1][c] = Wwall;
  }
}



void Blueprints::missing_bottom(const RoomIndex &roomIndex,
				                        const Loc &roomLoc) {
  for (int c = roomLoc.c + middleCols[roomIndex.across];
       c < roomLoc.c + middleCols[roomIndex.across] + objectDimMax.colMax;
       c++) {
    for (int r = roomLoc.r + middleRows[roomIndex.down] - objectDimMax.rowMax;
         r < roomLoc.r + W_ROOM_ROW_MAX;
         r++) {
      map[r][c] = Wladder;
    }
  }
}



void Blueprints::has_left(const RoomIndex &,const Loc &roomLoc) {
  for (int r = roomLoc.r; r < roomLoc.r + W_ROOM_ROW_MAX; r++) {
    map[r][roomLoc.c] = Wwall;
  }
}



void Blueprints::missing_left(const RoomIndex &roomIndex,
                              const Loc &roomLoc) {
  for (int c = roomLoc.c; c < roomLoc.c + middleCols[roomIndex.across]; c++) {
    map[roomLoc.r + middleRows[roomIndex.down]][c] = Wwall;
  }
}



void Blueprints::has_right(const RoomIndex &,const Loc &roomLoc) {
  for (int r = roomLoc.r; r < roomLoc.r + W_ROOM_ROW_MAX; r++) {
    map[r][roomLoc.c + W_ROOM_COL_MAX - 1] = Wwall;
  }
}



void Blueprints::missing_right(const RoomIndex &roomIndex,
                               const Loc &roomLoc) {
  for (int c = roomLoc.c + middleCols[roomIndex.across] + objectDimMax.colMax;
       c < roomLoc.c + W_ROOM_COL_MAX; 
       c++) {
    map[roomLoc.r + middleRows[roomIndex.down]][c] = Wwall;
  }
}



void Blueprints::ladder(const Loc &init) {
  // delta is only -1 and 1.
  for (int delta = -1; delta <= 1; delta += 2) {
    Loc loc = init;

    // To avoid hitting init twice.
    if (delta == 1) {
	    loc.r++;
    }

    // Running directly into wall or ladder.
    while (world->open(loc,True)) {
      map[loc.r][loc.c] = Wladder;

      // Stop when there is a wall to the left or right.
      Loc left, right;
      right.r = left.r = loc.r;
      left.c = loc.c - 1;
      right.c = loc.c + 1;
      if ((!world->open(left) || !world->open(right))) {
        // Extra extension at top.
        if (delta == -1) {
          for (int extra = 1; extra <= objectDimMax.rowMax; extra++) {
		        Loc extraLoc;
		        extraLoc.c = loc.c;
		        extraLoc.r = loc.r - extra;
		        if (world->open(extraLoc,True)) {
		          map[extraLoc.r][extraLoc.c] = Wladder;
            }
		        else {
		          break;
            }
	        }
        }
	      break;
	    }

      loc.r += delta;
    } // while
  } // for delta
}



Boolean Blueprints::wall_below(int rTop,int cTop,int num) {
  Loc loc;
  loc.c = cTop;

  for (loc.r = rTop; loc.r < rTop + num; loc.r++) {
    if (world->inside(loc) &&
        (map[loc.r][loc.c] == Wwall || map[loc.r][loc.c] == WupDown)) {
      return True;
    }
  }

  return False;
}



SpecialMap::~SpecialMap() {
}



RoomMaze SpecialMap::room_maze() {
  return NORMAL;
}




Boolean SpecialMap::horiz_extra_walls() {
  return True;
}
 


Boolean SpecialMap::vert_extra_walls() {
  return True;
}



Boolean SpecialMap::use_movers() {
  return True;
}



Boolean SpecialMap::big_physicals() {
  return False;
}
  


Boolean SpecialMap::do_doors() {
  return True;
}



// Functions for Mover class.
Mover::Mover(InStreamP in,WorldP w,LocatorP l) {
  world = w;
  locator = l;
  areaSet = True;
  timerSet = False;

  area.read(in);
  moverId.read(in);
  physMoverId.read(in);
}



void Mover::update_from_stream(const Area &a) {
  assert(areaSet);
  area = a;
}



int Mover::get_write_length() {
  return 
    Area::get_write_length() +        // area
    Identifier::get_write_length() + // moverId
    Identifier::get_write_length();  // physMoverId
}



void Mover::write(OutStreamP out) {
  area.write(out);
  moverId.write(out);
  physMoverId.write(out);
}



void Mover::init(WorldP w,LocatorP l,const Area &a,const Size &v,
                 MoverId mId) {
  world = w;
  locator = l;
  area = a;
  areaSet = True;
  vel = v;
  moverId = mId;
  timerSet = False;
  assert(check_area(area));
}



void Mover::init_area(const Area &a) {
  area = a;
  areaSet = True;
}



void Mover::init_not_area(WorldP w,LocatorP l,const Size &v,MoverId mId) {
  world = w;
  locator = l;
  vel = v;
  moverId = mId;
  timerSet = False;
  assert(check_area(area));
}



void Mover::clock() {
  // Vel is always valid and gives what the PREVIOUS velocity was.

  // Should never be off the track.
  assert(check_area(area));

  // not pausing at top or bottom.
  if (!timerSet) {
    // Make sure areaNext is still on the track of moverSquares.
    Area areaNext = area + vel;

    // Just move along.
    if (check_area(areaNext)) {
      area = areaNext;
    }
    // change direction.
    else if (area.wsquare_alligned()) {
      // Set timer.
      timer.set(W_MOVER_PAUSE_TIME);
      timerSet = True;
      // remember velocity
      velStored.width = -vel.width;
      velStored.height = -vel.height;
      vel.set_zero();
      // area doesn't change.
    }
    else {
      areaNext = area.wsquare_allign(vel);
      // After wsquare_allign(), area shouldn't be touching wsquares it
      // wasn't already touching.
      assert(check_area(areaNext) && !(area == areaNext));
      assert(areaNext.wsquare_alligned());
      vel = areaNext - area;
      area = areaNext;
    }
  } // timerSet

  // pausing at top or bottom.
  else {
    // Reverse direction.
    if (timer.ready() ) {
      // Can't use areaNext.
      // vel = velStored;
      // hack
      vel.width = (velStored.width > 0) ? W_MOVER_SPEED : ((velStored.width < 0) ? -W_MOVER_SPEED : 0);
      vel.height = (velStored.height > 0) ? W_MOVER_SPEED : ((velStored.height < 0) ? -W_MOVER_SPEED : 0);

      Area areaNext = area + vel;
      if (check_area(areaNext)) {
        area = areaNext;
      }
      // mover is stuck on a two square wide spot and isn't going anywhere.
      else {
        vel.set_zero();
      }

      timerSet = False;
    }
  }

  timer.clock();

  // Tell PhysMover that we've been clocked, so can update the Area it
  // presents to the world.
  PhysMoverP pM = (PhysMoverP)locator->lookup(physMoverId);
  if (pM) {
    pM->mover_clock();
  }
}



// True iff all squares covering area are moverSquares for this Mover.
Boolean Mover::check_area(const Area &area) {
  Loc list[AR_WSQUARES_MAX];
  int nItems;
  area.wsquares(list,nItems);
  // go through all wsquares on area.
  for (int m = 0; m < nItems; m++) {
    const Loc &loc = list[m];
    // wsquare outside the world.
    if (!world->inside(loc)) {
      return False;
    }
    // Mover is friend of World. (might change)
    UnionSquare *uSquare = world->unionSquares[loc.r][loc.c];
    if (!uSquare ||
        uSquare->type != UN_MOVER ||
        uSquare->mSquare.mover != this)
      return False;
  } // for
  return True;
}



UnionSquare *UnionSquare::read(InStreamP in,WorldP world) {
  UnionSquare *ret = new UnionSquare;
  assert(ret);
  
  ret->type = (int)in->read_char();

  switch (ret->type) {
  case UN_POSTER:
    ret->pSquare.poster = in->read_short();
    ret->pSquare.loc.read(in);
    in->read_short(); // padding
  break;

  case UN_DOOR:
    ret->dSquare.topBottom = in->read_short();
    ret->dSquare.dest.read(in);
    in->read_short(); // padding
  break;

  case UN_MOVER: {
    ret->mSquare.orientation = in->read_short();
    MoverId mId(in);
    ret->mSquare.mover = world->lookup(mId);
  }
  break;
    
  default:
    cerr << "Received invalid type of UnionSquare." << endl;
  }
  return ret;
}



int UnionSquare::get_write_length() {
  // Add padding to make Loc the size of Identifier.
  assert(Identifier::get_write_length() == Loc::get_write_length() + 2);

  return 
    sizeof(char) +              // type
    sizeof(short)  +              // poster/topBottom/orientation
    Identifier::get_write_length();    // loc/dest/moverId
}



void UnionSquare::write(OutStreamP out) {
  out->write_char((u_char)type);

  switch (type) {
  case UN_POSTER:
    assert(pSquare.poster <= USHRT_MAX);
    out->write_short((u_short)pSquare.poster);
    pSquare.loc.write(out);
    out->write_short(666); // padding
  break;

  case UN_DOOR:
    assert(dSquare.topBottom <= USHRT_MAX);
    out->write_short((u_short)dSquare.topBottom);
    dSquare.dest.write(out);
    out->write_short(666); // padding
  break;

  case UN_MOVER: {
    assert(mSquare.orientation <= USHRT_MAX);  
    out->write_short((u_short)mSquare.orientation);
    assert(mSquare.mover);
    MoverId mId = mSquare.mover->get_mover_id();
    mId.write(out);
  }
  break;
    
  default:
    assert(0);
  }
}



// Functions for World class.
World::World() {
  // Set by World::set_locator();
  locator = NULL;

  // Get pointer to static list of themes.
  themes = World_themes;

  objectDimMax.colMax = OBJECT_COL_MAX;
  objectDimMax.rowMax = OBJECT_ROW_MAX;

  mapPrint = MAP_PRINT_DEFAULT;
  rooms.acrossMax = 1;
  rooms.downMax = 1;
  roomsNext = rooms;
  rooms_dim_size_update();
  blueprints = NULL;
  moversNum = 0;
  uniqueGen = 0;
  justReset = True;

  specialMap = specialMapNext = NULL;

  worldFile = NULL;

  xValid = XVARS_VALID_INIT;

  Loc loc;
  for (loc.r = 0; loc.r < W_ROW_MAX_MAX; loc.r++) {
    for (loc.c = 0; loc.c < W_COL_MAX_MAX; loc.c++) {
      unionSquares[loc.r][loc.c] = NULL;
      map[loc.r][loc.c] = Wempty;
    }
  }

  
  // Initialize posterDims
  for (int n = 0; n < W_ALL_POSTERS_NUM; n++) {
    assert(posters[n].size.width % WSQUARE_WIDTH == 0 &&
           posters[n].size.width % WSQUARE_HEIGHT == 0);
    // The scaling factor of 2 is always true, regardless of whether we
    // are dealing with stretched coordinates or not.
    posterDims[n].colMax = posters[n].size.width / (WSQUARE_WIDTH * 2);
    posterDims[n].rowMax = posters[n].size.height / (WSQUARE_HEIGHT * 2);
  }
  
  choose_theme();

  // Caller now must explicitly ask for the title_map().
}



World::~World() {
  clear_everything();

  Utils::freeif(worldFile);
  
  delete specialMap;
  delete specialMapNext;

	// Generate new map.
  if (blueprints) {
    delete blueprints;
  }
}



void World::title_map(IViewportInfo* vInfoProvider) {
  ViewportInfo vInfo = vInfoProvider->get_info();
  Dim vDim = vInfo.get_viewport_dim();

  // Set to the dimensions of the viewport.
  clear_everything();
  rooms_dim_size_update(&vDim);


  // Put wall on top and bottom.
  Loc loc;
  for (loc.r = 0; loc.r < dim.rowMax; loc.r++) {
    for (loc.c = 0; loc.c < dim.colMax; loc.c++) {
      // Top and bottom.
      if ((loc.r == (dim.rowMax - 1)) || (loc.r == 0)) {
        map[loc.r][loc.c]= Wwall;
      }
    }
  }
        

  // Put title poster in center of screen.
  //
  // Title poster is always the last poster in the list.
  Loc start;
  start.c = dim.colMax / 2 - posterDims[W_ALL_POSTERS_NUM - 1].colMax / 2;
  start.r = dim.rowMax / 2 - posterDims[W_ALL_POSTERS_NUM - 1].rowMax / 2;

  for (loc.c = start.c; 
       loc.c <  start.c + posterDims[W_ALL_POSTERS_NUM - 1].colMax; 
       loc.c++) {
    for (loc.r = start.r; 
         loc.r < start.r + posterDims[W_ALL_POSTERS_NUM - 1].rowMax; 
         loc.r++) {
      if (! unionSquares[loc.r][loc.c]) {
        unionSquares[loc.r][loc.c] = new UnionSquare;
      }
      assert(unionSquares[loc.r][loc.c]);
      unionSquares[loc.r][loc.c]->type = UN_POSTER;
      unionSquares[loc.r][loc.c]->pSquare.poster = W_ALL_POSTERS_NUM - 1;
      unionSquares[loc.r][loc.c]->pSquare.loc.c = loc.c - start.c;
      unionSquares[loc.r][loc.c]->pSquare.loc.r = loc.r - start.r;
    }
  }
}



Dim World::get_room_dim() {
  Dim ret(W_ROOM_ROW_MAX,W_ROOM_COL_MAX);
  return ret;
}



Size World::get_room_size() {
  Size ret;
  ret.width = W_ROOM_COL_MAX * WSQUARE_WIDTH;
  ret.height = W_ROOM_ROW_MAX * WSQUARE_HEIGHT;
  return ret;
}



Boolean World::check_door(const Loc &loc,Loc &dest) {
  if (unionSquares[loc.r][loc.c] &&
      unionSquares[loc.r][loc.c]->type == UN_DOOR) {
    dest = unionSquares[loc.r][loc.c]->dSquare.dest;
    return True;
  }
  return False;
}



void World::set_rooms_next(const Rooms &r) {
  Utils::freeif(worldFile);

  roomsNext.acrossMax = Utils::minimum(W_ACROSS_MAX_MAX,r.acrossMax);
  roomsNext.downMax = Utils::minimum(W_DOWN_MAX_MAX,r.downMax);
}



void World::set_special_map_next(SpecialMapP map) {
  Utils::freeif(worldFile);
  if (specialMapNext) {
    delete specialMapNext;
  }
  // Ok for map to be NULL.
  specialMapNext = map;

  if (specialMapNext) {
    roomsNext = specialMapNext->get_rooms();
    assert(roomsNext.acrossMax <= W_ACROSS_MAX_MAX &&
           roomsNext.downMax <= W_DOWN_MAX_MAX);
  }
  // else don't touch roomsNext, caller will have set it.
}



Boolean World::overlap(const Box &box) {
  return ((box.loc.c < dim.colMax) &&
	  (box.loc.c + box.dim.colMax > 0) &&
	  (box.loc.r < dim.rowMax) &&
	  (box.loc.r + box.dim.rowMax > 0));
}



void World::draw(CMN_DRAWABLE buffer,Xvars &xvars,int dpyNum,
                 const Area &area,
                 Boolean reduceDraw,Boolean background3D) {
  if (!xvars.is_valid(xValid)) {
    init_x(xvars,IX_INIT,NULL);
  }

  if (!reduceDraw) {
    draw_background(buffer,xvars,dpyNum,area,background3D);
  }

  Pos aPos;
  Size aSize;
  area.get_rect(aPos,aSize);

  int moversFoundNum = 0;
  MoverP moversFound[MOVERS_MAX];

  // Draw squares and create list of movers encountered.
  Loc loc;
  for (loc.c = (int)floor((float)aPos.x / (float)WSQUARE_WIDTH);
       loc.c < (int)ceil((float)(aPos.x + aSize.width) / (float)WSQUARE_WIDTH);
       loc.c++) {
    for (loc.r = (int)floor((float)aPos.y / (float)WSQUARE_HEIGHT);
         loc.r < ceil((float)(aPos.y + aSize.height)/(float)WSQUARE_HEIGHT);
         loc.r++) {
      draw_square(buffer,xvars,dpyNum,loc,
                  loc.c * WSQUARE_WIDTH - aPos.x,
                  loc.r * WSQUARE_HEIGHT - aPos.y,reduceDraw);

      mover_list_add(moversFound,moversFoundNum,loc);
    }
  }

  // Draw movers once.
  for (int n = 0; n < moversFoundNum; n++) {
    Pos movPos;
    Size movSize;
    const Area &area = moversFound[n]->get_area();
    area.get_rect(movPos,movSize);
    draw_mover(buffer,xvars,dpyNum,moversFound[n],
               movPos.x - aPos.x,movPos.y - aPos.y);
  }

  if (!reduceDraw) {
    draw_outside(buffer,xvars,dpyNum,area);
  }
}



void World::draw_outside(CMN_DRAWABLE buffer,Xvars &xvars,int dpyNum,
                         Area area) {
  if (!xvars.is_valid(xValid)) {
    init_x(xvars,IX_INIT,NULL);
  }
  
  // In window coords. 
  area = xvars.stretch_area(area);
  Pos pos = area.get_pos();
  Size size = area.get_size();
  Size worldSize = xvars.stretch_size(get_size());
  
  // Yuck, divide by two and stretch.
  // World::outsides, backgrounds, and posters should
  // really be in unstretched coordinates.
  Size imgSize;
  imgSize.set(xvars.stretch_x(outsides[outsideIndex].size.width >> 1),
              xvars.stretch_y(outsides[outsideIndex].size.height >> 1));
  
  // Top side, stick out on right and left
  if (pos.y < 0) {
    for (int across = Utils::div(pos.x,imgSize.width);
         across * imgSize.width < pos.x + size.width;
         across++) {
      for (int down = Utils::div(pos.y,imgSize.height);
           down < 0;
           down++) {
        // area in window coords
        Area imgArea(Pos(across * imgSize.width 
                         - pos.x,down * imgSize.height - pos.y),
                     imgSize);
        // Clip to window (0,0,size.width,size.height)
        Size srcOffset = imgArea.clip(size);
        
        draw_outside_offset(buffer,xvars,
                            dpyNum,srcOffset,imgArea);
      }
    }
  }
  
  // Left side, stick out on bottom
  if (pos.x < 0) {
    for (int down = Utils::maximum(0,Utils::div(pos.y,imgSize.height));
         down * imgSize.height < pos.y + size.height;
         down++) {
      for (int across = Utils::div(pos.x,imgSize.width);
           (across * imgSize.width < pos.x + size.width) && (across < 0);
           across++) {
        // area in window coords
        Area imgArea(Pos(across * imgSize.width - pos.x,
                         down * imgSize.height - pos.y),imgSize);
        // Clip to window (0,0,size.width,size.height)
        Size srcOffset = imgArea.clip(size);
        
        draw_outside_offset(buffer,xvars,
                            dpyNum,srcOffset,imgArea);
      }
    }
  }
  
  
  // Bottom side, stick out on right
  if (pos.y + size.height > worldSize.height) {
    for (int across = Utils::maximum(0,Utils::div(pos.x,imgSize.width));
         across * imgSize.width < pos.x + size.width;
         across++) {
      for (int down = Utils::maximum(Utils::div(pos.y,imgSize.height),
                                     worldSize.height / imgSize.height);
           down * imgSize.height < pos.y + size.height;
           down++) {
        // area in window coords
        Area imgArea(Pos(across * imgSize.width 
                         - pos.x,down * imgSize.height - pos.y),
                     imgSize);
        // Clip to window (0,0,size.width,size.height)
        Size srcOffset = imgArea.clip(size);
        // Clip to world.
        srcOffset += imgArea.clip_top(worldSize.height - pos.y);
        
        draw_outside_offset(buffer,xvars,
                            dpyNum,srcOffset,imgArea);
      }
    }
  }
  
  // Right side, 
  if (pos.x + size.width > worldSize.width) {
    for (int down = Utils::maximum(0,Utils::div(pos.y,imgSize.height));
         (down * imgSize.height < pos.y + size.height) 
           && (down * imgSize.height < worldSize.height);
         down++) {
      for (int across = Utils::maximum(Utils::div(pos.x,imgSize.width),
                                       worldSize.width / imgSize.width);
           across * imgSize.width < pos.x + size.width;
           across++) {
        // area in window coords
        Area imgArea(Pos(across * imgSize.width 
                         - pos.x,down * imgSize.height - pos.y),
                     imgSize);
        // Clip to window (0,0,size.width,size.height)
        Size srcOffset = imgArea.clip(size);
        // Clip to world.
        srcOffset += imgArea.clip_left(worldSize.width - pos.x);
        
        draw_outside_offset(buffer,xvars,
                            dpyNum,srcOffset,imgArea);
      }
    }
  }
}



Boolean World::open(const Loc &loc,Boolean laddersClosed,Boolean postersClosed,
		                Boolean doorsClosed,Boolean outsideClosed) {
  if (!inside(loc)) {
    return !outsideClosed;
  }

  // a poster isn't closed at loc
  Boolean posterOpen =
  (!postersClosed ||
   !(unionSquares[loc.r][loc.c] &&
     unionSquares[loc.r][loc.c]->type == UN_POSTER));

  // a door isn't "closed" at loc
  Boolean doorOpen =
	  (!doorsClosed ||
	   !(unionSquares[loc.r][loc.c] &&
	     unionSquares[loc.r][loc.c]->type == UN_DOOR));

  // a moverSquare isn't "closed" at loc  (uses laddersClosed for now)
  Boolean moverSquareOpen =
	  (!laddersClosed ||
	   !(unionSquares[loc.r][loc.c] &&
	     unionSquares[loc.r][loc.c]->type == UN_MOVER));

  Boolean ret =
	  // map contains an open square at loc
	  (map[loc.r][loc.c] == Wempty ||
	   map[loc.r][loc.c] == WtextSquare ||
	   map[loc.r][loc.c] == Wsquanch ||
	   (map[loc.r][loc.c] == Wladder && !laddersClosed)) &&
    posterOpen &&
    doorOpen &&
    moverSquareOpen;

  // quick sanity check.
  if (unionSquares[loc.r][loc.c]) {
    assert(map[loc.r][loc.c] == Wempty || map[loc.r][loc.c] == Wwall);
  }

  return ret;
}



// The only World::open() that takes movers into account.
Boolean World::open(const Area &area,Boolean laddersClosed,
                    Boolean postersClosed,Boolean doorsClosed,
                    Boolean outsideClosed) {
  /* Avoid a certain type of crash when area is far out of the world.
     I.e. when area.pos is near overflow. */
  if (!inside(area.middle_wsquare())) {
    // Fix this to deal with outsideClosed.
    return False;
  }
  

  // Try all wsquares on top of area.
  Loc list[AR_WSQUARES_MAX];
  int nsquares;
  area.wsquares(list,nsquares);
  for (int n = 0; n < nsquares; n++) {
    if (!open(list[n],laddersClosed,postersClosed,
              doorsClosed,outsideClosed)) {
      return False;
    }

    // Have to add this inside() check because if outsideClosed is False, 
    // we can still get here.
    if (inside(list[n])) {
      // Look at movers corresponding to moverSquares that are covered by area.
      UnionSquare *uSquare = unionSquares[list[n].r][list[n].c];
      if (uSquare && uSquare->type == UN_MOVER) {
        const Area &moverArea = uSquare->mSquare.mover->get_area();
        if (moverArea.overlap(area)) {
          return False;
        }
      }
    }
  } // for
  
  return True;
}



Boolean World::open(const Box &box,Boolean laddersClosed,Boolean postersClosed,
		                Boolean doorsClosed) {
  Loc loc;
  for (loc.c = box.loc.c; loc.c < box.loc.c + box.dim.colMax; loc.c++) {
    for (loc.r = box.loc.r; loc.r < box.loc.r + box.dim.rowMax; loc.r++) {
      if (!open(loc,laddersClosed,postersClosed,doorsClosed)) {
        return False;
      }
    }
  }
  return True;
}



Wsig World::open_offset(Size &offset,const Area &areaNew,const Vel &vel) {
  if (open(areaNew)) {
    return W_NO_SIG;
  }

  // wrap-around disabled.
#if 0
  // Check for player wrapping around the world.
  if (check_wrap_around(offset,areaNew)) {
    return W_CLOSE;
  }
#endif

  Vel opp = -1.0f * vel;

  Dir best[4];
  Dir others[4];
  int bestNum,othersNum;
  opp.get_dirs_4(best,others,bestNum,othersNum);

  if (open_try_dirs(offset,areaNew,best,bestNum)) {
    return W_CLOSE;
  }

  if (open_try_dirs(offset,areaNew,others,othersNum)) {
    return W_CLOSE;
  }

	// Special "Hero stuck in corner" case.
	if (open_try_corners(offset,areaNew)) {
		return W_CLOSE;
  }

	// Big jump.
  if (open_try_diagonals(offset,areaNew)) {
    return W_CLOSE_BAD;
  }

  return W_FAILURE;
}



void World::compute_touching_hanging(Touching &touching, Hanging &hanging,
                                     MoverP &touchingMover,const Area &area) {
  Pos apos;
  Size asize;
  area.get_rect(apos,asize);

  touching = CO_air;
  hanging.corner = CO_air;

  TouchingList touchingList;
  area.touching_wsquares(touchingList);

  TouchingList edgeList;
  area.edge_wsquares(edgeList);

  // Set to NULL if not touching a Mover.
  touchingMover = NULL;

  // Last one takes priority.  Down is highest priority, then
  // Up, then right, left.
  th_helper(touching,hanging,touchingMover,touchingList.l,edgeList.l,
            True,area,apos.y,asize.height,
            WSQUARE_HEIGHT,CO_l,CO_l_UP,CO_l_DN);
  th_helper(touching,hanging,touchingMover,touchingList.r,edgeList.r,
            True,area,apos.y,asize.height,
            WSQUARE_HEIGHT,CO_r,CO_r_UP,CO_r_DN);
  th_helper(touching,hanging,touchingMover,touchingList.up,edgeList.up,
            False,area,apos.x,asize.width,
            WSQUARE_WIDTH,CO_up,CO_up_L,CO_up_R);
  th_helper(touching,hanging,touchingMover,touchingList.dn,edgeList.dn,
            False,area,apos.x,asize.width,
            WSQUARE_WIDTH,CO_dn,CO_dn_L,CO_dn_R);
}



void World::demo_reset() {
  // Don't reuse the nonce because may have old packets
  // uniqueGen = 0;

  justReset = True;
  choose_theme();
}



void World::clear_everything() {
  // Delete old unionSquares.
  Loc loc;
  for (loc.r = 0; loc.r < dim.rowMax; loc.r++) {
    for (loc.c = 0; loc.c < dim.colMax; loc.c++) {
      if (unionSquares[loc.r][loc.c]) {
        delete unionSquares[loc.r][loc.c];
        unionSquares[loc.r][loc.c] = NULL;
      }
    }
  }

  // Change rooms,dim,size of world.
  rooms = roomsNext;
  delete specialMap;
  if (specialMapNext) {  
    specialMap = specialMapNext->clone();
    assert(specialMap);
  }
  else {
    specialMap = NULL;
  }
  rooms_dim_size_update();

  // Clear map.
  for (loc.r = 0; loc.r < dim.rowMax; loc.r++) {
    for (loc.c = 0; loc.c < dim.colMax; loc.c++) {
      map[loc.r][loc.c] = Wempty;
    }
  }

  delete_movers();
}



void World::close_horiz_mover(const Loc &loc,Boolean &inHorizMover) {
  if (inHorizMover) {
    // initialize the mover.
    MoverId moverId;
    moverId.index = moversNum;
    moverId.unique = uniqueGen++;

    Size vel;
    vel.set(Utils::coin_flip() ? W_MOVER_SPEED : -W_MOVER_SPEED,0);

    // Use specified initial position
    if (movers[moversNum]->is_area_set()) {    
      movers[moversNum]->init_not_area(this,locator,vel,moverId);
    }
    // Start at right end of the track.
    else {
      // Doesn't check that we have at least two wsquares to the right.
      Pos pos((loc.c - 2) * WSQUARE_WIDTH,loc.r * WSQUARE_HEIGHT);
      Area area(AR_RECT,pos,moverSize);
      movers[moversNum]->init(this,locator,area,vel,moverId);
    }

    // Dummy mover object, so mover gets redrawn.
    PhysMoverP p = new PhysMover(this,locator,movers[moversNum]);
    assert(p);
    locator->add(p);
    movers[moversNum]->set_phys_mover_id(p->get_id());

    moversNum++;
    inHorizMover = False;
  } 
}



void World::close_vert_movers(const Dim &dim) {
  Loc l;
  // Traverse all squares.
  for (l.c = 0; l.c < (dim.colMax - 1); l.c++) {
    for (l.r = 0; l.r < dim.rowMax; l.r++) {
      Loc loc = l;
      Boolean inVertMover = False;
      // Go down, filling out mover.
      // (MoverP)-1 hack is described in read_from_file().
      while (loc.r < dim.rowMax &&
             unionSquares[loc.r][loc.c] && 
             unionSquares[loc.r][loc.c]->type == UN_MOVER &&
             (unionSquares[loc.r][loc.c]->mSquare.mover == NULL ||
              unionSquares[loc.r][loc.c]->mSquare.mover == (MoverP)-1) &&
             unionSquares[loc.r][loc.c]->mSquare.orientation == OR_VERT &&
             unionSquares[loc.r][loc.c+1] && 
             unionSquares[loc.r][loc.c+1]->type == UN_MOVER &&
             (unionSquares[loc.r][loc.c+1]->mSquare.mover == NULL ||
              unionSquares[loc.r][loc.c+1]->mSquare.mover == (MoverP)-1) &&
             unionSquares[loc.r][loc.c+1]->mSquare.orientation == OR_VERT) {
        if (!inVertMover) {
          // create the mover.
          inVertMover = True;
          assert(moversNum < MOVERS_MAX);
          movers[moversNum] = new Mover;
          assert(movers[moversNum]);
        }

        // If starting position of Mover is given by user.
        if (unionSquares[loc.r][loc.c]->mSquare.mover == (MoverP)-1 && 
            !movers[moversNum]->is_area_set()) {
          Pos pos(loc.c * WSQUARE_WIDTH,loc.r * WSQUARE_HEIGHT);
          Area area(AR_RECT,pos,moverSize);
          movers[moversNum]->init_area(area);
        }

        unionSquares[loc.r][loc.c]->mSquare.mover = movers[moversNum];        
        unionSquares[loc.r][loc.c+1]->mSquare.mover = movers[moversNum];        
        loc.r++;
      }  
      // Close out mover.
      if (inVertMover) {
        // initialize the mover.
        MoverId moverId;
        moverId.index = moversNum;
        moverId.unique = uniqueGen++;

        Size vel;
        vel.set(0,Utils::coin_flip() ? W_MOVER_SPEED : -W_MOVER_SPEED);

        // Use specified initial position
        if (movers[moversNum]->is_area_set()) {    
          movers[moversNum]->init_not_area(this,locator,vel,moverId);
        }
        // Start at top.
        else {
          Pos pos(l.c * WSQUARE_WIDTH,l.r * WSQUARE_HEIGHT);
          Area area(AR_RECT,pos,moverSize);
          movers[moversNum]->init(this,locator,area,vel,moverId);
        }

        // Dummy mover object, so mover gets redrawn.
        PhysMoverP p = new PhysMover(this,locator,movers[moversNum]);
        assert(p);
        locator->add(p);
        movers[moversNum]->set_phys_mover_id(p->get_id());

        moversNum++;
      }
    } // for
  } // for
}



Boolean World::read_from_file(char *filename) {
  FILE *fp = fopen(filename,"r");
  if (!fp) {
    cerr << "Could not open " << filename << endl;
    return False;
  }

  if (EOF == fscanf(fp,"XEvil World 1.0\n")) {
    cerr << "Invalid header in world file." << endl;
    fclose(fp);
    return False;
  }

  Dim dim;  
  if (2 != fscanf(fp,"%dx%d\n",&dim.colMax,&dim.rowMax)) {
    cerr << "Could not read size of world." << endl;
    fclose(fp);
    return False;
  }

  if (dim.colMax > W_COL_MAX_MAX) {
    cerr << "Across size is larger than maximum. Truncating to " << W_COL_MAX_MAX << endl;
    dim.colMax = W_COL_MAX_MAX;
  }

  if (dim.rowMax > W_ROW_MAX_MAX) {
    cerr << "Down size is larger than maximum. Truncating to " << W_ROW_MAX_MAX << endl;
    dim.rowMax = W_ROW_MAX_MAX;
  }

  Dim halfRoom(W_ROOM_ROW_MAX / 2,W_ROOM_COL_MAX / 2);
  if (dim.rowMax % halfRoom.rowMax != 0 || 
      dim.colMax % halfRoom.colMax != 0) {
    cerr << "Dimensions of world must be multiples of " << halfRoom.colMax << 
      " across and " << halfRoom.rowMax << " down." << endl;
    fclose(fp);
    return False;
  }

  if (dim.rowMax <= 0 || dim.colMax <= 0) {
    cerr << "Invalid dimension, negative or zero." << endl;
    fclose(fp);
    return False;
  }

  // No longer need to pad with extra Wwall.
  this->dim = dim;
  rooms_dim_size_update(&dim);

  char buffer[W_COL_MAX_MAX];
  Loc loc;
  for (loc.r = 0; loc.r < this->dim.rowMax; loc.r++) {
    // dim is the size of the data on file, this.dim is 
    // the size alligned to room boundries, fill rest with Wwall
    if (loc.r < dim.rowMax) {
      Boolean badRow = False;
      if (!fgets(buffer,W_COL_MAX_MAX,fp)) {
        badRow = True;
      }
      if (buffer[0] == ';') {
        // Hit a comment, ignore the line.
        loc.r--;
        continue;
      }
      if ((int)strlen(buffer) < dim.colMax) {
        badRow = True;
      }
      if (badRow) {
        cerr << "Could not read World data." << endl;
        fclose(fp);
        return False;   
      }      

      Boolean inHorizMover = False;
      for (loc.c = 0; loc.c < this->dim.colMax; loc.c++) {
        if (loc.c < dim.colMax) {
          char val;
          switch (buffer[loc.c]) {
            case '.':
              val = Wempty;
              close_horiz_mover(loc,inHorizMover);
              break;
            case '#':
              val = Wwall;
              close_horiz_mover(loc,inHorizMover);
              break;
            case 'H':
              val = Wladder;
              close_horiz_mover(loc,inHorizMover);
              break;
            case '*':
              val = WupDown;
              close_horiz_mover(loc,inHorizMover);
              break;
            case '^':
            case '|':
            case '&': // sticking in floor.
              val = Wempty;
              close_horiz_mover(loc,inHorizMover);
              if (loc.c + 1 < dim.colMax &&
                  (buffer[loc.c+1] == '^' ||
                   buffer[loc.c+1] == '|' ||
                   buffer[loc.c+1] == '&')) {
                Boolean markStartPos = (buffer[loc.c] == '^' && buffer[loc.c + 1] == '^');
                Boolean stickInFloor = (buffer[loc.c] == '&' && buffer[loc.c + 1] == '&');

                // Do two squares.
                for (int x = 0; x < 2; x++) {
                  unionSquares[loc.r][loc.c] = new UnionSquare;
                  assert(unionSquares[loc.r][loc.c]);
                  unionSquares[loc.r][loc.c]->type = UN_MOVER;

                  // Big hack here.  We don't create mover here since we don't know which
                  // mover goes with which wsquare, but we still need to remember where
                  // the initial position is.  Set mover to be (MoverP)(-1) for wsquares
                  // that the user specified as initial positions.
                  if (markStartPos) {
                    unionSquares[loc.r][loc.c]->mSquare.mover = (MoverP)-1;
                  }
                  else {
                    unionSquares[loc.r][loc.c]->mSquare.mover = NULL;
                  }
                  unionSquares[loc.r][loc.c]->mSquare.orientation = OR_VERT;
                  if (x == 0) {
                    map[loc.r][loc.c] = stickInFloor ? Wwall : Wempty;
                    loc.c++;
                  }
                  else {
                    assert(x == 1);
                    val = stickInFloor ? Wwall : Wempty;
                  } 
                }                
              }
              break;
            case '~':
            case '-':
            case '%':
              if (buffer[loc.c] == '%') {
                val = Wwall;
              }
              else {
                val = Wempty;
              }
              // Make sure at least two next to each other.
              if ((loc.c + 1 < dim.colMax && (buffer[loc.c+1] == '~' || buffer[loc.c+1] == '-' || buffer[loc.c+1] == '%')) ||
                  (loc.c - 1 >= 0 && (buffer[loc.c-1] == '~' || buffer[loc.c-1] == '-' || buffer[loc.c-1] == '%'))) {                    
                if (!inHorizMover) {
                  assert(moversNum < MOVERS_MAX);
                  movers[moversNum] = new Mover;
                  assert(movers[moversNum]);
                }

                inHorizMover = True;
                unionSquares[loc.r][loc.c] = new UnionSquare;
                assert(unionSquares[loc.r][loc.c]);
                unionSquares[loc.r][loc.c]->type = UN_MOVER;
                unionSquares[loc.r][loc.c]->mSquare.mover = movers[moversNum];
                unionSquares[loc.r][loc.c]->mSquare.orientation = OR_HORIZ;

                // If starting position of Mover is given by user.
                if (buffer[loc.c] == '~' && 
                    // Need two '~' in a row.
                    loc.c + 1 < dim.colMax && buffer[loc.c + 1] == '~' &&
                    // pos not already set.
                    !movers[moversNum]->is_area_set()) {
                  Pos pos(loc.c * WSQUARE_WIDTH,loc.r * WSQUARE_HEIGHT);
                  Area area(AR_RECT,pos,moverSize);
                  movers[moversNum]->init_area(area);
                }
              }
              break;
            default:
              cerr << "Error reading map, unknown character " << buffer[loc.c] << endl;
              val = Wempty;
              close_horiz_mover(loc,inHorizMover);
          }
          map[loc.r][loc.c] = val;
        }
        else {
          // Close out mover.
          close_horiz_mover(loc,inHorizMover);
          // Fill rest of row with Wwall
          map[loc.r][loc.c] = Wwall;              
        }
      } // for loc.c

    }
    else {
      // Fill remaining rows with Wwall
      for (loc.c = 0; loc.c < this->dim.colMax; loc.c++) {
        map[loc.r][loc.c] = Wwall;
      }                    
    }
  }

  close_vert_movers(dim);

  fclose(fp);
  return True;
}



void World::read_from_stream(InStreamP in) {
  Dim dim(in);

  themeIndex = in->read_int();
  themeIndex = Utils::minimum(W_THEME_NUM - 1,themeIndex);
  backgroundIndex = in->read_int();
#if WIN32
  backgroundIndex = Utils::minimum(W_ALL_BACKGROUNDS_NUM - 1,backgroundIndex);
#endif
  outsideIndex = in->read_int();
#if WIN32
  outsideIndex = Utils::minimum(W_ALL_OUTSIDES_NUM - 1,outsideIndex);
#endif
  // Take minimum with the max number, just in case the server gives an invalid
  // number.  Only for windows, UNIX client doesn't give a shit about these numbers.
  
  if (!in->alive()) {
    // failure.
    return;
  }
  rooms_dim_size_update(&dim);
  Loc l;
  for (l.c = 0; l.c < dim.colMax; l.c++) {
    for (l.r = 0; l.r < dim.rowMax; l.r++) {
      map[l.r][l.c] = in->read_char();
    }
  }

  assert(moversNum == 0);
  moversNum = in->read_int();
  int n;
  for (n = 0; n < moversNum; n++) {
    movers[n] = new Mover(in,this,locator);
    assert(movers[n]);
  }
  
  int uCount = in->read_int();
  for (n = 0; n < uCount; n++) {
    Loc loc;
    loc.read(in);
    if (unionSquares[loc.r][loc.c]) {
      delete unionSquares[loc.r][loc.c];
    }
    unionSquares[loc.r][loc.c] = UnionSquare::read(in,this);
    assert(unionSquares[loc.r][loc.c]);

    // Hack to deal with different poster sizes on UNIX vs. Windows.
    if (unionSquares[loc.r][loc.c]->type == UN_POSTER) {
      // Weed out posters that aren't there
      if (unionSquares[loc.r][loc.c]->pSquare.poster >= 
          W_ALL_POSTERS_NUM) {
        delete unionSquares[loc.r][loc.c];
        unionSquares[loc.r][loc.c] = NULL;
      }
      else {
        // Special check to weed out posters that are too big.      
        Loc &pLoc = unionSquares[loc.r][loc.c]->pSquare.loc;
        int p = unionSquares[loc.r][loc.c]->pSquare.poster;
        if (pLoc.c >= posterDims[p].colMax ||
            pLoc.r >= posterDims[p].rowMax) {
          delete unionSquares[loc.r][loc.c];
          unionSquares[loc.r][loc.c] = NULL;
        }
      }
    }
  }
}



int World::get_write_length() {
  // Count number of union squares.
  int uCount = 0;
  Loc l;
  for (l.c = 0; l.c < dim.colMax; l.c++) {
    for (l.r = 0; l.r < dim.rowMax; l.r++) {
      if (unionSquares[l.r][l.c]) {
        uCount++;
      }
    }
  }
  
  return 
    Dim::get_write_length() +                   // dim
    sizeof(int) +                               // themeIndex
    sizeof(int) +                               // backgroundIndex
    sizeof(int) +                               // outsideIndex
    dim.rowMax * dim.colMax * sizeof(char) +    // wsquares
    sizeof(int) +                               // moversNum
    moversNum * Mover::get_write_length() +     // movers[]
    sizeof(int) +                               // uCount
    uCount * (Loc::get_write_length() + 
              UnionSquare::get_write_length()); // unionSquares[]
}



void World::write(OutStreamP out) {
  dim.write(out);
  out->write_int(themeIndex);
  out->write_int(backgroundIndex);
  out->write_int(outsideIndex);
  if (!out->alive()) {
    return;
  }
  Loc l;
  for (l.c = 0; l.c < dim.colMax; l.c++) {
    for (l.r = 0; l.r < dim.rowMax; l.r++) {
      out->write_char(map[l.r][l.c]);
    }
  }

  // Write movers before union squares so that MoverSquares can refer to
  // the movers.
  out->write_int(moversNum);
  int n;
  for (n = 0; n < moversNum; n++) {
    movers[n]->write(out);
  }

  // Count number of union squares.
  int uCount = 0;
  for (l.c = 0; l.c < dim.colMax; l.c++) {
    for (l.r = 0; l.r < dim.rowMax; l.r++) {
      if (unionSquares[l.r][l.c]) {
        uCount++;
      }
    }
  }
  out->write_int(uCount);
  for (l.c = 0; l.c < dim.colMax; l.c++) {
    for (l.r = 0; l.r < dim.rowMax; l.r++) {
      if (unionSquares[l.r][l.c]) {
        l.write(out);
        unionSquares[l.r][l.c]->write(out);
      }
    }
  }
}



// For one room.
int World::get_write_length(const RoomIndex &) {
  return W_ROOM_ROW_MAX * W_ROOM_COL_MAX * sizeof(char);
}



// Write one room.
void World::write(OutStreamP outStream,const RoomIndex &idx) {
  if (!outStream->alive()) {
    return;
  }
  Loc l;
  for (l.r = idx.down * W_ROOM_ROW_MAX; 
       l.r < (idx.down + 1) * W_ROOM_ROW_MAX;
       l.r++) {
    for (l.c = idx.across * W_ROOM_COL_MAX; 
         l.c < (idx.across + 1) * W_ROOM_COL_MAX;
         l.c++) {
      if (inside(l)) {
        outStream->write_char(map[l.r][l.c]);
      }
      else {
        // There's not really any point in writing this.
        outStream->write_char(Woutside);
      }
    }
  }
}



// Read one room
void World::read(InStreamP in,const RoomIndex &idx) {
  if (!in->alive()) {
    return;
  }
  Loc l;
  for (l.r = idx.down * W_ROOM_ROW_MAX; 
       l.r < (idx.down + 1) * W_ROOM_ROW_MAX;
       l.r++) {
    for (l.c = idx.across * W_ROOM_COL_MAX; 
         l.c < (idx.across + 1) * W_ROOM_COL_MAX;
         l.c++) {
      if (inside(l)) {
        map[l.r][l.c] = in->read_char();
      }
      else {
        // just throw it away.
        in->read_char();
      }
    }
  }
}



void World::reset(const Dim *clearDim,InStreamP inStream) {
  // Don't reuse the nonce because may have old packets
  // uniqueGen = 0;

  justReset = True;

  assert(locator);

  // Among other things, sets specialMap=specialMapNext, etc.
  clear_everything();

  // Create empty world of given dimensions.
  if (clearDim) {
    rooms_dim_size_update(clearDim);
    return;
  }

  // Read from stream if supplied
  if (inStream) {
    read_from_stream(inStream);
    return;
  }

  // Read from file if set.
  if (specialMap == NULL && worldFile) {
    if (!read_from_file(worldFile)) {
      // Failed to read, go back to default.
      clear_everything();
      // Don't bother to try again.
      Utils::freeif(worldFile);
    }
  }

  // Create regular world if worldFile is NULL or if world can't be read
  // in.
  if (!worldFile) {
    // Before blueprints
    if (specialMap && specialMap->big_physicals()) {
      objectDimMax.colMax = OBJECT_BIG_COL_MAX;
      objectDimMax.rowMax = OBJECT_BIG_ROW_MAX;
    }
    else {
      objectDimMax.colMax = OBJECT_COL_MAX;
      objectDimMax.rowMax = OBJECT_ROW_MAX;
    }

    // Generate new map.
    if (blueprints) {
      delete blueprints;
    }

    // Creates the room boundries, runs per-room maze algorithm.
    blueprints = new Blueprints(this,map,objectDimMax,specialMap);
    assert(blueprints);

    if (mapPrint) {
      blueprints->print();
    }

    if (specialMap && specialMap->room_maze() == SpecialMap::EMPTY) {
      blueprints->implement_edges_only();
      // Fill in top rows, maybe this should be inside Blueprints.
      for (int c = 0; c < dim.colMax; c++) {
        for (int r = 0; r < W_EMPTY_DEAD_ROWS; r++) {
          map[r][c] = Wwall;
        }
      }
    }
    else {
      // Fill in walls between rooms and ladders, connecting adjacent rooms.
      blueprints->implement_rooms();
    }

    // Don't add horiz and vert movers if 
    // 1) movers turned off from command line OR
    // 2) a specialMap disallows them
    if (useMovers && !(specialMap && !specialMap->use_movers())) {
      add_movers();
    }

    blueprints->extra_walls();
    blueprints->up_downs();

    if (specialMap == NULL || specialMap->do_doors()) {
      add_doors();
    }

    add_posters();
  }

  choose_theme();
}



void World::choose_theme() {
  // To disable the unfinished MD5 theme, subtract one here, and kill 
  // W_DOORS_TRANSPARENT.

  const int ACTIVE_THEME_NUM = W_THEME_NUM;


  // Weigh choice of themes by number of backgrounds in each theme.
  int themeWeights[ACTIVE_THEME_NUM];
  for (int n = 0; n < ACTIVE_THEME_NUM; n++) {
    themeWeights[n] = themes[n].backgroundsNum;
  }

  themeIndex = Utils::weighted_choose(ACTIVE_THEME_NUM,themeWeights);

  backgroundIndex = Utils::choose(themes[themeIndex].backgroundsNum);
  // Want index into xdata.backgroundSurfaces
  backgroundIndex = themes[themeIndex].backgroundIndices[backgroundIndex];

  outsideIndex = Utils::choose(themes[themeIndex].outsidesNum);
  outsideIndex = themes[themeIndex].outsideIndices[outsideIndex];
}



MoverP World::lookup(const MoverId &id) {
  if (id.index != MoverId::INVALID &&
      id.index >= 0 &&
      id.index < moversNum &&
      movers[id.index]->get_mover_id() == id) {
    return movers[id.index];
  }
  return NULL;
}



void World::clock() {
  justReset = False;

  for (int n = 0; n < moversNum; n++) {
    movers[n]->clock();
  } // for n
}



Pos World::empty_rect(const Size &s) {
  int count = 0; // Guard against infinite loop.

  while (True) {
    assert (count < EMPTY_TRIES_MAX);
    count++;

    Pos ret;
    ret.x = Utils::choose(size.width);
    ret.y = Utils::choose(size.height);

    Area area(AR_RECT,ret,s);
    if (open(area,True,False,True)) {
      return ret;
    }
  }
}



Pos World::empty_touching_rect(const Size &s) {
  int count = 0; // Guard against infinite loop.

  while (True)
    {
      assert (count < EMPTY_TRIES_MAX);
      count++;

      Pos ret;
      ret.x = Utils::choose(size.width);
      ret.y = WSQUARE_HEIGHT * Utils::choose(dim.rowMax) - s.height;

      Area area(AR_RECT,ret,s);
      Touching touching;
      Hanging hanging;
      MoverP dummy;
      compute_touching_hanging(touching,hanging,dummy,area);
      if (open(area,True,False,True) && (touching == CO_dn))
	      return ret;
    }
}



Pos World::empty_accessible_rect(const Size &s) {
  int count = 0; // Guard against infinite loop.

  while (True) {
    assert (count < EMPTY_TRIES_MAX);
    count++;

    // Choose the room.
    RoomIndex r(Utils::choose(rooms.downMax),
                Utils::choose(rooms.acrossMax));

    Pos ret;
    if (empty_accessible_rect_one(ret,s,r)) {
      return ret;
    }
  }
}



Pos World::empty_accessible_rect(const Size &s,const RoomIndex &r) {
  int count = 0; // Guard against infinite loop.

  while (count < EMPTY_TRIES_MAX) {
    Pos ret;
    // Use supplied room.
    if (empty_accessible_rect_one(ret,s,r)) {
      return ret;
    }
    count++;
  }

  // Fuck it.
  // Start at left edge of room.
  Pos ret;
  assert(blueprints);
  ret.x = r.across * W_ROOM_COL_MAX * WSQUARE_WIDTH;
  // y coord so that area is just touching middle floor.
  ret.y = (r.down * W_ROOM_ROW_MAX + blueprints->get_middle_row(r.down))
           * WSQUARE_HEIGHT - s.height;
  // Should make helper function to avoid code dup in this and 
  // empty_accessible_rect_one().
  return ret;
}



Boolean World::empty_accessible_rect_one(Pos &ret,const Size &s,
					                               const RoomIndex &r) {
  Blueprints *bl = blueprints;
  assert(bl);

  // Start at right edge of room.
  ret.x = r.across * W_ROOM_COL_MAX * WSQUARE_WIDTH;

  // Choose x coord so that on right or left side of ladder, whereever
  // there is a floor.
  if (bl->blocked_right(r)) {
    if (bl->blocked_left(r)) {
      // Bail out, no suitable rect.
      return False;
    }
    else {
      // Left half of room.
      ret.x += Utils::choose(WSQUARE_WIDTH *
			           bl->get_middle_col(r.across));
    }
  }
  else {
    if (bl->blocked_left(r)) {
      // Right half of room.
      ret.x +=
	      WSQUARE_WIDTH *
	      (bl->get_middle_col(r.across) + objectDimMax.colMax) +
	      Utils::choose(WSQUARE_WIDTH *
		      (W_ROOM_COL_MAX -
		       bl->get_middle_col(r.across) -
		       objectDimMax.colMax));
    }
    else {
      if (Utils::coin_flip()) {
	      // Left half of room.
	      ret.x += Utils::choose(WSQUARE_WIDTH *
			       bl->get_middle_col(r.across));
      }
      else {
	      // Right half of room.
	      ret.x +=
	        WSQUARE_WIDTH *
	        (bl->get_middle_col(r.across) + objectDimMax.colMax) +
	        Utils::choose(WSQUARE_WIDTH *
			      (W_ROOM_COL_MAX -
			       bl->get_middle_col(r.across) -
			       objectDimMax.colMax));
      }
    }
  }

  // y coord so that area is just touching middle floor.
  ret.y = (r.down * W_ROOM_ROW_MAX + bl->get_middle_row(r.down))
    * WSQUARE_HEIGHT - s.height;

  Area area(AR_RECT,ret,s);
  if (open(area,True,False,True)) {
    return True;
  }
  else {
    return False;
  }
}



Boolean World::empty_box(Loc &loc,const Dim &d,Boolean laddersClosed,
			                   Boolean postersClosed,Boolean doorsClosed) {
  int count = 0; // Guard against infinite loop.

  while (True) {
    if (count >= EMPTY_TRIES_MAX) {
      return False;
    }
    count++;

    loc.c = Utils::choose(dim.colMax);
    loc.r = Utils::choose(dim.rowMax);

    Box box(loc,d);
    if (open(box,laddersClosed,postersClosed,doorsClosed)) {
      return True;
    }
  }
}



Boolean World::empty_touching_box(Loc &loc,const Dim &d,
				                          Boolean laddersClosed,
				                          Boolean postersClosed,
				                          Boolean doorsClosed) {
  int count = 0; // Guard against infinite loop.

  while (True) {
    if (count >= EMPTY_TRIES_MAX) {
      return False;
    }
    count++;

    loc.c = Utils::choose(dim.colMax);
    loc.r = Utils::choose(dim.rowMax);

    Area area(AR_RECT,loc,d);
    Touching touching;
    Hanging hanging;
    MoverP dummy;
    compute_touching_hanging(touching,hanging,dummy,area);
    if (open(area,laddersClosed,postersClosed,doorsClosed) &&
	      touching == CO_dn) {
	    return True;
    }
  }
}



void World::rooms_dim_size_update(const Dim *newDim) {
  if (newDim) {
    // Use newDim to set dim, rooms, and size.
    dim = *newDim;
    // Round rooms up.
    rooms.downMax = (int)ceil((float)dim.rowMax / (float)W_ROOM_ROW_MAX);
    rooms.acrossMax = (int)ceil((float)dim.colMax / (float)W_ROOM_COL_MAX);
  }  
  else {
    // Use this.rooms to set dim and size.
    dim.colMax = rooms.acrossMax * W_ROOM_COL_MAX;
    dim.rowMax = rooms.downMax * W_ROOM_ROW_MAX;
  }
  size.width = dim.colMax * WSQUARE_WIDTH;
  size.height = dim.rowMax * WSQUARE_HEIGHT;
}



void World::th_helper(
    // Return values.
    Touching &touching,
    Hanging &hanging,
    MoverP &touchingMover,

    const TouchingListItem &tItem,
    const TouchingListItem &eItem,  // For getting potential Movers.
    Boolean r_c, // row/column
    const Area &area,
    int coord,int length,
    int wsquare_length,
    Touching iftouching,
    Corner ifsmall, Corner iflarge) {

  // Compute touching.
  Boolean touchingOk = False;

  // Get list of all movers touching area on the given side.
  MoverP touchingMovers[MOVERS_MAX];
  int touchingMoversNum = 0;
  int n;
  for (n = 0; n < eItem.num; n++) {
    const Loc &loc = eItem.list[n];
    if (inside(loc) && unionSquares[loc.r][loc.c] &&
        unionSquares[loc.r][loc.c]->type == UN_MOVER &&
        area.touches(unionSquares[loc.r][loc.c]->mSquare.mover->get_area(),
                     iftouching)) {

      Boolean alreadyFound = False;
      for (int m = 0; m < touchingMoversNum; m++) {
        if (touchingMovers[m] == unionSquares[loc.r][loc.c]->mSquare.mover) {
          alreadyFound = True;
          continue;
        }
      }
      if (!alreadyFound) {
        touchingMovers[touchingMoversNum] = 
          unionSquares[loc.r][loc.c]->mSquare.mover;
        touching = iftouching;
        // May be set again in hanging.
        touchingMover = touchingMovers[touchingMoversNum]; 
        touchingOk = True;
        touchingMoversNum++;
      }
    }
  }


  // Is a mover touching.
  if (!touchingOk) {
    // Check all Locs.
    for (n = 0; n < tItem.num; n++) {
      if (!open(tItem.list[n])) {
        touching = iftouching;
        touchingMover = NULL;
        touchingOk = True;
        continue;
      }
    }
  }
  
  
  // Only compute hanging if touching.
  if (touchingOk) {
    int hangCutoff = (int)floor(HANGING_PERCENT * length);
    
    int n;
    for (n = 0; n < tItem.num; n++) {
      if (!open(tItem.list[n])) {
        break;
      }
    }

    // tItem.list[n] is first blocked wsquare.
    if ((n < tItem.num) &&
	    ((r_c ? tItem.list[n].r:tItem.list[n].c) * wsquare_length - coord
	     >= hangCutoff)) {
      hanging.corner = ifsmall;
      hanging.loc = tItem.list[n];
      hanging.type = Hanging::LOC;
    }
    
    for (n = tItem.num - 1; n >= 0; n--) {
      if (! open(tItem.list[n])) {
        break;
      }
    }
    
    // tItem.list[n] is last blocked wsquare.
    if ((n >= 0) &&
	    (coord + length -
	     (r_c ? tItem.list[n].r + 1: tItem.list[n].c + 1) * wsquare_length
	     >= hangCutoff)) {
      hanging.corner = iflarge;
      hanging.loc = tItem.list[n];
      hanging.type = Hanging::LOC;
    }
    
    
    // Really should sort movers by their areas and then treat them similar
    // to the Locs, but fuck it.
    for (n = 0; n < touchingMoversNum; n++) {
      Pos moverPos;
      Size moverSize;
      const Area &moverArea = touchingMovers[n]->get_area();
      moverArea.get_rect(moverPos,moverSize);
      if ((r_c ? moverPos.y : moverPos.x) - coord
          >= hangCutoff) {
        hanging.corner = ifsmall;
        hanging.moverId = touchingMovers[n]->get_mover_id();
        hanging.type = Hanging::MOVER;
        // touchingMover = touchingMovers[n];
      }
      
      if (coord + length -
          (r_c ? moverPos.y + moverSize.height : moverPos.x + moverSize.width)
          >= hangCutoff) {
        hanging.corner = iflarge;
        hanging.moverId = touchingMovers[n]->get_mover_id();
        hanging.type = Hanging::MOVER;
        // touchingMover = touchingMovers[n];
      }
    }
  } // touchingOk
}



Boolean World::check_wrap_around(Size &offset,const Area &area) {
  // Only consider wrapping-around if the slot is completely open
  // except for being partially outside the world.  So, pass in False
  // for the outsideClosed argument of World::open(). */
  if (!open(area,False,False,False,False)) {
    return False;
  }

  Pos arPos;
  Size arSize;
  area.get_rect(arPos,arSize);

  // Can't have any objects that are as big as the world.
  assert(arSize.width < size.width);

  // Go off left side of screen, appear flush against right wall.
  if (arPos.x < 0) {
    offset.width = size.width - arSize.width - arPos.x;
    offset.height = 0;

    // Only do wrap-around if the new position is open.
    Area newArea = area + offset;
    return open(newArea);
  }

  // Go off right side of screen, appear flush against left wall.
  if (arPos.x + arSize.width > size.width) {
    offset.width = -arPos.x;
    offset.height = 0;

    // Only do wrap-around if the new position is open.
    Area newArea = area + offset;
    return open(newArea);
  }

  return False;
}



Boolean World::open_iter(Area &area,int &changed,Dir dir) {
  int offOne = 0;

  Loc list[AR_WSQUARES_MAX];
  int nsquares;
  area.wsquares(list,nsquares);
  for (int n = 0; n < nsquares; n++) {
    // Support for up_downs.
    if (inside(list[n]) && (map[list[n].r][list[n].c] == WupDown) &&
	      ((dir == CO_R) || (dir == CO_L))) {
	    return False;
    }

    // Avoid closed block.
    if (! open(list[n])) {
	    int temp = area.avoid_wsquare_dir(list[n],dir);
	    offOne = Utils::maximum(offOne,temp);
    }

    // Look at mover corresponding to moverSquare at list[n].
    if (inside(list[n])) {
      UnionSquare *uSquare = unionSquares[list[n].r][list[n].c];
      if (uSquare && uSquare->type == UN_MOVER) {
				const Area &moverArea = uSquare->mSquare.mover->get_area();
				if (moverArea.overlap(area)) {
            // Movers always shift you up.
            if (dir != CO_UP) {
              return False;
            }

  	        int temp = area.avoid_area_dir(uSquare->mSquare.mover->get_area(),dir);
	          offOne = max(offOne,temp);
				}
      }
    }
  }
  if (offOne == 0) {
    return True;
  }

  changed += offOne;
  area.shift(open_size(offOne,dir));
  return False;
}



Size World::open_size(int offset,Dir dir) {
  Size ret;
  ret.width = ret.height = 0;

  switch (dir) {
  case CO_R:
    ret.width = offset;
    break;
  case CO_DN:
    ret.height = offset;
    break;
  case CO_L:
    ret.width = -offset;
    break;
  case CO_UP:
    ret.height = -offset;
    break;
  };

  return ret;
}



Size World::open_size(int offset1,Dir dir1,int offset2,Dir dir2) {
  Size ret;
  ret.width = ret.height = 0;

  switch (dir1) {
  case CO_R:
    ret.width = offset1;
    break;
  case CO_DN:
    ret.height = offset1;
    break;
  case CO_L:
    ret.width = -offset1;
    break;
  case CO_UP:
    ret.height = -offset1;
    break;
  };

  switch (dir2) {
  case CO_R:
    ret.width = offset2;
    break;
  case CO_DN:
    ret.height = offset2;
    break;
  case CO_L:
    ret.width = -offset2;
    break;
  case CO_UP:
    ret.height = -offset2;
    break;
  };

  return ret;
}



Boolean World::open_try_dirs(Size &offset,const Area &area,const Dir dirs[4],
                             int dirsNum) {
  if (!dirsNum) {
    return False;
  }

  // Trying several directions simultaneously.
  Area areas[4];
  int offsets[4];
  for (int n = 0; n < dirsNum; n++) {
    areas[n] = area;
    offsets[n] = 0;
  }

  for (int iter = 0; iter < OPEN_ITERATIONS; iter++) {
    Boolean oks[4];
    int n;
    for (n = 0; n < dirsNum; n++) {
	    oks[n] = open_iter(areas[n],offsets[n],dirs[n]);
    }

    int offsetOkMin = Utils::minimum(offsets,oks,dirsNum);

    // Return after first iteration that gives an open area in one of the directions.
    for (n = 0; n < dirsNum; n++) {
	    if (oks[n] && (offsets[n] == offsetOkMin)) {
	      offset = open_size(offsets[n],dirs[n]);
	      return True;
	    }
    }
  }

  return False;
}



Boolean World::open_try_corners(Size &offset,const Area &area) {
	Dir corners[4];
	corners[0] = CO_DN_R;
	corners[1] = CO_DN_L;
	corners[2] = CO_UP_L;
	corners[3] = CO_UP_R;

	// Try all four corners.
	for (int cNum = 0; cNum < 4; cNum++) {
		Loc outer[3];
		Loc inner;
		if (area.corner_offset(offset,outer,inner,corners[cNum])) {
			// Check that all 3 outer wsquares are closed.
			Boolean validCorner = True;
			for (int n = 0; n < 3; n++) {
				if (open(outer[n])) {
					validCorner = False;
					break;
				}
			}
			// Check that the inner wsquare of the corner is open.
			if (!open(inner)) {
				validCorner = False;
      }

			if (validCorner) {
				// offset is already set.
				return True;
      }
		}
	}

	return False;
}



Boolean World::open_try_diagonals(Size &offset,const Area &area) {
  Dir dirs[4];
  dirs[0] = CO_R;
  dirs[1] = CO_DN;
  dirs[2] = CO_L;
  dirs[3] = CO_UP;

  int offsets[4];
  int n;
  for (n = 0; n < 4; n++) {
    offsets[n] = 0;
  }

  Loc list[AR_WSQUARES_MAX];
  int nsquares;
  area.wsquares(list,nsquares);
  for (n = 0; n < nsquares; n++) {
    if (! open(list[n])) {
      for (int dIndex = 0; dIndex < 4; dIndex++) {
        int dOffset = area.avoid_wsquare_dir(list[n],dirs[dIndex]);
        offsets[dIndex] = Utils::maximum(dOffset,offsets[dIndex]);
      }
    }

    // Look at mover corresponding to moverSquare at list[n].
    if (inside(list[n])) {
      UnionSquare *uSquare = unionSquares[list[n].r][list[n].c];
      if (uSquare && uSquare->type == UN_MOVER) {
        const Area &moverArea = uSquare->mSquare.mover->get_area();
        if (moverArea.overlap(area)) {
          for (int dIndex = 0; dIndex < 4; dIndex++) {
            int dOffset = 
              area.avoid_area_dir(uSquare->mSquare.mover->get_area(),
                                  dirs[dIndex]);
            offsets[dIndex] = Utils::maximum(dOffset,offsets[dIndex]);
          }
        }
      }
    }
  } // if
  
  for (int dIndex = 0; dIndex < 4; dIndex++) {
    Area areaTest = area;
    offset = open_size(offsets[dIndex],dirs[dIndex],
                       offsets[(dIndex + 1) % 4],dirs[(dIndex + 1) % 4]);
    areaTest.shift(offset);
    if (open(areaTest)) {
      return True;
    }
  }
  
  return False;
}



void World::add_posters() {
  // Put in posters.
  for (int n = 0;
       n < POSTERS_ACTUAL_PERCENT * dim.rowMax * dim.colMax;
       n++) {
    // Choose poster appropriate for the current theme.
    int p = Utils::choose(themes[themeIndex].postersNum);
    p = themes[themeIndex].posterIndices[p];

    Loc pLoc;
    if (!empty_box(pLoc,posterDims[p],True,True,True)) {
	    break;
    }

    Loc loc;
    for (loc.c = pLoc.c; loc.c < pLoc.c + posterDims[p].colMax; loc.c++) {
      for (loc.r = pLoc.r; loc.r < pLoc.r + posterDims[p].rowMax; loc.r++) {
        if (! unionSquares[loc.r][loc.c]) {
          unionSquares[loc.r][loc.c] = new UnionSquare;
        }
        assert(unionSquares[loc.r][loc.c]);
        unionSquares[loc.r][loc.c]->type = UN_POSTER;
        unionSquares[loc.r][loc.c]->pSquare.poster = p;
        unionSquares[loc.r][loc.c]->pSquare.loc.c = loc.c - pLoc.c;
        unionSquares[loc.r][loc.c]->pSquare.loc.r = loc.r - pLoc.r;
  	  }
    }
  }
}



void World::add_doors() {
  // Put in doors.
  for (int n = 0;
       n < DOORS_ACTUAL_PERCENT * rooms.downMax * rooms.acrossMax;
       n++) {
    Loc loc1;
    Loc loc2;
    Dim doorDim(2,1);  // Hardwired size of doors.
    if (!empty_touching_box(loc1,doorDim,True,True,True)) {
      break;
    }
    if (!empty_touching_box(loc2,doorDim,True,True,True)) {
      break;
    }

    // Don't want doors on top of each other.
    Area area1(AR_RECT,loc1,doorDim);
    Area area2(AR_RECT,loc2,doorDim);
    if (area1.overlap(area2)) {
      break;
    }

    // Lower half of doors.
    Loc loc1d = loc1.move(1,0);
    Loc loc2d = loc2.move(1,0);

    // We actually create 4 doors, two pairs.  Each apparent door is
    // actually two DoorSquares.

    // Cross connect two new doors at loc1 and loc2.
    unionSquares[loc1.r][loc1.c] = new UnionSquare;
    assert(unionSquares[loc1.r][loc1.c]);
    unionSquares[loc1.r][loc1.c]->type = UN_DOOR;
    unionSquares[loc1.r][loc1.c]->dSquare.dest = loc2;
    unionSquares[loc1.r][loc1.c]->dSquare.topBottom = W_DOOR_TOP;

    unionSquares[loc2.r][loc2.c] = new UnionSquare;
    assert(unionSquares[loc2.r][loc2.c]);
    unionSquares[loc2.r][loc2.c]->type = UN_DOOR;
    unionSquares[loc2.r][loc2.c]->dSquare.dest = loc1;
    unionSquares[loc2.r][loc2.c]->dSquare.topBottom = W_DOOR_TOP;

    // Cross connect two new doors at loc1d and loc2d.
    unionSquares[loc1d.r][loc1d.c] = new UnionSquare;
    assert(unionSquares[loc1d.r][loc1d.c]);
    unionSquares[loc1d.r][loc1d.c]->type = UN_DOOR;
    unionSquares[loc1d.r][loc1d.c]->dSquare.dest = loc2d;
    unionSquares[loc1d.r][loc1d.c]->dSquare.topBottom = W_DOOR_BOTTOM;

    unionSquares[loc2d.r][loc2d.c] = new UnionSquare;
    assert(unionSquares[loc2d.r][loc2d.c]);
    unionSquares[loc2d.r][loc2d.c]->type = UN_DOOR;
    unionSquares[loc2d.r][loc2d.c]->dSquare.dest = loc1d;
    unionSquares[loc2d.r][loc2d.c]->dSquare.topBottom = W_DOOR_BOTTOM;
  } // for
}



void World::add_movers() {
  assert(moversNum <= MOVERS_MAX && blueprints);
  // Movers shouldn't be too big.
  assert(objectDimMax.colMax * WSQUARE_WIDTH >= moverSize.width);

  // Number of movers to create.
  const int moversActual = Utils::choose((int)(rooms.acrossMax * rooms.downMax
			                                   * MOVERS_PERCENT) + 1);

  for (int n = 0; n < moversActual && moversNum < MOVERS_MAX; n++) {
    Boolean which = Utils::coin_flip();
    Boolean ok = False;
    int tries = 0;
    while (!ok && tries < MOVER_TRIES) {
      // Decide between horizontal and vertical mover.
      if (which) {
        ok = add_vert_mover();
      }
      else {
        ok = add_horiz_mover();
      }

      tries++;
    }
  }
}



Boolean World::add_horiz_mover() {
  assert(moversNum <= MOVERS_MAX && blueprints);
  // Movers shouldn't be too big.
  assert(objectDimMax.colMax * WSQUARE_WIDTH >= moverSize.width);


  // loc will walk right or left, filling in track.
  Loc loc;
  loc.r = Utils::choose(dim.rowMax);
  loc.c = Utils::choose(dim.colMax);

  // Check space surrounding initial choice for minimum track length.
  int delta = Utils::coin_flip() ? 1 : -1;  // right or left.
  Loc check;
  for (check.c = loc.c - (delta == -1 ? MOVERS_HORIZ_MIN_TRACK : 1) * objectDimMax.colMax;
	 check.c < loc.c + (delta == 1 ? MOVERS_HORIZ_MIN_TRACK + 1 : 2) * objectDimMax.colMax; // mover track plus blank space
	 check.c++) {
	  for (check.r = loc.r - objectDimMax.rowMax;
	       check.r <= loc.r + objectDimMax.rowMax;
	       check.r++) {
	    if (!open(check,True,True,True)) {
        return False;
      }
    }
  }


  // Create mover
  movers[moversNum] = new Mover;
  assert(movers[moversNum]);

  // Start at the right edge of the mover and go left
  if (delta == -1) {
    loc.c += objectDimMax.colMax - 1;
  }
  // else we are at the left edge going right.


  // minimum and maximum values of left side of mover, inclusive
  int mLeft = loc.c;
  int mRight = loc.c;

  // Go right/left until hit a wall or decide to stop.
  // Must do at least MOVERS_HORIZ_MIN_TRACK multiples of the mover width.
  int n = 0;
  Boolean ok = True;
  while (ok && (n <= MOVERS_HORIZ_MIN_TRACK * objectDimMax.colMax || 
                Utils::choose(MOVERS_HORIZ_TRACK_LENGTH))) {
    // Check if we can put a mover at loc.
	  for (check.c = loc.c;
        check.c != loc.c + delta * objectDimMax.colMax;
        check.c += delta) {
      for (check.r = loc.r - objectDimMax.rowMax;
       check.r <= loc.r + objectDimMax.rowMax;
       check.r++) {
	      if (!open(check,True,True,True)) {
		      ok = False;
          // Our above check of the surrounding space was not correct.
          assert(n >= MOVERS_HORIZ_MIN_TRACK * objectDimMax.colMax);
        }
        if (!ok) {
          break;
        }
      }
      if (!ok) {
        break;
      }
    }
                
    // Add mover track at loc.
    if (ok) {
      // Add a mover square.
      assert(inside(loc) && 
             (map[loc.r][loc.c] == Wempty ) &&
             !unionSquares[loc.r][loc.c]);
      unionSquares[loc.r][loc.c] = new UnionSquare;
      assert(unionSquares[loc.r][loc.c]);
      unionSquares[loc.r][loc.c]->type = UN_MOVER;
      unionSquares[loc.r][loc.c]->mSquare.mover = movers[moversNum];
      unionSquares[loc.r][loc.c]->mSquare.orientation = OR_HORIZ;

      mLeft = Utils::minimum(mLeft,loc.c);
      mRight = Utils::maximum(mRight,loc.c);
      n++;

      loc.c += delta;
    }
  } // while ok

  // Check again that we got enough track.
  assert(mRight - mLeft + 1 >= MOVERS_HORIZ_MIN_TRACK * objectDimMax.colMax);


  // initialize mover.
  // Choose X position somewhere in range of mover squares.
  int rangeX = (mRight - mLeft + 1 - objectDimMax.colMax) * WSQUARE_WIDTH + 1;
  assert(rangeX > 0); // Or not enough room.
  int startX = Utils::choose(rangeX) + mLeft * WSQUARE_WIDTH; 

  Pos pos(startX,loc.r * WSQUARE_HEIGHT);
  Area area(AR_RECT,pos,moverSize);

  Size vel;
  vel.set(Utils::coin_flip() ? W_MOVER_SPEED : -W_MOVER_SPEED,0);
  // initialize the mover.
  MoverId moverId;
  moverId.index = moversNum;
  moverId.unique = uniqueGen++;

  movers[moversNum]->init(this,locator,area,vel,moverId);

  // Dummy mover object, so mover gets redrawn.
  PhysMoverP p = new PhysMover(this,locator,movers[moversNum]);
  assert(p);
  locator->add(p);

  movers[moversNum]->set_phys_mover_id(p->get_id());

  moversNum++;
  return True;
}



// What an ugly function.  I should redo this.
Boolean World::add_vert_mover() {
  assert(moversNum <= MOVERS_MAX && blueprints);
  // Movers shouldn't be too big.
  assert(objectDimMax.colMax * WSQUARE_WIDTH >= moverSize.width);


  // Choose a random room, check if its middle is a ladder, if so, change
  // the ladder to a bunch of mover squares and create a new mover.
  // Else continue.
  Loc init;
  int m; // holder for the down/across value of the room chosen.
  m = Utils::choose(rooms.acrossMax);
  init.c =  m * W_ROOM_COL_MAX + blueprints->get_middle_col(m);
  m = Utils::choose(rooms.downMax);
  init.r =  m * W_ROOM_ROW_MAX + blueprints->get_middle_row(m);


  // Found a ladder, create a new mover and set all the ladder wsquares to be
  // MoverSquares pointing to the new mover.
  if (!(map[init.r][init.c] == Wladder && 
        map[init.r][init.c+1] == Wladder)) {
    return False;
  } 

  assert(!unionSquares[init.r][init.c]);

  // All wsquares between mTop and mBottom, exclusive, are part of the
  // mover.
  int mTop = init.r;
  int mBottom = init.r;

  movers[moversNum] = new Mover;
  assert(movers[moversNum]);

  // delta is -1, then 1.  Go up, then down..
  for (int delta = -1; delta <= 1; delta += 2) {
    Loc loc = init;

    // To avoid hitting init twice, skip it when going down.
    if (delta == 1) {
      loc.r++;
    }

    // Scan up/down depending on value of delta.
    Boolean bothInside = inside(loc) && inside(loc.r,loc.c+1);
    Boolean bothLadders = bothInside && 
      (map[loc.r][loc.c] == Wladder && map[loc.r][loc.c+1] == Wladder);
    Boolean aboveInside = inside(loc.r-1,loc.c) && inside(loc.r-1,loc.c+1);
    Boolean aboveMoverSq = aboveInside && 
      map[loc.r-1][loc.c] == Wempty && 
      map[loc.r-1][loc.c+1] == Wempty &&
      unionSquares[loc.r-1][loc.c] && 
      unionSquares[loc.r-1][loc.c+1] &&
      unionSquares[loc.r-1][loc.c]->type == UN_MOVER && 
      unionSquares[loc.r-1][loc.c+1]->type == UN_MOVER;
    Boolean bothWalls = bothInside && 
      (map[loc.r][loc.c] == Wwall && map[loc.r][loc.c+1] == Wwall);
    // This shit is all so that movers can stick down one square into floor.
    // CAREFUL!! Crappy code duplicated below.

    while (bothLadders ||  // normal case 
           (delta == 1 && aboveMoverSq && bothWalls) ) { // sticking down one
      
      // Scan to right.
      for (; loc.c < init.c + objectDimMax.colMax; loc.c++) {
        // Add a mover square.
        assert(inside(loc) && 
               (map[loc.r][loc.c] == Wladder || map[loc.r][loc.c] == Wwall) &&
               !unionSquares[loc.r][loc.c]);
        unionSquares[loc.r][loc.c] = new UnionSquare;
        assert(unionSquares[loc.r][loc.c]);
        unionSquares[loc.r][loc.c]->type = UN_MOVER;
        unionSquares[loc.r][loc.c]->mSquare.mover = movers[moversNum];
        unionSquares[loc.r][loc.c]->mSquare.orientation = OR_VERT;

        // Still leave overlap of wall and moversquare.
        if (map[loc.r][loc.c] != Wwall) {
          map[loc.r][loc.c] = Wempty;
        }

        mTop = Utils::minimum(mTop,loc.r);
        mBottom = Utils::maximum(mBottom,loc.r + 1);
    	}
	    loc.c = init.c;

      // When up at the top.
      // Remove annoying ladder sticking up.
      if (delta ==  -1 &&
          map[loc.r - objectDimMax.rowMax - 1][loc.c] != Wladder &&
          // Just to be sure we don't kill of some unionSquares.
          !unionSquares[loc.r - objectDimMax.rowMax - 1][loc.c]) {
        int locTop = loc.r - objectDimMax.rowMax;
        for (loc.r--; loc.r >= locTop; loc.r--) {
          for (loc.c = init.c; loc.c < init.c + objectDimMax.colMax; 
               loc.c++) {
            map[loc.r][loc.c] = Wempty;
          }
        }
        
        break;
      } // annoying ladder sticking up.
      
      loc.c = init.c;
      loc.r += delta;

      
      bothInside = inside(loc) && inside(loc.r,loc.c+1);
      bothLadders = bothInside && 
        (map[loc.r][loc.c] == Wladder && map[loc.r][loc.c+1] == Wladder);
      aboveInside = inside(loc.r-1,loc.c) && inside(loc.r-1,loc.c+1);
      aboveMoverSq = aboveInside && 
        map[loc.r-1][loc.c] == Wempty && 
        map[loc.r-1][loc.c+1] == Wempty &&
        unionSquares[loc.r-1][loc.c] && 
        unionSquares[loc.r-1][loc.c+1] &&
        unionSquares[loc.r-1][loc.c]->type == UN_MOVER && 
        unionSquares[loc.r-1][loc.c+1]->type == UN_MOVER;
      bothWalls = bothInside && 
        (map[loc.r][loc.c] == Wwall && map[loc.r][loc.c+1] == Wwall);
      // This shit is all so that movers can stick down one square into 
      // the floor.

    } // while
  } // for delta
  // Note: loc is now at the left side of the former ladder,
  // one square beneath it.

  assert(mTop < mBottom);  // Or no squares were added.

  // Choose Y position somewhere in range of mover squares.
  int rangeY = (mBottom - mTop) * WSQUARE_HEIGHT - moverSize.height + 1;
  assert(rangeY > 0); // or movers are more than one square high.
  int startY = Utils::choose(rangeY) + mTop * WSQUARE_HEIGHT;

  Pos pos(init.c * WSQUARE_WIDTH,startY);
  Area area(AR_RECT,pos,moverSize);

  Size vel;
  vel.set(0,Utils::coin_flip() ? W_MOVER_SPEED : -W_MOVER_SPEED);
  // initialize the mover.
  MoverId moverId;
  moverId.index = moversNum;
  moverId.unique = uniqueGen++;

  movers[moversNum]->init(this,locator,area,vel,moverId);

  // Dummy mover object, so mover gets redrawn.
  PhysMoverP p = new PhysMover(this,locator,movers[moversNum]);
  assert(p);
  locator->add(p);

  movers[moversNum]->set_phys_mover_id(p->get_id());

  moversNum++;
  return True;
}



void World::delete_movers() {
  // The locator should take care of killing off the PhysMovers
  // in Locator::reset().

  int n;
  for (n = 0; n < moversNum; n++) {
    delete movers[n];
    movers[n] = NULL;
  }
  moversNum = 0;
}



void World::mover_list_add(MoverP list[],int &nItems,const Loc &loc) {
  if (!inside(loc)) {
    return;
  }

  UnionSquare *uSquare = unionSquares[loc.r][loc.c];
  if (uSquare && uSquare->type == UN_MOVER) {
    MoverP mover = uSquare->mSquare.mover;

    // Only add movers that overlap loc.
    const Area &area = mover->get_area();
    if (!area.overlap(loc)) {
      return;
    }

    // Check for duplicates.
    for (int n = 0; n < nItems; n++) {
      if (list[n] == mover) {
        return;
      }
    }

    // Add to list.
    assert(nItems < MOVERS_MAX);
    list[nItems] = mover;
    nItems++;
  }
}



Boolean World::useMovers = True; // On by default.

