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

// "intel.h"

#ifndef INTEL_H
#define INTEL_H

#if X11
	#ifndef NO_PRAGMAS
	#pragma interface
	#endif
#endif

// Include Files
#include "utils.h"
#include "coord.h"
#include "id.h"
#include "locator.h"
#include "world.h"


// Intel options defines
#define ITnone 0L
#define ITharmless (1L<<0)
#define ITclassFriends (1L<<1)
#define ITpsychotic (1L<<2)
#define ITignoreItems (1L<<3)
#define ITlimitedLifespan (1L<<4)
typedef unsigned long ITmask;


// Defines
#define IT_STRING_LENGTH 80
#define IT_INFINITE_LIVES -1

// Range that machine (and human for WIN32) players can see.
#define IT_VISION_RANGE 500


class Physical; 
typedef Physical *PhysicalP;
class Holder;
typedef Holder *HolderP;
class Modifier;
typedef Modifier *ModifierP;



struct IntelOptions {
  Boolean classFriends; 
  // Do not attack members of same class.  Does not apply to slaves of Humans.
  // DEFAULT: True

  Boolean harmless; 
  // Will not attack even if it has the ability to do so.
  // DEFAULT: False

  Boolean psychotic; 
  // Will never run away.  harmless overrides psychotic.
  // DEFAULT: False

  Boolean ignoreItems; 
  // Don't try to pick up items.
  // DEFAULT: False

  int limitedLifespan; 
  // Only for DoppelIntel and children.
  // If non-zero, kill self after fixed number of turns.
  // DEFAULT: 0
};



// IntelStatus used by ui.
// Remember Intel::die
struct IntelStatus {
  // strings and ClassIds must be kept consistent.

  char name[IT_STRING_LENGTH];
  ClassId classId;
  char className[IT_STRING_LENGTH];
  Health health; // -1 means dead.
  Health healthmax;//for health bar in windows version
  Mass mass;

  ClassId weaponClassId;  
  char weapon[IT_STRING_LENGTH];
  Boolean weaponReady;
  int ammo;

  ClassId itemClassId;
  char item[IT_STRING_LENGTH];
  int itemCount; // Number of the item.

  int lives; // Can be IT_INFINITE_LIVES.
  int humanKills;
  int enemyKills;
  int soups; // Human killed by something other than a human.  Only looked at
             // when game style is UIsettings::DUEL.

  
  void read(InStreamP);
  int get_write_length();
  void write(OutStreamP);
  /* EFFECTS: IO functions. */

  Boolean operator == (const IntelStatus&);
  /* EFFECTS: Member-wise comparison. */
};

  

// Class Definitions.
class Intel {
 public:
  Intel(WorldP w,LocatorP l,char *name,
       	const IntelOptions *ops,ITmask opMask); 
  /* EFFECTS: Create a new intel with the options specified in opMakse that are
     in ops.  ops will never be referenced if opMask is ITnone.  lives now
     dealt with in class Human. */

  Intel(InStreamP,WorldP,LocatorP);
  Boolean creation_ok() {return creationOk;}
  virtual void update_from_stream(InStreamP);
  virtual int get_write_length();
  virtual void write(OutStreamP);

  virtual ~Intel();
  
  virtual Boolean is_human();
  /* NOTE: is_machine() == (!is_human()). */
  virtual Boolean is_enemy();
  /* NOTE: is_neutral() == (!is_human() &&  !is_enemy()). */
  virtual Boolean is_seal_intel();
  virtual Boolean is_pet();

  Boolean alive() {return living;}
  WorldP get_world() {return world;}
  LocatorP get_locator() {return locator;}

  const Id &get_id() {return id;}
  /* NOTE: If dead, return an invalid id. */

  Boolean reincarnate_me();
  /* EFFECTS: Should *this be reincarnated. */

  Boolean intel_status_changed() {return intelStatusChanged;}

  void make_intel_status_dirty() {intelStatusChanged = True;}
  /* EFFECTS: Force a redraw on the UI. Used for shared lives. */

  const IntelStatus *get_intel_status();

