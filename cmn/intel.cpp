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

// "intel.cpp"


#if X11
#ifndef NO_PRAGMAS
#pragma implementation "intel.h"
#endif
#endif


// Include Files
#include "stdafx.h"
#include <cstring>
#if X11
#include <strstream>
#endif
#if WIN32
#include <strstrea.h>
#endif


#include "utils.h"
#include "physical.h"
#include "intel.h"
#include "actual.h"
#include "xetp.h"
#include "xdata.h"

using namespace std;

// Defines
#define FIGHT_RANGE 40 // made it smaller for Dogs 45  // [50-40]
#define FIGHT_RANGE_2 (FIGHT_RANGE * FIGHT_RANGE)
#define WANDER_WIDTH 1000
#define WANDER_HEIGHT 600
#define SHOT_CUTOFF 9
#define LADDER_JUMP_TIME 4
#define FTHROWER_RANGE_2 (90 * 90)
#define HEALTH_ATTACK_PERCENT .1
#define TRANSMOGIFIER_PERCENT .15
// For DoppelIntel
#define MASTER_MIN_DIST 100 //50
#define MASTER_MIN_DIST_2 (MASTER_MIN_DIST * MASTER_MIN_DIST)
#define MASTER_MAX_DIST  400 // 600  //   350 
#define MASTER_MAX_DIST_2 (MASTER_MAX_DIST * MASTER_MAX_DIST)
// Strategy change times for different strategies.
#define TO_AWAY_TARGET_TIME 30
#define TO_POS_TIME 100
#define TO_MASTER_TIME 100
#define DEFAULT_REFLEXES_TIME 4 // for seals, enemies, doppelgangers
#define PET_REFLEXES_MULTIPLIER 0.5 // better than average.
#define DROP_ITEM_CHANCE 50
#define DOPPEL_SUICIDE_TIME (60 * 25) // 60 sec
#define REFLEXES_TWEAK_CHANCE 20
// If in the same place for this many cycles, (cycles == turns * reflexes),
// then become bored.
#define BOREDOM_CYCLES 5



// Don't change these defaults.
// If you have to, at least update the comments on "struct IntelOptions".
const IntelOptions Intel::intelOptionsDefault = {
  True,  // classFriends
  False, // harmless
  False, // psychotic
  False, // ignoreItems
  0,     // limitedLifespan
};



void IntelStatus::read(InStreamP in) {
  Utils::string_read(in,name,IT_STRING_LENGTH);
  classId = in->read_int();
  Utils::string_read(in,className,IT_STRING_LENGTH);
  health = in->read_int();
  healthmax = in->read_int();
  mass = in->read_int();
  weaponClassId = in->read_int();
  Utils::string_read(in,weapon,IT_STRING_LENGTH);
  weaponReady = (Boolean)in->read_char();
  ammo = in->read_int();
  itemClassId = in->read_int();
  Utils::string_read(in,item,IT_STRING_LENGTH);
  itemCount = in->read_int();
  lives = in->read_int();
  humanKills = in->read_int();
  enemyKills = in->read_int();
  soups = in->read_int();
}



int IntelStatus::get_write_length() {
  return 
    Utils::get_string_write_length(name) +        // name
    sizeof(int) +                                 // classId
    Utils::get_string_write_length(className) +   // className
    sizeof(int) +                                 // health
    sizeof(int) +                                 // healthMax
    sizeof(int) +                                 // mass
    sizeof(int) +                                 // weaponClassId
    Utils::get_string_write_length(weapon) +      // weapon
    sizeof(char) +                                // weaponReady
    sizeof(int) +                                 // ammo
    sizeof(int) +                                 // itemClassId
    Utils::get_string_write_length(item) +        // item
    sizeof(int) +                                 // itemCount
    sizeof(int) +                                 // lives
    sizeof(int) +                                 // humanKills
    sizeof(int) +                                 // enemyKills
    sizeof(int);                                  // soups
}



void IntelStatus::write(OutStreamP out) {
  Utils::string_write(out,name);
  out->write_int(classId);
  Utils::string_write(out,className);
  out->write_int(health);
  out->write_int(healthmax);
  out->write_int(mass);
  out->write_int(weaponClassId);
  Utils::string_write(out,weapon);
  out->write_char((char)weaponReady);
  out->write_int(ammo);
  out->write_int(itemClassId);
  Utils::string_write(out,item);
  out->write_int(itemCount);
  out->write_int(lives);
  out->write_int(humanKills);
  out->write_int(enemyKills);
  out->write_int(soups);
}



Boolean IntelStatus::operator == (const IntelStatus &is) {
  if (strcmp(name,is.name)) {
    return False;
  }
  if (classId != is.classId) {
    return False;
  }
  if (strcmp(className,is.className)) {
    return False;
  }
  if (health != is.health) {
    return False;
  }
  if (healthmax != is.healthmax) {
    return False;
  }
  if (mass != is.mass) {
    return False;
  }
  if (weaponClassId != is.weaponClassId) {
    return False;
  }
  if (strcmp(weapon,is.weapon)) {
    return False;
  }
  if (weaponReady != is.weaponReady) {
    return False;
  }
  if (ammo != is.ammo) {
    return False;
  }
  if (itemClassId != is.itemClassId) {
    return False;
  }
  if (strcmp(item,is.item)) {
    return False;
  }
  if (itemCount != is.itemCount) {
    return False;
  }
  if (lives != is.lives) {
    return False;
  }
  if (humanKills != is.humanKills) {
    return False;
  }
  if (enemyKills != is.enemyKills) {
    return False;
  }
  if (soups != is.soups) {
    return False;
  }
  return True;
}



Intel::Intel(WorldP w,LocatorP l,char *name,
	           const IntelOptions *ops,ITmask opMask) {
  intelStatusChanged = True;
  living = True;
  world = w;
  locator = l;

  intelOptions = intelOptionsDefault;
  if (opMask & ITharmless) {
    intelOptions.harmless = ops->harmless;
  }
  if (opMask & ITclassFriends) {
    intelOptions.classFriends = ops->classFriends;
  }
  if (opMask & ITpsychotic) {
    intelOptions.psychotic = ops->psychotic;
  }
  if (opMask & ITignoreItems) {
    intelOptions.ignoreItems = ops->ignoreItems;
  }
  if (opMask & ITlimitedLifespan) {
    intelOptions.limitedLifespan = ops->limitedLifespan;
  }

  // intelStatus
  assert(Utils::strlen(name) < IT_STRING_LENGTH);
  Utils::strcpy(intelStatus.name,name); 
  intelStatus.classId = A_None;
  Utils::strcpy(intelStatus.className,"none");
  intelStatus.health = 0;
  intelStatus.mass = 0;
  intelStatus.weaponClassId = A_None;
  Utils::strcpy(intelStatus.weapon,"none");
  intelStatus.weaponReady = False;
  intelStatus.ammo = PH_AMMO_UNLIMITED;
  intelStatus.itemClassId = A_None;
  Utils::strcpy(intelStatus.item,"none");
  intelStatus.itemCount = 0;

  intelStatus.lives = -666; // This should never be used.
  intelStatus.humanKills = 0;
  intelStatus.enemyKills = 0;
  intelStatus.soups = 0;

  // dontCollide starts out as invalid.

  // Empty modifier list, mark as the head.
  modifiers = new Modifier(0,True);
  assert(modifiers);

  creationOk = True;
}



