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

// "physical.cpp"

#include "stdafx.h"

#if X11
#ifndef NO_PRAGMAS
#pragma implementation "physical.h"
#endif
#endif


// Include Files
#include "xdata.h"
#include "physical.h"
#include <iostream>

#if X11
#include <strstream>
#endif
#if WIN32
#include <strstrea.h>
#endif

#include "utils.h"
#include "coord.h"
#include "area.h"
#include "world.h"
#include "id.h"
#include "intel.h"
#include "locator.h"
#include "physical.h"
#include "actual.h"

#include "bitmaps/transform/transform.bitmaps"


// Defines
#define VEL_SMALL 1
#define MOVING_ANIM_TIME 2
#define MOVING_EXTRA_VEL_MAX 9
#define FALLING_VEL_MAX 12
#define HEAVY_DAMAGE_VEL 3
#define ITEM_CAN_TAKE_TIME 15
#define ITEM_DROP_OFFSET_WIDTH 20
#define ITEM_DROP_OFFSET_HEIGHT 20
#define ITEM_CLASS_MAX 2  // How items of a given class can be held.
#define LIFTABLE_EXCLUDED_TIME ITEM_CAN_TAKE_TIME
#define EXTRA_VEL_DAMP 1
#define DROP_SPEED 2

#define CREATURE_ACCELERATION_MAX 40
#define CREATURE_AIR_SPEED_MAX 40
#define CREATURE_CENTER_SPEED_MAX 40
#define CREATURE_CLIMB_SPEED_MAX 40
#define CREATURE_CRAWL_SPEED_MAX 40
#define CREATURE_JUMP_MAX 40
#define CREATURE_HIGH_VIBRATE 3
#define CREATURE_HEALTH_MAX_MAX 2000

#define CREATURE_DROPLETS_MAX 10  // Max droplets of blood per hit.
// Greater than 25 damage to get second droplet
#define CREATURE_DROPLET_FACTOR .04 

#define FIGHTER_DAMAGE_MAX 150
#define FIGHTER_SLIDE_TIME 4
#define FIGHTER_FREE_TIME 8

#define SHOT_OFFSET 19
#define HEAT_MAX 25
#define FLYING_GRAV_VEL_CUTOFF 1
#define CORPSE_TIME 140
#define FLASH_TIME 5
#define TOUCHABLE_RADIUS 20 //11 //10
#define HUGGER_TIME 90
#define USER_CYCLE_TIME 50
#define DROPLET_SPLATTER_SPEED 8


#if WIN32
#define M_PI 3.14159265
#endif


//// Functions
Physical::Physical(const PhysicalContext &p_c,WorldP w,LocatorP l) {
  world = w;
  locator = l;
  command = IT_NO_COMMAND;
  pc = &p_c;
  health = healthNext = pc->health;
  mass = massNext = pc->mass;
  // dontCollide initializes itself.
  intel = NULL;
  if (! staticValid) {
    init_static();
  }
  deleteMe = False;
  mapped = mappedNext = True;
  noDeathDelete = False;
  dieCalled = False;
  heat = heatNext = 0;
  previousHeatWasSecondary = False;
  dontCollideComposite = COMPOSITE_ID_NONE;

  // Add 1 to FLASH_TIME to draw for proper number of turns.
  Timer t(FLASH_TIME + 1);
  flashTimer = t;
  quietDeath = False;
  netData = NULL;

  creationOk = True;
}



Physical::Physical() {
  assert(0);
}



Physical::Physical(InStreamP in,const PhysicalContext &p_c,
                   WorldP w,LocatorP l) {
  creationOk = True;
  world = w;
  locator = l;
  command = IT_NO_COMMAND;
  pc = &p_c;
  mass = massNext = pc->mass;
  // dontCollide initializes itself.
  intel = NULL;
  if (! staticValid) {
    init_static();
  }
  deleteMe = False;
  noDeathDelete = False;
  previousHeatWasSecondary = False;
  dontCollideComposite = COMPOSITE_ID_NONE;
  quietDeath = False;
  netData = NULL;

  _update_from_stream(in,False);
}



void Physical::update_from_stream(InStreamP in) {
  // Things should be ok to start with, child classes haven't had a chance
  // to call creation_failed() yet.
  assert(creationOk);  

  _update_from_stream(in,True);
}



void Physical::_update_from_stream(InStreamP in,Boolean alreadyExists) {
  if (alreadyExists) {
    // If updating from stream, not creating from stream, 
    // the id read in should be the same as the existing one.
    Id streamId(in);
    if (!(id == streamId)) {
      creationOk = False;
    }
  }
  else {
    // create_from_stream(), old value of id should not be set yet.
    id.read(in);
  }

  u_short h2 = in->read_short();
  signed short h1 = (signed short)h2;
  health = (Health)h1;
  healthNext = health;
  

  heat = in->read_short();
  heatNext = heat;

  // mapped, dieCalled, and flash
  u_char flags = in->read_char();
  mapped = (flags & 0x1);
  mappedNext = mapped;

  // Not really needed, just to check an assertion.
  dieCalled = (flags & 0x2);

  Boolean flash = (flags & 0x4);
  if (flash) {
    // Will never be clocked anyway.
    flashTimer.set(1);
  }
  else {
    flashTimer.set(0);
  }

  // May stil be invalidated by child classes.
  creationOk = creationOk && in->alive();

  LocatorP locator = get_locator();
  IntelId iId;
  iId.read(in);
  intel = locator->lookup(iId);  // may be NULL.  
}



int Physical::get_write_length() {
  return 
    id.get_write_length() +            // id
    sizeof(short) +                    // health
    //    intel->get_write_length() +  // intel
    sizeof(short) +                    // heat
    sizeof(char)+                      // mapped, dieCalled, flash
    Identifier::get_write_length();    // intelId
}



void Physical::write(OutStreamP out) {
  id.write(out);

  signed short h1 = (signed short)health;
  u_short h2 = (u_short)h1;
  out->write_short(h2);

  // intel
  assert(heat < USHRT_MAX);
  out->write_short((u_short)heat);

  u_char flags = 0x0;
  flags |= mapped;
  flags |= (dieCalled << 1);
  flags |= (!flashTimer.ready() << 2);
  out->write_char(flags);

  IntelId iId;
  if (intel) {
    iId = intel->get_intel_id();
  }
  iId.write(out);
}



Physical::~Physical() {
  delete netData;
}



Health Physical::get_health_max() {
  return pc->health;
}



Dir Physical::get_dir() {
  return CO_air;
}



int Physical::get_drawing_level() {
  return 1;
}



Vel Physical::get_vel() // Returns (0,0)
{
  Vel vel; 
  return vel;
} 



Boolean Physical::is_moving() 
{
  return False;
}



Boolean Physical::is_shot() 
{
  return False;
}



Boolean Physical::is_item() 
{
  return False;
}



Boolean Physical::is_shield()
{
  return False;
}



Boolean Physical::is_bomb()
{
  return False;
}



Boolean Physical::is_weapon() 
{
  return False;
}



Boolean Physical::is_cutter() 
{
  return False;
}



Boolean Physical::is_gun() 
{
  return False;
}



Boolean Physical::is_creature() 
{
  return False;
}



AbilityP Physical::get_ability(AbilityId) {
  return NULL;
}



HolderP Physical::get_holder() {
  return NULL;
}



LocomotionP Physical::get_locomotion() {
  return NULL;
}


CompositeP Physical::get_composite() {
  return NULL;
}



Boolean Physical::is_auto_use()
{
  return False;
}



Boolean Physical::is_drugs()
{
  return False;
}



Boolean Physical::is_liftable()
{
  return False;
}



Boolean Physical::collidable() 
{
  return True;
}



PHsig Physical::get_id(Id &outside_id)
{
  // used to have check for idValid

  outside_id = id;
  return PH_NO_SIG;
}



void Physical::get_followers(PtrList &) {
  // Default is no followers.
}



void Physical::follow(const Area &,Dir,Boolean) {
  assert(0);
}



void Physical::set_id(const Id &new_id)
{
  id = new_id;

  // used to have check for idValid
}



NetDataP Physical::get_net_data() {
  if (!netData) {
    netData = new NetData();
  }
  return netData;
}



Boolean Physical::command_repeatable(ITcommand c) {
  switch (c) {
    case IT_WEAPON_CHANGE:
    case IT_WEAPON_DROP:
    case IT_ITEM_USE:
    case IT_ITEM_CHANGE:
    case IT_ITEM_DROP:
      return False;
    // Allow repeat for all movement keys and for WEAPON_USE
    default:
      return True;
  }
}



void Physical::heal() {
  set_health_next(get_health_max());
}



void Physical::set_intel(IntelP i) {
  intel = i;  
  if (i) {
    i->set_id(id);
  }
}



void Physical::set_mapped_next(Boolean val) {
  mappedNext = val;
}



void Physical::flash()
{
  flashTimer.set();
}



Boolean Physical::corporeal_attack(PhysicalP,int damage,AttackFlags flags) {
  // Got rid of hack of using negative numbers for superficial damage.
  assert(damage >= 0);
      
  if (
      // Ignore "superficial" damage.
      (flags & ATT_DAMAGE) && 
      // alive_next() checks if something else has already killed this.
      alive() && alive_next()) {
    healthNext = health - damage;
  }

  // For now dead objects return True.  I.e. corpses will bleed.
  return True;
}



void Physical::heat_attack(PhysicalP,int h,Boolean secondary) {
  // Secondary means the attack came from colliding with something that was
  // on fire, as opposed to directly being hit by fire, e.g. a flame thrower.
  
  assert(h >= 0);
  //  Don't propagate fires.
  if (! (secondary && previousHeatWasSecondary))  {
    heatNext = heat + h;
      
    if (heatNext > HEAT_MAX) {
      heatNext = HEAT_MAX;
    }
      
    if (heat == 0 && heatNext > 0) {
      LocatorP l = get_locator();
      ostrstream str;
      str << get_class_name() << " is on fire." << ends;
      locator->message_enq(str.str());
    }
  }
  previousHeatWasSecondary = secondary;
}



Boolean Physical::swap_protect() {
  return False;
}



Boolean Physical::frog_protect() {
  return False;
}



void Physical::avoid(PhysicalP) {
}



void Physical::collide(PhysicalP) {
}



void Physical::set_quiet_death() {
  quietDeath = True;
}



void Physical::act() {  
  const Area &area = get_area();
  
  /* The knocked out of world check is now done inside the locator. */
  
  if (heatNext > 0) {
    Pos pos;
    Size size;
    area.get_rect(pos,size);
    pos.x += Utils::choose(size.width);
    // Start fire in upper-half.
    if (size.height > 1) {
      pos.y += Utils::choose((int)(size.height * .5)); 
    }

    LocatorP locator = get_locator();
    PhysicalP fire = new Fire(get_world(),locator,pos);
    fire->set_dont_collide(this->get_id());
    locator->add(fire);
    heatNext--;
    
    // alive_next() checks if something else has already killed this.
    if (alive() && alive_next()) {
      healthNext -= Fire::get_damage();
    }
  }

  // Call non-virtual version, to handle following.
  // Parent update_area_next()'s will already have been called as 
  // necessary.
  _update_area_next(False);
}



void Physical::update() {
  mass = massNext;

  command = IT_NO_COMMAND;

  // Fire damage is now in Physical::act

  health = healthNext;
  mapped = mappedNext;
  heat = heatNext;
  flashTimer.clock();
}



void Physical::die() {
  if (dieCalled) {
    cerr << "Physical::die called twice" << endl;
  }
  if (healthNext >= 0) {
    cerr << "Physical::die, healthNext >= 0, value is " << healthNext << endl;
    healthNext = -1;
  }
  if (health < 0) {
    cerr << "Physical::die, health already less than 0, value is " << health << endl;
  }
  dieCalled = True;

  if (intel) {
    intel->die();
  }
  intel = NULL;

  if (!noDeathDelete) {
    set_delete_me();
  }
}



void Physical::dr_clock() {
  // Make followers follow, setting current and next variables.
  _update_area_next(True);
}



ClassId Physical::get_weapon_string(const char *&str) {
//  static const char noneStr[] = "none";
  str = "none";
  return A_None;
}



void Physical::drop_all(Boolean) {
}



Dir Physical::get_dir_next() {
  return CO_air;
}



ITcommand Physical::get_command() {
  return command;
}



void Physical::init_static() {
  // CO_air_* is not really a unit.

  unitAccs[CO_center_R].ddx = 1.0f;
  unitAccs[CO_center_L].ddx = -1.0f;
  unitAccs[CO_air_R].ddx = .4f;
  unitAccs[CO_air_L].ddx = -.4f;
  unitAccs[CO_air_R].ddy = unitAccs[CO_air_L].ddy = -1.0f;  // -0.95
  unitAccs[CO_air_UP].ddy = -1.0f;
  unitAccs[CO_air_DN].ddy = 1.0f;

  unitAccs[CO_center_R].ddy = unitAccs[CO_center_L].ddy = 
      unitAccs[CO_air].ddy = unitAccs[CO_air].ddx = 
	unitAccs[CO_center].ddx = 
      unitAccs[CO_center].ddy = unitAccs[CO_air_UP].ddx = 
	unitAccs[CO_air_DN].ddx = 
	  0.0f;

  unitAccs[CO_r].ddx = unitAccs[CO_r].ddy =
      unitAccs[CO_dn].ddx = unitAccs[CO_dn].ddy =
	unitAccs[CO_l].ddx = unitAccs[CO_l].ddy =
	  unitAccs[CO_up].ddx = unitAccs[CO_up].ddy =
	    0.0f;
  
  unitAccs[CO_r_DN].ddx = unitAccs[CO_r_UP].ddx =
    unitAccs[CO_dn_R].ddy = unitAccs[CO_dn_L].ddy =
      unitAccs[CO_l_DN].ddx = unitAccs[CO_l_UP].ddx =
	unitAccs[CO_up_R].ddy = unitAccs[CO_up_L].ddy =
	  unitAccs[CO_climb_DN].ddx = unitAccs[CO_climb_UP].ddx =
	    0.0f;
  
  unitAccs[CO_r_DN].ddy = unitAccs[CO_l_DN].ddy = unitAccs[CO_climb_DN].ddy = 
    0.5f;
  
  unitAccs[CO_r_UP].ddy = unitAccs[CO_l_UP].ddy = unitAccs[CO_climb_UP].ddy =
    -0.5f;
  
  unitAccs[CO_dn_R].ddx = unitAccs[CO_up_R].ddx = 0.5f;
  
  unitAccs[CO_dn_L].ddx = unitAccs[CO_up_L].ddx = -0.5f;
  
  int n;
  for (n = 0; n < 16; n++) {
    float theta = (float)M_PI * n / 8.0f;
    unitAccs[CO_R + n].ddx = (float)cos(theta);
    unitAccs[CO_R + n].ddy = (float)sin(theta);
  }

  for (n = 0; n < CO_DIR_MAX; n++) {
    unitVels[n] = unitAccs[n];
  }

  staticValid = True;
}



void Physical::update_area_next(Boolean currentAndNext) {
  _update_area_next(currentAndNext);
}



void Physical::_update_area_next(Boolean currentAndNext) {
  // Whether currentAndNext is true or not, get_area_next() and 
  // get_dir_next() should give the correct results here, because 
  // Physical::_update_area_next() is called last.
  const Area &areaNext = get_area_next();
  if (currentAndNext) {
    // sanity check
    const Area &area = get_area();
    if (!(area == areaNext)) {
      cerr << "Physical::_update_area_next() area does not equal "
           << "areaNext when expected." 
           << endl;
    }
  }
  Dir dirNext = get_dir_next();
  
  // Make all followers follow.
  PtrList followers;
  get_followers(followers);
  for (int n = 0; n < followers.length(); n++) {
    PhysicalP p = (PhysicalP)followers.get(n);
    p->follow(areaNext,dirNext,currentAndNext);
  }
}



Boolean Physical::staticValid = False;



Acc Physical::unitAccs[CO_DIR_MAX];



Vel Physical::unitVels[CO_DIR_MAX];



Protection::Protection(const ProtectionContext &pr_c,ProtectionXdata &x_data,
		       WorldP w,LocatorP l,const Area &ar)
  : Physical(pr_c.physicalContext,w,l)
{
  area = areaNext = areaBaseNext = ar;
  delta = 0;
  pXdata = &x_data;
  prc = &pr_c;
}



Protection::Protection(InStreamP in,
                       const ProtectionContext &pr_c,
                       ProtectionXdata &x_data,
                       WorldP w,	       
                       LocatorP l) 
  : Physical(in,pr_c.physicalContext,w,l)
{
  pXdata = &x_data;
  prc = &pr_c;
  // Leave areaBaseNext as the default.
  _update_from_stream(in);
}



void Protection::update_from_stream(InStreamP in) {
  Physical::update_from_stream(in);
  _update_from_stream(in);  
}



void Protection::_update_from_stream(InStreamP in) {
  delta = in->read_signed_char();
  area.read(in);
  areaNext = area;
}



int Protection::get_write_length() {
  return 
    Physical::get_write_length() + 
    sizeof(char) +             // delta
    Area::get_write_length();  // area
}



void Protection::write(OutStreamP out) {
  Physical::write(out);

  if (!(area == areaNext)) {
    cerr << "Protection::write(), area != areaNext." << endl;
  }
  out->write_signed_char(delta);
  area.write(out);
}



Boolean Protection::collidable()
{
  return False;
}



const Area &Protection::get_area()
{
  return area;
}



const Area &Protection::get_area_next()
{
  return areaNext;
}



void Protection::follow(const Area &followeeArea,Dir,
                        Boolean currentAndNext) {
  areaBaseNext = followeeArea;

  Pos basePos;
  Size baseSize;
  areaBaseNext.get_rect(basePos,baseSize);
  Pos nPos(basePos.x - delta,basePos.y - delta);
  Size nSize;
  nSize.width = baseSize.width + 2 * delta;
  nSize.height = baseSize.height + 2 * delta;

  if (nSize.width < 0) {
    nSize.width = 0;
  }
  if (nSize.height < 0) {
    nSize.height = 0;
  }
  // Don't get bigger than a grid cell.
  if (nSize.width > OL_GRID_WIDTH) {
    nSize.width = OL_GRID_WIDTH;
  }
  if (nSize.height > OL_GRID_HEIGHT) {
    nSize.height = OL_GRID_HEIGHT;
  }

  Area nArea(AR_RECT,nPos,nSize);
  areaNext = nArea;
  
  update_area_next(currentAndNext);
}



void Protection::act() {
  _act();
  Physical::act();
}



void Protection::update() {
  area = areaNext;
  
  Physical::update();
}



void Protection::dr_clock() {
  // We are not really doing all the right things here.
  // should do animation purely on the Client side like Moving does.
  // areaNext should be computed in _act(), not in follow().
  // We don't know if dr_clock() will be called before, or after
  // follow().  Of course, we already have the same problem with StandAlone.

  _act();
  Physical::dr_clock();
}



void Protection::update_area_next(Boolean currentAndNext) {
  // Also set current state.
  if (currentAndNext) {
    area = areaNext;
  }
  Physical::update_area_next(currentAndNext);
}



void Protection::_act() {
  delta++;
  if (delta == DELTA_MAX) {
    delta = DELTA_MIN;
  }
}



Moving::Moving(const MovingContext &m_c,
               MovingXdata &x_data,
               WorldP w,	       
               LocatorP l,
               const Pos &raw_pos,
               Dir dirInitial)
  : Physical(m_c.physicalContext,w,l) {
  movingXdata = &x_data;
  mc = &m_c;

  rawPos = rawPosNext = raw_pos;
  rawPosChanged = False;
  dir = dirNext = dirInitial; /* Don't change.  See Shell. */

  if (m_c.animMax[dir]) {
    movingAnimNum = Utils::choose(m_c.animMax[dir]);
  }
  else {
    movingAnimNum = 0;
  }
  assert(movingAnimNum < PH_ANIM_MAX);
  
  // offsets[CO_air] and sizes[CO_air] are guaranteed to be set.
  // Be careful to set movingXdata and mc before calling get_offsets().
  Size *offsets = get_offsets();
  Area nArea(AR_RECT,raw_pos + offsets[dir],mc->sizes[dir]);
  area = areaNext = nArea;

  WorldP world = get_world();
  hitWall = hitWallNext = !world->open(area);
  
  moverVel.set_zero();

  extraVelNextSet = False;

  assert(context_valid());

  // invisible initializes itself.
  invisibleNext = 0;

  attackHook = NULL;

  // NOTE: protection initializes itself.
};



Moving::Moving() {
  assert(0);
}



Moving::Moving(InStreamP in,
               const MovingContext &m_c,
               MovingXdata &x_data,
               WorldP w,	       
               LocatorP l) 
  : Physical(in,m_c.physicalContext,w,l) {
  movingXdata = &x_data;
  mc = &m_c;

  rawPosChanged = False;
  hitWall = hitWallNext = False;
  moverVel.set_zero();
  extraVelNextSet = False;
  assert(context_valid());
  // invisible initializes itself.
  invisibleNext = 0;
  attackHook = NULL;
  movingAnimNum = 0;  // Don't choose randomly, we don't know the dir.

  _update_from_stream(in);
}



DEFINE_UPDATE_FROM_STREAM(Moving,Physical)



void Moving::_update_from_stream(InStreamP in) {
  // invisible and movingAnimNum
  u_char flags = in->read_char();
  Boolean isInvisible = (flags & 0x80) >> 7;
  if (isInvisible) {
    // Don't really have to worry about this time running out, 
    // object will get nuked if CLIENT_OLD_OBJECT_KILL turns pass without 
    // another update.
    invisible.set(PH_INVISIBLE_TIME);
  }
  else {
    // Set not invisible.
    invisible.set(0);
  }
  // All but high bit, 128 frames maximum
  //  movingAnimNum = flags & 0x7F;

  rawPos.read(in);
  rawPosNext = rawPos;

  area.read(in);
  areaNext = area;

  Dir dirRead = (Dir)in->read_char();

  // Changing direction, so reset animation.
  if (dir != dirRead) {
    movingAnimNum = 0;
  }

  dir = dirRead;
  dirNext = dir;

  NetDataP netData = get_net_data();

  // CHANGED: Ignore animation we read in, just increment whatever we have.
  //
  // Make sure we have a valid animation frame.
  // X has less frames than Windows sometimes.
  if (mc->animMax[dir]) {
    // Only animate if this is the first update_from_stream() coming in this
    // turn.
    if (!netData->clocked_this_turn()) {
      // new_anim_num() uses the timer.
      movingAnimNum = new_anim_num(movingAnimNum,mc->animMax[dir]);
      //movingAnimNum = movingAnimNum % mc->animMax[dir];
    }
  }
  else {
    // For funny things like Fireball.
    movingAnimNum = 0;
  }

  protection.read(in);

  vel.read(in);
  velNext = vel;

  assert(movingAnimNum < PH_ANIM_MAX);

  // Do not call update_area_next(), or do anything to affect followers.
}



int Moving::get_write_length() {
  return 
    Physical::get_write_length() + 
    sizeof(char) +                  // movingAnimNum, invisible
    Pos::get_write_length() +       // rawPos
    Area::get_write_length() +      // area
    sizeof(char) +                  // dir
    protection.get_write_length() + // protection
    Vel::get_write_length();         // vel
  // Not sending extraVel, only used for vibrating right now.
}



void Moving::write(OutStreamP out) {
  Physical::write(out);

  // Put invisible on the front of movingAnimNum.
  assert(movingAnimNum < 128);
  u_char flags = (u_char)movingAnimNum;
  Boolean isInvisible = is_invisible();
  flags |= (isInvisible & 0x1) << 7;
  out->write_char(flags);


  //  assert(rawPos == rawPosNext);
  if (!(rawPos == rawPosNext)) {
    cerr << "ERROR: rawPos and rawPosNext don't match for some " 
         << get_class_name() << endl;
  }
  rawPos.write(out);

  //  assert(area == areaNext);
  if (!(area == areaNext)) {
    cerr << "ERROR: area and areaNext don't match for some " 
         << get_class_name() << endl;
  }
  area.write(out);

  if (dir != dirNext) {
    cerr << "ERROR: dir and dirNext don't match for some " 
         << get_class_name() << endl;
  }
  // assert(dir == dirNext);
  out->write_char((char)dir);

  protection.write(out);

  if (!(vel == velNext)) {
    cerr << get_class_name() << " vel does not equal velNext" << endl;
  }
  vel.write(out);
}



Boolean Moving::is_moving()
{
  return True;
}



const Area &Moving::get_area() 
{
  return area;
}



const Area &Moving::get_area_next() 
{
  return areaNext;
}



Vel Moving::get_vel()
{
  return vel;
}



Boolean Moving::ignore_walls()
{
  return False;
}



Dir Moving::get_dir() 
{
  return dir;
}



Id Moving::get_protection()
{
  return protection;
}



void Moving::get_followers(PtrList &list) {
  // The protection.
  LocatorP locator = get_locator();
  PhysicalP p = locator->lookup(protection);
  if (p) {
    list.add(p);
  }

  Physical::get_followers(list);
}



void Moving::follow(const Area &followeeArea,Dir,
                  Boolean currentAndNext) {
  Pos pos = followeeArea.get_middle();

  // Don't use set_middle_next(), because that won't handle currentAndNext
  // properly.  _set_middle_next() exists for this purpose.
  _set_middle_next(pos);
  update_area_next(currentAndNext);
}



// This is the version other objects call on this one during the act() 
// phase.
void Moving::set_middle_next(const Pos &middleNext) {
  _set_middle_next(middleNext);
  update_area_next(False);
}



// Used by children who may have to call update_area_next().
void Moving::_set_middle_next(const Pos &middleNext) {
  Pos pos;
  Size size;
  area.get_rect(pos,size);

  // BUG: This won't work right if a child overrides get_size_offset_next().
  Size *offsets = get_offsets();
  Pos rPosNext = middleNext - offsets[dir] - 0.5f * size;

  /* Important to set rawPosChanged if called before act. */
  set_raw_pos_next(rPosNext);  
}



void Moving::relocate(const Pos &rPos) {
  assert(area == areaNext);
  assert(dir == dirNext);
  assert(vel == velNext);
  assert(rawPos == rawPosNext);
//  assert(extraVel == extraVelNext);
  assert(hitWall == hitWallNext);

  Pos pos;
  Size size;
  area.get_rect(pos,size);
  Size delta = pos - rawPos;
  
  rawPos = rawPosNext = rPos;
  Area nArea(AR_RECT,rawPos + delta,size);
  area = areaNext = nArea;

  hitWall = hitWallNext = False;
}



void Moving::set_mapped_next(Boolean val) {
  LocatorP locator = get_locator();
  PhysicalP p = locator->lookup(protection);
  if (p) {
    p->set_mapped_next(val);
  }

  Physical::set_mapped_next(val);
}



void Moving::set_protection(const Id &id) {
  protection = id;
}



void Moving::set_invisible_next(int turns) {
  assert(mc->invisibility); 
  if (turns == -1) {
    // Means turn off invisibility.
    invisibleNext = -1;
  }
  else {
    // Don't want to reduce the value if already set.
    invisibleNext = Utils::maximum(invisibleNext,turns);
    // Works even if invisibleNext was formerly -1.
  }
}



Boolean Moving::corporeal_attack(PhysicalP killer,int damage,
                                 AttackFlags flags) {
  assert(damage >= 0);
  
  // Check to see if Protection blocks the attack attack.
  LocatorP l = get_locator();
  ProtectionP p = (ProtectionP)l->lookup(protection);
  if (p && p->corporeal_protect(damage)) {
    return False;
  }

  return Physical::corporeal_attack(killer,damage,flags);
}



void Moving::heat_attack(PhysicalP killer,int h,Boolean secondary) {
  // Protection from heat.
  LocatorP l = get_locator();
  ProtectionP p = (ProtectionP)l->lookup(protection);
  if (p && p->heat_protect(h,secondary)) {
    return;
  }

  Physical::heat_attack(killer,h,secondary);
}



void Moving::act() {
  // Just set next variables.
  _act();
  // Don't run into walls.
  _update_area_next(False);
  Physical::act();
}



// Used by act() and dr_clock().
void Moving::_act() {
  // Note: no restrictions on vel.  (There are in Creature.)
  extraVel.limit((float)MOVING_EXTRA_VEL_MAX);
  Vel sumVel = velNext + extraVel + moverVel;
  moverVel.set_zero();
  extraVel.damp((float)EXTRA_VEL_DAMP);
  
  if (!rawPosChanged) {
    rawPosNext = rawPos + sumVel;
  }
}



int Moving::new_anim_num(int animPrev,int animMax) {
  assert(animMax > 0 && animPrev >= 0);
  int ret = animPrev;
  
  // Timer so that one frame may be shown over multiple turns.
  if (animTimer.ready()) {
    ret = (ret + 1) % animMax;
    animTimer.set(get_anim_time());
  }
  animTimer.clock();
  
  assert(ret < PH_ANIM_MAX);
  return ret;
}



void Moving::update() {
  _update();
  Physical::update();
}



void Moving::_update() {
  rawPos = rawPosNext;  
  rawPosChanged = False;
  
  area = areaNext;

  if(mc->animMax[dir]) {
    movingAnimNum = new_anim_num(movingAnimNum,mc->animMax[dir]);
  }
  
  // If direction changed, reset animation counter.
  if (dir != dirNext) {
    movingAnimNum = 0;
  }
  dir = dirNext;
  
  hitWall = hitWallNext;
  vel = velNext;  

  if (extraVelNextSet) {
    extraVel = extraVelNext;
    extraVelNextSet = False;
  }

  // Clock first, so will be invisible for, e.g. 1 turn if 
  // invisibleNext is set to 1.
  invisible.clock();
  if (invisibleNext > 0) {
    // Never reduce the amount of time invisible.
    invisible.set(Utils::maximum(invisible.get_remaining(),invisibleNext));
  }
  // Turn off invisibility.
  if (invisibleNext == -1) {
    invisible.set(0);
  }
  // invisibleNext doesn't act like the other next variables, it is just 
  // non-zero for 1 turn.
  invisibleNext = 0;
}



void Moving::avoid(PhysicalP other)
{
  const Area &otherArea = other->get_area();
  Size offset = area.avoid_no_up(otherArea);
  set_raw_pos_next(rawPos + offset);
}