  const char *get_name() {return intelStatus.name;}
  int get_lives() {return _get_lives();} // Can be IT_INFINITE_LIVES.
  int get_human_kills() {return intelStatus.humanKills;}
  int get_enemy_kills() {return intelStatus.enemyKills;}
  int get_soups() {return intelStatus.soups;}
  /* NOTE: Could achieve the same thing with get_intel_status.  Convenience 
   functions. */

  Boolean is_playing();
  /* EFFECTS: Is the intel still playing in the game, i.e. either living or
     dead but with extra lives left. */

  IntelId get_killer_intel_id() {return killerIntelId;}

  IntelId get_intel_id() {return intelId;}

  IntelId get_dont_collide() {return dontCollide;}
  /* NOTE: Used by Locator. */

  ModifierP get_modifiers() {return modifiers;}
  /* EFFECTS: Return modifier list for this intel.  Never returns NULL.  
     This is not a copy,
     mutate the list to add modifiers to the intel. */

  void set_dont_collide(const IntelId &val) {dontCollide = val;}
  /* EFFECTS: Locator will not allow the physical with this intel to collide 
     with the physical with the intel dontCollide. */

  void set_id(const Id &i) {id = i;}
  /* REQUIRES: i is valid. */
  /* NOTE: Only needed for Physical::set_intel.  id is updated on every 
     clock. */

  void set_lives(int lives)
    {_set_lives(lives); intelStatusChanged = True;}
  /* NOTE: Can be IT_INFINITE_LIVES. */

  void set_name(char *name);
  /* NOTE: Used by Server to set the name provided by the client. */

  virtual void add_human_kill();

  virtual void add_enemy_kill();

  virtual void add_soup();

  void set_killer_intel_id(const IntelId &k) {killerIntelId = k;}

  void set_intel_id(const IntelId &i) {intelId = i;}
  /* NOTE: Should only be used by the Locator. */

  static ITcommand dir_to_command(Dir dir);
  /* REQUIRES: dir is a "pure" direction or CO_air. */
  /* NOTE: CO_air -> IT_CENTER. */

  static ITcommand dir_to_command_weapon(Dir dir);

  static Dir command_weapon_to_dir_4(ITcommand);

  static Dir command_weapon_to_dir_8(ITcommand);
  /* EFFECTS: If command is a weapon command return the corresponding Dir. */
  /* NOTE: Like Gun::compute_weapon_dir. */

  static Boolean is_command_weapon(ITcommand command)
  {return (command >= IT_WEAPON_R && command <= IT_WEAPON_UP_R) || 
     command == IT_WEAPON_CENTER;}

  static ITcommand center_pos_to_command(const Pos &pos);
  /* EFFECTS: Returns the command corresponding to the direction from (0,0) to
     pos. */
  /* NOTE: pos can have negative components. */
     
  static Size command_to_size(ITcommand command,int mag);
  /* EFFECTS: Change all commands to a Size where each coordinate
     has length -mag,0, or mag.  All weapon or item commands give (0,0). E.g. IT_L 
	 will return (-mag,0), IT_DN_R gives (mag,mag). */

#if 0
  virtual IntelP compatible_composite(IntelP intel) = 0;
  /* EFFECTS: If intel is a legal intel to be a member of
     the same composite object as this, return NULL. 
     Else, create and return a new intel that is 
     compatible. */
#endif

  void die(); 
  /* EFFECTS: Tell *this that it no longer has anything clocking it. */
  /* NOTE: This MUST be called at death.  Game requires it to refill enemy
     players. */

  void reincarnate();
  /* EFFECTS: Tell *this that it will be clocked again. */

  virtual void clock(PhysicalP p);
  /* NOTE: Should be called by Intel's children.  Call up the tree. */


#ifndef PROTECTED_IS_PUBLIC
 protected:
#endif
  const IntelOptions &get_intel_options() {return intelOptions;}

  virtual int _get_lives();
  virtual void _set_lives(int);
  /* EFFECTS: Allow children to override behavior of lives. */

  void creation_failed() {creationOk = False;}


