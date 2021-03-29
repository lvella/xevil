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

// "coord.h"  Coordinates and directions and the like.

#ifndef COORD_H
#define COORD_H

#if X11
	#ifndef NO_PRAGMAS
	#pragma interface
	#endif
#endif


// Include Files
#include <cstdint>
#include "utils.h"
#include "id.h"
#include "streams.h"


// Defines
#define WSQUARE_WIDTH 16
#define WSQUARE_HEIGHT 16
#define WSQUARE_WIDTH_INV (1.0 / WSQUARE_WIDTH)
#define WSQUARE_HEIGHT_INV (1.0 / WSQUARE_HEIGHT)


// Hack, needed by ui.h
#define R_NAME_MAX IT_STRING_LENGTH


// Used for defining the name of bitmap files/resource ids in the .bitmaps 
// files.
#if X11
#define CMN_ID(_unix___,_win32___) ((char*)_unix___)
#endif
#if WIN32
#define CMN_ID(_unix___,_win32___) _win32___
#endif


#define W_BLOCKS_NUM 7

#define W_THEME_NUM 7


// Ick. Only here because needed by xdata.h on UNIX.
//
// The total number of all blocks of all types in all themes.
#define W_ALL_BLOCKS_NUM 25
// All the possible backgrounds for all themes.
#define W_ALL_BACKGROUNDS_NUM 15
// All the outside pixmaps for all themes.
#define W_ALL_OUTSIDES_NUM 7
// twice the number of different doors, e.g. top and bottom
#define W_ALL_DOORS_NUM 12
// twice the number of different types, e.g. OR_VERT and OR_HORIZ
#define W_ALL_MOVER_SQUARES_NUM 10
// the number of movers, each takes up 2 squares.
#define W_ALL_MOVERS_NUM 5
// All the possible posters for all themes, and the title poster.
#define W_ALL_POSTERS_NUM 14


// Do doors use the transparent color.
#define W_DOORS_TRANSPARENT


// Still used by x11/xdata.h, purged from win32/xdata.h, though.
#define PH_ANIM_MAX 10

#define PH_AMMO_UNLIMITED -1

#define UI_ARENA_MESSAGE_TIME 80 // 60

// The number of humans may change during the game, ie. client-server game.
#define HU_UNSPECIFIED -1


// Order of directions IS guaranteed.
#define CO_center_R 0
#define CO_center_L 1
#define CO_air_R 2
#define CO_air_L 3
#define CO_air 4
#define CO_center 5

#define CO_r 6
#define CO_r_DN 7 
#define CO_r_UP 8
#define CO_dn 9
#define CO_dn_R 10
#define CO_dn_L 11
#define CO_l 12
#define CO_l_DN 13
#define CO_l_UP 14
#define CO_up 15
#define CO_up_R 16
#define CO_up_L 17

#define CO_climb 18
#define CO_climb_DN 19
#define CO_climb_UP 20
#define CO_air_UP 21
#define CO_air_DN 22
#define CO_climb_R 23
#define CO_climb_L 24

#define CO_R 25
#define CO_DN_R_R 26
#define CO_DN_R 27
#define CO_DN_DN_R 28
#define CO_DN 29
#define CO_DN_DN_L 30
#define CO_DN_L 31
#define CO_DN_L_L 32
#define CO_L 33
#define CO_UP_L_L 34
#define CO_UP_L 35
#define CO_UP_UP_L 36
#define CO_UP 37
#define CO_UP_UP_R 38
#define CO_UP_R 39
#define CO_UP_R_R 40

#define CO_DIR_MAX 41

#define CO_DIR_PURE 16  // "Pure" does not include CO_center.  Use CO_air.
#define CO_DIR_HALF_PURE 8