void Moving::collide(PhysicalP other) {
  Mass m1 = this->get_mass();
  Mass m2 = other->get_mass();

  // Don`t have collisions with objects of zero mass.
  if (m1 > 0 && m2 > 0) {
    Vel v1 = this->get_vel();
    Vel v2 = other->get_vel();
    Area a1 = this->get_area();
    Area a2 = other->get_area();
    Pos p1 = a1.get_middle();
    Pos p2 = a2.get_middle();
    Size delta = p2 - p1;

    // Fix the bug with objects being dragged along.
    if (delta.dot(v1) > 0 ||  // p1 running into p2
        delta.dot(v2) < 0) {  // p2 running into p1
      Vel newVel(compute_collision(m1,v1.dx,m2,v2.dx),
                 compute_collision(m1,v1.dy,m2,v2.dy)); 
      set_vel_next(0);
      extraVel = newVel;
    }
  }
}



void Moving::die() {
  LocatorP locator = get_locator();
  PhysicalP p = locator->lookup(protection);
  // Must be careful about killing p because we are in the die() phase.
  if (p && !p->die_called()) {
    p->kill_self();
    p->die();
  }
  Physical::die();
} 



void Moving::dr_clock() {
  _act();

  // Keep from going through walls.
  _update_area_next(False);
  // We use False instead of True here, because _update() will copy the
  // next variables into the current ones.

  _update();

  Physical::dr_clock();
}



Touching Moving::offset_generator(Dir dir) {
  return Coord::dir_to_touching(dir);
}



Dir Moving::get_dir_next() {
  return dirNext;
}



int Moving::get_anim_time() {
  return MOVING_ANIM_TIME;
}



Size *Moving::get_offsets() {
  check_generate_offsets(*mc,movingXdata);
  return movingXdata->offsets;
}



Boolean Moving::draw_outline(ColorNum &colorNum) {
  if (is_invisible()) {
    return False;
  }
  
  // Draw outline around object if controlled by a Human or if a slave to
  // a Human master.
  IntelP intel = get_intel();
  IntelP master = NULL;
  if (intel) {
    if (intel->is_human()) {
      colorNum = ((HumanP)intel)->get_color_num();
      return True;
    }
    else {
      LocatorP locator = get_locator();
      master = locator->lookup(((MachineP)intel)->get_master_intel_id());
      if (master && master->is_human()) {
        colorNum = ((HumanP)master)->get_color_num();
        return True;
      }
    }
  }
  
  return False;
}



Boolean Moving::generate_masks() {
  return False;
}



void Moving::update_area_next(Boolean currentAndNext) {
  _update_area_next(currentAndNext);
  Physical::update_area_next(currentAndNext);
}



void Moving::get_size_offset_next(Size &size,Size &offset,Dir dir) {
  Size *offsets = get_offsets();

  size = mc->sizes[dir];
  offset = offsets[dir];
}



void Moving::set_size_offset_current_and_next(const Size &size,
                                              const Size &offset) {
  Area nArea(AR_RECT,rawPos + offset,size);
  area = areaNext = nArea;
}



void Moving::patch_size_and_size_next() {
  // Make rawPos{Next} and area{Next} fit with the local implemenation's 
  // (virtual) size and offset.  Do this by centering on the area read in 
  // from the remote machine.

  // local size and offset
  Size lSize,lOffset; 
  assert(dir == dirNext);
  get_size_offset_next(lSize,lOffset,dir);
  
  // remote pos and size
  Pos rPos;
  Size rSize;
  area.get_rect(rPos,rSize);

  // Do nothing if they already match
  if (!(lSize == rSize)) {
    // A hack to have Creature-specific knowledge here, but fuck it.
    // This whole method is a hack.
    Touching touching = CO_dn; // for objects on the ground.
    if (is_creature()) {
      touching = Coord::dir_to_touching(dir);
    }

    // Create newPos so that touching is preserved over the change in size.
    Pos newPos;
    // Get X coordinate
    if (touching == CO_l) {
      newPos.x = rPos.x;
    }
    else if (touching == CO_r) {
      newPos.x = rPos.x + rSize.width - lSize.width;
    }
    else {
      newPos.x = rPos.x + (int)(0.5 * (rSize.width - lSize.width));
    }
    // Get Y coordinate
    if (touching == CO_up) {
      newPos.y = rPos.y;
    }
    else if (touching == CO_dn) {
      newPos.y = rPos.y + rSize.height - lSize.height;
    }
    else {
      newPos.y = rPos.y + (int)(0.5 * (rSize.height - lSize.height));
    }    
                  
    Area newArea(AR_RECT,newPos,lSize);
    area = newArea;
    areaNext = newArea;

    rawPos = newPos - lOffset;
    rawPosNext = rawPos;
  }
}



void Moving::_update_area_next(Boolean currentAndNext) {
  // Compute areaNext and hitWallNext.  May modify rawPosNext or dirNext.
  Size size,off_set;
  get_size_offset_next(size,off_set,dirNext);
  Area first_guess(AR_RECT,rawPosNext + off_set,size);
    
  if (ignore_walls()) {
    areaNext = first_guess;
    hitWallNext = False;
  }
  else {
    WorldP world = get_world();
    Size offset;

    // Use velNext to avoid ChopperBoy up down through wall bug.
    switch (world->open_offset(offset,first_guess,velNext)) {
    case W_NO_SIG:
      areaNext = first_guess;
      hitWallNext = False;
      break;
      
    case W_CLOSE:
      rawPosNext = rawPosNext + offset;
      areaNext = first_guess;
      areaNext.shift(offset);
      hitWallNext = True;
      /* Doesn't set velocity to 0. */
      break;
      
    case W_CLOSE_BAD:
      rawPosNext = rawPosNext + offset;
      areaNext = first_guess;
      areaNext.shift(offset);
      hitWallNext = True;
      set_vel_next(0); // ????
      break;
      
    case W_FAILURE:
      /* Abort all changes to area, rawPos, or dir. */
      areaNext = area;
      rawPosNext = rawPos;
      dirNext = dir; 
      hitWallNext = True;
      set_vel_next(0); // ??? Will help? 8/18/94
      break;
    }
  }
  
  // Also apply changes to current value.
  if (currentAndNext) {
    rawPos = rawPosNext;
    area = areaNext;
    dir = dirNext;
    hitWall = hitWallNext;
  }
}



// If you get an assertion here, you probably messed up something in one of the
// .bitmaps files.
Boolean Moving::context_valid()
{
  assert(PH_AUTO_GEN != 0 && PH_AUTO_GEN != (CMN_BITS_ID)1);

  /*  if (!(mc->sizes[CO_climb] == mc->sizes[CO_climb_UP]) ||
      !(mc->sizes[CO_climb] == mc->sizes[CO_climb_DN]))
      return False; 
*/ /* frog.bitmaps */

  // See Moving::corner

#if 0
  if ((mc->animMax[CO_r] && mc->animMax[CO_r_UP]) && !(mc->sizes[CO_r] == mc->sizes[CO_r_UP])) {
    assert(0);
    return False;
  }
  if ((mc->animMax[CO_r] && mc->animMax[CO_r_DN]) && !(mc->sizes[CO_r] == mc->sizes[CO_r_DN])) {  
    assert(0);
    return False;
  }
#endif

  if ((mc->animMax[CO_dn_R] && mc->animMax[CO_dn_L]) && 
      !(mc->sizes[CO_dn_R] == mc->sizes[CO_dn_L])) {
    assert(0);
    return False;
  }
#if 0
  if ((mc->animMax[CO_l] && mc->animMax[CO_l_UP]) && !(mc->sizes[CO_l] == mc->sizes[CO_l_UP])) {
    assert(0);
    return False;
  }
  if ((mc->animMax[CO_l] && mc->animMax[CO_l_DN]) && !(mc->sizes[CO_l] == mc->sizes[CO_l_DN])) {
    assert(0);
    return False;
  }
#endif

  if ((mc->animMax[CO_up_R] && mc->animMax[CO_up_L]) && 
      !(mc->sizes[CO_up_R] == mc->sizes[CO_up_L])) {
    assert(0);
    return False;
  }
  // No longer true that CO_dn or CO_up have to be same size as CO_dn_[RL] and
  // CO_up_[RL], respectively.  Hugger.
  //   
  // Removed even more constraints for dog on the ceiling and walls.
  
  // No longer assert that all objects have CO_air.


  for (int dir = 0; dir < CO_DIR_MAX; dir++) {
    if (mc->animMax[dir] < 0) {
      assert(0);
      return False;
    }
    
    int m;
    for (m = 0; m < mc->animMax[dir]; m++) {
      if (mc->pixmapBits[dir][m] == 0) {
        // Auto-generated pixmaps will have 0 for several of the
        // frames.
        if (mc->pixmapBits[dir][0] != PH_AUTO_GEN) {
          assert(0);
          return False;
        }
      }
#if X11
      if (mc->maskBits[dir][m] == 0) {
        // Auto-generated frames will also have 0 for the 
        // masks.
        // Also, masks are null for XPM specified pixmaps.
        if (mc->pixmapBits[dir][0] != PH_AUTO_GEN && !mc->useXPM) {
          assert(0);
          return False;
        }
      }
#endif
    } // for m
  } // for dir
    
  return True;
}



float Moving::compute_collision(Mass m1,float v1,Mass m2,float v2) {
  assert(m1 > 0 && m2 > 0);

  float A = m1 * v1 + m2 * v2;
  float B = 0.5f * m1 * v1 * v1 + 0.5f * m2 * v2 * v2;
  
  float a = (float)(m1*m2 + m1*m1);
  float b = -2 * m1 * A;
  float c = A * A - 2 * m2 * B;

  float disc = b * b - 4 * a * c;

  /* Hack.  If the discriminant is negative for some reason, returning 0 
     shouldn't do too much damage. */
  if (disc < 0) {
    return 0.0f;
  }

  float v1_1 = (float)((-b + sqrt(disc))/ (2 * a));
  float v1_2 = (float)((-b - sqrt(disc))/ (2 * a));
  
  return fabs(v1_1 - v1) > fabs(v1_2 - v1) ? v1_1 : v1_2;
}



void Moving::check_generate_offsets(const MovingContext &mc,MovingXdata *xdata) {
  if (xdata->offsetsValid) {
    return;
  }

  Dir dir;
  // Double check sizeMax
  Size sizeMax;
  sizeMax.set(0,0);
  for (dir = 0; dir < CO_DIR_MAX; dir++) {
    sizeMax.width = Utils::maximum(mc.sizes[dir].width,sizeMax.width);
    sizeMax.height = Utils::maximum(mc.sizes[dir].height,sizeMax.height);
  }
  if (!(sizeMax == mc.physicalContext.sizeMax)) {
    cerr << mc.physicalContext.className 
         << ": computed sizeMax (" << sizeMax.width << "," << sizeMax.height 
         << "), doesn't equal that in context (" << mc.physicalContext.sizeMax.width 
         << "," << mc.physicalContext.sizeMax.height << ")" << endl;
    assert(0);
  }

  // Some classes specify their own special code for generating offsets.
  if (mc.offsetGenerator) {
    Coord::generate_offsets(xdata->offsets,mc.sizes,sizeMax,
                            mc.offsetGenerator);
  }
  else {
    Coord::generate_offsets(xdata->offsets,mc.sizes,sizeMax,
                            Moving::offset_generator);
  }

  xdata->offsetsValid = True;
}



// Put the CMN_BITS_ID in bits 0-15, transform in 16-19, 
// isMask in bit 23, 24-31 are zero.
void* Moving::compute_key(Dir dir,int animNum,Boolean isMask,
                          const MovingContext* mc,MovingXdata*) {
  // We know unsigned int is 32 bits from XETP::check_sizes().
  // We also know that void* is at least 32 bits.

  assert(mc->animMax[dir] > 0);
  
  // If auto-generated bitmap, use the transformations as part of the 
  // SurfaceManager key.
  int tNum = 0;
  const TransformType* transforms = NULL;
  Dir baseDir = dir;
  if (mc->pixmapBits[dir][0] == PH_AUTO_GEN) {
    transforms = Transform2D::get_transforms(tNum,dir,mc->transformOverride);
    baseDir = Transform2D::get_base(dir,mc->transformOverride);
  }

  // Use the pixmap resource id as the base.
  u_int cmnBitsId = (unsigned int)mc->pixmapBits[baseDir][animNum];

  // It is ok to call OneTransform::compute_key() even if there is no 
  // transformation, will just return 0x0.  
  u_int transKey = OneTransform::compute_key(transforms,tNum);
  // transKey must fit in 4 bits.
  assert((transKey & 0xfffffff0) == 0x0);

  // cmnBitsId must fit in 16 bits.
  assert((cmnBitsId & 0xffff0000) == 0x0);
  u_int ret = (transKey << 16) | cmnBitsId;

  // In bit 23.
  if (isMask) {
    ret |= 0x1 << 23;
  }

  // Top-most byte must be empty, see comments about unique keys in s_man.h.
  assert((ret & 0xff000000) == 0x0);

  return (void*)ret;
}



void Moving::check_auto_generated(const MovingContext& mc) {
  int n;
  int m;
  for (n = 0; n < CO_DIR_MAX; n++) {
    if (mc.animMax[n] > 0 && mc.pixmapBits[n][0] == PH_AUTO_GEN) {
      assert(!Transform2D::is_base(n,mc.transformOverride));
      Dir base = Transform2D::get_base(n,mc.transformOverride);

      // Must be same number of frames.
      assert(mc.animMax[n] == mc.animMax[base]);

      // Size must be same as the transformed size of the base.
      int tNum;
      const TransformType* transforms = 
        Transform2D::get_transforms(tNum,n,mc.transformOverride);
      Size trSize = mc.sizes[base];
      for (m = 0; m < tNum; m++) {
        trSize = Transform2D::apply(transforms[m],trSize);
      }
      assert(mc.sizes[n] == trSize);
    }
  }
}



Shot::Shot(const ShotContext &s_c,ShotXdata &xdata,WorldP w,LocatorP l,
	   const Pos &p,const Id &shoot_er,
	   Dir shotDir,Dir movingDir) 
     : Moving(s_c.movingContext,xdata,w,l,p,movingDir) {
  context = &s_c;
  shooter = shoot_er;

  const Vel *unitVels = get_unit_vels();
  Vel initVel = (float)context->speed * unitVels[shotDir];
  set_vel(initVel);
  set_vel_next(initVel);

  // This is a little funny, changing next values in the constructor.
  // Won't be clocked until next turn.
  if (hit_wall()) {
    kill_self();
  }

  firstTurn = True;
}



CONSTRUCTOR_INTERNAL_NODE_IO(Shot,Moving,moving) {
  context = &cx;
  firstTurn = False;
  // shooter initializes itself.
}



Boolean Shot::is_shot() {
  return True;
}



void Shot::avoid(PhysicalP) {}



void Shot::collide(PhysicalP other) {
  if (other->is_shot()) {
    return;
  }

  // If we hit out in a wall, don't hurt anyone.
  if (hit_wall() && firstTurn) {
    return;
  }
  // Will kill_self() in ::act().

  LocatorP locator = get_locator();
  PhysicalP p = locator->lookup(shooter);
  // p may be NULL.
  other->corporeal_attack(p,context->damage);

  kill_self();
}



void Shot::act() {
  if (hit_wall()) {  // Used to be hit_wall_next() but lances died too soon.
    kill_self();
  }

  Moving::act();
}



void Shot::update() {
  firstTurn = False;
  Moving::update();
}



Droplet::Droplet(const DropletContext &cx,DropletXdata &x_data,
                 WorldP w,LocatorP l,const Pos &raw_pos)
  : Moving(cx.movingContext,x_data,w,l,raw_pos) {
  context = &cx;

  inAir = True;

  gravPulser.set_pulse_value(context->gravity);
}



CONSTRUCTOR_INTERNAL_NODE_IO(Droplet,Moving,moving) {
  inAir = False; // Do we need to stream this out?
  context = &cx;
}



Boolean Droplet::collidable() {
  return False;
}



void Droplet::act() {
  Dir dir = get_dir();

  if (inAir) {
    Touching touching;
    Hanging hanging;

    const Area &area = get_area();
    WorldP world = get_world();
	  MoverP dummy;
    world->compute_touching_hanging(touching,hanging,dummy,area);

    // If in air and hit wall, stick to wall.
    if ((!context->stickWalls && touching == CO_dn) || 
        (context->stickWalls && touching != CO_air)) {
      set_dir_next((Dir)touching);
      set_vel_next(0);
      timer.set(context->dissolveTime);
      inAir = False;
    }

    // Account for gravity.  
    else {  
      Vel velNext = get_vel_next();
      const int gravAcc = gravPulser();
      velNext.dy += gravAcc;

      // Reuse FALLING_VEL_MAX.
      velNext.limit((float)FALLING_VEL_MAX);
      set_vel_next(velNext);

      set_dir_next(velNext.get_dir());
    }
  }
  else {
    if (timer.ready()) {
      kill_self();
    }
  }
  // Note: doesn't check if world has changed.

  timer.clock();
  gravPulser.clock();
  Moving::act();
}



ClassId Droplet::choose_droplet_class(DropletSet set) {
  assert(set != DROPLET_NONE);
  DropletSet testSet = DROPLET_NONE;

  // This should be more extendable.
  PtrList list;
  if (set & DROPLET_BLOOD) {
    list.add((void*)A_Blood);
    testSet |= DROPLET_BLOOD;
  }
  if (set & DROPLET_GREEN_BLOOD) {
    list.add((void*)A_GreenBlood);
    testSet |= DROPLET_GREEN_BLOOD;
  }
  if (set & DROPLET_OIL) {
    list.add((void*)A_OilDroplet);
    testSet |= DROPLET_OIL;
  }
  if (set & DROPLET_MORE_OIL) {
    list.add((void*)A_OilDroplet);
    testSet |= DROPLET_MORE_OIL;
  }
  if (set & DROPLET_FEATHER) {
    list.add((void*)A_Feather);
    testSet |= DROPLET_FEATHER;
  }
  assert(!list.empty());

  // If this assert fails, then you probably need to add your new Droplet class
  // to this function.
  assert(testSet == set);

  int which = Utils::choose(list.length());
  return (ClassId)list.get(which);
}



PhysicalP Droplet::create_and_add_if(ClassId classId,WorldP w,
                                     LocatorP l,const Pos &p) {
  assert(classId != A_None);

  PhysicalP droplet = NULL;
  int index = get_available_entry(l);
  if (index != -1) {

    const PhysicalContext* pc = l->get_context(classId);
    assert(pc);
    droplet = pc->create(pc->arg,w,l,p);
    assert(droplet);
    
    l->add(droplet);

    // splatter up, right/left choosen randomly.
    int speed = (int)(DROPLET_SPLATTER_SPEED * 
                      ((Droplet*)droplet)->context->speedModifier);
    speed = Utils::maximum(speed,1);

    Vel vel((float)(Utils::choose(2 * speed + 1) - speed),
            (float)-(Utils::choose(speed) + 1)); 

    ((MovingP)droplet)->set_vel_next(vel);
    dropletList[index] = droplet->get_id();
  }
  return droplet;
}



int Droplet::get_available_entry(LocatorP locator) {
  // Initialize new slot. Never uninitialized, but doesn't matter.
  // No memory leak.
  if (dropletMax < DROPLET_MAX) {
    return dropletMax++;
  }

  // Else try to reuse existing slots.
  for (int n = 0; n < DROPLET_MAX; n++) {
    // Important!  Check for recentAdded objects in this lookup.
    if (!locator->lookup(dropletList[n],True))
      return n;
  }

  // Nothing free.  Tough shit.
  return -1;
}



Id Droplet::dropletList[DROPLET_MAX];



int Droplet::dropletMax = 0;



Falling::Falling(const FallingContext &f_c,FallingXdata &x_data,
		 WorldP w,LocatorP l,const Pos &raw_pos,Dir dirInitial) :
       Moving(f_c.movingContext,x_data,w,l,raw_pos,dirInitial) {
}



CONSTRUCTOR_INTERNAL_NODE_IO(Falling,Moving,moving) {
}



void Falling::act() {
  // Account for gravity.  
  if (gravity_on()) {
    Touching touching;
    Hanging hanging;

    const Area &area = get_area();
    WorldP world = get_world();
	  MoverP dummy;
    world->compute_touching_hanging(touching,hanging,dummy,area);

    Vel velNext = get_vel_next();
    if (touching != CO_dn) {
      velNext.dy += PH_GRAVITY; 
    }
    else {
      velNext.dy = 0.0f;
    }

    if (touching == CO_r || touching == CO_l) {
      velNext.dx = 0.0f;
    }

    velNext.limit((float)FALLING_VEL_MAX);
    set_vel_next(velNext);
  }

  Moving::act();
}



Boolean Falling::gravity_on() {
  return True;
}



Touchable::Touchable(const TouchableContext &t_c,TouchableXdata &x_data,
		     WorldP w,LocatorP l,const Pos &raw_pos) 
: Falling(t_c.fallingContext,x_data,w,l,raw_pos) 
{
  touched = False;
  context = &t_c;
}



CONSTRUCTOR_INTERNAL_NODE_IO(Touchable,Falling,falling) {
  touched = False;
  context = &cx;
}



int Touchable::get_drawing_level() {
  return 0;
}



Boolean Touchable::collidable()
{
  return False;
}



void Touchable::act()
{
  if (!touched)
    {
      PhysicalP nearby[OL_NEARBY_MAX];
      int nItems;
      LocatorP l = get_locator();
      l->get_nearby(nearby,nItems,this,TOUCHABLE_RADIUS);
      const Area &area = get_area();
      
      for (int n = 0; n < nItems; n++)
	    {
	      IntelP intel = nearby[n]->get_intel();
	      if (intel && intel->is_human())
	        {
	          const Area &otherArea = nearby[n]->get_area();
	          if (area.overlap(otherArea))
    		    touched = True;
	        }
	    }
    }

  Falling::act();
}



Heavy::Heavy(const HeavyContext &h_c,HeavyXdata &x_data,
	     WorldP w,LocatorP l,const Pos &raw_pos) 
     : Falling(h_c.fallingContext,x_data,w,l,raw_pos) {
  context = &h_c;
  // pusher starts out invalid
  alwaysHurts = False;
}



CONSTRUCTOR_INTERNAL_NODE_IO(Heavy,Falling,falling) {
  context = &cx;
  // pusher starts out invalid
  alwaysHurts = False;
}



void Heavy::collide(PhysicalP other) {
  LocatorP l = get_locator();
  Boolean didDamage = False;

  // If alwaysHurts is set, use different criteria for deciding 
  // whether to do damage.  Always do damage when Heavy is in the
  // air.
  if (alwaysHurts) {
    // Kind of a waste to compute_touching_hanging(), since 
    // Falling::act() already does it.
    WorldP world = get_world();
    Touching touching;
    Hanging hanging;
	  MoverP dummy;
    world->compute_touching_hanging(touching,hanging,dummy,get_area());

    if (touching != CO_dn) {
      // Attack is done by the pusher.
      // Although it is unlikely that there will be a pusher when
      // alwaysHurt is set.
      PhysicalP p = l->lookup(pusher);
      other->corporeal_attack(p,context->damage);

      didDamage = True;
    }    
  }
  // Usual logic for Heavy objects.
  else {
    const Vel &vel = get_vel();
    if (vel.dy >= HEAVY_DAMAGE_VEL) {
      const Area &area = get_area();
      const Area &otherArea = other->get_area();
      const Pos &otherMiddle = otherArea.get_middle();

      // If target is below or inside the Heavy.
      Dir dirTo = area.dir_to(otherMiddle);
      if ((dirTo == CO_DN_R) || (dirTo == CO_DN) || 
          (dirTo == CO_DN_L) || (dirTo == CO_air)) {
        // Attack is done by the pusher.
        PhysicalP p = l->lookup(pusher);
        other->corporeal_attack(p,context->damage);

        didDamage = True;
      }
    }
  } // alwaysHurts

  // Only accept the other as pusher if the Heavy is not in a state
  // where it can hurt others.
  if (!didDamage) {
    // Don't change pusher if bumped by another stationary object.
    //// Come to think of it, should really record the IntelId of 
    //// the pusher, not the Locator Id.
    if (other->get_intel()) {
      pusher = other->get_id();
    }
  }

  Falling::collide(other);
}



Liftable::Liftable(const LiftableContext &cx,LiftableXdata &x_data,
                   WorldP w,LocatorP l,const Pos &raw_pos) 
  : Heavy(cx.heavyContext,x_data,w,l,raw_pos) {
  // lifter starts out invalid
  excludedTimer.set_max(LIFTABLE_EXCLUDED_TIME);
  excludedTime = False;  
}



CONSTRUCTOR_INTERNAL_NODE_IO(Liftable,Heavy,heavy) {
  // lifter starts out invalid
  excludedTime = False;
}



Boolean Liftable::is_liftable() {
  return True;
}



int Liftable::get_drawing_level() {
  return 2;
}



Boolean Liftable::being_lifted() {
  if (excludedTime) {
    return False;
  }
  
  LocatorP l = get_locator();
  PhysicalP p = l->lookup(lifter);
  return (p != NULL);
}



void Liftable::update() {
  excludedTimer.clock();
  if (excludedTime && excludedTimer.ready()) {
    excludedTime = False;
    lifter.invalidate();
    // Disable set_dont_collide().
    set_dont_collide(lifter);
  }

  Heavy::update();
}



void Liftable::collide(PhysicalP other) {
  LocatorP l = get_locator();
  
  PhysicalP p = l->lookup(lifter);
  if (p && !excludedTime) {
    // Being lifted, so don't hurt anything.
    Falling::collide(other);
  }
  else {
    // Just like any other Heavy object.
    Heavy::collide(other);
  }
}



void Liftable::lift(PhysicalP l) {
  lifter = l->get_id();
  set_dont_collide(lifter);
  excludedTime = False;

  // Want lifter to get credit for any kills.
  set_pusher(lifter);
}



void Liftable::release(PhysicalP) {
  // Don't invalidate lifter or set_dont_collide() until timer expires.
  excludedTime = True;
  excludedTimer.set();
}



Item::Item(const ItemContext &c_x,
	   ItemXdata &x_data,
	   WorldP w,
	   LocatorP l,
	   const Pos &pos,Dir dirInitial) :
       Falling(c_x.fallingContext,x_data,w,l,pos,dirInitial) {
  //canTake = ntimer;
  excludedTimer.set_max(ITEM_CAN_TAKE_TIME);
  dieMessage = DESTROYED;
  held = False;
  context = &c_x;
  cantTake = False;
}



CONSTRUCTOR_INTERNAL_NODE_IO(Item,Falling,falling) {
  dieMessage = DESTROYED;
  context = &cx;
  cantTake = False;

  _update_from_stream(in);
}



DEFINE_UPDATE_FROM_STREAM(Item,Falling)



void Item::_update_from_stream(InStreamP in) {
  holderId.read(in);

  // Set value of held based on holderId.
  // We should really just get rid of "held" all together and 
  // just use whether holderId is valid or not.
  LocatorP locator = get_locator();
  if (locator->lookup(holderId)) {
    held = True;
  }
  else {
    held = False;
  }
}



int Item::get_write_length() {
  return 
    Falling::get_write_length() + 
    Identifier::get_write_length(); // holderId or dummy
}



void Item::write(OutStreamP out) {
  Falling::write(out);
  if (held) {
    holderId.write(out);
  }
  else {
    // Don't use holderId because it might be invalid.
    Id dummy;
    dummy.write(out);
  }
}



Boolean Item::can_take(PhysicalP taker) {
	return (taker->get_id() != excluded || excludedTimer.ready()) &&
		!held && 
		!cantTake;
}

  

int Item::get_drawing_level() {
  return 2;
}



Id Item::get_holder_id() {
  if (held) {
    return holderId;
  }
  else {
    Id invalid;
    return invalid;
  }
}



int Item::get_coolness() {
	return context->coolness;
}



void Item::set_quiet_death() {
  dieMessage = NONE;
  Falling::set_quiet_death();
}



void Item::taken(PhysicalP p)  {
  assert(!held);
  assert(!cantTake);
  held = True; 
  holderId = p->get_id();
  set_mapped_next(False);
}



void Item::dropped(PhysicalP dropper) {
  // Can happen in die phase (from User::die).  

  LocatorP locator = get_locator();
  assert(held);
  assert(locator->lookup(holderId) == dropper);

  held = False; 
  const Area area = dropper->get_area();

  Pos middleNext = area.get_middle(); 

  // Decide whether to drop to side or below.
  if (dropper->is_creature()) {
    Touching touching = ((CreatureP)dropper)->get_touching_area();
    if (touching == CO_dn) {
	    switch(dropper->get_dir()) {
	    case CO_center_R:
	    case CO_dn_R:
	      middleNext.x -= ITEM_DROP_OFFSET_WIDTH;
	      break;
	    case CO_center_L:
	    case CO_dn_L:
	      middleNext.x += ITEM_DROP_OFFSET_WIDTH;
	      break;
	    default:
	      middleNext.x += (Utils::coin_flip() ? ITEM_DROP_OFFSET_WIDTH : 
			       - ITEM_DROP_OFFSET_WIDTH);
	    }
	  }
    else {
      middleNext.y += ITEM_DROP_OFFSET_HEIGHT;
    }
  }
  else {
    middleNext.x += 
      (Utils::coin_flip() ? ITEM_DROP_OFFSET_WIDTH : - ITEM_DROP_OFFSET_WIDTH);
  }

  _set_middle_next(middleNext);
  excludedTimer.set();
  excluded = dropper->get_id();
  set_mapped_next(True);

  update_area_next(False);
}



void Item::use(PhysicalP) {
  dieMessage = USED;
  SoundRequest req(context->useSound,get_area());
  LocatorP locator = get_locator();
  locator->submitSoundRequest(req);
}



void Item::act() {
  excludedTimer.clock();
  Falling::act();
}



void Item::die() {
  LocatorP locator = get_locator();
  ostrstream msg;
  switch (dieMessage) {
  case NONE:
    break;
  case USED:
    msg << get_class_name() << " has been used." << ends;
    locator->message_enq(msg.str());
    break;
  case DESTROYED:
    msg << get_class_name() << " is destroyed." << ends;
    locator->message_enq(msg.str());
    break;
  };

 Falling::die();
}



Boolean Item::gravity_on() {
  return !is_held();
}



AutoUse::AutoUse(const AutoUseContext &c,
		 AutoUseXdata &x_data,
		 WorldP w,
		 LocatorP l,
		 const Pos &pos)
 : Item(c.itemContext,x_data,w,l,pos) {
  context = &c;
}



CONSTRUCTOR_INTERNAL_NODE_IO(AutoUse,Item,item) {
  context = &cx;  
}