 private:
  IntelStatus intelStatus; // intelStatus.lives is not used here.
  Boolean intelStatusChanged; // Since last get_intel_status.
  Boolean living;
  WorldP world;
  LocatorP locator;
  Id id;
  IntelId killerIntelId;  // Intel that most recently attacked the Intel.
  IntelId intelId;
  IntelOptions intelOptions;
  IntelId dontCollide; 
  ModifierP modifiers;
  Boolean creationOk;

  const static IntelOptions intelOptionsDefault;
};
typedef Intel *IntelP;



// Simply buffers input from a user interface.
class Human: public Intel {
 public: 
  Human(WorldP w,LocatorP l,char *name,int lives,
        int *sharedLives,ColorNum colorNum);
  /* NOTE: Uses all default options.  intelOptions should be meaningless for 
     a human. If "sharedLives" is non-null, ignore "lives" and share the lives. */

  Human(InStreamP,WorldP,LocatorP);
  virtual void update_from_stream(InStreamP);
  void _update_from_stream(InStreamP);
  virtual int get_write_length();
  virtual void write(OutStreamP);


  ColorNum get_color_num() {return colorNum;}

  virtual Boolean is_human();
    
  void set_command(ITcommand c);
  /* EFFECTS: Asynchronously set the command for the current turn to be c.
     Overrides previous settings. */

  ITcommand get_command() {return command;}
  /* EFFECTS: Return the most recently set command. */

  virtual void clock(PhysicalP p);
  /* EFFECTS: Sets the command for p. */

  void set_data(void *d) {data = d;}
  void *get_data() {return data;}
  /* NOTE: Generic data used by Game. E.g. for depth in ZIG-ZAG. */


#ifndef PROTECTED_IS_PUBLIC
 protected:
#endif
  virtual int _get_lives();
  virtual void _set_lives(int);
  /* NOTES: Allow for shared lives. */


 private:
  void *data;
  ITcommand command;
  ColorNum colorNum;
  int lives;
  int *sharedLives;
  ITcommand noRepeat;  // The command that can't be repeated.
};
typedef Human *HumanP;



class Boredom {
public:
  Boredom();

  Boolean check(LocatorP l,PhysicalP p);
  /* EFFECTS: Check if the intel is bored.  Used so creatures don't get
     stuck. Should be called for every cycle of reflexes, not every
     turn. */


private:
  Id lastId;
  Pos lastMiddle;
  Timer timer;
};



class Machine: public Intel {
public:
  Machine(WorldP w,LocatorP l,char *name,
	  const IntelOptions *ops,ITmask opMask);
	  
  virtual IntelId get_master_intel_id();
  /* EFFECTS: If a slave, return the IntelId of the master.  Otherwise, 
     return an invalid IntelId. */
		 
  virtual void clock(PhysicalP);
  /* NOTE: Not called by Machine's children. */


#ifndef PROTECTED_IS_PUBLIC
protected:
#endif
  enum Strategy {
    doNothing, 
    toPos,         // Move toward targetPos.
    toTarget,      // Move toward Physical targetId
    awayTarget,	   // Move away from Physical targetId
    attackTarget,	 // like toTarget, but attack if possible
    retreatTarget, // like awayTarget, but attack if possible
    toDir,         // Move in direction targetDir
  };

  Strategy get_strategy() {return strategy;}
  
  Timer &get_strategy_change() {return strategyChange;}
  /* EFFECTS: Return the timer that controls when to change strategy. */

  virtual int _get_reflexes_time();
  /* EFFECTS: Override this to set reflexes. */
  
  Boolean strategy_uses_target_id() 
  {return strategy == toTarget || strategy == awayTarget ||
     strategy == attackTarget || strategy == retreatTarget;} 
  Id get_target_id() {assert(strategy_uses_target_id()); return targetId;}
  
