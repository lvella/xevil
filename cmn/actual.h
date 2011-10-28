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

// "actual.h"
// Actual objects to be instantiated.

#ifndef ACTUAL_H
#define ACTUAL_H

#if X11
	#ifndef NO_PRAGMAS
	#pragma interface
	#endif
#endif 



// Include Files
#include "utils.h"
#include "coord.h"
#include "world.h"
#include "locator.h"
#include "physical.h"


// Class Declarations


// Dummy object, pointer to a Mover in the world, only exists so that 
// movers get drawn every turn.
class PhysMover: public Physical {
  // Some hacks here to act like a Physical.  World::clock() gets called 
  // before Locator::clock(), so we must juggle which Area we return depending
  // on where we are in the act()-update() phases.

public:
  PhysMover(WorldP,LocatorP,MoverP);
  
  DECLARE_LEAF_IO(PhysMover);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  virtual const Area &get_area();

  virtual const Area &get_area_next();
  /* NOTE: Should only be used by the Locator. */

  virtual Boolean collidable();
  virtual Boolean corporeal_attack(PhysicalP killer,int damage); 
  virtual void heat_attack(PhysicalP,int heat,Boolean secondary);

  virtual void act();
  virtual void update();

  virtual void draw(CMN_DRAWABLE buffer,Xvars &xvars,int dpyNum,
		    const Area &area);

  void mover_clock();
  /* NOTE: More of the hack to act like a Physical.  Called when corresponding
     Mover is clocked. */
  /* NOTE: Should only be called by World/Mover. */
  
  const static PhysicalContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */

  
private:
  static void init_x(Xvars&,IXCommand command,void* arg);
   
  const Area &earlier_area();
  const Area &later_area();
  /* Return the earlier/later of the two known Areas for the Mover. */

  enum State {BeforeAct,BetweenActUpdate,AfterUpdate};
  int state;

  MoverId moverId;
  Area areaPrev; // Only stored so can return a (const Area &) to something.
};

typedef PhysMover *PhysMoverP;



class Explosion: public Physical {
 public:
  Explosion(WorldP w,LocatorP l,const Pos &middle,const Id &bomber,int radius,
	    int damageMax);

  DECLARE_LEAF_IO(Explosion);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);
  
  virtual Boolean collidable();
  virtual const Area &get_area();
  virtual const Area &get_area_next();
  virtual void draw(CMN_DRAWABLE buffer,Xvars &xvars,int,const Area &area);
  virtual void act();

  const static PhysicalContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


 private:
  static void init_x(Xvars&,IXCommand command,void* arg);
   
  Id bomber;
  Area area; 
  Pos middle; // Same as in area.
  int radius;
  int damageMax;
};



class Fire: public Physical {
public:
  Fire(WorldP w,LocatorP l,const Pos &middle,Boolean coll = True);
  /* NOTE: If coll is False, the Fire will not collide with anything.
     I.e. Just for decoration. */

  DECLARE_LEAF_IO(Fire);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);
  
  static int get_damage();
  virtual const Area &get_area();
  virtual const Area &get_area_next();

  virtual int get_drawing_level();

  virtual Boolean collidable();
  virtual Boolean corporeal_attack(PhysicalP,int);
  virtual void heat_attack(PhysicalP,int,Boolean);

  virtual void draw(CMN_DRAWABLE buffer,Xvars &xvars,int,const Area &area);

  virtual void avoid(PhysicalP other);
  virtual void collide(PhysicalP other);

  virtual void act();
  virtual void update();

  const static PhysicalContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */

  static void init_x(Xvars&,IXCommand command,void* arg);


private:
  Boolean isCollidable;
  static FireXdata xdata; 
  static const CMN_BITS_ID fireBits;
  Area area,areaNext;
};



class FireExplosion: public Physical {
 public:
  FireExplosion(WorldP w,LocatorP l,const Pos &rawPos,const Id &shooter);

  DECLARE_LEAF_IO(FireExplosion);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);
  
  virtual Boolean collidable();
  virtual Boolean corporeal_attack(PhysicalP,int);
  virtual void heat_attack(PhysicalP,int,Boolean);

  virtual const Area &get_area();
  virtual const Area &get_area_next();
  virtual void draw(CMN_DRAWABLE buffer,Xvars &xvars,int,const Area &area);
  virtual void act();

  const static PhysicalContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  Timer timer;   
  Id shooter;
  Area area; 
};



// Protects from N_PROTECTION_N separate corporeal attacks.
class NProtection: public Protection {
public:
  NProtection(WorldP,LocatorP,const Area &);

  DECLARE_NULL_LEAF_IO(NProtection);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  const static ProtectionContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


private:
  virtual Boolean corporeal_protect(int);
  virtual Boolean heat_protect(int heat,Boolean);

  static void init_x(Xvars&,IXCommand command,void* arg);

  int n; // Number of hits that it can still take.
  static ProtectionXdata xdata;
};