Intel::Intel(InStreamP in,WorldP w,LocatorP l) {
  creationOk = False;

  memset(&intelStatus,'\0',sizeof(intelStatus));
  world = w;
  locator = l;
  intelOptions = intelOptionsDefault;

  // dontCollide starts out as invalid.

  // Empty modifier list, mark as the head.
  modifiers = new Modifier(0,True);
  assert(modifiers);

  update_from_stream(in);
}



void Intel::update_from_stream(InStreamP in) {
  /* intelStatus */
  intelStatus.read(in);

  // Always true.
  intelStatusChanged = True;

  /* living */
  living = (Boolean)in->read_char();

  /* id */
  id.read(in);

  /* intelId */
  intelId.read(in);

  creationOk = in->alive();
}



int Intel::get_write_length() {
  return
    intelStatus.get_write_length() +  // intelStatus
    sizeof(char) +                    // living
    Identifier::get_write_length() +  // id
    Identifier::get_write_length();   // intelId
}



void Intel::write(OutStreamP out) {
  intelStatus.write(out);
  out->write_char((char)living);
  id.write(out);
  intelId.write(out);
}



Intel::~Intel() {
  // Deletes entire list.
  delete modifiers;
}



Boolean Intel::is_human() {
  return False;
}



Boolean Intel::is_enemy() {
  return False;
}



Boolean Intel::is_seal_intel() {
  return False;
}



Boolean Intel::is_pet() {
  return False;
}



Boolean Intel::reincarnate_me()
{
  int lives = _get_lives();
  return !living && (lives == IT_INFINITE_LIVES || lives > 0);
}



const IntelStatus *Intel::get_intel_status() {
  intelStatusChanged = False; 
  // Set intelStatus.lives to the real value, which may depend on children.
  intelStatus.lives = _get_lives();
  return &intelStatus;
}



Boolean Intel::is_playing() {
  if (alive() || get_lives() == IT_INFINITE_LIVES || get_lives() > 0) {
    return True;
  }
  return False;
}



void Intel::set_name(char *newName) {
  assert(newName && (strlen(newName) < IT_STRING_LENGTH));
  strcpy(intelStatus.name,newName);
  
  intelStatusChanged = True;
}


 
void Intel::add_human_kill() {
  intelStatus.humanKills++; 
  intelStatusChanged = True;
}



void Intel::add_enemy_kill() {
  intelStatus.enemyKills++; 
  intelStatusChanged = True;
}



void Intel::add_soup() {
  intelStatus.soups++; 
  intelStatusChanged = True;
}



ITcommand Intel::dir_to_command(Dir dir) {
  assert(dir == CO_air || dir >= CO_R);
  ITcommand ret = IT_CENTER;
  if (dir != CO_air) {
    ret = (ITcommand)((dir - CO_R) * 0.5 + IT_R);
  }
  
  return ret;
}



ITcommand Intel::dir_to_command_weapon(Dir dir) {
  assert(dir == CO_air || dir >= CO_R);
  ITcommand ret = IT_WEAPON_CENTER;
  if (dir != CO_air)
    ret = (ITcommand)((dir - CO_R) * 0.5 + IT_WEAPON_R);
  
  return ret;
}



Dir Intel::command_weapon_to_dir_4(ITcommand command) {
  switch (command) {
  case IT_WEAPON_R:
    return CO_R;
  case IT_WEAPON_DN:
    return CO_DN;
  case IT_WEAPON_L:
    return CO_L;
  case IT_WEAPON_UP:
    return CO_UP;
  }
  
  return CO_air;
}



Dir Intel::command_weapon_to_dir_8(ITcommand command) {
  switch (command) {
  case IT_WEAPON_R:
    return CO_R;
  case IT_WEAPON_DN_R:
    return CO_DN_R;
  case IT_WEAPON_DN:
    return CO_DN;
  case IT_WEAPON_DN_L:
    return CO_DN_L;
  case IT_WEAPON_L:
    return CO_L;
  case IT_WEAPON_UP_L:
    return CO_UP_L;
  case IT_WEAPON_UP:
    return CO_UP;
  case IT_WEAPON_UP_R:
    return CO_UP_R;
  }

  return CO_air;
}




ITcommand Intel::center_pos_to_command(const Pos &pos) {
  if (pos.y > pos.x) {
   if (pos.y > -pos.x) {
      return IT_DN;
    }
    else {
      return IT_L;
    }
  }
  else {
    if (pos.y > -pos.x) {
      return IT_R;
    }
    else {
      return IT_UP;
    }
  }
}



Size Intel::command_to_size(ITcommand command,int mag) {
  Size ret;
  ret.set_zero();
  
  switch (command) {
  // Right
  case IT_R:
    ret.width = mag;
	break;
  // Down right.
  case IT_DN_R:
	ret.width = mag;
	ret.height = mag;
	break;
  // Down.
  case IT_DN:
	ret.height = mag;
	break;
  // Down left.
  case IT_DN_L:
	ret.width = -mag;
	ret.height = mag;
	break;
  // Left.
  case IT_L:
	ret.width = -mag;
	break;
  // Up left.
  case IT_UP_L:
	ret.width = -mag;
	ret.height = -mag;
	break;
  // Up.
  case IT_UP:
	ret.height = -mag;
	break;
  // Up right.
  case IT_UP_R:
	ret.width = mag;
	ret.height = -mag;
    break;
  // Else let it be (0,0).
  }

  return ret;
}
  


void Intel::die() {
  intelStatus.health = -1;  
  intelStatus.classId = A_None;
  strcpy(intelStatus.className,"tormented spirit");
  intelStatus.mass = 0;
  intelStatus.weaponClassId = A_None;
  strcpy(intelStatus.weapon,"none");
  intelStatus.weaponReady = False;
  intelStatus.ammo = PH_AMMO_UNLIMITED;
  intelStatus.itemClassId = A_None;
  strcpy(intelStatus.item,"none");

  intelStatusChanged = True; 

  // Want to keep id valid so can lookup to deliver messages
  // after Player is killed.
  // id.invalidate();
  living = False;

  // Don't keep drug high or doubleSpeed, etc. when you die.
  modifiers->clear();
}



void Intel::reincarnate() {
  int lives = _get_lives();
  if (lives != IT_INFINITE_LIVES) {
    // If multiple players loose last life at the same time, they all get
    // to come back from the dead.  To fix this right, reincarnate_me() or
    // something like it would have to grab a lock on the life.  
    // Locator starts reincarnating both of them.
    if (lives > 0) {
      _set_lives(lives - 1);
    }
    intelStatusChanged = True;
  }
  living = True;
}