typedef long int ClassId;
// Possible values for a ClassId.  There should be an entry for every class
// of Physical objects in the game.
// Also used as an index into Locator::contexts.
// Don't forget to register the class in Locator::register_classes().
//
// Note that A_CLASSES_NUM is public and accessible by anyone.
//
enum {
  A_Explosion,A_Fire,A_FireExplosion,
  A_NProtection,A_TProtection,A_XProtection,
  A_Trapdoor,A_Home,
  A_Shell,A_SwapShell,A_Lance,A_Laser,A_FrogShell,A_Fireball,A_Missile,
  A_Star,
  A_Blood,A_GreenBlood,A_OilDroplet,A_Feather,
  A_Grenade,A_Napalm,A_Egg,
  A_Xit,A_Flag,
  A_Rock,A_Weight,A_AltarOfSin,
  // Items
  A_Doppel,A_Cloak,A_Transmogifier,A_MedKit,A_CrackPipe,A_Caffine,A_PCP,
  A_NShield,A_TShield,A_Bomb,
  // Weapons
  A_Chainsaw,A_Pistol,A_MGun,A_Lancer,A_AutoLancer,A_FThrower,A_Launcher,
  A_Grenades,A_Napalms,A_Stars,A_Swapper,A_FrogGun,A_DogWhistle,
  A_DemonSummoner,
  // Creatures
  A_Enforcer,A_Frog,A_Hero,A_Zombie,A_Ninja,A_Alien,A_RedHugger,A_GreenHugger,
  A_ChopperBoy,A_Seal,
  A_FireDemon,A_Dragon,A_Walker,
  A_Dog,A_Mutt,
  A_Yeti,A_Chicken,
  // Misc
  A_PhysMover,

  // The number of "real" classes.
  A_CLASSES_NUM,

  // Identifiers for non-existent classes.  Used for the weapon/item
  // display in the Ui on Win32.
  // Add to Locator::nonExistentClassesItemInfo if you add new ones.
  A_Fireballs = A_CLASSES_NUM, 
  A_Lasers,
  A_DropLiftable,
  A_HandToHand,
  A_SuicideButton,
  A_Prickly, // The attack that the Prickly class uses.

  // Must be last.
  A_None 
}; 


// AbilityId
typedef int AbilityId;
enum {
  AB_Grounded = 0,
  AB_Suicide,
  AB_Hopping,
  AB_User,
  AB_Carrier,
  AB_Fighter,
  AB_Walking,	
  AB_Sticky,
  AB_Flying,
  AB_BuiltIn,
  AB_Hugger,
  AB_Prickly,
  AB_Healing,
  AB_Lifter,
  AB_Morphed,
  AB_AnimTime,
  AB_OnFire,
  AB_SwapProtect,
  AB_Segmented,
  AB_Sensitive,
  
  // WARNING, The Creature IO code assumes there are <= 32 abilities.  
  // Uses a bit field to write out.
  AB_MAX,
};
typedef u_int ABBitField;


typedef int ModifierId;
enum {M_Crack,M_CaffineM,M_PCPM,M_Alcohol,M_Heroin,
      M_DoubleSpeed,M_DoubleJump,M_DoubleHealth,
      M_LifterSpeed,
      M_None};



typedef int Capability;
typedef int Dir;
typedef int Touching;
typedef int Stance;
typedef int Corner;
typedef int Mass;
typedef int Health;
typedef int Frame;
typedef int Speed;
typedef int Quanta;
typedef u_int Turn;

typedef int TeamId;


enum RoleType {R_STAND_ALONE, R_CLIENT, R_SERVER, R_NONE};
// R_NONE is just a placeholder.  Ui uses it to indicate that
// role hasn't been set yet.



// Cross-platform sound.
typedef int SoundName;



class Area;



// Moved from intel.h so that xdata.h can get IT_WEAPON_R.
//
// Possible commands to be given to objects.
// Order of directions is guaranteed.  (See ui.h and game.cpp)
enum {
  IT_CENTER,
  IT_R, IT_DN_R, IT_DN, IT_DN_L,
  IT_L, IT_UP_L, IT_UP, IT_UP_R,
  IT_WEAPON_CENTER,IT_WEAPON_CHANGE,IT_WEAPON_DROP,
  IT_ITEM_USE,IT_ITEM_CHANGE,IT_ITEM_DROP,
  IT_CHAT,
  // The above are the possible values for user keys in the Ui, 
  // i.e. < UI_KEYS_MAX.