// Protects from heat or corporeal attacks for T_PROTECTION_TIME turns.
class TProtection: public Protection {
public:
  TProtection(WorldP,LocatorP,const Area &);

  DECLARE_NULL_LEAF_IO(TProtection);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  virtual void act();

  const static ProtectionContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


private:
  virtual Boolean corporeal_protect(int);
  virtual Boolean heat_protect(int heat,Boolean);

  static void init_x(Xvars&,IXCommand command,void* arg);

  Timer timer;
  static ProtectionXdata xdata;
};



// Protects from X_PROTECTION_HEALTH total amount of damage.
class XProtection: public Protection {
public:
  XProtection(WorldP,LocatorP,const Area &);

  DECLARE_NULL_LEAF_IO(XProtection);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  const static ProtectionContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


private:
  virtual Boolean corporeal_protect(int);
  virtual Boolean heat_protect(int heat,Boolean);

  static void init_x(Xvars&,IXCommand command,void* arg);

  int health;  // amount of damage it can still take.
  static ProtectionXdata xdata;
};



class Rock: public Liftable {
 public:
  Rock(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_LEAF_IO(Rock);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  const static LiftableContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  static LiftableXdata xdata;
};



class Weight: public Liftable {
 public:
  Weight(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_LEAF_IO(Weight);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  const static LiftableContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  static LiftableXdata xdata;
};



// Modifiers for AltarOfSin
class DoubleSpeed: public Doubler {
public:
  DoubleSpeed(int timed = 0);

  virtual ModifierId get_modifier_id();
  
  virtual int apply(Attribute attr,int startVal);
};


class DoubleJump: public Doubler {
public:
  virtual ModifierId get_modifier_id();

  virtual int apply(Attribute attr,int startVal);
};


// Should only be added on Creature's modifiers because
// of problems with transferring to a new body.
class DoubleHealth: public Doubler {
public:
  virtual ModifierId get_modifier_id();

  virtual int apply(Attribute attr,int startVal);
};


class AltarOfSin: public Heavy {
 public:
  AltarOfSin(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_LEAF_IO(AltarOfSin);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  virtual Boolean corporeal_attack(PhysicalP,int); 

  virtual void heat_attack(PhysicalP,int,Boolean);

  virtual void collide(PhysicalP);

  virtual void update();

  const static HeavyContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  Boolean turnTaken; // Only do one interesting thing per turn.

  static HeavyXdata xdata;

  // For granting powers.
//  const static HealingContext healingContext;
//  const static OnFireContext onFireContext;
//  const static BuiltInContext fireballsContext;
};



class Doppel: public AutoUse {
  // Not a AutoUse because Seals and other funny things get doppelganged.
 public:
  Doppel(WorldP,LocatorP,const Pos &);

  DECLARE_NULL_LEAF_IO(Doppel);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);
  
  virtual void use(PhysicalP);
  /* NOTE: Only certain classes may be used as doppelgangers.  If the
     User is not of one of these classes, the doppelganger class will
     be chosen randomly. */

  static Stats &get_stats(void *) {return stats;}

  const static AutoUseContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


 private:
  PhysicalP create_physical(const Area &,PhysicalP original);
  /* EFFECTS: Create a new Physical which is of the same class as original. */

  static void init_x(Xvars&,IXCommand command,void* arg);

  static AutoUseXdata xdata;
  static Stats stats;
};



class Cloak: public Item {
public:
  Cloak(WorldP,LocatorP,const Pos &);

  DECLARE_NULL_LEAF_IO(Cloak);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  virtual void use(PhysicalP);

  static Stats &get_stats(void *) {return stats;}

  const static ItemContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


private:
  static void attack_hook(PhysicalP p);

  static void init_x(Xvars&,IXCommand command,void* arg);

  static ItemXdata xdata;
  static Stats stats;
};



class Transmogifier: public AutoUse {
 public:
  Transmogifier(WorldP,LocatorP,const Pos &);

  DECLARE_NULL_LEAF_IO(Transmogifier);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);
  
  virtual void use(PhysicalP);

  static Stats &get_stats(void *) {return stats;}

  const static AutoUseContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


 private:
  PhysicalP new_physical(const Pos &middle,ClassId notThis);
  /* EFFECTS:  Create a new randomly chosen Physical centered at middle
     for the object using the Transmogifier.  Don't create a Physical of
     class notThis. */

  static Boolean transmogify_target_filter(const PhysicalContext *,void *);
  /* EFFECTS: Filter those classes whose transmogifyTarget flag is True. */

  static void init_x(Xvars&,IXCommand command,void* arg);

  static AutoUseXdata xdata;
  static Stats stats;
};
  


class MedKit: public AutoUse {
 public:
  MedKit(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_LEAF_IO(MedKit);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  virtual void use(PhysicalP);

  static Stats &get_stats(void *) {return stats;}

  const static AutoUseContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  static AutoUseXdata xdata;
  static Stats stats;
};



class Crack: public Modifier {
public:
  Crack();