Boolean AutoUse::is_auto_use()
{
  return True;
}



void AutoUse::collide(PhysicalP other) {
  IntelP otherIntel = other->get_intel();

  // First, ways in which AutoUse doesn't automatically use() itself.
  if (!other->is_creature() || 
      ((CreatureP)other)->get_holder() || 
      !otherIntel ||
      (!otherIntel->is_human() && context->humansOnly)) {
    Item::collide(other);
  }
  else {
    use(other);
  }
}



Drugs::Drugs(const DrugsContext &c,
		 DrugsXdata &x_data,
		 WorldP w,
		 LocatorP l,
		 const Pos &pos)
  : AutoUse(c.autoUseContext,x_data,w,l,pos) {
  context = &c;
}



CONSTRUCTOR_INTERNAL_NODE_IO(Drugs,AutoUse,autoUse) {
  context = &cx;
}



Boolean Drugs::is_drugs()
{
  return True;
}



void Drugs::taken(PhysicalP p) {
  // Prevent non-biologicals from picking up drugs.

  if (p->is_creature() && !((CreatureP)p)->biological()) {
    ostrstream msg;
    msg << "Non-Biological Creatures Cannot Use Drugs" << ends;
    LocatorP locator = get_locator();
    locator->arena_message_enq(msg.str(),p);
    kill_self();
  }
  else {
    AutoUse::taken(p);
  }
}



void Drugs::use(PhysicalP p)
{
  assert(p->alive() && p->is_creature());
  LocatorP locator = get_locator();

  if (((CreatureP)p)->biological()) {
    // check for chance of overdose if context->odChance is not 0.
    assert(context->odChance >= 0);
    if (context->odChance && Utils::choose(context->odChance) == 0) {
      p->kill_self();
      ostrstream msg;
      msg << p->get_class_name() << " dies from crack overdose." << ends;
      locator->message_enq(msg.str());
      
      ostrstream arenaMsg;
      arenaMsg << "You Died From Drug Overdose" << ends;
      locator->arena_message_enq(arenaMsg.str(),p);

      set_quiet_death(); // so doesn't report "has been used" message.
    }    
    else {
      /*** Add to Creature's modifier list ***/ 
      ModifierP modifiers = ((CreatureP)p)->get_modifiers();
      assert(modifiers);
      ModifierP m = create_modifier(); 
      modifiers->append_unique(m);
    }
  }
  else {
    // Shouldn't get here, should kill self when picking it up.
    ostrstream msg;
    msg << "Non-Biological Creatures Cannot Use Drugs" << ends;
    locator->arena_message_enq(msg.str(),p);
  }
  
  kill_self();
  AutoUse::use(p);
}



Shield::Shield(const ShieldContext &s_c,
	       ShieldXdata &x_data,
	       WorldP w,
	       LocatorP l,
	       const Pos &pos) 
: AutoUse(s_c.autoUseContext,x_data,w,l,pos) {
}



CONSTRUCTOR_INTERNAL_NODE_IO(Shield,AutoUse,autoUse) {
}



Boolean Shield::is_shield() {
  return True;
}



void Shield::use(PhysicalP p) {
  assert(p->is_moving());

  LocatorP l = get_locator();
  Id id = ((MovingP)p)->get_protection();
  ProtectionP oldProtection = (ProtectionP) l->lookup(id);
  if (oldProtection) {
    oldProtection->kill_self();
  }

  ProtectionP protection = create_protection(p->get_area());
  l->add(protection);
  ((MovingP)p)->set_protection(protection->get_id());

  kill_self();
  AutoUse::use(p);
}



Animated::Animated(const AnimatedContext &a_c,
                   AnimatedXdata &x_data,
                   WorldP w,
                   LocatorP l,
                   const Pos &pos,Frame frameInitial) :
  Item(a_c.itemContext,x_data,w,l,pos,frameInitial + FRAME_BASE) 
{}



CONSTRUCTOR_INTERNAL_NODE_IO(Animated,Item,item) {
}



Weapon::Weapon(const WeaponContext &c_x,
	       WeaponXdata &x_data,
	       WorldP w,
	       LocatorP l,
	       const Pos &pos,Dir dirInitial) 
     : Item(c_x.itemContext,x_data,w,l,pos,dirInitial) 
{
  enteredScope = False;
  wc = &c_x;
}



CONSTRUCTOR_INTERNAL_NODE_IO(Weapon,Item,item) {
  enteredScope = False;
  wc = &cx;
}



void Weapon::fire(const Id &,ITcommand)
{
  //not best way to do this
  //put noise in shot so it can decide if repeating.
  LocatorP locator;
  locator = get_locator();
  SoundRequest req(wc->soundid,get_area());
  locator->submitSoundRequest(req);
}



Boolean Weapon::useful_no_ammo() {
  return False;
}



Boolean Weapon::fire_repeatedly() {
  return True;
}



void Weapon::use(PhysicalP) {
  // Doesn't call Item::use().
}



void Weapon::enter_scope_next(PhysicalP)
{
/*  #ifdef PRINT_ERRORS
  if (enteredScope)
    cerr << "Warning:: " << get_class_name() << " already entered scope." << endl;
    #endif */
  enteredScope = True;
}



void Weapon::leave_scope_next(PhysicalP)
{
/* #ifdef PRINT_ERRORS
  if (!enteredScope)
    cerr << "Warning:: " << get_class_name() << " not in scope." << endl;
    #endif */
  enteredScope = False;
}



void Weapon::take_ammo_from(WeaponP other)
{
  int thisAmmo = get_ammo();
  int otherAmmo = other->get_ammo();

  if ((thisAmmo != PH_AMMO_UNLIMITED) && (otherAmmo != PH_AMMO_UNLIMITED))
    {
      int maxTake = get_ammo_max() - thisAmmo;
      int trans = Utils::minimum(maxTake,otherAmmo);
      
      set_ammo(thisAmmo + trans);
      other->set_ammo(otherAmmo - trans);
    }
}



Dir Weapon::compute_weapon_dir(ITcommand command)
{
  return Intel::command_weapon_to_dir_8(command);
}



Cutter::Cutter(const CutterContext &c_x,CutterXdata &x_data,
       WorldP w,LocatorP l,const Pos &pos)
     : Weapon(c_x.weaponContext,x_data,w,l,pos,UNHELD_DIR)
{
  // Now just uses UNHELD_DIR for direction when unheld.

  // When dropping Cutter, seems to drop too far to right??

  inScope = inScopeNext = False; 
  context = &c_x;
}



CONSTRUCTOR_INTERNAL_NODE_IO(Cutter,Weapon,weapon) {
  inScope = inScopeNext = False; 
  context = &cx;

  _update_from_stream(in);
}



DEFINE_UPDATE_FROM_STREAM(Cutter,Weapon)



void Cutter::_update_from_stream(InStreamP in) {
  inScope = (Boolean)in->read_char();
  inScopeNext = inScope;
}



int Cutter::get_write_length() {
  return 
    Weapon::get_write_length() + 
    sizeof(char);                   // inScope
}



void Cutter::write(OutStreamP out) {
  Weapon::write(out);
  
// Don't know why this happens.  Still need to check out this warning.
#if 0
  if (inScope != inScopeNext) {
    cerr << "WARNING: Cutter::write() inScope != inScopeNext" << endl;
  }
#endif
  out->write_char((u_char)inScope);
}



Boolean Cutter::is_cutter() {
  return True;
}



Boolean Cutter::ready() {
  return False;
}



int Cutter::get_ammo()
{
  return PH_AMMO_UNLIMITED;
}



int Cutter::get_ammo_max()
{
  return PH_AMMO_UNLIMITED;
}



Boolean Cutter::ignore_walls()
{
  return is_held();
}


			     
void Cutter::set_ammo(int)
{
  assert(0);
}



void Cutter::follow(const Area &followeeArea,Dir followeeDir,
                    Boolean currentAndNext) {
  Pos middle = followeeArea.get_middle();
  if (inScope) {
    set_dir_next(followeeDir);
    _set_middle_next(middle + context->offsets[followeeDir]);
  }
  else {
    _set_middle_next(middle);
  }
  // Don't use set_middle_next(), that won't handle currentAndNext correctly.
  update_area_next(currentAndNext);
}



void Cutter::enter_scope_next(PhysicalP user) {
  if(!entered_scope()) {
    set_dont_collide(user->get_id());
    set_mapped_next(True);
    killerId = user->get_id();
    set_dir_next(CO_center);
    inScopeNext = True;
    // moving will now get the correct results from 
    // Cutter::get_size_offset_next()
    update_area_next(False); 
    Weapon::enter_scope_next(user);
  }
#ifdef PRINT_ERRORS
  else {
    cerr << "Warning:: " << get_class_name() << " entered scope twice." << endl;
  }
#endif
}



void Cutter::leave_scope_next(PhysicalP user) {
  if (entered_scope()) {
    // Probably should remember what previous value of dontCollide was.
    Id invalid;
    set_dont_collide(invalid);
    
    set_mapped_next(False);
    set_dir_next(UNHELD_DIR);
    inScopeNext = False;
    // moving will now get the correct results from 
    // Cutter::get_size_offset_next()
    update_area_next(False); 
    Weapon::leave_scope_next(user);
  }
#ifdef PRINT_ERRORS
  else
    cerr << "Warning:: " << get_class_name() << " left scope twice." << endl;
#endif
}



void Cutter::collide(PhysicalP other) {
  if (inScope) {
    PhysicalP p;
    LocatorP locator = get_locator();
    if (p = locator->lookup(killerId)) {
	    assert(p->is_moving());
	    other->corporeal_attack(p,context->damage);
	    ((MovingP)p)->attack_hook();
    }
    else {
	    other->corporeal_attack(NULL,context->damage);
    }
    SoundRequest req(context->cuttingsound,get_area());
    locator->submitSoundRequest(req);
  }
  else {
    Weapon::collide(other);
  }
}



void Cutter::update() {
  inScope = inScopeNext;
  Weapon::update();
}



void Cutter::take_ammo_from(WeaponP other) {
  // Switch healths.
  
  if (get_health() >= 0 && other->get_health() > get_health()) {
    // Don't need temp variable because setting next variables.
    other->set_health_next(get_health());
    set_health_next(other->get_health());
  }
}



Gun::Gun(const GunContext &g_c,
		 GunXdata &x_data,
		 WorldP w,
		 LocatorP l,
		 const Pos &p)
  : Weapon(g_c.weaponContext,x_data,w,l,p) {
  assert(g_c.ammoInitial <= g_c.ammoMax);
  
  gc = &g_c;
  Timer ntimer(g_c.shotTime); 
  timer = ntimer;
  ammo = g_c.ammoInitial;
}



CONSTRUCTOR_INTERNAL_NODE_IO(Gun,Weapon,weapon) {
  gc = &cx;
  _update_from_stream(in);
}



DEFINE_UPDATE_FROM_STREAM(Gun,Weapon)




void Gun::_update_from_stream(InStreamP in) {
  u_short val = in->read_short();
  if (val == 0xffff) {
    ammo = PH_AMMO_UNLIMITED;
  }
  else {
    ammo = val;
  }
}



int Gun::get_write_length() {
  return 
    Weapon::get_write_length() + 
    sizeof(short); // ammo
}



void Gun::write(OutStreamP out) {
  Weapon::write(out);

  if (ammo == PH_AMMO_UNLIMITED) {
    out->write_short(0xffff);
  }
  else {
    assert(ammo < USHRT_MAX);  // strictly less than to avoid 0xffff
    out->write_short((u_short)ammo);
  }
}



Boolean Gun::is_gun()
{
  return True;
}



Boolean Gun::ready()
{
  return timer.ready();
}



int Gun::get_ammo()
{
	return ammo;
}



int Gun::get_ammo_max()
{
  return gc->ammoMax;
}



void Gun::update()
{
  timer.clock(); 
  Weapon::update();
}



void Gun::set_ammo(int val)
{
  assert(val <= gc->ammoMax && val >= 0);
  ammo = val;
}



Size Gun::get_shot_size(Dir)
{
  return Shell::get_size();
}



void Gun::fire(const Id &id,ITcommand command)
{
  Dir dir = compute_weapon_dir(command);
  _fire(id,dir);
}



void Gun::_fire(const Id &id,Dir dir,Boolean setTimer,Boolean costsAmmo)
/* NOTE: PH_AMMO_UNLIMITED is not currently tested. */
{
  assert((ammo == PH_AMMO_UNLIMITED) ||
         ((ammo <= gc->ammoMax) && (ammo >= 0)));
  
  // Person who fired weapon may not exist anymore. (See FThrower)
  LocatorP locator = get_locator();
  PhysicalP p;
  if (p = locator->lookup(id)) {
    if ((dir != CO_air) && ready() && 
        ((ammo == PH_AMMO_UNLIMITED) || (ammo > 0))) {
      const Area &area = p->get_area();
      
      //	  Pos newPos = area.adjacent_rect(get_shot_size(dir),dir);
      Pos newPos = Coord::shot_initial_pos(area,
             p->is_creature() ? ((CreatureP)p)->get_touching_area() : CO_air,
             get_shot_size(dir),dir);
      WorldP world = get_world();
      LocatorP locator = get_locator();
      
      PhysicalP shot = create_shot(p,world,locator,newPos,dir);
      shot->set_dont_collide(p->get_id());

      // Shot should not collide with any members of the shooter's composite object.
      CompositeP comp = p->get_composite();
      if (comp) {
        shot->set_dont_collide_composite(comp->get_composite_id());
      }

      locator->add(shot);
      if (setTimer)
        timer.set();
      
      if (costsAmmo && ammo != PH_AMMO_UNLIMITED)
        ammo--;
      Weapon::fire(id, dir);
    }
  }  
}



PhysicalP Gun::create_shot(PhysicalP shooter,WorldP world,LocatorP locator,
						   const Pos &pos,Dir dir) {
	PhysicalP shot = new Shell(world,locator,pos,shooter->get_id(),
                             dir);
	assert (shot);
	return shot;
}



SingleGun::SingleGun(const SingleGunContext &c,
					 SingleGunXdata &x_data,
					 WorldP w,LocatorP l,const Pos &p) 
					 : Gun(c.gunContext,x_data,w,l,p) 
{}



CONSTRUCTOR_INTERNAL_NODE_IO(SingleGun,Gun,gun) {
}



Boolean SingleGun::ready() {
  // Can only fire if previous shot has been destroyed.
  LocatorP locator = get_locator();
  
  return (locator->lookup(shotId) == NULL) && Gun::ready();
}



void SingleGun::fire(const Id &id,ITcommand command)
{
  assert(get_ammo() == PH_AMMO_UNLIMITED);
  
  LocatorP locator = get_locator();
  PhysicalP p;
  if (p = locator->lookup(id)) {
    Dir dir = compute_weapon_dir(command);
    if ((dir != CO_air) && ready()) {
      const Area &area = p->get_area();
      
      //	  Pos newPos = area.adjacent_rect(get_shot_size(dir),dir);
      Pos newPos = Coord::shot_initial_pos(area,
            p->is_creature() ? ((CreatureP)p)->get_touching_area() : CO_air,
            get_shot_size(dir),dir);
      WorldP world = get_world();
      
      LocatorP locator = get_locator();
      PhysicalP shot = create_shot(p,world,locator,newPos,dir);
      shot->set_dont_collide(p->get_id());

      // Shot should not collide with any members of the shooter's composite object.
      CompositeP comp = p->get_composite();
      if (comp) {
        shot->set_dont_collide_composite(comp->get_composite_id());
      }

      locator->add(shot);
      
      shotId = shot->get_id();
      Weapon::fire(id, dir);
    }
  }
}



Whistle::Whistle(const WhistleContext &cx,
                 WhistleXdata &xdata,
                 WorldP w,	       
                 LocatorP l,
                 const Pos &raw_pos)
  : Weapon(cx.weaponContext,xdata,w,l,raw_pos) {
  context = &cx;
  ammo = cx.ammo;
  petNum = 0;
}



CONSTRUCTOR_INTERNAL_NODE_IO(Whistle,Weapon,weapon) {
  context = &cx;
  petNum = 0;

  _update_from_stream(in);
}



DEFINE_UPDATE_FROM_STREAM(Whistle,Weapon)



void Whistle::_update_from_stream(InStreamP in) {
  u_short val = in->read_short();
  if (val == 0xffff) {
    ammo = PH_AMMO_UNLIMITED;
  }
  else {
    ammo = val;
  }
}



int Whistle::get_write_length() {
  return 
    Weapon::get_write_length() + 
    sizeof(short); // ammo
}



void Whistle::write(OutStreamP out) {
  Weapon::write(out);

  if (ammo == PH_AMMO_UNLIMITED) {
    out->write_short(0xffff);
  }
  else {
    assert(ammo < USHRT_MAX);  // strictly less than to avoid 0xffff
    out->write_short((u_short)ammo);
  }
}



Boolean Whistle::ready() {
  return True;
}



int Whistle::get_ammo() {
  return ammo;
}



int Whistle::get_ammo_max() {
  return PET_MAX;
}



int Whistle::get_coolness() {
  if (ammo <= 0) {
    return context->coolnessNoAmmo;
  }
  
  return Weapon::get_coolness();
}



Boolean Whistle::useful_no_ammo() {
  return True;
}



Boolean Whistle::fire_repeatedly() {
  return False;
}



void Whistle::fire(const Id &id,ITcommand command) {
  LocatorP locator = get_locator();    
  PhysicalP shooter = locator->lookup(id);
  IntelP shooterIntel = NULL;
  if (shooter) {
    shooterIntel = shooter->get_intel();  
  }
  // Make sure the guy who fired the whistle exists and has an intel,
  // else just give up and return.
  if (!shooter || !shooterIntel) {
    return;
  }
  
  if (ammo > 0 && petNum < PET_MAX) {
    // There is a possibility of an alternate pet only if alternateChance is
    // non-zero.
    Boolean alternatePet = 
      (context->alternateChance ? 
       (Utils::choose(context->alternateChance) == 0) :
       False);
    
    // Create all pets at once.
    int n;
    for (n = 0; n < ammo && petNum < PET_MAX; n++) {
      PetP pet = create_pet(shooterIntel,alternatePet,n);
      pets[petNum] = pet->get_intel_id();
      petNum++;
    }
    set_ammo(0);
    
    // First whistle blow after creating any pets says go get em.
  }    
  // Tell all pets to goGetEm or to return to master.
  // This happens if no ammo to make new pets, or we already have 
  // the max number of pets.
  else {
    LocatorP locator = get_locator();
    int n;
    for (n = 0; n < petNum; n++) {
      PetP pet = (PetP)locator->lookup(pets[n]);
      if (pet) {
        // Give command if pet should go get em, or IT_WEAPON_CENTER to 
        // return to master.    
        pet->go_get_em(shooter,command);
      }
    }
  }
}



void Whistle::act() {
  // Make sure Pet intelligences still exist.  Delete any that are gone.
  LocatorP locator = get_locator();
  int n;
  for (n = 0; n < petNum; ) {
    PetP pet = (PetP)locator->lookup(pets[n]);
    if (pet) {
      // Go to next.
      n++;        
    }
    else {
      pets[n] = pets[petNum - 1]; // copy from last element.
      petNum--;
      // Do not increment n.
    }
  }
  
  // Kill self if out of ammo and all pets are dead.
  if (ammo == 0 && petNum == 0) {
    kill_self();
  }

  Weapon::act();
}



void Whistle::set_ammo(int val) {
  ammo = val;
}



Modifier::Modifier(int timedVal,Boolean head) {
  if (timedVal) {
    timed = True;
    timer.set(timedVal);
  }
  else {
    timed = False;
  }

  next = NULL;
  prev = NULL;
  isHead = head;
}



Modifier::~Modifier() {
  // Can be NULL.
  delete next;
}



ModifierId Modifier::get_modifier_id() {
  return M_None;
}


#if 0
int Modifier::apply(Attribute,int startVal) {
  // Do nothing.
  return startVal;
}
#endif


void Modifier::combine(ModifierP m) {
  assert(get_modifier_id() == m->get_modifier_id());
  if (timed) {
    timer.set(timer.get_remaining() + m->timer.get_remaining());
  }

  delete m;
}


#if 0
void Modifier::prepend(Modifier *m) {
  assert(isHead && !m->next && !m->prev && !prev);
  m->next = next;
  next->prev = m;
  m->prev = this;
  next = m;
}
#endif


void Modifier::append_unique(Modifier *m) {
  assert(isHead && !m->next && !m->prev && !prev);
  ModifierId mId = m->get_modifier_id();

  Modifier *p = this;
  Modifier *beforeP;
  do {
    // If find Modifier of same type.
    if (p->get_modifier_id() == mId) {
      p->combine(m);
      return;
    }

    beforeP = p;
    p = p->next;
  } while (p);

  // Else append to list.
  beforeP->next = m;
  m->prev = beforeP;
}



void Modifier::clear() {
  assert(isHead);
  // Clear out everything after this.
  delete next;
  next = NULL;
}



void Modifier::remove() {
  assert(!isHead);
  if (next) {
    next->prev = prev;
    next = NULL;
  }
  if (prev) {
    prev->next = next;
    prev = NULL;
  }
}
 

#if 0
void Modifier::remove(Modifier *m) {
  assert(isHead && m != this);

  Modifier *p = this;
  while (p->next != m && p->next) {
    p = p->next;
  }
  p->next = m->next;
  m->next = NULL;
}
#endif


#if 0
int Modifier::apply_list(Attribute attr,int startVal) {
  // Feels like 6.001 all over again.
  int nextVal = apply(attr,startVal);
  if (next)
    return next->apply_list(attr,nextVal);
  else 
    return nextVal;
}
#endif



int Modifier::apply(Attribute attr,int val) {
  if (next)
    return next->apply(attr,val);
  else
    return val;
}



void Modifier::clock(PhysicalP p) {
  Boolean deleteThis = False;
  if (timed && timer.ready()) {
    deleteThis = True;
    remove(); // fixup surrounding list
  }

  timer.clock();

  // Call down chain.
  if (next)
    next->clock(p);

  // Only call at end of function.
  if (deleteThis) {
    preDie(p);
    delete this;
  }
}



void Modifier::preDie(PhysicalP) {
}



Doubler::Doubler(int timed)
:Modifier(timed) {
  multiplier = 2.0f;
  increment = 1.0f;
}



void Doubler::combine(ModifierP other) {
  assert(other->get_modifier_id() == get_modifier_id());
  multiplier += increment;
  delete other;
  // Should probably also call Modifier::combine to add the timers.
}



Creature::Creature(const CreatureContext &c_c,
		   CreatureXdata &x_data,
       PtrList *caps,
		   WorldP w,	       
		   LocatorP l,
		   const Pos &raw_pos)
     : Moving(c_c.movingContext,x_data,w,l,raw_pos) {
  touching = CO_air;
  canClimb = False;
  stance = stanceNext = CO_air;
  grav = gravNext = 0;
  cc = &c_c;
  Timer nTimer(CORPSE_TIME);
  corpseTimer = nTimer;
  assert(context_valid());
  birthTime = time(NULL);
  stunNext = 0;
  wantClimb = False;

  // Empty modifier list, mark as the head.
  modifiers = new Modifier(0,True);
  assert(modifiers);

  initialize_abilities(caps);
  check_abilities();
}



Creature::Creature() {
  assert(0);
}



Creature::Creature(InStreamP in,const CreatureContext &cx,
                   CreatureXdata &x_data,
                   WorldP w,LocatorP l)
   : Moving(in,cx.movingContext,x_data,w,l) {
  touching = CO_air;
  canClimb = False;
  stance = stanceNext = CO_air;
  grav = gravNext = 0;
  cc = &cx;
  assert(context_valid());
  birthTime = 0;
  stunNext = 0;
  wantClimb = False;

  // Empty modifier list, mark as the head.
  modifiers = new Modifier(0,True);
  assert(modifiers);

  // Will set all abilities to NULL
  initialize_abilities(NULL);

  // Will read abilities from the stream.
  _update_from_stream(in);

  if (!in->alive()) {
    creation_failed();
    return;
  }

  check_abilities();
}



void Creature::update_from_stream(InStreamP in) {
  Moving::update_from_stream(in);
  _update_from_stream(in);
}



void Creature::_update_from_stream(InStreamP in) {
  ABBitField bitIn = in->read_int();
  ABBitField bitCurrent = compute_abilities_bit_field();

  // Update the existing abilities from the stream.
  if (bitIn == bitCurrent) {
    // Abilities will have been written in order.
    for (int n = 0; n < AB_MAX; n++) {
      if (abilities[n]) {
        abilities[n]->update_from_stream(in);
      }
    }
  }
  // Abilities have changed, recreate all abilities.
  else {
    // Delete all existing ones.
    int n;
    for (n = 0; n < AB_MAX; n++) {
      delete abilities[n];
      abilities[n] = NULL;
    }
    // Read new ones from stream.
    for (n = 0; n < AB_MAX; n++) {
      // If stream provides ability n.
      if (bitIn & (0x1 << n)) {
        abilities[n] = Ability::create_from_stream(in,n,this);
      }
    }         
  }
}



int Creature::get_write_length() {
  // bit field of all the abilities.
  int bitFieldLen = sizeof(ABBitField);

  // size of actual abilities
  int abLen = 0;
  for (int n = 0; n < AB_MAX; n++) {
    if (abilities[n]) {
      abLen += abilities[n]->get_write_length();
    }
  }
  
  return Moving::get_write_length() + bitFieldLen + abLen;
}



void Creature::write(OutStreamP out) {
  Moving::write(out);

  int n;
  ABBitField bitField = compute_abilities_bit_field();
  out->write_int(bitField);

  for (n = 0; n < AB_MAX; n++) {
    if (abilities[n]) {
      abilities[n]->write(out);
    }
  }
}



Creature::~Creature() {
  delete modifiers;

  for (int n = 0; n < AB_MAX; n++) {
    // may be NULL
    delete abilities[n];
  }
}



Boolean Creature::is_creature() {
  return True;
}



ITcommand Creature::get_command() {
  ITcommand mCommand = Moving::get_command();
  if (stunned()) {
    // The only commands allowed when stunned are
    // IT_WEAPON_CHANGE, IT_WEAPON_DROP, IT_ITEM_USE, IT_ITEM_CHANGE, 
    // and IT_ITEM_DROP.  Can still whip out your chainsaw to cut up huggers.
    if (mCommand == IT_WEAPON_CHANGE ||
      mCommand == IT_WEAPON_DROP ||
      mCommand == IT_ITEM_USE ||
      mCommand == IT_ITEM_CHANGE ||
      mCommand == IT_ITEM_DROP) {
      return mCommand;
    }
    else {
      return IT_NO_COMMAND;
    }
  }
  else {
    return mCommand;
  }
}



Boolean Creature::on_door() {
  const Area &area = get_area();
  Loc dest;
  WorldP world = get_world();
  return world->check_door(area.middle_wsquare(),dest);
}



void Creature::get_followers(PtrList &list) {
 for (int n = 0; n < AB_MAX; n++) {
   if (abilities[n]) {
     abilities[n]->get_followers(list);
   }
 }
 Moving::get_followers(list);
}



Boolean Creature::command_repeatable(ITcommand c) {
  // Don't repeat going through doors, or getting up from a crouch.
  if (c == IT_UP && (on_door() || get_stance() == CO_dn)) {
    return False;
  }

  // See if any ability disallows the command repeat.
  for (int n = 0; n < AB_MAX; n++) {
    if (abilities[n]) {
      if (!abilities[n]->command_repeatable(c)) {
        return False;
      }
    }
  }
  
  return Moving::command_repeatable(c);
}



Boolean Creature::standing_squarely() {
  Boolean ret = ((stance == CO_dn || stance == CO_center) &&
                 hanging.corner != CO_dn_R && hanging.corner != CO_dn_L);
  return ret;
}



ClassId Creature::get_weapon_string(const char *&str) {
  // Check all the abilities.
  for (int n = 0; n < AB_MAX; n++) {
    if (abilities[n]) {
      ClassId cId = abilities[n]->get_weapon_string(str);      
      if (cId != A_None) {
        return cId;
      }
    }
  }

  return Moving::get_weapon_string(str);
}



int Creature::get_drawing_level() {
  // Check all the abilities.
  for (int n = 0; n < AB_MAX; n++) {
    if (abilities[n]) {
      int val = abilities[n]->get_drawing_level();      
      if (val != -1) {
        return val;
      }
    }
  }

  return Moving::get_drawing_level();
}




int Creature::get_anim_time() {
  // Check all the abilities.
  for (int n = 0; n < AB_MAX; n++) {
    if (abilities[n]) {
      int val = abilities[n]->get_anim_time();      
      if (val != -1) {
        return val;
      }
    }
  }

  return Moving::get_anim_time();
}



void Creature::follow(const Area &followeeArea,Dir followeeDir,
                      Boolean currentAndNext) {
  // Check all the abilities.
  for (int n = 0; n < AB_MAX; n++) {
    // Go until one returns True.
    if (abilities[n]) {
      if (abilities[n]->follow(followeeArea,followeeDir,currentAndNext)) {
        return;
      }   
    }
  }

  Moving::follow(followeeArea,followeeDir,currentAndNext);
}



Boolean Creature::swap_protect() {
  // Go until one of the abilities returns True.
  for (int n = 0; n < AB_MAX; n++) {
    if (abilities[n]) {
      if (abilities[n]->swap_protect()) {
        return True;
      }
    }
  }

  return Moving::swap_protect();
}



Boolean Creature::frog_protect() {
  // Go until one of the abilities returns True.
  for (int n = 0; n < AB_MAX; n++) {
    if (abilities[n]) {
      if (abilities[n]->frog_protect()) {
        return True;
      }
    }
  }

  return Moving::frog_protect();
}



Boolean Creature::stick_touching(Touching touching) {
  // Does any ability allow touching in the given direction
  for (int n = 0; n < AB_MAX; n++) {
    if (abilities[n]) {
      if (abilities[n]->stick_touching(touching)) {
        return True;
      }
    }
  }

  return (touching == CO_dn);
}



int Creature::compute_droplets_num(int damage) {
  if (damage == 0) {
    return 0;
  }

  // "superficial damage"
  if (damage < 0) {
    damage = -damage;
  }
  
  return Utils::minimum(CREATURE_DROPLETS_MAX,
	      (int)ceil(damage * CREATURE_DROPLET_FACTOR));
}



void Creature::stun_next(int time) {
  stunNext = time;
#if 0
  stunTimer.set(time);

  // Kind of a hack, mucking with 
  set_vel(0);
  set_vel_next(0);
  set_extra_vel_next(0);
  set_stance_next(CO_air);
#endif
}