void Intel::clock(PhysicalP p) {
  id = p->get_id();

  //// Update intelStatus
  
  // classId, class
  ClassId classId = p->get_class_id();
  if (classId != intelStatus.classId) {
    intelStatus.classId = classId;
    strcpy(intelStatus.className,p->get_class_name());
    intelStatusChanged = True;
  }

  // health
  Health health = p->get_health();
  if ((health >= 0) && (health != intelStatus.health)) {
    intelStatus.health = health;
    intelStatusChanged = True;
  }
  // healthMax
  health=p->get_health_max();
  if (health!=intelStatus.healthmax) {
    intelStatus.healthmax = health;
    intelStatusChanged = True;
  }
  
  // mass
  Mass mass = p->get_mass();
  if (mass != intelStatus.mass) {
    intelStatus.mass = mass;
    intelStatusChanged = True;
  }

  // weaponClassId, weapon
  const char *weapon;
  ClassId wClassId = p->get_weapon_string(weapon);
  if (wClassId != intelStatus.weaponClassId) {
    intelStatus.weaponClassId = wClassId;
    strcpy(intelStatus.weapon,(char *)weapon);
    intelStatusChanged = True;
  }

  HolderP holder = NULL;
  CreatureP cre = NULL;
  if (p->is_creature()) {
    cre = ((CreatureP)p);
    holder = cre->get_holder();
  }

  if (holder) {
    assert(cre);

    PhysicalP weapon = holder->get_weapon_current();
    Boolean weaponReady = False;
    BuiltInP builtIn;
    
    // weaponReady
    if (weapon) {
      weaponReady = ((WeaponP)weapon)->ready();
    }
    // WARNING: code duplication below.
    else if (cre->get_ability(AB_Fighter)) {
      weaponReady = True;
    }
    // this code assumes that no Creature has both the Fighter and 
    // BuiltIn capabilities
    else if (builtIn = (BuiltInP)cre->get_ability(AB_BuiltIn)){
      if (builtIn->ready()) {
        weaponReady = True;
      }
    }
    if (intelStatus.weaponReady != weaponReady) {
	    intelStatus.weaponReady = weaponReady;
	    intelStatusChanged = True;
	  }

    // ammo
    int ammo = weapon ? ((WeaponP)weapon)->get_ammo() : PH_AMMO_UNLIMITED;
    if (intelStatus.ammo != ammo) {
      intelStatus.ammo = ammo;
      intelStatusChanged = True;
    }
  
    // itemClassId, item
    PhysicalP item = holder->get_item_current();
    ClassId itemClassId = item ? item->get_class_id() : A_None;
    if (intelStatus.itemClassId != itemClassId) {
	    intelStatus.itemClassId = itemClassId;
	    strcpy(intelStatus.item,item ? item->get_class_name() : "none");
	    intelStatusChanged = True;
	  }
  
    // itemCount
    int itemCount = holder->get_item_count();
    if (intelStatus.itemCount != itemCount) {
      intelStatus.itemCount = itemCount;
      intelStatusChanged = True;
    }
  }

  // Not Holder, some code duplication.
  else { 
    
    // weaponReady
    Boolean weaponReady = False;
    BuiltInP builtIn;
    if (cre && cre->get_ability(AB_Fighter)) {
      weaponReady = True;
    }
    // this code assumes that no Creature has both the Fighter and 
    // BuiltIn capabilities
    else if (cre && (builtIn = (BuiltInP)cre->get_ability(AB_BuiltIn))){
      if (builtIn->ready()) {
        weaponReady = True;
      }
    }
    if (intelStatus.weaponReady != weaponReady) {
      intelStatus.weaponReady = weaponReady;
      intelStatusChanged = True;
    }
  
    // ammo
    if (intelStatus.ammo != PH_AMMO_UNLIMITED) {
      intelStatus.ammo = PH_AMMO_UNLIMITED;
      intelStatusChanged = True;
    }
  
    // itemClassId, item
    // assumes that no Creature has both Holder and Suicide
    //    
    // Maybe we should have a get_item_string() like get_weapon_string()?
    if (cre && cre->get_ability(AB_Suicide)) {
      if (intelStatus.itemClassId != A_SuicideButton) {
        intelStatus.itemClassId = A_SuicideButton;
        strcpy(intelStatus.item,"suicide button");
        intelStatusChanged = True;
      }
    }
    else if (intelStatus.itemClassId != A_None) {
      intelStatus.itemClassId = A_None;
      strcpy(intelStatus.item,"none");
      intelStatusChanged = True;
    }

    // itemCount  
    intelStatus.itemCount = 0;
  }

  // Kills does not depend on the physical.
}



int Intel::_get_lives() {
  return 1;
}



void Intel::_set_lives(int)
{}



Human::Human(WorldP w,LocatorP l,char *name,int lves,
             int *sharedLves,ColorNum cNum)
: Intel(w,l,name,NULL,ITnone) {
  lives = lves;
  sharedLives = sharedLves;  

  // One life is used for current life.  
  int lv = Human::_get_lives();
  if (lv > 0) {
    // Avoid funny bug when new client connects to COOPERATIVE game with zero
    // lives remaining.  It used to set lives to -1 which is infinite lives.
    Human::_set_lives(lv - 1);
  }

  command = IT_NO_COMMAND;
  noRepeat = IT_NO_COMMAND;
  assert(cNum < Xvars::HUMAN_COLORS_NUM);
  colorNum = cNum;
  data = NULL;
}



Human::Human(InStreamP in,WorldP w,LocatorP l)
: Intel(in,w,l) {
  command = IT_NO_COMMAND;
  noRepeat = IT_NO_COMMAND;
  data = NULL;

  // Just reflect the value of lives.
  sharedLives = NULL;
  
  _update_from_stream(in);
}



void Human::update_from_stream(InStreamP in) {
  Intel::update_from_stream(in);
  _update_from_stream(in);
}



void Human::_update_from_stream(InStreamP in) {
  colorNum = in->read_short();
  u_short lvs = in->read_short();
  if (lvs == 0xffff) {
    lives = IT_INFINITE_LIVES;
  }
  else {
    lives = lvs;
  }
}



int Human::get_write_length() {
  return
    Intel::get_write_length() +
    sizeof(short) +             // colorNum
    sizeof(short);              // lives
}



void Human::write(OutStreamP out) {
  Intel::write(out);
  assert(colorNum <= USHRT_MAX);
  out->write_short((u_short)colorNum);

  int lvs;
  if (sharedLives) {
    lvs = *sharedLives;
  }
  else {
    lvs =lives;
  }
  if (lvs == IT_INFINITE_LIVES) {
    out->write_short(0xffff);
  }
  else {
    // strictly less than, because 0xffff means IT_INFINITE_LIVES
    assert(lvs < USHRT_MAX); 
    out->write_short((u_short)lvs);
  }
}



Boolean Human::is_human()
{
  return True;
}



void Human::set_command(ITcommand c) {
  command = c;

#if 0
  if (netOut) {
    XETP::send_command(netOut,get_intel_id(),command);
    // Want to send this immediately.
    if (netOut->get_protocol() == GenericStream::UDP) {
      ((UDPOutStreamP)netOut)->flush();
    }
  }
#endif
}



void Human::clock(PhysicalP p) { 
  // A little sanity check.
  int *livesPtr = sharedLives;
  if (!livesPtr) {
    livesPtr = &lives;
  }
  assert(*livesPtr == IT_INFINITE_LIVES || *livesPtr >= 0);

  assert(alive());
  // Disable old noRepeat if we didn't get the same command again.
  if (noRepeat != IT_NO_COMMAND && command != noRepeat) {
    noRepeat = IT_NO_COMMAND;
  }

  // Here is where noRepeat actually blocks the command from being executed.
  if (noRepeat == command) {
    command = IT_NO_COMMAND;
  }

  // Set the command this turn, but disallow it until some other command 
  // comes in.
  // Set the new noRepeat.
  if (command != IT_NO_COMMAND && !p->command_repeatable(command)) {
    noRepeat = command;
  }
  // We always call set_command, we need to set it even for IT_NO_COMMAND.
  p->set_command(command); 
  command = IT_NO_COMMAND; 
  Intel::clock(p);
}