  virtual ModifierId get_modifier_id();

  virtual int apply(Attribute attr,int startVal);
};


class CrackPipe: public Drugs {
 public:
  CrackPipe(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_LEAF_IO(CrackPipe);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  static Stats &get_stats(void *) {return stats;}

  const static DrugsContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


#ifndef PROTECTED_IS_PUBLIC
  protected:
#endif
  virtual ModifierP create_modifier();


 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  static DrugsXdata xdata;
  static Stats stats;
};



class CaffineM: public Modifier {
public:
  CaffineM();

  virtual ModifierId get_modifier_id();

  virtual int apply(Attribute attr,int startVal);
};


class Caffine: public Drugs {
 public:
  Caffine(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_LEAF_IO(Caffine);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  static Stats &get_stats(void *) {return stats;}

  const static DrugsContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


#ifndef PROTECTED_IS_PUBLIC
  protected:
#endif
  virtual ModifierP create_modifier();


 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  static DrugsXdata xdata;
  static Stats stats;
};



class PCPM: public Modifier {
public:
  PCPM();

  virtual ModifierId get_modifier_id();

  virtual int apply(Attribute attr,int startVal);


#ifndef PROTECTED_IS_PUBLIC
  protected:
#endif
  void preDie(PhysicalP p);
  /* EFFECTS: Whack off half of p's health. */
};


class PCP: public Drugs {
 public:
  PCP(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_LEAF_IO(PCP);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  static Stats &get_stats(void *) {return stats;}

  const static DrugsContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


#ifndef PROTECTED_IS_PUBLIC
  protected:
#endif
  virtual ModifierP create_modifier();


 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  static DrugsXdata xdata;
  static Stats stats;
};



class NShield: public Shield {
public:
  NShield(WorldP,LocatorP,const Pos &);

  DECLARE_NULL_LEAF_IO(NShield);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  virtual void use(PhysicalP);

  static Stats &get_stats(void *) {return stats;}

  const static ShieldContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */

  
private:
  virtual ProtectionP create_protection(const Area &);

  static void init_x(Xvars&,IXCommand command,void* arg);

  static ShieldXdata xdata;
  static Stats stats;
};



class TShield: public Shield {
public:
  TShield(WorldP,LocatorP,const Pos &);

  DECLARE_NULL_LEAF_IO(TShield);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  virtual void use(PhysicalP);

  static Stats &get_stats(void *) {return stats;}

  const static ShieldContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


private:
  virtual ProtectionP create_protection(const Area &);

  static void init_x(Xvars&,IXCommand command,void* arg);

  static ShieldXdata xdata;
  static Stats stats;
};



class Bomb: public Animated {
 public:
  Bomb(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_LEAF_IO(Bomb);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  virtual void use(PhysicalP p);
  /* NOTE: p can be NULL. */

  virtual void act();

  virtual Boolean is_bomb();

  virtual void set_quiet_death();

  virtual void die();
  /* EFFECTS: Create an explosion. */

  static Stats &get_stats(void *) {return stats;}

  const static AnimatedContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  static AnimatedXdata xdata;
  Timer timer;
  Boolean active;
  Id bomber; // Only valid if active.
  Frame frame; 
  Boolean defused;
  static Stats stats;
};



class Shell: public Shot {
 public:
  Shell(WorldP w,LocatorP l,const Pos &pos,
  const Id &shooter,
	Dir dir);

  DECLARE_NULL_LEAF_IO(Shell);

  static Size get_size();  // doubles as get_size_max().
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  static Stats &get_stats(void *) {return stats;}

  const static ShotContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  static ShotXdata xdata;
  static Stats stats;
};



class SwapShell: public Shot {
 public:
  SwapShell(WorldP w,LocatorP l,const Pos &pos,
	    const Id &shooter,
      const Id& swapper,Dir dir);
  /* NOTE: shooter is the person who fired it.  swapper is the swapper the 
     shell came from. */

  DECLARE_NULL_LEAF_IO(SwapShell);

  static Size get_size();  // Doubles as get_size_max().
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  virtual void collide(PhysicalP);
  /* NOTE: Does not call up the tree. */

  static Stats &get_stats(void *) {return stats;}

  const static ShotContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  static ShotXdata xdata;
  Id swapper;
  static Stats stats;
};



class Lance: public Shot {
 public:
  Lance(WorldP w,LocatorP l,const Pos &pos,
  const Id &shooter,
	Dir dir);
  static Size get_size(Dir);

  DECLARE_NULL_LEAF_IO(Lance);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  static Dir compute_weapon_dir(ITcommand);
  /* EFFECTS: Weapons firing a Lance shoot diagonal shots different than
     most weapons. */

  static Stats &get_stats(void *) {return stats;}

  const static ShotContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  static ShotXdata xdata;
  static Stats stats;
};



class Laser: public Shot {
 public:
  Laser(WorldP w,LocatorP l,const Pos &pos,
  const Id &shooter,
	Dir dir);
  static Size get_size(Dir);