void Creature::set_intel(IntelP intel) {
  for (int n = 0; n < AB_MAX; n++) {
    if (abilities[n]) {
      abilities[n]->set_intel(intel);
    }
  }

  Moving::set_intel(intel);
}



void Creature::set_quiet_death() {
  // Get rid of corpses.
  if (!alive()) {
    set_delete_me();
  }

  Moving::set_quiet_death();
}



void Creature::heal() {
  for (int n = 0; n < AB_MAX; n++) {
    if (abilities[n]) {
      abilities[n]->heal();
    }
  }

  Moving::heal();
}



void Creature::set_mapped_next(Boolean val) {
  for (int n = 0; n < AB_MAX; n++) {
    if (abilities[n]) {
      abilities[n]->set_mapped_next(val);
    }
  }

  Moving::set_mapped_next(val);
}



void Creature::drop_all(Boolean killNonPersistent) {
  for (int n = 0; n < AB_MAX; n++) {
    if (abilities[n]) {
      abilities[n]->drop_all(killNonPersistent);
    }
  }

  Moving::drop_all(killNonPersistent);
}



Boolean Creature::corporeal_attack(PhysicalP killer,int damage,
                                   AttackFlags flags) {
  assert(damage >= 0);
  
  // Does not take the possible existence of a Protection into account for
  // figuring out killer.

  IntelP kIntel;
  IntelId kIntelId; // Starts out invalid.
  if (killer && (kIntel = killer->get_intel())) {
    kIntelId = kIntel->get_intel_id();
  }
  
  IntelP intel = get_intel();
  if (intel) {
    intel->set_killer_intel_id(kIntelId);
  }

  // Protection from corporeal attacks.
  damage = apply_modifiers(Modifier::CORPOREAL_ATTACK,damage);
  if (damage < 0) {
    // Negative numbers mean superficial damage, i.e. remove the
    // ATT_DAMAGE bit.
    damage = -damage;
    flags = ~(~flags | ATT_DAMAGE);
  }

  // Put in a hook for Ability::corporeal_attack() when we need it.


  if (Moving::corporeal_attack(killer,damage,flags)) {
    // Don't create blood if attack is blocked.
    
    if ((cc->dropletSet != DROPLET_NONE) && (flags & ATT_DROPLETS)) {
      // Create Blood drop.
      LocatorP locator = get_locator();
      const Area &area = get_area();
      Pos middle = area.get_middle();

      for (int n = 0; n < compute_droplets_num(damage); n++) {
        // Choose the specific type of droplet
        ClassId dropletClass = Droplet::choose_droplet_class(cc->dropletSet);
        // dropletClass shouldn't be A_None, since we already checked for
        // an empty dropletSet
        const PhysicalContext* pc = locator->get_context(dropletClass);
        assert(pc); // Or droplet wasn't registered with the Locator.

        Size bloodSize = pc->sizeMax;
        Pos rawPos(middle.x - bloodSize.width / 2,
                    middle.y - bloodSize.height / 2);

        // Blood velocity now choosen in Droplet::create_and_add_if.

        // Try to create blood drop, don't care about return value.
        Droplet::create_and_add_if(dropletClass,get_world(),
                                   locator,rawPos);
      }
    }
    return True;
  }
  return False;
}



void Creature::heat_attack(PhysicalP killer,int heat,Boolean secondary) {
  // Does not take the possible existence of a Protection into account for
  // figuring out killer.

  IntelP kIntel;
  IntelId kIntelId; // Starts out invalid.
  if (killer && (kIntel = killer->get_intel())) {
    kIntelId = kIntel->get_intel_id();
  }
  
  IntelP intel = get_intel();
  if (intel) {
    intel->set_killer_intel_id(kIntelId);
  }

  // Protection from heat attacks.
  //  
  // Not sure whether modifiers should go before or after abilities.
  heat = apply_modifiers(Modifier::HEAT_ATTACK,heat);

  // See if one of the Abilities wants to handle it.
  for (int n = 0; n < AB_MAX; n++) {
    if (abilities[n]) {
      if (abilities[n]->heat_attack(killer,heat,secondary)) {
        return;
      }
    }
  }

  _heat_attack(killer,heat,secondary);
}



void Creature::_heat_attack(PhysicalP killer,int heat,Boolean secondary) {
  Moving::heat_attack(killer,heat,secondary);
}



void Creature::act() {
  // Make all abilities act.
  for (int n = 0; n < AB_MAX; n++) {
    if (abilities[n]) {
      abilities[n]->act();
    }
  }


  WorldP world = get_world();
  Vel velNext = get_vel_next();

  // Account for gravity.  
  // Do we really want to be using gravNext instead of grav??
  if (gravNext > 0) {
    // gravNext gives the acceleration, change in velocity.
    velNext.dy += gravNext; 
  }
  else if (gravNext < 0) {
    // Negative values of grav, -X, really mean the reciprocal, 1/X, i.e.
    // pull down one unit every X turns.
    if (gravTimer.ready()) {
      velNext.dy += 1;
      gravTimer.set(-gravNext);
    }
  }

  // Limit velocities.
  switch (stanceNext) {
  case CO_air:
    velNext.limit((float)apply_modifiers(Modifier::AIR_SPEED,cc->airSpeed,
                  CREATURE_AIR_SPEED_MAX));
  break;
  case CO_center:
    velNext.limit((float)apply_modifiers(Modifier::CENTER_SPEED,cc->centerSpeed,
                  CREATURE_CENTER_SPEED_MAX));
  break;
  case CO_climb:
    velNext.limit((float)apply_modifiers(Modifier::CLIMB_SPEED,cc->climbSpeed,
                  CREATURE_CLIMB_SPEED_MAX));
  break;
  case CO_r:
  case CO_l:
  case CO_up: {
    // Hack, shouldn't put knowledge about Sticky here.
    // Should make a Creature::get_wall_crawl_speed() that defers to the 
    // abilities.  Then Sticky::get_wall_crawl_speed() would return non-zero.
    Sticky* sticky = (Sticky*)get_ability(AB_Sticky);
    // We shouldn't be on the walls if we aren't Sticky.
    // This may change sometime, requiring us to undo this hack.
    assert(sticky);
    Speed wallCrawlSpeed = sticky->get_wall_crawl_speed();
    velNext.limit((float)apply_modifiers(Modifier::CLIMB_SPEED,wallCrawlSpeed,
                  CREATURE_CLIMB_SPEED_MAX));    
  }
  break;
  case CO_dn:
    velNext.limit((float)apply_modifiers(Modifier::CRAWL_SPEED,cc->crawlSpeed,
                  CREATURE_CRAWL_SPEED_MAX));
  break;

  default:
    assert(0);
  }    
  set_vel_next(velNext);
  set_dir_next(compute_dir(stanceNext,velNext));


  // See if we are high.
  if (alive()) {
    int high = apply_modifiers(Modifier::HIGH,0);
    if (high) {
      // vibrate parallel to the wall we are touching, or up/down for ladder.
      const Vel *unitVels = get_unit_vels();
      Dir dir = Coord::parallel_dir(stanceNext);
      if (dir == CO_air) {
        // vibrate in any of sixteen directions.
        dir = Utils::choose(CO_DIR_PURE) + CO_R;
      }
      set_extra_vel_next((float)(CREATURE_HIGH_VIBRATE * 
                                 (Utils::coin_flip() ? 1 : -1)) * // direction or its opposite
                         unitVels[dir]);
    }
  }


  // Move along with Movers.
  MoverP mover = world->lookup(touchingMoverId);
  // Only move along with mover if you stick in the right direction.
  if (mover) {
    if (stick_touching(touching)) {
      set_mover_vel(mover->get_vel());
    }
  }
  
  
  // Check for doors.
  ITcommand command = get_command();
  Loc dest;
  const Area &area = get_area();
  Loc loc = area.middle_wsquare();
  const Pos &rawPos = get_raw_pos();
  // If on a door and command is IT_UP, go through the door.
  if (command == IT_UP && world->check_door(loc,dest))
    {
      Pos p;
      p.x = rawPos.x - loc.c * WSQUARE_WIDTH + dest.c * WSQUARE_WIDTH;
      p.y = rawPos.y - loc.r * WSQUARE_HEIGHT + dest.r * WSQUARE_HEIGHT;
      set_raw_pos_next(p);
    }

  // If marked for wanting to climb and we can do so, then climb.
  if (canClimb && wantClimb) {
    set_stance_next(CO_climb);
    set_vel_next(0);
    set_grav_next(0);
    center_wsquare_x_next(area.middle_wsquare());
    set_want_climb(False);
  }
  
  if (! alive()) {
    if (get_intel()) {
	    cerr << "Warning: Creature::act(): Corpse with non-NULL intelligence."
	         << endl;
    }
    corpseTimer.clock();
  }

  Moving::act();
}



void Creature::update() {
  // update all abilities
  for (int n = 0; n < AB_MAX; n++) {
    if (abilities[n]) {
      abilities[n]->update();
    }
  }

  // Clock Creature's modifiers, then Intel's modifiers.
  // Maybe Intel's modifiers should be clocked in Intel::clock, but for
  // now I'll keep them in the same place.
  modifiers->clock(this);
  IntelP intel = get_intel();
  if (intel) {
    ModifierP iModifiers = intel->get_modifiers();
    iModifiers->clock(this);
  }

  stance = stanceNext;

  // Don't want to muck with gravTimer if gravNext value is just being set
  // to the current value.
  if (grav != gravNext) {
    grav = gravNext;
    // Could have the gravTimer start out ready, and get set on the next 
    // clock.  Only matters if set_grav_next() is called over and over again
    // quickly with different values.
    // If we run into problems, do some subtraction and figure out the 
    // exact value to set for the initial time remaining to compensate for
    // the change in the value of gravNext.
    if (grav < 0) {
      gravTimer.set(-grav);
    }
  }
  gravTimer.clock();

  const Area area = get_area_next();
  
  WorldP world = get_world();
  MoverP touchingMover;
  world->compute_touching_hanging(touching,hanging,touchingMover,area);
  if (touchingMover) {
    touchingMoverId = touchingMover->get_mover_id();
  }
  else {
    touchingMoverId.invalidate();
  }

  canClimb = world->can_climb(area.middle_wsquare());

  // Remove corpse after time limit or if damaged enough.
  if (!get_quiet_death() && !alive() && 
      (corpseTimer.ready() || (get_health() < - cc->corpseHealth))) {
    LocatorP locator = get_locator();
    ostrstream msg;
    if (corpseTimer.ready()) {
	    msg << get_class_name() << " corpse has decomposed." << ends;
	    locator->message_enq(msg.str());
    }
    else {
      msg << get_class_name() << " corpse has been destroyed." << ends;
      locator->message_enq(msg.str());
    }
    set_delete_me();
  }

  if (stunNext) {
    stunTimer.set(stunNext);

    // All before Moving::update.
    set_vel_next(0);
    set_extra_vel_next(0);
    set_stance_next(CO_air);

    stunNext = 0;
  }

  stunTimer.clock();
  alienImmune.clock();

  Moving::update();
}



void Creature::die() {
  // die for all abilities
  for (int n = 0; n < AB_MAX; n++) {
    if (abilities[n]) {
      abilities[n]->die();
    }
  }

  LocatorP locator = get_locator();
  if (!get_quiet_death()) {
    if (cc->deadSoundId) {
      SoundRequest req(cc->deadSoundId,get_area());
      locator->submitSoundRequest(req);
    }
    // Increment Creature stats
    stats.add_death(birthTime);

    // Increment stats for the leaf node.
    const PhysicalContext &pc = cc->movingContext.physicalContext;
    if (pc.get_stats) {
      Stats &leafStats = pc.get_stats(pc.arg);
      leafStats.add_death(birthTime);
    }
  }
  
  // Prepare for being a corpse.
  if (!get_quiet_death()) {    
    corpseTimer.set();
    
    // Set current and next values because called in update phase.
    Vel velNew(0.0f,(float)DROP_SPEED);
    set_vel_next(velNew);
    set_stance_next(CO_air);
    
    ostrstream msg;
    msg << get_class_name() << " has died." << ends;
    locator->message_enq(msg.str());
    
    set_no_death_delete();
  }
  
  
  // Deal with awarding kills.
  IntelP intel = get_intel();
  IntelId killerIntelId;
  if (intel) {
    killerIntelId = intel->get_killer_intel_id();
  }
  IntelP killerIntel;
  if (killerIntel = locator->lookup(killerIntelId)) {
    // Compute soups.  I.e. A human killed by something other than a 
    // different human.
    if (intel && intel->is_human() && 
        (!killerIntel || !killerIntel->is_human() || killerIntel == intel)) {
      intel->add_soup();
    }

    // Award killer with a kill of the appropriate type.  Don't get kills
    // for taking yourself out.
    if (intel && killerIntel && intel != killerIntel) {
      if (intel->is_human()) {
        killerIntel->add_human_kill();
      }
      else if (((MachineP)intel)->is_enemy()) {
        killerIntel->add_enemy_kill();
      }
    }
  }
  else if (intel && intel->is_human()) {
    intel->add_soup();
  }
  
  // We may be changing the next variables because the creature will be a 
  // corpse next turn.
  update_area_next(False);
 
  Moving::die();
}



void Creature::dr_clock() {
  CompositeP comp = get_composite();
  if (comp && !comp->is_leader()) {
    // The leader will take care of calling follow() to make all the
    // others follow.
    return;
  }
  Moving::dr_clock();
}



void Creature::collide(PhysicalP other) {
  // Go until one of the abilities returns True.
  for (int n = 0; n < AB_MAX; n++) {
    if (abilities[n]) {
      if (abilities[n]->collide(other)) {
        return;
      }
    }
  }

  Moving::collide(other);
}



AbilityP Creature::get_ability(AbilityId cId) {
  // Take advantage of abilities being indexed by AbilityId.
  if (abilities[cId]) {
    assert(abilities[cId]->get_ability_id() == cId);
    return abilities[cId];
  }
  return NULL;
}



HolderP Creature::get_holder() {
  for (int n = 0; n < AB_MAX; n++) {
    if (abilities[n] && abilities[n]->is_holder()) {
      return (HolderP)abilities[n];
    }
  }
  return NULL;
}



LocomotionP Creature::get_locomotion() {
  for (int n = 0; n < AB_MAX; n++) {
    if (abilities[n] && abilities[n]->is_locomotion()) {
      return (LocomotionP)abilities[n];
    }
  }
  return NULL;
}



CompositeP Creature::get_composite() {
  for (int n = 0; n < AB_MAX; n++) {
    if (abilities[n] && abilities[n]->is_composite()) {
      return (CompositeP)abilities[n];
    }
  }
  return NULL;
}



void Creature::add_ability(AbilityP a) {
  AbilityId aId = a->get_ability_id();
  assert(!abilities[aId]);

  // Would be better to have this in the constructor, but we
  // have the chicken-and-egg problem when constructing Creatures.
  a->initialize(this);
  abilities[aId] = a;

  // For debugging.
  check_abilities();
}



void Creature::remove_ability(AbilityId aId) {
  assert(abilities[aId]);

  // To make User, Carrier, drop stuff, etc.
  abilities[aId]->die();
  // May be weird for Morphed, will cause Corpse to be dropped.

  delete(abilities[aId]);
  abilities[aId] = NULL;
}



Health Creature::get_health_max() {
  return apply_modifiers(Modifier::HEALTH_MAX,
                         cc->movingContext.physicalContext.health,
                         CREATURE_HEALTH_MAX_MAX);
}



Boolean Creature::generate_masks() {
  // Could check abilities, e.g. Alien doesn't really need masks,
  // but difficult to keep in-sync if abilities change.
  return True;
}



Speed Creature::get_acceleration() {
  return apply_modifiers(Modifier::ACCELERATION,
                         cc->acceleration,
                         CREATURE_ACCELERATION_MAX);
}



Speed Creature::get_jump() {
  return apply_modifiers(Modifier::JUMP,
                         cc->jump,
                         CREATURE_JUMP_MAX);
}



Touching Creature::get_touching_stance() {
  if (stance == CO_center) {
    return  CO_dn;
  }
  else if (stance == CO_climb) {
    return  CO_air;
  }
  else {
    return (Touching)stance;
  }
}



void Creature::center_wsquare_x_next(const Loc &loc) {
  const Area area = get_area();
  const MovingContext *mc = get_moving_context();
  const Pos rawPos = get_raw_pos();
  assert (stanceNext == CO_climb);
  assert (area.overlap(loc));

  Size *offsets = get_offsets();

  Pos rpos(loc.c * WSQUARE_WIDTH + 
	   (WSQUARE_WIDTH - mc->sizes[CO_climb].width) / 2 -
	   offsets[CO_climb].width,
	   rawPos.y);

  set_raw_pos_next(rpos);
}



void Creature::corner(const Hanging &hanging) {
  Pos rpos;
  const MovingContext *mc = get_moving_context();
  WorldP world = get_world();
  Size *offsets = get_offsets();


  ////// Corner around a Loc.
  if (hanging.type == Hanging::LOC) {
    const Loc &loc = hanging.loc;
    switch (hanging.corner) {
    case CO_up_L:  // Final is CO_r_DN.
      assert(stanceNext == CO_r);
      rpos.x = loc.c * WSQUARE_WIDTH - mc->sizes[CO_r].width - 
        offsets[CO_r].width;
      rpos.y = (loc.r + 1) * WSQUARE_HEIGHT - 
        (int)ceil(0.5 * mc->sizes[CO_r].height) - offsets[CO_r].height;
      break;

    case CO_dn_L:  // Final is CO_r_UP.
      assert(stanceNext == CO_r);
      rpos.x = loc.c * WSQUARE_WIDTH - mc->sizes[CO_r].width - 
        offsets[CO_r].width;
      rpos.y = loc.r * WSQUARE_HEIGHT - (int)floor(0.5 * mc->sizes[CO_r].height) -
        offsets[CO_r].height;
      break;
    
    case CO_l_UP:  // Final is CO_dn_R.
      assert(stanceNext == CO_dn);
      rpos.x = (loc.c + 1) * WSQUARE_WIDTH - 
        (int)ceil(0.5 * mc->sizes[CO_dn].width) - offsets[CO_dn].width;
      rpos.y = loc.r * WSQUARE_HEIGHT - mc->sizes[CO_dn].height - 
        offsets[CO_dn].height;
      break;

    case CO_r_UP:  // Final is CO_dn_L.
      assert(stanceNext == CO_dn);
      rpos.x = loc.c * WSQUARE_WIDTH - (int)floor(0.5 * mc->sizes[CO_dn].width) -
        offsets[CO_dn].width;
      rpos.y = loc.r * WSQUARE_HEIGHT - mc->sizes[CO_dn].height - 
        offsets[CO_dn].height;
      break;

    case CO_up_R:  // Final is CO_l_DN.
      assert(stanceNext == CO_l);
      rpos.x = (loc.c + 1) * WSQUARE_WIDTH - offsets[CO_l].width;
      rpos.y = (loc.r + 1) * WSQUARE_HEIGHT - 
        (int)ceil(0.5 * mc->sizes[CO_l].height) - offsets[CO_l].height;
      break;

    case CO_dn_R:  // Final is CO_l_UP.
      assert(stanceNext == CO_l);
      rpos.x = (loc.c + 1) * WSQUARE_WIDTH - offsets[CO_l].width;
      rpos.y = loc.r * WSQUARE_HEIGHT - 
        (int)floor(0.5 * mc->sizes[CO_l].height) - offsets[CO_l].height;
      break;

    case CO_l_DN:  // Final is CO_up_R.
      assert(stanceNext == CO_up);
      rpos.x = (loc.c + 1) * WSQUARE_WIDTH - 
        (int)ceil(0.5 * mc->sizes[CO_up].width) - offsets[CO_up].width;
      rpos.y = (loc.r + 1) * WSQUARE_HEIGHT - offsets[CO_up].height;
      break;

    case CO_r_DN:  // Final is CO_up_L.
      assert(stanceNext == CO_up);
      rpos.x = loc.c * WSQUARE_WIDTH - (int)floor(0.5 * mc->sizes[CO_up].width) -
        offsets[CO_up].width;
      rpos.y = (loc.r + 1) * WSQUARE_HEIGHT - offsets[CO_up].height;
      break;

    default:
      assert(0);
    }
  }

  ////// Corner around a Mover.
  else if (hanging.type == Hanging::MOVER) {
    MoverP mover = world->lookup(hanging.moverId);
    if (!mover) {
      // Mover doesn't exist anymore.
      return;
    }
    const Area &area = mover->get_area();
    Pos pos;
    Size size;
    area.get_rect(pos,size);

    switch (hanging.corner) {
    case CO_up_L:  // Final is CO_r_DN.
      assert(stanceNext == CO_r);
      rpos.x = pos.x - mc->sizes[CO_r].width - 
        offsets[CO_r].width;
      rpos.y = (pos.y + size.height) - 
        (int)ceil(0.5 * mc->sizes[CO_r].height) - offsets[CO_r].height;
      break;

    case CO_dn_L:  // Final is CO_r_UP.
      assert(stanceNext == CO_r);
      rpos.x = pos.x - mc->sizes[CO_r].width - 
        offsets[CO_r].width;
      rpos.y = pos.y - (int)floor(0.5 * mc->sizes[CO_r].height) -
        offsets[CO_r].height;
      break;
    
    case CO_l_UP:  // Final is CO_dn_R.
      assert(stanceNext == CO_dn);
      rpos.x = (pos.x + size.width) - 
        (int)ceil(0.5 * mc->sizes[CO_dn].width) - offsets[CO_dn].width;
      rpos.y = pos.y - mc->sizes[CO_dn].height - 
        offsets[CO_dn].height;
      break;

    case CO_r_UP:  // Final is CO_dn_L.
      assert(stanceNext == CO_dn);
      rpos.x = pos.x - (int)floor(0.5 * mc->sizes[CO_dn].width) -
        offsets[CO_dn].width;
      rpos.y = pos.y - mc->sizes[CO_dn].height - 
        offsets[CO_dn].height;
      break;

    case CO_up_R:  // Final is CO_l_DN.
      assert(stanceNext == CO_l);
      rpos.x = (pos.x + size.width) - offsets[CO_l].width;
      rpos.y = (pos.y + size.height) - 
        (int)ceil(0.5 * mc->sizes[CO_l].height) - offsets[CO_l].height;
      break;

    case CO_dn_R:  // Final is CO_l_UP.
      assert(stanceNext == CO_l);
      rpos.x = (pos.x + size.width) - offsets[CO_l].width;
      rpos.y = pos.y - 
        (int)floor(0.5 * mc->sizes[CO_l].height) - offsets[CO_l].height;
      break;

    case CO_l_DN:  // Final is CO_up_R.
      assert(stanceNext == CO_up);
      rpos.x = (pos.x + size.width) - 
        (int)ceil(0.5 * mc->sizes[CO_up].width) - offsets[CO_up].width;
      rpos.y = (pos.y + size.height) - offsets[CO_up].height;
      break;

    case CO_r_DN:  // Final is CO_up_L.
      assert(stanceNext == CO_up);
      rpos.x = pos.x - (int)floor(0.5 * mc->sizes[CO_up].width) -
        offsets[CO_up].width;
      rpos.y = (pos.y + size.height) - offsets[CO_up].height;
      break;

    default:
      assert(0);
    }
  }
  ////// invalid Hanging::type
  else {
    assert(0);
  }
  
  set_raw_pos_next(rpos);
}



int Creature::apply_modifiers(Attribute attr,int initial,int maximum) {
  // First apply Creature's modifiers
  int ret = modifiers->apply(attr,initial);

  // Then, apply Intel's modifiers.
  IntelP intel = get_intel();
  if (intel) {
    ModifierP iModifiers = intel->get_modifiers();
    ret = iModifiers->apply(attr,ret);
  }

  // limit to maximum if supplied.
  if (maximum != -1) {
    ret = Utils::minimum(ret,maximum);
  }

  return ret;
}



Dir Creature::get_dir_next() {
  return Moving::get_dir_next();
}



void Creature::update_area_next(Boolean currentAndNext) {
  Moving::update_area_next(currentAndNext);
}



void Creature::get_size_offset_next(Size &size,Size &offset,Dir dir) {
  // Give it to the first ability that returns True.
  for (int n = 0; n < AB_MAX; n++) {
    if (abilities[n]) {
      if (abilities[n]->get_size_offset_next(size,offset,dir)) {
        return;
      }
    }
  }

  _get_size_offset_next(size,offset,dir);
}



void Creature::_get_size_offset_next(Size &size,Size &offset,Dir dir) {
  // get_quiet_death() check taken care of in Creature::die.
  if (!alive_next() && !get_quiet_death()) {
    Moving::get_size_offset_next(size,offset,DEAD_DIR);
  }
  else {
    Moving::get_size_offset_next(size,offset,dir);
  }
}



void Creature::get_pixmap_mask(Xvars &xvars,int dpyNum,CMN_IMAGEDATA &pixmap,
                               Dir dir,int animNum) {
  // Give it to the first ability that returns True.
  for (int n = 0; n < AB_MAX; n++) {
    if (abilities[n]) {
      if (abilities[n]->get_pixmap_mask(xvars,dpyNum,pixmap,dir,animNum)) {
        return;
      }
    }
  }

  _get_pixmap_mask(xvars,dpyNum,pixmap,dir,animNum);
}



void Creature::_get_pixmap_mask(Xvars &xvars,int dpyNum,CMN_IMAGEDATA &pixmap,
							                  Dir dir,int animNum) {
  if (!alive()) {
    Moving::get_pixmap_mask(xvars,dpyNum,pixmap,DEAD_DIR,0);
  }
  else {
    Moving::get_pixmap_mask(xvars,dpyNum,pixmap,dir,animNum);
  }
}



void Creature::set_stance_next(const Stance &stance) {
  assert((stance == CO_r) || (stance == CO_dn) || (stance == CO_l) ||
         (stance == CO_up) || (stance == CO_air) || 
         (stance == CO_center) || (stance == CO_climb)); 
  stanceNext = stance; 
}




Dir Creature::compute_dir(const Stance &st,const Vel &v) {
  Dir ret = CO_center;
  
  switch (st) {
  case CO_r:
    if (fabs(v.dy) < VEL_SMALL)
      ret = CO_r;
    else
      ret = v.dy > 0 ? CO_r_DN : CO_r_UP;
    break;
    
  case CO_dn:
    if (fabs(v.dx) < VEL_SMALL)
      ret = CO_dn;
    else
      ret = v.dx > 0 ? CO_dn_R : CO_dn_L;
    break;

  case CO_l:
    if (fabs(v.dy) < VEL_SMALL)
      ret = CO_l;
    else
      ret = v.dy > 0 ? CO_l_DN : CO_l_UP;
    break;
    
  case CO_up:
    if (fabs(v.dx) < VEL_SMALL)
      ret = CO_up;
    else
      ret = v.dx > 0 ? CO_up_R : CO_up_L;
    break;

  case CO_center:
    if (fabs(v.dx) < VEL_SMALL)
      ret = CO_center;
    else
      ret = v.dx > 0 ? CO_center_R : CO_center_L;
    break;

  // If diagonal, will give horizontal direction.
  case CO_air:
    if (fabs(v.dx) < VEL_SMALL) {
      if (fabs(v.dy) < VEL_SMALL) {
	      ret = CO_air;
      }
      else {
        // Changed
        ret = v.dy > 0 ? CO_air_DN : CO_air_UP;
      }
    }
    else {
      ret = v.dx > 0 ? CO_air_R : CO_air_L;
    }
    break;

  // If diagonal, will give horizontal direction.
  case CO_climb:
    if (fabs(v.dx) < VEL_SMALL) {
      if (fabs(v.dy) < VEL_SMALL) {
	      ret = CO_climb;
      }
      else {
	      // Changed
	      ret = v.dy > 0 ? CO_climb_DN : CO_climb_UP;
      }
    }
    else {
      ret = v.dx > 0 ? CO_climb_R : CO_climb_L;
    }
    break;
    
  default:
    assert (0);
    break;
  }

  return ret;
}



Boolean Creature::context_valid() {
  if ((cc->crawlSpeed < 0) || (cc->centerSpeed < 0) || (cc->airSpeed < 0) || 
      (cc->climbSpeed < 0) || (cc->jump < 0) || (cc->acceleration < 0)) {
    return False;
  }
  
  return True;
}



void Creature::initialize_abilities(PtrList *caps) {
  // Initialize abilities list
  int n;
  for (n = 0; n < AB_MAX; n++) {
    abilities[n] = NULL;
  }

  // caps may be NULL if there are no abilities.
  if (caps) {
    // WARNING: Make sure to call Ability::initialize().
    for (n = 0; n < caps->length(); n++) {
      AbilityP c = (AbilityP)caps->get(n);
      AbilityId id = c->get_ability_id();
      assert (id >= 0 && id < AB_MAX);
      // Check for duplicate abilities
      assert(abilities[id] == NULL);
      abilities[id] = c;
      abilities[id]->initialize(this);
    }
    delete caps;
  }
}



void Creature::check_abilities() {
  // Check for some incompatabilities.
  assert(!(abilities[AB_Fighter] && abilities[AB_BuiltIn]));

  // should be one locomotion and <= one holer.
  int locomotions = 0;  
  int holders = 0;
  int n;
  for (n = 0; n < AB_MAX; n++) {
    if (abilities[n]) {
      if (abilities[n]->is_locomotion()) {
        locomotions++;
      }
      if (abilities[n]->is_holder()) {
        holders++;
     }
    }
  }
  assert(locomotions == 1);
  assert(holders <= 1);
}



ABBitField Creature::compute_abilities_bit_field() {
  ABBitField bitField = 0x0;
  for (int n = 0; n < AB_MAX; n++) {
    if (abilities[n]) {
      bitField |= (0x1 << n);
    }
  }

  return bitField;
}



Stats Creature::stats;



Ability::Ability() {
  cre = NULL;
}



Ability::~Ability(){
}



Ability::Ability(InStreamP,CreatureP creat) {
  cre = creat;
}



void Ability::initialize(CreatureP creat) {
  assert(cre == NULL); // Guard against double initialization.
  cre = creat;
}



void Ability::update_from_stream(InStreamP) {
}



int Ability::get_write_length() {
  return 0;
}



void Ability::write(OutStreamP) {
}



Boolean Ability::is_holder() {
  return False;
}



Boolean Ability::is_locomotion() {
  return False;
}



Boolean Ability::is_composite() {
  return False;
}