int Human::_get_lives() {
  if (sharedLives) {
    return *sharedLives;
  }
  return lives;
}



void Human::_set_lives(int val) {
  if (_get_lives() != IT_INFINITE_LIVES) {
    if (sharedLives) {
      // Set shared value.
      *sharedLives = val;
      
      // Mark all Humans dirty just to be sure.
      LocatorP l = get_locator();
      for (int n = 0; n < l->humans_registered(); n++) {
        HumanP human = l->get_human(n);
        human->make_intel_status_dirty();
      }
    }
    else {
      lives = val;
    }
  }
}



Boredom::Boredom() {
  // Don't really need to do this.
  timer.set(BOREDOM_CYCLES);
}



Boolean Boredom::check(LocatorP l,PhysicalP p) {
  assert(p);
  const Area& area = p->get_area();
  Pos middle = area.get_middle();

  // Don't get bored if p's position has changed, or if p has changed.
  PhysicalP lastP = l->lookup(lastId);
  if (p != lastP || middle != lastMiddle) {
    timer.set(BOREDOM_CYCLES);
    lastId = p->get_id();
    lastMiddle = middle;
    timer.clock();  // Don't really need to do this.
    return False;
  }
  
  // Don't need to update lastP or lastMiddle because we know they haven't
  // changed.
  if (timer.ready()) {
    // Will remain bored until p or p's position changes.
    return True;
  }

  // Get one cycle more bored.
  timer.clock();
  return False;
}



Machine::Machine(WorldP w,LocatorP l,char *name,const IntelOptions *ops,
		 ITmask opMask) 
: Intel(w,l,name,ops,opMask) {
  // strategyChange uses default initializer.
  Timer oTimer(LADDER_JUMP_TIME);
  ladderJump = oTimer;
  strategy = doNothing;
}



IntelId Machine::get_master_intel_id() {
  IntelId invalid;
  return invalid;
}



void Machine::clock(PhysicalP p) {
  assert(alive() && p->is_creature());
  const IntelOptions &ops = get_intel_options();
  LocatorP locator = get_locator();
  Boolean commandSet = False;
  
  if (reflexes.ready()) {    
    // Composite objects don't do their own movement unless they are the
    // leader.
    CompositeP composite = p->get_composite();
    if (!composite || composite->is_leader()) {
      use_items(commandSet,p);

      // Check for boredom, if bored, 
      Boolean bored = boredom.check(get_locator(),p);
      if (bored) {
        if (strategy_to_random_pos(p)) {
          strategyChange.set(TO_POS_TIME);
        }
      }
      // Normal case, possibly set a new strategy.
      else {
        // May just leave strategy alone.
        choose_strategy(p);
      }

      // Instead of just sitting there, go somewhere randomly.
      if (strategy == doNothing) {
        if (strategy_to_random_pos(p)) {
          strategyChange.set(TO_POS_TIME);
        }
      }

    
      // Act on current strategy.
      switch (strategy) {
      case doNothing:
        if (!commandSet) {
          p->set_command(IT_NO_COMMAND);
          commandSet = True;
        }
        break;
      
      case toPos:
        if (!commandSet) {
          commandSet = move_pos(p,targetPos,ladderJump);
          if (!commandSet) {
            // We got there, do something else.
            strategyChange.set(0);
          }
        }
        break;
      
      case toTarget: {
        PhysicalP target = locator->lookup(targetId);
        if (!commandSet && target) {
          commandSet = move_target(p,target,ladderJump);
        }
      }
      break;
    
      case awayTarget: {
        PhysicalP target = locator->lookup(targetId);
        if (!commandSet && target) {
          commandSet = away_target(p,target,ladderJump);
        }
      }
      break;
    
      case attackTarget: {
        PhysicalP target = locator->lookup(targetId);
      
        // Check is_creature so won't attack items.
        if (!commandSet && !ops.harmless && target && target->is_creature()) {
          commandSet = attack_target(p,target);
        }
      
        if (!commandSet && target) {
          commandSet = move_target(p,target,ladderJump);
        }
      }
      break;
    
      case retreatTarget: {
        PhysicalP target = locator->lookup(targetId);
      
        // Check is_creature so won't attack items.
        if (!commandSet && !ops.harmless && target && target->is_creature()) {
          commandSet = attack_target(p,target);
        }
      
        if (!commandSet && target) {
          commandSet = away_target(p,target,ladderJump);
        }
      }
      break;
    
      case toDir:
        if (!commandSet) {
          commandSet = move_dir(p,targetDir,ladderJump);
        }
        break;
      }
    } // composite check
   
    int rflexes = _get_reflexes_time(); 
    // Fix the dog stuck on corner bug.  Tweak the value for reflexes
    // every once in a while so we don't end up with a case where 
    // the Machine intel is active with the same frequency as a 
    // looping physics phenomena.  In this case, Dog pets had a reflexes
    // time of 2 and the bouncing on the corner happened every other turn,
    // so the dog intel was only active when it was in the air.
    if (Utils::choose(REFLEXES_TWEAK_CHANCE) == 0) {
      rflexes++;
    }
    reflexes.set(rflexes);
  }
  
  reflexes.clock();
  strategyChange.clock();
  ladderJump.clock();
  Intel::clock(p);
}



int Machine::_get_reflexes_time() {
  return DEFAULT_REFLEXES_TIME;
}



Boolean Machine::strategy_to_random_pos(PhysicalP p) {
  const Area &area = p->get_area();
  Pos middle = area.get_middle();

  // Choose range that is inside the world.
  Pos minPos(middle.x - WANDER_WIDTH / 2,
             middle.y - WANDER_WIDTH / 2);
  Pos maxPos(middle.x + WANDER_WIDTH / 2,
             middle.y + WANDER_WIDTH / 2);

  WorldP w = get_world();
  Size worldSize = w->get_size();
  minPos.x = Utils::maximum(0,minPos.x);
  minPos.y = Utils::maximum(0,minPos.y);
  maxPos.x = Utils::minimum(worldSize.width,maxPos.x);
  maxPos.y = Utils::minimum(worldSize.height,maxPos.y);
  
  // Make sure we have some area to choose in.
  if (minPos.x < maxPos.x && minPos.y < maxPos.y) {
    targetPos.x = minPos.x + Utils::choose(maxPos.x - minPos.x);
    targetPos.y = minPos.y + Utils::choose(maxPos.y - minPos.y);
    strategy = toPos;
    return True;
  }
  return False;
}



// By default, try to kill others and get items.
void Machine::choose_strategy(PhysicalP p) {
  LocatorP locator = get_locator();
  const IntelOptions &ops = get_intel_options();
  
  // New strategy.
  // Make sure target is non-NULL for strategies that require it.
  if (strategyChange.ready()) {
    Boolean isEnemy;
    PhysicalP target = choose_target(isEnemy,p,IT_VISION_RANGE);
    HolderP holder = p->get_holder();    

    if (target) {
      if (isEnemy) {
        if ((ops.psychotic ||
             (((holder && (has_gun(holder) || has_cutter(holder))) ||
               p->get_ability(AB_Fighter) || 
               p->get_ability(AB_BuiltIn) ||
               p->get_ability(AB_Prickly)) &&
              (p->get_health() > 
               HEALTH_ATTACK_PERCENT * p->get_health_max()))) &&
            !ops.harmless) {
          set_attackTarget(target->get_id());
        }
        else {
          set_retreatTarget(target->get_id());
        }
      }
      else {
        set_toTarget(target->get_id());
      }
      strategyChange.set(TO_AWAY_TARGET_TIME);
    }
    else {
      set_doNothing();
      // Should set the strategyChange timer.
    }
  }
	
  // strategyChange not ready.
  else {
	// Make sure that target still exists.
	if (strategy_uses_target_id()) {
      if (!locator->lookup(targetId)) {
        set_doNothing();
      }
    }
  }
}