  IT_WEAPON_R,IT_WEAPON_DN_R,IT_WEAPON_DN,IT_WEAPON_DN_L,
  IT_WEAPON_L,IT_WEAPON_UP_L,IT_WEAPON_UP,IT_WEAPON_UP_R,
  
  IT_NO_COMMAND
};
typedef int ITcommand;
#define IT_COMMAND_MAX IT_NO_COMMAND



class Stats {
 public:
  Stats() {creations = 0; uses = 0; deaths = 0; aveLifespan = 0.0f;}

  long get_creations() const {return creations;}
  long get_uses() const {return uses;}
  long get_deaths() const {return deaths;}

  float get_ave_lifespan() const {return aveLifespan;}
  /* NOTE: Value returned is in seconds. */

  void add_creation() { if (enabled) creations++;}
  void add_use() {if (enabled) uses++;}
  void add_death(time_t birthTime);
  
  static void enable() {enabled = True;}
  /* EFFECTS: Stats will only be recorded after the call to enable(). */
  /* NOTE: Used so that stats will not be recorded during the demo screen. */

 private:
  long creations;
  long uses; // use, explosion.
  long deaths;
  float aveLifespan; // Valid iff deaths > 0.

  Boolean static enabled;
};



struct Pos {
  Pos() {x = 0; y = 0;}
  Pos(int xx,int yy) {x = xx; y = yy;}
  Pos(InStreamP in) {read(in);}

  void read(InStreamP in);
  static int get_write_length();
  void write(OutStreamP out) const;

  int distance(const Pos &) const;
  /* EFFECTS: Returns distance between two points. */

  int distance_2(const Pos&) const;
  /* EFFECTS: Returns square of distance between two points. */

  void set_zero() {x = y = 0;}

  int x; int y;
}; // In pixels



struct Vel {
  Vel() {dx = 0.0f; dy = 0.0f;}
  Vel(float x,float y) {dx = x; dy = y;}
  void set_zero() {dx = 0.0f; dy = 0.0f;}

  void read(InStreamP in);
  static int get_write_length();
  void write(OutStreamP out) const;

  Vel shrink(float k) const;
  void damp(float k);
  Boolean is_zero() const;
  Dir get_dir() const;
  void limit(float k);
  /* REQUIRES: k >= 0 */
  /* EFFECTS: Force dx and dy to be <= k. */

  void get_dirs_4(Dir in[4],Dir out[4],int &inNum,int &outNum);
  /* MODIFIES: in, out, inNum, outNum */
  /* EFFECTS: Partitions {CO_R, CO_DN, CO_L, CO_UP} into in and out.  inNum 
     and outNum are set to the sizes of the respective sets. */


  float dx; float dy;
};



struct Dim {
  Dim() {rowMax = colMax = 0;}
  Dim(int rm,int cm) {rowMax = rm; colMax = cm;}
  Dim(InStreamP in) {read(in);}

  void read(InStreamP in);

  static int get_write_length();

  void write(OutStreamP out) const;

  int rowMax, colMax; // In WSQUARES.
}; 



struct Size {
  const Size &operator +=(const Size &other) {width += other.width; height += other.height; return *this;}
  
  Dir get_dir();
  /* EFFECTS: Returns one of {CO_R..CO_UP_R,CO_air}. */

  void get_dirs_4(Dir &d1,Dir &d2);
  /* MODIFIES: d1, d2 */
  /* EFFECTS: Gets the two directions of {CO_R,CO_DN,CO_L,CO_UP} that 
     correspond to *this.  If there is only one, it is returned as d1 and 
     d2.  If *this has zero size, d1 == d2 == CO_air on return. */

  void set(int w,int h){width = w; height = h;}

  void set(const Dim &dim) {width = dim.colMax * WSQUARE_WIDTH;
                            height = dim.rowMax * WSQUARE_HEIGHT;}