Boolean Ability::get_pixmap_mask(Xvars &,int,CMN_IMAGEDATA &,Dir,int) {
  return False;
}



Boolean Ability::get_size_offset_next(Size &,Size &,Dir) {
  return False;
}



ClassId Ability::get_weapon_string(const char *&) {
  return A_None;
}



void Ability::drop_all(Boolean) {
}
  


void Ability::get_followers(PtrList &) {
}  



Boolean Ability::follow(const Area &,Dir,Boolean) {
  return False;
}



Boolean Ability::command_repeatable(ITcommand) {
  return True;
}



int Ability::get_drawing_level() {
  // means use default.
  return -1;
}
  


int Ability::get_anim_time() {
  // means use default.
  return -1;
}



Boolean Ability::heat_attack(PhysicalP,int,Boolean) {
  return False;
}



Boolean Ability::swap_protect() {
  return False;
}



Boolean Ability::frog_protect() {
  return False;
}



void Ability::set_intel(IntelP) {
}



void Ability::heal() {
}


  
void Ability::set_mapped_next(Boolean) {
}



void Ability::act() {
}



void Ability::update() {
}



void Ability::die() {
}



Boolean Ability::collide(PhysicalP) {
  return False;
}



Boolean Ability::stick_touching(Touching) {
  return False;
}



void Ability::register_abilities() {
  create_function[AB_Grounded] = Grounded::create_from_stream;
  create_function[AB_Suicide] = Suicide::create_from_stream;
  create_function[AB_Hopping] = Hopping::create_from_stream;
  create_function[AB_User] = User::create_from_stream;
  create_function[AB_Carrier] = Carrier::create_from_stream;
  create_function[AB_Fighter] = Fighter::create_from_stream;
  create_function[AB_Walking] = Walking::create_from_stream;
  create_function[AB_Sticky] = Sticky::create_from_stream;
  create_function[AB_Flying] = Flying::create_from_stream;
  create_function[AB_BuiltIn] = BuiltIn::create_from_stream;
  create_function[AB_Hugger] = Hugger::create_from_stream;
  create_function[AB_Prickly] = Prickly::create_from_stream;
  create_function[AB_Healing] = Healing::create_from_stream;
  create_function[AB_Lifter] = Lifter::create_from_stream;
  create_function[AB_Morphed] = Morphed::create_from_stream;
  create_function[AB_AnimTime] = AnimTime::create_from_stream;
  create_function[AB_OnFire] = OnFire::create_from_stream;
  create_function[AB_SwapProtect] = SwapProtect::create_from_stream;
  create_function[AB_Segmented] = Segmented::create_from_stream;
  create_function[AB_Sensitive] = Sensitive::create_from_stream;

  // Make sure we got 'em all.
  for (int n = 0; n < AB_MAX; n++) {
    assert(create_function[n]);
  }
}



AbilityP Ability::create_from_stream(InStreamP in,AbilityId aId,
                                     CreatureP creat) {
  // Just call the apropriate create function.
  assert(create_function[aId]);
  AbilityP ret = create_function[aId](in,creat);
  assert(ret);
  return ret;
}



AbilityCreateFunctionType Ability::create_function[AB_MAX];



Locomotion::Locomotion() 
: Ability() {
}



Locomotion::Locomotion(InStreamP in,CreatureP c) 
: Ability(in,c) {
}



Boolean Locomotion::is_locomotion() {
  return True;
}



Grounded::Grounded(const GroundedContext &,GroundedXdata &) 
:Locomotion() {
}


Grounded::Grounded(InStreamP in,CreatureP c,
                   const GroundedContext &,
                   GroundedXdata &)
: Locomotion(in,c) {
}



DEFINE_CREATE_ABILITY_FROM_STREAM(Grounded)



GroundedContext Grounded::defaultContext;



GroundedXdata Grounded::defaultXdata;



AbilityId Grounded::get_ability_id() {
  return AB_Grounded;
}



#if 1
#define DO_INCREASE(x) do {if (x < 0) x = 0.0f;} while (0)
#define DO_DECREASE(x) do {if (x > 0) x = 0.0f;} while (0)
#else
#define DO_INCREASE(x)
#define DO_DECREASE(x)
#endif

void Grounded::act() {
  // Methods of Moving or Creature.
  Speed acceleration = cre->get_acceleration();  
  const Stance stance = cre->get_stance();
  const Touching touchingArea = cre->get_touching_area();
  const Touching touchingStance = cre->get_touching_stance();
  Vel vel = cre->get_vel();
  const Acc *unitAccs = cre->get_unit_accs();
  const ITcommand command = cre->get_command(); 
  
  
  if (touchingArea != touchingStance) {
    // Hit the ground.
    if (touchingArea == CO_dn) {
      cre->set_stance_next(CO_center);
      Vel velNew(vel.dx,0.0f);
      cre->set_vel_next(velNew);
    }
    // Hit side walls.
    else if ((touchingArea == CO_r) || (touchingArea == CO_l)) {
      Vel velNew(0.0f,vel.dy);
      cre->set_vel_next(velNew);
    }
    // In the air.
    else if (touchingArea == CO_air) {
      cre->set_stance_next(CO_air);
    }
  }
  else if (touchingArea == CO_dn) {
    Vel velNew(vel.dx,0.0f);
    cre->set_vel_next(velNew);
  }
  
  // Set gravity.
  cre->set_grav_next((touchingArea == CO_dn) ? 0 : PH_GRAVITY);
  
  /* Interpret commands from controlling intelligence.  May override some of 
     above settings. */
  switch (command) {
  case IT_CENTER:
    if (stance == CO_center) {
      cre->set_vel_next(0);
    }
    break;
    
  case IT_R:
    if (stance == CO_center) {
      DO_INCREASE(vel.dx);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_center_R]);
    }
    break;
    
  case IT_L:
    if (stance == CO_center) {
      DO_DECREASE(vel.dx);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_center_L]);
    }
    break;
  }
}



Suicide::Suicide(const SuicideContext &,SuicideXdata &)
:Ability() {
}



DEFINE_CREATE_ABILITY_FROM_STREAM(Suicide)



SuicideContext Suicide::defaultContext;



SuicideXdata Suicide::defaultXdata;



Suicide::Suicide(InStreamP in,CreatureP c,
          const SuicideContext &,
          SuicideXdata &)
: Ability(in,c) {
}



AbilityId Suicide::get_ability_id() {
  return AB_Suicide;
}



void Suicide::act() {
  if (cre->get_command() == IT_ITEM_USE) {
    cre->kill_self();
  }
}



Hopping::Hopping(const HoppingContext &,HoppingXdata &)
:Locomotion() {
}



DEFINE_CREATE_ABILITY_FROM_STREAM(Hopping)



HoppingContext Hopping::defaultContext;



HoppingXdata Hopping::defaultXdata;



Hopping::Hopping(InStreamP in,CreatureP c,
                 const HoppingContext &,
                 HoppingXdata &)
: Locomotion(in,c) {
}



AbilityId Hopping::get_ability_id() {
  return AB_Hopping;
}



void Hopping::act() {
  // Methods of Moving or Creature.
  Speed acceleration = cre->get_acceleration();  
  Speed jump = cre->get_jump();
  const Stance stance = cre->get_stance();
  const Touching touchingArea = cre->get_touching_area();
  const Touching touchingStance = cre->get_touching_stance();
  const Vel vel = cre->get_vel();
  const Vel *unitVels = cre->get_unit_vels();
  const Boolean canClimb = cre->can_climb();
  const Area area = cre->get_area();
  const ITcommand command = cre->get_command(); 
  Speed halfJump = (int)(jump * 0.5);
  
  
  // Default set gravity.
  cre->set_grav_next((touchingArea == CO_dn) || 
		(canClimb && (stance == CO_climb) && cre->alive()) ? 0 : PH_GRAVITY);

  
  // Change stance if necessary.
  if (stance != CO_climb) {
    if (touchingArea != touchingStance) {
      cre->set_want_climb(False);

      // Hit the ground.
      if (touchingArea == CO_dn) {
        cre->set_stance_next(CO_center);
        cre->set_vel_next(0);
      }
      // Side walls.
      else if (touchingArea == CO_r) {
        Vel velNew(0.0f,vel.dy);
        cre->set_vel_next(velNew);
      }
      else if (touchingArea == CO_l) {
        Vel velNew(0.0f,vel.dy);
	      cre->set_vel_next(velNew);
	    }
      else if (touchingArea == CO_air) {
	      cre->set_stance_next(CO_air);
      }
      else if (touchingArea != CO_up) {  
	      assert(0);
      }
	  }
    else if (touchingArea == CO_dn) {
      Vel velNew(vel.dx,0.0f);
      cre->set_vel_next(velNew);
    }
  }
  else {// stance == CO_climb
    // Stop climbing if not in climbing square. 
    if (!canClimb) { 
	    cre->set_stance_next(CO_air);
    }
    else if (! vel.is_zero()) {
	    cre->set_vel_next(0);
    }
  }
  

  /* Interpret commands from controlling intelligence.  May override some of 
     above settings. */
  switch (command) {
  case IT_CENTER:
    if (stance == CO_air) {
      cre->set_want_climb(True);
    }
    if ((stance != CO_climb) && canClimb) {
	    cre->set_stance_next(CO_climb);
	    cre->set_grav_next(0);
	    cre->set_vel_next(0);
	    cre->center_wsquare_x_next(area.middle_wsquare());
    }
    break;
    
  case IT_R:
    if ((stance == CO_center) || (stance == CO_climb)) {
	    cre->set_vel_next((float)jump * unitVels[CO_UP_R_R]);
	    cre->set_stance_next(CO_air);
    }
    break;
    
  case IT_DN_R:
    if (stance == CO_climb) {
	    cre->set_vel_next((float)jump * unitVels[CO_DN_R]);
	    cre->set_stance_next(CO_air);
    }
    else if ((stance == CO_center) || (stance == CO_climb)) {
	    cre->set_vel_next((float)halfJump * unitVels[CO_UP_R_R]);
	    cre->set_stance_next(CO_air);
    }
    break;
    
  case IT_DN:
    if (stance == CO_air) {
      cre->set_want_climb(True);
    }
    if ((stance != CO_climb) && canClimb) {
	    cre->set_stance_next(CO_climb);
	    cre->set_grav_next(0);
	    cre->set_vel_next(0);
	    cre->center_wsquare_x_next(area.middle_wsquare());
    }
    else if (stance == CO_climb) {
      cre->set_vel_next((float)jump * unitVels[CO_climb_DN]);
    }
    break;
    
  case IT_DN_L:
    if (stance == CO_climb) {
	    cre->set_vel_next((float)jump * unitVels[CO_DN_L]);
	    cre->set_stance_next(CO_air);
    }
    else if ((stance == CO_center) || (stance == CO_climb)) {
	    cre->set_vel_next((float)halfJump * unitVels[CO_UP_L_L]);
	    cre->set_stance_next(CO_air);
    }
    break;
    
  case IT_L:
    if ((stance == CO_center) || (stance == CO_climb)) {
	    cre->set_vel_next((float)jump * unitVels[CO_UP_L_L]);
	    cre->set_stance_next(CO_air);
    }
    break;
    
  case IT_UP_L:
    if ((stance == CO_center) || (stance == CO_climb)) {
	    cre->set_vel_next((float)jump * unitVels[CO_air_L]);
	    cre->set_stance_next(CO_air);
    }
    break;
    
  case IT_UP:
    // Doors are handled by Creature.
    if (cre->on_door())
      break;
    if (stance == CO_air) {
      cre->set_want_climb(True);
    }
    if ((stance != CO_climb) && canClimb) {
	    cre->set_stance_next(CO_climb);
	    cre->set_vel_next(0);
	    cre->set_grav_next(0);
	    cre->center_wsquare_x_next(area.middle_wsquare());
    }
    else if (stance == CO_center) {// Jump up.
	    cre->set_vel_next((float)jump * unitVels[CO_air_UP]);
      cre->set_stance_next(CO_air);
    }
    else if (stance == CO_climb) {
      cre->set_vel_next((float)jump * unitVels[CO_climb_UP]);
    }
    break;
    
  case IT_UP_R:
    // Jump right.
    if ((stance == CO_center) || (stance == CO_climb)) {
	    cre->set_vel_next((float)jump * unitVels[CO_air_R]);
	    cre->set_stance_next(CO_air);
    }
    break;
  };
}



Holder::Holder()
: Ability() {
}



Holder::Holder(InStreamP in,CreatureP c) 
: Ability(in,c) {
}



Boolean Holder::is_holder() {
  return True;
}



Boolean Holder::ok_to_hold(PhysicalP other) {
  if (!(cre->alive() && 
        other->is_item() && ((ItemP)other)->can_take(cre))) {
    return False;
  } 

  // Must be a better way than to put explicit knowledge about Lifter here.  
  LifterP lifter = (LifterP)cre->get_ability(AB_Lifter);
  if (lifter && lifter->is_lifting()) {
    return False;
  }

  return True; 
}



User::User(const UserContext &cx,UserXdata &) 
:Holder() {
  weaponsNum = weaponCurrent = itemsNum = itemCurrent = 0; 
  context = &cx;

  Timer dummy(USER_CYCLE_TIME);
  weaponCycleTimer = dummy;
  itemCycleTimer = dummy;
  itemCount = 0;
}



DEFINE_CREATE_ABILITY_FROM_STREAM(User)



UserContext User::defaultContext;



UserXdata User::defaultXdata;



User::User(InStreamP in,CreatureP c,
           const UserContext &cx,
           UserXdata &)
: Holder(in,c) {
  weaponsNum = weaponCurrent = itemsNum = itemCurrent = 0; 
  context = &cx;
  itemCount = 0;

  update_from_stream(in);
}



void User::update_from_stream(InStreamP in) {
  Id id(in);

  LocatorP locator = cre->get_locator();
  PhysicalP p = locator->lookup(id);
  if (p) {
    // Set user to have p as the only weapon, and make it the current weapon.
    if (p->is_weapon()) {
      weaponsNum = 1;
      weapons[0] = id;
      weaponCurrent = 0;
      return;
    }
    
    cerr << "ERROR: User::update_from_stream(), "
         << "should have received a weapon." << endl;
    // Drop through and clear weapons.
  }

  // Clear all weapons.
  weaponsNum = 0;
  weaponCurrent = 0;
}



int User::get_write_length() {
  // The current weapon.
  return Identifier::get_write_length();
}



void User::write(OutStreamP out) {
  // Write the identifier of the current weapon.
  if (weaponCurrent != weaponsNum) {
    weapons[weaponCurrent].write(out);    
  }
  else {
    Id invalid;
    invalid.write(out);
  }
}



AbilityId User::get_ability_id() {
  return AB_User;
}



int User::get_weapons_num() {
  return weaponsNum;
}



int User::get_items_num() {
  return itemsNum;
}



PhysicalP User::get_weapon(int n) {
  assert(n >= 0);
  if (n < weaponsNum) {
    LocatorP locator = cre->get_locator();
    PhysicalP ret;
    if (ret = locator->lookup(weapons[n])) {
      return ret; 
    }
  }

  return NULL;
}



PhysicalP User::get_item(int n) {
  assert(n >= 0);
  if (n < itemsNum) {
    LocatorP locator = cre->get_locator();
    PhysicalP ret;
    if (ret = locator->lookup(items[n])) {
    	return ret;
    }
  }

  return NULL;
}



PhysicalP User::get_weapon_current() {
  assert (weaponCurrent <= weaponsNum);
  if (weaponCurrent != weaponsNum) {
    LocatorP locator = cre->get_locator();
    PhysicalP ret;
    if (ret = locator->lookup(weapons[weaponCurrent])) {
      return ret;
    }
  }

  return NULL;
}



ClassId User::get_weapon_string(const char *&str) {
  PhysicalP p = get_weapon_current();
  if (p) {
    str = p->get_class_name();
    return p->get_class_id();
  }

  return A_None;
}



PhysicalP User::get_item_current() {
  assert (itemCurrent <= itemsNum);
  if (itemCurrent != itemsNum) {
    PhysicalP ret;
    LocatorP locator = cre->get_locator();
    if (ret = locator->lookup(items[itemCurrent])) {
      return ret;
    }
  }

  return NULL;
}



int User::get_item_count() {
  return itemCount;
}



void User::get_followers(PtrList &list) {
  LocatorP locator = cre->get_locator();
  int n;

  // weapons
  for (n = 0; n < weaponsNum; n++) {
    PhysicalP p = locator->lookup(weapons[n]);
    if (p) {
      list.add(p);
    }
  }

  // items
  for (n = 0; n < itemsNum; n++) {
    PhysicalP p = locator->lookup(items[n]);
    if (p) {
      list.add(p);
    }
  }
}



Boolean User::command_repeatable(ITcommand command) {
  // Don't repeat firing a weapon if
  //   1) it is just becoming empty
  //   2) it doesn't allow repeated firing.
  if (Intel::is_command_weapon(command)) {
    if (weaponCurrent != weaponsNum) {
      LocatorP locator = cre->get_locator();
      WeaponP p;
      if (p = (WeaponP)locator->lookup(weapons[weaponCurrent])) {
	      int ammo = p->get_ammo();
        // Is about to become empty, PH_AMMO_UNLIMITED is not affected by this.
        if (ammo == 1 && p->ready()) {
          return False;
        }
        if (!p->fire_repeatedly()) {
          return False;
        }
      }
    }
  }
  return True;
}



void User::set_mapped_next(Boolean val) {
  if (cre->get_mapped_next() != val) {
    LocatorP locator = cre->get_locator();
    PhysicalP weapon;
    if ((weaponCurrent != weaponsNum) && 
        (weapon = locator->lookup(weapons[weaponCurrent]))) {
      if (val) {
	      ((WeaponP)weapon)->enter_scope_next(cre);
      }
	    else {
	      ((WeaponP)weapon)->leave_scope_next(cre);
      }
    }
	}
}



// NOTE: This does not check if the Item persists.  E.g. Bombs will
// be dropped.
void User::drop_all(Boolean killNonPersistent) {
  // Damn, this is easy.
  while (weaponsNum > 0) {
    weapon_drop(killNonPersistent);
  }
  while (itemsNum > 0) {
    item_drop(killNonPersistent);
  }
}



void User::act() {
  assert(weaponCurrent <= weaponsNum);
  assert(itemCurrent <= itemsNum);
  // Make sure that don't have any weapons or items if can't use them.
  assert((context->usesWeapons || weaponsNum == 0) &&
         (context->usesItems || itemsNum == 0));
  
  LocatorP locator = cre->get_locator();
  ITcommand command = cre->get_command();
  
  
  if (Intel::is_command_weapon(command)) {
    // Fire weapon.  
    weapon_use(command);
  }
  else {
    switch (command) {
    case IT_WEAPON_CHANGE:
      weapon_change();
      break;
    case IT_WEAPON_DROP:
      weapon_drop();
      break;
    case IT_ITEM_USE:
      item_use();
      break;
    case IT_ITEM_CHANGE:
      item_change();
      break;
    case IT_ITEM_DROP:
      item_drop();
      break;
    }
  }
  
  assert(weaponCurrent <= weaponsNum);
  assert(itemCurrent <= itemsNum);
  

  /* Go through list of weapons and items, moving them along with the object
     or deleting them if they no longer exist or have no ammo. */

  const Area &areaNext = cre->get_area_next();
  Dir dirNext = cre->get_dir_next();

  // Traverse list of weapons.
  int n;
  for (n = 0; n < weaponsNum;) {
    assert(weaponCurrent <= weaponsNum);
    
    PhysicalP p;
    if (p = locator->lookup(weapons[n])) {
      if (((WeaponP)p)->get_ammo() > 0 || 
	        ((WeaponP)p)->get_ammo() == PH_AMMO_UNLIMITED ||
            ((WeaponP)p)->useful_no_ammo()) {
          //	    ((WeaponP)p)->follow(areaNext,dirNext,False);
      }
      else {
	      p->kill_self();
      }
      n++;
    }
    // p is NULL, clean up the weapon.
    else {
      weapons[n] = weapons[weaponsNum - 1];
      weaponsNum--;

      if (weaponCurrent == weaponsNum && n != weaponsNum) {
        weaponCurrent = n; // Keep same current weapon.
      }
      else if (weaponCurrent == weaponsNum + 1) {
	      weaponCurrent = weaponsNum;  // So still no weapon.
      }
      else if (weaponCurrent == n) {
	      weaponCurrent = coolest_weapon(); // Still some weapon active.
      }

      PhysicalP weapon;
      if ((weaponCurrent != weaponsNum) && 
	        (weapon = locator->lookup(weapons[weaponCurrent]))) {
	      ((WeaponP)weapon)->enter_scope_next(cre);
      }
    }
  }

  
  // Traverse list of items.

  // Count number of current item that User has.  Set itemCountNext.
  // NOTE: This might not be completely accurate, as itemCurrent can change
  // during this traversal.  But, who cares.  It'll be updated next turn.
  itemCountNext = 0;
  ClassId itemCurrentClassId = A_None;
  PhysicalP item;
  if (itemCurrent != itemsNum && 
      (item = locator->lookup(items[itemCurrent]))) {
    itemCurrentClassId = item->get_class_id();
  }

  for (n = 0; n < itemsNum;) {
    assert(itemCurrent <= itemsNum);
    
    PhysicalP p;
    if (p = locator->lookup(items[n])) {
      // Increment itemCountNext.
      if (p->get_class_id() == itemCurrentClassId) {
        itemCountNext++;
      }
      // Make item follow user.
        //  ((ItemP)p)->follow(areaNext,dirNext,False);
      n++;
    }
    // p is NULL
    else {
      items[n] = items[itemsNum - 1];
      itemsNum--;
  
      if (itemCurrent == itemsNum && n != itemsNum) {
        itemCurrent = n; // Keep same current item.
      }
      else if (itemCurrent == itemsNum + 1) {
        itemCurrent = itemsNum;  // So still no item.
      }
      else if (itemCurrent == n) {
        itemCurrent = coolest_item();  // Some item or itemsNum.
      }
    }
  }

  assert(weaponCurrent <= weaponsNum);
  assert(itemCurrent <= itemsNum);
  
  weaponCycleTimer.clock();
  itemCycleTimer.clock();
}



void User::update() {
  itemCount = itemCountNext;
}



Boolean User::collide(PhysicalP other) { 
  if (!ok_to_hold(other)) {
    return False;
  }

  // Used below.
  int itemClassCount = 0;

  // Pick up weapon.
  if (other->is_weapon()) {
	  WeaponP weapon;
    Boolean destroyOther = False;

    // If this User can't use weapons.
    if (!context->usesWeapons) {
      destroyOther = True;
    }
    // If User already has this weapon.
    else if (has_weapon(&weapon,other->get_class_id()))  { 
      // Take ammo from other and kill other.
      // Cutter heals itself to other's health.
      weapon->take_ammo_from((WeaponP)other);
      destroyOther = True;
    }
    // If User is already holding the max number of weapons.
    else if (weaponsNum >= PH_WEAPONS_MAX) {
      ostrstream msg;
      msg << "Can only hold " << PH_WEAPONS_MAX << " weapons" << ends;          
      LocatorP locator = cre->get_locator();
      locator->arena_message_enq(msg.str(),cre);
      destroyOther = True;
    }
    // Actually pick it up.
    else {
	    ((ItemP)other)->taken(cre);
      weapons[weaponsNum] = other->get_id();

      // Increment weaponCurrent along with weaponsNum, so if no weapon was 
      // selected, still no weapon selected.
      if (weaponCurrent == weaponsNum) {
	      weaponCurrent++;
      }
      weaponsNum++;

      if (weaponsNum == 1) { // Used to be 0.
        // Will set it to new weapon or builtIn weapon.
        weaponCurrent = coolest_weapon();
        // If new weapon is selected.
        if (weaponCurrent == 0) {
	        ((WeaponP)other)->enter_scope_next(cre);
        }
      }
      // else don't change weaponCurrent.
    }

    // We are destroying the weapon for some reason.
    if (destroyOther) {
      other->set_quiet_death();
      other->kill_self();
    }  
  } // is weapon

  // Pick up item.
  else if (context->usesItems && 
           itemsNum < PH_ITEMS_MAX &&
		       ((itemClassCount = item_class_count(other->get_class_id())) <
            ITEM_CLASS_MAX)) {
    assert(!other->is_weapon());

    ((ItemP)other)->taken(cre);
    items[itemsNum] = other->get_id();
    itemsNum++;

    // itemCurrent is 0 if no previous item.
  }
  else {
    LocatorP locator = cre->get_locator();
    if (itemsNum == PH_ITEMS_MAX) {
      ostrstream msg;
      msg << "Can only hold " << PH_ITEMS_MAX << " items" << ends;          
      locator->arena_message_enq(msg.str(),cre);
    }
    else if (itemClassCount >= ITEM_CLASS_MAX) {
      ostrstream msg;
      msg << "Can only hold " << ITEM_CLASS_MAX 
        << " of any one item" << ends;
      locator->arena_message_enq(msg.str(),cre);
    }

    other->set_quiet_death();
    other->kill_self();
  }

  // Did something with other.
  return True;
}



void User::die() {
  LocatorP locator = cre->get_locator();

  // Can't just call User::drop_all() because that isn't guaranteed to
  // be safe in the "die" phase.  

  // Drop all weapons.
  int n;     
  for (n = 0; n < weaponsNum; n++) {
    PhysicalP weapon = locator->lookup(weapons[n]);
    if (weapon) {
      if (n == weaponCurrent) {
	      ((WeaponP)weapon)->leave_scope_next(cre);
      }

      // FIXED: Don't drop persistent items if doing a quiet death.
      if (((ItemP)weapon)->persists() && !cre->get_quiet_death()) {
	      ((ItemP)weapon)->dropped(cre);
      }
      else if (!weapon->die_called()) {
        weapon->set_quiet_death();
        weapon->kill_self();
        weapon->die();
      }
	  }
  }
  weaponCurrent = 0;
  weaponsNum = 0;


  // Drop all items.
  itemCurrent = 0;
  for (n = 0; n < itemsNum; n++) {
    PhysicalP item = locator->lookup(items[n]);
    if (item) {
      // FIXED: Don't drop persistent items if doing a quiet death.
      if (((ItemP)item)->persists() && !cre->get_quiet_death()) {
	      ((ItemP)item)->dropped(cre);
      }
      else if (!item->die_called()) {
	      item->set_quiet_death();
	      item->kill_self();
	      item->die();
      }
    }
  }
  itemsNum = 0;
}



Boolean User::has_weapon(Weapon **weaponOut,ClassId classId) {
  for (int n = 0; n < weaponsNum; n++) {
    PhysicalP weapon;
    LocatorP locator = cre->get_locator();

    // Clean up deleted weapons elsewhere.
    if ((weapon = locator->lookup(weapons[n])) &&
    	  (weapon->get_class_id() == classId)) {
	    if (weaponOut) {
	      *weaponOut = (WeaponP)weapon;
      }
  	  return True;
    }
  }
  return False;
}



int User::coolest_weapon() {
  LocatorP locator = cre->get_locator();
  int index = weaponsNum;
  int coolnessMax = 0;

  BuiltInP builtIn = (BuiltInP)cre->get_ability(AB_BuiltIn);
  if (builtIn) {
    coolnessMax = builtIn->get_weapon_coolness();
  }

  // Your builtIn weapon must be defaultable.
  assert(coolnessMax >= 0);

  int n;
  for (n = 0; n < weaponsNum; n++) {
    WeaponP weapon = (WeaponP)locator->lookup(weapons[n]);
    if (weapon && 
        weapon->get_coolness() > coolnessMax &&
        // Only weapons of nonnegative coolness.
        weapon->get_coolness() >= 0) {
      coolnessMax = weapon->get_coolness();
      index = n;
    }
  }
  
  return index;
}



/* Mostly copied from User::coolest_weapon. */
int User::coolest_item() {
  LocatorP locator = cre->get_locator();
  int index = itemsNum;
  int coolnessMax = 0;

  // Your builtIn item must be defaultable.
  assert(coolnessMax >= 0);

  int n;
  for (n = 0; n < itemsNum; n++) {
    ItemP item = (ItemP)locator->lookup(items[n]);
    if (item && 
        item->get_coolness() > coolnessMax &&
        // Only items of nonnegative coolness.
        item->get_coolness() >= 0) {
      coolnessMax = item->get_coolness();
      index = n;
    }
  }
  
  return index;
}



int User::next_coolest_weapon() {
  LocatorP locator = cre->get_locator();

  // Find the coolness of having weapon set to "none".
  BuiltInP builtIn = (BuiltInP)cre->get_ability(AB_BuiltIn);
  int noneCoolness = 0;
  if (builtIn) {
    noneCoolness = builtIn->get_weapon_coolness();
  } 

  // Find coolness of current weapon or of builtIn weapon.
  int currentCoolness = 0;
  if (weaponCurrent != weaponsNum) {
    WeaponP weapon = (WeaponP)locator->lookup(weapons[weaponCurrent]);
    if (weapon) {
      currentCoolness = weapon->get_coolness();
    }
    else {
      // Somehow, the current weapon got destroyed, just return coolest
      // of all weapons.
      return coolest_weapon();
    }
  }
  // Current weapon is none.
  else {
    currentCoolness = noneCoolness;
  }


  // Look for weapon of maximum coolness less than currentCoolness.
  Boolean found = False;
  int maxx;
  int index;
  int n;
  for (n = 0; n < weaponsNum; n++) {
    WeaponP weapon = (WeaponP)locator->lookup(weapons[n]);
    if (weapon && 
	      (!found || weapon->get_coolness() > maxx) &&
	      weapon->get_coolness() < currentCoolness) {
	    maxx = weapon->get_coolness();
	    index = n;
	    found = True;
    }
  }

  // See if setting weapon to none is the best fit.
  if ((noneCoolness < currentCoolness) &&
      (!found || noneCoolness > maxx)) {
    maxx = noneCoolness;  // unnecessary.
    index = weaponsNum;
    found = True;
  }
  
  if (!found) {
    // Cycle around.
    return coolest_weapon();
  }
  else {
    return index;
  }
}



