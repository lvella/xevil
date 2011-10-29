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

// "actual.cpp"
// Actual objects to be instantiated.


#include "stdafx.h"
#if X11
#ifndef NO_PRAGMAS
#pragma implementation "actual.h"
#endif
#endif

// Include Files
#if X11
#include <strstream>
#endif
#if WIN32
#include <strstrea.h>
#include "resource.h"
#endif

#include "utils.h"
#include "coord.h"
#include "world.h"
#include "physical.h"
#include "actual.h"

#include "sound_cmn.h"


// Bitmap files.  Also contain constants and some simple methods.
// Most importantly, they contain the "contexts".
#include "bitmaps/phys_mover/phys_mover.bitmaps"
#include "bitmaps/explosion/explosion.bitmaps"
#include "bitmaps/fire/fire.bitmaps"
#include "bitmaps/fire_explosion/fire_explosion.bitmaps"
#include "bitmaps/n_protection/n_protection.bitmaps"
#include "bitmaps/t_protection/t_protection.bitmaps"
#include "bitmaps/x_protection/x_protection.bitmaps"

#include "bitmaps/shell/shell.bitmaps"
#include "bitmaps/swap_shell/swap_shell.bitmaps"
#include "bitmaps/lance/lance.bitmaps"
#include "bitmaps/laser/laser.bitmaps"
#include "bitmaps/frog_shell/frog_shell.bitmaps"
#include "bitmaps/fireball/fireball.bitmaps"
#include "bitmaps/missile/missile.bitmaps"
#include "bitmaps/star/star.bitmaps"
#include "bitmaps/blood/blood.bitmaps"
#include "bitmaps/green_blood/green_blood.bitmaps"
#include "bitmaps/oil_droplet/oil_droplet.bitmaps"
#include "bitmaps/feather/feather.bitmaps"

#include "bitmaps/trapdoor/trapdoor.bitmaps"
#include "bitmaps/home/home.bitmaps"
#include "bitmaps/grenade/grenade.bitmaps"
#include "bitmaps/napalm/napalm.bitmaps"
#include "bitmaps/egg/egg.bitmaps"
#include "bitmaps/xit/xit.bitmaps"
#include "bitmaps/flag/flag.bitmaps"

#include "bitmaps/rock/rock.bitmaps"
#include "bitmaps/weight/weight.bitmaps"
#include "bitmaps/altar_of_sin/altar_of_sin.bitmaps"
#include "bitmaps/doppel/doppel.bitmaps"
#include "bitmaps/cloak/cloak.bitmaps"
#include "bitmaps/transmogifier/transmogifier.bitmaps"
#include "bitmaps/med_kit/med_kit.bitmaps"
#include "bitmaps/crack_pipe/crack_pipe.bitmaps"
#include "bitmaps/caffine/caffine.bitmaps"
#include "bitmaps/p_c_p/p_c_p.bitmaps"
#include "bitmaps/n_shield/n_shield.bitmaps"
#include "bitmaps/t_shield/t_shield.bitmaps"
#include "bitmaps/bomb/bomb.bitmaps"

#include "bitmaps/chainsaw/chainsaw.bitmaps"
#include "bitmaps/pistol/pistol.bitmaps"
#include "bitmaps/m_gun/m_gun.bitmaps"
#include "bitmaps/lancer/lancer.bitmaps"
#include "bitmaps/auto_lancer/auto_lancer.bitmaps"
#include "bitmaps/f_thrower/f_thrower.bitmaps"
#include "bitmaps/launcher/launcher.bitmaps"
#include "bitmaps/grenades/grenades.bitmaps"
#include "bitmaps/napalms/napalms.bitmaps"
#include "bitmaps/stars/stars.bitmaps"
#include "bitmaps/swapper/swapper.bitmaps"
#include "bitmaps/frog_gun/frog_gun.bitmaps"
#include "bitmaps/dog_whistle/dog_whistle.bitmaps"
#include "bitmaps/demon_summoner/demon_summoner.bitmaps"

#include "bitmaps/enforcer/enforcer.bitmaps"
#include "bitmaps/frog/frog.bitmaps"
#include "bitmaps/hero/hero.bitmaps"
#include "bitmaps/zombie/zombie.bitmaps"
#include "bitmaps/ninja/ninja.bitmaps"
#include "bitmaps/alien/alien.bitmaps"
#include "bitmaps/hugger/hugger.bitmaps"
#include "bitmaps/chopper_boy/chopper_boy.bitmaps"
#include "bitmaps/seal/seal.bitmaps"
#include "bitmaps/fire_demon/fire_demon.bitmaps"
#include "bitmaps/dragon/dragon.bitmaps"
#include "bitmaps/walker/walker.bitmaps"
#include "bitmaps/dog/dog.bitmaps"
#include "bitmaps/yeti/yeti.bitmaps"
#include "bitmaps/chicken/chicken.bitmaps"

using namespace std;