Boolean Machine::filter_target(PhysicalP) {
  return True;
}
			


Boolean Machine::attack_target(PhysicalP p,PhysicalP target) {
  // Fire a gun if available.
  HolderP holder = (HolderP)p->get_holder();
  // has_gun() only checks weapons, not items, so it works for Carrier.
  Boolean useGun = holder && has_gun(holder);  
  if (useGun) {
    if (weapon_current_is_gun(holder)) {
      PhysicalP weapon = holder->get_weapon_current();
      assert(weapon->is_weapon());
      if (((WeaponP)weapon)->ready()) {
        const Area &area = p->get_area();
        Pos pos = area.get_middle();
        const Area &targetArea = target->get_area();
        Pos targetPos = targetArea.get_middle();
        
        Size rel = targetPos - pos;
        Boolean distOk = True;
        
        if (weapon->get_class_id() == A_FThrower) {
          distOk = rel.abs_2() < FTHROWER_RANGE_2;
        }
        
        if (distOk) {
          Dir weaponDir = rel.get_dir();
          
          const Vel *unitVels = p->get_unit_vels();
          float z = rel.cross(unitVels[weaponDir]);
          
          if (fabs(z) <= SHOT_CUTOFF) {
            p->set_command(dir_to_command_weapon(weaponDir));
            return True;
          }
        }
      }
  	}
    else { /* if (weapon_current_is_gun(p)) */
      p->set_command(IT_WEAPON_CHANGE);
      return True;
    }
  } /* if (useGun) */
  
  
  // Use cutter if available.
  // has_cutter() only checks weapons, not items, so it works for Carrier.
  Boolean useCutter = !useGun && holder && has_cutter(holder);
  if (useCutter) {
    if (!weapon_current_is_cutter(holder)) {
      p->set_command(IT_WEAPON_CHANGE);
      return True;
    }
    // else move toward target.
  }
  

  // Use built in weapon if available.
  BuiltInP builtIn = (BuiltInP)p->get_ability(AB_BuiltIn);
  Boolean useBuiltIn = !useGun && !useCutter && builtIn;

  if (useBuiltIn) {
    // Must set current weapon to none.
    if (holder && holder->get_weapon_current()) {
      p->set_command(IT_WEAPON_CHANGE);
      return True;
    }
    // See if we can fire the built-in weapon.
    else {
      if (builtIn->ready()) {
        const Area &area = p->get_area();
        Pos pos = area.get_middle();
        const Area &targetArea = target->get_area();
        Pos targetPos = targetArea.get_middle();
      
        Size rel = targetPos - pos;
        Dir weaponDir = rel.get_dir();
        const Vel *unitVels = p->get_unit_vels();
        float z = rel.cross(unitVels[weaponDir]);
      
        if (fabs(z) <= SHOT_CUTOFF) {
          p->set_command(dir_to_command_weapon(weaponDir));
          return True;
        }
      }
    }
  }
  
  
  // We don't really know how to drop weights on people, so just get 
  // rid of the Liftable.
  Boolean dropLiftable = !useGun && !useCutter && !useBuiltIn
    && p->get_ability(AB_Lifter) && weapon_current_is_drop_liftable(p);
  if (dropLiftable) {
    p->set_command(IT_WEAPON_DROP);
    return True;
  }


  // Close range fighting
  if (!useGun && !useCutter && !useBuiltIn && !dropLiftable 
      && p->get_ability(AB_Fighter)) {
    // Change current weapon to none.
    if (holder && holder->get_weapon_current()) {
      p->set_command(IT_WEAPON_CHANGE);
      return True;
    }
    else {
      const Area &area = p->get_area();
      const Area &targetArea = target->get_area();
      const Pos middle = area.get_middle();
      const Pos targetMiddle = targetArea.get_middle();

      if (middle.distance_2(targetMiddle) <= FIGHT_RANGE_2) {
        //  Dir dirTo = area.dir_to(targetArea);
        Size rel = targetMiddle - middle;
        Dir dirTo = rel.get_dir();

        p->set_command(dir_to_command_weapon(dirTo));
        return True;
      }
      // Else run to target.
    }
  }
  
  return False;
}



Boolean Machine::move_target(PhysicalP physical,PhysicalP target,
			     Timer &ladderJump) {
  const Area &area = physical->get_area();
  Dir toTarget = area.dir_to(target->get_area());
  
  return move_dir(physical,toTarget,ladderJump);
}



Boolean Machine::away_target(PhysicalP physical,PhysicalP target,
			     Timer &ladderJump) {
  const Area &area = physical->get_area();
  Dir awayTarget = Coord::dir_opposite(area.dir_to(target->get_area()));
  
  return move_dir(physical,awayTarget,ladderJump);
}



Boolean Machine::move_pos(PhysicalP physical,const Pos &targetPos,
                          Timer &ladderJump) {
  const Area &area = physical->get_area();
  Dir toPos = area.dir_to(targetPos);

  if (toPos == CO_air) {
    // We've arrived.
    return False;
  }
  return move_dir(physical,toPos,ladderJump);
}