/*  Mostly copied from User::next_coolest_weapon. */
int User::next_coolest_item() {
  LocatorP locator = cre->get_locator();

  // Find coolness of current item.
  int currentCoolness;
  if (itemCurrent != itemsNum) {
    ItemP item = (ItemP)locator->lookup(items[itemCurrent]);
    if (item) {
      currentCoolness = item->get_coolness();
    }
    else {
      // Somehow, the current item got destroyed, just return coolest
      // of all items.
      return coolest_item();
    }
  }
  else {
    currentCoolness = 0;
  }


  // Look for item of maximum coolness less than currentCoolness.
  Boolean found = False;
  int maxx;
  int index;
  int n;
  for (n = 0; n < itemsNum; n++) {
    ItemP item = (ItemP)locator->lookup(items[n]);
    if (item && 
        (!found || item->get_coolness() > maxx) &&
        (item->get_coolness() < currentCoolness)) {
      maxx = item->get_coolness();
      index = n;
      found = True;
    }
  }

  if (!found) {
    // Cycle around.
    return coolest_item();
  }
  else {
    return index;
  }
}



void User::weapon_use(ITcommand command) {
  if (weaponCurrent != weaponsNum) {
    LocatorP locator = cre->get_locator();
    PhysicalP p;
    if (p = locator->lookup(weapons[weaponCurrent])) {
      ((WeaponP)p)->fire(cre->get_id(),command);

      // Inform Creature that an attack occurred.
      cre->attack_hook();
  	}
  }
}



void User::weapon_change() {
  LocatorP locator = cre->get_locator();

  PhysicalP weapon = NULL;
  if ((weaponCurrent != weaponsNum) && 
      (weapon = locator->lookup(weapons[weaponCurrent]))) {
    ((WeaponP)weapon)->leave_scope_next(cre);
  }

  int coolest = coolest_weapon();
  // If its been awhile since the last weapon change AND
  // we're not already on the coolest weapon,
  if (weaponCycleTimer.ready() && weaponCurrent != coolest) {
    weaponCurrent = coolest;
  }
  else {
    // If we changed weapons recently, cycle to the next coolest weapon.
    weaponCurrent = next_coolest_weapon();
  }
  
  // weapon was just changed
  weaponCycleTimer.set();
  
  if ((weaponCurrent != weaponsNum) && 
      (weapon = locator->lookup(weapons[weaponCurrent]))) {
    ((WeaponP)weapon)->enter_scope_next(cre);
    weapon->flash();
  }
}



void User::weapon_drop(Boolean killNonPersistent) {
  LocatorP locator = cre->get_locator();

  if (weaponCurrent != weaponsNum) { // Some weapon is current.
    // Clean up dead weapons elsewhere.

    PhysicalP weapon;
    if (weapon = locator->lookup(weapons[weaponCurrent])) {
	    ((WeaponP)weapon)->leave_scope_next(cre);

      // kill off non-persistent objects, like Bomb.
      if (killNonPersistent && !((ItemP)weapon)->persists()) {      
        weapon->set_quiet_death();
        weapon->kill_self();
      }
      // Regular drop
      else {
	      ((ItemP)weapon)->dropped(cre);
      }

	    weaponsNum--;
	    weapons[weaponCurrent] = weapons[weaponsNum];
	    
	    // Some weapon or weaponCurrent == weaponsNum.
	    weaponCurrent = coolest_weapon();
	  
      PhysicalP weapon;
      if ((weaponCurrent != weaponsNum) &&
	        (weapon = locator->lookup(weapons[weaponCurrent]))) {
	      ((WeaponP)weapon)->enter_scope_next(cre);
      }
    }
    // BUG, can get into an infinite loop here from User::drop_all
  }
  else { // No current weapon.
    /* Cycle to next available weapon, so can keep dropping. */
    weapon_change();
  }
}



void User::item_use() {
  if (itemCurrent != itemsNum) {
    LocatorP locator = cre->get_locator();
    PhysicalP p;
    if (p = locator->lookup(items[itemCurrent])) {
      ((ItemP)p)->use(cre);
    }
  }
}



void User::item_change() {
  int coolest = coolest_item();
  // If its been awhile since the last item change AND
  // we're not already on the coolest item,
  if (itemCycleTimer.ready() && itemCurrent != coolest) {
    itemCurrent = coolest;
  }
  else {
    // If we changed items recently, cycle to the next coolest item.
    itemCurrent = next_coolest_item();
  }
  
  // item was just changed
  itemCycleTimer.set();

  PhysicalP p;
  LocatorP locator = cre->get_locator();
  if ((itemCurrent != itemsNum) &&
      (p = locator->lookup(items[itemCurrent]))) {
    p->flash();
  }
}



void User::item_drop(Boolean killNonPersistent) {
  LocatorP locator = cre->get_locator();

  if (itemCurrent != itemsNum) { // Some item is current.
    // Clean up dead items elsewhere.

    PhysicalP item;
    if (item = locator->lookup(items[itemCurrent])) {

      // kill off non-persistent objects, like Bomb.
      if (killNonPersistent && !((ItemP)item)->persists()) {      
        item->set_quiet_death();
        item->kill_self();
      }
      // Regular drop
      else {
        ((ItemP)item)->dropped(cre);
      }

      itemsNum--;
      items[itemCurrent] = items[itemsNum];
	  
      // Some item current.
      itemCurrent = coolest_item();
    }
  }
  else { // No current item.
    /* Cycle to next available item, so can keep dropping. */
    item_change();
  }
}



int User::item_class_count(ClassId cId) {
  LocatorP l = cre->get_locator();
  int ret = 0;
  for (int n = 0; n < itemsNum; n++) {
    PhysicalP p = l->lookup(items[n]);
    if (p && p->get_class_id() == cId) {
      ret++;
    }
  }
  return ret;
}


Carrier::Carrier(const CarrierContext &,CarrierXdata &) 
:Holder() {
  hasItem = False;
}



DEFINE_CREATE_ABILITY_FROM_STREAM(Carrier)



CarrierContext Carrier::defaultContext;



CarrierXdata Carrier::defaultXdata;



Carrier::Carrier(InStreamP in,CreatureP c,
                 const CarrierContext &,
                 CarrierXdata &)
: Holder(in,c) {
  hasItem = False;
}



AbilityId Carrier::get_ability_id() {
  return AB_Carrier;
}



int Carrier::get_weapons_num() {
  return 0;
}



int Carrier::get_items_num() {
  return hasItem ? 1 : 0;
}



PhysicalP Carrier::get_weapon(int) {
  return NULL;
}



PhysicalP Carrier::get_weapon_current() {
  return NULL;
}



PhysicalP Carrier::get_item(int n) {
  assert(n >= 0); 
  if (n == 0 && hasItem) {
    LocatorP locator = cre->get_locator();
    PhysicalP ret = locator->lookup(item);
    return ret;
  }
  return NULL;
}



PhysicalP Carrier::get_item_current() {
  // Returns NULL if !has_item(), so does the right thing.
  return get_item(0);
}



int Carrier::get_item_count() {
  return hasItem ? 1 : 0;
}



void Carrier::get_followers(PtrList &list) {
  if (hasItem) {
    LocatorP l = cre->get_locator();
    PhysicalP p = l->lookup(item);    
    if (p) {
      list.add(p);
    }
  }
}



void Carrier::drop_all(Boolean killNonPersistent) {
  if (!hasItem) {
    return;
  }

  LocatorP l = cre->get_locator();
  PhysicalP p = l->lookup(item);
  if (!p) {
    return;
  }

  if (killNonPersistent && !((ItemP)p)->persists()) {
    p->set_quiet_death(); 
    p->kill_self();
  }
  else {
    ((ItemP)p)->dropped(cre);
  }
  hasItem = False;
}			



void Carrier::act() {
  LocatorP locator = cre->get_locator();
  ITcommand command = cre->get_command();
  PhysicalP p = NULL;
  if ( hasItem ) {
    p = locator->lookup(item);
    if (p) {
      assert(p->is_item());
    }
  }

  switch (command) {
  case IT_ITEM_USE:
    // Can only use AutoUse items.
    if (p) {
      if (p->is_auto_use()) {
        ((ItemP)p)->use(cre);
      }
      else {
        ostrstream str;
        str << "Cannot use " << p->get_class_name() << ends;
        locator->arena_message_enq(str.str(),cre);
      }
    }
    break;        
  case IT_ITEM_CHANGE:
    if (p) {
      p->flash();
    }
    break;
  case IT_ITEM_DROP:
    if (p) {
      ((ItemP)p)->dropped(cre);
      hasItem = False;
    }
  }

  if (p) {
#if 0
    const Area &areaNext = get_area_next();
    Dir dirNext = get_dir_next();
    // Make item follow user.
    ((ItemP)p)->follow(areaNext,dirNext,False);
#endif
  }
  else {
  // If item has been destroyed.
    hasItem = False;
  }
}



Boolean Carrier::collide(PhysicalP other) {
  if (!ok_to_hold(other)) {
    return False;
  } 

  if (hasItem) {
    return False;
  }

  // Both weapons and items are put in the item slot.
  ((ItemP)other)->taken(cre);    
  item = other->get_id();
  hasItem = True;
  return True;
}



void Carrier::die() {
  if (hasItem) {
    LocatorP l = cre->get_locator();
    PhysicalP p = l->lookup(item);    
    if (p) {
      // FIXED: Don't drop persistent items if doing a quiet death.
      // Copied this fix from User, haven't tested it too much.
      if (((ItemP)p)->persists() && !cre->get_quiet_death()) {
	      ((ItemP)p)->dropped(cre);
      }
	    else if (!p->die_called()) {
        p->set_quiet_death();
        p->kill_self();
        p->die();
      }
	  }
  }
}



Fighter::Fighter(const FighterContext &f_c,FighterXdata &) 
:Ability() {
  fc = &f_c;
  attack = attackNext = attackNone;
}



Fighter::Fighter(InStreamP in,CreatureP c,
                 const FighterContext &cx,
                 FighterXdata &)
: Ability(in,c) {
  fc = &cx;
  update_from_stream(in);
}



void Fighter::update_from_stream(InStreamP in) {
  char attk = in->read_char();
  attack = (Attack)attk;
  attackNext = attack;
}



int Fighter::get_write_length() {
  return sizeof(char);
}



void Fighter::write(OutStreamP out) {
  assert(attack == attackNext);
  out->write_char((char)attack);
}



// Right now, this is the only Ability that needs special code, 
// the rest use DEFINE_CREATE_ABILITY_FROM_STREAM. (also AnimTime)
/////  Not true anymore??  Get rid of this?
AbilityP Fighter::create_from_stream(InStreamP in,CreatureP creat) {
  ClassId cId = creat->get_class_id();
  AbilityP ret = new Fighter(in,creat,*lookup_context(cId),defaultXdata);
  assert(ret);
  return ret;
}



void Fighter::initialize(CreatureP c) {
  Ability::initialize(c);

  // Just so we will assert if we forgot to register the Fighter.
  lookup_context(cre->get_class_id());

  // If you assert here, check your ".bitmaps" file.
  // MovingContext::offsetGenerator should be set to Fighter::offset_generator
  // for all Fighters.  See "ninja.bitmaps".
  assert(cre->get_offset_generator() == Fighter::offset_generator);
  // Similar to above.  Should be Fighter::transformOverride for all Fighters.
  assert(cre->get_transform_override() == &Fighter::transformOverride ||
         cre->get_transform_override() == &Fighter::uprightTransformOverride);
}



AbilityId Fighter::get_ability_id() {
  return AB_Fighter;
}



Boolean Fighter::get_pixmap_mask(Xvars &xvars,int dpyNum,CMN_IMAGEDATA &pixmap,
                                 Dir dir,int) {
  // Alive check is somewhat of a hack.
  if (cre->alive() && attack && dir_to_attack(dir)) {
    // Just map the dir to the attackDir used to store the
    // fighter pixmaps.
    Dir attackDir = dir_to_attack_dir(dir);
    cre->get_pixmap_mask(xvars,dpyNum,pixmap,attackDir,0);
    return True;
  }

  return False;
}



Boolean Fighter::get_size_offset_next(Size &size,Size &offset,
                                   Dir dirNext) {
  // Alive check is somewhat of a hack.
  if (cre->alive_next() && attackNext && dir_to_attack(dirNext)) {
    // Just map the dir to the attackDir used to store the
    // fighter pixmaps.
    Dir attackDirNext = dir_to_attack_dir(dirNext);
    cre->get_size_offset_next(size,offset,attackDirNext);
    return True;
  }

  return False;
}


			     
ClassId Fighter::get_weapon_string(const char *&str) {
  // If we are a Holder with a current weapon, let Holder take
  // care of it.
  HolderP holder = cre->get_holder();
  if (holder && holder->get_weapon_current()) {
    return A_None;
  }

  // If a Lifter lifting something, let lifter take care of it.
  LifterP lifter = (LifterP)cre->get_ability(AB_Lifter);
  if (lifter && lifter->is_lifting()) {
    return A_None;
  }

  // Set to hand-to-hand
  str = "hand-to-hand";  
  return A_HandToHand;
}



void Fighter::act() {
  ITcommand command = cre->get_command();
  const Dir dir = cre->get_dir();

  // No longer attacking, e.g. hit the ground after a flying attack.
  if (attack && (attack != dir_to_attack(dir))) {
    attackNext = attackNone;
  }

  // Finished sliding along the ground for a stuck attack.
  if (attack == attackStuck && timer.ready()) {
    attackNext = attackNone;
    cre->set_vel_next(0);
  }
  // Only need to time-out for an attackFree if the Creature is Flying,
  // anything else will just hit the ground in its own time.
  if (attack == attackFree && timer.ready() &&
      cre->get_ability(AB_Flying)) {
    attackNext = attackNone;
    // Don't zero velocity.
  }
  timer.clock();

  // If we have a current weapon, don't attack.
  HolderP holder = cre->get_holder();
  if (holder && holder->get_weapon_current()) {
    return;
  }

  // If a Lifter lifting something, don't attack.
  LifterP lifter = (LifterP)cre->get_ability(AB_Lifter);
  if (lifter && lifter->is_lifting()) {
    return;
  }

  // Start new attack.
  if (!attack && Intel::is_command_weapon(command)) {
    Boolean isFlying = (cre->get_ability(AB_Flying) != NULL);
    Boolean didAttack = True;
    Dir attackDir;

    // Choose attack depending on stance.
    switch (cre->get_stance()) {
    case CO_center:
      switch (command) {
      case IT_WEAPON_R:
        attack_stuck(CO_center_R,CO_center);
        break;
      case IT_WEAPON_DN_R:
        attack_stuck(CO_dn_R,CO_dn);
        break;
      case IT_WEAPON_DN_L:
        attack_stuck(CO_dn_L,CO_dn);
        break;
      case IT_WEAPON_L:
        attack_stuck(CO_center_L,CO_center);
        break;
      case IT_WEAPON_UP_L:
        attack_free_horizontal(CO_UP_L);  
        break;
      case IT_WEAPON_UP:
        attack_free_vertical(CO_air_UP);
        break;
      case IT_WEAPON_UP_R:
        attack_free_horizontal(CO_UP_R); 
        break;
      default:
        didAttack = False;
      }
      break;

    case CO_air: 
      // Non-flying creatures can only do a head stomp while in the air.
      if (command == IT_WEAPON_DN) {
        attack_free_vertical(CO_DN);
      }
      // Flying creatures can attack in any direction while in the air.
      // For UP and DN, zero the horizontal velocity.
      else if (isFlying && command == IT_WEAPON_UP) {
        attack_free_vertical(CO_UP);
      }
      // For attacks other than UP and DN, keep the horizontal velocity.
      else if (isFlying && 
               ((attackDir = Intel::command_weapon_to_dir_8(command)) 
                != CO_air)) {
        attack_free(attackDir);
      }
      else {
        didAttack = False;
      }
      break;
        
    case CO_r:
      switch (command) {
      case IT_WEAPON_L:
      case IT_WEAPON_DN_L:
        attack_free_horizontal(CO_center_L);
        break;
      case IT_WEAPON_UP_L:
        attack_free_horizontal(CO_UP_L);
        break;
      case IT_WEAPON_UP:
      case IT_WEAPON_UP_R:
        attack_stuck(CO_r_UP,CO_r);
        break;
      case IT_WEAPON_DN:
      case IT_WEAPON_DN_R:
        attack_stuck(CO_r_DN,CO_r);
        break;
      default:
        didAttack = False;
      }
      break;

    case CO_dn:
      switch (command) {
      case IT_WEAPON_UP_R:
        attack_free_horizontal(CO_UP_R);
        break;
      case IT_WEAPON_UP_L:
        attack_free_horizontal(CO_UP_L);
        break;
      case IT_WEAPON_UP:
        attack_free_vertical(CO_air_UP);
        break;
      case IT_WEAPON_R:
      case IT_WEAPON_DN_R:
        attack_stuck(CO_dn_R,CO_dn);
        break;
      case IT_WEAPON_L:
      case IT_WEAPON_DN_L:
        attack_stuck(CO_dn_L,CO_dn);
        break;
      default:
        didAttack = False;
      }
      break;

    case CO_l:
      switch (command) {
      case IT_WEAPON_R:
      case IT_WEAPON_DN_R:
        attack_free_horizontal(CO_center_R);
        break;
      case IT_WEAPON_UP_R:
        attack_free_horizontal(CO_UP_R);
        break;
      case IT_WEAPON_UP:
      case IT_WEAPON_UP_L:
        attack_stuck(CO_l_UP,CO_l);
        break;
      case IT_WEAPON_DN:
      case IT_WEAPON_DN_L:
        attack_stuck(CO_l_DN,CO_l);
        break;
      default:
        didAttack = False;
      }
      break;

    case CO_up:
      switch (command) {
      case IT_WEAPON_DN:
        attack_free_vertical(CO_air_DN);
        break;
      case IT_WEAPON_R:
      case IT_WEAPON_DN_R:
      case IT_WEAPON_UP_R:
        attack_stuck(CO_up_R,CO_up);
        break;
      case IT_WEAPON_L:
      case IT_WEAPON_DN_L:
      case IT_WEAPON_UP_L:
        attack_stuck(CO_up_L,CO_up);
        break;
      default:
        didAttack = False;
      }
      break;

    case CO_climb:
      switch (command) {
      case IT_WEAPON_R:
      case IT_WEAPON_DN_R:
        attack_free_horizontal(CO_center_R);
        break;
      case IT_WEAPON_DN:
        attack_free_vertical(CO_air_DN);
        break;
      case IT_WEAPON_L:
      case IT_WEAPON_DN_L:
        attack_free_horizontal(CO_center_L);
        break;
      case IT_WEAPON_UP_L:
        attack_free_horizontal(CO_UP_L);
        break;
      case IT_WEAPON_UP:
        attack_free_vertical(CO_air_UP);
        break;
      case IT_WEAPON_UP_R:
        attack_free_horizontal(CO_UP_R);
        break;
      default:
        didAttack = False;
      }
      break;

    default:
      didAttack = False;
      break;
    }

    // Inform Creature that an attack occurred.
    if (didAttack) {
      SoundRequest req(fc->attackSound,cre->get_area());
      LocatorP locator=cre->get_locator();
      locator->submitSoundRequest(req);
      cre->attack_hook();
    }
  }  // if starting a new attack.

  // Careful about adding logic here, there is a return up above.
}



void Fighter::update() {
  attack = attackNext;
}



Boolean Fighter::collide(PhysicalP other) {
  if (attack) {
    // Special case so Pets  with same master don't hurt each other.
    //// This is unnecessary with the new Teams code.
    IntelP intel = cre->get_intel();
    IntelP oIntel = other->get_intel();
    if (intel && oIntel && 
  	  !intel->is_human() && !oIntel->is_human() && 
	    ((MachineP)intel)->get_master_intel_id() == 
	    ((MachineP)oIntel)->get_master_intel_id()) {
	    return False;
    }

    Dir dir = cre->get_dir();
    const Area &area = cre->get_area();
    Pos pos;
    Size size;
    area.get_rect(pos,size);

    const Area &otherArea = other->get_area();

    // Compute the Attack from the current dir, don't just use current value
    // of this->attack.
    Attack attackFromDir = dir_to_attack(dir);

    if (attackFromDir) {
      // If there is no hotSpot specified for the current direction, generate 
      // one by
      // transforming from the hotSpot of the base for the direction.
      //
      // This is not as clean as I'd like:
      // 1) We are directly looking at MovingContext
      // 2) This scheme will mess up if any creature legitimately has a 
      // hotSpot of (0,0).
      //
      // Hot Spots are indexed in FighterContext by the non-attacking dir.
      Size hotSpot = fc->hotSpots[dir];
      if (hotSpot.width == 0 && hotSpot.height == 0) {
        // Get the base direction for dir.
        const MovingContext *mc = cre->get_moving_context();
        Dir base = Transform2D::get_base(dir,mc->transformOverride);

        // Convert from Size to Pos so we can use Transform2D's methods.
        Pos newHotSpot(fc->hotSpots[base].width,fc->hotSpots[base].height);
        // The base must have a valid hot spot.
        assert(!(newHotSpot.x == 0 && newHotSpot.y == 0));

        // Get the size of the base.
        Dir attackBase = dir_to_attack_dir(base);
        const Size &baseSize = mc->sizes[attackBase];

        // Transform the hot spot of the base.
        newHotSpot = Transform2D::apply_all(dir,newHotSpot,
                                            baseSize,mc->transformOverride);

        // Convert back from Pos to Size.
        hotSpot.width = newHotSpot.x;
        hotSpot.height = newHotSpot.y;
      }


      // If the hotSpot is inside other's area, then do some damage.
      if (otherArea.overlap(pos + hotSpot)){
        // Move just enough to get out of the way.
        Size offset = area.avoid(otherArea);
        const Pos &rawPos = cre->get_raw_pos();
        cre->set_raw_pos_next(rawPos + offset);

        int damage = (attackFromDir == attackStuck) ? fc->damageStuck : 
                                                      fc->damageFree;
        damage = cre->apply_modifiers(Modifier::DAMAGE,damage,
                                      FIGHTER_DAMAGE_MAX);
        other->corporeal_attack(cre,damage);
        attackNext = attackNone;
        return True;
      }
    } // attackFromDir
  }
  
  return False;
}



Attack Fighter::dir_to_attack(Dir dir) {
  switch (dir) {
  case CO_center_R:
  case CO_center_L:
  case CO_r_DN:
  case CO_r_UP:
  case CO_dn_R:
  case CO_dn_L:
  case CO_l_DN:
  case CO_l_UP:
  case CO_up_R:
  case CO_up_L:
    return attackStuck;

  case CO_air_R:
  case CO_air_DN: 
  case CO_air_L:
  case CO_air_UP:
    return attackFree;
  }

  return attackNone;
}



Dir Fighter::dir_to_attack_dir(Dir dir) {
  switch (dir) {
  case CO_center_R:
    return CO_DN_R_R;

  case CO_center_L:
    return CO_DN_R;

  case CO_air_R:
    return CO_DN_DN_R;

  case CO_air_L:
    return CO_DN;

  case CO_r_DN:
    return CO_DN_DN_L;

  case CO_r_UP:
    return CO_DN_L;

  case CO_dn_R:
    return CO_DN_L_L;

  case CO_dn_L:
    return CO_L;

  case CO_l_DN:
    return CO_UP_L_L;

  case CO_l_UP:
    return CO_UP_L;

  case CO_up_R:
    return CO_UP_UP_L;

  case CO_up_L:
    return CO_UP;

  case CO_air_UP:
    return CO_UP_UP_R;

  case CO_air_DN: 
    return CO_UP_R;


  default:
    assert(0);
    return CO_air;
  }
}



Boolean Fighter::is_attack_dir(Dir dir) {
  return dir >= CO_DN_R_R && dir <= CO_UP_R;
}



Dir Fighter::attack_dir_to_dir(Dir dir) {
  switch (dir) {
  case CO_DN_R_R:
    return CO_center_R;

  case CO_DN_R:
    return CO_center_L;

  case CO_DN_DN_R:
    return CO_air_R;

  case CO_DN:
    return CO_air_L;

  case CO_DN_DN_L:
    return CO_r_DN;

  case CO_DN_L:
    return CO_r_UP;

  case CO_DN_L_L:
    return CO_dn_R;

  case CO_L:
    return CO_dn_L;

  case CO_UP_L_L:
    return CO_l_DN;

  case CO_UP_L:
    return CO_l_UP;

  case CO_UP_UP_L:
    return CO_up_R;

  case CO_UP:
    return CO_up_L;

  case CO_UP_UP_R:
    return CO_air_UP;

  case CO_UP_R:
    return CO_air_DN;


  default:
    assert(0);
    return CO_air;
  }
}



void Fighter::attack_stuck(Dir dir,Stance stance) {
  const Vel *unitVels = cre->get_unit_vels();

  cre->set_vel_next((float)fc->slide * unitVels[dir]);
  cre->set_stance_next(stance);
  timer.set(FIGHTER_SLIDE_TIME);
  attackNext = attackStuck;
}



void Fighter::attack_free(Dir dir) {
  const Acc *unitAccs = cre->get_unit_accs();

  cre->set_vel_next(cre->get_vel() + fc->jumpHorizontal * unitAccs[dir]);
  cre->set_stance_next(CO_air);
  timer.set(FIGHTER_FREE_TIME);  // Ignored unless cre has the Flying Ability.
  attackNext = attackFree;
}



void Fighter::attack_free_vertical(Dir dir) {
  const Acc *unitAccs = cre->get_unit_accs();
  Vel vel = cre->get_vel();

  Vel velNext(0.0f,vel.dy + fc->jumpVertical * unitAccs[dir].ddy);
  cre->set_vel_next(velNext);
  cre->set_stance_next(CO_air);
  timer.set(FIGHTER_FREE_TIME);  // Ignored unless cre has the Flying Ability.
  attackNext = attackFree;
}



Touching Fighter::offset_generator(Dir dir) {
  if (is_attack_dir(dir)) {
    Dir nonAttack = attack_dir_to_dir(dir);
    return Creature::offset_generator(nonAttack);
  }
  return Creature::offset_generator(dir);
}



const FighterContext *Fighter::lookup_context(ClassId cId) {
  // Need to add a new entry every time we add a new 
  // class that uses Fighter.
  // Not very extendable.  This sucks, Beavis.
  switch (cId) {
    case A_Ninja:
      return &Ninja::fighterContext;
    case A_Hero:
      return &Hero::fighterContext;
    case A_Zombie:
      return &Zombie::fighterContext;
    case A_Dog:
      return &Dog::fighterContext;
    case A_Mutt:
      return &Mutt::fighterContext;
    case A_Chicken:
      return &Chicken::fighterContext;
    default:
      assert(0);
  }
  return NULL;
}



FighterXdata Fighter::defaultXdata;



Walking::Walking(const WalkingContext &,WalkingXdata &) 
:Locomotion() {
}



DEFINE_CREATE_ABILITY_FROM_STREAM(Walking)



WalkingContext Walking::defaultContext;



WalkingXdata Walking::defaultXdata;



Walking::Walking(InStreamP in,CreatureP c,
                 const WalkingContext &,
                 WalkingXdata &)
: Locomotion(in,c) {
}



AbilityId Walking::get_ability_id() {
  return AB_Walking;
}



void Walking::act() {
  Vel vel = cre->get_vel();
  const Pos &rawPos = cre->get_raw_pos();

  // Avoid getting stuck.
/*  if (cre->alive() && velPrev.dy > 0 && rawPosPrev == rawPos)
    {
      Vel velNext(0,vel.dy);
      cre->set_vel_next(velNext);
#ifdef PRINT_ERRORS
      cerr << "Walking::_act()  trying to get unstuck." << endl;
#endif
      return;
    }
    
  velPrev = vel;
  rawPosPrev = rawPos;
  */

  // Methods of Moving or Creature.
  Speed acceleration = cre->get_acceleration();  
#if WIN32
  acceleration = (int)(acceleration * 1.5);
#endif
  Speed jump = cre->get_jump();
  const Stance stance = cre->get_stance();
  const Touching touchingArea = cre->get_touching_area();
  const Touching touchingStance = cre->get_touching_stance();
  const Hanging &hanging = cre->get_hanging();
  const Acc *unitAccs = cre->get_unit_accs();
  const Boolean canClimb = cre->can_climb();
  const Area area = cre->get_area();
  const ITcommand command = cre->get_command(); 
  
  
  // Default set gravity.
  cre->set_grav_next((touchingArea == CO_dn) || 
		(canClimb && (stance == CO_climb) && 
		 cre->alive()) ? 0 : PH_GRAVITY);

  
  // Change stance if necessary.
  if (stance != CO_climb) {
    if (touchingArea != touchingStance)	{
      cre->set_want_climb(False);
      
      // Hit the ground.
      if (touchingArea == CO_dn) {
        cre->set_stance_next(CO_center);
        Vel velNew(vel.dx,0.0f);
        cre->set_vel_next(velNew);
      }
      // Side walls.
      else if (touchingArea == CO_r) {
        // Keep the +-1 in so that when Heros jump against small
        // walls/bumps he still goes over.
        Vel velNew(1.0f,vel.dy);
        cre->set_vel_next(velNew);
      }
      else if (touchingArea == CO_l) {
        Vel velNew(-1.0f,vel.dy);
        cre->set_vel_next(velNew);
      }
      // In the air.
      else if (touchingArea == CO_air) {
        cre->set_stance_next(CO_air);
      }
      else if (touchingArea != CO_up) {
        assert(0);
      }
    }
    else if (touchingArea == CO_dn) {
      if (vel.dy != 0) {
        Vel velNew(vel.dx,0.0f);
        cre->set_vel_next(velNew);
      }
  	}
  }
  // Stop climbing if not in climbing square. 
  else if (!canClimb) { // stance == CO_climb
    cre->set_stance_next(CO_air);
  }

  
  /* Interpret commands from controlling intelligence.  May override some of 
     above settings. */
  switch (command) {
  case IT_CENTER:
    if (stance == CO_air) {
      cre->set_want_climb(True);
    }
    if ((stance != CO_climb) && canClimb && (stance != CO_dn) &&
        (stance != CO_center)) {
      cre->set_stance_next(CO_climb);
      cre->set_vel_next(0);
      cre->set_grav_next(0);
      cre->center_wsquare_x_next(area.middle_wsquare());
      cre->set_want_climb(False);
    }
    else if (stance != CO_air) {
      cre->set_vel_next(0);
    }
  break;
  
  case IT_R:
    if ((stance == CO_dn) || (stance == CO_center) || // Regular walking.
        (stance == CO_climb)) {
      DO_INCREASE(vel.dx);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_center_R]);
    }
  break;
  
  case IT_DN_R:
    if (stance == CO_dn) {
      DO_INCREASE(vel.dx);
      DO_INCREASE(vel.dy);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_dn_R]);
    }
    else if (stance == CO_center) { // Crawling.
      DO_INCREASE(vel.dx);
      DO_INCREASE(vel.dy);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_dn_R]);
      cre->set_stance_next(CO_dn);
    }
    else if (stance == CO_climb) {
      DO_INCREASE(vel.dx);
      DO_INCREASE(vel.dy);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_center_R]);
    }
  break;
  
  case IT_DN:
    if (stance == CO_air) {
      cre->set_want_climb(True);
    }
    if ((stance != CO_climb) && canClimb && 
        !cre->standing_squarely()) {
      cre->set_stance_next(CO_climb);
      cre->set_vel_next(0);
      cre->set_grav_next(0);
      cre->center_wsquare_x_next(area.middle_wsquare());
      cre->set_want_climb(False);
    }
    // slow down.
    else if (stance == CO_dn) {
      cre->set_vel_next(0);
    }
    else if (stance == CO_center) {
      cre->set_stance_next(CO_dn);
    }
    else if (stance == CO_climb) {
      DO_INCREASE(vel.dy);
      if (touchingArea == CO_dn) {
        cre->set_stance_next(CO_dn);
        Vel velNew(vel.dx,0.0f);
        cre->set_vel_next(velNew);
      }
      else {
        cre->set_vel_next(vel + acceleration * unitAccs[CO_climb_DN]);
      }
    }
  break;
  
  case IT_DN_L:
    if (stance == CO_dn) { // Crawling.
      DO_DECREASE(vel.dx);
      DO_INCREASE(vel.dy);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_dn_L]);
    }
    else if (stance == CO_center) { // Crawling.
      DO_DECREASE(vel.dx);
      DO_INCREASE(vel.dy);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_dn_L]);
      cre->set_stance_next(CO_dn);
    }
    else if (stance == CO_climb) {
      DO_DECREASE(vel.dx);
      DO_INCREASE(vel.dy);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_center_L]);
    }
  break;
  
  case IT_L:
    if ((stance == CO_dn) || (stance == CO_center) || // Regular walking.
        (stance == CO_climb)) {
      DO_DECREASE(vel.dx);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_center_L]);
    }
  break;
  
  case IT_UP_L:
    if ((stance == CO_dn) || (stance == CO_center) || (stance == CO_climb)) {
      // Jump left.
      DO_DECREASE(vel.dx);
      cre->set_vel_next(vel + jump * unitAccs[CO_air_L]);
      cre->set_stance_next(CO_air);
    }
  break;
  
  case IT_UP:
    if (stance == CO_air) {
      cre->set_want_climb(True);
    }
    // Doors are handled by Creature.
    if (cre->on_door())
      break;
    if ((stance != CO_climb) && canClimb) {
      cre->set_stance_next(CO_climb);
      cre->set_vel_next(0);
      cre->set_grav_next(0);
      cre->center_wsquare_x_next(area.middle_wsquare());
      cre->set_want_climb(False);
    }
    else if (stance == CO_dn) {
      cre->set_stance_next(CO_center);
    }
    else if (stance == CO_center) { // Jump up.
      cre->set_vel_next(vel + jump * unitAccs[CO_air_UP]);
      cre->set_stance_next(CO_air);
    }
    else if (stance == CO_climb) {
      DO_DECREASE(vel.dy);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_climb_UP]);
    }
  break;
  
  case IT_UP_R:
    // Jump right.
    if ((stance == CO_dn) || (stance == CO_center) || (stance == CO_climb)) {
      DO_INCREASE(vel.dx);
      cre->set_vel_next(vel + jump * unitAccs[CO_air_R]);
      cre->set_stance_next(CO_air);
    }
    break;
  }
}