///// Some helper macros to define creatures with certain abilities.
#define DEFINE_CREATURE_CTORS_1(CLASSNAME,ABILITY0,ability0) \
CLASSNAME::CLASSNAME(WorldP w,LocatorP l,const Pos &rawPos) \
: Creature(creatureContext,creatureXdata, \
           new PtrList(new ABILITY0(ability0 ## Context,ability0 ## Xdata)), \
           w,l,rawPos) {} \
\
CLASSNAME::CLASSNAME(InStreamP inStream,WorldP w,LocatorP l) \
  : Creature(inStream,creatureContext,creatureXdata, \
             w,l) {}

#define DEFINE_CREATURE_CTORS_2(CLASSNAME,ABILITY0,ability0,ABILITY1,ability1) \
CLASSNAME::CLASSNAME(WorldP w,LocatorP l,const Pos &rawPos) \
: Creature(creatureContext,creatureXdata, \
           new PtrList(new ABILITY0(ability0 ## Context,ability0 ## Xdata), \
                       new ABILITY1(ability1 ## Context,ability1 ## Xdata)), \
           w,l,rawPos) {} \
\
CLASSNAME::CLASSNAME(InStreamP inStream,WorldP w,LocatorP l) \
  : Creature(inStream,creatureContext,creatureXdata, \
             w,l) {}

#define DEFINE_CREATURE_CTORS_3(CLASSNAME,ABILITY0,ability0,ABILITY1,ability1,ABILITY2,ability2) \
CLASSNAME::CLASSNAME(WorldP w,LocatorP l,const Pos &rawPos) \
: Creature(creatureContext,creatureXdata, \
           new PtrList(new ABILITY0(ability0 ## Context,ability0 ## Xdata), \
                       new ABILITY1(ability1 ## Context,ability1 ## Xdata), \
                       new ABILITY2(ability2 ## Context,ability2 ## Xdata)), \
           w,l,rawPos) {} \
\
CLASSNAME::CLASSNAME(InStreamP inStream,WorldP w,LocatorP l) \
  : Creature(inStream,creatureContext,creatureXdata, \
             w,l) {}

#define DEFINE_CREATURE_CTORS_4(CLASSNAME,ABILITY0,ability0,ABILITY1,ability1,ABILITY2,ability2,ABILITY3,ability3) \
CLASSNAME::CLASSNAME(WorldP w,LocatorP l,const Pos &rawPos) \
: Creature(creatureContext,creatureXdata, \
           new PtrList(new ABILITY0(ability0 ## Context,ability0 ## Xdata), \
                       new ABILITY1(ability1 ## Context,ability1 ## Xdata), \
                       new ABILITY2(ability2 ## Context,ability2 ## Xdata), \
                       new ABILITY3(ability3 ## Context,ability3 ## Xdata)), \
           w,l,rawPos) {} \
\
CLASSNAME::CLASSNAME(InStreamP inStream,WorldP w,LocatorP l) \
  : Creature(inStream,creatureContext,creatureXdata, \
             w,l) {}

#define DEFINE_CREATURE_CTORS_5(CLASSNAME,ABILITY0,ability0,ABILITY1,ability1,ABILITY2,ability2,ABILITY3,ability3,ABILITY4,ability4) \
CLASSNAME::CLASSNAME(WorldP w,LocatorP l,const Pos &rawPos) \
: Creature(creatureContext,creatureXdata, \
           new PtrList(new ABILITY0(ability0 ## Context,ability0 ## Xdata), \
                       new ABILITY1(ability1 ## Context,ability1 ## Xdata), \
                       new ABILITY2(ability2 ## Context,ability2 ## Xdata), \
                       new ABILITY3(ability3 ## Context,ability3 ## Xdata), \
                       new ABILITY4(ability4 ## Context,ability4 ## Xdata)), \
           w,l,rawPos) {} \
\
CLASSNAME::CLASSNAME(InStreamP inStream,WorldP w,LocatorP l) \
  : Creature(inStream,creatureContext,creatureXdata, \
             w,l) {}


PhysMover::PhysMover(WorldP w,LocatorP l,MoverP mover)
: Physical(context,w,l)
{
	moverId = mover->get_mover_id();
	areaPrev = mover->get_area();
	//  state = AfterUpdate;
	state = BeforeAct;
}



PhysMover::PhysMover(InStreamP in,WorldP w,LocatorP l)
  : Physical(in,context,w,l) {
  state = AfterUpdate;

  _update_from_stream(in);
}



DEFINE_UPDATE_FROM_STREAM(PhysMover,Physical)



void PhysMover::_update_from_stream(InStreamP in) {
  moverId.read(in);
  areaPrev.read(in);

  WorldP world = get_world();
  MoverP mover = world->lookup(moverId);
  if (mover) {
    mover->update_from_stream(areaPrev);
  }
}



int PhysMover::get_write_length() {
  return 
    Physical::get_write_length() + 
    Identifier::get_write_length() +   // moverId
    Area::get_write_length();          // area
}



void PhysMover::write(OutStreamP out) {
  Physical::write(out);

  // Not always true.  Is true after a few turns?
  //  if (state != AfterUpdate) {
  //  cerr << "PhysMover: state should be AfterUpdate when write() is called."
  //       << endl;
  //}

  Area area = get_area();
  Area areaNext = get_area_next();
  if (!(area == areaNext)) {
    cerr << "PhysMover: area should equal areaNext when write() is called." 
         << endl;
  }

  moverId.write(out);
  area.write(out);
}



DEFINE_CREATE_FROM_STREAM(PhysMover)
  


const Area &PhysMover::get_area()
{
	if (state == AfterUpdate)
		return later_area();
	else
		return earlier_area();
}



const Area &PhysMover::get_area_next()
{
	if (state == BeforeAct)
		return earlier_area();
	else
		return later_area();
}



Boolean PhysMover::collidable() {
	return False;
}



Boolean PhysMover::corporeal_attack(PhysicalP,int) 
{
  return False;
}



void PhysMover::heat_attack(PhysicalP,int,Boolean)
{}



void PhysMover::act()
{
	//  assert(state == BeforeAct);  Fails at new level.
	state = BetweenActUpdate;
}



void PhysMover::update()
{
	assert(state == BetweenActUpdate);
	state = AfterUpdate;
}



// Don't actually draw, Locator will draw the background for it.
void PhysMover::draw(CMN_DRAWABLE,Xvars &,int,const Area &) {
}



void PhysMover::init_x(Xvars &,IXCommand,void*) {
}



const Area &PhysMover::earlier_area() {
	WorldP world = get_world();
	MoverP mover = world->lookup(moverId);
	if (mover) {
		areaPrev = mover->get_area() - mover->get_vel();
	}

	// Just to return something.
	return areaPrev;
}



const Area &PhysMover::later_area() {
	WorldP world = get_world();
	MoverP mover = world->lookup(moverId);
	if (mover) {
		return mover->get_area();
	}

	// Just to return something.
	return areaPrev;
}



void PhysMover::mover_clock()
{
	//  assert(state == AfterUpdate);
	// Don't assert, actually two mover_clock() calls before first act() call.
	state = BeforeAct;
}



Explosion::Explosion(WorldP w,LocatorP l,const Pos &mid,
					 const Id &bomb_er,int r,int dMax)
  : Physical(context,w,l) 
{ 
  bomber = bomb_er;
  Pos pos;
  pos.x = mid.x - EXPLOSION_VISIBLE_RADIUS;
  pos.y = mid.y - EXPLOSION_VISIBLE_RADIUS;
  
  Size size;
  size.width = size.height = 2 * EXPLOSION_VISIBLE_RADIUS;
  
  Area narea(AR_RECT,pos,size);
  area = narea;
  middle = mid;
  radius = r;
  damageMax = dMax;
}



Explosion::Explosion(InStreamP in,WorldP w,LocatorP l)
  : Physical(in,context,w,l) {
  damageMax = 0;
  _update_from_stream(in);
}



DEFINE_UPDATE_FROM_STREAM(Explosion,Physical)



void Explosion::_update_from_stream(InStreamP in) {
  area.read(in);  
  middle.read(in);
  radius = in->read_short();
}



int Explosion::get_write_length() {
  return 
    Physical::get_write_length() + 
    Area::get_write_length() +     // area
    Pos::get_write_length() +      // middle
    sizeof(short);                 // radius
}



void Explosion::write(OutStreamP out) {
  Physical::write(out);

  area.write(out);
  middle.write(out);
  assert(radius <= USHRT_MAX);
  out->write_short((u_short)radius);
}



DEFINE_CREATE_FROM_STREAM(Explosion)



Boolean Explosion::collidable()
{
  return False;
}



const Area &Explosion::get_area()
{
	return area;
}



const Area &Explosion::get_area_next()
{
	return area;
}



void Explosion::act() {
	PhysicalP nearby[OL_NEARBY_MAX];
	int nitems;
	LocatorP locator = get_locator();
	locator->get_nearby(nearby,nitems,this,radius);
	
	PhysicalP p = locator->lookup(bomber);
	int hit = 0;
	
	for (int n = 0; n < nitems; n++) {
    // Perhaps should check that nearby[n] is mapped.
		if (nearby[n]->collidable()) {
		  const Area &otherArea = nearby[n]->get_area();
		  int distance = middle.distance(otherArea.get_middle());
		  assert (distance >= 0);
		  nearby[n]->corporeal_attack(p,damageMax*(radius-distance)/radius);
		  hit++;
    }
	}

	ostrstream msg;
	msg << "Explosion hits " << hit << " objects." << ends;
	locator->message_enq(msg.str());

	kill_self();
	Physical::act();
}



void Explosion::init_x(Xvars &,IXCommand,void*) {
}



Fire::Fire(WorldP w,LocatorP l,const Pos &mid,Boolean coll)
: Physical(context,w,l)
{
  Pos pos;
  pos.x = mid.x - FIRE_DIAMETER_INIT / 2;
  pos.y = mid.y - FIRE_DIAMETER_INIT / 2;
  Size size;
  size.width = size.height = FIRE_DIAMETER_INIT;
  
  Area nArea(AR_RECT,pos,size);
  area = areaNext = nArea;
  isCollidable = coll;
}



Fire::Fire(InStreamP in,WorldP w,LocatorP l)
  : Physical(in,context,w,l) {
  isCollidable = False;  
  _update_from_stream(in);
}



DEFINE_UPDATE_FROM_STREAM(Fire,Physical)



void Fire::_update_from_stream(InStreamP in) {
  area.read(in);  
  areaNext = area;
}



int Fire::get_write_length() {
  return 
    Physical::get_write_length() + 
    Area::get_write_length();
}



void Fire::write(OutStreamP out) {
  Physical::write(out);

  assert(area == areaNext);
  area.write(out);
}



DEFINE_CREATE_FROM_STREAM(Fire)



int Fire::get_damage() {
	return FIRE_DAMAGE;
}



const Area &Fire::get_area() {
	return area;
}



const Area &Fire::get_area_next() {
	return areaNext;
}



int Fire::get_drawing_level() {
  // Draw like everything else for now.
  return 1;
}



Boolean Fire::collidable() {
	return isCollidable;
}



void Fire::avoid(PhysicalP){
}



void Fire::collide(PhysicalP other) {
	if (other->is_shot())
		return;
	other->heat_attack(NULL,FIRE_HEAT,True);
}



Boolean Fire::corporeal_attack(PhysicalP,int) {
  return False;
}



void Fire::heat_attack(PhysicalP,int,Boolean) {
}



void Fire::act() {
  Pos pos;
  Size size;
  area.get_rect(pos,size);
  
  size.width -= (FIRE_RADIUS_DELTA * 2);
  size.height = size.width;
  if (size.width <= 0) { // Will be gone before next draw.
    kill_self();
  }
  else {
    pos.x += FIRE_RADIUS_DELTA;
    pos.y += FIRE_VEL_Y;
    Area nArea(AR_RECT,pos,size);
    areaNext = nArea;
  }
  Physical::act();
}



void Fire::update() {
  area = areaNext;
  Physical::update();
}



FireExplosion::FireExplosion(WorldP w,LocatorP l,
                             const Pos &rawPos,const Id &sh)
  : Physical(context,w,l) { 
  shooter = sh;

  Area narea(AR_RECT,rawPos,Fireball::get_size());
  area = narea;
  timer.set(F_THROWER_FIRE_TIME);
}



FireExplosion::FireExplosion(InStreamP in,WorldP w,LocatorP l)
  : Physical(in,context,w,l) {
  timer.set(1);
  _update_from_stream(in);
}



DEFINE_UPDATE_FROM_STREAM(FireExplosion,Physical)



void FireExplosion::_update_from_stream(InStreamP in) {
  area.read(in);  
}



int FireExplosion::get_write_length() {
  return 
    Physical::get_write_length() + 
    Area::get_write_length();     // area
}



void FireExplosion::write(OutStreamP out) {
  Physical::write(out);

  area.write(out);
}



DEFINE_CREATE_FROM_STREAM(FireExplosion)



Boolean FireExplosion::collidable() {
  return False;
}



Boolean FireExplosion::corporeal_attack(PhysicalP,int) {
  return False;
}
  


void FireExplosion::heat_attack(PhysicalP,int,Boolean) {
}



const Area &FireExplosion::get_area() {
	return area;
}



const Area &FireExplosion::get_area_next() {
	return area;
}



void FireExplosion::draw(CMN_DRAWABLE,Xvars &,int,const Area &) {
  // Don't draw at all.
}



void FireExplosion::act() {
  if (timer.ready()) {
    kill_self();
  }
  // Create fireballs going in both directions.
  else {
    WorldP world = get_world();
    LocatorP locator = get_locator();

    PhysicalP left = 
      new Fireball(world,locator,area.get_pos(),shooter,CO_L);
    PhysicalP right = 
      new Fireball(world,locator,area.get_pos(),shooter,CO_R);
    assert(left && right);
    locator->add(left);
    locator->add(right);
  }

  timer.clock();
	Physical::act();
}



void FireExplosion::init_x(Xvars &,IXCommand,void*) {
}



NProtection::NProtection(WorldP w,LocatorP l,const Area &area)
: Protection(context,xdata,w,l,area) {
	n = N_PROTECTION_N;
}



CONSTRUCTOR_LEAF_IO(NProtection,Protection) {
  n = 0;
}



DEFINE_CREATE_FROM_STREAM(NProtection)



Boolean NProtection::corporeal_protect(int) {
	assert(n >= 0);
  if (n <= 0) {
		return False;
  }
	n--;
  if (n == 0 ) {
		kill_self();
  }
	return True;
}



Boolean NProtection::heat_protect(int,Boolean) {
	assert(n >= 0);
	return False;
}



void NProtection::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Protection::init_x(xvars,command,arg,context,xdata);
}



TProtection::TProtection(WorldP w,LocatorP l,const Area &area)
: Protection(context,xdata,w,l,area) {
	timer.set(T_PROTECTION_TIME);
}



CONSTRUCTOR_LEAF_IO(TProtection,Protection) {
}



DEFINE_CREATE_FROM_STREAM(TProtection)



void TProtection::act() {
  if (timer.ready()) {
		kill_self();
  }
	timer.clock();

	Protection::act();
}



Boolean TProtection::corporeal_protect(int) {
	return True;
}



Boolean TProtection::heat_protect(int,Boolean) {
	return True;
}



void TProtection::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Protection::init_x(xvars,command,arg,context,xdata);
}



XProtection::XProtection(WorldP w,LocatorP l,const Area &area)
: Protection(context,xdata,w,l,area) {
  health = X_PROTECTION_HEALTH;
}



CONSTRUCTOR_LEAF_IO(XProtection,Protection) {
}



DEFINE_CREATE_FROM_STREAM(XProtection)



Boolean XProtection::corporeal_protect(int damage) {
  health -= damage;
  if (health <= 0) {
    kill_self();
  }
  return True;
}



Boolean XProtection::heat_protect(int heat,Boolean) {
  // Approximate the amount of damage the heat attack would do.
  int damage = heat * Fire::get_damage();
  return XProtection::corporeal_protect(damage);
}



void XProtection::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Protection::init_x(xvars,command,arg,context,xdata);
}



Rock::Rock(WorldP w,LocatorP l,const Pos &pos) 
: Liftable(context,xdata,w,l,pos) {
}



CONSTRUCTOR_LEAF_IO(Rock,Liftable) {
}



DEFINE_CREATE_FROM_STREAM(Rock)



void Rock::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.heavyContext.fallingContext.movingContext,
                 xdata);
}



Weight::Weight(WorldP w,LocatorP l,const Pos &pos) 
: Liftable(context,xdata,w,l,pos) 
{}



CONSTRUCTOR_LEAF_IO(Weight,Liftable) {
}



DEFINE_CREATE_FROM_STREAM(Weight)



void Weight::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.heavyContext.fallingContext.movingContext,
                 xdata);
}



DoubleSpeed::DoubleSpeed(int timed)
: Doubler(timed)
{
  multiplier = 1.5f;
  increment = 0.5f;
}



ModifierId DoubleSpeed::get_modifier_id() {
  return M_DoubleSpeed;
}



int DoubleSpeed::apply(Attribute attr,int val) {
  switch (attr) {
    case ACCELERATION:
    case AIR_SPEED:
    case CENTER_SPEED:
    case CLIMB_SPEED:
    case CRAWL_SPEED:
      val = (int)(val * multiplier);
      break;
  };

  if (next)
    return next->apply(attr,val);
  else
    return val;
}



ModifierId DoubleJump::get_modifier_id() {
  return M_DoubleJump;
}



int DoubleJump::apply(Attribute attr,int val) {
  switch (attr) {
    // Side effect that flying creatures get an extra boost, but AltarOfSin
    // doesn't give DoubleJump to Flying creatures.
    case AIR_SPEED: 

    case JUMP:
      val = (int)(val * multiplier);
      break;
  }

  if (next)
    return next->apply(attr,val);
  else
    return val;
}



ModifierId DoubleHealth::get_modifier_id() {
  return M_DoubleHealth;
}



int DoubleHealth::apply(Attribute attr,int val) {
  if (attr == HEALTH_MAX) {
    val = (int)(val * multiplier);
  }

  if (next)
    return next->apply(attr,val);
  else
    return val;
}



AltarOfSin::AltarOfSin(WorldP w,LocatorP l,const Pos &pos)
: Heavy(context,xdata,w,l,pos) {
  turnTaken = False;
}



CONSTRUCTOR_LEAF_IO(AltarOfSin,Heavy) {
  turnTaken = False;
}



DEFINE_CREATE_FROM_STREAM(AltarOfSin)



Boolean AltarOfSin::corporeal_attack(PhysicalP other,int) {
  /* Don't fuck with the Altar of Sin. */
  
  if (turnTaken) {
    return False;
  }
  
  IntelP intel;
  if (other && (other->get_class_id() != A_AltarOfSin) &&
      (intel = other->get_intel())) {
    LocatorP locator = get_locator();
    ostrstream str;
    
    // Turned into a frog/baby seal.
    if (Utils::coin_flip() && other->is_moving() && 
        // Don't frog a frog, or something that has already been morphed.
        other->get_class_id() != A_Frog && 
        other->get_class_id() != A_Seal && 
        !other->get_ability(AB_Morphed)) {

      Boolean doFrog = Utils::coin_flip();
      
      str << intel->get_name() << " attacks the Altar of Sin and is "
          << "turned into a " << (doFrog ? "frog" : "baby-seal")
          << "." << ends;
      locator->message_enq(str.str());
  
      other->set_intel(NULL);
      if (!other->get_mapped()) {
        cerr << "Warning:AltarOfSin::corporeal_attack: object "
         << "should be mapped." << endl;
      }
      other->set_mapped_next(False);
    
      const Area &area = other->get_area();
    
      PhysicalP morph;
      if (doFrog) {
        Pos pos = area.get_middle() - 0.5f * Frog::get_size_max();
        morph = new Frog(get_world(),locator,pos);
      }
      else {
        Pos pos = area.get_middle() - 0.5f * Seal::get_size_max();
        morph = new Seal(get_world(),locator,pos);
      } 
      assert(morph);
      
      // Give frog the morphed ability, so it will eventually turn back.
      AbilityP morphed = new Morphed(FrogShell::morphedContext,Morphed::defaultXdata,other);
      ((CreatureP)morph)->add_ability(morphed);
        
      morph->set_intel(intel);
      locator->add(morph);
    }
    // Lose all health.
    else {
      str << "BLASPHMER!  " << intel->get_name() << 
          " loses health for daring to attack the Altar of Sin." << ends;
      locator->message_enq(str.str());
      
      int damage = other->get_health();
      other->corporeal_attack(this,damage);
    }
      
    locator->arena_message_enq(Utils::strdup("Don't FUCK with the Altar of Sin"),
                                 other);
    turnTaken = True;
  }

  return True;
}



void AltarOfSin::heat_attack(PhysicalP,int,Boolean){
}



void AltarOfSin::collide(PhysicalP other) {
  IntelP intel;
  if (!turnTaken && other->is_creature() && 
      (intel = other->get_intel()) && intel->is_human()) {
    int lives = intel->get_lives();
    LocatorP locator = get_locator();
    ostrstream msg;
    ostrstream arenaMsg;

    // Choose different blessings to give.
    int n = 0;
    Boolean ok = False;
    while (!ok && n < ALTAR_OF_SIN_TRIES) {
      n++;
      ok = True;
      ModifierP mod;

      /*** Appending to the Intel's or Creature's modifier list. ***/
      ModifierP modList = intel->get_modifiers();
      assert(modList);
      ModifierP pModList = ((CreatureP)other)->get_modifiers();
      assert(pModList);
      
      int which = Utils::choose(9);
      switch (which) {
        // extra life or extra kills.
        case 0:
        // If human has infinite lives, give him some points(kills) instead.
        if (lives == IT_INFINITE_LIVES) {
#if 0
          for (int m = 0; m < ALTAR_OF_SIN_KILLS; m++) {
            intel->add_human_kill();
          }

          msg << intel->get_name() << " sells soul for " 
              << ALTAR_OF_SIN_KILLS << " kills." << ends;
          arenaMsg << "You Gain " << ALTAR_OF_SIN_KILLS << " Kills" << ends;
#endif
          // Everybody hates the "You Gain 5 Kills" thing.  So, I removed it.
          ok = False;
        }
        else {
          intel->set_lives(lives + 1);
          msg << intel->get_name() << " sells soul for an extra life." << ends;
          arenaMsg << "You Sold Your Soul For an Extra Life" << ends;
        }
        break;
    
    
        // double speed
        case 1:
        mod = new DoubleSpeed();
        assert(mod);          
        modList->append_unique(mod);
        msg << intel->get_name() << " sells soul for Double Speed." << ends;
        arenaMsg << "Double Speed" << ends;
        break;

        
        // extra jump
        case 2:
        if (other->get_ability(AB_Flying) || other->get_ability(AB_Grounded)) {
          // extra jump is useless or nearly so.
          ok = False;
          break;
        }
        mod = new DoubleJump();
        assert(mod);          
        modList->append_unique(mod);
        msg << intel->get_name() << " sells soul for extra jumping powers." 
            << ends;
        arenaMsg << "Extra Jumping Powers" << ends;
        break;
        

        // double maximum health
        case 3:
        mod = new DoubleHealth();
        assert(mod);          

        // Add to Creature's modifiers because of problems with
        // initializing health when transferred from one physical
        // to another.
        pModList->append_unique(mod);
        
        // So max health takes effect immediately.
        other->heal();
        
        msg << intel->get_name() << " sells soul for Double Health." << ends;
        arenaMsg << "Double Health" << ends;
        break;


        // Healing powers
        case 4:  {     
          // Already has healing powers.
          if (other->get_ability(AB_Healing)) {
            ok = False;
            break;
          }
          AbilityP h = new Healing(Hero::healingContext,Healing::defaultXdata);
          // Already checked that other is a Creature().
          ((CreatureP)other)->add_ability(h);

          // Might as well make sure they can take advantage of it now.  
          other->heal();

          msg << intel->get_name() << " sells soul for Healing Powers." << ends;
          arenaMsg << "Healing Powers" << ends;
        }
        break;


        // HellFire
        case 5:  {     
          // Already has OnFire powers.
          if (other->get_ability(AB_OnFire)) {
            ok = False;
            break;
          }
          AbilityP a = new OnFire(FireDemon::onFireContext,OnFire::defaultXdata);
          // Already checked that other is a Creature().
          ((CreatureP)other)->add_ability(a);

          msg << intel->get_name() << " sells soul for HellFire Powers." << ends;
          arenaMsg << "HellFire Powers" << ends;    
        }
        break;


        // Fireballs
        case 6:  {     
          // Already has BuiltIn weapon
          if (other->get_ability(AB_BuiltIn)) {
            ok = False;
            break;
          }

          // Don't want to be BuiltIn and Fighter at the same time.
          AbilityP fighter = other->get_ability(AB_Fighter);
          if (fighter) {
            ((CreatureP)other)->remove_ability(AB_Fighter);
          }

          AbilityP a = new BuiltIn(FireDemon::builtInContext,BuiltIn::defaultXdata);
          // Already checked that other is a Creature().
          ((CreatureP)other)->add_ability(a);

          msg << intel->get_name() << " sells soul for Fireballs." << ends;
          arenaMsg << "Fireballs" << ends;    
        }
        break;
        
        // Flying powers
        case 7:  {     
          // Already has flying powers.
          if (other->get_ability(AB_Flying)) {
            ok = False;
            break;
          }

          // Remove the existing Locomotion
          LocomotionP l = other->get_locomotion();
          if (l) {
            ((CreatureP)other)->remove_ability(l->get_ability_id());
          }
          // else something is wrong.

          AbilityP a = new Flying(ChopperBoy::flyingContext,Flying::defaultXdata);
          // Already checked that other is a Creature().
          ((CreatureP)other)->add_ability(a);

          msg << intel->get_name() << " sells soul for Flying Powers." << ends;
          arenaMsg << "Flying Powers" << ends;
        }
        break;


        // Sticky ability
        case 8: {
          // Already has sticky.
          // Also, don't kill off Flying to replace with Sticky.
          if (other->get_ability(AB_Sticky) || other->get_ability(AB_Flying)) {
            ok = False;
            break;
          }

          // Has to have all the bitmaps required for Sticky, e.g.
          // climbing on the walls and ceiling.
          if (!Sticky::has_required_frames(((MovingP)other)->get_moving_context())) {
            ok = False;
            break;
          }

          // Remove the existing Locomotion
          LocomotionP l = other->get_locomotion();
          if (l) {
            ((CreatureP)other)->remove_ability(l->get_ability_id());
          }
          // else something is wrong.

          AbilityP a = new Sticky(Ninja::stickyContext,Sticky::defaultXdata);
          // Already checked that other is a Creature().
          ((CreatureP)other)->add_ability(a);

          msg << intel->get_name() << " sells soul for Sticky Powers." << ends;
          arenaMsg << "Sticky Powers" << ends;          
        }
        break;


        default:
        ok = False; // try again.
      }
    } // while
      
    locator->message_enq(msg.str());
    locator->arena_message_enq(arenaMsg.str(),other);

    kill_self();
    turnTaken = True;
  }
  else {
    Heavy::collide(other);
  }
}



void AltarOfSin::update()
{
  turnTaken = False;
  Heavy::update();
}



void AltarOfSin::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.fallingContext.movingContext,
    xdata);
}



Doppel::Doppel(WorldP w,LocatorP l,const Pos &pos)
: AutoUse(context,xdata,w,l,pos)
{}



CONSTRUCTOR_LEAF_IO(Doppel,AutoUse) {
}



DEFINE_CREATE_FROM_STREAM(Doppel)



void Doppel::use(PhysicalP p) {
	assert(p && p->get_intel());

	// If p is not allowed to use doppelgangers, just return.
	const PhysicalContext *usersPC = p->get_context();
  if (!usersPC->doppelUser) {
		return;
  }

	stats.add_use();
	WorldP world = get_world();
	LocatorP locator = get_locator();
	IntelP masterIntel = p->get_intel();
	
	PhysicalP obj = create_physical(p->get_area(),p);

	NeutralP neutral = 
		new DoppelIntel(world,locator,"Doppel",
                    &DoppelIntel::suggestedOptions,DoppelIntel::suggestedMask,
		                masterIntel);
	assert(neutral);
	locator->register_neutral(neutral);

	locator->add(obj);
	obj->set_intel(neutral);

	//  obj->set_dont_collide(p);
	// Use the dont_collide functions of the intel, not the physical.
	neutral->set_dont_collide(masterIntel->get_intel_id());  

	kill_self();
	Item::use(p);
}



// New version which uses the PhysicalContext create function
// More general and easy to extend.
PhysicalP Doppel::create_physical(const Area &area,PhysicalP original) {
	WorldP world = get_world();
	LocatorP locator = get_locator();
	Pos middle = area.get_middle();
	PhysicalP obj;
	
	// Get the PhysicalContext(description of its class) of the original
	const PhysicalContext *originalContext = original->get_context();

	// Use the original's PhysicalContext to create a new object.
	obj = originalContext->create(originalContext->arg,world,locator,
		middle - 0.5f * originalContext->sizeMax);

	assert(obj);
	return obj;
}



void Doppel::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.itemContext.fallingContext.movingContext,
    xdata);
}



Stats Doppel::stats;



Cloak::Cloak(WorldP w,LocatorP l,const Pos &pos)
: Item(context,xdata,w,l,pos)
{}



CONSTRUCTOR_LEAF_IO(Cloak,Item) {
}



DEFINE_CREATE_FROM_STREAM(Cloak)



void Cloak::use(PhysicalP p) {
  assert(p->is_moving());

	// Destroy any protection p may be using.
	LocatorP l = get_locator();
	PhysicalP prot = l->lookup(((MovingP)p)->get_protection());
  if (prot) {
		prot->kill_self();
  }

  // Only make invisible if the Object supports invisibilty.
	// Don't bother doing anything on an already invisible user.
	if (((MovingP)p)->supports_invisibility() && 
      !((MovingP)p)->is_invisible()) {
    ((MovingP)p)->set_invisible_next(PH_INVISIBLE_TIME);

		// So p will stop being invisible when p attacks something.
		((MovingP)p)->set_attack_hook(attack_hook);
  	
    stats.add_use();
	}

	kill_self();
	Item::use(p);
}



void Cloak::attack_hook(PhysicalP p) {
	assert(p->is_moving());
  // Means turn it off.
	((MovingP)p)->set_invisible_next(-1);
}



void Cloak::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.fallingContext.movingContext,
    xdata);
}



Stats Cloak::stats;



Transmogifier::Transmogifier(WorldP w,LocatorP l,const Pos &pos)
: AutoUse(context,xdata,w,l,pos)
{}



CONSTRUCTOR_LEAF_IO(Transmogifier,AutoUse) {
}



DEFINE_CREATE_FROM_STREAM(Transmogifier)



void Transmogifier::use(PhysicalP other) {
  assert(other->alive());
  stats.add_use();
  
  IntelP intel = other->get_intel();
  
  // Must check that other does not have protection against swapping.
  if (intel && !other->swap_protect()) {
    LocatorP locator = get_locator();
    const Area &area = other->get_area();
    // Don't transmogify something into a new object of the same class
    // it already is.  That's boring.
    PhysicalP p = new_physical(area.get_middle(),other->get_class_id());
    locator->add(p);
    
    p->set_intel(intel);
    other->set_intel(NULL);
    // Kill non-persistent objects, don't leave Bombs around.
    other->drop_all(True); 
    other->set_quiet_death();
    other->kill_self();
  }
  kill_self();
  AutoUse::use(other);
}



// New version which uses Locator::filter_contexts.
// More general and easy to extend.
PhysicalP Transmogifier::new_physical(const Pos &middle,ClassId notThis) {
  const PhysicalContext *list[A_CLASSES_NUM];
  int size;
  LocatorP l = get_locator();
  WorldP w = get_world();

  // Get list of all classes that are potential transmogify targets.
  size = l->filter_contexts(list,NULL,transmogify_target_filter,
                            (void*)notThis);

  // Something is wrong if size == 0, because there are no classes to 
  // transmogify into.
  assert(size);

  // Randomly choose.
  int which = Utils::choose(size);

  // Create object centered on middle.
  Pos pos = middle - 0.5f * list[which]->sizeMax;
    // Make sure a create function exists.
  assert(list[which]->create);
  PhysicalP obj = list[which]->create(list[which]->arg,w,l,pos);

  return obj;
}



Boolean Transmogifier::transmogify_target_filter(const PhysicalContext *pc,
                                                 void *closure) {
  // notThis is used to prevent transmogifying into the same class an object
  // already is.
  ClassId notThis = (ClassId)closure;
  if (pc->classId == notThis) {
    return False;
  }
  return pc->transmogifyTarget;
}



void Transmogifier::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.itemContext.fallingContext.movingContext,
                 xdata);
}



Stats Transmogifier::stats;



MedKit::MedKit(WorldP w,LocatorP l,const Pos &pos) 
: AutoUse(context,xdata,w,l,pos) {
}



CONSTRUCTOR_LEAF_IO(MedKit,AutoUse) {
}



DEFINE_CREATE_FROM_STREAM(MedKit)



void MedKit::use(PhysicalP p) {
  stats.add_use();
  assert(p->alive());
  p->heal();
  kill_self();
  AutoUse::use(p);
}



void MedKit::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.itemContext.fallingContext.movingContext,
                 xdata);
}



Stats MedKit::stats;



Crack::Crack()
: Modifier(CRACK_TIME) {}



ModifierId Crack::get_modifier_id() {
  return M_Crack;
}



int Crack::apply(Attribute attr,int val) {
  switch (attr) {
  case ACCELERATION:
  case AIR_SPEED:
  case CENTER_SPEED:
  case CLIMB_SPEED:
  case CRAWL_SPEED:
    // Using *= gives warnings on some compilers.
    val = (int)(val * 2.0);
    break;
  case JUMP:
    val = (int)(val * 1.5);
    break;
  case DAMAGE:
    val = val * 2;
    break;
  case HIGH:
    val = 1;  // Creature is high.
    break;
  };
  
  if (next)
    return next->apply(attr,val);
  else
    return val;
}



CrackPipe::CrackPipe(WorldP w,LocatorP l,const Pos &pos) 
: Drugs(context,xdata,w,l,pos) 
{}



CONSTRUCTOR_LEAF_IO(CrackPipe,Drugs) {
}



DEFINE_CREATE_FROM_STREAM(CrackPipe)



ModifierP CrackPipe::create_modifier() {
  stats.add_use();
  ModifierP crack = new Crack();
  assert(crack);
  return crack;
}



void CrackPipe::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.autoUseContext.itemContext.fallingContext.movingContext,
                 xdata);
}



Stats CrackPipe::stats;



CaffineM::CaffineM()
: Modifier(CAFFINE_M_TIME) {}



ModifierId CaffineM::get_modifier_id() {
  return M_CaffineM;
}



int CaffineM::apply(Attribute attr,int val) {
  switch (attr) {
  case ACCELERATION:
  case AIR_SPEED:
  case CENTER_SPEED:
  case CLIMB_SPEED:
  case CRAWL_SPEED:
    // Using *= gives warnings on some compilers.
    val = (int)(val * 1.4);
    break;
  case JUMP:
    val = (int)(val * 1.3);
    break;
  case HIGH:
    val = 1;  // Creature is high.
    break;
  };
  
  if (next)
    return next->apply(attr,val);
  else
    return val;
}



Caffine::Caffine(WorldP w,LocatorP l,const Pos &pos) 
: Drugs(context,xdata,w,l,pos) 
{}



CONSTRUCTOR_LEAF_IO(Caffine,Drugs) {
}



DEFINE_CREATE_FROM_STREAM(Caffine)



ModifierP Caffine::create_modifier() {
  stats.add_use();
  ModifierP m = new CaffineM();
  assert(m);
  return m;
}



void Caffine::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.autoUseContext.itemContext.fallingContext.movingContext,
                 xdata);
}