  void read(InStreamP in);
  static int get_write_length();
  void write(OutStreamP out) const;
  /* EFFECTS: IO for writing each coordinate as a u_char. */

  void read_32(InStreamP in);
  static int get_write_length_32();
  void write_32(OutStreamP out) const;
  /* EFFECTS: IO for writing each coordinate as an int. */

  float cross(const Vel &v);
  /* EFFECTS: z component of the cross product of the size and the Vel. */

  float dot(const Vel &);
  /* EFFECTS: Dot product. */

  void set_zero() {width = height = 0;}

  int abs_2() {return width*width + height*height;}


  int width; 
  int height;
}; 



// Constructors mess with TouchingList in area.h
struct Loc {
  void read(InStreamP in);
  static int get_write_length();
  void write(OutStreamP out) const;
  
  void set(int rr,int cc) {r = rr; c = cc;}
  
  Loc move(int dr,int dc) 
  {Loc ret; ret.r = r + dr; ret.c = c + dc; return ret;}
  /* EFFECTS: Return new Loc that is *this + (dr,dc) */


  int r,c; // In WSQUARES.
}; 



struct Box {
  Box() {};
  Box(const Loc &l,const Dim &d) {loc = l; dim = d;}
  Boolean overlap(const Loc &);


  Loc loc; Dim dim; // In WSQUARES.
}; 



struct GLoc {
  int vert, horiz;
};



struct RoomIndex {
  RoomIndex() : down(0), across(0) {}
  RoomIndex(int d,int a) : down(d), across(a) {}

  RoomIndex(InStreamP in) {read(in);}
  void read(InStreamP in) 
  {down = in->read_char(); across = in->read_char();}

  static int get_write_length() {return 2 * sizeof(char);}
  void write(OutStreamP out) const 
  {assert(down <= UCHAR_MAX && across <= UCHAR_MAX);
   out->write_char((u_char)down); out->write_char((u_char)across);}


  // In rooms.
  int down; 
  int across; 
}; 



struct Rooms {
  Rooms() {downMax = acrossMax = 0;}
  Rooms(int dn,int acc) {downMax = dn; acrossMax = acc;}

  Rooms(InStreamP in) {read(in);}
  void read(InStreamP in) 
  {downMax = in->read_char(); acrossMax = in->read_char();}

  static int get_write_length() {return 2 * sizeof(char);}
  void write(OutStreamP out) const 
  {out->write_char((u_char)downMax); out->write_char((u_char)acrossMax);}


  int downMax, acrossMax;
}; 



struct Acc {
  operator Vel()
    {Vel ret(ddx,ddy); return ret;}
  /* EFFECTS: Converts from an acceleration to velocity.  Assumes initial 
     velocity is 0. */


  float ddx, ddy;
};



struct Hanging {
  Hanging() {corner = CO_air;}
  enum {LOC,MOVER};

  Corner corner;
  int type; // Hanging::LOC or Hanging::MOVER.
  
  // Only one of these two is valid.  Can't be a union because
  // MoverId has constructor.
  Loc loc; // Not meaningful if corner == CO_air.
  MoverId moverId;
};



typedef int Grav;



// For Physical::corporeal_attack.
typedef u_int AttackFlags;
// Actually do damage.  Don't use this if you just want to generate
// droplets for dramatic effect.
#define ATT_DAMAGE 0x1
// Should (blood) droplets be generated.
#define ATT_DROPLETS 0x2