  void set_doNothing() {strategy = doNothing;}
  void set_toPos(const Pos &pos) {strategy = toPos; targetPos = pos;} 
  void set_toTarget(const Id &id)	{strategy = toTarget; targetId = id;}
  void set_awayTarget(const Id &id) {strategy = awayTarget; targetId = id;}
  void set_attackTarget(const Id &id)	{strategy = attackTarget; targetId = id;}
  void set_retreatTarget(const Id &id)	{strategy = retreatTarget; targetId = id;}
  void set_toDir(Dir dir) {strategy = toDir; targetDir = dir;}
  /* EFFECTS: Mutators to set the strategy. */
  /* NOTE: Designed so that no more that one of {targetPos,targetId,targetDir}
	   is set at once. And that one corresponds to the strategy. */
  /* NOTE: Not necessarirly called in the choose_strategy phase. */
  /* NOTE: Should probably make these calls set the strategyChange 
     timer, instead of exposing the timer. */

  Boolean strategy_to_random_pos(PhysicalP);
  /* EFFECTS: Helper function, calls set_toPos with a random location
     in the world.  Return whether strategy was changed successfully. */
  
  virtual void choose_strategy(PhysicalP);
  /* EFFECTS: Sets the strategy.  May use or set the strategyChange timer. */
  
  PhysicalP choose_target(Boolean &isEnemy,PhysicalP p,int range);
  /* MODIFIES: isEnemy */
  /* EFFECTS: Return a new target for p (within range distance).
		Sets isEnemy to True 
		iff returned pointer is an enemy (as opposed to an item).
    Returns NULL if no suitable target found. */
  /* NOTE: Doesn't set targetId or strategy. */
  /* NOTE: Not virtual, but can be called by children inside
     choose_strategy(). */
  
  virtual Boolean filter_target(PhysicalP p);
  /* EFFECTS: Used by children to filter out things the Machine shouldn't
     set as the target. Return whether p is a valid target. */
  /* NOTE: Can call up the tree. */

  static Boolean attack_target(PhysicalP p,PhysicalP target);
  static Boolean move_target(PhysicalP p,PhysicalP target,Timer &ladderJump);
  static Boolean away_target(PhysicalP p,PhysicalP target,Timer &ladderJump);
  static Boolean move_pos(PhysicalP p,const Pos &targetPos,Timer &ladderJump);
  static Boolean move_dir(PhysicalP p,Dir d,Timer &ladderJump);
  /* REQUIRES: p and target must be non-NULL. */
  /* REQUIRES: d is an element of {CO_R..CO_UP_R,CO_air} */
  /* EFFECTS: The above are all utility functions that attempt to set the
     command for p to do something.  Returns whether the command was set or
     not. */
  /* IMPLEMENTATION NOTE: Declared static to ensure that no data members of 
     Machine are used in these utility functions.  */


  static Boolean has_gun(HolderP);
  static Boolean has_cutter(HolderP);
  static Boolean has_shield(HolderP);
  static Boolean has_drugs(HolderP);
  static Boolean has_item(HolderP,ClassId);
  static Boolean weapon_current_is_gun(HolderP);
  static Boolean weapon_current_is_cutter(HolderP);
  /* EFFECTS: Return info about a Holder. */

  static Boolean weapon_current_is_drop_liftable(PhysicalP);
  /* NOTE: Semi-hack to check for dropping Liftables. */


private:
  void use_items(Boolean &commandSet,PhysicalP p);
  /* EFFECTS: Called in Machine::clock(), choose which
     item to use and use it.  Weapons are used in attack_target().
     p is the Physical this is controlling. */
  /* MODIFIES: commandSet */
  

  // Check to see if we're stuck in the same place for too long.
  Boredom boredom;

  Strategy strategy;
  Timer strategyChange;
  
  // Only one of the following three is valid.
  Pos targetPos;
  Id targetId;
  Dir targetDir;
  
  Timer reflexes;
  Timer ladderJump; // don't get back on ladder immediately after jumping off.
};
typedef Machine *MachineP;



class Enemy: public Machine {
public:
  Enemy(WorldP w,LocatorP l,char *name,
	const IntelOptions *ops,ITmask opMask);

  static void set_reflexes_time(int time) {reflexesTime = time;}
  static int get_reflexes_time() {return reflexesTime;}
  /* EFFECTS: How fast machines react.  1 means every turn. 2 is 
  every other turn, etc. */
	  