Sticky::Sticky(const StickyContext &cx,StickyXdata &) 
: Locomotion() {
  wantCorner1 = wantCorner2 = CO_air;
  set_dont_attach(False);
  context = &cx;
}



DEFINE_CREATE_ABILITY_FROM_STREAM(Sticky)



StickyContext Sticky::defaultContext;



StickyXdata Sticky::defaultXdata;



Sticky::Sticky(InStreamP in,CreatureP c,
               const StickyContext &,
               StickyXdata &)
: Locomotion(in,c) {
  wantCorner1 = wantCorner2 = CO_air;
  set_dont_attach(False);
}



void Sticky::initialize(CreatureP c) {
  Locomotion::initialize(c);

  assert(has_required_frames(cre->get_moving_context()));
}



AbilityId Sticky::get_ability_id() {
  return AB_Sticky;
}



void Sticky::act() {
  // Methods of Moving or Creature.
  Speed acceleration = cre->get_acceleration();  
#if WIN32
  acceleration = (int)(acceleration * 1.5);
#endif
  Speed jump = cre->get_jump();
  const Stance stance = cre->get_stance();
  const Touching touchingArea = cre->get_touching_area();
  const Touching touchingStance = cre->get_touching_stance();
  const Hanging &hanging = cre->get_hanging();
  Vel vel = cre->get_vel();
  const Acc *unitAccs = cre->get_unit_accs();
  const Boolean canClimb = cre->can_climb();
  const Area area = cre->get_area();
  const ITcommand command = cre->get_command(); 
  
  // Disable the dontAttach flag if we aren't in the air anymore.
  if (stance != CO_air && get_dont_attach()) {
    set_dont_attach(False);
  }

  
  // Have to do this check here because we need the flag set
  // before checking whether we should attach to walls.
  if ((command == IT_UP_L || command == IT_UP_R) &&
      stance == CO_air) {
    set_dont_attach(True);
  }


  /* Set gravity.  Don't lock on walls if dead. */
  cre->set_grav_next(
    // Dead and not on the ground
    (!cre->alive() && (touchingArea != CO_dn)) ||

    // Touching wall or ceiling, but dontAttach flag is set.
    (get_dont_attach() && (touchingArea == CO_r || touchingArea == CO_l || touchingArea == CO_up)) ||

    // In the air and not on a ladder.
    ((touchingArea == CO_air) && !(canClimb && (stance == CO_climb)))
		
     ? PH_GRAVITY : 0);

  

  /* Make stance correspond to touching.  I.e. attach / detach to wall.
     Do not attach to wall if climbing. */
  if ((stance != CO_climb) || !canClimb) {

    if ((touchingArea != touchingStance) || 
      ((stance == CO_climb) && !canClimb)) {
      cre->set_want_climb(False);

      if (touchingArea == CO_dn) {
        cre->set_stance_next(CO_center);
        Vel velNew(vel.dx,0.0f);
        cre->set_vel_next(velNew);
      }
      else if (touchingArea == CO_r) {
        if (get_dont_attach()) {
          // Keep the +-1 in so that when jump against small
          // walls/bumps he still goes over.
          Vel velNew(1.0f,vel.dy);
          cre->set_vel_next(velNew);
        }
        else {
          // attach
          cre->set_stance_next(CO_r);
          cre->set_vel_next(0);
//            Vel velNew(0.0f,vel.dy);
//            cre->set_vel_next(velNew);
        }
      }
      else if (touchingArea == CO_l) {
        if (get_dont_attach()) {
          Vel velNew(-1.0f,vel.dy);
          cre->set_vel_next(velNew);
        }
        else {
          // attach
          cre->set_stance_next(CO_l);
          cre->set_vel_next(0);
//            Vel velNew(0.0f,vel.dy);
//            cre->set_vel_next(velNew);
        }
      }
      else if (touchingArea == CO_up) {
        if (!get_dont_attach()) {
          cre->set_stance_next(CO_up);
//            cre->set_vel_next(0);
          Vel velNew(vel.dx,0.0f);
          cre->set_vel_next(velNew);
        }
      }
      else if (touchingArea == CO_air) {
        cre->set_stance_next(CO_air);
      }
      else {
        assert(0);
      }
    }
    else if ((touchingArea == CO_dn) || (touchingArea == CO_up)) {
      // Maybe we should get on/off ladder here, right now is done
      // below.
      if (vel.dy != 0) {
        Vel velNew(vel.dx,0.0f);
        cre->set_vel_next(velNew);
      }
    }
    else if ((touchingArea == CO_r) || (touchingArea == CO_l)) {
      if (vel.dx != 0) {
        Vel velNew(0.0f,vel.dy);
        cre->set_vel_next(velNew);
      }
    }
  }

  // Stop climbing if not in climbing square. 
  //  else if (! canClimb) // stance == CO_climb
  //    cre->set_stance_next(CO_air);
  

  /* Interpret commands from controlling intelligence.  May override some of 
     above settings. */
  switch (command) {
  case IT_CENTER:
    if (stance == CO_air) {
      cre->set_want_climb(True);
    }
    set_want_corner(CO_air);  
    if ((stance != CO_climb) && canClimb && (stance != CO_dn) &&
      (stance != CO_center)) {
      cre->set_stance_next(CO_climb);
      cre->set_vel_next(0);
      cre->set_grav_next(0);
      cre->center_wsquare_x_next(area.middle_wsquare());
    }
    else if (stance != CO_air) {
      cre->set_vel_next(0);
    }
    break;
    
  case IT_R:
    set_want_corner(CO_air);  
    if (stance == CO_r) { // On right wall.
      cre->set_vel_next(0);
      set_want_corner(CO_r_UP,CO_r_DN);
    }
    else if ((stance == CO_dn) || (stance == CO_center) || // Regular walking.
       (stance == CO_climb)) {
      DO_INCREASE(vel.dx);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_center_R]);
    }
    else if (stance == CO_l) { // On left wall.
      DO_INCREASE(vel.dx);
      cre->set_vel_next(vel + PH_PUSH_OFF * unitAccs[CO_R]);
    }
    else if (stance == CO_up) {// On ceiling.
      DO_INCREASE(vel.dx);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_up_R]);
    }
    break;
    
  case IT_DN_R:
    set_want_corner(CO_air);  
    if (stance == CO_r) {// On right wall.
      DO_INCREASE(vel.dx);
      DO_INCREASE(vel.dy);
      set_want_corner(CO_r_DN);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_r_DN]);
    }
    else if (stance == CO_dn) {
      DO_INCREASE(vel.dx);
      DO_INCREASE(vel.dy);
      set_want_corner(CO_dn_R);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_dn_R]);
    }
    else if ((stance == CO_l) || (stance == CO_up)) {
      DO_INCREASE(vel.dx);
      DO_INCREASE(vel.dy);
      cre->set_vel_next(vel + PH_PUSH_OFF * unitAccs[CO_DN_R]);
    }
    else if (stance == CO_center) { // regular walking
      DO_INCREASE(vel.dx);
      DO_INCREASE(vel.dy);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_dn_R]);
      cre->set_stance_next(CO_dn);
    }
    else if (stance == CO_climb) {
      DO_INCREASE(vel.dx);
      DO_INCREASE(vel.dy);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_center_R]);
    }
    break;
    
  case IT_DN:
    if (stance == CO_air) {
      cre->set_want_climb(True);
    }
    set_want_corner(CO_air);  
    if ((stance != CO_climb) && canClimb && 
        !cre->standing_squarely()) {
      cre->set_stance_next(CO_climb);
      cre->set_vel_next(0);
      cre->set_grav_next(0);
      cre->center_wsquare_x_next(area.middle_wsquare());
    }
    else if (stance == CO_dn) {
      cre->set_vel_next(0);
      set_want_corner(CO_dn_R,CO_dn_L);
    }
    else if (stance == CO_center) { // duck
      cre->set_stance_next(CO_dn);
    }
    else if (stance == CO_up) {
      DO_INCREASE(vel.dy);
      cre->set_vel_next(vel + PH_PUSH_OFF * unitAccs[CO_DN]);
    }
    else if (stance == CO_r) {
      DO_INCREASE(vel.dy);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_r_DN]);
    }
    else if (stance == CO_l) {
      DO_INCREASE(vel.dy);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_l_DN]);    
    }
    else if (stance == CO_climb) {
      if (touchingArea == CO_dn) {
        cre->set_stance_next(CO_dn);
        Vel velNew(vel.dx,0.0f);
        cre->set_vel_next(velNew);
      }
      else {
        DO_INCREASE(vel.dy);
        cre->set_vel_next(vel + acceleration * unitAccs[CO_climb_DN]);
      }
    }
    break;
    
  case IT_DN_L:
    set_want_corner(CO_air);  
    if ((stance == CO_r) || (stance == CO_up)) {// On other walls.
      DO_DECREASE(vel.dx);
      DO_INCREASE(vel.dy);
      cre->set_vel_next(vel + PH_PUSH_OFF * unitAccs[CO_DN_L]);
    }
    else if (stance == CO_dn) {// Crawling.
      DO_DECREASE(vel.dx);
      DO_INCREASE(vel.dy);
      set_want_corner(CO_dn_L);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_dn_L]);
    }
    else if (stance == CO_l) {// On left wall.
      DO_DECREASE(vel.dx);
      DO_INCREASE(vel.dy);
      set_want_corner(CO_l_DN);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_l_DN]);
    }
    else if (stance == CO_center) {// Crawling.
      DO_DECREASE(vel.dx);
      DO_INCREASE(vel.dy);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_dn_L]);
      cre->set_stance_next(CO_dn);
    }
    else if (stance == CO_climb) {
      DO_DECREASE(vel.dx);
      DO_INCREASE(vel.dy);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_center_L]);
    }
    break;
    
  case IT_L:
    set_want_corner(CO_air);  
    if (stance == CO_l) {// On left wall.
      set_want_corner(CO_l_DN,CO_l_UP);
      cre->set_vel_next(0);
    }
    else if ((stance == CO_dn) || (stance == CO_center) || // Regular walking.
             (stance == CO_climb)) {
      DO_DECREASE(vel.dx);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_center_L]);
    }
    else if (stance == CO_r) {// On right wall.
      DO_DECREASE(vel.dx);
      cre->set_vel_next(vel +  PH_PUSH_OFF * unitAccs[CO_L]);
    }
    else if (stance == CO_up) {// On ceiling.
      DO_DECREASE(vel.dx);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_up_L]);
    }
    break;
    
  case IT_UP_L:
    set_want_corner(CO_air);  
    if ((stance == CO_r) || (stance == CO_dn) || (stance == CO_center) ||
        (stance == CO_climb)) {
      // Jump left.
      DO_DECREASE(vel.dx);
      DO_DECREASE(vel.dy);
      cre->set_vel_next(vel + jump * unitAccs[CO_air_L]);
      cre->set_stance_next(CO_air);
    }
    else if (stance == CO_l) {// On left wall.
      DO_DECREASE(vel.dx);
      DO_DECREASE(vel.dy);
      set_want_corner(CO_l_UP);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_l_UP]);
    }
    else if (stance == CO_up) {// On ceiling.
      DO_DECREASE(vel.dx);
      DO_DECREASE(vel.dy);
      set_want_corner(CO_up_L);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_up_L]);
    }
    break;
    
  case IT_UP:
    // Doors are handled by Creature.
    if (cre->on_door()) {
      break;
    }
    if (stance == CO_air) {
      cre->set_want_climb(True);
    }
    set_want_corner(CO_air);  
    if ((stance != CO_climb) && canClimb && stance != CO_up) {
      cre->set_stance_next(CO_climb);
      cre->set_vel_next(0);
      cre->set_grav_next(0);
      cre->center_wsquare_x_next(area.middle_wsquare());
    }
    else if (stance == CO_up) {// On ceiling.
      set_want_corner(CO_up_R,CO_up_L);
      cre->set_vel_next(0);
    }
    else if (stance == CO_dn) {
      cre->set_stance_next(CO_center);
    }
    else if (stance == CO_center) {// Jump up.
      DO_DECREASE(vel.dy);
      cre->set_vel_next(vel + jump * unitAccs[CO_air_UP]);
      cre->set_stance_next(CO_air);
    }
    else if (stance == CO_r) {// On right wall.
      DO_DECREASE(vel.dy);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_r_UP]);
    }
    else if (stance == CO_l) {// On left wall.
      DO_DECREASE(vel.dy);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_l_UP]);
    }
    else if (stance == CO_climb) {
      if (touchingArea == CO_up) {
        cre->set_stance_next(CO_up);
        Vel velNew(vel.dx,0.0f);
        cre->set_vel_next(velNew);
      }
      else {
        DO_DECREASE(vel.dy);
        cre->set_vel_next(vel + acceleration * unitAccs[CO_climb_UP]);
      } 
    }
    break;
    
  case IT_UP_R:
    set_want_corner(CO_air);  
    if (stance == CO_r) {// On right wall.
      DO_INCREASE(vel.dx);
      DO_DECREASE(vel.dy);
      set_want_corner(CO_r_UP);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_r_UP]);
    }
    else if ((stance == CO_dn) || (stance == CO_l) || (stance == CO_center) ||
       (stance == CO_climb)) {
      /* Jump right. */
      DO_INCREASE(vel.dx);
      DO_DECREASE(vel.dy);
      cre->set_vel_next(vel + jump * unitAccs[CO_air_R]);
      cre->set_stance_next(CO_air);
    }
    else if (stance == CO_up) // On ceiling.
    {
      DO_INCREASE(vel.dx);
      DO_DECREASE(vel.dy);
      set_want_corner(CO_up_R);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_up_R]);
    }
    break;
  }
  
  // Go around outside corners. 
  if (want_corner(hanging.corner)) {
    cre->set_stance_next(cornered_stance(hanging));

    // Transfer verto to horiz velocity if going from vertical to horiz stance.
    if (hanging.corner == CO_r_DN ||hanging.corner == CO_l_UP) {
      // Positive Y becomes positive X. 
      Vel velNew(vel.dy,0.0f);
      cre->set_vel_next(velNew);
    }
    else if ( hanging.corner == CO_l_DN || hanging.corner == CO_r_UP) {
      // Positive Y becomes negative X.
      Vel velNew(-vel.dy,0.0f);
      cre->set_vel_next(velNew);
    }
    else {
      cre->set_vel_next(0);
    }

    cre->corner(hanging);
    set_want_corner(CO_air);
  }
}



Boolean Sticky::has_required_frames(const MovingContext *mc) {
  assert(mc);

  // Needs fucking everything.
  for (Dir dir = CO_center_R; dir < CO_R; dir++) {
    if (mc->animMax[dir] < 1) {
      return False;
    }
  }
  return True;
}



Boolean Sticky::want_corner(const Corner &corner) {
  return (corner != CO_air) && 
          ((corner == wantCorner1) || (corner == wantCorner2));
}



Boolean Sticky::stick_touching(Touching touching) {
  switch (touching) {
    case CO_r:
//    case CO_dn:   Already granted by Creature.
    case CO_l:
    case CO_up:
      return True;

    default:
      return False;
  }
}



Stance Sticky::cornered_stance(const Hanging &hanging) {
  switch (hanging.corner) {
  case CO_r_DN:
    return CO_up;
  case CO_r_UP:
    return CO_dn;
  case CO_dn_R:
    return CO_l;
  case CO_dn_L:
    return CO_r;
  case CO_l_DN:
    return CO_up;
  case CO_l_UP:
    return CO_dn;
  case CO_up_R:
    return CO_l;
  case CO_up_L:
    return CO_r;
  default:
    assert(0);
    return CO_air;
  }
}



Flying::Flying(const FlyingContext &cx,FlyingXdata &) 
:Locomotion() {
  context = &cx;
}



DEFINE_CREATE_ABILITY_FROM_STREAM(Flying)



FlyingContext Flying::defaultContext;



FlyingXdata Flying::defaultXdata;



Flying::Flying(InStreamP in,CreatureP c,
               const FlyingContext &cx,
               FlyingXdata &)
: Locomotion(in,c) {
  context = &cx;
}



AbilityId Flying::get_ability_id() {
  return AB_Flying;
}



void Flying::act() {
  // Methods of Moving or Creature.
  Speed acceleration = cre->get_acceleration();  
//#if WIN32  Took it out after XEvil 2.01
  acceleration = (int)(acceleration * 1.5);
//#endif
  Speed jump = cre->get_jump();  
  const Stance stance = cre->get_stance();
  const Touching touchingArea = cre->get_touching_area();
  const Touching touchingStance = cre->get_touching_stance();
  Vel vel = cre->get_vel();
  const Acc *unitAccs = cre->get_unit_accs();
  const ITcommand command = cre->get_command(); 

  // Used to be an assertion.  But, now with dynamic abilities, is possible
  // for a creature to be in some other stance when given the Flying ability.
  // Drawing code should be ok.  If the Creature used to have a Locomotion 
  // that put him in the current stance, then it must have the frames to 
  // draw in the stance.
  if (stance != CO_air && stance != CO_center) {
    cre->set_stance_next(CO_air);
  }

  if (touchingArea != CO_dn && 
      // This keeps us from falling down too fast, doesn't affect slowdown
      // while going upwards.
      vel.dy <= FLYING_GRAV_VEL_CUTOFF) {
    // We negate the context.gravTime value.  See comments on 
    // Creature::set_grav_next().
    cre->set_grav_next(-context->gravTime);
  }
  else {
    cre->set_grav_next(0);
  }

  // Limits on velNext imposed by Creature.
  if (touchingArea != touchingStance) {
    if (touchingArea == CO_dn) {
	    cre->set_stance_next(CO_center);
      Vel velNew(vel.dx,0.0f);
      cre->set_vel_next(velNew);
    }
    else if (touchingArea == CO_air) {
      cre->set_stance_next(CO_air);
    }
  }  


  // If we are a fighter in the middle of an attack, ignore all commands 
  // until the attack is over.
  Fighter* fighter = (Fighter*)cre->get_ability(AB_Fighter);
  if (fighter && fighter->is_attacking()) {
    return;
  }


  /* Interpret commands from controlling intelligence.  May override some of 
     above settings. */
  switch (command) {
  case IT_CENTER:
    cre->set_vel_next(0);
    break;

  case IT_R: 
    DO_INCREASE(vel.dx);
    cre->set_vel_next(vel + acceleration * unitAccs[CO_R]);
    break;
  
  case IT_DN_R:
    DO_INCREASE(vel.dx);
    DO_INCREASE(vel.dy);
    if (stance == CO_air) {
      cre->set_vel_next(vel + acceleration * unitAccs[CO_DN_R]);
    }
    else {
      cre->set_vel_next(vel + acceleration * unitAccs[CO_center_R]);
    }
    break;
  
  case IT_DN:
    if (stance == CO_air) {
      DO_INCREASE(vel.dy);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_DN]);
    }
    else {
      cre->set_vel_next(0);
    }
    break;
  
  case IT_DN_L:
    DO_DECREASE(vel.dx);
    DO_INCREASE(vel.dy);
    if (stance == CO_air) {
      cre->set_vel_next(vel + acceleration * unitAccs[CO_DN_L]);
    }
    else {
      cre->set_vel_next(vel + acceleration * unitAccs[CO_center_L]);
    }
    break;
  
  case IT_L:
    DO_DECREASE(vel.dx);
    cre->set_vel_next(vel + acceleration * unitAccs[CO_L]);
    break;
  
  case IT_UP_L:
    if (stance == CO_air) {
      DO_DECREASE(vel.dx);
      DO_DECREASE(vel.dy);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_air_L]);
    }
    else {
      DO_DECREASE(vel.dx);
      cre->set_vel_next(vel + jump * unitAccs[CO_air_L]);
	    cre->set_stance_next(CO_air);
    }
    break;

  case IT_UP:
    // Doors are handled by Creature.
    if (cre->on_door())
      break;
    if (stance == CO_air) {
      DO_DECREASE(vel.dy);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_air_UP]);
    }
    else {
      cre->set_vel_next(vel + jump * unitAccs[CO_air_UP]);
	    cre->set_stance_next(CO_air);
    }
    break;

  case IT_UP_R:
    if (stance == CO_air) {
      DO_INCREASE(vel.dx);
      DO_DECREASE(vel.dy);
      cre->set_vel_next(vel + acceleration * unitAccs[CO_air_R]);
    }
    else {
      DO_INCREASE(vel.dx);
      cre->set_vel_next(vel + jump * unitAccs[CO_air_R]);
      cre->set_stance_next(CO_air);
    }
    break;
  };
}



BuiltIn::BuiltIn(const BuiltInContext &cx,BuiltInXdata &) 
:Ability() {
  // AltarOfSin requires that BuiltInXdata not be used, passes in dummy.

  context = &cx;
  Timer nTimer(cx.shotTime);
  shotTimer = nTimer;
}



DEFINE_CREATE_ABILITY_FROM_STREAM(BuiltIn)



BuiltInContext BuiltIn::defaultContext;



BuiltInXdata BuiltIn::defaultXdata;



BuiltIn::BuiltIn(InStreamP in,CreatureP c,
                 const BuiltInContext &cx,
                 BuiltInXdata &)
: Ability(in,c) {
  context = &cx;
}



AbilityId BuiltIn::get_ability_id() {
  return AB_BuiltIn;
}



ClassId BuiltIn::get_weapon_string(const char *&str) {
  // If we are a Holder with a current weapon, let Holder take 
  // care of it.
  HolderP holder = cre->get_holder();
  if (holder && holder->get_weapon_current()) {
    return A_None;
  }
  
  // Set to the BuiltIn weapon.
  str = context->weaponStr;
  return context->weaponClassId;
}



Boolean BuiltIn::ready() {
  return shotTimer.ready();
}



void BuiltIn::act() {
  Dir shotDir;
  // If the context specifies a particular compute_weapon_dir function, 
  // use it.  Else, default.
  if (context->compute_weapon_dir) {
    shotDir = context->compute_weapon_dir(cre->get_command());
  }
  else {
    shotDir = Intel::command_weapon_to_dir_8(cre->get_command());
  }

  assert(context->create_shot && context->get_shot_size);

  HolderP holder = cre->get_holder();
  if (shotTimer.ready() && 
      shotDir != CO_air &&
      // Don't fire if Creature is a holder with a current weapon.
      !(holder && holder->get_weapon_current())) {
    const Area &area = cre->get_area();
    LocatorP locator = cre->get_locator();
    Pos shotPos = 
    Coord::shot_initial_pos(area,cre->get_touching_area(),
                            context->get_shot_size(shotDir),shotDir);
    PhysicalP shot = context->create_shot(cre->get_world(),locator,shotPos,
                                          cre->get_id(),shotDir);
    shot->set_dont_collide(cre->get_id());

    // Shot should not collide with any members of the shooter's composite object.
    CompositeP comp = cre->get_composite();
    if (comp) {
      shot->set_dont_collide_composite(comp->get_composite_id());
    }

    locator->add(shot);
    shotTimer.set();

    // Make sound.
    if (context->attackSound) {
  	  SoundRequest req(context->attackSound,cre->get_area());
	    locator->submitSoundRequest(req);
    }

    // Signal that we attacked something.
    cre->attack_hook();
  } 
  
  shotTimer.clock();
}



Hugger::Hugger(const HuggerContext &cx,HuggerXdata &) 
:Ability() {
  context = &cx;
}



DEFINE_CREATE_ABILITY_FROM_STREAM(Hugger)



HuggerContext Hugger::defaultContext;



HuggerXdata Hugger::defaultXdata;



Hugger::Hugger(InStreamP in,CreatureP c,
               const HuggerContext &cx,
               HuggerXdata &)
: Ability(in,c) {
  context = &cx;
}



AbilityId Hugger::get_ability_id() {
  return AB_Hugger;
}



int Hugger::get_drawing_level() {
  return 2;
}



void Hugger::act() {
  LocatorP locator = cre->get_locator();
  PhysicalP huggee = locator->lookup(huggeeId);
  
  if (cre->alive() && huggee) {
    // Someone else killed the huggee while being hugged.
    if (!huggee->alive()) {
      Id invalid;
      huggeeId = invalid;
    }
    // Create new alien.
    else if (!cre->stunned() && !cre->stunned_next()) {
      const Area &area = huggee->get_area();
      Pos middle = area.get_middle();
      Size alienSize = Alien::get_size_max();
      Pos alienPos(middle.x - alienSize.width / 2,
                     middle.y - alienSize.height / 2);
      PhysicalP alien = new Alien(cre->get_world(),locator,alienPos);
      assert(alien);
      locator->add(alien);
      // Set the intelligence of the new Alien.
      if (context->useHuggeeIntel) {
        // Use intelligence of huggee.  
        alien->set_intel(huggee->get_intel()); 
        huggee->set_intel(NULL);

        // No kill is awarded for this death.
      }
      else {
        // Do one point of damage to huggee, so hugger will get credit for the
        // kill.  Do this before swapping the intels.
        huggee->corporeal_attack(cre,1);

        // Use own intelligence.  Huggee's intel dies with the huggee's
        // physical body.
        alien->set_intel(cre->get_intel());
        cre->set_intel(NULL);
      }

      // kill huggee
      huggee->kill_self();

      // kill hugger
      cre->kill_self();
    }
    // huggee is alive() but hugger is stunned.
    else { 
      // Stay locked onto hugee's face.
      const Area &area = huggee->get_area();
      Pos p;
      Size s;
      area.get_rect(p,s);
      // Guess where hugee's face is, upper-left + (width/2,height*.3).
      Pos pNext(p.x + (int) (0.5 * s.width),
                  p.y + (int) (0.3 * s.height));
      cre->_set_middle_next(pNext);
        // Don't need to call update_area_next(), because Moving::act() is
        // yet to happen.
    }
    
  } // if (alive() && huggee)
}



Boolean Hugger::collide(PhysicalP other) {
  LocatorP l = cre->get_locator();

  if (
      // Dead Huggers don't hug.
      cre->alive() &&

      // Make sure not already hugging something.
      !l->lookup(huggeeId) &&  

      // Only hug living biological creatures.
      other->alive() &&
      other->is_creature() &&
      ((CreatureP)other)->biological() &&


  ///// Don't need the following two checks now that we have Teams.

      // Don't hug aliens or other huggers.
      other->get_class_id() != A_Alien && 
      !((CreatureP)other)->get_ability(AB_Hugger) &&

      // Don't hug alien immune creatures.
      !((CreatureP)other)->get_alien_immune()
      ) 
  {
    // Kind of a hack, stun self as well as huggee.
    cre->stun_next(HUGGER_TIME);

    // Stun other for one turn longer so that we have one turn to 
    // turn other into an alien.
    ((CreatureP)other)->stun_next(HUGGER_TIME + 1);

    // Don't want other to get killed or hugged when already being hugged.
    // Need two extra turns, one to turn other into Alien, and one for
    // synchronization as alienImmune is immediate and stun is not.
    ((CreatureP)other)->set_alien_immune(HUGGER_TIME + 2);

    huggeeId = other->get_id();

    return True;
  }

  return False;
}



Prickly::Prickly(const PricklyContext &cx,PricklyXdata &)
:Ability() {
  context = &cx;
}
	


DEFINE_CREATE_ABILITY_FROM_STREAM(Prickly)



PricklyContext Prickly::defaultContext;