  DECLARE_NULL_LEAF_IO(Laser);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  static Dir compute_weapon_dir(ITcommand);
  /* EFFECTS: Weapons firing a Laser shoot diagonal shots different than
     most weapons. */

  static Stats &get_stats(void *) {return stats;}

  const static ShotContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  static ShotXdata xdata;
  static Stats stats;
};



class FrogShell: public Shot {
 public:
  FrogShell(WorldP w,LocatorP l,const Pos &pos,
      const Id &shooter,
	    const Id &frogGun,Dir dir);

  DECLARE_NULL_LEAF_IO(FrogShell);

  static Size get_size(); // Doubles as get_size_max().
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  virtual void collide(PhysicalP);
  /* NOTE: Does not call up the tree. */

  static Stats &get_stats(void *) {return stats;}

  const static ShotContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */

  const static MorphedContext morphedContext;
  /* NOTE: public so AltarOfSin can get to it. */


 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  static ShotXdata xdata;
  Id frogGun;
  static Stats stats;
};



class Fireball: public Shot {
 public:
  Fireball(WorldP w,LocatorP l,const Pos &pos,const Id &shooter,
	   Dir dir,int heat = -1,int time = -1,Boolean hurtOnFire = False);
  /* NOTE: Will use heat and time instead of FIREBALL_HEAT and FIREBALL_TIME
     if they are specified.  If hurtOnFire is True, these fireballs will
     do physical damage to Creatures with the OnFire ability. */

  DECLARE_NULL_LEAF_IO(Fireball);

  static Size get_size(); // Doubles as get_size_max().
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  virtual void collide(PhysicalP other);
  virtual void act();

  const static ShotContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */

 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  static ShotXdata xdata;
  Timer timer;
  int heat;
  Boolean hurtOnFire;
};



// Only seeks after creatures.
class Missile: public Shot {
 public:
  Missile(WorldP w,LocatorP l,const Pos &pos,
          const Id &shooter,
          Dir dir);
  static Size get_size(Dir);

  DECLARE_NULL_LEAF_IO(Missile);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  virtual void act();

  static Stats &get_stats(void *) {return stats;}

  const static ShotContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  Boolean hasTarget;
  Id targetId;
  Timer timer;
  Timer rotate;
  Id shooterId;

  static ShotXdata xdata;
  static Stats stats;
};



class Star: public Shot {
 public:
  Star(WorldP w,LocatorP l,const Pos &pos,
       const Id &shooter,
	     Dir dir);

  DECLARE_NULL_LEAF_IO(Star);

  static Size get_size(); // Doubles as get_size_max.
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  static Stats &get_stats(void *) {return stats;}

  const static ShotContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  static ShotXdata xdata;
  static Stats stats;
};



class Blood: public Droplet {
public:
  DECLARE_NULL_LEAF_IO(Blood);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  const static DropletContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  Blood(WorldP,LocatorP,const Pos &);
  /* NOTE: Use Droplet::create_and_add_if(). */

  static DropletXdata xdata;
};



class GreenBlood: public Droplet {
public:
  DECLARE_NULL_LEAF_IO(GreenBlood);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  const static DropletContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  GreenBlood(WorldP,LocatorP,const Pos &);
  /* NOTE: Use Droplet::create_and_add_if(). */

  static DropletXdata xdata;
};



class OilDroplet: public Droplet {
public:
  DECLARE_NULL_LEAF_IO(OilDroplet);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  const static DropletContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  OilDroplet(WorldP,LocatorP,const Pos &);
  /* NOTE: Use Droplet::create_and_add_if(). */

  static DropletXdata xdata;
};



class Feather: public Droplet {
public:
  DECLARE_NULL_LEAF_IO(Feather);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  const static DropletContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */

private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  Feather(WorldP,LocatorP,const Pos &);
  /* NOTE: Use Droplet::create_and_add_if(). */

  static DropletXdata xdata;
};



class Trapdoor: public Moving {
public:
  enum {SEALS_MAX = 15}; // for one trapdoor.  Used to be 20

  Trapdoor(WorldP w,LocatorP l,const Pos &pos,const Id &homeId);

  DECLARE_NULL_LEAF_IO(Trapdoor);

  static Size get_size(); // Doubles as get_size_max().
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  virtual Boolean collidable();

  virtual int get_drawing_level();
  /* NOTE: Trapdoor drawn behind everything else. */

  void append_seals_out(int &num,IntelId *lem);
  /* MODIFIES: num, lem */
  /* EFFECTS: Set lem to be an array of all the IntelIds of the seals that 
     have been generated by this trapdoor,  i.e are "out". */

  virtual void act();

  const static MovingContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  Timer timer;
  IntelId seals[SEALS_MAX];
  int sealsNum;
  Id homeId; // Generated seals will seek this Home.
  