  virtual Boolean is_enemy();


#ifndef PROTECTED_IS_PUBLIC
protected:
#endif
  int _get_reflexes_time();
  /* NOTE: To pass info to Machine. */

  
private:
  static int reflexesTime;
};
typedef Enemy *EnemyP;



class Neutral: public Machine {
public:
  Neutral(WorldP w,LocatorP l,char *name,
	const IntelOptions *ops,ITmask opMask);
	  
  virtual Boolean is_enemy();
};
typedef Neutral *NeutralP;



class SealIntel: public Neutral {
public:
  SealIntel(WorldP w,LocatorP l,char *name,const Id &homeId);
  
  virtual Boolean is_seal_intel();


#ifndef PROTECTED_IS_PUBLIC
protected:
#endif
 	virtual void choose_strategy(PhysicalP);
	/* EFFECTS: Sets the strategy.  May use or set the strategyChange timer. */


 private:
  Id homeId;
};



class DoppelIntel: public Neutral {
public:
	DoppelIntel(WorldP w,LocatorP l,char *name,
    const IntelOptions *ops,ITmask opMask,
	  IntelP master = NULL);
	
	virtual IntelId get_master_intel_id();
  
	virtual void add_human_kill();

  virtual void add_enemy_kill();

  virtual void add_soup();

  virtual void clock(PhysicalP);

  static IntelOptions suggestedOptions;
  static ITmask suggestedMask;


#ifndef PROTECTED_IS_PUBLIC
protected:
#endif
	void set_master_intel_id(const IntelId &id) {masterIntelId = id;}
	
	virtual void choose_strategy(PhysicalP p);
	/* EFFECTS: Filters out the master or other objects controlled by
		the same master. */
	/* NOTE: p is the Physical this is controlling. */

	virtual Boolean filter_target(PhysicalP candidate);

	PhysicalP lookup_and_compute_dist_2(int &dist_2,
																      PhysicalP p,const Id &id);
	/* EFFECTS: A helper routine. If both p and Locator::lookup(id)
	are non-NULL. Return the result of looking up id, and set
	dist_2 to be the square of the distance between the two. Else return NULL. */
	/* MODIFIES: dist_2 */

  
private:
  Timer suicideTimer;
  IntelId masterIntelId;
};



// Pet acts like a finite state machine with four states, 
// Heel, Return, Seek, and Attacking.  Transitions are implemented
// in choose_strategy() and go_get_em().
class Pet: public DoppelIntel {
public:
  Pet(WorldP w,LocatorP l,char *name,
	    const IntelOptions *ops,ITmask opMask,
	    IntelP master = NULL);
    
  Boolean is_pet();

  void go_get_em(PhysicalP p,ITcommand command);
  /* EFFECTS: p is ordering the pet to attack in the specified direction if 
	IT_WEAPON_{some dir}, or to come back to him if command is IT_WEAPON_CENTER. 
	This will also set p to be the new master. */

	static char *mode_string(int mode);
	/* NOTE: For debugging. */


#ifndef PROTECTED_IS_PUBLIC
protected:
#endif
  virtual int _get_reflexes_time();

  virtual void choose_strategy(PhysicalP p);

  virtual Boolean filter_target(PhysicalP candidate);


private:
  enum {Heel, Returning, Seek, Attacking};
  enum {RETURN_TIME = 500, 
	HEEL_RADIUS = 18, 
	ATTACK_RADIUS = 100, 
	GIVE_UP_ATTACK_RADIUS = 400};
  // Squares of distances used for comparisons.
	enum {HEEL_RADIUS_2 = HEEL_RADIUS * HEEL_RADIUS,
	      ATTACK_RADIUS_2 = ATTACK_RADIUS * ATTACK_RADIUS,
	      GIVE_UP_ATTACK_RADIUS_2 = GIVE_UP_ATTACK_RADIUS * GIVE_UP_ATTACK_RADIUS};
  
  int mode;
};
typedef Pet *PetP;
#endif