Boolean Machine::move_dir(PhysicalP physical,Dir dir,
			  Timer &ladderJump) {
  assert(physical->is_creature());
  CreatureP creature = (CreatureP) physical;
  ITcommand command = IT_NO_COMMAND;
  
  if (creature->get_ability(AB_Flying)) {
    command = dir_to_command(dir);
  }
  // Sticky, Walking, Grounded, Hopping
  else {
    Stance stance = creature->get_stance();
    Touching touching = creature->get_touching_area();
    const Hanging &hanging = creature->get_hanging();

    // Grounded creatures should not try to climb ladders.
    //// This sucks, creature->can_climb() should delegate to abilities
    //// and thus return False if the locomotion is AB_Grounded.
    Boolean canClimb = creature->can_climb() && !creature->get_ability(AB_Grounded);
    
    switch (dir) {
    case CO_R:
      switch (stance) {
      case CO_center:
      case CO_dn:
      case CO_l:
      case CO_up:
        if ((touching != CO_R) || creature->get_ability(AB_Sticky))
          command = IT_R;
        break;
      case CO_climb:
        if ((touching != CO_R) || creature->get_ability(AB_Sticky)) {
          command = IT_UP_R;
          ladderJump.set();
	    }
        break;
      };
      break;
      
    case CO_DN_R:
      if (canClimb && touching != CO_dn && ladderJump.ready()) {
	    command = IT_DN;
	    break;
	  }
      switch (stance) {
      case CO_center:
      case CO_dn:
      case CO_up:
        command = IT_R;
        break;
      case CO_r:
        command = IT_DN;
        break;
      case CO_l:
        command = IT_DN_R;
        break;
      case CO_climb:
        if (touching != CO_dn)
          command = IT_DN;
        else {
          command = IT_R;
	      ladderJump.set();
	    }
        break;
      }
      break;
      
    case CO_DN:
      switch (stance) {
      case CO_r:
      case CO_up:
      case CO_l:
      case CO_climb:
        if ((touching != CO_dn) || canClimb ||
            (hanging.corner == CO_dn_R) || (hanging.corner == CO_dn_L))
          command = IT_DN;
        break;
      case CO_center:
        if (canClimb)
          command = IT_DN;
        break;
      }
      break;
      
    case CO_DN_L:
      if (canClimb && touching != CO_dn && ladderJump.ready()) {
        command = IT_DN;
        break;
	  }
      switch (stance) {
      case CO_center:
      case CO_dn:
      case CO_up:
        command = IT_L;
        break;
      case CO_l:
        command = IT_DN;
        break;
      case CO_r:
        command = IT_DN_L;
        break;
      case CO_climb:
        if (touching != CO_dn && ladderJump.ready())
          command = IT_DN;
        else {
	      command = IT_L;
	      ladderJump.set();
	    }
        break;
      }
      break;
      
    case CO_L:
      switch (stance) {
      case CO_center:
      case CO_dn:
      case CO_r:
      case CO_up:
        if ((touching != CO_L) || (creature->get_ability(AB_Sticky)))
          command = IT_L;
        break;
      case CO_climb:
        if ((touching != CO_L) || (creature->get_ability(AB_Sticky))) {
          command = IT_UP_L;
          ladderJump.set();
        }
	  break;
      }
      break;
      
    case CO_UP_L:
      if (canClimb && touching != CO_up && ladderJump.ready()) {
	    command = IT_UP;
	    break;
	  }
      switch (stance) {
      case CO_center:
      case CO_dn:
      case CO_up:
        command = IT_L;
        break;
      case CO_r:
        command = IT_UP;
        break;
      case CO_l:
        command = IT_UP_L;
        break;
      case CO_climb:
        if (touching != CO_up)
          command = IT_UP;
        else {
	      command = IT_L;
	      ladderJump.set();
	    }
        break;
      };
      break;
      
    case CO_UP:
      switch (stance) {
      case CO_center:
      case CO_air:
      case CO_dn:
      case CO_r:
      case CO_l:
      case CO_climb:
        if ((touching != CO_up) || canClimb || 
            (hanging.corner == CO_up_R) || (hanging.corner == CO_up_L))
          command = IT_UP;
        break;
      }
      break;
      
    case CO_UP_R:
      if (canClimb && touching != CO_up && ladderJump.ready()) {
	    command = IT_UP;
	    break;
	  }
      switch (stance) {
      case CO_center:
      case CO_dn:
      case CO_up:
        command = IT_R;
        break;
      case CO_r:
      case CO_l:
        command = IT_UP_R;
        break;
      case CO_climb:
        if (touching != CO_up)
          command = IT_UP;
        else {
	      command = IT_R;
	      ladderJump.set();
	    }
        break;
      }
      break;
    }
    
    if ((command == IT_NO_COMMAND) && !canClimb) {
      command = IT_CENTER;
    }
  }
  
  if (command != IT_NO_COMMAND) {
    physical->set_command(command);
    return True;
  }
  
  return False;
}



Boolean Machine::has_gun(HolderP p) {
  for (int n = 0; n < p->get_weapons_num(); n++) {
    PhysicalP gun = p->get_weapon(n);
    if (gun && gun->is_gun()) {
      return True;
    }
  }
  return False;
}



Boolean Machine::has_cutter(HolderP p) {
  for (int n = 0; n < p->get_weapons_num(); n++) {
    PhysicalP w = p->get_weapon(n);
    if (w && w->is_cutter()) {
      return True;
    }
  }
  return False;
}



Boolean Machine::has_shield(HolderP p) {
  for (int n = 0; n < p->get_items_num(); n++) {
    PhysicalP w = p->get_item(n);
    if (w && w->is_shield()) {
      return True;
    }
  }
  return False;
}



Boolean Machine::has_drugs(HolderP p) {
  for (int n = 0; n < p->get_items_num(); n++) {
    PhysicalP w = p->get_item(n);
    if (w && w->is_drugs()) {
      return True;
    }
  }
  return False;
}



Boolean Machine::has_item(HolderP p,ClassId classId) {
  for (int n = 0; n < p->get_items_num(); n++) {
    PhysicalP item = p->get_item(n);
    if (item && item->get_class_id() == classId) {
      return True;
    }
  }
  return False;
}



Boolean Machine::weapon_current_is_gun(HolderP p) {
  PhysicalP weapon = p->get_weapon_current();
  return weapon && weapon->is_gun();
}



Boolean Machine::weapon_current_is_cutter(HolderP p) {
  PhysicalP weapon = p->get_weapon_current();
  return weapon && weapon->is_cutter();
}



Boolean Machine::weapon_current_is_drop_liftable(PhysicalP p) {
  // This is a HACK, we should really make Lifter override 
  // get_weapon_current(), etc.
  const char *dummy;
  ClassId classId = p->get_weapon_string(dummy);
  if (classId == A_DropLiftable) {
    return True;
  }
  return False;
}



PhysicalP Machine::choose_target(Boolean &isEnemy,PhysicalP p,
                                 int range) {
  LocatorP locator = get_locator();
  
  PhysicalP nearby[OL_NEARBY_MAX];
  int nearbyNum;
  locator->get_nearby(nearby,nearbyNum,p,range);

  PhysicalP items[OL_NEARBY_MAX];
  int itemsNum = 0;
  PhysicalP enemies[OL_NEARBY_MAX];
  int enemiesNum = 0;


  // Pull out possible items and enemies from nearby.
  const IntelOptions &ops = get_intel_options();
  ClassId classId = p->get_class_id();
  for (int n = 0; n < nearbyNum; n++) {
    IntelP intel = nearby[n]->get_intel();
    IntelP master = NULL;
    if (intel && !intel->is_human()) {
      master = locator->lookup(((MachineP)intel)->get_master_intel_id());
    }
    
    
    // Possible enemies
    if (
      // Must be a creature
      nearby[n]->is_creature() && 

      // Must be alive
      nearby[n]->alive() && 

      // Must have intelligence
      intel && 

      // Machines ignoring machines of same class.
      (intel->is_human() ||
       (master && master->is_human()) || // classFriends doesn't apply to slaves of humans
       !(ops.classFriends && nearby[n]->get_class_id() == classId)) &&

      // Don't go after members of same team.
      !locator->same_team(p,nearby[n]) &&

      // Ignore invisible targets (Can cast to MovingP because is_creature())
      !((MovingP)nearby[n])->is_invisible() &&

      // Give children a chance to filter out targets
      filter_target(nearby[n])

      ) {
      enemies[enemiesNum] = nearby[n];
      enemiesNum++;
    }


    // possible items
    else if (
      // Not ignoring items
      !ops.ignoreItems &&

      // Must be user or carrier to pick up items
      //// For now, Carriers don't actively go after items, not worth it
      //// to pick up one thing.
      (p->get_ability(AB_User)) && 

      // Must be item and can take it.
      nearby[n]->is_item() && 
      ((ItemP)nearby[n])->can_take(p) &&

      // Ignore invisible targets (Can cast to MovingP because is_item().)
      !((MovingP)nearby[n])->is_invisible() &&

      // Give children a chance to filter out targets
      filter_target(nearby[n])
      ) {
      items[itemsNum] = nearby[n];
      itemsNum++;
    }
  } // for
  
  if (enemiesNum) {
    PhysicalP target = enemies[Utils::choose(enemiesNum)];
    isEnemy = True;
    return target;
  }
  
  if (itemsNum) {
    PhysicalP target = items[Utils::choose(itemsNum)];
    isEnemy = False;
    return target;
  }
  
  return NULL;
}