Stats Caffine::stats;



PCPM::PCPM()
: Modifier(PCP_M_TIME) {}



ModifierId PCPM::get_modifier_id() {
  return M_PCPM;
}



int PCPM::apply(Attribute attr,int val) {
  switch (attr) {
  case HIGH:
    val = 1;  // Creature is high.
    break;
  case CORPOREAL_ATTACK:
    if (val > 0) {
      // Turn damage into "superficial damage".
      val = -val;
    }
    break;
  case DAMAGE:
    val = val * 2;
    break;
  case HEAT_ATTACK:
    val = 0;
    break;
  };
  
  if (next) {
    return next->apply(attr,val);
  }
  else {
    return val;
  }
}



void PCPM::preDie(PhysicalP p) {
  // Whack off half of health.
  p->corporeal_attack(NULL,p->get_health() / 2);
}



PCP::PCP(WorldP w,LocatorP l,const Pos &pos) 
: Drugs(context,xdata,w,l,pos) {
}



CONSTRUCTOR_LEAF_IO(PCP,Drugs) {
}



DEFINE_CREATE_FROM_STREAM(PCP)



ModifierP PCP::create_modifier() {
  stats.add_use();
  ModifierP m = new PCPM();
  assert(m);
  return m;
}



void PCP::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.autoUseContext.itemContext.fallingContext.movingContext,
                 xdata);
}



