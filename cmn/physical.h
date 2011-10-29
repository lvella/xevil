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

// "physical.h"  
// Interior nodes of the physical object tree.  These classes
// are never instantiated.


#ifndef PHYSICAL_H
#define PHYSICAL_H

#if X11
	#ifndef NO_PRAGMAS
	#pragma interface
	#endif
#endif

// Include Files
#include "utils.h"
#include "coord.h"
#include "xdata.h"
#include "area.h"
#include "world.h"
#include "id.h"
#include "intel.h"
#include "locator.h"
#include "streams.h"


// Defines
#define PH_WEAPONS_MAX 10
#define PH_ITEMS_MAX 20
//#define PH_AMMO_UNLIMITED  in coord.h

// Careful, the Droplet classes use this.
#define PH_GRAVITY 2   
#define PH_PUSH_OFF 3

// PH_AUTO_GEN means generate pixmap bits from another direction 
// via Transfrom2D.
// Assuming that no pixmap bits are 0xffffffff.
// Safe on Windows because resource symbols are a max of 65K.
#define PH_AUTO_GEN ((CMN_BITS_ID)-1)

// Default time to be invisible for cloaks, or for invisible client object
// if no new updates come in.
#define PH_INVISIBLE_TIME 1500  // One minute.


// A bit flag of possible droplet types.
typedef u_int DropletSet;

// Possible members of DropletSet.
#define DROPLET_NONE         0x0         // Must be 0x0
#define DROPLET_BLOOD        (0x1 << 0)
#define DROPLET_GREEN_BLOOD  (0x1 << 1)
#define DROPLET_OIL          (0x1 << 2)
// Kinda hack, to be able to increase chance of oil, used by Walker.
#define DROPLET_MORE_OIL     (0x1 << 3)  
#define DROPLET_FEATHER      (0x1 << 4)
// WARNING: If you add another droplet type, also add it to
// Droplet::choose_droplet_class().  Yeah, I know, should be 
// a better way. 



////////////////////////////////////////////////////////////////////////////
//----------------------- MACROS FOR STREAM IO ---------------------------//

// Add a ';' after this.
#define DECLARE_INTERNAL_NODE_IO(CLASSNAME) \
  CLASSNAME(InStreamP,const CLASSNAME ## Context &, \
            CLASSNAME ## Xdata &, \
            WorldP, \
            LocatorP); \
  virtual void update_from_stream(InStreamP in); \
  void _update_from_stream(InStreamP); \
  virtual int get_write_length(); \
  virtual void write(OutStreamP)