void Machine::use_items(Boolean &commandSet,PhysicalP p) {
  LocatorP locator = get_locator();
  HolderP holder = p->get_holder();
  if (!holder) {
    return;
  }  

  // Use MedKit.
  if (!commandSet && 
      (p->get_health() < p->get_health_max()) && 
      (has_item(holder,A_MedKit))) {
    PhysicalP item = holder->get_item_current();
    if (item && (item->get_class_id() == A_MedKit)) {
      assert(item->is_auto_use());
      p->set_command(IT_ITEM_USE);
    }
    else {
      p->set_command(IT_ITEM_CHANGE);
    }
    commandSet = True;
  }
	
  // Use Transmogifier.
  if (!commandSet && 
      (p->get_health() < TRANSMOGIFIER_PERCENT * p->get_health_max()) &&
      (has_item(holder,A_Transmogifier))) {
    PhysicalP item = holder->get_item_current();
    if (item && (item->get_class_id() == A_Transmogifier)) {
      assert(item->is_auto_use());
      p->set_command(IT_ITEM_USE);
    }
    else {
      p->set_command(IT_ITEM_CHANGE);
    }
    commandSet = True;
  }

  // Use Shield.
  if (!commandSet && 
      p->is_moving() && has_shield(holder)) {
    PhysicalP protectionP = 
      locator->lookup(((MovingP)p)->get_protection());
    if (!protectionP) {
      PhysicalP item = holder->get_item_current();
      if (item && item->is_shield()) {
        assert(item->is_auto_use());
        p->set_command(IT_ITEM_USE);
      }
      else {
        p->set_command(IT_ITEM_CHANGE);
      }
      commandSet = True;
    }
    // else, don't wipe out the current protection.
  }

#if 0  //haven't tested it with the Holder logic
  // Careful with Aliens.

  // Use Doppel.
  if (!commandSet && p->get_holder()) && p->is_moving() && 
      has_item(p,A_Doppel))
	{
      PhysicalP item = p->get_item_current();
      if (item && item->get_class_id() == A_Doppel) {
        p->set_command(IT_ITEM_USE);
      }
      else {
        p->set_command(IT_ITEM_CHANGE);
      }
      commandSet = TRUE;
	}
#endif		

  // Use Cloak.
  if (!commandSet && p->get_ability(AB_User) &&   // Carrier can't use cloak.
      (has_item(holder,A_Cloak))) {
    PhysicalP item = holder->get_item_current();
    if (item && (item->get_class_id() == A_Cloak)) {
      p->set_command(IT_ITEM_USE);
    }
    else {
      p->set_command(IT_ITEM_CHANGE);
    }
    commandSet = True;
  }

  // Use Drugs.
  if (!commandSet && 
      (has_drugs(holder))) {
    PhysicalP item = holder->get_item_current();
    if (item && item->is_drugs()) {
      assert(item->is_auto_use());
      p->set_command(IT_ITEM_USE);
    }
    else {
      p->set_command(IT_ITEM_CHANGE);
    }
    commandSet = True;
  }

  // Randomly drop items after holding them for a while.
  if (!commandSet && 
      holder->get_items_num() > 0 &&
      // If User/Carrier has > 0 items, one should always be current, but 
      // this is not ALWAYS true.  See comments in class Holder.
      holder->get_item_current() && 
      (Utils::choose(DROP_ITEM_CHANCE) == 0)) {
    p->set_command(IT_ITEM_DROP);
    commandSet = True;
  }      
}



Enemy::Enemy(WorldP w,LocatorP l,char *name,
	const IntelOptions *ops,ITmask opMask)
    :Machine(w,l,name,ops,opMask) {}
  


Boolean Enemy::is_enemy()
{
  return True;
}



int Enemy::_get_reflexes_time()
{
  // Return static member.
  return reflexesTime;
}



int Enemy::reflexesTime = DEFAULT_REFLEXES_TIME;



Neutral::Neutral(WorldP w,LocatorP l,char *name,
	const IntelOptions *ops,ITmask opMask)
	  :Machine(w,l,name,ops,opMask) {}
  


Boolean Neutral::is_enemy()
{
  return False;
}



SealIntel::SealIntel(WorldP w,LocatorP l,char *name,const Id &home_id)
:Neutral(w,l,name,NULL,ITnone)
{
//  Timer oTimer(LADDER_JUMP_TIME);
//  ladderJump = oTimer;
  homeId = home_id;
}



Boolean SealIntel::is_seal_intel()
{
  return True;
}



void SealIntel::choose_strategy(PhysicalP p) {
  // Doesn't use Neutral::choose_strategy().
  // Doesn't care about strategyChange timer.
  
  assert(alive() && p->is_creature());
  LocatorP l = get_locator();

  PhysicalP home = l->lookup(homeId);
  if (!home) {
    cerr << "Error in LIntel, home not found." << endl;
    return;
  }
//  assert(home);
  const Area &area = home->get_area();
  Pos middle = area.get_middle();

	set_toPos(middle);
}	



DoppelIntel::DoppelIntel(WorldP w,LocatorP l,char *name,
	  const IntelOptions *ops,ITmask opMask,IntelP master)
		:Neutral(w,l,name,ops,opMask) {
  assert(master);
  masterIntelId = master->get_intel_id();

  // Set timer to kill self if specified
  // Careful to use get_intel_options(), not the passed in parameter.
  const IntelOptions& options = get_intel_options();
  if (options.limitedLifespan > 0) {
    suicideTimer.set(options.limitedLifespan);
  }
}



IntelId DoppelIntel::get_master_intel_id() {
  return masterIntelId;
}



void DoppelIntel::add_human_kill() {
  LocatorP l = get_locator();
  IntelP intel = l->lookup(masterIntelId);
  if (intel) {
    intel->add_human_kill();
    return;
  }
  Neutral::add_human_kill();
}



void DoppelIntel::add_enemy_kill() {
  LocatorP l = get_locator();
  IntelP intel = l->lookup(masterIntelId);
  if (intel) {
    intel->add_enemy_kill();
    return;
  }
  Neutral::add_enemy_kill();
}



void DoppelIntel::add_soup() {
  assert(0);
}



void DoppelIntel::clock(PhysicalP p) {
  const IntelOptions& ops = get_intel_options();
  
  // Kill self if we have a limited lifespan.
  if (ops.limitedLifespan > 0) {
    if (suicideTimer.ready()) {
      p->set_quiet_death();
      p->kill_self();
    }
    suicideTimer.clock();
  }

  Neutral::clock(p);
}



void DoppelIntel::choose_strategy(PhysicalP p) {
  // We want to be able to override whatever Neutral chooses.
  Neutral::choose_strategy(p);


  Timer &strategyChange = get_strategy_change();
  Strategy strategy = get_strategy();
  LocatorP locator = get_locator();

  // Make sure master is not to close or too far away.
  PhysicalP masterP = NULL; 
  int master_dist_2; // meaningful iff masterP.
  IntelP master = locator->lookup(masterIntelId);
  if (master) {
    masterP = lookup_and_compute_dist_2(master_dist_2,p,master->get_id());
  }

  // Wandering too far from the master.
  if (masterP && master_dist_2 > MASTER_MAX_DIST_2) {
    strategyChange.set(TO_MASTER_TIME);
    set_toTarget(masterP->get_id());
  }

  // We're close enough to the master, do something else.
  if (masterP && 
      strategy == toTarget && masterP->get_id() == get_target_id() &&
      master_dist_2 < MASTER_MIN_DIST_2) {
    // Change strategy next turn.
    strategyChange.set(1);
  }
}