Stats PCP::stats;



NShield::NShield(WorldP w,LocatorP l,const Pos &pos) 
: Shield(context,xdata,w,l,pos) {
}



CONSTRUCTOR_LEAF_IO(NShield,Shield) {
}



DEFINE_CREATE_FROM_STREAM(NShield);



void NShield::use(PhysicalP p) {
  stats.add_use();
  Shield::use(p);
}



ProtectionP NShield::create_protection(const Area &area) {
  ProtectionP pr = new NProtection(get_world(),get_locator(),area);
  assert(pr);
  return pr;
}



void NShield::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,
                 context.autoUseContext.itemContext.fallingContext.movingContext,
                 xdata);
}



Stats NShield::stats;



TShield::TShield(WorldP w,LocatorP l,const Pos &pos) 
: Shield(context,xdata,w,l,pos) {
}



CONSTRUCTOR_LEAF_IO(TShield,Shield) {
}



DEFINE_CREATE_FROM_STREAM(TShield)



void TShield::use(PhysicalP p) {
	stats.add_use();
	Shield::use(p);
}



ProtectionP TShield::create_protection(const Area &area) {
  ProtectionP pr = new TProtection(get_world(),get_locator(),area);
  assert(pr);
  return pr;
}



void TShield::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,
                 context.autoUseContext.itemContext.fallingContext.movingContext,
                 xdata);
}



Stats TShield::stats;



Bomb::Bomb(WorldP w,LocatorP l,const Pos &p) :
Animated(context,xdata,w,l,p,BOMB_FRAME_INACTIVE) 
{
//	set_frame(BOMB_FRAME_INACTIVE);
//	set_frame_next(BOMB_FRAME_INACTIVE);
	frame = BOMB_FRAME_INACTIVE;

	Timer ntimer(BOMB_TIME); 
	timer = ntimer;
	active = False;
	defused = False;
}



CONSTRUCTOR_LEAF_IO(Bomb,Animated) {
  active = False;
  defused = False;
}



DEFINE_CREATE_FROM_STREAM(Bomb)



Boolean Bomb::is_bomb() {
  return True;
}