// Operators on the geometric primitives
Boolean operator == (const Loc &l1, const Loc &l2);
Boolean operator == (const Pos &p1, const Pos &p2);
Boolean operator != (const Pos &p1, const Pos &p2);
Boolean operator == (const Vel &, const Vel &);
Boolean operator == (const Size &s1, const Size &s2);
Boolean operator == (const GLoc &,const GLoc &);
Boolean operator != (const GLoc &,const GLoc &);
Boolean operator == (const Dim &,const Dim &);
Boolean operator == (const Rooms &,const Rooms &);
Pos operator + (const Pos &pos,const Size &size);
Pos operator - (const Pos &pos,const Size &size);
Size operator - (const Pos &p1,const Pos &p2);
Pos operator + (const Pos &pos, const Vel &vel);
Size operator * (float k,const Size &size);
Vel operator + (const Vel &,const Vel &);
Vel operator + (const Vel &, const Acc &acc);
Vel operator + (const Vel &, const Size &size);
Vel operator * (float k,const Vel &vel);
Vel operator / (float k,const Vel &vel);
Vel operator + (float k,const Vel &vel);
Acc operator * (int k,const Acc &acc);
// Useful for adding two offsets.
Size operator + (const Size& s1,const Size& s2); 



class Coord {
public:
  static Boolean is_dir_pure(Dir dir);
  
  static Dir dir_opposite(Dir dir);
  /* REQUIRES: dir must be a pure dir. */

  static Dir movement_dir_4(Dir dir);
  /* EFFECTS: Returns a dir in {CO_air,CO_R,CO_DN,CO_L,CO_UP}.  CO_air if
     the dir corresponds to one where the object is not moving.  Otherwise, 
     one of the others. */

  static Dir parallel_dir(Stance stance);
  /* EFFECTS: Returns CO_DN or CO_R such the returned direction is parallel
     to the 
     wall corresponding to stance.  E.g. moving a creature touching a wall
     (in stance) will
     still be touching the wall. Return CO_air if stance is CO_air. */

  static Pos shot_initial_pos(const Area &area,Touching touching,
			      const Size &shotSize,Dir shotDir);
  /* EFFECTS: Compute the starting position of the shot.  The shot should be
     flush on the inside of the area, if possible.  If the shot is too big,
     make sure it doesn't stick out the opposite side.  Uses touching to
     adjust location so that shot does not start out in a wall. */

  static int pure_to_half_pure(Dir d) 
    {assert (d >= CO_R && d < CO_DIR_MAX); return (d - CO_R) >> 1;}
  static int half_pure_to_pure(Dir d) 
    {assert (d >= 0 && d < 8); return 2 * d + CO_R;}
  /* EFFECTS: Convert between "pure" and "half pure" directions.  A "pure" 
     dir is 
     16 directions based at CO_R.  A "half pure" direction is 8 directions 
     based at zero. */

  static Touching dir_to_touching(Dir d);
  /* NOTE: Also used as the default "offset_generator", see MovingContext. */

  static Touching zero_offset_generator(Dir d);
  /* EFFECTS: Will make offset of (0,0) for all directions. */

  static void generate_offsets(Size offsets[CO_DIR_MAX],
                               const Size sizes[CO_DIR_MAX],
                               const Size &sizeMax,
                               Touching (*dirToTouching)(Dir));
  /* MODIFIES: offsets */
  /* EFFECTS: Fill offsets with the appropriate offsets for the given set of 
     sizes inside a rect of size sizeMax.  dirToTouching is used to 
     determine */
};



// Difficulty levels.
struct DifficultyLevel {
  int reflexes;  // For Machine intel
  int enemiesInitial; // enemies on first level
  int enemiesIncr; // this many more enemies each level
  int enemiesMax; // maximum number of enemies (for levels)
  float rankMultiplier; // adjust num kills according to difficulty
  char *name; // of this difficulty level
};
enum {DIFF_TRIVIAL,DIFF_NORMAL,DIFF_HARD,DIFF_BEND_OVER,
      DIFFICULTY_LEVELS_NUM, DIFF_NONE=DIFFICULTY_LEVELS_NUM};



typedef intptr_t TickType;
enum {TICK_HUMAN,TICK_NEUTRAL,TICK_ENEMY,TICK_OTHER,TICK_MAX};



class NetData {
public:
  NetData();
  /* EFFECTS: Initialize data. */