// This is unnecessary with the new TEAM logic.
Boolean DoppelIntel::filter_target(PhysicalP p) {
  LocatorP locator = get_locator();
  IntelP intel = p->get_intel();

  PhysicalP masterP = NULL;
  IntelP master = locator->lookup(masterIntelId);
  if (master) {
    Id masterId = master->get_id();
    masterP = locator->lookup(masterId);
  }

  // Not your master.
  if (p == masterP) {
    return False;
  }

  // Don't want to have slaves of same master fighting each other.
  if (intel && 
      !intel->is_human() && 
      ((MachineP)intel)->get_master_intel_id() == masterIntelId) {
    return False;
  }

  return Neutral::filter_target(p);
}



PhysicalP DoppelIntel::lookup_and_compute_dist_2(int &dist_2,
                                                 PhysicalP p,const Id &id) {
  LocatorP locator = get_locator();
  PhysicalP ret = locator->lookup(id);
  if (ret && p) {
    const Area &retArea = ret->get_area();
    const Area &area = p->get_area();
    Pos retMiddle = retArea.get_middle();
    Pos middle = area.get_middle();
    Size diff = retMiddle - middle;
    dist_2 = diff.abs_2();
  }
  else {
    ret = NULL;
  }
  return ret;
}



IntelOptions DoppelIntel::suggestedOptions = {
  False, /* classFriends */
  False,  /*  NOT SPECIFIED */
	True,  /* psychotic */
  True, /* ignoreItems */
  DOPPEL_SUICIDE_TIME, /* limitedLifespan */
};



ITmask DoppelIntel::suggestedMask = 
  ITclassFriends | ITpsychotic | ITignoreItems | ITlimitedLifespan;



Pet::Pet(WorldP w,LocatorP l,char *name,
	const IntelOptions *ops,ITmask opMask,IntelP master)
	  :DoppelIntel(w,l,name,ops,opMask,master) {
	mode = Heel;
}



Boolean Pet::is_pet() {
  return True;
}



void Pet::go_get_em(PhysicalP shooter,ITcommand command) {
	assert(shooter);
	LocatorP locator = get_locator();
	IntelP shooterIntel = shooter->get_intel();
	assert(shooterIntel);

	// Master has changed.
	if (shooterIntel->get_intel_id() != get_master_intel_id()) {
		// The physical controlled by the pet won't collide with the whoever the 
		// shooterIntel is controlling.
		set_dont_collide(shooterIntel->get_intel_id());
		set_master_intel_id(shooterIntel->get_intel_id());
		// Pet::choose_strategy() will take care of returning to master.
	}

		
	//// Transitions in finite state machine.
	
	// Heel -> Seek
	if (mode == Heel) {
		Dir dir = command_weapon_to_dir_8(command);
		if (dir != CO_air) {
			set_toDir(dir);
			mode = Seek;
			return;
		}
	}
	
  // Seek -> Returning
  // Attacking -> Returning
  if (mode == Seek || mode == Attacking) {
    IntelP master = locator->lookup(get_master_intel_id());
    if (master) {
      set_toTarget(master->get_id());
    }
    mode = Returning;
    return;
  }
}



int Pet::_get_reflexes_time() {
  return (int)(DoppelIntel::_get_reflexes_time() * PET_REFLEXES_MULTIPLIER);
}



void Pet::choose_strategy(PhysicalP p) {
  // We don't use strategyChange timer at all??  Is this right?

	assert(p);

	// Look up master 
	LocatorP locator = get_locator();
	IntelP master = locator->lookup(get_master_intel_id());
	//if (!(master && master->alive())) {
		// If master is dead, act like a Neutral.
	//	Neutral::choose_strategy(p);
	//	return;
	//}

	int master_dist_2;
	PhysicalP masterP = master ? 
		lookup_and_compute_dist_2(master_dist_2,p,master->get_id())
		: (PhysicalP)NULL;
	if (!masterP) {
		// Act like a Neutral, except won't attack master or go after items
		// (because of filter_target).
		Neutral::choose_strategy(p); 
		return;
	}
	//// masterP is now guaranteed to be valid.


	Timer &strategyChange = get_strategy_change();

  ////// Does this do anything?? //////		
	// Use strategy change timer to make the Pet come back when he's
	// been away too long.
	if (master_dist_2 < HEEL_RADIUS_2) {
		// Keeps getting set every time pet is near enough to master.
		strategyChange.set(RETURN_TIME);					

		// Drop anything Carrier is holding to give to the master.
		if (p->get_ability(AB_Carrier)) {
      // Don't kill non-persistent Items, Master might want them.
			p->drop_all(False);  
    }
	}



	//// Transitions in finite state machine.

	// Heel -> Returning
	if (mode == Heel &&
		master_dist_2 > HEEL_RADIUS_2) {
		mode = Returning;
		set_toTarget(masterP->get_id());
		return;
	}


	// Returning -> Heel
	if (mode == Returning &&
		master_dist_2 < HEEL_RADIUS_2) {
		mode = Heel;
		set_toTarget(masterP->get_id());
		return;
	}

	
	// Seek -> Returning
	// Attacking -> Returning
	if ((mode == Seek || mode == Attacking) &&
			strategyChange.ready()) {
		mode = Returning;
		set_toTarget(masterP->get_id());
		return;
	}


	// Seek -> Attacking
	// Heel -> Attacking
	// (added) Returning -> Attacking
	if (mode == Seek || mode == Heel || mode == Returning) {
		Boolean isEnemy;
		PhysicalP target = choose_target(isEnemy,p,ATTACK_RADIUS);
		if (target && isEnemy) {
			set_attackTarget(target->get_id());
			mode = Attacking;
			return;
		}
	}


	// Attacking -> Returning
	if (mode == Attacking) {
		// Just to be sure the strategy is in sync.
		if (strategy_uses_target_id()) { 
			int target_dist_2;
			PhysicalP target = lookup_and_compute_dist_2(target_dist_2,p,get_target_id());
			if (!target || !target->alive() || target_dist_2 > GIVE_UP_ATTACK_RADIUS_2)
			{
				// Target is dead or too far away.
				set_toTarget(masterP->get_id());
				mode = Returning;
				return;
			}
		}
	}
}



Boolean Pet::filter_target(PhysicalP candidate) {
  // Only actively seek creatures.  Not weapons and items.
  // (If the object the Pet is controlling can pick up weapons/items,
  // it still do so if it happens to run into the weapon/item.
  if (!candidate->is_creature()) {
#if 0
      // Don't go after Prickly, Enforcers, Aliens, etc.
      candidate->get_ability(AB_Prickly) ||
      // Don't go after FireDemons, Dragons
      candidate->get_ability(AB_OnFire)
#endif
    return False;
  }
  else {
    return DoppelIntel::filter_target(candidate);
  }
}



char *Pet::mode_string(int mode) {
	switch (mode) {
		case Heel:		
			return "Heel";
		case Returning:
			return "Returning";
		case Seek:
			return "Seek";
		case Attacking:
			return "Attacking";
		default:
			return "Unknown";
	}
}