void Bomb::use(PhysicalP bomberP) {
  if (!active) {
    frame = BOMB_FRAME_ACTIVE;
    timer.set();
    active = True;
    if (bomberP) {
      bomber = bomberP->get_id();
    }
    LocatorP locator = get_locator();
#if 0
    // Don't send the 4,3,2,etc to to message bar anymore.
    ostrstream msg;
    msg << frame << ends;
    locator->message_enq(msg.str());
#endif    

    set_cant_take();
  }
  Animated::use(bomberP);
}



void Bomb::act() {
  if (active)
    timer.clock();
  
  if (active && timer.ready()) {
    if (! frame) {
      kill_self();
    }
    else	{
	    timer.set();
	    frame--;
	    
#if 0
	    LocatorP locator = get_locator();
	    ostrstream msg;
	    msg << frame << ends;
	    locator->message_enq(msg.str());
#endif
   	}
  }
  
  set_frame_next(frame);
  
  Animated::act();
}



void Bomb::set_quiet_death() {
  defused = True;
  Animated::set_quiet_death();
}



void Bomb::die() {
  if (!defused) {
    // Kind of a hack, recording bombs that exploded, not bombs that 
    // were used.
    stats.add_use();
    
    WorldP world = get_world();
    LocatorP locator = get_locator();
    const Area area = get_area();
    
    PhysicalP explosion = 
      new Explosion(world,locator,area.get_middle(),bomber,
                    BOMB_EXPLOSION_RADIUS,BOMB_EXPLOSION_DAMAGE_MAX);
    assert (explosion);
    locator->add(explosion);
  }  
  
  Animated::die();
}



void Bomb::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,
                 context.itemContext.fallingContext.movingContext,
                 xdata);
}



Stats Bomb::stats;



Trapdoor::Trapdoor(WorldP w,LocatorP l,const Pos &pos,const Id &home_id)
: Moving(context,xdata,w,l,pos) {
  Timer nTimer(TRAPDOOR_TIME);
  timer = nTimer;
  timer.set();
  
  sealsNum = 0;
  homeId = home_id;
}



CONSTRUCTOR_LEAF_IO(Trapdoor,Moving) {
  sealsNum = 0;
}



DEFINE_CREATE_FROM_STREAM(Trapdoor)



Boolean Trapdoor::collidable()
{
	return False;
}



int Trapdoor::get_drawing_level()
{
	return 0;
}



void Trapdoor::append_seals_out(int &num,IntelId *lems)
{
  // num starts out at the correct initial value.
  for (int i = 0; i < sealsNum; i++, num++) {
    lems[num] = seals[i];
  }
}



void Trapdoor::act()
{
  if (timer.ready() && sealsNum < SEALS_MAX)
	{
		LocatorP l = get_locator();
		WorldP w = get_world();
		const Area &area = get_area();
		Pos pos = area.get_middle();
		Size sealSize = Seal::get_size_max();
		pos.x -= sealSize.width / 2;
		pos.y += sealSize.height;
		PhysicalP seal = new Seal(w,l,pos);
		assert(seal);

		char sealStr[20];
		ostrstream str(sealStr,20);
		str << "seal-" << sealsNum << ends;
		NeutralP sealIntel = new SealIntel(w,l,sealStr,homeId);
		seal->set_intel(sealIntel);
		l->register_neutral(sealIntel);
		seals[sealsNum] = sealIntel->get_intel_id();

		l->add(seal);
		
		sealsNum++;
		timer.set();
	}

	timer.clock();
	Moving::act();
}



void Trapdoor::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context,
                 xdata);
}



Home::Home(WorldP w,LocatorP l,const Pos &pos)
: Moving(context,xdata,w,l,pos)
{
  sealsSafe = 0;
}



CONSTRUCTOR_LEAF_IO(Home,Moving) {
  sealsSafe = 0;
}



DEFINE_CREATE_FROM_STREAM(Home)



Boolean Home::collidable()
{
	return False;
}



int Home::get_drawing_level()
{
	return 0;
}



void Home::act() {
	PhysicalP nearby[OL_NEARBY_MAX];
	int nItems;
	LocatorP locator = get_locator();
	locator->get_nearby(nearby,nItems,this,HOME_RADIUS);

	for (int n = 0; n < nItems; n++) {
		IntelP intel = nearby[n]->get_intel();
		if (intel && intel->is_seal_intel()) {
			nearby[n]->set_quiet_death();
			nearby[n]->kill_self();
			sealsSafe++;
		}
	}
	Moving::act();
}



void Home::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context,
                 xdata);
}



Shell::Shell(WorldP w,LocatorP l,const Pos &p,const Id &shooter,
			 Dir d) 
  : Shot(context,xdata,w,l,p,shooter,d) 
{
  stats.add_creation();
}



CONSTRUCTOR_LEAF_IO(Shell,Shot) {
}



DEFINE_CREATE_FROM_STREAM(Shell)



void Shell::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.movingContext,
                 xdata);
}



Stats Shell::stats;



SwapShell::SwapShell(WorldP w,LocatorP l,const Pos &p,
					 const Id &sh,const Id &sw,Dir d)
  : Shot(context,xdata,w,l,p,sh,d)
{
  swapper = sw;
  stats.add_creation();
}



CONSTRUCTOR_LEAF_IO(SwapShell,Shot) {
}



DEFINE_CREATE_FROM_STREAM(SwapShell)



void SwapShell::collide(PhysicalP other) {
	if (other->is_shot()) {
		return;
  }

	LocatorP locator = get_locator();
	const Id &shooter = get_shooter();
	PhysicalP shooterP;

	// Do nothing if shooter no longer exists.
	if (!other->swap_protect() && 
		(shooterP = locator->lookup(shooter))) {
		// Swap souls.
		IntelP tmp = other->get_intel();
		/* Don't swap unless both shooter and other have intelligence. */
		if (tmp && shooterP->get_intel())  {
			other->set_intel(shooterP->get_intel());
			shooterP->set_intel(tmp);
			
			// Destroy swapper.
			PhysicalP swapperP = locator->lookup(swapper);
			if (swapperP) {
				swapperP->kill_self();
      }
		}
	}

	kill_self();
}



void SwapShell::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.movingContext,
                 xdata);
}



Stats SwapShell::stats;



Lance::Lance(WorldP w,LocatorP l,const Pos &p,const Id &shooter,Dir d)
: Shot(context,xdata,w,l,p,shooter,d,d) 
{
  stats.add_creation();
}



CONSTRUCTOR_LEAF_IO(Lance,Shot) {
}



DEFINE_CREATE_FROM_STREAM(Lance)



// This function is also used by Laser::compute_weapon_dir().
Dir Lance::compute_weapon_dir(ITcommand command)
{
	switch (command) {
	case IT_WEAPON_R:
		return CO_R;
	case IT_WEAPON_DN_R:
		return CO_DN_R_R;
	case IT_WEAPON_DN:
		return CO_DN;
	case IT_WEAPON_DN_L:
		return CO_DN_L_L;
	case IT_WEAPON_L:
		return CO_L;
	case IT_WEAPON_UP_L:
		return CO_UP_L_L;
	case IT_WEAPON_UP:
		return CO_UP;
	case IT_WEAPON_UP_R:
		return CO_UP_R_R;
	};
	return CO_air;
}



void Lance::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.movingContext,
                 xdata);
}



Stats Lance::stats;



Laser::Laser(WorldP w,LocatorP l,const Pos &p,const Id &shooter,Dir d)
: Shot(context,xdata,w,l,p,shooter,d,d) 
{
  stats.add_creation();
}



CONSTRUCTOR_LEAF_IO(Laser,Shot) {
}



DEFINE_CREATE_FROM_STREAM(Laser)



Dir Laser::compute_weapon_dir(ITcommand command)
{
  // Kind of a hack.
  return Lance::compute_weapon_dir(command);
}



void Laser::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.movingContext,xdata);
}



Stats Laser::stats;



FrogShell::FrogShell(WorldP w,LocatorP l,const Pos &p,
					 const Id &shooter,const Id &frog_gun,Dir d) 
					 : Shot(context,xdata,w,l,p,shooter,d) 
{
  frogGun = frog_gun;
  stats.add_creation();
}



CONSTRUCTOR_LEAF_IO(FrogShell,Shot) {
}



DEFINE_CREATE_FROM_STREAM(FrogShell)
  
  
  
void FrogShell::collide(PhysicalP other) {
  // Be careful not to frog a Frog, or something that is already morphed.
  if (other->is_shot() || other->get_class_id() == A_Frog || 
      other->get_ability(AB_Morphed)) {
    return;
  }
  
  IntelP intel = other->get_intel();
  if (!other->frog_protect() && intel && other->is_moving()) {
    other->set_intel(NULL);
    assert(other->get_mapped());
    other->set_mapped_next(False);
    
    const Area &area = other->get_area();
    Pos pos = area.get_middle() - 0.5f * Frog::get_size_max();
    
    LocatorP locator = get_locator();
    PhysicalP frog = new Frog(get_world(),locator,pos);
    assert(frog);
    
    // Give frog the morphed ability, so it will eventually turn back.
    AbilityP morphed = new Morphed(morphedContext,Morphed::defaultXdata,other);
    ((CreatureP)frog)->add_ability(morphed);
    
    frog->set_intel(intel);
    locator->add(frog);
    
    // Destroy frogGun.
    PhysicalP p;
    if (p = locator->lookup(frogGun)) {
      p->kill_self();
    }
  }
  
  kill_self();
}



void FrogShell::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.movingContext,
                 xdata);
}



Stats FrogShell::stats;



Fireball::Fireball(WorldP w,LocatorP l,const Pos &p,const Id &shooter,
				   Dir d,int h,int t,Boolean hOnFire) 
  : Shot(context,xdata,w,l,p,shooter,d) {
  if (t == -1) {
    timer.set(FIREBALL_TIME);
  }
  else {
    timer.set(t);
  }
  
  if (h == -1) {
    heat = FIREBALL_HEAT;
  }
  else {
    heat = h;
  }

  hurtOnFire = hOnFire;
}



CONSTRUCTOR_LEAF_IO(Fireball,Shot) {
  heat = 0;
  hurtOnFire = False;
}



DEFINE_CREATE_FROM_STREAM(Fireball)



void Fireball::collide(PhysicalP other) {
  // Fireballs are not destroyed by shots.
  if (other->is_shot() && other->get_class_id() != A_Missile) {
    return;
  }

  LocatorP locator = get_locator();
  PhysicalP p = locator->lookup(get_shooter());

  // If we have the flag set to hurt Creatures with the OnFire 
  // ability,do a one-time corporeal_attack().
  if (hurtOnFire && other->get_ability(AB_OnFire)) {
    other->corporeal_attack(p,FIREBALL_BACKUP_PHYSICAL_DAMAGE);
    kill_self();
  }
  else {
    other->heat_attack(p,heat);
  }
}



void Fireball::act() {
  if (timer.ready()) {
    kill_self();
  }

  timer.clock();
  Shot::act();
}



void Fireball::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.movingContext,
                 xdata);
}



Missile::Missile(WorldP w,LocatorP l,const Pos &p,const Id &sh,Dir d)
: Shot(context,xdata,w,l,p,sh,d,d) 
{
	timer.set(MISSILE_DESTRUCT_TIME);
	Timer nTimer(MISSILE_ROTATE_TIME);
	rotate = nTimer;
	hasTarget = False;
	shooterId = sh;
	stats.add_creation();
}