  static void enable_clocked_flag() {gClocked = 0;}
  static Boolean clocked_flag_enabled() {return gClocked >= 0;}
  Boolean clocked_this_turn() 
  {assert(gClocked >= 0); return fClocked == gClocked;}
  void set_clocked_this_turn() 
  {assert(gClocked >= 0); fClocked = gClocked;}
  static void toggle_clocked_flag() 
  {assert(gClocked >= 0); gClocked = !gClocked;}
  /* EFFECTS: Simple on/off system for determining if an Object has been 
     clocked yet this turn.  Only used by Client. 
     Must call enable_clocked_flag() before using any of the other functions,
     besides clocked_flag_enabled(). */

  TickType get_tick_type() {return tickType;}
  void set_tick_type(TickType val) {tickType = val;}
  /* NOTE: TICK_MAX means unspecified.  TickType will always be unspecified unless
     the physical was read in from a stream with a tickType specified. */

  Turn get_last_modified() {return lastModified;}
  /* EFFECTS: When was the last time this object was updated from the net. */

  void touch(Turn now) {lastModified = Utils::maximum(lastModified,now);}
  /* EFFECTS: Inform this that it was updated.  Could have Turn as an another
     argument to update_from_stream(), but I'm not going through all those 
     files again. */

  Boolean get_sent_flag() {return sent;}
  void set_sent_flag(Boolean val) {sent = val;}
  /* EFFECTS: Server uses this to keep track of whether an object has been
     sent to a Connection or not. */


private:
  Turn lastModified;
  TickType tickType;

  signed char fClocked;
  static signed char gClocked;
  // Start out the same.

  Boolean sent;

  // Could also have creationOk.
};

typedef NetData *NetDataP;



// Transform2D
enum TransformType {
  TR_NONE,
  TR_RT_1,  // Rotate 90 clockwise
  TR_RT_2,  // Rotate 180 clockwise
  TR_RT_3,  // Rotate 270 clockwise
  TR_RF_X   // Reflect in X direction.
};


// Used for auto-generating pixmaps for Moving.
struct OneTransform {
  // Max number of 2D transforms to get from a base to any dir.
  enum {TRANSFORMS_MAX = 2};  

  int transformsNum;
  Dir base;
  TransformType transforms[TRANSFORMS_MAX];

  static unsigned int compute_key(const TransformType* transforms,int tNum);
  /* EFFECTS: Generate a 3-bit key that uniquely represents the 
     transform. */
  /* NOTE: This might not be the best place for this function.
     Moving::init_x() uses it for generating keys for the SurfaceManager. */
};



typedef OneTransform TransformMap[CO_DIR_MAX];



// For an overrid TransformMap, this means don't override.
#define TR_USE_DEFAULT -1



class Transform2D {
public:
  // is_base(), get_base(), and get_transforms() all take an optional TransformMap
  // that will override the default transforms.  See "Fighter".

  static Boolean is_base(Dir d,TransformMap* override);
  /* EFFECTS: Is the given direction a base direction that can be used to 
     auto-generate other pixmaps. */

  static Dir get_base(Dir d,TransformMap* override);
  /* REQUIRES: d is not a base */
  /* EFFECTS: Return the base used to generate the bimap for the given 
     direction. */

  static const TransformType* get_transforms(int &tNum,Dir d,TransformMap* override);
  /* REQUIRES: d is not a base */
  /* MODIFIES: tNum */
  /* EFFECTS: Return the transformations necessary to get from the appropriate
     base to d.  Set tNum to the number of transformations.  Transformations must
     be applied in the given order.  tNum is 0 for the identity transformation. */

  static Pos apply(TransformType tt,const Pos &pos,const Size &size);
  /* EFFECTS: Apply transform tt to pos within a rectangle of the given size. */

  static Size apply(TransformType tt,const Size &size);
  /* EFFECTS: Given the size of a rectangle, return the size of the transformed
     rectangle. */

  static Pos apply_all(Dir d,const Pos &pos,const Size &size,TransformMap* override);
  /* EFFECTS: Convenience method.  Apply all transforms needed to get from pos in the coordinate system
     of the base of d to dest. */


private:
  static TransformMap transforms;
};

#endif


  