PricklyXdata Prickly::defaultXdata;



Prickly::Prickly(InStreamP in,CreatureP c,
                 const PricklyContext &cx,
                 PricklyXdata &)
: Ability(in,c) {
  context = &cx;
}



AbilityId Prickly::get_ability_id() {
  return AB_Prickly;
}



ClassId Prickly::get_weapon_string(const char *&str) {
  // Code copied from Fighter::get_weapon_string().
  // Sucks to have Abilities having to depend on peer Abilities.

  // If we are a Holder with a current weapon, let Holder take
  // care of it.
  HolderP holder = cre->get_holder();
  if (holder && holder->get_weapon_current()) {
    return A_None;
  }

  // If a Lifter lifting something, let lifter take care of it.
  LifterP lifter = (LifterP)cre->get_ability(AB_Lifter);
  if (lifter && lifter->is_lifting()) {
    return A_None;
  }

  // Set to hand-to-hand
  str = "prickly";  
  return A_Prickly;
}



Boolean Prickly::collide(PhysicalP other) {
  if (
      // Dead don't hurt.
      cre->alive() &&
      
      // Use an AutoUse, don't destroy it.  Would be better to put this check
      // somewhere else.
      !other->is_auto_use() &&

      other->get_class_id() != A_AltarOfSin) {
    
    int damage = cre->apply_modifiers(Modifier::DAMAGE,context->damage);
    other->corporeal_attack(cre,damage);
    cre->attack_hook();
    return True;
  }
  
  return False;
}



Healing::Healing(const HealingContext &cx,HealingXdata &)
:Ability() {
  // don't use Xdata, AltarOfSin passes in dummy value.

  context = &cx;	
  Timer t(context->time);
  healTimer = t;
  healTimer.set();
  // dont set healthMin until Ability::cre is valid, in initialize().
}



DEFINE_CREATE_ABILITY_FROM_STREAM(Healing)



HealingContext Healing::defaultContext;



HealingXdata Healing::defaultXdata;



Healing::Healing(InStreamP in,CreatureP c,
                 const HealingContext &cx,
                 HealingXdata &)
: Ability(in,c) {
  context = &cx;	
  healthMin = cre->get_health();
}



void Healing::initialize(CreatureP c) {
  Ability::initialize(c);
  
  // Must be after Ability::initialize().
  healthMin = cre->get_health();
}



AbilityId Healing::get_ability_id() {
  return AB_Healing;
}



void Healing::heal() {
  healthMin = cre->get_health_max();
}



void Healing::act() {
  // alive_next() checks if something else has already killed this.
  if (cre->alive() && cre->alive_next()) {
    Health health = cre->get_health();

    // BUG FIX: If we were just healed via Healing::heal(), don't heal 
    // incrementallythis turn.  Wait until next turn when
    // health has the new value.  Otherwise we set healthMin incorrectly.
    // Would probably be a cleaner fix to have a "healthMinNext" like Moving does.
    if (cre->get_health_next() <= health) {
      if (health < healthMin) {
        healthMin = health;
      }
      
      // Only heal every few turns.
      if (healTimer.ready()) {
        // Heal up to multiplier * min health in units of unit.
        Health healthBest = 
          Utils::minimum(cre->get_health_max(),((int)(healthMin * context->multiplier)));
        Health healthNext = health + context->unit;
        if (healthNext > healthBest) {
          healthNext = healthBest;
        }
    
        cre->set_health_next(healthNext);
        healTimer.set();
      }
    }
    healTimer.clock();
  }
}



LifterSpeedModifier::LifterSpeedModifier() 
  : Modifier() {
  lifting = False;
}



ModifierId LifterSpeedModifier::get_modifier_id() {
  return M_LifterSpeed;
}



int LifterSpeedModifier::apply(Attribute attr,int val) {
  // Cut speed in half if lifting something.
  if (lifting) {
    switch (attr) {
      //    case ACCELERATION:
    case AIR_SPEED:
    case CENTER_SPEED:
    case CLIMB_SPEED:
    case CRAWL_SPEED:
    // Using *= gives warnings on some compilers.
      val = (int)(val * 0.75);
      break;
    }
  }

  if (next) {
    return next->apply(attr,val);
  }
  else {
    return val;
  }
}



Lifter::Lifter(const LifterContext &cx,LifterXdata &)
:Ability() {
  context = &cx;
}



DEFINE_CREATE_ABILITY_FROM_STREAM(Lifter)



LifterContext Lifter::defaultContext;



LifterXdata Lifter::defaultXdata;



Lifter::Lifter(InStreamP in,CreatureP c,
               const LifterContext &cx,
               LifterXdata &)
: Ability(in,c) {
  context = &cx;
  // Don't bother adding LifterSpeedModifier
}



void Lifter::initialize(CreatureP c) {
  // Call first, to set cre.
  Ability::initialize(c);

  ModifierP modifiers = cre->get_modifiers();
  assert(modifiers);
  speedModifier = new LifterSpeedModifier();
  assert(speedModifier);
  modifiers->append_unique(speedModifier);
}



AbilityId Lifter::get_ability_id() {
  return AB_Lifter;
}



ClassId Lifter::get_weapon_string(const char *&str) {
  // Don't have to worry about Holder since Lifter will only 
  // lift if Holder has no items.

  LocatorP l = cre->get_locator();
  PhysicalP p = l->lookup(liftable);
  if (p) {
    if (context->throwSpeed) {
      str = "throw";
    }
    else {
      str = "drop";
    }
    return A_DropLiftable;
  }
  
  return A_None;
}



void Lifter::act() {
  LocatorP l = cre->get_locator();
  PhysicalP p = l->lookup(liftable);
  if (p) {
    assert(p->is_liftable());
    
    // Make the Liftable follow the Lifter
    Area area = cre->get_area();
    Pos pos;
    Size size;
    area.get_rect(pos,size);
    Pos middle = area.get_middle();
    Area otherArea = p->get_area();
    Size otherSize = otherArea.get_size();

    Stance stance = cre->get_stance();
    // Liftable drags on the floor or is carried.
    if (stance == CO_center || stance == CO_dn ||
        // If can carry, always do so.
        context->throwSpeed) {
      // Start with position on floor.
      int yMiddle = 
        pos.y + size.height 
        - otherSize.height + (otherSize.height >> 1);

      int xMiddle = middle.x;

      // Carry head height
      if (context->throwSpeed) {
        // Center liftable about at head-height, unless that is lower
        // down than the floor y value.
        int headYMiddle = pos.y + (int)(0.3 * size.height);
        yMiddle = Utils::minimum(yMiddle,headYMiddle);

        // Don't lag when carrying.
        Vel vel = cre->get_vel();
        xMiddle = (int)(middle.x + vel.dx);
      }
      // else drag on floor

      Pos middleNext(xMiddle,yMiddle);
      // Can cast to moving because it is a Liftable.
      ((MovingP)p)->set_middle_next(middleNext);
    }
    // Liftable hangs beneath the Lifter.
    else {      
      Pos middleNext(middle.x,
                     pos.y + size.height + (otherSize.height >> 1));
      // Can cast to moving because it is a Liftable.
      ((MovingP)p)->set_middle_next(middleNext);
    }



    // Put all this shit in follow().


    // We picked up a weapon, so drop the Liftable.
    HolderP holder = cre->get_holder();    
    if (holder && holder->get_weapons_num() > 0) {
      drop_liftable(p);
      return;
    }
    
    // Drop/throw the Liftable if any type of WEAPON command is issued.
    ITcommand command = cre->get_command();
    if (command == IT_WEAPON_CHANGE ||
        command == IT_WEAPON_DROP || 
        command == IT_WEAPON_CENTER) {
      drop_liftable(p);
      return;
    }
    // Throw if we can.
    else if (Intel::is_command_weapon(command)) {
      if (context->throwSpeed) {
        Dir dir = Intel::command_weapon_to_dir_8(command);
        // Throw.
        throw_liftable(p,dir);
      }
      else {
        // Can only drop.
        drop_liftable(p);
      }
      
      return;
    }
  }


  // Tell speedModifier whether or not to slow us down.
  ((LifterSpeedModifier *)speedModifier)->set_lifting(p != NULL);
}



Boolean Lifter::collide(PhysicalP other) {
  // Don't do anything if already lifting.
  LocatorP l = cre->get_locator();
  PhysicalP p = l->lookup(liftable);
  if (p) {
    // Disable collisions with weapons while we are lifting something.
    if (other->is_weapon()) {
      return True;
    }
    else {
      // Normal collisions.
      return False;
    }
  }

  // Can only lift Liftables.
  if (!other->is_liftable()) {
    return False;
  }

  // Already being lifted by someone else.
  if (((LiftableP)other)->being_lifted()) {
    return False;
  }

  // Not allowed to pick anything up if we have weapons.
  HolderP holder = cre->get_holder();
  if (holder && holder->get_weapons_num() > 0) {
    return False;
  }


  // Similar to the way Heavy works, but more restrictive.
  const Area &area = cre->get_area();
  const Area &otherArea = other->get_area();
  const Pos &otherMiddle = otherArea.get_middle();

  Dir dirTo = area.dir_to(otherMiddle);
  // Make it easier to pick up stuff, wider range of acceptable dirs.
  if ((dirTo == CO_DN || dirTo == CO_DN_R || dirTo == CO_DN_L) ||
      // If we can throw, we can pick up from any direction.
      context->throwSpeed > 0) {
    liftable = other->get_id();
    ((LiftableP)other)->lift(cre);

    // Don't need to make the Liftable follow here, because _act() will take
    // care of it.  collide() happens before act().
  }
  return True;
}



Boolean Lifter::is_lifting() {
  LocatorP l = cre->get_locator();
  PhysicalP p = l->lookup(liftable);
  return (p != NULL);
}



void Lifter::drop_liftable(PhysicalP p) {
  if (p) {
    ((LiftableP)p)->release(cre);
    liftable.invalidate();

    // Dropping is considered an attack.
    cre->attack_hook();
  }
}



void Lifter::throw_liftable(PhysicalP p,Dir dir) {
  if (p && p->is_moving()) {
    ((LiftableP)p)->release(cre);
    liftable.invalidate();

    const Vel *unitVels = cre->get_unit_vels();
    ((MovingP)p)->set_extra_vel_next((float)context->throwSpeed * unitVels[dir]);

    // Throwing is considered an attack.
    cre->attack_hook();
  }
}



Morphed::Morphed(const MorphedContext &cx,MorphedXdata &,
                 PhysicalP p)
:Ability() {
  if (p) {
    assert(p->get_intel() == NULL);
    unmapped = p->get_id();
    timer.set(cx.revertTime);
    unmappedSet = True;
  }
  else {
    unmappedSet = False;
  }
}



DEFINE_CREATE_ABILITY_FROM_STREAM(Morphed)



MorphedContext Morphed::defaultContext;



MorphedXdata Morphed::defaultXdata;



Morphed::Morphed(InStreamP in,CreatureP c,
                 const MorphedContext &,MorphedXdata &)
: Ability(in,c) {
  unmappedSet = False;
}



AbilityId Morphed::get_ability_id() {
  return AB_Morphed;
}



void Morphed::get_followers(PtrList &list) {
  // Included the unmapped object, e.g. so you don't get stuck as a frog for
  // the new level.
  if (unmappedSet) {
    LocatorP locator = cre->get_locator();
    PhysicalP p = locator->lookup(unmapped);
    if (p) {
      list.add(p);
    }
  }
}



void Morphed::drop_all(Boolean killNonPersistent) {
  // Drop everything the unmapped object is carrying.
  if (unmappedSet) {
    LocatorP locator = cre->get_locator();
    PhysicalP p = locator->lookup(unmapped);
    if (p) {
      p->drop_all(killNonPersistent);
    }
  }
}



void Morphed::act() {
  // alive() may not be necessary.
  if (cre->alive() && unmappedSet) {
    LocatorP locator = cre->get_locator();
    PhysicalP p = locator->lookup(unmapped);
    if (p && p->alive()) {

#if 0
      // Should be taken care of by the follower logic.
      assert(p->is_moving());
      const Area &area = cre->get_area();
      ((MovingP)p)->set_middle_next(area.get_middle());
#endif

      // Morph cre back into its original form.
      if (timer.ready()) {
	      p->set_intel(cre->get_intel());
	      p->set_mapped_next(True);
	      cre->set_intel(NULL);
	      cre->set_quiet_death();
	      cre->kill_self();
	      
	      unmappedSet = False;
      }
    }
  }
  
  if (unmappedSet) {
    timer.clock();
  }
}  



void Morphed::die() {
	if (unmappedSet) {
		LocatorP locator = cre->get_locator();
		PhysicalP p;
		if ((p = locator->lookup(unmapped)) && p->alive()) {
			assert(p->get_intel() == NULL);
			p->set_intel(cre->get_intel());
			cre->set_intel(NULL);

      // propagate the quiet death flag, or map so that the corpse is visible.
			if (cre->get_quiet_death()) {
				p->set_quiet_death();
			}
			else {
				p->set_mapped_next(True);
        // Is this necessary with the follower logic?
				const Area &area = cre->get_area();
				assert(p->is_moving());
				((MovingP)p)->set_middle_next(area.get_middle());
			}

      // Kill off the unmapped object
			if (!p->die_called()) {
			/* Partial hack to prevent corpse from getting knocked out of
				the world. */
				const Area &area = p->get_area_next();
				WorldP world = cre->get_world();
				if (!world->inside(area.middle_wsquare())) {
					p->set_quiet_death();
        }

				p->kill_self();
				p->die();
			}
			
      // Want the corpse of the unmapped, not the Morphed Creature.
			cre->set_quiet_death();
		}
	}
}



AnimTime::AnimTime(const AnimTimeContext &cx,AnimTimeXdata &)
:Ability() {
  context = &cx;
  assert(context->animTime > 0);
}



AbilityP AnimTime::create_from_stream(InStreamP in,CreatureP creat) {
  ClassId cId = creat->get_class_id();
  AbilityP ret = new AnimTime(in,creat,*lookup_context(cId),defaultXdata);
  assert(ret);
  return ret;
}



AnimTime::AnimTime(InStreamP in,CreatureP c,
                   const AnimTimeContext &cx,AnimTimeXdata &)
: Ability(in,c) {
  context = &cx;
}



void AnimTime::initialize(CreatureP c) {
  Ability::initialize(c);

  // Just so will assert if a class wasn't registered.
  lookup_context(cre->get_class_id());
}



AbilityId AnimTime::get_ability_id() {
  return AB_AnimTime;
}



int AnimTime::get_anim_time() {
  return context->animTime;
}



// Sucks ass, not extendable.
const AnimTimeContext *AnimTime::lookup_context(ClassId cId) {
  switch (cId) {
    case A_Seal:
      return &Seal::animTimeContext;
    case A_Walker:
      return &Walker::animTimeContext;
    case A_Yeti:
      return &Yeti::animTimeContext;
    case A_Zombie:
      return &Zombie::animTimeContext;
    default:
      assert(0);
  }
  return NULL;
}



AnimTimeXdata AnimTime::defaultXdata;



OnFire::OnFire(const OnFireContext &cx,OnFireXdata &)
:Ability() {
  context = &cx;

  // don't use OnFireXdata, AltarOfSin passes in dummy value.
}



DEFINE_CREATE_ABILITY_FROM_STREAM(OnFire)



OnFireContext OnFire::defaultContext;



OnFireXdata OnFire::defaultXdata;



OnFire::OnFire(InStreamP in,CreatureP c,
                   const OnFireContext &cx,OnFireXdata &)
:Ability(in,c) {
  context = &cx;
}



AbilityId OnFire::get_ability_id() {
  return AB_OnFire;
}



Boolean OnFire::heat_attack(PhysicalP,int,Boolean) {
  // We took care of it, do nothing.
  return True;
}



void OnFire::act() {
  // For composite objects, only the leader is OnFire.
  // May need something more general later.
  CompositeP comp = cre->get_composite();
  if (comp) {
    if (!comp->is_leader()) {
      return;
    }
  }

  // Dead object only burn if context->deadBurns is set.
  if (!(cre->alive() || context->deadBurns)) {
    return;
  }
  
  const Area &area = cre->get_area();
  Pos pos;
  Size size;
  area.get_rect(pos,size);
  LocatorP locator = cre->get_locator();
  
  // Generate fire.
  for (int n = 0; n < context->fires; n++) {
    Pos firePos(pos.x + Utils::choose(size.width),
                pos.y + Utils::choose((int)(.8 * size.height))); 
    
    PhysicalP fire = new Fire(cre->get_world(),locator,firePos,False);
    locator->add(fire);
  }
}



Boolean OnFire::collide(PhysicalP other) {
  // Don't burn things that we want to pick up.
  HolderP holder = cre->get_holder();
  if (holder && holder->ok_to_hold(other)) {
    return False;
  }

	if (cre->alive() && !other->is_shot()) {
		other->heat_attack(cre,context->heat);
		cre->attack_hook();
    return True;
	}
  return False;
}



SwapProtect::SwapProtect(const SwapProtectContext &cx,SwapProtectXdata &)
:Ability() {
  swapResistance = cx.swapResistance;
}



DEFINE_CREATE_ABILITY_FROM_STREAM(SwapProtect)



SwapProtectContext SwapProtect::defaultContext;



SwapProtectXdata SwapProtect::defaultXdata;



SwapProtect::SwapProtect(InStreamP in,CreatureP c,
                   const SwapProtectContext &,SwapProtectXdata &)
:Ability(in,c) {
  swapResistance = 0;
}



AbilityId SwapProtect::get_ability_id() {
  return AB_SwapProtect;
}



Boolean SwapProtect::swap_protect() {
	if (swapResistance > 0) {
		swapResistance--;
		return True;
	}
	return False;
}



Boolean SwapProtect::frog_protect() {
	return True;
}



Composite::Composite() 
: Ability() {
}



Composite::Composite(InStreamP in,CreatureP cre) 
: Ability(in,cre) {
}



Boolean Composite::is_composite() {
  return True;
}



PhysicalP Composite::get_draw_before_me() {
  return NULL;
}



CompositeId Composite::idGenerator = 0;



Segmented::Segmented(const SegmentedContext &cx,SegmentedXdata &)
:Composite() {
  context = &cx;
  isLeader = False;
  leaderAnimNum = 0;
  leaderAnimTimer.set_max(LEADER_ANIM_TIME);
  followDistance = context->followDistance;
}



DEFINE_CREATE_ABILITY_FROM_STREAM(Segmented)



Segmented::Segmented(InStreamP in,CreatureP c,
                     const SegmentedContext &cx,SegmentedXdata &)
:Composite(in,c) {
  context = &cx;
  isLeader = False;
  update_from_stream(in);
  // don't bother with leaderAnimTimer
  // followDistance will come from update_from_stream().
}



void Segmented::update_from_stream(InStreamP in) {
  isLeader = (Boolean)in->read_char();
  next.read(in);
  leaderAnimNum = (int)in->read_char();
  followDistance = (int)in->read_short();
}



int Segmented::get_write_length() {
  return 
    sizeof(char) +    // isLeader
    Identifier::get_write_length() + // next, needed for get_draw_before_me()
    sizeof(char) +    // leaderAnimNum
    sizeof(u_short);  // followDistance
}



void Segmented::write(OutStreamP out) {
  out->write_char(isLeader);
  next.write(out);
  out->write_char((u_char)leaderAnimNum);
  assert(followDistance < USHRT_MAX);
  out->write_short((u_short)followDistance);
}



AbilityId Segmented::get_ability_id() {
  return AB_Segmented;
}



void Segmented::get_followers(PtrList &list) {
  if (cre->alive()) {
    LocatorP l = cre->get_locator();
    PhysicalP nextP = l->lookup(next);
    if (nextP) {
      if (nextP->alive() 
          && nextP->is_creature() 
          && nextP->get_ability(AB_Segmented)) {
        list.add(nextP);
      }
    }
  }  
}



Boolean Segmented::follow(const Area &followeeArea,Dir,
                          Boolean currentAndNext) {
  // Make cre follow followeeArea
  const Area &area = cre->get_area();
  Pos fPos = followeeArea.get_middle();
  Pos pos = area.get_middle();
  int dist_2 = pos.distance_2(fPos);
  
  // Don't get too close.
  if (dist_2 > followDistance * followDistance) {
    // Kinda expensive, a sqrt() and a divide.
    float dist = (float)sqrt((double)dist_2);
    float invert_dist = 1.0f / dist;
    // delta points from followee to cre
    Vel delta((float)(pos.x - fPos.x),(float)(pos.y - fPos.y));

    // Move along vector from nPos to pos.
    Pos newPos = fPos + (followDistance * invert_dist) * delta;

    // The "approved" way of setting the next pos for ::follow().
    // Don't use set_middle_next(), because that won't handle currentAndNext
    // properly.  _set_middle_next() exists for this purpose.
    cre->_set_middle_next(newPos);
    cre->update_area_next(currentAndNext);
  }       
  // else if already too close, don't worry about it.

  // True even if we are too close.  In either case, we handled it.
  return True;
}



Boolean Segmented::get_pixmap_mask(Xvars &xvars,int dpyNum,CMN_IMAGEDATA &pixmap,
			                            Dir dir,int animNum) {
  Dir mappedDir;
  int mappedAnimNum;
  map_dir_anim_num(mappedDir,mappedAnimNum,dir,animNum);

  // Call non-virtual Creature::_get_pixmap_mask, to avoid recursion.
  cre->_get_pixmap_mask(xvars,dpyNum,pixmap,mappedDir,mappedAnimNum);

  return True;
}



void Segmented::set_intel(IntelP intel) {
  LocatorP l = cre->get_locator();

  // Is this the head of a chain of null intels.
  // That is the only thing where a Human is allowed to control
  // more than just one segment.
  Boolean headOfNullIntels = True;
  if (l->lookup(prev)) {
    headOfNullIntels = False;
  }
  else {
    PhysicalP p = cre;
    while (p) {
      if (p->get_intel()) {
        headOfNullIntels = False;
        break;
      }

      SegmentedP s = (SegmentedP)p->get_ability(AB_Segmented);
      if (s) {
        p = l->lookup(s->next);
        // Success as soon as p is NULL here.
      }
      // This shouldn't happen.
      else {
        headOfNullIntels = False;
        break;
      }
    }
  }
  // Another hack here, this doesn't work properly if the locator hasn't 
  // been clocked yet, but doesn't break anything.  When called from 
  // Game::create_human_and_physical().
  

  // kind of a hack.
  // If a human takes over, change the compositeId.
  if (!headOfNullIntels && intel && intel->is_human()) {
    CompositeId newCId = Composite::generate_id();
    set_composite_id(newCId);
  }

  // TODO: If a human soul-swaps into a dragon, do 
  // something smart like turn all the 
  // following segments into doppelgangers, or make them all 
  // harmless, or remove their intel.  Careful not to fuck up the
  // locator's registry of intels.
}



void Segmented::act() {
  LocatorP l = cre->get_locator();

  // Disconnect prev segment if not valid.
  PhysicalP p = l->lookup(prev);
  if (p && !compatible_prev(p)) {
    prev.invalidate();
    SegmentedP segPrev = (SegmentedP)p->get_ability(AB_Segmented);
    if (segPrev) {
      segPrev->next.invalidate();
    }
  }

  // Become leader if prev has died or has become incompatible.
  // Must be after checking for disconnecting prev segment.
  if (!isLeader) {
    if (!l->lookup(prev)) {
      assume_leadership();
    }
  }  

  // Disconnect next segment if not valid.
  PhysicalP n = l->lookup(next);
  if (n && !compatible_next(n)) {
    next.invalidate();
    SegmentedP segNext = (SegmentedP)n->get_ability(AB_Segmented);
    if (segNext) {
      segNext->prev.invalidate();
    }
  }

  leaderAnimTimer.clock();
}



void Segmented::update() {
  if (leaderAnimTimer.ready()) {
    const MovingContext* mc = cre->get_moving_context();
    // Choose the leader animNum randomly.
    // We will probably want to make this behavior more general if we use
    // Segmented for more than just Dragon.
    leaderAnimNum = Utils::choose(mc->animMax[LEADER_DIR]);

    leaderAnimTimer.set();
  }    
}



Boolean Segmented::compatible_prev(PhysicalP p) {
  assert(p);

  // Only a null intel can follow another null intel.
  IntelP intel = cre->get_intel();
  IntelP prevIntel = p->get_intel();
  if (intel && !prevIntel) {
    return False;
  }

  if (!p->alive() || !cre->alive()) {
    return False;
  }

  SegmentedP seg = (SegmentedP)p->get_ability(AB_Segmented);
  if (!seg) {
    return False;
  }
  // Disconnect if composite ids don't match.
  if (seg->get_composite_id() != get_composite_id()) {
    return False;
  }

  // A human cannot follow anything except null.
  if (intel && intel->is_human() && prevIntel) {
    return False;
  }  
  // Ok for a non-human to follow.
  // Machine::clock() will cause non-humans to do nothing if not the
  // leader.

  return True;
}



Boolean Segmented::compatible_next(PhysicalP n) {
  assert(n);

  // Only a null intel can follow another null intel.
  IntelP intel = cre->get_intel();
  IntelP nextIntel = n->get_intel();
  if (!intel && nextIntel) {
    return False;
  }

  if (!n->alive() || !cre->alive()) {
    return False;
  }

  SegmentedP seg = (SegmentedP)n->get_ability(AB_Segmented);
  if (!seg) {
    return False;
  }

  // Disconnect if composite ids don't match.
  if (seg->get_composite_id() != get_composite_id()) {
    return False;
  }

  // human can only be followed by null intel.
  if (intel && intel->is_human() && nextIntel) {
    return False;
  }  
  // Ok for a non-human to follow a human, 
  // Machine::clock() will cause non-humans to do nothing if not the
  // leader.

  return True;
}



Boolean Segmented::is_leader() {
  return isLeader;
}



PhysicalP Segmented::get_draw_before_me() {
  // Always draw the Segmented object from tail to head.
  LocatorP l = cre->get_locator();
  PhysicalP p = l->lookup(next);

  // May be NULL.
  return p;
}



void Segmented::create_and_add_composite(PtrList &ret,WorldP world,
                                         LocatorP locator,
                                         int segmentsNum,const Pos &pos,
  PhysicalP (*create)(void *arg,WorldP w,LocatorP l,const Pos &pos),
                                         void *arg) {
  assert(segmentsNum > 0 && ret.length() == 0);

  CompositeId cId = generate_id();
  ret.fill(segmentsNum);

  // Create chain from back to front.  
  PhysicalP nextSegment = NULL; // The segment just created.
  for (int n = 0; n < segmentsNum; n++) {
    // Create them all in the same place.  No reason not to.
    PhysicalP obj = create(arg,world,locator,pos);
    assert(obj && obj->is_creature());

    // Mark all objects in the chain as being part of the same composite 
    // Creature.
    SegmentedP seg = (SegmentedP)obj->get_ability(AB_Segmented);
    // The create() function better create something with the Segmented abiltiy.
    assert(seg);
    seg->set_composite_id(cId);

    // Must add before seg->insert_before() so has valid Id.
    locator->add(obj);
    if (nextSegment) {
      SegmentedP seg = (SegmentedP)obj->get_ability(AB_Segmented);
      assert(seg);
      seg->insert_before((CreatureP)nextSegment);
    }
    nextSegment = obj;

    // Insert in reverse order so head is at element 0.
    ret.set(segmentsNum - 1 - n,obj);
  }

  // nextSegment ends up pointing at the beginning of the chain.
  assert(nextSegment);
}



void Segmented::insert_before(CreatureP nextC) {
  LocatorP l = cre->get_locator();
  SegmentedP nextS = (SegmentedP)nextC->get_ability(AB_Segmented);
  assert(nextS);

  Id id = cre->get_id();
//  assert(l->lookup(id));
  Id nextId = nextC->get_id();
//  assert(l->lookup(nextId));

  // Generic linked list insert.
  prev = nextS->prev;
  PhysicalP prevP = l->lookup(prev);
  if (prevP) {
    SegmentedP prevS = (SegmentedP)prevP->get_ability(AB_Segmented);
    assert(prevS);
    assert(prevS->next == nextId); // Or chain was broken.
    prevS->next = id;  
  }
  nextS->prev = id;
  next = nextId;

  // Each segment agrees not to collide with the next one.
  cre->set_dont_collide(nextC->get_id());
}



void Segmented::assume_leadership() {
// Doesn't work when you shoot the head.
#if 0
  // kind of a hack.
  // If a human takes over, change the compositeId.
  IntelP intel = cre->get_intel();
  if (intel && intel->is_human()) {
    LocatorP l = cre->get_locator();
    CompositeId newCId = Composite::generate_id();

    // Don't recurse, human will only control one segment.
    PhysicalP p = cre;
//    while (p) {        
      SegmentedP seg = (SegmentedP)p->get_ability(AB_Segmented);
      assert(seg);
      seg->set_composite_id(newCId);
//      p = l->lookup(seg->get_next());
    //}
  }
#endif
  isLeader = True;
}



void Segmented::map_dir_anim_num(Dir& mappedDir,int& mappedAnimNum,Dir dir,int animNum) {
  mappedDir = dir;
  mappedAnimNum = animNum;

  // Map direction to get leader pixmaps.  Don't remap dead pixmaps.
  if (dir != Creature::DEAD_DIR && is_leader()) {
    mappedAnimNum = leaderAnimNum;
    mappedDir = LEADER_DIR;
  }
}



SegmentedContext Segmented::defaultContext;



SegmentedXdata Segmented::defaultXdata;



Sensitive::Sensitive(const SensitiveContext &cx,SensitiveXdata &)
:Ability() {
  context = &cx;
}



Sensitive::Sensitive(InStreamP in,CreatureP c,
                     const SensitiveContext &cx,SensitiveXdata &)
:Ability(in,c) {
  context = &cx;
  
  // Not yet implemented.
  assert(context->corporealMultiplier == 0);
}



DEFINE_CREATE_ABILITY_FROM_STREAM(Sensitive)



AbilityId Sensitive::get_ability_id() {
  return AB_Sensitive;
}



Boolean Sensitive::heat_attack(PhysicalP killer,int heat,Boolean secondary) {
  if (context->heatMultiplier == 0) {
    return False;
  }

  // Multiply the heat by heatMultiplier.
  cre->_heat_attack(killer,context->heatMultiplier * heat,secondary);
  // Ignoring the value returned by Creature::_heat_attack().

  return True;
}



SensitiveContext Sensitive::defaultContext;



SensitiveXdata Sensitive::defaultXdata;