CONSTRUCTOR_LEAF_IO(Missile,Shot) {
}



DEFINE_CREATE_FROM_STREAM(Missile)



void Missile::act() {
  LocatorP locator = get_locator();
  
  // Find a target.
  if (!hasTarget) {
    PhysicalP nearby[OL_NEARBY_MAX];
    int nearbyNum;
    // Should work even though this not fully constructed.
    locator->get_nearby(nearby,nearbyNum,this,MISSILE_RADIUS);
    
    // Search through list for best possible candidate.
    const Area &area = get_area();
    Pos middle = area.get_middle();
    int distance_2 = -1;  // Best so far.
    
    IntelP nearbyIntel;
    PhysicalP shooter;
    IntelP shooterIntel;
    
    for (int n = 0; n < nearbyNum; n++) {
      if (nearby[n]->alive() &&
	  
	      nearby[n]->is_creature() &&
	      
	      // Don't go after invisible things.
	      !(nearby[n]->is_moving() && ((MovingP)nearby[n])->is_invisible())&&
	      
	      // Don't go after shooter.
	      nearby[n]->get_id() != shooterId &&
	      
	      // Don't go after nearby if it is a slave to the shooter's intel
	      // or the shooter is a slave to nearby.
	      !(
	        (shooter  = locator->lookup(shooterId)) &&
	        (nearbyIntel = nearby[n]->get_intel()) &&
	        (shooterIntel = shooter->get_intel()) &&
	        ((!nearbyIntel->is_human() && 
            ((MachineP)nearbyIntel)->get_master_intel_id() == shooterIntel->get_intel_id()
           ) 
           ||
	         (!shooterIntel->is_human() && 
            ((MachineP)shooterIntel)->get_master_intel_id() == nearbyIntel->get_intel_id()
           )
          )
	       ) &&

        // Don't go after members of shooters team
        !(shooter && locator->same_team(shooter,nearby[n]))) {

	        const Area &area = nearby[n]->get_area();
	        int dist_2 = middle.distance_2(area.get_middle());
	        
	        if (distance_2 == -1 || dist_2 < distance_2) {
            targetId = nearby[n]->get_id();
            distance_2 = dist_2;
            hasTarget = True;
          }
      } // big if statement
    } // for n
  } // hasTarget  

  
  if (rotate.ready()) {
    rotate.set();
    
    PhysicalP target;
    if ((target = locator->lookup(targetId)) &&
        // Lose sight of invisible objects.
        !(target->is_moving() && ((MovingP)target)->is_invisible())) {
      Boolean noChange = False;
      Dir dir = get_dir();
      const Area &area = get_area();
      const Area &targetArea = target->get_area();
      Dir dirTo = area.dir_to(targetArea);
      if (dir > dirTo) {
        if (dir - dirTo > (CO_DIR_PURE / 2)) {
          dir++;
        }
        else {
          dir--;
        }
      }
      else if (dirTo > dir) {
        if (dirTo - dir > (CO_DIR_PURE / 2)) {
          dir--;
        }
        else {
          dir++;
        }
      }
      else {
        noChange = True;
      }
	
      if (!noChange) {
        if (dir >= CO_DIR_MAX) {
          dir -= CO_DIR_PURE;
        }
        if (dir < (CO_DIR_MAX - CO_DIR_PURE)) {
          dir += CO_DIR_PURE;
        }

        set_dir_next(dir);
        const Vel *unitVels = get_unit_vels();
        set_vel_next((float)context.speed * unitVels[dir]);
      }
    } // good target
  } // rotate.ready()
  
  if (timer.ready()) {
    kill_self();
  }
  
  timer.clock();
  rotate.clock();
  Shot::act();
}



void Missile::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.movingContext,
                 xdata);
}



Stats Missile::stats;



Star::Star(WorldP w,LocatorP l,const Pos &p,const Id &sh,Dir d)
: Shot(context,xdata,w,l,p,sh,d) {
	stats.add_creation();
}



CONSTRUCTOR_LEAF_IO(Star,Shot) {
}



DEFINE_CREATE_FROM_STREAM(Star)



void Star::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.movingContext,
                 xdata);
}



Stats Star::stats;



Blood::Blood(WorldP w,LocatorP l,const Pos &pos)
	 : Droplet(context,xdata,w,l,pos) {
}



CONSTRUCTOR_LEAF_IO(Blood,Droplet) {
}



DEFINE_CREATE_FROM_STREAM(Blood)



void Blood::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.movingContext,
                 xdata);
}



GreenBlood::GreenBlood(WorldP w,LocatorP l,const Pos &pos)
	 : Droplet(context,xdata,w,l,pos){
}



CONSTRUCTOR_LEAF_IO(GreenBlood,Droplet) {
}



DEFINE_CREATE_FROM_STREAM(GreenBlood)



void GreenBlood::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.movingContext,
                 xdata);
}



OilDroplet::OilDroplet(WorldP w,LocatorP l,const Pos &pos)
	 : Droplet(context,xdata,w,l,pos) {
}



CONSTRUCTOR_LEAF_IO(OilDroplet,Droplet) {
}



DEFINE_CREATE_FROM_STREAM(OilDroplet)



void OilDroplet::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.movingContext,
                 xdata);
}



Feather::Feather(WorldP w,LocatorP l,const Pos &pos)
: Droplet(context,xdata,w,l,pos) {
}



CONSTRUCTOR_LEAF_IO(Feather,Droplet) {
}



DEFINE_CREATE_FROM_STREAM(Feather)



void Feather::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.movingContext,
                 xdata);
}



Grenade::Grenade(WorldP w,LocatorP l,const Pos &pos,const Id &sh,
				 Dir dir,Speed speed)
				 : Falling(context,xdata,w,l,pos,dir) {
  assert(dir != CO_air);
  const Vel *unitVels = get_unit_vels();
  Vel initVel = (float)speed * unitVels[dir]; 
  set_vel(initVel);
  set_vel_next(initVel);
  
  timer.set(GRENADE_TIME);
  shooter = sh;
  defused = False;
}



Grenade::Grenade(WorldP w,LocatorP l,const Pos &pos,const Id &sh,
				 const Vel &vel)
  : Falling(context,xdata,w,l,pos,vel.get_dir())
{
  stats.add_creation();

  set_vel(vel);
  set_vel_next(vel);
  timer.set(GRENADE_TIME);
  shooter = sh;
  defused = False;
}



CONSTRUCTOR_LEAF_IO(Grenade,Falling) {
  defused = False;
}



DEFINE_CREATE_FROM_STREAM(Grenade)



void Grenade::set_quiet_death()
{
  defused = True;
  Falling::set_quiet_death();
}



void Grenade::act()
{
  if (timer.ready()) {
    kill_self();
  }
  
  timer.clock();
  Falling::act();
}



void Grenade::die()
{
	if (!defused)
	{
		WorldP world = get_world();
		LocatorP locator = get_locator();
		const Area area = get_area();
		
		PhysicalP explosion = 
			new Explosion(world,locator,area.get_middle(),shooter,
			GRENADE_EXPLOSION_RADIUS,GRENADE_EXPLOSION_DAMAGE_MAX);
		assert (explosion);
		locator->add(explosion);
	}
	Falling::die();
}



void Grenade::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.movingContext,
                 xdata);
}



Stats Grenade::stats;



Napalm::Napalm(WorldP w,LocatorP l,const Pos &pos,const Id &sh,
				 const Vel &vel)
: Falling(context,xdata,w,l,pos,vel.get_dir()) {
  stats.add_creation();

  set_vel(vel);
  set_vel_next(vel);
  timer.set(GRENADE_TIME);
  shooter = sh;
  defused = False;
}



CONSTRUCTOR_LEAF_IO(Napalm,Falling) {
  defused = False;
}



DEFINE_CREATE_FROM_STREAM(Napalm)



void Napalm::set_quiet_death()
{
  defused = True;
  Falling::set_quiet_death();
}



// Don't want NapalmGrenades to set each other off.
void Napalm::heat_attack(PhysicalP,int,Boolean) {
}



void Napalm::act()
{
  if (timer.ready()) {
    kill_self();
  }
  
  timer.clock();
  Falling::act();
}



void Napalm::die() {
  // Create a FireExplosion object to spit out fireballs.
	if (!defused)	{
		WorldP world = get_world();
		LocatorP locator = get_locator();
		const Area area = get_area();
		
    Pos pos = 
      Coord::shot_initial_pos(area,CO_dn,Fireball::get_size(),CO_air);
		PhysicalP p = 
      new FireExplosion(world,locator,pos,shooter);
		assert(p);
		locator->add(p);
	}
	Falling::die();
}



void Napalm::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.movingContext,
                 xdata);
}



Stats Napalm::stats;



Egg::Egg(WorldP w,LocatorP l,const Pos &pos,
		 const IntelOptions &ops,ITmask opMask)
		 : Falling(context,xdata,w,l,pos)
{
  intelOptions = ops;
  intelOpMask = opMask;
  open = False;
}



CONSTRUCTOR_LEAF_IO(Egg,Falling) {
  intelOpMask = ITnone;
  _update_from_stream(in);
}



DEFINE_UPDATE_FROM_STREAM(Egg,Falling)



void Egg::_update_from_stream(InStreamP in) {
  open = (Boolean)in->read_char();
}



int Egg::get_write_length() {
  return 
    Falling::get_write_length() + 
    sizeof(char);
}



void Egg::write(OutStreamP out) {
  Falling::write(out);

  out->write_char((char)open);
}



DEFINE_CREATE_FROM_STREAM(Egg)



void Egg::act() {
	if (!open) {
		LocatorP l = get_locator();
		PhysicalP nearby[OL_NEARBY_MAX];
		int nItems;
		l->get_nearby(nearby,nItems,this,EGG_RADIUS);
		// Check for nearby humans.
		for (int n = 0; n < nItems; n++)
		{
			IntelP intel = nearby[n]->get_intel();
			if (intel && 
				intel->is_human() &&
				!nearby[n]->get_ability(AB_Hugger) &&
				nearby[n]->get_class_id() != A_Alien)
				// Trigger the egg.
			{
				PhysicalP hugger;
				IntelP intel;
				WorldP w = get_world();
				
				// Hugger initial position is above the egg.
				const Area &area = get_area();
				Pos eggMiddle = area.get_middle();
				
				// Decide whether to create a red or green face hugger.
				Boolean gHugger = 
					Utils::choose(EGG_GREEN_HUGGER_PERCENT) == 0;
				
				Size huggerSize = gHugger ? GreenHugger::get_size_max() :
				RedHugger::get_size_max();
				Pos p(eggMiddle.x - huggerSize.width / 2,
					eggMiddle.y - EGG_HUGGER_EJECT_HEIGHT);
				
				// Create a Green or Red Hugger.
				if (gHugger) {
					// Create a GreenHugger, a neutral intelligence.
					hugger = new GreenHugger(w,l,p);
					intel = new Neutral(w,l,"Hugger",
						&intelOptions,intelOpMask);
					l->register_neutral((NeutralP)intel);
				}
				else {
					// Create a RedHugger, an enemy intelligence.
					hugger = new RedHugger(w,l,p);
					intel = new Enemy(w,l,"Hugger",
						&intelOptions,intelOpMask);
					l->register_enemy((EnemyP)intel);
				}
				assert(hugger);
				assert(intel);
				hugger->set_intel(intel);
				l->add(hugger);

				// Don't want Hugger to bump into the egg.
				hugger->set_dont_collide(this->get_id());    
				
				// Make egg open.
				open = True;
				set_dir_next(CO_center); // The pixmap for an open egg.
				break;
			}
		}
	}
	Falling::act();
}