  static MovingXdata xdata;
};
typedef Trapdoor *TrapdoorP;



class Home: public Moving {
public:
  Home(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_LEAF_IO(Home);

  static Size get_size(); // Doubles as get_size_max().
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  virtual Boolean collidable();


  virtual int get_drawing_level();
  /* NOTE: Home drawn behind everything else. */

  int get_seals_safe() {return sealsSafe;}

  virtual void act();

  const static MovingContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  int sealsSafe;
  
  static MovingXdata xdata;
};
typedef Home *HomeP;



class Grenade: public Falling {
public:
  Grenade(WorldP w,LocatorP l,const Pos &pos,const Id &sh,
	  Dir dir,Speed speed);
  Grenade(WorldP,LocatorP,const Pos &,const Id &,const Vel &);

  DECLARE_NULL_LEAF_IO(Grenade);

  static Size get_size(Dir dir);
  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  virtual void set_quiet_death();

  virtual void act();

  virtual void die();

  static Stats &get_stats(void *) {return stats;}

  const static FallingContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  Timer timer;
  Id shooter;
  Boolean defused;

  static FallingXdata xdata;
  static Stats stats;
};



// Code copied from Grenade, should have common parent.
class Napalm: public Falling {
public:
  Napalm(WorldP,LocatorP,const Pos &,const Id &,const Vel &);

  DECLARE_NULL_LEAF_IO(Napalm);

  static Size get_size(Dir dir);
  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  virtual void set_quiet_death();

  virtual void heat_attack(PhysicalP,int,Boolean);
  /* NOTE: Don't want NapalmGrenades to set each other off. */

  virtual void act();

  virtual void die();

  static Stats &get_stats(void *) {return stats;}

  const static FallingContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  Timer timer;
  Id shooter;
  Boolean defused;
 
  static FallingXdata xdata;
  static Stats stats;
};



class Egg: public Falling {
public:
  Egg(WorldP w,LocatorP l,const Pos &pos,
      const IntelOptions &ops,ITmask opMask);

  DECLARE_LEAF_IO(Egg);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  virtual void act();

  const static FallingContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  Boolean open;
  
  // The intel options for the new Hugger.  Should be the same as for an
  // Alien.
  IntelOptions intelOptions;
  ITmask intelOpMask;