#define DECLARE_ROOT_NODE_IO(CLASSNAME) \
  CLASSNAME(InStreamP,const CLASSNAME ## Context &,WorldP,LocatorP); \
  virtual void update_from_stream(InStreamP); \
  virtual int get_write_length(); \
  virtual void write(OutStreamP)

#define DECLARE_NULL_INTERNAL_NODE_IO(CLASSNAME) \
  CLASSNAME(InStreamP,const CLASSNAME ## Context &, \
            CLASSNAME ## Xdata &, \
            WorldP,LocatorP)

// Appropriate Context will be named "cx" and Xdata named "x_data".
#define CONSTRUCTOR_INTERNAL_NODE_IO(CLASSNAME,PARENT,parent) \
   CLASSNAME::CLASSNAME(InStreamP in,const CLASSNAME ## Context &cx, \
                         CLASSNAME ## Xdata &x_data, \
                         WorldP w,LocatorP l) \
   : PARENT(in,cx.parent ## Context,x_data,w,l)

// Used by internal nodes and leaves.
#define DEFINE_UPDATE_FROM_STREAM(CLASSNAME,PARENT) \
  void CLASSNAME::update_from_stream(InStreamP in) { \
    PARENT::update_from_stream(in); \
    _update_from_stream(in); \
  }

#define DECLARE_ABILITY_IO(CLASSNAME) \
  CLASSNAME(InStreamP,CreatureP,const CLASSNAME ## Context &,CLASSNAME ## Xdata &); \
  void update_from_stream(InStreamP in); \
  int get_write_length(); \
  void write(OutStreamP); \
  static AbilityP create_from_stream(InStreamP,CreatureP)  

#define DECLARE_NULL_ABILITY_IO(CLASSNAME) \
  CLASSNAME(InStreamP,CreatureP,const CLASSNAME ## Context &,CLASSNAME ## Xdata &); \
  static AbilityP create_from_stream(InStreamP,CreatureP)


#if 0
// Hack to shut the fucking compiler up about unused variables.
#define ABILITY_IO_HACK \
  void *dummy; \
  dummy = (void *)in; \
  dummy = (void *)&cx; \
  dummy = (void *)&x_data;

// Define several functions.
#define DEFINE_NULL_PRE_LEAF_IO(CLASSNAME) \
  void CLASSNAME::update_from_stream(InStreamP) { \
  } \
  int CLASSNAME::get_write_length() { \
    return 0; \
  } \
  void CLASSNAME::write(OutStreamP) { \
  }
#endif

#define DECLARE_NULL_LEAF_IO(CLASSNAME) \
  CLASSNAME(InStreamP,WorldP,LocatorP); \
  static PhysicalP create_from_stream(InStreamP,WorldP,LocatorP)

#define DECLARE_LEAF_IO(CLASSNAME) \
  CLASSNAME(InStreamP,WorldP,LocatorP); \
  virtual void update_from_stream(InStreamP); \
  void _update_from_stream(InStreamP); \
  virtual int get_write_length(); \
  virtual void write(OutStreamP); \
  static PhysicalP create_from_stream(InStreamP,WorldP,LocatorP)

// Assumes the appropriate context is named "context".
#define CONSTRUCTOR_LEAF_IO(CLASSNAME,PARENT) \
  CLASSNAME::CLASSNAME(InStreamP in,WorldP w,LocatorP l) \
  : PARENT(in,context,xdata,w,l)

#define DEFINE_CREATE_FROM_STREAM(CLASSNAME) \
  PhysicalP CLASSNAME::create_from_stream(InStreamP in, \
                                          WorldP w,LocatorP l) { \
    PhysicalP p = new CLASSNAME(in,w,l); \
    if (p->creation_ok() && in->alive()) { \
      return p; \
    } \
    else { \
      delete p; \
      return NULL; \
    } \
  } 

// Note that defaultContext and defaultXdata are static to CLASSNAME.
#define DEFINE_CREATE_ABILITY_FROM_STREAM(CLASSNAME) \
AbilityP CLASSNAME::create_from_stream(InStreamP in,CreatureP creat) { \
  AbilityP ret = new CLASSNAME(in,creat,defaultContext,defaultXdata); \
  assert(ret); \
  return ret; \
}

#if 0
#define DECLARE_CREATURE_IO(CLASSNAME) \
  CLASSNAME(InStreamP,WorldP,LocatorP); \
  virtual void update_from_stream(InStreamP); \
  virtual int get_write_length(); \
  virtual void write(OutStreamP); \
  static PhysicalP create_from_stream(InStreamP,WorldP,LocatorP)
#endif

#define DECLARE_NULL_CREATURE_IO(CLASSNAME) \
  CLASSNAME(InStreamP, WorldP,LocatorP); \
  static PhysicalP create_from_stream(InStreamP,WorldP,LocatorP)


//--------------------------- END IO MACROS ------------------------------//
////////////////////////////////////////////////////////////////////////////



enum PHsig {PH_NO_SIG, PH_NOT_SET, PH_ID_CHANGED};



class Protection;
typedef Protection *ProtectionP;
class Modifier;
typedef Modifier *ModifierP;
class Ability;
typedef Ability *AbilityP;
class Holder;
typedef Holder *HolderP;
class Locomotion;
typedef Locomotion *LocomotionP;
class Composite;
typedef Composite *CompositeP;



////////// Physical
/* Physical is the parent class of all physical objects. */

/* PhysicalContext plays a double role.  Like all the other "contexts", 
   MovingContext, CreatureContext, etc., it is a place to specify paramaters
   for a specific class.  E.g. if you look at the PhysicalContext in 
   ninja.bitmaps you will see where the mass, max health, etc, for a Ninja is
   specified.
   
   But, it also plays a role for the Object Locator.  All classes of Physical
   objects (not just the objects, but the classes) are registered with
   Locator::register_context().  So, the Locator has a list of all the classes
   in the game.  For example, if the Locator wanted to create one of each type
   of object, it could just go through the list of registed classes and 
   call the PhysicalContext->create() method on each.
   Only PhysicalContext is used for this extra functionality.  MovingContext,
   ShotContext, etc, only play the first role.
   */
struct PhysicalContext {
  Health health;
  Mass mass;
  ClassId classId;
  const char *className;


  /* Global fields for the Game as a whole. */

  // Can a Transmogifier turn something into this class of object.
  Boolean transmogifyTarget;
  // Can objects of this class use doppelgangers.  E.g. we don't want 
  // FireDemons to start duplicating themselves.  This is independent of 
  // whether this class is derived from class User.  
  Boolean doppelUser;
  // Can a Human player start out as an object of this class.
  Boolean potentialHuman;
  // Can an Enemy(Machine) player start out as an object of this class.
  Boolean potentialEnemy;
#if 0  // This needs to be added for Game::intel_options_for()
  // Only meaningful if potentialEnemy is True.
  // If an enemy is created of this class, will it receive a psychotic Intel.
  Boolean enemyIsPsychotic;
#endif
  // This gives the likelihood that an enemy will
  // be created of this class.  For reference Ninja is 3.  E.g. if 
  // enemyWeight is 1, then there will be one third as many of these objects
  // as there are Ninjas.
  // NOTE: This may be used even if potentialEnemy is False.  A Scenario may
  // specifically request a set of potential enemies, e.g. Seals.
  int enemyWeight;
  // Is this a weapon or item that can magically fall out of the sky.
  // Only one of potentialWeapon, potentialOtherItem should be True.
  Boolean potentialWeapon;
  Boolean potentialOtherItem;
  // If the Physical is a potential item or weapon, this gives a measure of
  // how many of the object will be created.  You can think of this as the 
  // "percent" of the World that is covered by the item/weapon.
  float objectWorldPercent;
  // If the Physical is a potential item or weapon, this gives the maximum
  // number of times the object will appear that level.  
  // 0 means infinity, i.e. no limit.
  int levelMaxTimes;
  // The maximum size of the Physical corresponding to this context.
  Size sizeMax;
  // A pointer to a function that creates an instance of the Physical 
  // corresponding to this context.
  PhysicalP (*create)(void *arg,WorldP w,LocatorP l,const Pos &pos);
  // Flags that determine what statistics will be printed out when XEvil is
  // exited.  You probably only want to set one of these, if any.
  // E.g. For Shot, we are interested in the number that were 
  // created (fired), so statsCreations would be True.  E.g. For MedKit, 
  // we care more about how many were used than how many were created, so we
  // set statsUses.  For Ninja, the most interesting thing is how many of 
  // got killed (and average lifespan), so we set statsDeath.  E.g. for Rock
  // we don't set any of them, (who cares about a rock).
  Boolean statsCreations;
  Boolean statsUses;
  Boolean statsDeaths;
  // Return the stats for the class.  This will be NULL if the class does not
  // have stats.  I.e. a NULL function, NOT a function that returns NULL.
  // This is non-NULL iff any of the stats{Creations,Uses,Deaths} flags
  // are set.
  // 
  // I could probably have put the actual Stats here instead of
  // every class having to have its own stats.  But, I really wanted to keep
  // all the "Contexts" constant, and Stats obviously have to change during
  // game play.
  Stats &(*get_stats)(void *arg);  // Not const anymore.
  // Argument passed to create() and get_stats().  NULL except for templates.
  void *arg;
  // A very poorly named function.  Multi-purpose graphics function for this 
  // class.  See Moving::init_x().
  void (*init_x)(Xvars &xvars,IXCommand command,void* arg);
  // Read in a new Physical from an InStream.  Return NULL if failure.
  PhysicalP (*create_from_stream)(InStreamP,WorldP,LocatorP);
  // If non-null, will return whether the given class derives from 
  // Item or Weapon.  Will also return a bitmap icon suitable for a 
  // status window (WIN32 only for now).
  // If null, the class is guaranteed not to be an Item or Weapon.
  void (*get_item_info)(Boolean &isItem,Boolean &isWeapon,CMN_BITS_ID &iconId);
  
//******* This needs to be added in, see Locator::draw_ticks()  ********/
  // Should always draw the tick marks on the edge of the screen for this 
  // Physical, e.g. Flag, Xit
//  Boolean alwaysDrawTicks;
};


class Physical {
public:
  Physical(const PhysicalContext &,WorldP,LocatorP);
  /* EFFECTS: Create a new, mapped physical with no Id with undefined area. */

  Physical();
  /* NOTE: Should never be called. */

  Boolean creation_ok() {return creationOk;}
  /* EFFECTS: Create physical by reading in from stream.  Use 
     Physical::creation_ok() to see if it was successfully read in. */

  DECLARE_ROOT_NODE_IO(Physical);
  void _update_from_stream(InStreamP,Boolean alreadyExists);
  /* NOTE: alreadyExists is used for checking consistency when
     updating an existing object from a stream. */
  
  virtual ~Physical();

  virtual const Area &get_area() = 0;

  virtual const Area &get_area_next() = 0;
  /* NOTE: Should only be used by the Locator. */

  Health get_health() {return health;}

  Health get_health_next() {return healthNext;}
  /* NOTE: Should only be used by the Locator to prepare for death.
     Somewhat of a hack. */

  virtual Health get_health_max();
  /* NOTE: Overridden in Creature. */

  Mass get_mass() {return mass;}
  virtual Vel get_vel();
  virtual Dir get_dir();

  ClassId get_class_id() {return pc->classId;}
  const char *get_class_name() {return pc->className;}

  virtual int get_drawing_level();

  const PhysicalContext *get_context() {return pc;}

  Boolean delete_me() {return deleteMe;}

  Boolean alive() {assert (health >= 0 || dieCalled); return health >= 0;}
  /* NOTE: Is publicly known that (health >= 0) <=> alive.  So this function is
   just for convenience. */

  Boolean die_called() 
//  {assert (healthNext < 0 || !dieCalled);  return dieCalled;}
    {return dieCalled;}
  /* EFFECTS: Returns whether die() has been called or not.  Not clocked. */

  /* Should only be used for abstract classes.  Actual classes can be tested
     for with get_class_id(). */
  virtual Boolean is_moving();
  virtual Boolean is_shot();
  virtual Boolean is_item(); 
  virtual Boolean is_shield();
  virtual Boolean is_bomb();
  virtual Boolean is_weapon();
  virtual Boolean is_cutter();
  virtual Boolean is_gun();
  virtual Boolean is_creature(); 
  virtual Boolean is_auto_use();
  virtual Boolean is_drugs();
  virtual Boolean is_liftable();


  virtual AbilityP get_ability(AbilityId);
  virtual HolderP get_holder();
  virtual LocomotionP get_locomotion();
  virtual CompositeP get_composite();
  /* NOTE: These are really only meaningful for Creature.  Provided here
     for convenience. */

  Boolean get_mapped() {return mapped;}

  virtual Boolean collidable();
  /* NOTE: This value never changes for an object. */

  const Acc *get_unit_accs() {return unitAccs;}
  const Vel *get_unit_vels() {return unitVels;}

  Id get_id() {return id;}

  PHsig get_id(Id &id);
  /* MODIFIES: id */
  /* EFFECTS: Set id to be the Id and return PH_NO_SIG if set.  Otherwise, 
     return PH_NOT_SET. */

  Id get_dont_collide() {return dontCollide;}
  /* EFFECTS: If there is another object that *this is not allowed to collide
     with, return it.  Otherwise return NULL; */

  CompositeId get_dont_collide_composite() {return dontCollideComposite;}
  /* NOTE: Return COMPOSITE_ID_NONE if the flag isn't set. */

  IntelP get_intel() {return intel;}
  /* NOTE: Can be NULL. */

  Boolean get_flash() {return !flashTimer.ready();}

  Boolean get_quiet_death() {return quietDeath;}

  virtual void get_followers(PtrList &list);
  /* EFFECTS: Fill list with all the follower objects of the Creature, e.g.
     Chainsaws, Shields.  list will be a list of PhysicalP. */
  /* NOTE: All mapped and unmapped followers. */
  /* NOTE: Call up the tree. */
  /* WARNING: Careful not to cache the results, as it is a list of PhysicalP,
     not of Id. */
  /* NOTE: Maybe we should be using PtrList::add_unique() instead of add() to
     fill the list. */

  virtual void follow(const Area &followeeArea,Dir followeeDir,
                      Boolean currentAndNext);
  /* EFFECTS: Follow the followee.  If currentAndNext is True, set current
   and next state, e.g. after reading from a stream.  If False, only set
   next state, e.g. normal clock(). */
  /* NOTE: Sure would be nice to have delegation to a IFollowable 
     interface. */

  NetDataP get_net_data();
  /* EFFECTS: Get the chuck of data specific to network play.  Not a copy,
   do not free results. */

  void set_command(ITcommand c) {command = c;}
  /* EFFECTS: Sets the command to be c, overrides any previous command 
     setting. */
  /* NOTE: command is not clocked. */

  virtual Boolean command_repeatable(ITcommand c);
  /* EFFECTS: Given the current state of the Physical, should c be allowed 
     to be sent repeatedly through set_command().  Or must there be a delay
     after the invocation of set_command(c).  Decide whether we can do key 
     repeat or not. */

  void set_id(const Id &id);
  /* REQUIRES: Id not already set. */
  /* EFFECTS: Set the Id to be id. */

  void set_dont_collide(const Id &other) {dontCollide = other;}
  /* EFFECTS: *this will not be allowed to collide with other.  Any previous 
     value will be overridden.  Pass in an invalid Id to disable dontCollide. */
  
  void set_dont_collide_composite(CompositeId compId) {dontCollideComposite = compId;}
  /* EFFECTS: Similar to set_dont_collide(), but for all members of a composite object. */

  virtual void set_intel(IntelP i);
  /* REQUIRES: Object has been added to locator (has valid id.) */
  /* NOTE: Can be NULL. */
  /* NOTE: Is virtual so Creature can inform Abilities of change. */

  void set_health_next(Health h) {assert(alive()); healthNext = h;}

  virtual void heal();
  /* EFFECTS: Called by MedKit. */
  /* NOTE: Calls up the tree. */
  
  virtual void set_mapped_next(Boolean val);
  /* NOTE: Should be ok to set the value to the previous value. */
  /* NOTE: idempotent */
  /* Calls up the tree. */

  void set_delete_me() {deleteMe = True;}
  /* NOTE: Should only be called by self and Locator. */

  void flash();
  /* EFFECTS: Object will be drawn for FLASH_TIME turns. */

  void set_no_death_delete() {noDeathDelete = True;}

  virtual Boolean corporeal_attack(PhysicalP,int damage,
           AttackFlags flags = ATT_DAMAGE | ATT_DROPLETS);
  /* EFFECTS: Inflict some sort of bodily damage on the Physical.
     Return True if attack succeeded, False if it was blocked in
     some way.  killer is the Physical responsible for doing the damage, or
     NULL. Logically OR together the desired flags for the type of attack, see
     definition of AttackFlags. Default is to do damage, and generate blood 
     droplets. */
  virtual void heat_attack(PhysicalP,int heat,Boolean secondary = False);
  /* EFFECTS: Perform heat or fire attack on the Physical. */
  /* NOTE: Sometimes call up the tree. */
  /* NOTE: killer is the one responsible for causing the damage.  Can be NULL.
     Adds kills to the killer. */
  /* NOTE: Only the last call before the update cycle takes effect. */

  virtual Boolean swap_protect();
  virtual Boolean frog_protect();
  /* EFFECTS: Returns whether the Physical is protected from a swap or frog
     attack.  May have side effects, so call only once per attack. */
  /* NOTE: Using a Transmogifier is considered a swap attack. */
  
  virtual void avoid(PhysicalP);
  virtual void collide(PhysicalP);
  /* EFFECTS: Collision procedures.  avoid is called on the lighter of the two
     objects.  collide is called on both objects. */
  /* NOTE: Not always called up the tree. */

  void intelligence() {if (intel) intel->clock(this);}

  void kill_self() {healthNext = -1;}
  /* NOTE: Called in act/collision_checks phase.  Do not call this in the 
     die phase. (or update phase?) */
  /* NOTE: I think you CAN call it in the die phase, but only if you check
     for die_called() or something like that. */

  virtual void set_quiet_death();
  /* EFFECTS: When this dies, do not do any funny things like leaving corpses
     or exploding or any other type of physical evidence. */
  /* NOTE: Calls up the tree. */

  void virtual act();
  /* EFFECTS: Action phase.  All next variables must be set here.  Commands 
     are interpreted here.*/

  void virtual update(); 
  /* EFFECTS: Set current variables to be the next ones.  No interactions 
     between physical objects. */

  void virtual draw(CMN_DRAWABLE buffer,Xvars &xvars,int dpyNum,
		    const Area &area) = 0;
  /* REQUIRES: buffer is at least as big as area. */
  /* EFFECTS: Draw the physical object in buffer.  buffer represents area. */
  /* NOTE: Does not check for overlap */
  /* NOTE: X variables initialized in draw.  Thus, if draw is never called for
     a base class, the X variables never need to be initialized. */

  void virtual die();
  /* EFFECTS:  If the *this dies a natural death (I.e. health < 0), then this
     function is called.  Not called if *this is 
     destroyed for any other reason.  E.g. end of game.  The default is to 
     kill the intel and set_delete_me. */
  /* NOTE: Called only in the die phase. */
  /* NOTE: Calls up the tree. */
  /* NOTE: Guaranteed to be called only once. */

  virtual void dr_clock();
  /* EFFECTS: Perform one step of dead reckoning on the client.
     This will only be called on turns where update_from_stream() isn't 
     called. Simulates calls to act() and update(). */
  /* NOTE: calls up the tree. */

  virtual ClassId get_weapon_string(const char *&str);
  /* EFFECTS: str is set to point to a static string containing the string
     for ClassId. */
  /* NOTE: BuiltIn returns something for get_weapon_string(), but NULL
     for get_weapon_current. */

  virtual void drop_all(Boolean killNonPersistent);
  /* EFFECTS: Drop all Items being carried.  If killNonPersistent is set, non 
     persistent objects, like Bomb, will be quietly killed, not dropped. */
  /* NOTE: Not in Holder since, e.g. Frog::drop_all must be able to call drop_all
     on the unmapped object. */
  /* NOTE: Do NOT call this in the die phase, e.g. from a call to Physical::die(). */

  void hack_update_area_next() {update_area_next(True);}
  /* NOTE: Game::off_clock_kill() wants area to match areaNext when
     killing objects. */


#ifndef PROTECTED_IS_PUBLIC
protected:
#endif
  WorldP get_world() {return world;}

  Boolean alive_next() {return healthNext >= 0;}

  virtual Dir get_dir_next();

  LocatorP get_locator() {return locator;}

  virtual ITcommand get_command();
  /* EFFECTS: Gets the command. */
  /* NOTE: command is not clocked. */

  Boolean get_mapped_next() {return mappedNext;}

  void set_mass_next(Mass mss) {massNext = mss;}

  void creation_failed() {creationOk = False;}
  /* EFFECTS: Used by children to mark that the creation of the Physical
     from a stream failed. */

  virtual void update_area_next(Boolean currentAndNext);
  /* EFFECTS: Called to adjust for any change in the size and position of 
     the Physical.  Used in two ways.  
     (1) During the act() phase it is called 
     with False anytime something effects the next values affecting area, 
     may be called before or after this->act().  
     (2) Outside of the clock, e.g. when reading from a stream, it is 
     called with True.  Right now this only happens when one object calls
     follow() on another. */
  

private:
  void _update_area_next(Boolean currentAndNext);
  /* EFFECTS: Handle making object follow(). */

  void init_static();

  static Boolean staticValid;
  static Acc unitAccs[CO_DIR_MAX]; 
  static Vel unitVels[CO_DIR_MAX]; 
  Id id;
  WorldP world;
  LocatorP locator;
  ITcommand command;
  const PhysicalContext *pc; 
  Health health, healthNext;
  Mass mass, massNext;
  Id dontCollide;
  CompositeId dontCollideComposite;
  IntelP intel;
  Boolean deleteMe;
  Boolean mapped,mappedNext;
  Boolean noDeathDelete; // Should set_delete_me be called at death.
  Boolean dieCalled;
  int heat, heatNext;
  Boolean previousHeatWasSecondary;
  Timer flashTimer;
  Boolean quietDeath;
  NetDataP netData; // Only allocated when needed.
  Boolean creationOk;
};
// PhysicalP defined in locator.h



////////// Protection
// Parent: Physical
// Created by a Shield.  Protects a Moving against various types of attacks.
// Not a subclass of Moving because we don't want to worry about hitting 
// walls.
struct ProtectionContext {
  CMN_COLOR colorName;
  PhysicalContext physicalContext;
};


class Protection : public Physical {
public:
  Protection(const ProtectionContext &,ProtectionXdata &,
	     WorldP,LocatorP,const Area &);

  DECLARE_INTERNAL_NODE_IO(Protection);

  virtual Boolean collidable();
  virtual const Area &get_area();
  virtual const Area &get_area_next();

  virtual void follow(const Area &followeeArea,Dir followeeDir,
                      Boolean currentAndNext);
  ///////// Following comments only apply if currentAndNext is False.
  /* NOTE: Can be called multiple times in one turn. */
  /* EFFECTS: Called in act() phase by the Moving being protected. 
   aNext is the next Area of the Moving.  Can be called before or after
   this->act(). */

  virtual Boolean corporeal_protect(int damage) = 0;
  virtual Boolean heat_protect(int heat,Boolean secondary) = 0;
  /* EFFECTS: The Moving has received an attack.  Return wheter the Protection
     will protect against the attack. */

  virtual void draw(CMN_DRAWABLE,Xvars &,int,const Area &);

  virtual void act();
  virtual void update();

  virtual void dr_clock();


#ifndef PROTECTED_IS_PUBLIC
protected:
#endif
  virtual void update_area_next(Boolean currentAndNext);

  static void init_x(Xvars &,IXCommand command,void* arg,const ProtectionContext &,ProtectionXdata &);


private:
  void _act();

  enum {DELTA_MIN = -2,DELTA_MAX = 5};  // DELTA_MIN <= delta < DELTA_MAX


  const ProtectionContext *prc;
  ProtectionXdata *pXdata;
  int delta;  // When drawn, how far the Protection sticks out.
  Area areaBaseNext;
  Area area,areaNext;
};
// ProtectionP defined above.



////////// Moving
// Parent: Physical
// Has all 19 directions.  Multiple pixmaps.  Can change size and position. 
// Top speed is VEL_MAX.

/* Only sizes[CO_air] and offsets[CO_air] are required to be set.  
   Gives initial size and offset. */
struct MovingContext {
  CMN_COLOR foreColorName;
  Boolean foreWhiteDefault;
  CMN_COLOR backColorName;
  Boolean backWhiteDefault;
  int animMax[CO_DIR_MAX];
  Size sizes[CO_DIR_MAX]; // in world coords
  Size depreciated_offsets[CO_DIR_MAX];  
  CMN_BITS_ID pixmapBits[CO_DIR_MAX][PH_ANIM_MAX];
  CMN_BITS_ID maskBits[CO_DIR_MAX][PH_ANIM_MAX];
  PhysicalContext physicalContext;
  // The bitmaps are already in high resolution, don't stretch them.
  // Only used on Win32.
  Boolean hiRes;
  // Used to override behavior for generating offsets.
  // If NULL, use Moving::offset_generator.
  Touching (*offsetGenerator)(Dir); 
  // Used to override behavior for auto-generating bitmaps.
  // Specifies which bitmaps are transformations of other
  // bitmaps.
  TransformMap* transformOverride;
  // The bitmaps are specified as color XPM (X Pixmap).
  // Only used on X11.
  Boolean useXPM;
  // Can this object ever be invisible.  Right now, this only applies
  // to Creatures that use a Cloak.  Masks be generated iff this flag 
  // is True.
  Boolean invisibility;
};



class Moving: public Physical {
public:
  Moving(const MovingContext &m_c,MovingXdata &x_data,
	       WorldP world,LocatorP l,const Pos &rawPos,
	       Dir dirInitial = CO_air);

  Moving();
  /* NOTE: Should never be called. */

  DECLARE_INTERNAL_NODE_IO(Moving);
 
  void patch_size_and_size_next();
  /* EFFECTS: Deal with the problem that client and server may have different 
     ideas about the size of a Moving object. */

  virtual Boolean is_moving();

  virtual const Area &get_area();

  virtual const Area &get_area_next();
  /* NOTE: Should only be used by the Locator or in a protected context. */

  Boolean is_invisible() {return !invisible.ready();}
  /* NOTE: Invisible as long as the invisibility timer is running. */

  virtual Boolean ignore_walls();
  /* NOTE: Override this to make something able to "walk through walls." */

  virtual Vel get_vel();
  const Pos &get_raw_pos() {return rawPos;}
  virtual Dir get_dir();

  virtual Id get_protection();

  virtual void get_followers(PtrList &list);

  Boolean supports_invisibility() {return mc->invisibility;}
  /* EFFECTS: Can this object be made invisible. */

  virtual void follow(const Area &followeeArea,Dir followeeDir,
                      Boolean currentAndNext);
  
  void set_vel_next(const Vel &vel) {velNext = vel;}
  void set_vel_next(int zero) {assert (zero == 0); velNext.set_zero();}
  /* EFFECTS: Sets the next velocity for the object to be vel.  Can be called
     multiple times before update, only the last call is used. */

  void set_middle_next(const Pos &pos);
  /* EFFECTS: Sets the middle to pos according to the current (not next) values
     of dir, and area (for size). */
  /* NOTE: May be called before or after act phase. */

  void relocate(const Pos &p);
  /* EFFECTS: Moves the raw position of the Moving to p. Can and must be 
     called outside of Locator::clock. */
  /* REQUIRES: Must be relocated to an open area of the World. */
  /* NOTE: Sets current and next values. */

  virtual void set_mapped_next(Boolean val);

  void set_extra_vel_next(const Vel &vel) 
  {extraVelNext = vel; extraVelNextSet = True;}
  void set_extra_vel_next(int zero) 
  {assert (zero == 0); velNext.set_zero(); extraVelNextSet = True;}

  virtual void set_protection(const Id &);

  void set_invisible_next(int turns);
  /* EFFECTS: For turns >= 0, sets object to be invisible for at least 
     the given number of turns.  For turns == -1, turns off invisibility
     next turn. */
  /* REQUIRES: supports_invisibility() is True. */

  void set_attack_hook(void (*hook)(PhysicalP)) {attackHook = hook;}
  /* NOTE: Overrides previous value if any. */

  void attack_hook() {if (attackHook) attackHook(this);}
  /* EFFECTS: Should be called whenever *this attacks anything. */

  virtual Boolean corporeal_attack(PhysicalP killer,int damage,AttackFlags); 
  /* EFFECTS: Return False if attack was blocked. */

  virtual void heat_attack(PhysicalP,int heat,Boolean secondary = False);

  virtual int new_anim_num(int animPrev,int animMax);
  /* EFFECTS: Override to determine behavior of animation.  Returns
    new animNum given the old value and the maximum animNum for that
    direction. */

  virtual void act();
  virtual void update();
  virtual void draw(CMN_DRAWABLE,Xvars &,int,const Area &);
  virtual void avoid(PhysicalP);
  virtual void collide(PhysicalP);
  virtual void die();
  virtual void dr_clock();

  const MovingContext *get_moving_context() {return mc;}
  /* NOTE: Now public so we can tell when we can turn something into a Sticky. 
     Perhaps we should just export the list of animMax. */

  static Touching offset_generator(Dir dir);
  /* EFFECTS: The default offset_generator.  Map a direction to the
     "wall" it touches. */


#ifndef PROTECTED_IS_PUBLIC
protected:
#endif
  Boolean hit_wall() {return hitWall;}
  Boolean hit_wall_next() {return hitWallNext;}

  virtual Dir get_dir_next();

  Vel get_vel_next() {return velNext;}

  virtual int get_anim_time();
  /* EFFECTS: Override this to make objects animate faster or slower.
     Returns the number of turns per frame.  Default is MOVING_ANIM_TIME. */

  Size *get_offsets();
  /* EFFECTS: Return array of size CO_DIR_MAX with all the offsets for
     this Moving. */
  /* DEFAULT: Compute generic offsets from Coord::generate_offsets(). */

  virtual Boolean draw_outline(ColorNum &colorNum);
  /* MODIFIES: colorNum */
  /* EFFETS: Override this to control drawing the four dots in the 
     corners around the object.  colorNum should be set if True is returned. */

  virtual Boolean generate_masks();
  /* EFFECTS: On Windows decide whether to auto-generate masks for 
     this object. */
  /* DEFAULT: False */

  void set_vel(const Vel &v) {vel = v;}
  void set_vel(int zero) {assert (zero == 0); vel.set_zero();}

  void set_dir(const Dir &d) {dir = d;}
  /* NOTE: Only used by Lance for initialization. */

  void set_dir_next(const Dir &d) {dirNext = d;}

  void _set_middle_next(const Pos &pos);
  /* NOTE: Nothing special about this, just like set_dir_next() or 
     set_raw_pos_next() in that it just sets the next variables, does not
     call update_area_next. */

  void set_mover_vel(const Size &v) {moverVel = v;}
  /* EFFECTS: Called in act phase before Moving::act(), only lasts for that
      turn.  Used by Creature::act() to follow a Mover. */

  void set_raw_pos_next(const Pos &rpos)
    {rawPosNext = rpos; rawPosChanged = True;}

  void update_area_next(Boolean currentAndNext);
  /* EFFECTS: Compute areaNext and hitWallNext.  May modify rawPosNext or 
     dirNext. */
  /* NOTE: May be called more than once per turn. */

  virtual void get_pixmap_mask(Xvars &xvars,int dpyNum,
                               CMN_IMAGEDATA &imageData,
                               Dir dir,int animNum);
  /* MODIFIES: imageData */
  /* NOTE: Only used so that children of Moving can affect Moving's actions. 
     imageData is basically a pixmap and a mask. */
  
  virtual void get_size_offset_next(Size &size,Size &offset,Dir dirNext);
  /* MODIFIES: size, offset */
  /* NOTE: Only used so that children of Moving can affect Moving's actions. */

  void set_size_offset_current_and_next(const Size &size,const Size &offset);
  /* EFFECTS: Sets immediate and next values of size and offset. */
  /* NOTE: Kind of a hack.  Only needs to be called if a child uses 
	 get_size_offset_next() and does something different on the initial state. 
     Only called in child constructor. */

  static void init_x(Xvars &,IXCommand command,void* arg,const MovingContext &,MovingXdata &);
  /* EFFECTS: Initialize all graphics for the concrete class that derives from 
     Moving.  E.g. Ninja::init_x() will call Moving::init_x() to initialize all the
     graphics in the Moving part of the Ninja.   On windows, this is a two-phase
     process.  First init_x() is called with command==IX_ADD.  This just adds the 
     requests for surfaces to the SurfaceManager.  Later, it is called with 
     command=IX_INIT which actually loads the bitmaps into the surfaces.  On UNIX, 
     there is only one phase and command is always IX_INIT. */
  /* NOTE: Now called up the tree. */
  /* NOTE: This method is now poorly names as we are overloading its use.  There is
     now also an IX_LIST command on Windows.  We should really call this gfx_function, 
     or something to indicate that this is a multi-purpose graphics-related function 
     for the class.  Have to go through all the .bitmaps files to fix the naming. */
  /* NOTE: See the platform-specific definition of IXCommand to see all the 
     commands. */


private:
  void _act();
  void _update();
  /* NOTE: Used by act(), update() and by dr_clock(). 
     No special semantics like act(), or update(), or update_area_next().
     Consider _act() to be any other method that sets the next dirs, like
     set_dir_next(). */

  void _update_area_next(Boolean currentAndNext);

  Boolean context_valid();
  /* EFFECTS: Returns True if this->cx is valid, False otherwise. */

  float compute_collision(Mass m1,float v1,Mass m2,float v2);

  static void check_generate_offsets(const MovingContext &,MovingXdata *);
  /* EFFECTS: Make sure offsets have been generated for the MovingContext. */
  /* NOTE: Hack here, using movingXdata to store non-graphics-related info. */

  static void* compute_key(Dir dir,int animNum,Boolean isMask,
                           const MovingContext *,MovingXdata *);
  /* EFFECTS: Compute hash key for storing a bitmap in the SurfaceManager.  Only 
     used on Windows. */

  static void check_auto_generated(const MovingContext&);
  /* EFFECTS: Check that sizes and animMax match up for auto-generated
     bitmaps. */


  MovingXdata *movingXdata;
  int movingAnimNum;
  Timer animTimer;
  const MovingContext *mc;
  Pos rawPos,rawPosNext; Boolean rawPosChanged;
  Area area,areaNext;   
  Dir dir,dirNext;
  Vel vel,velNext; 
  Boolean extraVelNextSet;
  Vel extraVel,extraVelNext; // Follows clock in non-standard way.
  Size moverVel; // Only exists in act() phase.
  Boolean hitWall,hitWallNext;
  Id protection;
  Timer invisible;  // Invisible if the timer is running, i.e. !ready().
  // Number of turns to be invisible, -1 means turn off invisibilty.
  int invisibleNext;  
  void (*attackHook)(PhysicalP);
};
typedef Moving *MovingP;



////////// Shot
// Parent: Moving

struct ShotContext {
  int damage; 
  Speed speed;
  SoundName soundId;
  Boolean repeatingSound;
  MovingContext movingContext;
};


typedef MovingXdata ShotXdata ;


class Shot: public Moving {
public:
  Shot(const ShotContext &,ShotXdata &,WorldP,LocatorP,
       const Pos &,const Id &shooter,
       Dir shotDir,Dir movingDir = CO_air);
  /* NOTE: compId may be COMPOSITE_ID_NONE */  

  DECLARE_NULL_INTERNAL_NODE_IO(Shot);

  const Id &get_shooter() {return shooter;}

  virtual Boolean is_shot();

  virtual void avoid(PhysicalP other);
  virtual void collide(PhysicalP other);

  virtual void act();

  virtual void update();


#ifndef PROTECTED_IS_PUBLIC
 protected:
#endif
  int get_damage() {return context->damage;}


private:
  Boolean firstTurn;
  Id shooter;
  const ShotContext *context;
};



////////// Droplet
// Parent: Moving
// Moving with gravity.  Sticks to walls.

struct DropletContext {
  // Time until droplet disolves.
  int dissolveTime;     
  // How fast the droplets fall.
  // See definition of PulseValue in utils.h
  // Can use PH_GRAVITY.
  PulseValue gravity;
  // How fast to launch the droplet.
  float speedModifier;
  // Can this droplet stick to walls and ceiling.  Else, just to floor.
  Boolean stickWalls;

  MovingContext movingContext;
};

typedef MovingXdata DropletXdata;



class Droplet: public Moving {
public:
  Droplet(const DropletContext &,DropletXdata &,
          WorldP world,LocatorP l,const Pos &rawPos);

  DECLARE_NULL_INTERNAL_NODE_IO(Droplet);

  virtual Boolean collidable();

  virtual void act();

  static ClassId choose_droplet_class(DropletSet);
  /* REQUIRES: DropletSet is not empty, i.e. DROPLET_NONE */
  /* EFFECTS: Choose a specific class of Droplet, given the 
     DropletSet. */

  static PhysicalP create_and_add_if(ClassId,WorldP,LocatorP,
                                     const Pos &);
  /* EFFECTS: If possible, create a droplet of the given type and
     add it to the Locator.  Else, do nothing.  Droplet will 
     enforce the policy of only allowing a limited number of
     Droplets to exist at any given time. */
  /* NOTE: This is the approved way of creating a Droplet during
     normal game play.  We also use the PhysicalContext::create()
     method during network play. */


private: 
  static int get_available_entry(LocatorP l);
  /* EFFECTS: Return index for set_entry() or -1 if none 
    available. */


  // After hit a wall, kill self after a few turns.
  Timer timer;

  // Either flying through the air, or hit a wall.
  Boolean inAir;

  // How much gravity to apply each turn.
  Pulser gravPulser;

  const DropletContext *context;


  // Max number of droplets of all type in the game.
  enum {DROPLET_MAX = 45};
  static Id dropletList[DROPLET_MAX];
  static int dropletMax;
};



////////// Falling
// Parent: Moving
// Moving with gravity.  Falls until it is blocked by the world.  

struct FallingContext {
  MovingContext movingContext;
};


typedef MovingXdata FallingXdata;


class Falling: public Moving {
public:
  Falling(const FallingContext &h_c,FallingXdata &x_data,
	  WorldP world,LocatorP l,const Pos &rawPos,
	  Dir dirInitial = CO_air);
  
  DECLARE_NULL_INTERNAL_NODE_IO(Falling);

  virtual void act();


#ifndef PROTECTED_IS_PUBLIC
protected:
#endif
  virtual Boolean gravity_on();
  /* EFFECTS: Called during act() to see if gravity affects
     the Falling. */
  /* DEFAULT: True */
};



////////// Touchable
// Parent: Falling
// Remembers if it has ever been touched by a Human controlled Physical.

struct TouchableContext {
  FallingContext fallingContext;
};


typedef FallingXdata TouchableXdata;


class Touchable: public Falling {
 public:
  Touchable(const TouchableContext &,TouchableXdata &x_data,
	    WorldP,LocatorP,const Pos &);

  DECLARE_NULL_INTERNAL_NODE_IO(Touchable);

  virtual int get_drawing_level();
  /* EFFECTS:  Touchable is drawn behind everything else. */
  
  Boolean wasTouched() {return touched;}
  /* EFFECTS:  Has a Physical with Human intelligence touched the Xit. */
  
  virtual Boolean collidable();
  virtual void act();


private:
  Boolean touched;
  const TouchableContext *context;
};
typedef Touchable *TouchableP;



//////////// Heavy
// Parent: Falling
// Does damage to things it lands on.

struct HeavyContext {
  int damage;
  FallingContext fallingContext;
};


typedef FallingXdata HeavyXdata;


class Heavy: public Falling {
 public:
  Heavy(const HeavyContext &h_c,
	HeavyXdata &x_data,
	WorldP world,
	LocatorP l,
	const Pos &rawPos);
  
  DECLARE_NULL_INTERNAL_NODE_IO(Heavy);

  virtual void collide(PhysicalP);
  /* EFFECTS: Crush things it falls on. */

  void set_always_hurts_in_air() {alwaysHurts = True;}
  /* EFFECTS: Usually a Heavy only hurts things below it and only when
     it is going at least a certain speed down.  The always_hurts()
     flag makes the Heavy hurt anything as long as it is in the air. */
  /* NOTE: Kind of a hack to make the LookOut scenario work right. */


#ifndef PROTECTED_IS_PUBLIC
protected:
#endif
  void set_pusher(const Id &id) {pusher = id;}
  /* EFFECTS: Used by Liftable to give the Lifter credit for kills. */


 private:
  Id pusher; // Who last touched the heavy.
  Boolean alwaysHurts;
  const HeavyContext *context;
};
typedef Heavy* HeavyP;



//////////// Liftable
// Parent: Heavy
// Can be carried by a Lifter

struct LiftableContext {
  HeavyContext heavyContext;
};

typedef HeavyXdata LiftableXdata;

class Liftable: public Heavy {
 public:
  Liftable(const LiftableContext &h_c,
           LiftableXdata &x_data,
           WorldP world,
           LocatorP l,
           const Pos &rawPos);
  
  DECLARE_NULL_INTERNAL_NODE_IO(Liftable);

  virtual Boolean is_liftable();

  virtual int get_drawing_level();

  Boolean being_lifted();
  /* EFFECTS: Return if something is already lifting this Liftable. */

  virtual void update();
  
  virtual void collide(PhysicalP);
  /* NOTE: Don't hurt anything if being lifted. */
  
  void lift(PhysicalP lifter);
  /* EFFECTS: A lifter has started lifting the Liftable. */

  void release(PhysicalP lifter);
  /* EFFECTS: A lifter has dropped/thrown the Liftable. */
    

private:
  Id lifter;  // valid iff being lifted or excludedTime.
  Timer excludedTimer;
  Boolean excludedTime; // Was just dropped, don't pick up again.
};
typedef Liftable *LiftableP;

  

//////////// Item
// Parent: Falling
// 
struct ItemContext {
  Boolean persists; 
  // Provides ranking for weapons/items when switching.  
  // No two weapons should have the same coolness and no two items
  // should have the same coolness.  
  // Negative coolness means that the item should never be automatically 
  // selected.
  int coolness; 
	SoundName useSound;
  FallingContext fallingContext;
};


typedef FallingXdata ItemXdata;


class Item: public Falling {
public:
  Item(const ItemContext &c_x,
       ItemXdata &x_data,
       WorldP w,
       LocatorP l,
       const Pos &pos,Dir dirInitial = CO_air);
  
  DECLARE_INTERNAL_NODE_IO(Item);

  Boolean is_item() {return True;}

  Boolean is_held() {return held;}
  
  Boolean can_take(PhysicalP taker);
  /* EFFECTS: Returns whether the object can be picked up by taker. */

  virtual int get_drawing_level();

  Id get_holder_id();
  /* EFFECTS: Returns the holder Id if held or an invalid Id if not held.  */

	virtual int get_coolness();
	/* NOTE: Coolness may change for an item. */

  void set_quiet_death();

  Boolean persists() {return context->persists;}

  virtual void taken(PhysicalP);
  /* EFFECTS:  The object has been taken by another Physical. */
  /* NOTE: Changes immediate externally visible state.  Should only be called
     in the collision phase. */

  void dropped(PhysicalP);
  /* EFFECTS:  The object has been dropped by another Physical. */
  /* NOTE: Specially coded to work in both the act and die phases. */

  virtual void use(PhysicalP);
  /* EFFECTS: p uses *this. */
  /* NOTES: Called by another object in act phase.  Can also be called in
     collide phase. (see AutoUse::collide)  Calls up the tree. */

  virtual void act();
  
  virtual void die();
  
  
#ifndef PROTECTED_IS_PUBLIC
protected:
#endif
  virtual Boolean gravity_on();
  /* NOTE: Don't want to be affected by gravity while held. */

  void set_cant_take() {cantTake = True;}
  
  
private:
  enum MESSAGE {NONE,USED,DESTROYED};

  Boolean held;
  Id holderId; // Valid iff held.
  Boolean dieMessage;

  //  Timer canTake;
  // While excludedTimer is not ready(), excluded cannot take this.
  Id excluded;
  Timer excludedTimer;
  
  Boolean cantTake;
  const ItemContext *context;
};
typedef Item *ItemP;



//////////// AutoUse
// Parent: Item
// Automatically gets used when it collides with a non-user creature.
// WARNING: All children of this class must be able to be used by a non-user.

struct AutoUseContext {
  Boolean humansOnly; // Only automatically used by humans.
  ItemContext itemContext;
};


typedef ItemXdata AutoUseXdata;


class AutoUse : public Item {
public:
  AutoUse(const AutoUseContext &c_x,
	  AutoUseXdata &x_data,
	  WorldP w,
	  LocatorP l,
	  const Pos &pos);

  DECLARE_NULL_INTERNAL_NODE_IO(AutoUse);

  virtual Boolean is_auto_use();
  
  virtual void collide(PhysicalP);


private:
  const AutoUseContext *context;
};



//////////// Drugs
// Parent: AutoUse
struct DrugsContext {
  int odChance;  // zero means no chance of OD
  AutoUseContext autoUseContext;
};

typedef AutoUseXdata DrugsXdata;

class Drugs : public AutoUse {
public:
  Drugs(const DrugsContext &c_x,DrugsXdata &x_data,WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_INTERNAL_NODE_IO(Drugs);

  virtual Boolean is_drugs();

  virtual void taken(PhysicalP);

  virtual void use(PhysicalP);


#ifndef PROTECTED_IS_PUBLIC
  protected:
#endif
  virtual ModifierP create_modifier() = 0;
  /* EFFECTS: Override to give drug of your choice. */

  const DrugsContext *context;
};



//////////// Shield
// Parent: AutoUse
struct ShieldContext {
  AutoUseContext autoUseContext;
};


typedef AutoUseXdata ShieldXdata;


class Shield: public AutoUse {
public:
  Shield(const ShieldContext &,ShieldXdata &,
	 WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_INTERNAL_NODE_IO(Shield);

  virtual Boolean is_shield();
  
  virtual void use(PhysicalP);

  
#ifndef PROTECTED_IS_PUBLIC
protected:
#endif
  virtual ProtectionP create_protection(const Area&) = 0;
  /* EFFECTS: Create the type of Protection specific to this type of Shield. */
};



//////////// Animated
// Parent: Item
//
struct AnimatedContext {
  ItemContext itemContext;
};

typedef ItemXdata AnimatedXdata;

class Animated: public Item {
public:
  Animated(const AnimatedContext &,AnimatedXdata &,
	   WorldP,LocatorP,const Pos &,Frame frameInitial = CO_R);
  /* NOTE: CO_R should really be FRAME_BASE, lame compilers. */

  DECLARE_NULL_INTERNAL_NODE_IO(Animated);


#ifndef PROTECTED_IS_PUBLIC
protected:
#endif
  void set_frame(Frame fr) {set_dir(fr + FRAME_BASE);}
  void set_frame_next(Frame fr) {set_dir_next(fr + FRAME_BASE);}
  Frame get_frame() {return get_dir() - FRAME_BASE;}


private:
  enum {FRAME_BASE = CO_R};
};



//////////// Weapon
// Parent: Item
// 
struct WeaponContext {
  // Defaultable has been superceeded by using negative values for
  // ItemContext::coolness.
  SoundName soundid;
  ItemContext itemContext;
};


typedef ItemXdata WeaponXdata;


class Weapon;
typedef Weapon *WeaponP;


class Weapon: public Item {
public:
  Weapon(const WeaponContext &c_x,
	 WeaponXdata &x_data,
	 WorldP w,
	 LocatorP l,
	 const Pos &pos,Dir dirInitial = CO_air);
  
  DECLARE_NULL_INTERNAL_NODE_IO(Weapon);

  Boolean is_weapon() {return True;}

  virtual Boolean ready() = 0;
  /* EFFECTS: Can the weapon be fired now. */
  /* NOTE: Sometimes calls up the tree.*/

  //  Boolean defaultable() {return wc->defaultable;}
  /* EFFECTS: Is this a type of weapon that can safely be set automatically as
     the current weapon.  E.g. You do not want to set a soul-swapper as the 
     current weapon unless the user explicitly says so. */

  virtual int get_ammo() = 0;
  virtual int get_ammo_max() = 0;
  /* NOTE: Can return PH_AMMO_UNLIMITED. */

  virtual Boolean useful_no_ammo();
  /* EFFECTS: Is this weapon useful even if it is out of ammo.  (Otherwise User 
    will kill it when it runs out.) */
  /* DEFAULT: False */

  virtual Boolean fire_repeatedly();
  /* EFFECTS: Should this weapon be fired repeatedly if the user holds down the 
     fire key. */
  /* DEFAULT: True */

  virtual void fire(const Id &id,ITcommand command);
  /* REQUIRES: command is a weapon command. */
  /* EFFECTS: Fire the weapon according to the specified command.  id is the
     physical firing the weapon.  */

  virtual void use(PhysicalP);
  /* NOTE: Does nothing. Doesn't even call Item::use(). */

  virtual void enter_scope_next(PhysicalP user);
  virtual void leave_scope_next(PhysicalP user);
  /* NOTE: Called during act(collide) or update phase. Should be just 
     act(). */
  /* NOTE: Calls up the tree. */
  /* NOTE: ok to call multiple times in same turn, but must enter/leave scope
     in proper order. */

  virtual void take_ammo_from(WeaponP other);
  /* EFFECTS: Take as much ammo as possible from the other weapon. */


#ifndef PROTECTED_IS_PUBLIC
 protected:
#endif
  Boolean entered_scope() {return enteredScope;}

  virtual void set_ammo(int) = 0;

  virtual Dir compute_weapon_dir(ITcommand c);
  /* EFFECTS:  Returns the direction the weapon should be fired, 
     {CO_R ... CO_UP_R}, or CO_air. */


private:
  const WeaponContext *wc;
  Boolean enteredScope; // not clocked
};
// typedef Weapon *WeaponP; Defined above.



//////////// Cutter
// Parent: Weapon
// NOTE: Uses CO_center for cutter directly in front of user.
struct CutterContext {
  int damage;  // per turn
  Size offsets[CO_DIR_MAX];  // From User's middle to Cutter's middle.

  SoundName cuttingsound;
  WeaponContext weaponContext;
};

typedef WeaponXdata CutterXdata;

class Cutter: public Weapon {
public:
  Cutter(const CutterContext &c_x,CutterXdata &x_data,
	 WorldP w,LocatorP l,const Pos &pos);
  
  DECLARE_INTERNAL_NODE_IO(Cutter);

  virtual Boolean is_cutter();

  virtual Boolean ready();

  virtual int get_ammo();
  virtual int get_ammo_max();

  virtual Boolean ignore_walls();
  /* NOTE: We don't want cutters that are held to get bounced through walls. */

  virtual void set_ammo(int);

  virtual void follow(const Area &,Dir,Boolean);

  virtual void enter_scope_next(PhysicalP);
  virtual void leave_scope_next(PhysicalP);

  virtual void collide(PhysicalP);

  virtual void update();

  virtual void take_ammo_from(WeaponP);
  /* EFFECTS: "Heal" up the chainsaw as if chainsaws had been switched. */

  
private:
  enum {UNHELD_DIR = CO_R};
  
  Dir dir_4_from_user_dir(Dir);

  Boolean inScope,inScopeNext;
  Id killerId;  // Valid iff inScope.
  const CutterContext *context;
};



//////////// Gun
// Parent: Weapon
// Shoots shells.  Has timer between allowed shots.
struct GunContext {
  int shotTime;
  int ammoInitial;
  int ammoMax;  
  WeaponContext weaponContext;
};
typedef WeaponXdata GunXdata;


class Gun: public Weapon {
public:
  Gun(const GunContext &,GunXdata &,WorldP,LocatorP,const Pos &);

  DECLARE_INTERNAL_NODE_IO(Gun);

  virtual Boolean is_gun();
  virtual Boolean ready();
  int get_ammo();
  int get_ammo_max();

  virtual void update();


#ifndef PROTECTED_IS_PUBLIC
protected:
#endif
  void set_ammo(int);
  void set_shot_timer() {timer.set();}

  virtual Size get_shot_size(Dir);

  virtual void fire(const Id &id,ITcommand command);
  void _fire(const Id &id,Dir dir,Boolean setTimer = True,
	     Boolean costsAmmo = True);
  /* NOTE: Only the first of these two functions should be overridden. */
  /* EFFECTS: Gun is fired by the shooter id with ITcommand command or in Dir
     dir.  setTimer says whether or not to force a delay between shots. 
     costsAmmo says whether a unit of ammo is subtracted for the shot.*/
  
  virtual PhysicalP create_shot(PhysicalP shooter,WorldP world,
				LocatorP locator,const Pos &pos,Dir dir);
  /* EFFECTS: Create the object to be fired out of the gun.  E.g. shell, 
     swap_shell. shooter is the object firing the gun. */
  /* DEFAULT: Create a Shell. */


private:
  Timer timer;
  int ammo;
  const GunContext *gc;
};



//////////// SingleGun
// Parent: Gun
struct SingleGunContext {
  GunContext gunContext;
};
typedef GunXdata SingleGunXdata;


class SingleGun: public Gun {
public:
  SingleGun(const SingleGunContext &,SingleGunXdata &,
	    WorldP,LocatorP,const Pos &);

  DECLARE_NULL_INTERNAL_NODE_IO(SingleGun);

  virtual Boolean ready();
  

private:
  virtual void fire(const Id &id,ITcommand command);
  
  Id shotId;
};



//////////// Whistle
// Parent: Weapon
struct WhistleContext {
  int coolnessNoAmmo;
  int ammo;  // Must be <= Whistle::PET_MAX. 
  // Chance that the alternate pet will be created, 0 if no alternate
  // pet is ever created.
  int alternateChance; 
  WeaponContext weaponContext;
};

typedef WeaponXdata WhistleXdata;

class Whistle: public Weapon {
public:    
  Whistle(const WhistleContext &,WhistleXdata &,WorldP w,LocatorP l,const Pos &pos);

  DECLARE_INTERNAL_NODE_IO(Whistle);
  
  virtual Boolean ready();
  
  virtual int get_ammo();
  virtual int get_ammo_max();
  /* NOTE: Can return PH_AMMO_UNLIMITED. */
  
  virtual int get_coolness();
  
  virtual Boolean useful_no_ammo();
  
  virtual Boolean fire_repeatedly();

  virtual void fire(const Id &id,ITcommand command);
  /* REQUIRES: command is a weapon command. */
  /* EFFECTS: Fire the weapon according to the specified command.  id is the
     physical firing the weapon.  */
  
  virtual void act();
  

#ifndef PROTECTED_IS_PUBLIC
protected: 
#endif
  virtual void set_ammo(int);
  
  virtual PetP create_pet(IntelP master,Boolean alternatePet,int n) = 0;
  /* EFFECTS: Create the object that the Whistle controls. 
     Whistles may have the ability to create an alternate pet than the
     ususal one.  alternatePet will be True in this case. 
     0 < n < number of pets created this turn, in case the Whistle wants to 
     do something different for different pets.
     Create and register Intel and Physical.  Return intel. */
  

private:
  enum {PET_MAX = 10};    
  
  IntelId pets[PET_MAX];  // Must all be class Pet.
  int petNum;
  int ammo;
  const WhistleContext *context;
};



////////// Creature
// Parent: Moving
// Moving with stances.  Can be affected by gravity.  Has list of Capabilites.
struct CreatureContext {
  Speed crawlSpeed;
  Speed centerSpeed; 
  Speed airSpeed;
  Speed climbSpeed;
  Speed jump;
  Speed acceleration;
  Health corpseHealth; // A positive number.

  Boolean biological;
  // Logical OR of the possible droplet classes, 
  // e.g. (DROPLET_BLOOD | DROPLET_FEATHER) for Chicken.
  // DROPLET_NONE is the empty set, i.e. no blood droplets.
  DropletSet dropletSet; 

  SoundName deadSoundId;
  MovingContext movingContext;
};

typedef MovingXdata CreatureXdata;  


// Modifier is the base class for all Modifiers as you'd expect.
// But, it's not actually abstract.  You instantiate a Modifier by itself
// to act as the dummy head element in a list of Modifiers.
typedef int Attribute;

class Modifier {
public:
enum {
  // Modifies the default values in creatureContext
  ACCELERATION,AIR_SPEED,CENTER_SPEED,CLIMB_SPEED,CRAWL_SPEED,JUMP, 
  // Damage done to others, fighter, prickly
  DAMAGE, 
  // A Boolean, tells whether Creature is high or not.
  HIGH, 
  // Changes maximum health, not current health
  HEALTH_MAX, 
  // Damage done to self by something else. 
  CORPOREAL_ATTACK,HEAT_ATTACK, 
  // Dummy.
  ATTRIB_NONE
};


/////// Operations acting on this specific Modifier.
  Modifier(int timed = 0,Boolean head = False);
  /* EFFECTS: If head is True, this is the start of a new list of modifiers.
      else, this is a modifier to insert into a list. */

  virtual ~Modifier();
  /* EFFECTS: Delete self and any modifiers following in the chain.
     If you want to just delete a single modifier, make sure to remove
     it from its list first. */

  void clear();
  /* EFFECTS: Make this an empty list.  I.e. only the head. */
  /* REQUIRES: This is the head. */

  virtual ModifierId get_modifier_id();

//  virtual int apply(Attribute attr,int startVal);
  /* EFFECTS: Apply this Modifier to the attribute. */

  virtual void combine(Modifier *other);
  /* EFFECTS: Combine other modifier into this one. */  
  /* NOTE: Call up the tree. */

  void remove();
  /* EFFECTS: Remove self from list. */


/////// List operations, where "this" is the head of the list.
//  void prepend(Modifier *);
  /* EFFECTS: Add to front of list. */

  void append_unique(Modifier *);
  /* EFFECTS: Add to end of list if none with same ModifierId, else use 
      combine() to merge them. */

//  void remove(Modifier *);


/////// List operations on the remainder of list.
  virtual int apply(Attribute attr,int startVal);
  /* EFFECTS: Apply all Modifiers in list starting with this. */

  virtual void clock(PhysicalP p);
  /* EFFECTS: Clock all modifiers in list. */


#ifndef PROTECTED_IS_PUBLIC
protected:
#endif
  virtual void preDie(PhysicalP p);
  /* EFFECTS: Called just before modifier kills itself. */


  Modifier *next;
  Modifier *prev;
  Boolean timed;
  Timer timer;
  

private:
  Boolean isHead; // Is this the dummy Modifer at the head of the list.
};



// doubles an attribute, then triples, then quadruples as you
// combine() more of them.
class Doubler: public Modifier {
public:
  Doubler(int timed = 0);

  void combine(ModifierP);


#ifndef PROTECTED_IS_PUBLIC
protected:
#endif
  // Children can change these values.
  float multiplier;
  float increment;
};



class Creature: public Moving {
public:
  Creature(const CreatureContext &m_c,
	         CreatureXdata &x_data,
           PtrList *abilities,
	         WorldP world,LocatorP l,const Pos &rawPos);
  /* NOTE: Creature owns the memory pointed to by abilities, and will 
    delete it when finished. */

  Creature();
  /* NOTE: Should never be called. */

  // Can't use DECLARE_INTERNAL_NODE_IO() because of the PtrList argument.
  Creature(InStreamP,const CreatureContext &,
           CreatureXdata &,
           WorldP,LocatorP);
  virtual void update_from_stream(InStreamP in);
  void _update_from_stream(InStreamP);
  virtual int get_write_length();
  virtual void write(OutStreamP);
 
  virtual ~Creature();

  virtual Boolean is_creature();

  Stance get_stance() {return stance;}

  Boolean can_climb() {return canClimb;}

  virtual ITcommand get_command();
  /* NOTE: Will return ITnone if the Creature is stunned. */

  const Hanging &get_hanging() {return hanging;}
  /* EFFECTS: Get the corner that the object is hanging off of, or CO_AIR if
     not hanging off a corner. */

  Touching get_touching_area() {return touching;}
  /* EFFECTS: The object's actual touching.  I.e. according to its area. */

  Boolean get_alien_immune() {return !alienImmune.ready();}
  /* EFFECTS: Is Creature immune to Alien and Hugger attacks. */

  ModifierP get_modifiers() {return modifiers;}
  /* EFFECTS: Returns the modifier list for this Creature.  Not a copy. 
     Will never return NULL. */

  virtual void get_followers(PtrList &list);
  /* NOTE: Just for the abilities. */

  Boolean stunned() {return !stunTimer.ready();}

  Boolean biological() {return cc->biological;}
  /* EFFECTS: Is the Creature a biological organism.  E.g. as opposed to a 
     machine. */

  Boolean on_door();
  /* NOTE: Convenience function to say whether *this is on top of a door. */

  Boolean get_want_climb() {return wantClimb;}
  void set_want_climb(Boolean val) {wantClimb = val;}
  /* NOTE: Actually, ALL climbing should be in Creature. */

  virtual Boolean command_repeatable(ITcommand c);
  /* EFFECTS: Don't repeat going through doors, or getting up from a crouch. */

  Boolean standing_squarely();
  /* EFFECTS: Standing squarely on solid ground, not hanging over the edge. */

  virtual ClassId get_weapon_string(const char *&str);

  virtual int get_drawing_level();
  /* NOTE: Just for the abilities. */

  virtual int get_anim_time();
  /* NOTE: Just for the abilities. */

  virtual void follow(const Area &followeeArea,Dir followeeDir,
                      Boolean currentAndNext);
  /* NOTE: Delegate to abilities. */
  
  virtual Boolean swap_protect();
  /* NOTE: Just for the abilities. */

  virtual Boolean frog_protect();
  /* NOTE: Just for the abilities. */

  void stun_next(int time);
  /* EFFECTS: Stun the creature for time turns. */

  virtual void set_intel(IntelP i);
  /* NOTE: Just for informing the abilities. */

  void set_alien_immune(int immTime) {alienImmune.set(immTime);}
  /* EFFECTS: Will be immune to alien and hugger attacks for immTime turns. */
  /* NOTE: Immediate. */

  virtual void set_quiet_death();
  /* NOTE: Calling set_quiet_death on a corpse destroys it. */

  virtual void heal();
  /* NOTE: Just for informing the abilities. */

  virtual void set_mapped_next(Boolean val);
  /* NOTE: Just for informing the abilities. */

  virtual void drop_all(Boolean killNonPersistent);
  /* NOTE: Just for informing the abilities. */

  virtual Boolean corporeal_attack(PhysicalP,int damage,AttackFlags); 
  virtual void heat_attack(PhysicalP,int heat,Boolean secondary); 

  virtual void act();
  virtual void update();
  virtual void die();

  virtual void dr_clock();
  /* NOTE: Just to deal with weirdness with Composite objects. */

  virtual void collide(PhysicalP);

  virtual AbilityP get_ability(AbilityId);
  /* EFFECTS: Return the ability if the Creature has it, else return 
     NULL. */

  virtual HolderP get_holder();
  /* EFFECTS: Return the Holder ability, if the Creature has it, 
     else NULL. */  

  virtual LocomotionP get_locomotion();

  virtual CompositeP get_composite();

  void add_ability(AbilityP);
  /* REQUIRES: The creature does not already have the ability. */
  /* EFFECTS: Add a new abilitity to the Creature. */
  /* NOTE: Creature now owns the memory for the ability. */

  void remove_ability(AbilityId abilityId);
  /* REQUIRES: The creature has the ability in question. */
  /* EFFECTS: Remove ability from the Creature.  Calls the die() method on the
     ability. */

  static const Stats &get_stats() {return stats;}

  static TransformMap uprightTransformOverride;
  /* EFFECTS: A convenience object.  Not actually used by Creature, but useful
     for auto-generating pixmaps for creatures that walk upright when they
     are on the walls or ceiling, e.g. Walker with the Sticky ability.  The
     default in Transform2D is to generate bitmaps for wall and ceiling that
     are rotations of the crawling bitmaps (CO_dn_*).  This lets you generate 
     bitmaps for wall and ceiling that are rotations of the upright walking 
     bitmaps (CO_center_*). */


// Now they must be public so that the children of Ability can 
// access them.  It is not enough to make Ability a friend class.
#if 0
#ifndef PROTECTED_IS_PUBLIC
protected:
#endif
#endif
  enum {DEAD_DIR = CO_R};

  virtual Boolean generate_masks();
  /* EFFECTS: On Windows decide whether to auto-generate masks for 
     this object. */

  Stance get_stance_next() {return stanceNext;}

  virtual Health get_health_max();


  Speed get_acceleration(); 
  Speed get_jump(); 
  // Only used for an assert in Fighter::Fighter
  Touching (*get_offset_generator())(Dir)
  {return cc->movingContext.offsetGenerator;}
  TransformMap* get_transform_override()
  {return cc->movingContext.transformOverride;}
  /* NOTE: These methods replace get_creature_context().  Add more if necessary. */


  Touching get_touching_stance();
  /* EFFECTS: The object's touching according to its stance. */

  time_t get_birth_time() {return birthTime;}

  virtual void get_pixmap_mask(Xvars &,int dpyNum,CMN_IMAGEDATA &imageData,
                               Dir dir,int animNum);
  /* NOTE: Calls through all abilities.  Calls up the tree. */
  /* MODIFIES: pixmap, mask */

  void _get_pixmap_mask(Xvars &,int dpyNum,CMN_IMAGEDATA &imageData,
                        Dir dir,int animNum);
  /* EFFECTS: Creature-specific functionality of get_pixmap_mask(), does not call 
     abilities.  Does call up the tree. */
  /* NOTE: Abilities can use this to implement Ability::get_pixmap_mask(). */

  void _heat_attack(PhysicalP killer,int heat,Boolean secondary);
  /* EFFECTS: Creature-specific functionality of get_pixmap_mask(), does not 
     call abilities.  Called after applying modifiers. Does call up the 
     tree. */
  /* NOTE: Abilities can use this to implement Ability::heat_attack(). */
  
  virtual void get_size_offset_next(Size &size,Size &offset,Dir dirNext);
  /* MODIFIES: size, offset */

  void _get_size_offset_next(Size &size,Size &offset,Dir dirNext);
  /* MODIFIES: size, offset */
  /* EFFECTS: Creature-specific functionality of get_size_offset_next(), does 
     not call abilities.  Does call up the tree. */
  /* NOTE: Abilities can use this to implement Ability::get_size_offset_next(). */

  Boolean stunned_next() {return stunNext ? True : False;}

  int compute_droplets_num(int damage);
  /* EFFECTS: Number of droplets (of blood) to produce when hit with the
     given amount of damage. */

  void set_stance(const Stance &st) {stance = st;}

  void set_stance_next(const Stance &stance);
  /* EFFECTS: Sets the next stanceection for the object to be stance.  Can be
     called multiple times before update, only the last call is used. */

  void set_grav_next(const Grav &g) {gravNext = g;}
  /* EFFECTS: Sets the pull of gravity for the next turn to be grav.
     g may be negative, special meaning.  If grav is -X, then add one
     increment of velocity every X turns.  I.e. -X really means an acceleration
     of 1/X. */
  /* NOTE: This assumes that gravity always pulls downward.  Will need to 
     implement some other way of representing fractional gravity if we want to
     create "upside-down world" or something like that, i.e. where gravity 
     really would be a negative number. */

  void center_wsquare_x_next(const Loc &loc);
  /* REQUIRES: loc overlaps with the area, stanceNext set to CO_climb */
  /* EFFECTS: Tries to center the x position object on the wsquare at loc.  
     (May be bumped a bit if a wall prevents exact centering.) */

  void corner(const Hanging &hanging);
  /* REQUIRES: stanceNext is set to correspond to h.corner. */
  /* EFFECTS: Tries to move the object around the corner it is hanging off of.
     Note that hanging.corner is the initial corner, not the desired final 
     one. */

  int apply_modifiers(Attribute attr,int initial,int maximum = -1);
  /* EFFECTS: Apply Intel and Creature modifiers to attr, starting with
    initial.  Clamp to maximum if supplied. */
  /* NOTE: Modifier::CORPOREAL_ATTACK has special behavior.  Will return 
     negative of the value if should do "superficial" damage, i.e.
     call Physical::corporeal_attack() without the ATT_DAMAGE flag. */

  // Must make these public for Ability to get to them.
  LocatorP get_locator() {return Moving::get_locator();}  
  Boolean get_mapped_next() {return Moving::get_mapped_next();}
  Boolean alive_next() {return Moving::alive_next();}
  virtual Dir get_dir_next();
  void set_raw_pos_next(const Pos &rpos) {Moving::set_raw_pos_next(rpos);}
  WorldP get_world() {return Moving::get_world();}
  void _set_middle_next(const Pos &pos) {Moving::_set_middle_next(pos);}
  virtual void update_area_next(Boolean currentAndNext);


private:
  virtual Boolean stick_touching(Touching touching);
  /* EFFECTS: Should the Creature stick to walls in the specified direction. */

  static Dir compute_dir(const Stance &stance,const Vel &vel);
  /* USES: nothing */
  /* EFFECTS: Return the dir corresponding to stance and vel. */

  Boolean context_valid();

  void initialize_abilities(PtrList *caps);

  void check_abilities();

  ABBitField compute_abilities_bit_field();


  Touching touching;
  MoverId touchingMoverId;
  Hanging hanging;
  Boolean canClimb;
  Stance stance,stanceNext; 

  // See notes on set_grav_next() about meaning of negative values.
  Grav grav,gravNext; 
  Timer gravTimer;

  const CreatureContext *cc;
  Timer corpseTimer;
  Boolean wantClimb;  // actually, ALL climbing should be in Creature.
  //  Id killerId;   Now in Intel.
  time_t birthTime;
  static Stats stats;

  // If non-zero, indicates how many turns Creature should be stunned for.
  int stunNext; 
  Timer stunTimer;
  Timer alienImmune;

  ModifierP modifiers;

  // Could make this a PtrList to save space at the cost of lookup time.
  AbilityP abilities[AB_MAX];

// Don't cache, so we don't have to worry about maintaining the cache.
//  HolderP holder;
//  LocomotionP locomotion;
};
typedef Creature *CreatureP;



typedef AbilityP (*AbilityCreateFunctionType)(InStreamP,CreatureP);

class Ability {
public:
  Ability();

  virtual ~Ability();

  Ability(InStreamP,CreatureP);
  /* EFFECTS: Stream ctor. */

  virtual void initialize(CreatureP);
  /* EFFECTS: Called immediately after Creature has finished constructing itself.
     Pass in the Creature. */
  /* NOTE: This is not used for the stream constructor. */

  virtual void update_from_stream(InStreamP in);
  /* DEFAULT: do nothing */

  virtual int get_write_length();
  /* DEFAULT: return 0 */

  virtual void write(OutStreamP);
  /* DEFAULT: do nothing */


  virtual AbilityId get_ability_id() = 0;

  virtual Boolean is_holder();
  /* DEFAULT: False */

  virtual Boolean is_locomotion();
  /* DEFAULT: False */
  
  virtual Boolean is_composite();
  /* DEFAULT: False */

  virtual Boolean get_pixmap_mask(Xvars &,int dpyNum,CMN_IMAGEDATA &,Dir,int);
  virtual Boolean get_size_offset_next(Size &size,Size &offset,Dir dirNext);
  /* DEFAULT: return False */

  virtual ClassId get_weapon_string(const char *&str);
  /* MODIFIES: str */
  /* DEFAULT: return A_None */
  /* NOTE: Must only modify str if something other than A_None is 
     returned. */

  virtual void drop_all(Boolean killNonPersistent);
  /* DEFAULT: do nothing */  

  virtual void get_followers(PtrList &list);
  /* DEFAULT: do nothing */

  virtual Boolean follow(const Area &followeeArea,Dir followeeDir,
                         Boolean currentAndNext);
  /* DEFAULT: return False */

  virtual Boolean command_repeatable(ITcommand);
  /* DEFAULT: True. */

  virtual int get_drawing_level();
  /* EFFECTS: Return the drawing level if the Ability wants to override the
     Creature's behavior, else return -1. */
  /* DEFAULT: return -1 */

  virtual int get_anim_time();
  /* EFFECTS: Return the anim time if the Ability wants to override the
     Creature's behavior, else return -1. */
  /* DEFAULT: return -1 */

  virtual Boolean heat_attack(PhysicalP killer,int heat,Boolean secondary);
  /* DEFAULT: return False */

  virtual Boolean swap_protect();
  /* DEFAULT: return False */

  virtual Boolean frog_protect();
  /* DEFAULT: return False */
  
  virtual void set_intel(IntelP i);
  /* DEFAULT: do nothing. */

  virtual void heal();
  /* DEFAULT: do nothing */

  virtual void set_mapped_next(Boolean val);
  /* DEFAULT: do nothing */

  virtual void act();
  /* DEFAULT: do nothing */

  virtual void update();
  /* DEFAULT: do nothing */

  virtual void die();
  /* NOTE: Also called when the ability is removed from a Creature. */  
  /* DEFAULT: do nothing */

  virtual Boolean collide(PhysicalP other);
  /* DEFAULT: return False */

  virtual Boolean stick_touching(Touching touching);
  /* EFFECTS: Does this Ability explicitly give the ability to stick when 
     touching in the specified direction.  False means no extra ability is 
     granted. */     
  /* DEFAULT: return False */

  static void register_abilities();
  /* EFFECTS: Must be called before any abilities have been read from stream. */

  static AbilityP create_from_stream(InStreamP in,AbilityId aId,CreatureP);
  /* EFFECTS: Create and initialize() an ability of type aId from the stream.  
     The ability will be used for the given Creature. */


#ifndef PROTECTED_IS_PUBLIC
protected:
#endif
  CreatureP cre; // short for easy use.

  // Static list of all create_from_stream() functions.
  static AbilityCreateFunctionType create_function[AB_MAX];
};



///////// Locomotion
// Gives the ability to move in some fashion.
class Locomotion: public Ability {
public:
  Locomotion();
  Locomotion(InStreamP,CreatureP);

  virtual Boolean is_locomotion();
};



///////// Grounded
// Sticks to the ground only.  Affected by gravity.  Can't climb or jump. 
struct GroundedContext {
  int dummy;
}; 


struct GroundedXdata {};


class Grounded: public Locomotion {
 public:
  Grounded(const GroundedContext &,GroundedXdata &);

  DECLARE_NULL_ABILITY_IO(Grounded);
  
  virtual AbilityId get_ability_id();

  virtual void act();

  static GroundedContext defaultContext;
  static GroundedXdata defaultXdata;
};



//////// Suicide
// Kills self if the command IT_ITEM_USE is seen.
struct SuicideContext {
  int dummy;
};


struct SuicideXdata {};


class Suicide: public Ability {
 public:
  Suicide(const SuicideContext &,SuicideXdata &);

  DECLARE_NULL_ABILITY_IO(Suicide);

  virtual AbilityId get_ability_id();
  
  virtual void act();

  static SuicideContext defaultContext;
  static SuicideXdata defaultXdata;
};



////////// Hopping
// Parent: Creature
// 
struct HoppingContext {
  int dummy;
}; 


struct HoppingXdata {};


class Hopping: public Locomotion {
 public:
  Hopping(const HoppingContext &,HoppingXdata &);

  DECLARE_NULL_ABILITY_IO(Hopping);
  
  virtual AbilityId get_ability_id();

  virtual void act();

  static HoppingContext defaultContext;
  static HoppingXdata defaultXdata;
};



///////// Holder
// Can hold weapons and/or items.
class Holder: public Ability {
public:
  Holder();
  Holder(InStreamP,CreatureP);

  virtual Boolean is_holder();

  virtual int get_weapons_num() = 0;
  virtual int get_items_num() = 0;
  /* NOTE: Returned value is not valid after current turn. */
  
  virtual PhysicalP get_weapon(int) = 0;
  virtual PhysicalP get_item(int) = 0;
  virtual PhysicalP get_weapon_current() = 0;
  virtual PhysicalP get_item_current() = 0;
  /* NOTE: Can return NULL. */
  /* IMPLEMENTATION: get_item_current() should try to return non-NULL if 
     get_items_num() is > 0.  Unlike weapons, there should always be an
     item ready if possible.  We don't quite live up to this right now, as
     User and Carrier count the items in act() when they should do it in
     update() to see if any items have been killed that turn. */

  virtual int get_item_count() = 0;
  /* EFFECTS: If get_item() is non-NULL, return the number of that item
     the Physical is carrying.  Elese return 0. */

  // already has get_weapon_string.

  Boolean ok_to_hold(PhysicalP other);
  /* EFFECTS: Checks a) cre is alive
                     b) other is an Item and can_take()
                     c) no conflicts with other Abilities (important).
   */
  /* NOTE: Now is public, so OnFire knows not to burn stuff. */
};



////////// User
// Holds and uses items and weapons.  The items and weapons are made to follow
// the User. 
struct UserContext {
  Boolean usesWeapons;
  Boolean usesItems;
};  


struct UserXdata {};


class User: public Holder {
public:
  User(const UserContext &,UserXdata &);

  DECLARE_ABILITY_IO(User);

  virtual AbilityId get_ability_id();

  virtual int get_weapons_num();
  virtual int get_items_num(); 
  /* NOTE: Returned value is not valid after current turn. */
  
  virtual PhysicalP get_weapon(int);
  virtual PhysicalP get_item(int); 
  virtual PhysicalP get_weapon_current();
  virtual PhysicalP get_item_current();
  /* NOTE: Can return NULL. */

  virtual int get_item_count();

  virtual ClassId get_weapon_string(const char *&str);
  /* EFFECTS: str is set to point to a static string containing the string
     for ClassId. */

  virtual void get_followers(PtrList &list);

  virtual Boolean command_repeatable(ITcommand c);

  virtual void set_mapped_next(Boolean val);
  /* EFFECTS: Bring current weapon in/out of scope and then change mapped. */

  virtual void drop_all(Boolean killNonPersistent);

  virtual void act();

  virtual void update();

  virtual Boolean collide(PhysicalP);

  virtual void die();

  static UserContext defaultContext;
  static UserXdata defaultXdata;


private:
  Boolean has_weapon(Weapon **weapon,ClassId classId);
  /* MODIFIES: weapon */
  /* EFFECTS: Like has_weapon(ClassId) except will return the 
     weapon in weapon if not NULL. */

  int coolest_weapon();
  /* EFFECTS: Return index of weapon with highest coolness or weaponsNum if 
     builtIn weapon is coolest.  Only consider weapons with nonNegative 
     coolness. */

  int coolest_item();
  /* EFFECTS: Return index of coolest item or itemsNum. */

  int next_coolest_weapon();
  /* EFFECTS: Return the coolest weapon (or builtIn weapon) that is less cool
     than the current weapon. Can be non-negative coolness.  If none found, 
     return coolest_weapon(). */

  int next_coolest_item();
  /* EFFECTS: Return coolest item less cool than current item.  Cycle around
     to coolest_item() if none found. */

  //  int defaultable_weapon();
  /* REQUIRES: weaponsNum and weapons are properly updated. */
  /* EFFECTS: Returns the index of a wepon in weapons that is defaultable or
     weaponsNum if there is none. */

  void weapon_use(ITcommand command);
  void weapon_change();
  void weapon_drop(Boolean killNonPersistent = False);
  void item_use();
  void item_change();
  void item_drop(Boolean killNonPersistent = False);
  /* NOTE: Called in User::_act(). */

  int item_class_count(ClassId);
  /* EFFECTS: Returns the number of items held of the given class. */

  Id weapons[PH_WEAPONS_MAX];
  int weaponsNum;
  // weaponCurrent == weaponsNum if using builtIn weapon or no weapon.
  int weaponCurrent; 
  // If weaponCycleTimer is not ready(), cycle to next weapon on IT_WEAPON_CHANGE.
  // If ready(), start again from coolest_weapon().
  Timer weaponCycleTimer;
  //  Memory of previous weapon dir is now in ui.

  Id items[PH_ITEMS_MAX];
  int itemsNum;
  // itemCurrent == itemsNum if no selected item.
  int itemCurrent; 
  // Like weaponCycleTimer.
  Timer itemCycleTimer;
  // How many of the current item the User has.
  int itemCount,itemCountNext;
  
  const UserContext *context;
};
typedef User *UserP;



////////// Carrier
// Holds one item or weapon.  The items and weapons are made to follow
// the Carrier.  Can use items, but not weapons. 
struct CarrierContext {
    int dummy; 
};


struct CarrierXdata {};


class Carrier: public Holder {
public:
  Carrier(const CarrierContext &,CarrierXdata &);

  DECLARE_NULL_ABILITY_IO(Carrier);
  
  virtual AbilityId get_ability_id();

  virtual int get_weapons_num();
  virtual int get_items_num(); 
  /* NOTE: Returned value is not valid after current turn. */
  
  virtual PhysicalP get_weapon(int);
  virtual PhysicalP get_weapon_current(); 
  virtual PhysicalP get_item(int); 
  virtual PhysicalP get_item_current();
  /* NOTE: Can return NULL. */

  virtual int get_item_count();

  virtual void get_followers(PtrList &list);
  
  virtual void drop_all(Boolean killNonPersistent);

  virtual void act();

  virtual Boolean collide(PhysicalP);

  virtual void die();

  static CarrierContext defaultContext;
  static CarrierXdata defaultXdata;


private:
  Boolean hasItem;
  Id item;
};
typedef Carrier *CarrierP;



////////// Fighter
// Can attack in different directions.
struct FighterContext {
  CMN_COLOR foreground;  // default = black
  CMN_COLOR background;  // default = white
  int slide;
  int jumpHorizontal;
  int jumpVertical;
  int damageStuck;
  int damageFree;

  Size hotSpots[CO_DIR_MAX];  // Must add in offset to use.
  SoundName attackSound;
};


// Bitmaps stored in FighterXdata have all moved to MovingXdata.
struct FighterXdata {};


enum Attack {attackNone, attackStuck, attackFree};


class Fighter: public Ability {
public:
  Fighter(const FighterContext &f_c,FighterXdata &x_data);
  /* NOTE: This is currently the only Ability that uses its xdata.  Also, 
     the only one
     that needs the correct context if we are a Client. */

  DECLARE_ABILITY_IO(Fighter);

  virtual void initialize(CreatureP);

  virtual AbilityId get_ability_id();

  virtual Boolean get_pixmap_mask(Xvars &,int dpyNum,CMN_IMAGEDATA &imageData,
                               Dir dir,int animNum);
  
  virtual Boolean get_size_offset_next(Size &size,Size &offset,Dir dirNext);

  virtual ClassId get_weapon_string(const char *&str);

  virtual void act();

  virtual void update();

  virtual Boolean collide(PhysicalP);

  Boolean is_attacking() {return attack != attackNone;}

  static Touching offset_generator(Dir);
  /* EFFECTS: Used to modify the way Moving generates offsets.  It takes
     into account the mapping of dir to attackDir, i.e. holding the attacking
     pixmaps in unused slots in MovingContext. */
  /* NOTE: All classes that have the fighter ability must have 
     Fighter::offset_generator as the value of MovingContext::offsetGenerator.
     This will cause problems if we want to be able to dynamically add
     or remove the Fighter Ability to/from a creature.  Fix it when it
     comes up. */

  static TransformMap transformOverride;
  /* EFFECTS: Similar to offset_generator in that it overrides behavior in 
     Moving to deal with the mapping of dir to attackDir.  In this case it is 
     needed to provide info about which bitmaps are transformations of other 
     bitmaps. */
  static TransformMap uprightTransformOverride;


private:
  static Attack dir_to_attack(Dir dir);

  static Dir dir_to_attack_dir(Dir dir);
  /* REQUIRES: dir has a corresponding attack dir */
  /* EFFECTS: Returns the dir used to store the attacking pixmaps given
     the non-attacking dir. */

  static Boolean is_attack_dir(Dir dir);

  static Dir attack_dir_to_dir(Dir dir);
  /* REQUIRES: dir is an attack dir */
  /* EFFECTS: Inverse of dir_to_attack_dir. */

  void attack_stuck(Dir dir,Stance stance);
  void attack_free(Dir dir);
  void attack_free_horizontal(Dir dir) {attack_free(dir);}
  /* NOTE: Does not zero vertical velocity. */

  void attack_free_vertical(Dir dir);
  /* NOTE: Vertical velocity only. */

  static const FighterContext *lookup_context(ClassId);
  /* NOTE: Fighter has its own functional registry of all Fighters in the 
     system. Not very extendable right now.  Would have to be redone for 
     templates. */
  static FighterXdata defaultXdata;

  const FighterContext *fc;
  Attack attack, attackNext;

  // For limiting the time of stuck and free attacks.
  // Only time-out free attacks for Flying Creatures.
  Timer timer;
};



////////// Walking
// Sticks to the ground only.  Affected by gravity.
struct WalkingContext {
  int dummy;
}; 


struct WalkingXdata {};


class Walking: public Locomotion {
public:
  Walking(const WalkingContext &,WalkingXdata &);

  DECLARE_NULL_ABILITY_IO(Walking);
  
  virtual AbilityId get_ability_id();

  virtual void act();

  static WalkingContext defaultContext;
  static WalkingXdata defaultXdata;


private:
  Pos rawPosPrev;
  Vel velPrev;
};



////////// Sticky
// Sticks to and walks on flat surfaces in four directions.  Affected by 
// gravity.
struct StickyContext {
  Speed wallCrawlSpeed;
};  


struct StickyXdata {};


class Sticky: public Locomotion {
public:
  Sticky(const StickyContext &,StickyXdata &);

  DECLARE_NULL_ABILITY_IO(Sticky);
  
  virtual void initialize(CreatureP);

  virtual AbilityId get_ability_id();

  virtual Boolean stick_touching(Touching touching);

  virtual void act();

  Speed get_wall_crawl_speed() {return context->wallCrawlSpeed;}
  /* EFFECTS: Returns the maximum speed for crawling on walls and 
     ceiling. */  

  static Boolean has_required_frames(const MovingContext *);
  /* EFFECTS: Does a Creature with the given MovingContext have all the 
     animation frames needed for the Sticky Ability. */
  
  static StickyContext defaultContext;
  static StickyXdata defaultXdata;
  /* NOTE: Just dummies to create object from a stream. */


#ifndef PROTECTED_IS_PUBLIC
protected:
#endif
  Boolean want_corner(const Corner &corner);
  /* EFFECTS: Returns True if the object should go around the given corner,
     False otherwise.  corner is the initial, not final corner.*/

  Stance cornered_stance(const Hanging &h);
  /* EFFECTS: Returns the stance to take in order to go around the corner
     of h. */

  void set_want_corner(const Corner &c1) 
    {wantCorner1 = c1; wantCorner2 = CO_air;}
  void set_want_corner(const Corner &c1,const Corner &c2) 
    {wantCorner1 = c1; wantCorner2 = c2;}
  /* EFFECTS: Tell the object that it should go around a corner if possible.
     The given values are in effect until changed. */
  /* NOTE: set_want_corner(CO_air) should be used to disable cornering. */


private:
  ///// DISABLED
  void set_dont_attach(Boolean val) {dontAttach = val;}
  Boolean get_dont_attach() {return False; /* dontAttach; */}
  /* NOTE: We may want to make these virtual or methods of Creature
     later on. */
  
  Corner wantCorner1,wantCorner2;
  Boolean dontAttach;

  const StickyContext *context;
};



////////// Flying
// Floats around.  Can walk on the ground.
struct FlyingContext 
{
  int gravTime;
};  

struct FlyingXdata {};

class Flying: public Locomotion {
public:
  Flying(const FlyingContext &,FlyingXdata &);
  
  DECLARE_NULL_ABILITY_IO(Flying);
  
  virtual AbilityId get_ability_id();

  virtual void act();

  static FlyingContext defaultContext;
  static FlyingXdata defaultXdata;


private:
  const FlyingContext *context;
};



////////// BuiltIn
// Has a directional built in weapon.
// NOTE: BuiltIn depends on existence of Holder, in act() and 
// get_weapon_string().
struct BuiltInContext {
  int shotTime;
  ClassId weaponClassId;
  const char *weaponStr;
  int coolness; // of the builtIn weapon.
  SoundName attackSound;

  Size (*get_shot_size)(Dir);
  PhysicalP (*create_shot)(WorldP,LocatorP,const Pos &,const Id &shooter,Dir);
  Dir (*compute_weapon_dir)(ITcommand); // may be NULL.
};  


struct BuiltInXdata {};


class BuiltIn: public Ability {
public:
  BuiltIn(const BuiltInContext &,BuiltInXdata &);
  /* NOTE: Must pass in static functions for get_shot_size, create_shot, and
     compute_weapon_dir.  compute_weapon_dir may be NULL, BuiltIn will use the
     default. */
  
  DECLARE_NULL_ABILITY_IO(BuiltIn);
  
  virtual AbilityId get_ability_id();

  virtual ClassId get_weapon_string(const char *&str);
  /* EFFECTS: str is set to point to a static string containing the string
     for ClassId. */

  virtual void act();

  int get_weapon_coolness() {return context->coolness;}
  /* EFFECTS: Return the coolness of the built-in weapon. */

  Boolean ready();
  /* EFFECTS: Is the BuiltIn ready to fire. */
  /* NOTE: This is not overriding any parent's ready() method anymore. */

  static BuiltInContext defaultContext;
  static BuiltInXdata defaultXdata;



private:
#if 0
  virtual Dir compute_weapon_dir(ITcommand);
  /* EFFECTS:  Returns the direction the weapon should be fired, 
     {CO_R ... CO_UP_R}, or CO_air. */

  virtual Size get_shot_size(Dir) = 0;

  virtual PhysicalP create_shot(const Pos &,Dir) = 0;
#endif

  Timer shotTimer;
  const BuiltInContext *context;
};
typedef BuiltIn *BuiltInP;



////////// Hugger
// Hugs onto any creature it collides with (except A_Alien and A_Hugger)
// and stuns them, then turns them into an Alien.
struct HuggerContext {
  // Which intel, the hugger's or the huggee's intel is implanted in the new
  // Alien.
  Boolean useHuggeeIntel; 
};  


struct HuggerXdata {};


class Hugger: public Ability {
public:
  Hugger(const HuggerContext &,HuggerXdata &);

  DECLARE_NULL_ABILITY_IO(Hugger);
  
  virtual AbilityId get_ability_id();

  virtual int get_drawing_level();

  virtual void act();

  virtual Boolean collide(PhysicalP other);

  static HuggerContext defaultContext;
  static HuggerXdata defaultXdata;


private:
  Id huggeeId;
  const HuggerContext *context;
};



///////// Prickly
// Hurts anything it touches.
struct PricklyContext {
  int damage;
};


struct PricklyXdata {};


class Prickly: public Ability {
public:
  Prickly(const PricklyContext &,PricklyXdata &);
	
  DECLARE_NULL_ABILITY_IO(Prickly);
  
  virtual AbilityId get_ability_id();

  virtual ClassId get_weapon_string(const char *&str);

  virtual Boolean collide(PhysicalP);

  static PricklyContext defaultContext;
  static PricklyXdata defaultXdata;
  

private:
  const PricklyContext *context;
};



///////// Healing
// Heals up to twice its minimum ever damage.
struct HealingContext
{
	float multiplier;
	int unit;
	int time;
};

// Not used now.
struct HealingXdata {};

class Healing: public Ability {
public:
  Healing(const HealingContext &,HealingXdata &);
  // Ignores the HealingXdata parameter.  Important for AltarOfSin.

  DECLARE_NULL_ABILITY_IO(Healing);

  virtual void initialize(CreatureP);
  
  virtual AbilityId get_ability_id();
  
  virtual void heal();
  
  virtual void act();

  static HealingContext defaultContext;
  static HealingXdata defaultXdata;


private:
	Timer healTimer;
	Health healthMin;
	const HealingContext *context;
};



///////// Lifter
// Can lift Liftables if there is no current weapon.
struct LifterContext {
  int throwSpeed;  // 0 means the Lifter can't throw
};


struct LifterXdata {};


class LifterSpeedModifier : public Modifier {
public:
  LifterSpeedModifier();

  virtual ModifierId get_modifier_id();
  
  virtual int apply(Attribute attr,int startVal);

   void set_lifting(Boolean val) {lifting = val;}
  /* NOTE: Lifter sets data on LifterSpeedModifier instead of vice-versa
     because we can't easily cast a Physical to a Lifter.  C++ virtual 
     base class bullshit. */
  /* NOTE about NOTE: We could fix this now with our Ability architecture. */


private:
  Boolean lifting;
};


class Lifter: public Ability {
public:
  Lifter(const LifterContext &,LifterXdata &);

  DECLARE_NULL_ABILITY_IO(Lifter);

  virtual void initialize(CreatureP);
  
  virtual AbilityId get_ability_id();

  virtual ClassId get_weapon_string(const char *&str);

  virtual void act();
  /* NOTE: Make Liftable follow Lifter. */

  virtual Boolean collide(PhysicalP);

  Boolean is_lifting();

  static LifterContext defaultContext;
  static LifterXdata defaultXdata;


private:
  void drop_liftable(PhysicalP liftable);

  void throw_liftable(PhysicalP liftable,Dir);

  Id liftable; // lifting something iff this is valid.
  ModifierP speedModifier; // cleaned up by Creature.
  const LifterContext *context;
};
typedef Lifter *LifterP;



///////// Morphed
// Some other object was morphed into this one.  Will revert back to the
// original one after some time.
struct MorphedContext
{
  int revertTime;
};

struct MorphedXdata {};

class Morphed: public Ability {
public:
  Morphed(const MorphedContext &,MorphedXdata &,PhysicalP);

  DECLARE_NULL_ABILITY_IO(Morphed);
  
  virtual AbilityId get_ability_id();

  virtual void get_followers(PtrList &list);
  
  virtual void drop_all(Boolean killNonPersistent);

  virtual void act();

  virtual void die();

  static MorphedContext defaultContext;
  static MorphedXdata defaultXdata;


private:
  Boolean unmappedSet;
  Id unmapped;
  Timer timer; // Only meaningful if unmappedSet.
};



///////// AnimTime
// Something with other than MOVING_ANIM_TIME
struct AnimTimeContext {
  int animTime;
}; 

struct AnimTimeXdata {};

class AnimTime: public Ability {
public:
  AnimTime(const AnimTimeContext &,AnimTimeXdata &);

  DECLARE_NULL_ABILITY_IO(AnimTime);
  
  virtual void initialize(CreatureP);
  
  virtual AbilityId get_ability_id();

  virtual int get_anim_time();

  static const AnimTimeContext *lookup_context(ClassId);
  static AnimTimeXdata defaultXdata;


private:
  const AnimTimeContext *context;    
};



///////// OnFire
// Constantly spouting flames, resistant to heat attacks, burns those that
// touch it.
struct OnFireContext {
  int fires;  // How many Fire objects are created per turn.
  int heat;
  Boolean deadBurns; // Does a dead object burn.
}; 

struct OnFireXdata {};

class OnFire: public Ability{
public:
  OnFire(const OnFireContext &,OnFireXdata &);

  DECLARE_NULL_ABILITY_IO(OnFire);
  
  virtual AbilityId get_ability_id();

  virtual Boolean heat_attack(PhysicalP,int heat,Boolean secondary);

  virtual void act();

  virtual Boolean collide(PhysicalP other);

  static OnFireContext defaultContext;
  static OnFireXdata defaultXdata;


private:
  const OnFireContext *context;
};



///////// SwapProtect
// Has limited protection from swap attacks.  Unlimited protection from Frog attacks.
//
struct SwapProtectContext {
  int swapResistance;
}; 

struct SwapProtectXdata {};

class SwapProtect: public Ability {
public:
  SwapProtect(const SwapProtectContext &,SwapProtectXdata &);

  DECLARE_NULL_ABILITY_IO(SwapProtect);
  
  virtual AbilityId get_ability_id();

  virtual Boolean swap_protect();

  virtual Boolean frog_protect();

  static SwapProtectContext defaultContext;
  static SwapProtectXdata defaultXdata;


private:
  int swapResistance;
};



///////// Composite
// A set of creatures that act like a single creature.
//
class Composite: public Ability {
public:
  Composite();
  Composite(InStreamP,CreatureP);

  Boolean is_composite();

  virtual Boolean is_leader() = 0;
  /* EFFECTS: Is this object the leader of the rest of the objects. */

  CompositeId get_composite_id() {return compositeId;}

  void set_composite_id(CompositeId cId) {compositeId = cId;}

  virtual PhysicalP get_draw_before_me();
  /* DEFAULT: return NULL */
  /* EFFECTS: Override this to control the drawing order of the Composite
     object. Return an object that must be drawn before this one. */

  static void reset_id_generator() {idGenerator = 0;}

#if 0
  static CompositeId extract_composite_id(PhyscialP p);
  /* EFFECTS: Helper function to get the compositeId from a physical, or return
     COMPOSITE_ID_NONE, if p is not a Composite object. */
#endif


#ifndef PROTECTED_IS_PUBLIC
protected:
#endif 
  static CompositeId generate_id() {return idGenerator++;}
  /* EFFECTS: Create a unique CompositeId for a new composite object. */


private: 
  CompositeId compositeId;
  static CompositeId idGenerator;
};



///////// Segmented
// One segment of a segmented composite creature.  Acts like the head if
// it is the first in the chain.
//
// Use it by fully creating each Creature segment, then link them together.
//
struct SegmentedContext {
  int followDistance;
}; 

struct SegmentedXdata {};

class Segmented: public Composite {
public:
  Segmented(const SegmentedContext &,SegmentedXdata &);

  DECLARE_ABILITY_IO(Segmented);
  /* EFFECTS: Write out prev and next, so can draw properly. */  

  virtual AbilityId get_ability_id();

  virtual void get_followers(PtrList &list);

  virtual Boolean follow(const Area &followeeArea,Dir followeeDir,
                         Boolean currentAndNext);

  virtual Boolean get_pixmap_mask(Xvars &,int dpyNum,CMN_IMAGEDATA &imageData,
                                  Dir dir,int animNum);
  /* EFFECTS: Used to decide whether to draw the head or not. */

  virtual void set_intel(IntelP i);
  /* NOTE: All the intel's in the chain must be consistent. */

  virtual void act();

  virtual void update();

  virtual Boolean is_leader();

  virtual PhysicalP get_draw_before_me();

  Id get_prev() {return prev;}
  Id get_next() {return next;}
  /* EFFECTS: Search through chain of segments. */

  static void create_and_add_composite(PtrList &ret,
    WorldP,LocatorP,int segmentsNum,const Pos &,
    PhysicalP (*create)(void *arg,WorldP w,LocatorP l,const Pos &pos),
    void *arg);
  /* MODIFIES: ret */
  /* REQUIRES: create() creates something that has the Segmented ability. */
  /* EFFECTS: Create a Composite object as a chain of Segments.
     Add all segments to the locator. Fill ret with all the Physicals of 
     the chain,
     with the head in element 0. */
  /* NOTE: Must return all segments in ret, since we can't traverse the 
     segments via their Id's 
     until after the first Locator::clock(). */

  static SegmentedContext defaultContext;
  static SegmentedXdata defaultXdata;


private:
  // Where to store the bitmaps in MovingContext.
  enum {
    LEADER_DIR = CO_DN_R_R,
    LEADER_ANIM_TIME = 4,
  };
  
  void insert_before(CreatureP c);
  /* REQUIRES: c must have the Segmented Ability. */
  /* EFFECTS: Insert this object in the chain before c. */

  Boolean compatible_prev(PhysicalP);
  Boolean compatible_next(PhysicalP);
  /* EFFECTS: Is the object a compatible member of this composite object. */

  void assume_leadership();
  /* EFFECTS: Called when a non-leader becomes a leader. Not called in
     the constructor for a leader. */

  void map_dir_anim_num(Dir& mappedDir,int& mappedAnimNum,Dir dir,int animNum);
  /* MODIFIES: mappedDir, mappedAnimNum */
  /* EFFECTS: Get the direction and animNum to use to actually display
     the head, body, or dead pixmap. */


  // These should be separated into current and next variables.
  Boolean isLeader;
  Id prev;
  Id next;
  Timer leaderAnimTimer;
  int leaderAnimNum; // Chosen randomly.  Animation frame to use if leader.
  const SegmentedContext* context;
  // Use this followDistance, not the one in context.  So, that reading 
  // from stream works properly.
  int followDistance;
};
typedef Segmented* SegmentedP;



///////// Sensitive
// Sensitive to certain types of attacks, e.g. Yeti takes extra damage from
// heat attacks.  Not an ability you really want to have.  
struct SensitiveContext {
  int corporealMultiplier; // 0 means no change
  int heatMultiplier;  // 0 means no change
}; 

struct SensitiveXdata {};

class Sensitive: public Ability {
public:
  Sensitive(const SensitiveContext&,SensitiveXdata&);

  DECLARE_NULL_ABILITY_IO(Sensitive);
  /* EFFECTS: Write out prev and next, so can draw properly. */  

  virtual AbilityId get_ability_id();

//  virtual Boolean corporeal_attack(PhysicalP killer,int damage); 
// Implement it when we need it.

  virtual Boolean heat_attack(PhysicalP killer,int heat,Boolean secondary);

  static SensitiveContext defaultContext;
  static SensitiveXdata defaultXdata;


private:
  const SensitiveContext* context;  
};
typedef Sensitive* SensitiveP;
#endif