void Egg::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.movingContext,
                 xdata);
}



Xit::Xit(WorldP w,LocatorP l,const Pos &pos) 
: Touchable(context,xdata,w,l,pos)
{}



CONSTRUCTOR_LEAF_IO(Xit,Touchable) {
}



DEFINE_CREATE_FROM_STREAM(Xit)



void Xit::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.fallingContext.movingContext,
                 xdata);
}



Flag::Flag(WorldP w,LocatorP l,const Pos &pos) 
: Touchable(context,xdata,w,l,pos)
{}



CONSTRUCTOR_LEAF_IO(Flag,Touchable) {
}



DEFINE_CREATE_FROM_STREAM(Flag)



void Flag::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.fallingContext.movingContext,
                 xdata);
}



Chainsaw::Chainsaw(WorldP w,LocatorP l,const Pos &p) 
: Cutter(context,xdata,w,l,p) 
{}



CONSTRUCTOR_LEAF_IO(Chainsaw,Cutter) {
}



DEFINE_CREATE_FROM_STREAM(Chainsaw)



void Chainsaw::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.weaponContext.itemContext.fallingContext.movingContext,
                 xdata);
}



//  This is here 
Pistol::Pistol(WorldP w,LocatorP l,const Pos &p) 
: Gun(context,xdata,w,l,p) 
{}



CONSTRUCTOR_LEAF_IO(Pistol,Gun) {
}



DEFINE_CREATE_FROM_STREAM(Pistol)



void Pistol::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.weaponContext.itemContext.fallingContext.movingContext,
                 xdata);
}



MGun::MGun(WorldP w,LocatorP l,const Pos &p) 
: Gun(context,xdata,w,l,p) 
{}



CONSTRUCTOR_LEAF_IO(MGun,Gun) {
}



DEFINE_CREATE_FROM_STREAM(MGun)



void MGun::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.weaponContext.itemContext.fallingContext.movingContext,
                 xdata);
}



Swapper::Swapper(WorldP w,LocatorP l,const Pos &p)
  : SingleGun(context,xdata,w,l,p) 
{}



CONSTRUCTOR_LEAF_IO(Swapper,SingleGun) {
}



DEFINE_CREATE_FROM_STREAM(Swapper)



Size Swapper::get_shot_size(Dir)
{
  return SwapShell::get_size();
}



PhysicalP Swapper::create_shot(PhysicalP shooter,WorldP w,LocatorP l,
							   const Pos &pos,Dir d)
{
  PhysicalP swapShell = new SwapShell(w,l,pos,shooter->get_id(),get_id(),d);
  assert (swapShell);
  return swapShell;
}



void Swapper::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.gunContext.weaponContext.itemContext.fallingContext.movingContext,
                 xdata);
}



Lancer::Lancer(WorldP w,LocatorP l,const Pos &p) 
: Gun(context,xdata,w,l,p) 
{}



CONSTRUCTOR_LEAF_IO(Lancer,Gun) {
}



DEFINE_CREATE_FROM_STREAM(Lancer)



Size Lancer::get_shot_size(Dir dir)
{
  return Lance::get_size(dir);
}



Dir Lancer::compute_weapon_dir(ITcommand command)
{
  return Lance::compute_weapon_dir(command);
}



PhysicalP Lancer::create_shot(PhysicalP shooter,WorldP w,LocatorP l,
							  const Pos &pos,Dir d)
{
  PhysicalP lance = new Lance(w,l,pos,shooter->get_id(),d);
  assert (lance);
  return lance;
}



void Lancer::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.weaponContext.itemContext.fallingContext.movingContext,
                 xdata);
}



AutoLancer::AutoLancer(WorldP w,LocatorP l,const Pos &p) 
: Gun(context,xdata,w,l,p) 
{}



CONSTRUCTOR_LEAF_IO(AutoLancer,Gun) {
}



DEFINE_CREATE_FROM_STREAM(AutoLancer)



Size AutoLancer::get_shot_size(Dir dir)
{
  return Lance::get_size(dir);
}



Dir AutoLancer::compute_weapon_dir(ITcommand command)
{
  return Lance::compute_weapon_dir(command);
}



PhysicalP AutoLancer::create_shot(PhysicalP shooter,WorldP w,LocatorP l,
							  const Pos &pos,Dir d)
{
  PhysicalP lance = new Lance(w,l,pos,shooter->get_id(),d);
  assert (lance);
  return lance;
}



void AutoLancer::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.weaponContext.itemContext.fallingContext.movingContext,
                 xdata);
}



FrogGun::FrogGun(WorldP w,LocatorP l,const Pos &p) :
SingleGun(context,xdata,w,l,p) 
{}



CONSTRUCTOR_LEAF_IO(FrogGun,SingleGun) {
}



DEFINE_CREATE_FROM_STREAM(FrogGun)



Size FrogGun::get_shot_size(Dir)
{
	return FrogShell::get_size();
}



PhysicalP FrogGun::create_shot(PhysicalP shooter,WorldP w,LocatorP l,
							   const Pos &pos,Dir d)
{
	PhysicalP shot = new FrogShell(w,l,pos,shooter->get_id(),get_id(),d);
	assert (shot);
	return shot;
}



void FrogGun::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.gunContext.weaponContext.itemContext.fallingContext.movingContext,
                 xdata);
}



DogWhistle::DogWhistle(WorldP w,LocatorP l,const Pos &p) 
: Whistle(context,xdata,w,l,p) 
{}



CONSTRUCTOR_LEAF_IO(DogWhistle,Whistle) {
}



DEFINE_CREATE_FROM_STREAM(DogWhistle)



PetP DogWhistle::create_pet(IntelP master,Boolean alternatePet,int n) {
  // The default pet is a Dog, alternate pet is a RedHugger, ha, ha.

  const Area &area = get_area();
  Pos pos = area.get_middle() - 
    0.5f * (alternatePet ? RedHugger::get_size_max() : Dog::get_size_max());

  // So multiple dogs don't pop up on top of each other.
  Size offset;
  offset.set(Utils::choose(21) - 3,Utils::choose(21) - 3);
  pos = pos + offset;

  LocatorP l = get_locator();

  PhysicalP p;
  if (alternatePet) {
    p = new RedHugger(get_world(),get_locator(),pos);
  }
  else {
    if (n % 2 == 0) {
      p = new Dog(get_world(),get_locator(),pos);
    }
    else {
      p = new Mutt(get_world(),get_locator(),pos);
    }
  }
  assert(p);
  IntelOptions ops;
  ops.psychotic = True;
  ops.classFriends = False; // So Dogs attack enemy dogs. (unnecessary?)
  // Don't give a limited lifespan.
  PetP pet = new Pet(get_world(),get_locator(),"Spot",&ops,
                     ITpsychotic | ITclassFriends,master);
  assert(pet);
  pet->set_dont_collide(master->get_intel_id()); // unnecessary.
  p->set_intel(pet);
  l->add(p);
  l->register_neutral(pet);

  return pet;
}



void DogWhistle::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.weaponContext.itemContext.fallingContext.movingContext,
                 xdata);
}



DemonSummoner::DemonSummoner(WorldP w,LocatorP l,const Pos &p) 
: Whistle(context,xdata,w,l,p) {
}



CONSTRUCTOR_LEAF_IO(DemonSummoner,Whistle) {
}



DEFINE_CREATE_FROM_STREAM(DemonSummoner)



PetP DemonSummoner::create_pet(IntelP master,Boolean,int) {
  // Doesn't use alternatePet

  const Area &area = get_area();
  Pos pos = area.get_middle() - 0.5f * FireDemon::get_size_max();

  // Don't bother offseting randomly for multiple Demons.

  LocatorP l = get_locator();

  PhysicalP p = new FireDemon(get_world(),get_locator(),pos);
  assert(p);

  IntelOptions ops;
  ops.psychotic = True;
  ops.classFriends = False; // If summon multiple demons.
  ops.limitedLifespan = DEMON_SUMMONER_SUICIDE_TIME;
  PetP pet = new Pet(get_world(),get_locator(),"WormChewer",&ops,
                     ITpsychotic | ITclassFriends | ITlimitedLifespan,master);
  assert(pet);
  pet->set_dont_collide(master->get_intel_id());
  p->set_intel(pet);
  l->add(p);
  l->register_neutral(pet);

  return pet;
}



void DemonSummoner::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.weaponContext.itemContext.fallingContext.movingContext,
                 xdata);
}



FThrower::FThrower(WorldP w,LocatorP l,const Pos &p) :
Gun(context,xdata,w,l,p) {
  Timer nTimer(F_THROWER_FIRE_TIME);
  stopFiring = nTimer;
  isFiring = False;
}



CONSTRUCTOR_LEAF_IO(FThrower,Gun) {
}



DEFINE_CREATE_FROM_STREAM(FThrower)



PhysicalP FThrower::create_shot(PhysicalP shooter,WorldP w,LocatorP l,
								const Pos &pos,Dir d)
{
	PhysicalP fireball = new Fireball(w,l,pos,shooter->get_id(),d);
	assert (fireball);
	return fireball;
}



void FThrower::fire(const Id &id,ITcommand command)
{
	killerId = id;
	fireCommand = command;
	stopFiring.set();
	isFiring = True;
}



void FThrower::act()
{
	if (isFiring && stopFiring.ready())
		isFiring = False;

	if (isFiring)
		Gun::fire(killerId,fireCommand);
	
	stopFiring.clock();
	Gun::act();
}



Size FThrower::get_shot_size(Dir)
{
	return Fireball::get_size();
}



void FThrower::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.weaponContext.itemContext.fallingContext.movingContext,
                 xdata);
}



Launcher::Launcher(WorldP w,LocatorP l,const Pos &p) 
: Gun(context,xdata,w,l,p) 
{}



CONSTRUCTOR_LEAF_IO(Launcher,Gun) {
}



DEFINE_CREATE_FROM_STREAM(Launcher)



Size Launcher::get_shot_size(Dir dir)
{
  return Missile::get_size(dir);
}



PhysicalP Launcher::create_shot(PhysicalP shooter,WorldP world,
								LocatorP locator,const Pos &pos,Dir dir)
{
	PhysicalP shot = new Missile(world,locator,pos,shooter->get_id(),dir);
	assert (shot);
	return shot;
}



void Launcher::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.weaponContext.itemContext.fallingContext.movingContext,
                 xdata);
}



Grenades::Grenades(WorldP w,LocatorP l,const Pos &p) 
: Gun(context,xdata,w,l,p) 
{}



CONSTRUCTOR_LEAF_IO(Grenades,Gun) {
}