  static FallingXdata xdata;
};



// NOTE: Called Xit instead of Exit to avoid naming conflicts with the C
// function exit().
class Xit: public Touchable {
public:
  Xit(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_LEAF_IO(Xit);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  const static TouchableContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */

  
 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  static TouchableXdata xdata;
};



class Flag: public Touchable {
public:
  Flag(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_LEAF_IO(Flag);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  const static TouchableContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */

  
 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  static TouchableXdata xdata;
};



class Chainsaw: public Cutter {
 public:
  Chainsaw(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_LEAF_IO(Chainsaw);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  const static CutterContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  static CutterXdata xdata;
};



class Pistol: public Gun {
 public:
  Pistol(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_LEAF_IO(Pistol);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  const static GunContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  static GunXdata xdata;
};



// Machine Gun.
class MGun: public Gun {
 public:
  MGun(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_LEAF_IO(MGun);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  const static GunContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  static GunXdata xdata;
};



class Lancer: public Gun {
 public:
  Lancer(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_LEAF_IO(Lancer);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  const static GunContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


 private:
  virtual Size get_shot_size(Dir);
  virtual Dir compute_weapon_dir(ITcommand);
  virtual PhysicalP create_shot(PhysicalP,WorldP,LocatorP,const Pos &,Dir dir);

  static void init_x(Xvars&,IXCommand command,void* arg);

  static GunXdata xdata;
};



class AutoLancer: public Gun {
 public:
  AutoLancer(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_LEAF_IO(AutoLancer);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  const static GunContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


 private:
  virtual Size get_shot_size(Dir);
  virtual Dir compute_weapon_dir(ITcommand);
  virtual PhysicalP create_shot(PhysicalP,WorldP,LocatorP,const Pos &,Dir dir);

  static void init_x(Xvars&,IXCommand command,void* arg);

  static GunXdata xdata;
};



class FThrower: public Gun {
 public:
  FThrower(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_LEAF_IO(FThrower);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  virtual PhysicalP create_shot(PhysicalP,WorldP,LocatorP,const Pos &,Dir dir);

  virtual void fire(const Id &,ITcommand);

  virtual void act();
  
  const static GunContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  virtual Size get_shot_size(Dir);

  Timer stopFiring;
  Id killerId; // Valid iff isFiring.
  ITcommand fireCommand;   // Valid iff isFiring.
  Boolean isFiring;

  static GunXdata xdata;
};



class Launcher: public Gun {
 public:
  Launcher(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_LEAF_IO(Launcher);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  const static GunContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


private:
  virtual Size get_shot_size(Dir);
  virtual PhysicalP create_shot(PhysicalP,WorldP,LocatorP,const Pos &,Dir dir);

  static void init_x(Xvars&,IXCommand command,void* arg);

  static GunXdata xdata;
};



class Grenades: public Gun {
 public:
  Grenades(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_LEAF_IO(Grenades);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  const static GunContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


private:
  virtual Size get_shot_size(Dir);
  virtual PhysicalP create_shot(PhysicalP,WorldP,LocatorP,const Pos &,Dir dir);

  static void init_x(Xvars&,IXCommand command,void* arg);

  static GunXdata xdata;
};



class Napalms: public Gun {
 public:
  Napalms(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_LEAF_IO(Napalms);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  const static GunContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


private:
  virtual Size get_shot_size(Dir);
  virtual PhysicalP create_shot(PhysicalP,WorldP,LocatorP,const Pos &,Dir dir);

  static void init_x(Xvars&,IXCommand command,void* arg);

  static GunXdata xdata;
};



class Stars: public Gun {
 public:
  Stars(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_LEAF_IO(Stars);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  const static GunContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


 private:
  virtual Size get_shot_size(Dir);
  virtual PhysicalP create_shot(PhysicalP,WorldP,LocatorP,const Pos &,Dir dir);
  
  virtual void fire(const Id &id,ITcommand command);

  static void init_x(Xvars&,IXCommand command,void* arg);

  static GunXdata xdata;
};



class Swapper: public SingleGun {
 public:
  Swapper(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_LEAF_IO(Swapper);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  const static SingleGunContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


private:
  virtual Size get_shot_size(Dir);
  virtual PhysicalP create_shot(PhysicalP,WorldP,LocatorP,const Pos &,Dir dir);

  static void init_x(Xvars&,IXCommand command,void* arg);

  static SingleGunXdata xdata;
};



class FrogGun: public SingleGun {
 public:
  FrogGun(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_LEAF_IO(FrogGun);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  const static SingleGunContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


private:
  virtual Size get_shot_size(Dir);
  virtual PhysicalP create_shot(PhysicalP,WorldP,LocatorP,const Pos &,Dir dir);

  static void init_x(Xvars&,IXCommand command,void* arg);

  static SingleGunXdata xdata;
};



class DogWhistle: public Whistle {
public:
  DogWhistle(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_LEAF_IO(DogWhistle);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  const static WhistleContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */

#ifndef PROTECTED_IS_PUBLIC
protected: 
#endif

  virtual PetP create_pet(IntelP master,Boolean alternatePet,int n);


private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  static WhistleXdata xdata;
};



class DemonSummoner: public Whistle {
public:
  DemonSummoner(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_LEAF_IO(DemonSummoner);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  const static WhistleContext context;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


#ifndef PROTECTED_IS_PUBLIC
protected: 
#endif
  virtual PetP create_pet(IntelP master,Boolean alternatePet,int);


private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  static WhistleXdata xdata;
};



class Enforcer: public Creature {
// public Grounded, public Suicide, public Prickly 
 public:
  Enforcer(WorldP w,LocatorP l,const Pos &pos);
      
  DECLARE_NULL_CREATURE_IO(Enforcer);    
      
  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  static Stats &get_stats(void *) {return stats;}

  const static CreatureContext creatureContext;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */


 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  const static GroundedContext groundedContext;
  const static SuicideContext suicideContext;
  const static PricklyContext pricklyContext;
  static CreatureXdata creatureXdata;
  static GroundedXdata groundedXdata;
  static SuicideXdata suicideXdata;
  static PricklyXdata pricklyXdata;
  static Stats stats;
};



class Frog: public Creature {
// public Hopping, public Suicide, (public Morphed)
 public:
  Frog(WorldP w,LocatorP l,const Pos &pos);
  /* NOTE: Now caller must add the Morphed Ability, if desired. */

  DECLARE_NULL_CREATURE_IO(Frog);    

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  static Stats &get_stats(void *) {return stats;}

  const static CreatureContext creatureContext;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */
  const static HoppingContext hoppingContext;


 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  const static SuicideContext suicideContext;

  static CreatureXdata creatureXdata;
  static HoppingXdata hoppingXdata;
  static SuicideXdata suicideXdata;

  static Stats stats;
};



class Hero: public Creature {
// public Walking, public Fighter, public User, public Healing
 public:
  Hero(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_CREATURE_IO(Hero);    

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  static Stats &get_stats(void *) {return stats;}
  
  const static CreatureContext creatureContext;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */

  static FighterXdata fighterXdata;
  const static FighterContext fighterContext;
  /* NOTE: Similar hack so that Fighter::lookup_context() can get these. */
  const static HealingContext healingContext;


private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  static CreatureXdata creatureXdata;
  static WalkingXdata walkingXdata;
  static UserXdata userXdata;
  static HealingXdata healingXdata;
  static LifterXdata lifterXdata;

  const static WalkingContext walkingContext;
  const static UserContext userContext;
  const static LifterContext lifterContext;
  static Stats stats;
};



class Zombie: public Creature {
// public Walking, public Fighter
 public:
  Zombie(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_CREATURE_IO(Zombie);    

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  static Stats &get_stats(void *) {return stats;}
  
  const static CreatureContext creatureContext;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */

  static FighterXdata fighterXdata;
  const static FighterContext fighterContext;
  const static AnimTimeContext animTimeContext;
  /* NOTE: Similar hack so that Fighter::lookup_context() can get these. */


private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  static CreatureXdata creatureXdata;
  static WalkingXdata walkingXdata;
  static AnimTimeXdata animTimeXdata;

  const static WalkingContext walkingContext;
  static Stats stats;
};



class Ninja: public Creature {
// public Sticky, public Fighter, public User
 public:
  Ninja(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_CREATURE_IO(Ninja);    

  static Size get_size_max();

  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);
    
  static Stats &get_stats(void *) {return stats;}

  const static CreatureContext creatureContext;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */

  static FighterXdata fighterXdata;
  const static FighterContext fighterContext;
  /* NOTE: Similar hack so that Fighter::lookup_context() can get these. */

  // Used by AltarOfSin
  const static StickyContext stickyContext;


private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  static CreatureXdata creatureXdata;
  static StickyXdata stickyXdata;
  static UserXdata userXdata;
  
  const static UserContext userContext;
  static Stats stats;
};



class Alien: public Creature {
// public Sticky, public Prickly
 public:
  Alien(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_CREATURE_IO(Alien);    

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  static Stats &get_stats(void *) {return stats;}

  const static CreatureContext creatureContext;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */

 
 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  const static StickyContext stickyContext;
  const static PricklyContext pricklyContext;
  const static HealingContext healingContext;

  static CreatureXdata creatureXdata;
  static StickyXdata stickyXdata;
  static PricklyXdata pricklyXdata;
  static HealingXdata healingXdata;

  static Stats stats;
};



class RedHugger: public Creature {
// public Sticky, public Hugger
 public:
  RedHugger(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_CREATURE_IO(RedHugger);    

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  const static CreatureContext creatureContext;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */

  
 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  const static StickyContext stickyContext;
  const static HuggerContext huggerContext;

  static CreatureXdata creatureXdata;
  static StickyXdata stickyXdata;
  static HuggerXdata huggerXdata;
};



class GreenHugger: public Creature {
// public Sticky, public Hugger
 public:
  GreenHugger(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_CREATURE_IO(GreenHugger);    

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  const static CreatureContext creatureContext;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */

  
 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  const static StickyContext stickyContext;
  const static HuggerContext huggerContext;

  static CreatureXdata creatureXdata;
  static StickyXdata stickyXdata;
  static HuggerXdata huggerXdata;
};



class ChopperBoy: public Creature {
// public Flying, public User, public Lifter
 public:
  ChopperBoy(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_CREATURE_IO(ChopperBoy);    

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  static Stats &get_stats(void *) {return stats;}
  
  const static CreatureContext creatureContext;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */
  const static FlyingContext flyingContext;
  // Used by AltarOfSin

 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  static CreatureXdata creatureXdata;
  static FlyingXdata flyingXdata;
  static UserXdata userXdata;
  static LifterXdata lifterXdata;
  
  const static UserContext userContext;
  const static LifterContext lifterContext;
  static Stats stats;
};



class Seal: public Creature {
//, public Grounded, public Suicide, public AnimTime
 public:
  Seal(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_CREATURE_IO(Seal);    

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  static Stats &get_stats(void *) {return stats;}

  const static CreatureContext creatureContext;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */
  const static AnimTimeContext animTimeContext;


 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  const static GroundedContext groundedContext;
  const static SuicideContext suicideContext;

  static CreatureXdata creatureXdata;
  static GroundedXdata groundedXdata;
  static SuicideXdata suicideXdata;
  static AnimTimeXdata animTimeXdata;
  static Stats stats;
};



class FireDemon: public Creature {
//, public Flying, public BuiltIn, public OnFire, public SwapProtect
 public:
  FireDemon(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_CREATURE_IO(FireDemon);    

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);
  

  const static CreatureContext creatureContext;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */
  const static BuiltInContext builtInContext;
  const static OnFireContext onFireContext;
    
  static Size get_shot_size(Dir);
  static PhysicalP create_shot(WorldP,LocatorP,const Pos &,const Id &shooter,Dir dir);
  /* NOTE: For BuiltIn. */
  /* NOTE: Public so that AltarOfSin can use them. */
  

 private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  static CreatureXdata creatureXdata;
  static FlyingXdata flyingXdata;
  static BuiltInXdata builtInXdata;
  static OnFireXdata onFireXdata;
  static SwapProtectXdata swapProtectXdata;  

  const static FlyingContext flyingContext;
  const static SwapProtectContext swapProtectContext;
};



class Dragon: public Creature {
//, public Flying, public BuiltIn, public OnFire, public Segmented
 public:
  enum {SEGMENTS_NUM = 15};