DEFINE_CREATE_FROM_STREAM(Grenades)



Size Grenades::get_shot_size(Dir dir)
{
	return Grenade::get_size(dir);
}



PhysicalP Grenades::create_shot(PhysicalP shooter,WorldP world,
								LocatorP locator,const Pos &pos,Dir dir)
{
	Speed speed;
	if (dir == CO_UP_L || dir == CO_UP || dir == CO_UP_R) {
		speed = GRENADES_TOSS_SPEED;
  }
	else if (dir == CO_R || dir == CO_DN_R || dir == CO_DN_L || dir == CO_L) {
		speed = GRENADES_ROLL_SPEED;
  }
	else if (dir == CO_DN) {
		speed = 0;
  }
	else {
		assert(0);
  }

	assert(shooter->is_moving());
	const Vel *unitVels = get_unit_vels();
	Vel vel = ((MovingP)shooter)->get_vel() + (float)speed * unitVels[dir];

	//  PhysicalP shot = new Grenade(world,locator,pos,shooter->get_id(),dir,speed);
	PhysicalP shot = new Grenade(world,locator,pos,shooter->get_id(),vel);
	assert (shot);
	return shot;
}



void Grenades::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.weaponContext.itemContext.fallingContext.movingContext,
                 xdata);
}



Napalms::Napalms(WorldP w,LocatorP l,const Pos &p) 
: Gun(context,xdata,w,l,p) 
{}



CONSTRUCTOR_LEAF_IO(Napalms,Gun) {
}



DEFINE_CREATE_FROM_STREAM(Napalms)



Size Napalms::get_shot_size(Dir dir)
{
	return Napalm::get_size(dir);
}



// Copied from Grenades, should have common parent.
PhysicalP Napalms::create_shot(PhysicalP shooter,WorldP world,
								LocatorP locator,const Pos &pos,Dir dir)
{
  // Using same parameters as grenades.
	Speed speed;
	if (dir == CO_UP_L || dir == CO_UP || dir == CO_UP_R)
		speed = GRENADES_TOSS_SPEED;
	else if (dir == CO_R || dir == CO_DN_R || dir == CO_DN_L || dir == CO_L)
		speed = GRENADES_ROLL_SPEED;
	else if (dir == CO_DN)
		speed = 0;
	else
		assert(0);

	assert(shooter->is_moving());
	const Vel *unitVels = get_unit_vels();
	Vel vel = ((MovingP)shooter)->get_vel() + (float)speed * unitVels[dir];

	//  PhysicalP shot = new Grenade(world,locator,pos,shooter->get_id(),dir,speed);
	PhysicalP shot = new Napalm(world,locator,pos,shooter->get_id(),vel);
	assert (shot);
	return shot;
}



void Napalms::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.weaponContext.itemContext.fallingContext.movingContext,
                 xdata);
}



Stars::Stars(WorldP w,LocatorP l,const Pos &p) 
: Gun(context,xdata,w,l,p) 
{}



CONSTRUCTOR_LEAF_IO(Stars,Gun) {
}



DEFINE_CREATE_FROM_STREAM(Stars)



Size Stars::get_shot_size(Dir)
{
	return Star::get_size();
}



PhysicalP Stars::create_shot(PhysicalP shooter,WorldP world,LocatorP locator,
							 const Pos &pos,Dir dir)
{
	PhysicalP shot = new Star(world,locator,pos,shooter->get_id(),dir);
	assert (shot);
	return shot;
}



void Stars::fire(const Id &shooterId,ITcommand command)
{
	LocatorP locator = get_locator();
	PhysicalP shooter = locator->lookup(shooterId);
	if (shooter)
	{
		// Ninjas firing stars give a wider spread.
		int delta = (shooter->get_class_id() == A_Ninja) ? 2 : 1;

		// Fire a spread of shots around center.
		Dir center = Intel::command_weapon_to_dir_8(command);
		if (center != CO_air)
			for (Dir baseDir = center - delta; baseDir <= center + delta; baseDir++)
		{
			Dir dir;
			if (baseDir >= CO_DIR_MAX)
				dir = baseDir - CO_DIR_PURE;
			else if (baseDir < CO_DIR_MAX - CO_DIR_PURE)
				dir = baseDir + CO_DIR_PURE;
			else
				dir = baseDir;

			// Only set the timer on the last shot.
			Boolean lastOne = (baseDir == center + delta);
			Gun::_fire(shooterId,dir,lastOne,lastOne);
		}
	}
}



void Stars::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,context.weaponContext.itemContext.fallingContext.movingContext,
                 xdata);
}



DEFINE_CREATURE_CTORS_3(Enforcer,Grounded,grounded,Suicide,suicide,Prickly,prickly)



DEFINE_CREATE_FROM_STREAM(Enforcer)



void Enforcer::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,creatureContext.movingContext,creatureXdata);
}



Stats Enforcer::stats;



DEFINE_CREATURE_CTORS_2(Frog,Suicide,suicide,Hopping,hopping)



DEFINE_CREATE_FROM_STREAM(Frog)



void Frog::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,creatureContext.movingContext,creatureXdata);
}



Stats Frog::stats;



// Temporarily disable Hero's lifting ability until we have time to finish it
// properly.
//DEFINE_CREATURE_CTORS_5(Hero,User,user,Fighter,fighter,Walking,walking,Healing,healing,Lifter,lifter)
DEFINE_CREATURE_CTORS_4(Hero,User,user,Fighter,fighter,Walking,walking,Healing,healing)



DEFINE_CREATE_FROM_STREAM(Hero)



void Hero::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,creatureContext.movingContext,creatureXdata);
}



Stats Hero::stats;



DEFINE_CREATURE_CTORS_3(Zombie,Fighter,fighter,Walking,walking,AnimTime,animTime)



DEFINE_CREATE_FROM_STREAM(Zombie)



void Zombie::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,creatureContext.movingContext,creatureXdata);
}



Stats Zombie::stats;



DEFINE_CREATURE_CTORS_3(Ninja,User,user,Fighter,fighter,Sticky,sticky)



DEFINE_CREATE_FROM_STREAM(Ninja)



void Ninja::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,creatureContext.movingContext,creatureXdata);
}



Stats Ninja::stats;



DEFINE_CREATURE_CTORS_3(Alien,Sticky,sticky,Prickly,prickly,Healing,healing)



DEFINE_CREATE_FROM_STREAM(Alien)



void Alien::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,creatureContext.movingContext,creatureXdata);
}



Stats Alien::stats;



DEFINE_CREATURE_CTORS_2(RedHugger,Sticky,sticky,Hugger,hugger)



DEFINE_CREATE_FROM_STREAM(RedHugger)



void RedHugger::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,creatureContext.movingContext,creatureXdata);
}



DEFINE_CREATURE_CTORS_2(GreenHugger,Sticky,sticky,Hugger,hugger)



DEFINE_CREATE_FROM_STREAM(GreenHugger)



void GreenHugger::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,creatureContext.movingContext,creatureXdata);
}



DEFINE_CREATURE_CTORS_3(ChopperBoy,User,user,Flying,flying,Lifter,lifter)



DEFINE_CREATE_FROM_STREAM(ChopperBoy)



void ChopperBoy::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,creatureContext.movingContext,creatureXdata);
}



Stats ChopperBoy::stats;



DEFINE_CREATURE_CTORS_3(Seal,Grounded,grounded,Suicide,suicide,AnimTime,animTime)



DEFINE_CREATE_FROM_STREAM(Seal)



void Seal::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,creatureContext.movingContext,creatureXdata);
}



Stats Seal::stats;



DEFINE_CREATURE_CTORS_4(FireDemon,Flying,flying,BuiltIn,builtIn,OnFire,onFire,SwapProtect,swapProtect)



DEFINE_CREATE_FROM_STREAM(FireDemon)



Size FireDemon::get_shot_size(Dir) {
	return Fireball::get_size();
}



PhysicalP FireDemon::create_shot(WorldP w,LocatorP l,const Pos &pos,
                                 const Id &shooter,Dir dir) {
	PhysicalP shot = 
		new Fireball(w,l,pos,shooter,dir,
		             FIRE_DEMON_FIREBALL_HEAT,FIRE_DEMON_FIREBALL_TIME,
                 True);  // Special fireballs that hurt those with the OnFire ability.
	assert(shot);
	return shot;
}



void FireDemon::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,creatureContext.movingContext,creatureXdata);
}



DEFINE_CREATURE_CTORS_4(Dragon,Flying,flying,BuiltIn,builtIn,OnFire,onFire,Segmented,segmented)



DEFINE_CREATE_FROM_STREAM(Dragon)



Size Dragon::get_shot_size(Dir) {
	return Fireball::get_size();
}



PhysicalP Dragon::create_shot(WorldP w,LocatorP l,const Pos &pos,
                              const Id &shooter,Dir dir) {
	PhysicalP shot = 
		new Fireball(w,l,pos,shooter,dir,
		             DRAGON_FIREBALL_HEAT,DRAGON_FIREBALL_TIME,
                 True);  // Special fireballs that hurt those with the OnFire ability.
	assert(shot);
	return shot;
}



void Dragon::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,creatureContext.movingContext,creatureXdata);
}



DEFINE_CREATURE_CTORS_4(Walker,User,user,Walking,walking,BuiltIn,builtIn,AnimTime,animTime)



DEFINE_CREATE_FROM_STREAM(Walker)



PhysicalP Walker::create_shot(WorldP w,LocatorP l,const Pos &pos,
                              const Id &shooter,Dir dir) {
	PhysicalP shot = new Laser(w,l,pos,shooter,dir);
	assert(shot);
	return shot;
}



void Walker::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,creatureContext.movingContext,creatureXdata);
}



Stats Walker::stats;



DEFINE_CREATURE_CTORS_3(Dog,Carrier,carrier,Fighter,fighter,Walking,walking)



DEFINE_CREATE_FROM_STREAM(Dog)



Boolean Dog::draw_outline(ColorNum &) {
  // Dogs look stupid with an outline, the Dog is too small.
  return False;
}



void Dog::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,creatureContext.movingContext,creatureXdata);
}



Stats Dog::stats;



DEFINE_CREATURE_CTORS_3(Mutt,Carrier,carrier,Fighter,fighter,Walking,walking)



DEFINE_CREATE_FROM_STREAM(Mutt)



Boolean Mutt::draw_outline(ColorNum &) {
  // Dogs look stupid with an outline, the Dog is too small.
  return False;
}



void Mutt::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,creatureContext.movingContext,creatureXdata);
}



Stats Mutt::stats;



DEFINE_CREATURE_CTORS_4(Yeti,Prickly,prickly,Walking,walking,
                        AnimTime,animTime,Sensitive,sensitive);



DEFINE_CREATE_FROM_STREAM(Yeti)



void Yeti::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,creatureContext.movingContext,creatureXdata);
}



Stats Yeti::stats;



DEFINE_CREATURE_CTORS_2(Chicken,Flying,flying,Fighter,fighter)



DEFINE_CREATE_FROM_STREAM(Chicken)



void Chicken::init_x(Xvars &xvars,IXCommand command,void* arg) {
  Moving::init_x(xvars,command,arg,creatureContext.movingContext,creatureXdata);
}



Stats Chicken::stats;