  Dragon(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_CREATURE_IO(Dragon);    

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);
  

  const static CreatureContext creatureContext;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */
  const static BuiltInContext builtInContext;
  const static OnFireContext onFireContext;
    

 private:
  static Size get_shot_size(Dir);
  static PhysicalP create_shot(WorldP,LocatorP,const Pos &,const Id &shooter,Dir dir);

  static void init_x(Xvars&,IXCommand command,void* arg);

  static CreatureXdata creatureXdata;
  static FlyingXdata flyingXdata;
  static BuiltInXdata builtInXdata;
  static OnFireXdata onFireXdata;
  static SegmentedXdata segmentedXdata;  

  const static FlyingContext flyingContext;
  const static SegmentedContext segmentedContext;
};



class Walker: public Creature {
//, public Walking, public User, public BuiltIn, public AnimTime
public:
  Walker(WorldP,LocatorP,const Pos &);

  DECLARE_NULL_CREATURE_IO(Walker);    

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  static Stats &get_stats(void *) {return stats;}
  
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */
  const static CreatureContext creatureContext;
  const static AnimTimeContext animTimeContext;
  // Also used by AltarOfSin
  const static BuiltInContext builtInContext;


private:
  static PhysicalP create_shot(WorldP,LocatorP,const Pos &,const Id &shooter,Dir);
  
  static void init_x(Xvars&,IXCommand command,void* arg);

  static CreatureXdata creatureXdata;
  static WalkingXdata walkingXdata;
  static BuiltInXdata builtInXdata;
  static UserXdata userXdata;
  static AnimTimeXdata animTimeXdata;
  
  const static WalkingContext walkingContext;
  const static UserContext userContext;
  static Stats stats;
};



// Also see "Mutt".
class Dog: public Creature {
// public Walking, public Fighter, public Carrier
 public:
  Dog(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_CREATURE_IO(Dog);    

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  virtual Boolean draw_outline(ColorNum &);
  /* NOTE: Special hack for X. */

  static Stats &get_stats(void *) {return stats;}
  
  const static CreatureContext creatureContext;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */

  static FighterXdata fighterXdata;
  const static FighterContext fighterContext;
  /* NOTE: Similar hack so that Fighter::lookup_context() can get these. */


private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  static CreatureXdata creatureXdata;
  static WalkingXdata walkingXdata;
  static CarrierXdata carrierXdata;

  const static WalkingContext walkingContext;
  const static CarrierContext carrierContext;
  static Stats stats;
};



// Just like dog except a little slower and a little tougher.  
// Sure would be nice to have less code duplication between Dog and Mutt.
class Mutt: public Creature {
// public Walking, public Fighter, public Carrier
 public:
  Mutt(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_CREATURE_IO(Mutt);    

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  virtual Boolean draw_outline(ColorNum &);
  /* NOTE: Special hack for X. */

  static Stats &get_stats(void *) {return stats;}
  
  const static CreatureContext creatureContext;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */

  static FighterXdata fighterXdata;
  const static FighterContext fighterContext;
  /* NOTE: Similar hack so that Fighter::lookup_context() can get these. */


private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  static CreatureXdata creatureXdata;
  static WalkingXdata walkingXdata;
  static CarrierXdata carrierXdata;

  const static WalkingContext walkingContext;
  const static CarrierContext carrierContext;
  static Stats stats;
};



class Yeti: public Creature {
// Walking, Prickly
 public:
  Yeti(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_CREATURE_IO(Yeti);

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  static Stats &get_stats(void *) {return stats;}
  
  const static CreatureContext creatureContext;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */
  const static AnimTimeContext animTimeContext;


private:
  static void init_x(Xvars&,IXCommand command,void* arg);

  static CreatureXdata creatureXdata;
  static WalkingXdata walkingXdata;
  static PricklyXdata pricklyXdata;
  static AnimTimeXdata animTimeXdata;
  static SensitiveXdata sensitiveXdata;

  const static WalkingContext walkingContext;
  const static PricklyContext pricklyContext;
  const static SensitiveContext sensitiveContext;

  static Stats stats;
};



class Chicken: public Creature {
// public Flying, public Fighter
 public:
  Chicken(WorldP w,LocatorP l,const Pos &pos);

  DECLARE_NULL_CREATURE_IO(Chicken);    

  static Size get_size_max();
  static PhysicalP create(void *,WorldP,LocatorP,const Pos &);

  static Stats &get_stats(void *) {return stats;}
  
  const static CreatureContext creatureContext;
  /* NOTE: Making this public is a bit of a hack.  Locator::register_contexts()
     needs to get to it and I don't want to have to make a separate accessor
     function for every class. */

  const static FighterContext fighterContext;


 private:
  static void init_x(Xvars&,IXCommand command,void* arg);
  
  static CreatureXdata creatureXdata;
  static FlyingXdata flyingXdata;
  static FighterXdata fighterXdata;

  const static FlyingContext flyingContext;
  
  static Stats stats;
};
#endif
