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

// game_style.cpp

#if X11
	#ifndef NO_PRAGMAS
	#pragma implementation "game_style.h"
	#endif
#endif

// Include Files
#include "stdafx.h"
extern "C" {
#include <string.h>
}

#if X11
#include <strstream>
#endif

#if WIN32
#include <strstrea.h>
#endif

#include <iomanip>
#include "utils.h"
#include "coord.h"
#include "world.h"
#include "locator.h"
#include "game_style.h"
#include "actual.h"
#include "sound_cmn.h"

using namespace std;


#define SCENARIO_BONUS_TIME 650
#define SCENARIO_HIVE_ALIENS 13
#define SCENARIO_HIVE_EGGS 8
#define SCENARIO_FLAG_ENEMIES 15
#define SCENARIO_SEALS_ENEMIES 4
#define SCENARIO_ZIG_ZAG_ENEMIES 30 // 40 // 50
#define SCENARIO_POUND_DOGS 20
#define SCENARIO_JAPAN_TOWN_NINJAS 15
#define SCENARIO_COOP_CHICKENS 10
#define SCENARIO_SEALS_SEALS (SCENARIO_SEALS_TRAPDOORS * Trapdoor::SEALS_MAX)
#define SCENARIO_SEALS_NEED 15  // Used to be 20, make it easier.
#define SCENARIO_ANTI_SEALS_MAX_SLIPPAGE 3  // number of Seals that can get away
#define LOOK_OUT_INIT_VEL -10

#define LEVEL_TIME 3000
#define HUMAN_LIVES 4
#define HUMAN_LIVES_SCENARIOS 7 //10
#define NORMAL_SCENARIOS_FREQUENCY 5 // Every fifth level is a SCENARIO.



////////////////////////// Specific Scenarios //////////////////////////////
enum ScenarioType {
  EXTERMINATE, // We don't use it right now.
  BONUS,
  HIVE,
  CAPTURE_THE_FLAG,
  SEALS,
  ANTI_SEALS,
  KILL_THE_FIRE_DEMON,
  KILL_THE_DRAGON,
  ZIG_ZAG,
  THE_POUND,
  JAPAN_TOWN,
  THE_COOP,
  LOOK_OUT,
  SCENARIO_NONE, // Must be last.
};



class Scenario {
public:
  Scenario();
  virtual ~Scenario();

  virtual ScenarioType get_scenario_type() = 0;
  /* NOTE: Only used so we don't choose the same scenario twice in a row. */

  virtual Boolean class_friends();
  /* EFFECTS: Are Enemies classFriends with each other. */
  /* DEFAULT: True */

  virtual void set_human_data(HumanP,WorldP,LocatorP);
  /* DEFAULT: do nothing */

  virtual Pos human_initial_pos(WorldP,LocatorP,const Size &s);
  /* DEFAULT: world.empty_rect(s) */

  virtual Boolean can_refill_game_objects();
  /* DEFAULT: True */

  virtual int new_level_check(int enemiesPlaying,WorldP,LocatorP,
                              int level,Boolean &lStrChanged,ostrstream &levelStr,Timer &timer,
                              IPhysicalManagerP) = 0;

  virtual void setup_world(WorldP,LocatorP,const DifficultyLevel &) = 0;
  virtual void new_level(int level,WorldP world,LocatorP locator,
                         const DifficultyLevel &dLevel,
                         ostrstream &lStr,ostrstream &lTitleStr,
                         IPhysicalManagerP manager,int humansNum) = 0;
  /* EFFECTS: These two functions start a new level in a scenario.  setup_world() should
     just setup the map and size of the world.  new_level() is called after world.reset()
     and does the bulk of the work. */
  /* NOTE: Unlike GameStyle::new_level, level is constant. It has already been incremented. 
     Also, there is no kill_fntn as the physicals have already been cleaned. */


  virtual void new_level_set_timer(Timer &timer);
  /* DEFAULT: do nothing */

  virtual Boolean advance_level(); 
  /* DEFAULT: True */ 

  virtual Boolean award_bonus();
  /* DEFAULT: False */

  virtual void refill_enemies(Boolean enemiesRefill,WorldP,LocatorP,IPhysicalManagerP);
  /* DEFAULT: do nothing */

  virtual void filter_weapons_and_other_items(LocatorP,int &weaponsNum,
                                              const PhysicalContext *weapons[],
                                              int &oItemsNum,
                                              const PhysicalContext *oItems[]);
  /* DEFAULT: Do nothing. */

  virtual unsigned int get_soundtrack();
  /* DEFAULT: IDW_SOUNDTRACK */

  virtual SoundName get_midisoundtrack(){return SoundNames::SOUND_RANDOM;}
  /* DEFAULT: SOUND_RANDOM */
};



class Exterminate : public Scenario {
public:
  Exterminate();

  virtual ScenarioType get_scenario_type() {return EXTERMINATE;}

  virtual int new_level_check(int enemiesPlaying,WorldP,LocatorP,
                              int level,Boolean &lStrChanged,ostrstream &levelStr,Timer &timer,
                              IPhysicalManagerP);

  virtual void setup_world(WorldP,LocatorP,const DifficultyLevel &);
  virtual void new_level(int level,WorldP world,LocatorP locator,
                         const DifficultyLevel &dLevel,
                         ostrstream &lStr,ostrstream &lTitleStr,
                         IPhysicalManagerP manager,int humansNum);


private:
  int enemiesNum;
};



class Bonus : public Scenario {
public:
  Bonus();

  virtual ScenarioType get_scenario_type() {return BONUS;}

  virtual Boolean can_refill_game_objects();

  virtual int new_level_check(int enemiesPlaying,WorldP,LocatorP,
                              int level,Boolean &lStrChanged,ostrstream &levelStr,Timer &timer,
                              IPhysicalManagerP);

  virtual void setup_world(WorldP,LocatorP,const DifficultyLevel &);
  virtual void new_level(int level,WorldP world,LocatorP locator,
                         const DifficultyLevel &dLevel,
                         ostrstream &lStr,ostrstream &lTitleStr,
                         IPhysicalManagerP manager,int humansNum);

  virtual void new_level_set_timer(Timer &timer);


private:
  Id frogs[SCENARIO_BONUS_FROGS]; 
  int frogsRemaining;
};



class Hive : public Scenario {
public:
  Hive();

  virtual ScenarioType get_scenario_type() {return HIVE;}

  virtual int new_level_check(int enemiesPlaying,WorldP,LocatorP,
                              int level,Boolean &lStrChanged,ostrstream &levelStr,Timer &timer,
                              IPhysicalManagerP);

  virtual Boolean award_bonus();
  /* EFFECTS: Return True. */

  virtual void setup_world(WorldP,LocatorP,const DifficultyLevel &);
  virtual void new_level(int level,WorldP world,LocatorP locator,
                         const DifficultyLevel &dLevel,
                         ostrstream &lStr,ostrstream &lTitleStr,
                         IPhysicalManagerP manager,int humansNum);

  virtual void refill_enemies(Boolean enemiesRefill,WorldP,LocatorP,IPhysicalManagerP);

  static Boolean aliens_team(LocatorP,PhysicalP,PhysicalP,void *);
  /* EFFECTS: Aliens and Huggers */

  virtual SoundName get_midisoundtrack(){return SoundNames::HIVE_SOUNDTRACK;}


private:
  Id xitId;
};



class CaptureTheFlag : public Scenario {
public:
  CaptureTheFlag();

  virtual ScenarioType get_scenario_type() {return CAPTURE_THE_FLAG;}

  virtual int new_level_check(int enemiesPlaying,WorldP,LocatorP,
                              int level,Boolean &lStrChanged,ostrstream &levelStr,Timer &timer,
                              IPhysicalManagerP);

  virtual void setup_world(WorldP,LocatorP,const DifficultyLevel &);
  virtual void new_level(int level,WorldP world,LocatorP locator,
                         const DifficultyLevel &dLevel,
                         ostrstream &lStr,ostrstream &lTitleStr,
                         IPhysicalManagerP manager,int humansNum);

  virtual void refill_enemies(Boolean enemiesRefill,WorldP,LocatorP,IPhysicalManagerP);


private:
  class FlagMap: public SpecialMap {
  public:
    virtual SpecialMapP clone() {
      return new FlagMap;
    }

    virtual Rooms get_rooms() {
      // down, then across.
      Rooms ret(3,6);
      return ret;
    }

    virtual Boolean big_physicals() {
      // so FireDemon can get flags.
      return True;
    }
  };

  Id flagIds[SCENARIO_FLAG_FLAGS];
  int flagsRemaining;
};



// Has methods common to Seals and AntiSeals.
class GenericSeals: public Scenario {
public:
  GenericSeals();

  virtual void setup_world(WorldP,LocatorP,const DifficultyLevel &);
  virtual void new_level(int level,WorldP world,LocatorP locator,
                         const DifficultyLevel &dLevel,
                         ostrstream &lStr,ostrstream &lTitleStr,
                         IPhysicalManagerP manager,int humansNum);

  virtual Boolean advance_level();  

  virtual void refill_enemies(Boolean enemiesRefill,WorldP,LocatorP,IPhysicalManagerP);

  virtual unsigned int get_soundtrack();

  virtual SoundName get_midisoundtrack(){return SoundNames::SEAL_SOUNDTRACK;}


#ifndef PROTECTED_IS_PUBLIC
protected:
#endif
  void compute_out_safe_active_dead(LocatorP,int &lemOut,int &lemSafe,int &lemActive,int &lemDead);
  /* MODIFIES: lemOut, lemSafe, lemActive, lemDead */
  /* EFFECTS: Compute the current number of seals out, safe, active, and dead. */
  /* NOTE: Usually lemOut == lemSafe + lemActive + lemDead, but not always because of 
     one turn needed to get new Seals in the locator. */
  
  virtual Boolean (*get_team())(LocatorP,PhysicalP,PhysicalP,void *) = 0;
  
  virtual void set_level_strings(int level,ostrstream &lStr,ostrstream &lTitleStr) = 0;
  /* EFFECTS: Called in new_level() to set the appropriate messages. */


  class SealsMap: public SpecialMap {
  public:
    virtual SpecialMapP clone() {
      return new SealsMap;
    }

    virtual Rooms get_rooms() {
      // down, then across.
      Rooms ret(2,4);
      return ret;
    }

    virtual RoomMaze room_maze() {
      return HORIZ_ONLY; 
    }

    virtual Boolean vert_extra_walls() {
      return False;
    }
  };

  
  Id homeId;
  Id trapdoorIds[SCENARIO_SEALS_TRAPDOORS];
  int sealsOut,sealsSafe,sealsActive,sealsDead;
  Boolean dontAdvance;
};



class Seals : public GenericSeals {
public:
  Seals();

  virtual ScenarioType get_scenario_type() {return SEALS;}

  virtual int new_level_check(int enemiesPlaying,WorldP,LocatorP,
                              int level,Boolean &lStrChanged,ostrstream &levelStr,Timer &timer,
                              IPhysicalManagerP);

  virtual Boolean award_bonus();
  /* EFFECTS: Return True. */


#ifndef PROTECTED_IS_PUBLIC
protected:
#endif
  virtual Boolean (*get_team())(LocatorP,PhysicalP,PhysicalP,void *);

  virtual void set_level_strings(int level,ostrstream &lStr,ostrstream &lTitleStr);


private:
  static Boolean seals_team(LocatorP,PhysicalP,PhysicalP,void *humanIndex);
  /* EFFECTS: A team consisting of all SealIntels, human number humanIndex,
   and all slaves of human number humanIndex. */
};



class AntiSeals : public GenericSeals {
public:
  AntiSeals();

  virtual ScenarioType get_scenario_type() {return ANTI_SEALS;}

  virtual int new_level_check(int enemiesPlaying,WorldP,LocatorP,
                              int level,Boolean &lStrChanged,ostrstream &levelStr,Timer &timer,
                              IPhysicalManagerP);


#ifndef PROTECTED_IS_PUBLIC
protected:
#endif
  virtual Boolean (*get_team())(LocatorP,PhysicalP,PhysicalP,void *);

  virtual void set_level_strings(int level,ostrstream &lStr,ostrstream &lTitleStr);


private:
  static Boolean anti_seals_team(LocatorP,PhysicalP,PhysicalP,void *humanIndex);
};



class KillTheFireDemon : public Scenario {
public:
  KillTheFireDemon();

  virtual ScenarioType get_scenario_type() {return KILL_THE_FIRE_DEMON;}

  virtual SoundName get_midisoundtrack(){return SoundNames::FIRE_SOUNDTRACK;}

  virtual int new_level_check(int enemiesPlaying,WorldP,LocatorP,
                              int level,Boolean &lStrChanged,ostrstream &levelStr,Timer &timer,
                              IPhysicalManagerP);

  virtual void setup_world(WorldP,LocatorP,const DifficultyLevel &);
  virtual void new_level(int level,WorldP world,LocatorP locator,
                         const DifficultyLevel &dLevel,
                         ostrstream &lStr,ostrstream &lTitleStr,
                         IPhysicalManagerP manager,int humansNum);

  virtual Boolean award_bonus();
  /* EFFECTS: Return True. */


private:
  class FireDemonMap: public SpecialMap {
  public:
    virtual SpecialMapP clone() {
      return new FireDemonMap;
    }

    virtual Rooms get_rooms() {
      // down, then across.
      Rooms ret(3,3);
      return ret;
    }

    virtual Boolean big_physicals() {
      // so FireDemon can get around.
      return True;
    }
  };
};



class KillTheDragon : public Scenario {
public:
  KillTheDragon();

  virtual ScenarioType get_scenario_type() {return KILL_THE_DRAGON;}

  virtual int new_level_check(int enemiesPlaying,WorldP,LocatorP,
                              int level,Boolean &lStrChanged,ostrstream &levelStr,Timer &timer,
                              IPhysicalManagerP);

  virtual void setup_world(WorldP,LocatorP,const DifficultyLevel &);
  virtual void new_level(int level,WorldP world,LocatorP locator,
                         const DifficultyLevel &dLevel,
                         ostrstream &lStr,ostrstream &lTitleStr,
                         IPhysicalManagerP manager,int humansNum);

  virtual SoundName get_midisoundtrack(){return SoundNames::FIRE_SOUNDTRACK;}

  virtual Boolean award_bonus();
  /* EFFECTS: Return True. */
};



class ZigZag : public Scenario {
public:
  ZigZag();

  virtual ScenarioType get_scenario_type() {return ZIG_ZAG;}

  virtual void set_human_data(HumanP,WorldP,LocatorP);

  virtual Pos human_initial_pos(WorldP,LocatorP,const Size &s);

  virtual int new_level_check(int enemiesPlaying,WorldP,LocatorP,
                              int level,Boolean &lStrChanged,ostrstream &levelStr,Timer &timer,
                              IPhysicalManagerP);

  virtual void setup_world(WorldP,LocatorP,const DifficultyLevel &);
  virtual void new_level(int level,WorldP world,LocatorP locator,
                         const DifficultyLevel &dLevel,
                         ostrstream &lStr,ostrstream &lTitleStr,
                         IPhysicalManagerP manager,int humansNum);

  virtual void refill_enemies(Boolean enemiesRefill,WorldP,LocatorP,IPhysicalManagerP);

  virtual Boolean award_bonus();
  /* EFFECTS: Return True. */


private:
  class ZigZagMap: public SpecialMap {
  public:
    virtual SpecialMapP clone() {
      return new ZigZagMap;
    }

    virtual Rooms get_rooms() {
      // down, then across.
      Rooms ret(3,W_ACROSS_MAX_MAX);
      return ret;
    }

    virtual RoomMaze room_maze() {
      return ZIG_ZAG; 
    }

    // Can't have people teleporting around, now.
    virtual Boolean do_doors() {
      return False;
    }
  };

  Id xitId;
};



class ThePound : public Scenario {
public:
  ThePound();

  virtual ScenarioType get_scenario_type() {return THE_POUND;}

  virtual int new_level_check(int enemiesPlaying,WorldP,LocatorP,
                              int level,Boolean &lStrChanged,ostrstream &levelStr,Timer &timer,
                              IPhysicalManagerP);

  virtual void setup_world(WorldP,LocatorP,const DifficultyLevel &);
  virtual void new_level(int level,WorldP world,LocatorP locator,
                         const DifficultyLevel &dLevel,
                         ostrstream &lStr,ostrstream &lTitleStr,
                         IPhysicalManagerP manager,int humansNum);
};



class JapanTown : public Scenario {
public:
  JapanTown();

  virtual ScenarioType get_scenario_type() {return JAPAN_TOWN;}

  virtual int new_level_check(int enemiesPlaying,WorldP,LocatorP,
                              int level,Boolean &lStrChanged,ostrstream &levelStr,Timer &timer,
                              IPhysicalManagerP);

  virtual void setup_world(WorldP,LocatorP,const DifficultyLevel &);
  virtual void new_level(int level,WorldP world,LocatorP locator,
                         const DifficultyLevel &dLevel,
                         ostrstream &lStr,ostrstream &lTitleStr,
                         IPhysicalManagerP manager,int humansNum);

  virtual void filter_weapons_and_other_items(LocatorP,int &weaponsNum,
                                              const PhysicalContext *weapons[],
                                              int &oItemsNum,
                                              const PhysicalContext *oItems[]);
};



class TheCoop : public Scenario {
public:
  TheCoop();

  virtual ScenarioType get_scenario_type() {return THE_COOP;}

  virtual int new_level_check(int enemiesPlaying,WorldP,LocatorP,
                              int level,Boolean &lStrChanged,ostrstream &levelStr,Timer &timer,
                              IPhysicalManagerP);

  virtual void setup_world(WorldP,LocatorP,const DifficultyLevel &);
  virtual void new_level(int level,WorldP world,LocatorP locator,
                         const DifficultyLevel &dLevel,
                         ostrstream &lStr,ostrstream &lTitleStr,
                         IPhysicalManagerP manager,int humansNum);
};



class LookOut: public Scenario {
public:
  LookOut();

  virtual ScenarioType get_scenario_type() {return LOOK_OUT;}

  virtual Pos human_initial_pos(WorldP,LocatorP,const Size &s);

  virtual int new_level_check(int enemiesPlaying,WorldP,LocatorP,
                              int level,Boolean &lStrChanged,ostrstream &levelStr,Timer &timer,
                              IPhysicalManagerP);

  virtual void setup_world(WorldP,LocatorP,const DifficultyLevel &);

  virtual void new_level(int level,WorldP world,LocatorP locator,
                         const DifficultyLevel &dLevel,
                         ostrstream &lStr,ostrstream &lTitleStr,
                         IPhysicalManagerP manager,int humansNum);


private:
  enum {
    WEIGHTS_MAX = 40,
    WEIGHT_TIME = 3,
  };


  class LookOutMap: public SpecialMap {
  public:
    virtual SpecialMapP clone() {
      return new LookOutMap;
    }

    virtual Rooms get_rooms() {
      // down, then across.
      Rooms ret(1,8);
      return ret;
    }

    virtual RoomMaze room_maze() {
      return SpecialMap::EMPTY;
    }

    virtual Boolean horiz_extra_walls() {
      return False;
    }

    virtual Boolean vert_extra_walls() {
      return False;
    }

    Boolean use_movers() {
      return False;
    }

    virtual Boolean do_doors() {
      return False;
    }
  };


  void create_heavy(WorldP,LocatorP);
  /* EFFECTS: Try to create a single weight. */

  Id xitId;
  // Rotating window of weights to use.
  Id weights[WEIGHTS_MAX];
  // The index of the next weight to allocate.
  int weightNext;
  // Put some time between dropping weights.
  Timer timer;
};



GameStyle::GameStyle() {
}



GameStyle::~GameStyle()
{}



GameStyle *GameStyle::by_type(GameStyleType type) {
  GameStyle *ret = NULL;  

  switch (type) {
    case LEVELS:
      ret = new Normal();
      break;
    case LEVELS_ONLY:
      ret = new Levels();
      break;
    case KILL:
      ret = new KillKillKill();
      break;
    case DUEL:
      ret = new Duel();
      break;
    case EXTENDED:
      ret = new ExtendedDuel();
      break;
    case TRAINING:
      ret = new Training();
      break;
    case SCENARIOS:
      ret = new Scenarios();
      break;
  }      
  assert(ret);

  return ret;
}



Boolean GameStyle::uses_enemies_num(GameStyleType style) {
  return (style == KILL || 
					style == DUEL ||
					style == EXTENDED);
}



Boolean GameStyle::uses_enemies_refill(GameStyleType style) {
  // They happen to be the same now.
  return uses_enemies_num(style);
}



Boolean GameStyle::class_friends() {
  return True;
}  



int GameStyle::human_initial_lives() {
  return HUMAN_LIVES;
}



void GameStyle::set_human_data(HumanP,WorldP,LocatorP)
{}  



Pos GameStyle::human_initial_pos(WorldP world,LocatorP,const Size &size) {
  return world->empty_rect(size);
}



Boolean GameStyle::can_refill_game_objects() {
  return True;
}



Boolean GameStyle::need_difficulty(int) {
  return True;
}



int GameStyle::new_level_check(int,WorldP,LocatorP,
                                  int,Boolean &,ostrstream &,Timer &timer,
                                  IPhysicalManagerP) {  
  // A timer-based level.
  if (timer.ready()) {
    return 0;
  }
  
  // continue
  return -1;
}



Boolean GameStyle::game_over_check(int,int humansPlaying,int) {
  if (humansPlaying == HU_UNSPECIFIED) {
    return False;
  }

  return (humansPlaying == 0);  
}



Boolean GameStyle::advance_level() {
  return True;
}



Boolean GameStyle::award_bonus() {
  return False;
}



void GameStyle::new_level_set_timer(Timer &timer) {
  timer.set(LEVEL_TIME);
}



char *GameStyle::can_reset(int,int,Boolean) {
  // Ok to reset.
  return NULL;
}



void GameStyle::clean_physicals(Boolean doMinimum,WorldP world,
                                LocatorP locator,IPhysicalManagerP manager) {
  // Clean up all Physicals in the Game.  World must have been reset so 
  // that human players can be moved to the inside of the world.
  PhysicalIter pIter(*locator);
  PhysicalP p;


  // Make two passes, one just to get the followers.
  PtrList leaveAlone;  // Explicitly leave these Objects alone.
  while (p = pIter()) {
    int action = partition(p,doMinimum,world,locator);
    if (action == RELOCATE || action == LEAVE_ALONE) {
      // Don't worry about add_unique(), as of now, no object can be a
      // follower of more than one other object.
      //
      // Also, get all followers of followers.  Fix Dragon bug.
      get_followers_recursive(leaveAlone,p);
    }
  }


  // Now that we have the leaveAlone list, actually clean the objects.
  PhysicalIter pIter2(*locator);
  while (p = pIter2()) {
    int action = partition(p,doMinimum,world,locator);
    if (action == RELOCATE) {
      const Area &area = p->get_area();
      IntelP intel = p->get_intel();
      Size size = area.get_size();
      Pos posNew;
      if (intel && intel->is_human()) {
        // Let specific scenario decide.
        posNew = human_initial_pos(world,locator,size);
      }
      else {
        // If necessary, add machine_initial_pos().
        posNew = world->empty_rect(size);
      }
      ((MovingP)p)->relocate(posNew);
    }
    else if (action == KILL_OFF && !leaveAlone.contains(p)) {
      manager->kill_physical(p);
    }
    // else leave object alone
  }
}



void GameStyle::filter_weapons_and_other_items(LocatorP,int &,
                                               const PhysicalContext *[],
                                               int &,
                                               const PhysicalContext *[]) {
}



unsigned int GameStyle::get_soundtrack() {
  return SoundNames::SOUNDTRACK;
}



SoundName GameStyle::get_midisoundtrack() {
  return SoundNames::SOUND_RANDOM;
}



int GameStyle::partition(PhysicalP p,Boolean doMinimum,
                         WorldP world,LocatorP locator) {
  IntelP intel = p->get_intel();
  Boolean controlled = False;
  
  // Save Human controlled physicals.
  if (intel && intel->is_human()) {
    controlled = True;
  }


  // Save enemies if doMinimum is True.  Only used now for KillKillKill 
  // game style.
  if (doMinimum && intel && intel->is_enemy()) {
    controlled = True;
  }


  // Save Machines that are slaves of Humans.
  if (intel && !intel->is_human()) {
    IntelP master = 
      locator->lookup(((MachineP)intel)->get_master_intel_id());
    if (master && master->is_human()) {
      controlled = True;
    }
  }
  

  // Move human/slave to a new location inside the World.
  if (controlled) {
    assert(p->is_moving());
    const Area &area = p->get_area();
    Loc middle = area.middle_wsquare();
    if (!doMinimum || !world->inside(middle)) {
      // Relocate if doing everthing or if we need to keep it inside.
      return RELOCATE;
    }
    return LEAVE_ALONE;
  }
  else if (p->is_item()) {
    if (!doMinimum) {
      // The follower logic will take care of items held by Human players
      return KILL_OFF;
    }
    return LEAVE_ALONE;
  }
  // Kill everything else, ha, ha, ha.
  else {
    return KILL_OFF;
  }
}



void GameStyle::get_followers_recursive(PtrList& leaveAlone,PhysicalP p) {
  PtrList immediate;
  p->get_followers(immediate);
  leaveAlone.append(immediate);

  // Call self on all followers.
  for (int n = 0; n < immediate.length(); n++) {
    PhysicalP f = (PhysicalP)immediate.get(n);
    get_followers_recursive(leaveAlone,f);
  }
}



Normal::Normal() {
  playScenarios = False;
  levels = new Levels();
  scenarios = new Scenarios();
  assert(levels && scenarios);
}



Normal::~Normal() {
  delete levels;
  delete scenarios;
}



GameStyleP Normal::clone() {
  GameStyleP ret = new Normal();
  assert(ret);
  return ret;
}



void Normal::describe(ostrstream &str) {
  str << "Complete each level/scenario to proceed to the next one." << ends;
}



GameStyleType Normal::get_type() {
  // A bit of a hack
  return LEVELS;
}



int Normal::human_initial_lives() {
  return HUMAN_LIVES;
}



void Normal::set_human_data(HumanP h,WorldP w,LocatorP l) {
  GameStyleP delegate = get_delegate();
  delegate->set_human_data(h,w,l);
}  



Pos Normal::human_initial_pos(WorldP w,LocatorP l,const Size &s) {
  GameStyleP delegate = get_delegate();
  return delegate->human_initial_pos(w,l,s);
}



Boolean Normal::can_refill_game_objects() {
  GameStyleP delegate = get_delegate();
  return delegate->can_refill_game_objects();
}



char *Normal::can_reset(int humansNumNext,int,Boolean) {
  if (humansNumNext == HU_UNSPECIFIED) {
    return NULL;
  }
  
  if (humansNumNext < 1) {
    return Utils::strdup("NEED AT LEAST ONE HUMAN PLAYER");
  }
  return NULL;
}



void Normal::reset(WorldP w,LocatorP l,const DifficultyLevel &dLevel,int enemiesNumNext) {
  levels->reset(w,l,dLevel,enemiesNumNext);
  scenarios->reset(w,l,dLevel,enemiesNumNext);
}



int Normal::new_level_check(int enemiesPlaying,WorldP w,LocatorP l,
                             int level,Boolean &lStrChanged,ostrstream &levelStr,Timer &timer,
                             IPhysicalManagerP manager) {
  GameStyleP delegate = get_delegate();
  return delegate->new_level_check(enemiesPlaying,w,l,level,lStrChanged,levelStr,timer,manager);
}



Boolean Normal::advance_level() {
  GameStyleP delegate = get_delegate();
  return delegate->advance_level();
}



Boolean Normal::award_bonus() {
  GameStyleP delegate = get_delegate();
  return delegate->award_bonus();
}



void Normal::new_level(int level,WorldP world,LocatorP locator,
                       const DifficultyLevel &dLevel,
                       ostrstream &lStr,ostrstream &lTitleStr,
                       IPhysicalManagerP manager,int humansNum) {
  // Choose which delegate to use.
  if (level % NORMAL_SCENARIOS_FREQUENCY == 0) {
    playScenarios = True;    
  }
  else {
    playScenarios = False;    
  }     

  GameStyleP delegate = get_delegate();
  delegate->new_level(level,world,locator,dLevel,lStr,lTitleStr,manager,humansNum);
}



void Normal::new_level_set_timer(Timer &timer) {
  GameStyleP delegate = get_delegate();
  delegate->new_level_set_timer(timer);
}



void Normal::refill_enemies(Boolean eRefill,WorldP w,LocatorP l,
                               IPhysicalManagerP manager) {
  GameStyleP delegate = get_delegate();
  delegate->refill_enemies(eRefill,w,l,manager);
}



unsigned int Normal::get_soundtrack() {
  GameStyleP delegate = get_delegate();
  return delegate->get_soundtrack();
}



SoundName Normal::get_midisoundtrack() {
  GameStyleP delegate = get_delegate();
  return delegate->get_midisoundtrack();
}



Levels::Levels() 
{}



GameStyleP Levels::clone() {
  GameStyleP ret = new Levels();
  assert(ret);
  return ret;
}



void Levels::describe(ostrstream &str) {
  str << "Human player(s) fights through increasing levels of "
	    << "difficulty." << "\n" 
	    << "To complete a level you must kill all enemy players." 
	    << ends;
}



GameStyleType Levels::get_type() {
  return LEVELS_ONLY;
}



char *Levels::can_reset(int humansNumNext,int,Boolean) {
  if (humansNumNext == HU_UNSPECIFIED) {
    return NULL;
  }
  if (humansNumNext < 1) {
    return Utils::strdup("LEVELS REQUIRES AT LEAST ONE HUMAN PLAYER");
  }
  return NULL;
}



void  Levels::reset(WorldP,LocatorP,const DifficultyLevel &dLevel,int) {
  // So first level will start out with enemiesInitial.
  enemiesNum = dLevel.enemiesInitial - dLevel.enemiesIncr;
}



int Levels::new_level_check(int enemiesPlaying,WorldP,LocatorP,
                              int,Boolean &,ostrstream &,Timer &,
                              IPhysicalManagerP) {
  if (enemiesPlaying == 0) {
    return 1;
  }
  else {
    return -1;
  }
}



void Levels::new_level(int level,WorldP world,LocatorP locator,
                       const DifficultyLevel &dLevel,
                       ostrstream &lStr,ostrstream &lTitleStr,
                       IPhysicalManagerP manager,int) {
  world->reset();  

//  clean_physicals(True,world,locator,manager);
  clean_physicals(False,world,locator,manager);


  lTitleStr << "[" << level << "] Kill All Machines" << ends;
  lStr << "Level: " << level << ends;

  enemiesNum += dLevel.enemiesIncr;
  enemiesNum = Utils::minimum(enemiesNum,dLevel.enemiesMax);
  for (int m = 0; m < enemiesNum; m++) {
    manager->create_enemy(NULL);
  }
}



void Levels::new_level_set_timer(Timer &)
{}



void Levels::refill_enemies(Boolean,WorldP,LocatorP,
                            IPhysicalManagerP) 
{
  // Never refill.
}



unsigned int Levels::get_soundtrack() {
  return SoundNames::SOUNDTRACK_LEVELS;
}



KillKillKill::KillKillKill() {
  justReset = False;
  enemiesNum = 0;
}



GameStyleP KillKillKill::clone() {
  GameStyleP ret = new KillKillKill();
  assert(ret);
  return ret;
}



void KillKillKill::describe(ostrstream &str) {
  str << "Every human and machine for him/her/itself." << ends;
}



GameStyleType KillKillKill::get_type() {
  return KILL;
}



Boolean KillKillKill::class_friends() {
  return False;
}  



char *KillKillKill::can_reset(int humansNumNext,int enemiesNumNext,Boolean) {
  if (humansNumNext == HU_UNSPECIFIED) {
    return NULL;
  }
  if (humansNumNext + enemiesNumNext < 2) {
    return Utils::strdup("NEED AT LEAST 2 HUMAN/ENEMY PLAYERS FOR "
	                       "KILL,KILL,KILL");
  }
  return NULL;
}



Boolean KillKillKill::need_difficulty(int enemiesNumNext) {
  return (enemiesNumNext > 0);
}



void KillKillKill::reset(WorldP,LocatorP,const DifficultyLevel &,
                         int enemiesNumNext) {
  enemiesNum = enemiesNumNext;
  
  justReset = True;
}



Boolean KillKillKill::game_over_check(int humansNum,int humansPlaying,
                                      int enemiesPlaying) {
  if (humansPlaying == HU_UNSPECIFIED) {
    return False;
  }

  if (humansNum == 0) {
    // Screen-saver mode.  Let the bad-guys fight it out.
    return (enemiesPlaying <= 1); 
  }
  else {
    // Like a regular game.  Game is over when you die, or you kill 'em all.
    // Probably should check for regenerateEnemies, 
    return humansPlaying == 0 || enemiesPlaying == 0;
  }
}



void KillKillKill::new_level(int level,WorldP world,LocatorP locator,
                             const DifficultyLevel &,
                             ostrstream &lStr,ostrstream &lTitleStr,
                             IPhysicalManagerP manager,int) {
  world->reset();

  // Don't wipe out the Enemies.
  clean_physicals(True,world,locator,manager);

  lTitleStr << "Level: " << level << ends;
  lStr << "Level: " << level << ends;

  // Only create enemies first time or if enemiesRefill is set.
  if (justReset) {
    justReset = False;
    for (int m = 0; m < enemiesNum; m++) {
      manager->create_enemy(NULL);
    }
  }
}



void KillKillKill::refill_enemies(Boolean enemiesRefill,WorldP,LocatorP locator,
                                  IPhysicalManagerP manager) {
  // Only refill enemies if enemiesRefill flag is set.
  if (!enemiesRefill) {
    return;
  }  

  int diff = enemiesNum - locator->enemies_alive();
  assert(diff >= 0);
  
  for (int n = 0; n < diff; n++) {
    manager->create_enemy(NULL);
  }
}



SoundName KillKillKill::get_midisoundtrack() {
  return SoundNames::KILL_SOUNDTRACK;
}



Duel::Duel() {
}



GameStyleP Duel::clone() {
  GameStyleP ret = new Duel();
  assert(ret);
  return ret;
}



void Duel::describe(ostrstream &str) {
  str << "Human vs. human battle to the death." 
	    << "\n" << "Each human has 3 lives." << ends;
}



GameStyleType Duel::get_type() {
  return DUEL;
}



Boolean Duel::need_difficulty(int enemiesNumNext) {
  return (enemiesNumNext > 0);
}



char *Duel::can_reset(int humansNumNext,int,Boolean cooperative) {
  if (humansNumNext == HU_UNSPECIFIED) {
    return NULL;
  }
  if (humansNumNext < 2) {
    return Utils::strdup("NEED AT LEAST 2 HUMAN PLAYERS FOR A DUEL");
  }
  if (cooperative) {
    return Utils::strdup("CAN'T PLAY DUEL WITH COOPERATIVE MODE");
  }
  return NULL;
}



void Duel::reset(WorldP,LocatorP,const DifficultyLevel &,
                 int enemiesNumNext) {
  enemiesNum = enemiesNumNext;
  
  justReset = True;
}



Boolean Duel::game_over_check(int,int humansPlaying,int) {
  if (humansPlaying == HU_UNSPECIFIED) {
    return False;
  }

  return (humansPlaying <= 1);
}



void Duel::new_level(int level,WorldP world,LocatorP locator,
                     const DifficultyLevel &,
                     ostrstream &lStr,ostrstream &lTitleStr,
                     IPhysicalManagerP manager,int) {
  world->reset();
  clean_physicals(False,world,locator,manager);

  lTitleStr << "Level: " << level << ends;
  lStr << "Level: " << level << ends;

  // Only create enemies first time.
  if (justReset) {
    justReset = False;
    for (int m = 0; m < enemiesNum; m++) {
      manager->create_enemy(NULL);
    }
  }
}



void Duel::refill_enemies(Boolean enemiesRefill,WorldP,LocatorP locator,
                          IPhysicalManagerP manager) {
  // Only refill enemies if enemiesRefill flag is set.
  if (!enemiesRefill) {
    return;
  }  

  int diff = enemiesNum - locator->enemies_alive();
  assert(diff >= 0);
  
  for (int n = 0; n < diff; n++) {
    manager->create_enemy(NULL);
  }
}



ExtendedDuel::ExtendedDuel() 
{}



GameStyleP ExtendedDuel::clone() {
  GameStyleP ret = new ExtendedDuel();
  assert(ret);
  return ret;
}



void ExtendedDuel::describe(ostrstream &str) {
  str << "Human vs. human battle to the death." 
	    << "\n" << "Unlimited lives." << ends;
}



GameStyleType ExtendedDuel::get_type() {
  return EXTENDED;
}



int ExtendedDuel::human_initial_lives() {
  return IT_INFINITE_LIVES;  
}




Boolean ExtendedDuel::need_difficulty(int enemiesNumNext) {
  return (enemiesNumNext > 0);
}



char *ExtendedDuel::can_reset(int humansNumNext,int,Boolean cooperative) {
  if (humansNumNext == HU_UNSPECIFIED) {
    return NULL;
  }
  if (humansNumNext < 2) {
    return Utils::strdup("NEED AT LEAST 2 HUMAN PLAYERS FOR A DUEL");
  }
  if (cooperative) {
    return Utils::strdup("CAN'T PLAY DUEL WITH COOPERATIVE MODE ON");
  }
  return NULL;
}



void ExtendedDuel::reset(WorldP,LocatorP,const DifficultyLevel &,
                         int enemiesNumNext) {
  enemiesNum = enemiesNumNext;  
  
  justReset = True;
}



Boolean ExtendedDuel::game_over_check(int,int humansPlaying,int) {
  if (humansPlaying == HU_UNSPECIFIED) {
    return False;
  }

  // This never really happens, since all humans have infinite number of
  // lives.
  return (humansPlaying <= 1);
}



void ExtendedDuel::new_level(int level,WorldP world,LocatorP locator,
                             const DifficultyLevel &,
                             ostrstream &lStr,ostrstream &lTitleStr,
                             IPhysicalManagerP manager,int) {
  world->reset();
  clean_physicals(False,world,locator,manager);

  lTitleStr << "Level: " << level << ends;
  lStr << "Level: " << level << ends;

  // Only create enemies first time.
  if (justReset) {
    justReset = False;
    for (int m = 0; m < enemiesNum; m++) {
      manager->create_enemy(NULL);
    }
  }
}



void ExtendedDuel::refill_enemies(Boolean enemiesRefill,WorldP,LocatorP locator,
                                  IPhysicalManagerP manager) {
  // Only refill enemies if enemiesRefill flag is set.
  if (!enemiesRefill) {
    return;
  }  

  int diff = enemiesNum - locator->enemies_alive();
  assert(diff >= 0);
  
  for (int n = 0; n < diff; n++) {
    manager->create_enemy(NULL);
  }
}



Training::Training() 
{}



GameStyleP Training::clone() {
  GameStyleP ret = new Training();
  assert(ret);
  return ret;
}



void Training::describe(ostrstream &str) {
  str << "No enemies.  Useful for learning the controls." << ends;
}



GameStyleType Training::get_type() {
  return TRAINING;
}



Boolean Training::need_difficulty(int) {
  return False;
}



void Training::reset(WorldP,LocatorP,const DifficultyLevel &,int)
{}



void Training::new_level(int level,WorldP world,LocatorP locator,
                        const DifficultyLevel &,
                        ostrstream &lStr,ostrstream &lTitleStr,
                        IPhysicalManagerP manager,int) {
  world->reset();
  clean_physicals(False,world,locator,manager);

  lTitleStr << "Level: " << level << ends;
  lStr << "Level: " << level << ends;
}



void Training::refill_enemies(Boolean,WorldP,LocatorP,IPhysicalManagerP){
}



Scenarios::Scenarios() {
  scenario = NULL;
}



Scenarios::~Scenarios() {
  delete scenario;
}



GameStyleP Scenarios::clone() {
  GameStyleP ret = new Scenarios();
  assert(ret);
  return ret;
}



void Scenarios::describe(ostrstream &str) {
  str << "A number of different scenarios." << "\n"
	    << "You must complete each scenario to continue on to the "
	    << "next one." << ends;
}



GameStyleType Scenarios::get_type() {
  return SCENARIOS;
}



Boolean Scenarios::class_friends() {
  assert(scenario);
  return scenario->class_friends();
}



int Scenarios::human_initial_lives() {
  return HUMAN_LIVES_SCENARIOS;
}



void Scenarios::set_human_data(HumanP h,WorldP w,LocatorP l) {
  assert(scenario);
  scenario->set_human_data(h,w,l);
}  



Pos Scenarios::human_initial_pos(WorldP w,LocatorP l,const Size &s) {
  // defer to specific scenario
  assert(scenario);
  return scenario->human_initial_pos(w,l,s);
}



Boolean Scenarios::can_refill_game_objects() {
  // defer to specific scenario
  assert(scenario);
  return scenario->can_refill_game_objects();
}



char *Scenarios::can_reset(int humansNumNext,int,Boolean) {
  if (humansNumNext == HU_UNSPECIFIED) {
    return NULL;
  }
  if (humansNumNext < 1) {
    return Utils::strdup("SCENARIOS REQUIRES AT LEAST ONE HUMAN PLAYER");
  }
  return NULL;
}



void Scenarios::reset(WorldP,LocatorP,const DifficultyLevel &,int) {
  delete scenario;
  scenario = NULL;
}



int Scenarios::new_level_check(int enemiesPlaying,WorldP w,LocatorP l,
                             int level,Boolean &lStrChanged,
                               ostrstream &levelStr,Timer &timer,
                             IPhysicalManagerP manager) {
  assert(scenario);
  return scenario->new_level_check(enemiesPlaying,w,l,level,
                                   lStrChanged,levelStr,timer,manager); 
}



Boolean Scenarios::advance_level() {
  if (scenario) {
    return scenario->advance_level();
  }
  else {
    // For first time.
    return True;
  }
}



Boolean Scenarios::award_bonus() {
  if (scenario) {
    return scenario->award_bonus();
  }
  else {
    // For first time.
    return False;
  }
}



void Scenarios::new_level(int level,WorldP world,LocatorP locator,
                       const DifficultyLevel &dLevel,
                       ostrstream &lStr,ostrstream &lTitleStr,
                       IPhysicalManagerP manager,int humansNum) {
  // Works even if scenario is NULL.
  if (!scenario || scenario->advance_level()) {
    // Sets new value for this->scenario.
    choose_scenario();
  }

  scenario->setup_world(world,locator,dLevel);

  world->reset();

  clean_physicals(False,world,locator,manager);

  scenario->new_level(level,world,locator,dLevel,
                      lStr,lTitleStr,manager,humansNum);
}



void Scenarios::new_level_set_timer(Timer &timer) {
  assert(scenario);
  scenario->new_level_set_timer(timer);
}



void Scenarios::refill_enemies(Boolean eRefill,WorldP w,LocatorP l,
                               IPhysicalManagerP manager) {
  assert(scenario);
  scenario->refill_enemies(eRefill,w,l,manager);
}



void Scenarios::set_override(char *val) {
//  override = Utils::strdup(val);  
  override = val;
}



void Scenarios::choose_scenario() {
  ScenarioType prevScenario = 
    scenario ? scenario->get_scenario_type() : SCENARIO_NONE;
  delete scenario;
  scenario = NULL;

  // Check for command line argument, "-scenario".
  // Ick, not extensible.
  if (override) {
	  if (!(strcmp("exterminate",override))) {
      scenario = new Exterminate();
    }
	  else if (!(strcmp("bonus",override))) {
      scenario = new Bonus();
    }
	  else if (!(strcmp("hive",override))) {
      scenario = new Hive();
    }
	  else if (!(strcmp("flag",override))) {
      scenario = new CaptureTheFlag();
    }
	  else if (!(strcmp("baby-seals",override))) {
      scenario = new Seals();
    }
	  else if (!(strcmp("anti-baby-seals",override))) {
      scenario = new AntiSeals();
    }
	  else if (!(strcmp("fire-demon",override))) {
      scenario = new KillTheFireDemon();
    }
	  else if (!(strcmp("dragon",override))) {
      scenario = new KillTheDragon();
    }
	  else if (!(strcmp("zig-zag",override))) {
      scenario = new ZigZag();
    }
	  else if (!(strcmp("the-pound",override))) {
      scenario = new ThePound();
    }
	  else if (!(strcmp("japan-town",override))) {
      scenario = new JapanTown();
    }
	  else if (!(strcmp("the-coop",override))) {
      scenario = new TheCoop();
    }
	  else if (!(strcmp("chicken-little",override))) {
      scenario = new LookOut();
    }
  }

  // We chose one from the override, so we are done.
  if (scenario) {
    return;
  }   

  // Keep trying until we choose one that isn't the same as the last one.
  while (True) {
    switch (Utils::choose(12)) {
      case 0:
        scenario = new Bonus();
        break;
      case 1:
        scenario = new Hive();
        break;
      case 2:
        scenario = new CaptureTheFlag();
        break;
      case 3:
        scenario = new Seals();
        break;
      case 4:
        scenario = new AntiSeals();
        break;
      case 5:
        scenario = new KillTheFireDemon();
        break;
      case 6:
        scenario = new KillTheDragon();
        break;
      case 7:
        scenario = new ZigZag();
        break;
      case 8:
        scenario = new ThePound();
        break;
      case 9:
        scenario = new JapanTown();
        break;
      case 10:
        scenario = new TheCoop();
        break;
      case 11:
        scenario = new LookOut();
        break;
      default:
        assert(0);
    }
    // Done, don't choose same scenario twice in a row.
    if (scenario->get_scenario_type() != prevScenario) {
      return;
    }
    // Try again.
    else {
      delete scenario;
      scenario = NULL;
    }
  }
}



void Scenarios::filter_weapons_and_other_items(LocatorP l,
                                               int &wNum,const PhysicalContext *w[],
                                               int &oINum,const PhysicalContext *oI[]) {
  assert(scenario);
  scenario->filter_weapons_and_other_items(l,wNum,w,oINum,oI);
}



unsigned int Scenarios::get_soundtrack() {
  // Needs to work before first new level.
  if (scenario) {
    return scenario->get_soundtrack();
  }
  else {
    return GameStyle::get_soundtrack();
  }
}



SoundName Scenarios::get_midisoundtrack() {
  // Needs to work before first new level.
  if (scenario) {
    return scenario->get_midisoundtrack();
  }
  else {
    return GameStyle::get_midisoundtrack();
  }
}



Boolean Scenarios::class_team(LocatorP,PhysicalP p1,PhysicalP p2,void *closure) {
  ClassId classId = (ClassId)closure;
  return class_team_member(p1,classId) && class_team_member(p2,classId);
}



Boolean Scenarios::class_team_member(PhysicalP p,ClassId classId) {
  IntelP intel = p->get_intel();
  if (intel && intel->is_enemy() && p->get_class_id() == classId) {
    return True;
  }
  return False;
}



char *Scenarios::override = NULL;




////////////////////////// Specific Scenarios ////////////////////////////////////


Scenario::Scenario()
{}



Scenario::~Scenario()
{}



Boolean Scenario::class_friends() {
  return True;
}



void Scenario::set_human_data(HumanP,WorldP,LocatorP)
{}  



Pos Scenario::human_initial_pos(WorldP world,LocatorP,const Size &size) {
  return world->empty_rect(size);
}



Boolean Scenario::can_refill_game_objects() {
  return True;
}



void Scenario::new_level_set_timer(Timer &)
{}



Boolean Scenario::advance_level() {
  return True;
}



Boolean Scenario::award_bonus() {
  return False;
}



void Scenario::refill_enemies(Boolean,WorldP,LocatorP,IPhysicalManagerP)
{}



void Scenario::filter_weapons_and_other_items(LocatorP,int &,const PhysicalContext *[],
                                              int &,const PhysicalContext *[]) {
}



unsigned int Scenario::get_soundtrack() {
  return SoundNames::SOUNDTRACK;
}



Exterminate::Exterminate()
{}



int Exterminate::new_level_check(int enemiesPlaying,WorldP,LocatorP,
                                 int,Boolean &,ostrstream &,Timer &,
                                 IPhysicalManagerP) {
  if (enemiesPlaying == 0) {
    return 1;
  }
  return -1;  
}



void Exterminate::setup_world(WorldP,LocatorP,const DifficultyLevel &) {
  // Nothing to do.
}



void Exterminate::new_level(int level,WorldP,LocatorP,
                            const DifficultyLevel &dLevel,
                            ostrstream &lStr,ostrstream &lTitleStr,
                            IPhysicalManagerP manager,int) {
  lTitleStr << "[" << level << "] EXTERMINATE" << ends;
  lStr << "[" << level << "] EXTERMINATE\nKill all machines." << ends;

  // Doesn't increment like LEVELS game style
  enemiesNum = dLevel.enemiesIncr;

  for (int m = 0; m < enemiesNum; m++) {
    manager->create_enemy(NULL);
  }
}



Bonus::Bonus()
{
  frogsRemaining = 0;
}



Boolean Bonus::can_refill_game_objects() {
  return False;
}



int Bonus::new_level_check(int,WorldP,LocatorP locator,
                           int level,Boolean &lStrChanged,
                           ostrstream &levelStr,Timer &timer,
                           IPhysicalManagerP) {
  // Ran out of time.
  if (timer.ready()) {
    locator->message_enq(Utils::strdup("FAILED TO KILL ALL THE FROGS"));
    return 0;
  }

  // Count number of Frogs still alive.
  int remaining = 0;
  for (int n = 0; n < SCENARIO_BONUS_FROGS; n++) {
    PhysicalP p = locator->lookup(frogs[n]);
	  if (p && p->alive()) {
      remaining++;
    }
  }
	  
  // Update ui->
  if (remaining != frogsRemaining) {
    frogsRemaining = remaining;
    levelStr << "[" << level << "] BONUS LEVEL\nfrogs remaining: " 
	      << remaining << ends;
    lStrChanged = True;
    if (remaining > 0) {
	    ostrstream arenaStr;
	    arenaStr << remaining << " Frog" << 
	      (remaining > 1 ? "s" : "") << " Remaining" << ends;
	    locator->arena_message_enq(arenaStr.str());
    }

    // Will only be called once per level.
    if (frogsRemaining == 0) {
//      ostrstream ostr;
//      ostr << "**** You must feel proud of yourself for killing all "
//      << "those defenseless frogs. ****" << ends;
//      locator->message_enq(ostr.str());
      locator->arena_message_enq(Utils::strdup("Got em all"));
      return 1;
    }
  }

  return -1;
}	  



void Bonus::setup_world(WorldP world,LocatorP,const DifficultyLevel &) {
  Rooms rooms(2,1);
  world->set_rooms_next(rooms);
}



void Bonus::new_level(int level,WorldP world,LocatorP locator,
                      const DifficultyLevel &,
                      ostrstream &lStr,ostrstream &lTitleStr,
                      IPhysicalManagerP manager,int) {
  // Create frogs
  for (int n = 0; n < SCENARIO_BONUS_FROGS; n++) {
	  PhysicalP p = manager->create_enemy(manager->enemy_physical(A_Frog));
	  frogs[n] = p->get_id();
  }

  // Create one weapon
  PhysicalP weap = NULL;
  switch(Utils::choose(5)) {
    case 0:
    weap = new FThrower(world,locator,world->empty_rect(FThrower::get_size_max()));
    break;
    case 1:
    weap = new Grenades(world,locator,world->empty_rect(Grenades::get_size_max()));
    break;
    case 2:
    weap = new Chainsaw(world,locator,world->empty_rect(Chainsaw::get_size_max()));
    break;
    case 3:
    weap = new MGun(world,locator,world->empty_rect(MGun::get_size_max()));
    break;
    case 4:
    weap = new Stars(world,locator,world->empty_rect(Pistol::get_size_max()));
    break;
  }
  assert(weap);
  locator->add(weap);

  frogsRemaining = SCENARIO_BONUS_FROGS;

  lTitleStr << "[" << level << "] Bonus Level: Kill " 
		  << (int)SCENARIO_BONUS_FROGS << " Frogs"  << ends;
  lStr << "[" << level << "] BONUS LEVEL\nfrogs remaining: " 
	     << (int)SCENARIO_BONUS_FROGS << ends;
}



void Bonus::new_level_set_timer(Timer &timer) {
  // Limited time to kill the frogs.
  timer.set(SCENARIO_BONUS_TIME);
}



Hive::Hive() {
}



int Hive::new_level_check(int,WorldP,LocatorP locator,
                          int,Boolean &,ostrstream &,Timer &,
                          IPhysicalManagerP) {
  PhysicalP xit = locator->lookup(xitId);
  assert(xit); // The Xit should never be destroyed.
  if (((TouchableP)xit)->wasTouched()) {
    return 0;
  }
  return -1;  
}



Boolean Hive::award_bonus() {
  return True;
}



void Hive::setup_world(WorldP world,LocatorP,const DifficultyLevel &) {
  Rooms rooms(5,5);
  world->set_rooms_next(rooms);
}



void Hive::new_level(int level,WorldP world,LocatorP locator,
                     const DifficultyLevel &,
                     ostrstream &lStr,ostrstream &lTitleStr,
                     IPhysicalManagerP manager,int) {
	// Aliens are created in Game::clock.

  // So Aliens and Huggers are friends.
  locator->add_team(aliens_team,NULL,NULL);
	
	// Create the Xit.
	Pos pos = world->empty_accessible_rect(Xit::get_size_max());
	PhysicalP p = new Xit(world,locator,pos);
	assert(p);
	locator->add(p);
	xitId = p->get_id();

	// Create some eggs.
	IntelOptions eggOptions;
	ITmask eggMask = manager->intel_options_for(eggOptions,A_Alien);

	for (int n = 0; n < SCENARIO_HIVE_EGGS; n++) {
	  Pos pos = world->empty_rect(Egg::get_size_max());
	  PhysicalP egg = new Egg(world,locator,pos,eggOptions,eggMask);
	  assert(egg);
	  locator->add(egg);
	}

	lTitleStr << "[" << level << "] HIVE" << ends;
	lStr << "[" << level << "] HIVE.\nFind the exit." << ends;
}



void Hive::refill_enemies(Boolean,WorldP,LocatorP locator,IPhysicalManagerP manager) {
  int diff = SCENARIO_HIVE_ALIENS - locator->enemies_alive();

  // diff might be less than zero if some RedHuggers have come into existence.
  //assert(diff >= 0);

  for (int n = 0; n < diff; n++) {
    manager->create_enemy(manager->enemy_physical(A_Alien));
  }
}



static Boolean Hive_aliens_team_member(PhysicalP p) {
  // Eggs are on the team, even though they don't have intelligence.
  if (p->get_class_id() == A_Egg) {
    return True;
  }

  // Creature in process of being hugged.
  if (p->is_creature() && ((CreatureP)p)->get_alien_immune()) {
    return True;
  }

  IntelP intel = p->get_intel();
  if (!intel) {
    return False;
  }

  if (intel->is_human()) {
    return False;
  }

  // Hugger or alien.
  if (p->get_class_id() == A_Alien || p->get_ability(AB_Hugger)) {
    return True;
  }

  return False;
}



Boolean Hive::aliens_team(LocatorP,PhysicalP p1,PhysicalP p2,void *) {
  return Hive_aliens_team_member(p1) && Hive_aliens_team_member(p2);
}



CaptureTheFlag::CaptureTheFlag() {
}



int CaptureTheFlag::new_level_check(int,WorldP,LocatorP locator,
                                    int level,Boolean &lStrChanged,ostrstream &levelStr,Timer &,
                                    IPhysicalManagerP manager) {
  int fRemaining = 0;
  for (int n = 0; n < SCENARIO_FLAG_FLAGS; n++) {
    PhysicalP p = locator->lookup(flagIds[n]);
    if (p) {
	    fRemaining++;
	    if (((TouchableP)p)->wasTouched()) {
        manager->kill_physical(p);
      }
    }
  }


  if (fRemaining != flagsRemaining) {
    flagsRemaining = fRemaining;
    levelStr <<  "[" <<  level <<  "] COLLECT " 
	     <<  (int)SCENARIO_FLAG_FLAGS 
	     << " FLAGS\nremaining: "<<  flagsRemaining <<   ends;
    lStrChanged = True;

    ostrstream arenaStr;
    if (fRemaining != 0) {
      arenaStr << flagsRemaining << " Flag" <<
	      (flagsRemaining > 1 ? "s" : "") << " Remaining" << ends;
    }
    else {
      arenaStr << "All Flags Collected" << ends;
    }
      
    locator->arena_message_enq(arenaStr.str());
  }

  if (fRemaining == 0) { // All flags are gone, so end level.
    return 1;
  }


  return -1;
}



void CaptureTheFlag::setup_world(WorldP world,LocatorP,const DifficultyLevel &) {
  SpecialMap* map = new FlagMap;
  assert(map);
  world->set_special_map_next(map);
}



void CaptureTheFlag::new_level(int level,WorldP world,LocatorP locator,
                               const DifficultyLevel &,
                               ostrstream &lStr,ostrstream &lTitleStr,
                               IPhysicalManagerP,int) {
  for (int n = 0; n < SCENARIO_FLAG_FLAGS; n++) {
	  Pos pos = world->empty_accessible_rect(Flag::get_size_max());
	  PhysicalP p = new Flag(world,locator,pos);
	  assert(p);
	  locator->add(p);
	  flagIds[n] = p->get_id();
  }
  flagsRemaining = SCENARIO_FLAG_FLAGS;

  lTitleStr << "[" << level << "] Capture The Flag: " 
		  << (int)SCENARIO_FLAG_FLAGS << " Flags" << ends;
  lStr << "[" << level << "] COLLECT " << (int)SCENARIO_FLAG_FLAGS 
	     << " FLAGS.\nremaining: " << (int)SCENARIO_FLAG_FLAGS << ends;
}



void CaptureTheFlag::refill_enemies(Boolean,WorldP,LocatorP locator,IPhysicalManagerP manager) {
  int diff = SCENARIO_FLAG_ENEMIES - locator->enemies_alive();
  assert(diff >= 0);
  
  for (int n = 0; n < diff; n++) {
    manager->create_enemy(NULL);
  }  
}



GenericSeals::GenericSeals() {
  dontAdvance = False;
  sealsOut = sealsSafe = sealsActive = sealsDead = 0;
}



void GenericSeals::setup_world(WorldP world,LocatorP,const DifficultyLevel &) {
  SpecialMap* map = new SealsMap;
  assert(map);
  world->set_special_map_next(map);
}



void GenericSeals::new_level(int level,WorldP world,LocatorP locator,
                         const DifficultyLevel &,
                         ostrstream &lStr,ostrstream &lTitleStr,
                         IPhysicalManagerP,int) {
  // Add a team for each human in the game.
  // Don't use locator->humans_registered() because they may not have been
  // registered at this time.
  TeamOptions ops;
  ops.membersDontCollide = False;

  // Only need one team rule now.
  Boolean (*teamFntn)(LocatorP,PhysicalP,PhysicalP,void *) = get_team();
  locator->add_team(teamFntn,NULL,&ops);


  // You have to save/kill the seals to go on.
  dontAdvance = True;

  // Know these areas are open because they are within the 
  // (OBJECT_COL_MAX,OBJECT_ROW_MAX) margain around the World edge.
  Size worldSize = world->get_size();
  Size homeSize = Home::get_size();
  Size trapdoorSize = Trapdoor::get_size();
  Pos homePos((worldSize.width - homeSize.width) / 2,
		    worldSize.height - WSQUARE_HEIGHT - homeSize.height);
  PhysicalP home = new Home(world,locator,homePos);
  assert(home);
  locator->add(home);
  homeId = home->get_id();


  // Create Trapdoor after Home because it needs homeId.
  Pos trapdoorPos(3 * WSQUARE_WIDTH,WSQUARE_HEIGHT);
  Pos trapdoorPos2(worldSize.width - 3 * WSQUARE_WIDTH - trapdoorSize.width,
                   WSQUARE_HEIGHT);

  PhysicalP trapdoor = new Trapdoor(world,locator,trapdoorPos,homeId);
  locator->add(trapdoor);
  trapdoorIds[0] = trapdoor->get_id();

  PhysicalP trapdoor2 = new Trapdoor(world,locator,trapdoorPos2,homeId);
  locator->add(trapdoor2);
  trapdoorIds[1] = trapdoor2->get_id();

  sealsOut = sealsSafe =  sealsActive = sealsDead = 0;

  // lStr and lTitleStr were in the wrong order, ok now.
  set_level_strings(level,lStr,lTitleStr); 
}



Boolean GenericSeals::advance_level() {
  return !dontAdvance;
}



void GenericSeals::refill_enemies(Boolean,WorldP,LocatorP locator,IPhysicalManagerP manager) {
  int diff = SCENARIO_SEALS_ENEMIES - locator->enemies_alive();
  assert(diff >= 0);
  
  // Change the num argument to enemy_physical if this list is changed.
  int list[] = {A_Ninja,A_Dog,A_Walker};  // No more Hero or Alien

  for (int n = 0; n < diff; n++) {
    manager->create_enemy(manager->enemy_physical(list,3));
  }
}



unsigned int GenericSeals::get_soundtrack() {
  return SoundNames::SOUNDTRACK_SEAL;
}



void GenericSeals::compute_out_safe_active_dead(LocatorP locator,int &lemOut,int &lemSafe,int &lemActive,int &lemDead) {  
  // Lookup home and trapdoors.
  HomeP home = (HomeP)locator->lookup(homeId);
  TrapdoorP trapdoor = (TrapdoorP)locator->lookup(trapdoorIds[0]);
  TrapdoorP trapdoor2 = (TrapdoorP)locator->lookup(trapdoorIds[1]);
  assert(home && trapdoor && trapdoor2);

  // Count seals out, active, and safe.
  // Care about SealIntels, not Seals.
  IntelId seals[SCENARIO_SEALS_SEALS]; 
  lemOut = 0;
  trapdoor->append_seals_out(lemOut,seals);
  trapdoor2->append_seals_out(lemOut,seals);
 
  lemSafe = home->get_seals_safe();
  lemActive = 0;
  lemDead = 0;
  for (int n = 0; n < lemOut; n++) {
    IntelP sealIntel = locator->lookup(seals[n]);	      
    if (sealIntel) {
      PhysicalP seal = locator->lookup(sealIntel->get_id());
	    if (seal) {
        if (seal->alive()) {
	        lemActive++;
        }
        else {
          lemDead++;
        }
      }
    }
    else {
      lemDead++;
    }
  }

  // Don't  count safe seals as dead.
  lemDead -= lemSafe;
  assert(lemDead >= 0);
  lemDead = Utils::maximum(lemDead,0); // keep playing anyway.

  // Can't use this because, for one turn, lemOut is incremented, but lemActive hasn't
  // caught up.
  // lemDead = lemOut - lemSafe - lemActive;
}



Seals::Seals() {
}



int Seals::new_level_check(int,WorldP,LocatorP locator,
                              int level,Boolean &lStrChanged,ostrstream &levelStr,Timer &,
                              IPhysicalManagerP) {  
  int lemOut,lemSafe,lemActive,lemDead;
  compute_out_safe_active_dead(locator,lemOut,lemSafe,lemActive,lemDead);

  // unnecessary.
  sealsDead = lemDead;
      
  // Update the Ui if necessary.
  if (lemSafe != sealsSafe || lemOut != sealsOut ||
      lemActive != sealsActive) {

    // Only update arena string if sealsSafe has changed.
    if (lemSafe != sealsSafe && lemSafe != SCENARIO_SEALS_NEED) {
      ostrstream arenaStr;
      arenaStr << "Save " << (SCENARIO_SEALS_NEED - lemSafe)
               << " More Baby Seals" << ends;
      locator->arena_message_enq(arenaStr.str());
    }

    sealsSafe = lemSafe;
    sealsOut = lemOut;
    sealsActive = lemActive;
    levelStr << "[" << level << "] SAVE " 
            << (int)SCENARIO_SEALS_NEED << " BABY SEALS\n" 
            << "out: " << sealsOut << " safe: " << sealsSafe
            << " dead: " << lemDead
            << ends;
    lStrChanged = True;

    // New level if enough seals are safe or too many are dead.
    if (lemDead > (SCENARIO_SEALS_SEALS - SCENARIO_SEALS_NEED)
	      || lemSafe >= SCENARIO_SEALS_NEED) {
      if (lemSafe < SCENARIO_SEALS_NEED) {
	      ostrstream msg;
	      msg << "YOU FAILED TO SAVE " << (int)SCENARIO_SEALS_NEED
	          << " BABY SEALS. TRY THIS LEVEL AGAIN." << ends;
	      locator->arena_message_enq(msg.str());
      }
      else {
        locator->arena_message_enq(Utils::strdup("Good work.  Go to the next level."));

        // Success!! 
        dontAdvance = False;
      }	

      return 1;
    }
  }

  return -1;    
}



Boolean Seals::award_bonus() {
  return True;
}



Boolean (* Seals::get_team())(LocatorP,PhysicalP,PhysicalP,void *) {
  return seals_team;
}


  
void Seals::set_level_strings(int level,ostrstream &lStr,ostrstream &lTitleStr) {
  lTitleStr << "[" << level << "] SAVE THE BABY SEALS" << ends;
  lStr << "[" << level << "] SAVE "<< 
	  (int)SCENARIO_SEALS_NEED << " BABY SEALS\n" 
	     << "out: 0 safe: 0 dead: 0" << ends;
}



/* Team consists of:
     Seal X Seal
     Seal X (Human | HumanSlave)
     (Human | HumanSlave) X Seal
   Does not have:
     (Human | HumanSlave) X (Human | HumanSlave)
 */
static Boolean Seals_team_check(LocatorP locator,PhysicalP p1,PhysicalP p2) {
  IntelP p1Intel = p1->get_intel();
  IntelP p2Intel = p2->get_intel();
  if (!p1Intel || !p2Intel) {
    return False;
  }

  // p is a Seal
  if (p1Intel->is_seal_intel()) {
    if (p2Intel->is_seal_intel()) {
      return True;
    }
    
    if (p2Intel->is_human()) {
      return True;
    }
    // p2Intel is a machine
    else {
      IntelId p2MasterId = ((MachineP)p2Intel)->get_master_intel_id();
      IntelP p2Master = locator->lookup(p2MasterId);
      if (p2Master && p2Master->is_human()) {
        return True;
      }    
    }
  }
  return False;
}



Boolean Seals::seals_team(LocatorP locator,
                                PhysicalP p1,PhysicalP p2,void *) {
  if (Seals_team_check(locator,p1,p2)) {
    return True;
  }
  if (Seals_team_check(locator,p2,p1)) {
    return True;
  }
  return False;
}



AntiSeals::AntiSeals() {
}



int AntiSeals::new_level_check(int,WorldP,LocatorP locator,
                                  int level,Boolean &lStrChanged,ostrstream &levelStr,Timer &,
                                  IPhysicalManagerP) {  
  // level not over by default.
  int ret = -1;
  
  int lemOut,lemSafe,lemActive,lemDead;
  compute_out_safe_active_dead(locator,lemOut,lemSafe,lemActive,lemDead);


  // Failure
  if (lemSafe > SCENARIO_ANTI_SEALS_MAX_SLIPPAGE) {
	  ostrstream msg;
#if 0
    // Not enough space on screen, need two lines.
	  msg << "YOU LET TOO MANY OF THE FAT BASTARDS GET AWAY\n"
	      << "Try this level again" << ends;
#else
	  msg << "YOU LET TOO MANY OF THE FAT BASTARDS GET AWAY" << ends;
#endif
	  locator->arena_message_enq(msg.str());

    // start new level
    ret = 1;
  }
  // Failure message takes precedence over the rest.
  else {    
    if (lemSafe != sealsSafe) {
      assert(lemSafe > sealsSafe); // strictly increasing.
      locator->arena_message_enq(Utils::strdup("Careful. One just got away."));
    }
    if (lemDead != sealsDead || lemSafe != sealsSafe) {
      int lemToKill = SCENARIO_SEALS_SEALS - lemDead - lemSafe;

      // Success!!    
      if (lemToKill == 0) {
        locator->arena_message_enq(Utils::strdup("Satan approves of your Blood-Lust. Go to the next level."));

        // start new level
        dontAdvance = False;
        ret = 1;
      }
      // Update UI
      else if (lemSafe == sealsSafe) {  // o.w. the "one got away" message takes priority
        ostrstream arenaStr;
        arenaStr << "Kill "
          << lemToKill << " More Baby Seals" << ends;
        locator->arena_message_enq(arenaStr.str());

        levelStr << "[" << level << "] KILL " 
                << lemToKill << " BABY SEALS" << ends;
        lStrChanged = True;
      }
    }  // lemDead != sealsDead
  } // lemSafe check


  sealsOut = lemOut;
  sealsActive = lemActive;
  sealsDead = lemDead;
  sealsSafe = lemSafe;

  return ret;    
}



Boolean (*AntiSeals::get_team())(LocatorP,PhysicalP,PhysicalP,void *) {
  return anti_seals_team;
}


  
void AntiSeals::set_level_strings(int level,ostrstream &lStr,ostrstream &lTitleStr) {
  lTitleStr << "[" << level << "] KILL THE BABY SEALS" << ends;
  lStr << "[" << level << "] KILL "<< 
	  (int)SCENARIO_SEALS_SEALS << " BABY SEALS" << ends;
}



static Boolean anti_seals_team_check(LocatorP locator,PhysicalP p) {
  IntelP pIntel = p->get_intel();
  if (!pIntel) {
    return False;
  }

  // p is a Seal
  if (pIntel->is_seal_intel()) {
    return True;
  }

  // p is a machine
  if (!pIntel->is_human()) {
    IntelP master = locator->lookup(((MachineP)pIntel)->get_master_intel_id());
    // Slaves of humans are not on the team.
    if (master && master->is_human()) {
      return False;
    }
    return True;
  }
  return False;
}



Boolean AntiSeals::anti_seals_team(LocatorP locator,
                                PhysicalP p1,PhysicalP p2,void *) {
  if (anti_seals_team_check(locator,p1) && 
      anti_seals_team_check(locator,p2)) {
    return True;
  }
  return False;
}



KillTheFireDemon::KillTheFireDemon() {
}



int KillTheFireDemon::new_level_check(int enemiesPlaying,WorldP,LocatorP locator,
                                   int,Boolean &,ostrstream &,Timer &,
                                   IPhysicalManagerP) {
  if (enemiesPlaying == 0) {
    locator->arena_message_enq(Utils::strdup("Take That, You Mother-Fucking Spawn From Hell"));
    return 1;
  }
  return -1;    
}



void KillTheFireDemon::setup_world(WorldP world,LocatorP,const DifficultyLevel &) {
  SpecialMap* map = new FireDemonMap;
  assert(map);
  world->set_special_map_next(map);
}



void KillTheFireDemon::new_level(int level,WorldP,LocatorP,
                                 const DifficultyLevel &,
                                 ostrstream &lStr,ostrstream &lTitleStr,
                                 IPhysicalManagerP manager,int) {
  manager->create_enemy(manager->enemy_physical(A_FireDemon));

  lTitleStr << "[" << level << "] FIRE DEMON" << ends;
  lStr << "[" << level << "] Kill the Fire Demon." << ends;
}



Boolean KillTheFireDemon::award_bonus() {
  return True;
}



KillTheDragon::KillTheDragon(){
}



int KillTheDragon::new_level_check(int enemiesPlaying,WorldP,LocatorP locator,
                                   int,Boolean &,ostrstream &,Timer &,
                                   IPhysicalManagerP) {
  if (enemiesPlaying == 0) {
    locator->arena_message_enq(Utils::strdup("The Infernal Worm has been vanquished"));
    return 1;
  }
  return -1;    
}



void KillTheDragon::setup_world(WorldP world,LocatorP,const DifficultyLevel &) {
  Rooms rooms(2,2);
  world->set_rooms_next(rooms);
}



void KillTheDragon::new_level(int level,WorldP w,LocatorP l,
                                 const DifficultyLevel &,
                                 ostrstream &lStr,ostrstream &lTitleStr,
                                 IPhysicalManagerP manager,int) {
  const PhysicalContext *cx = &Dragon::creatureContext.movingContext.physicalContext;
  Pos pos = w->empty_rect(cx->sizeMax);
  PtrList segments;
  Segmented::create_and_add_composite(segments,w,l,Dragon::SEGMENTS_NUM,pos,cx->create,cx->arg);

  for (int n = 0; n < segments.length(); n++) {
    PhysicalP p = (PhysicalP)segments.get(n);
    // Add intel, register enemy, etc.
    // Pass in False, since we already added the object to the Locator.
    manager->create_enemy(p,False);
  }

  lTitleStr << "[" << level << "] Dragon" << ends;
  lStr << "[" << level << "] Kill the Dragon." << ends;
}



Boolean KillTheDragon::award_bonus() {
  return True;
}



ZigZag::ZigZag(){
}



void ZigZag::set_human_data(HumanP human,WorldP world,LocatorP) {
  // Set depth for zig-zag scenario, so doesn't display it immediatly.
  Rooms worldRooms = world->get_rooms();
  human->set_data((void *)(worldRooms.downMax - 1));
}



Pos ZigZag::human_initial_pos(WorldP world,LocatorP,const Size &size) {
  // ZIG_ZAG, start humans in lower left.

  // Doesn't really need to be accessible, just haven't made the
  // RoomIndex version of World::empty_rect().
  Rooms worldRooms = world->get_rooms();
  RoomIndex lowerLeft(worldRooms.downMax - 1,0);
  // Player starts in lower-left-most room.
  return world->empty_accessible_rect(size,lowerLeft);
}



int ZigZag::new_level_check(int,WorldP world,LocatorP locator,
                            int,Boolean &,ostrstream &,Timer &,
                            IPhysicalManagerP) {
  PhysicalP xit = locator->lookup(xitId);
  assert(xit); // The Xit should never be destroyed.
  if (((TouchableP)xit)->wasTouched()) {
	  return 0;
  }

  // See if we need to inform human of change in depth.
  for (int n = 0; n < locator->humans_registered(); n++) {
    HumanP human = locator->get_human(n);
    if (human) {
      // previous depth
      int depthOld = (int)human->get_data();
      Id id = human->get_id();

      // Get physical for intelligence
      PhysicalP p = locator->lookup(id);
      if (p) {
        // Compute current depth
        const Area &area = p->get_area();
        Loc mSquare = area.middle_wsquare();
        Dim roomDim = world->get_room_dim();
        int depth = mSquare.r / roomDim.rowMax;

        if (depth != depthOld) {
	        // so doesn't get set next turn.
	        human->set_data((void *)depth);

          // Use 1-based counting for the User.
	        ostrstream str;
	        str << "Depth " << (depth + 1) << ends;
	        locator->arena_message_enq(str.str(),p);
        }
      }
  	}
  } // for

  return -1;
}



void ZigZag::setup_world(WorldP world,LocatorP ,const DifficultyLevel &) {
  SpecialMap* map = new ZigZagMap;
  assert(map);
  world->set_special_map_next(map);
}



void ZigZag::new_level(int level,WorldP world,LocatorP locator,
                       const DifficultyLevel &,
                       ostrstream &lStr,ostrstream &lTitleStr,
                       IPhysicalManagerP,int) {
  Rooms worldRooms = world->get_rooms();
  RoomIndex upperRight(0,worldRooms.acrossMax - 1);

  // Create the Xit.
  Pos pos = world->empty_accessible_rect(Xit::get_size_max(),upperRight);
  PhysicalP p = new Xit(world,locator,pos);
  assert(p);
  locator->add(p);
  xitId = p->get_id();

  // 1-based counting of depth for the user.
  lTitleStr << "[" << level << "] ZIG-ZAG: Depth " 
		  << worldRooms.downMax << ends;
  lStr << "[" << level << "] ZIG-ZAG.\nFind the exit." << ends;
}



void ZigZag::refill_enemies(Boolean,WorldP,LocatorP locator,IPhysicalManagerP manager) {
  int diff = SCENARIO_ZIG_ZAG_ENEMIES - locator->enemies_alive();
  assert(diff >= 0);
  
  for (int n = 0; n < diff; n++) {
    manager->create_enemy(NULL);
  }
}



Boolean ZigZag::award_bonus() {
  return True;
}



ThePound::ThePound(){
}



int ThePound::new_level_check(int enemiesPlaying,WorldP,LocatorP locator,
                              int,Boolean &,ostrstream &,Timer &,
                              IPhysicalManagerP) {
  if (enemiesPlaying == 0) {
    locator->arena_message_enq(Utils::strdup("Killed the Mutts"));
    return 1;
  }
  return -1;    
}



void ThePound::setup_world(WorldP world,LocatorP,const DifficultyLevel &) {
  Rooms rooms(2,3);
  world->set_rooms_next(rooms);
}



void ThePound::new_level(int level,WorldP,LocatorP locator,
                         const DifficultyLevel &,
                         ostrstream &lStr,ostrstream &lTitleStr,
                         IPhysicalManagerP manager,int) {
  locator->add_team(Scenarios::class_team,(void*)A_Dog,NULL);

  for (int n = 0; n < SCENARIO_POUND_DOGS; n++) {
    manager->create_enemy(manager->enemy_physical(A_Dog));
  }

  lTitleStr << "[" << level << "] The Pound" << ends;
  lStr << "[" << level << "] The Pound" << ends;
}



JapanTown::JapanTown()
{}



int JapanTown::new_level_check(int enemiesPlaying,WorldP,LocatorP locator,
                              int,Boolean &,ostrstream &,Timer &,
                              IPhysicalManagerP) {
  if (enemiesPlaying == 0) {
    locator->arena_message_enq(Utils::strdup("Damn Ninjas.  Worse than roaches."));
    return 1;
  }
  return -1;    
}



void JapanTown::setup_world(WorldP world,LocatorP,const DifficultyLevel &) {
  Rooms rooms(2,3);
  world->set_rooms_next(rooms);
}



void JapanTown::new_level(int level,WorldP,LocatorP locator,
                         const DifficultyLevel &,
                         ostrstream &lStr,ostrstream &lTitleStr,
                         IPhysicalManagerP manager,int) {
  locator->add_team(Scenarios::class_team,(void*)A_Ninja,NULL);

  for (int n = 0; n < SCENARIO_JAPAN_TOWN_NINJAS; n++) {
    manager->create_enemy(manager->enemy_physical(A_Ninja));
  }

  lTitleStr << "[" << level << "] Japan-Town" << ends;
  lStr << "[" << level << "] Japan-Town" << ends;
}



void JapanTown::filter_weapons_and_other_items(LocatorP locator,
                                               int &weaponsNum,
                                               const PhysicalContext *weapons[],
                                               int &,
                                               const PhysicalContext *[]) {
  weapons[0] = locator->get_context(A_Stars);
  weapons[1] = locator->get_context(A_Lancer);
  weaponsNum = 2;
  assert(weapons[0] && weapons[1]);

  // Leave items alone.
}



TheCoop::TheCoop(){
}



int TheCoop::new_level_check(int enemiesPlaying,WorldP,LocatorP locator,
                              int,Boolean &,ostrstream &,Timer &,
                              IPhysicalManagerP) {
  if (enemiesPlaying == 0) {
    locator->arena_message_enq(Utils::strdup("Finger Lickin' Good"));
    return 1;
  }
  return -1;    
}



void TheCoop::setup_world(WorldP world,LocatorP,const DifficultyLevel &) {
  Rooms rooms(4,3);
  world->set_rooms_next(rooms);
}



void TheCoop::new_level(int level,WorldP,LocatorP locator,
                         const DifficultyLevel &,
                         ostrstream &lStr,ostrstream &lTitleStr,
                         IPhysicalManagerP manager,int) {
  locator->add_team(Scenarios::class_team,(void*)A_Chicken,NULL);

  for (int n = 0; n < SCENARIO_COOP_CHICKENS; n++) {
    manager->create_enemy(manager->enemy_physical(A_Chicken));
  }

  lTitleStr << "[" << level << "] The Coop" << ends;
  lStr << "[" << level << "] The Coop" << ends;
}



LookOut::LookOut() {
  timer.set_max(WEIGHT_TIME);

  // other members get initialized in new_level().
}



Pos LookOut::human_initial_pos(WorldP world,LocatorP,const Size &size) {
  // start humans in leftmost room.

  Size worldSize = world->get_size();
  Size roomSize = world->get_room_size();
  Pos ret(Utils::choose(roomSize.width / 2),
          worldSize.height - WSQUARE_HEIGHT - size.height);
  return ret;
}



int LookOut::new_level_check(int,WorldP world,LocatorP locator,
                             int,Boolean &,
                             ostrstream &,Timer &,
                             IPhysicalManagerP) {
  PhysicalP xit = locator->lookup(xitId);
  assert(xit); // The Xit should never be destroyed.
  if (((TouchableP)xit)->wasTouched()) {
	  return 0;
  }

  if (timer.ready()) {
    // Create one new Heavy object.
    create_heavy(world,locator);
    timer.set();
  }
  timer.clock();

  return -1;
}



void LookOut::setup_world(WorldP world,LocatorP,const DifficultyLevel &) {
  SpecialMap* map = new LookOutMap;
  assert(map);
  world->set_special_map_next(map);
}



void LookOut::new_level(int level,WorldP world,LocatorP locator,
                        const DifficultyLevel &,
                        ostrstream &lStr,ostrstream &lTitleStr,
                        IPhysicalManagerP,int) {

  Id invalid;
  // Make sure all weights start out invalid.
  for (int n = 0; n < WEIGHTS_MAX; n++) {
    weights[n] = invalid;
  }  
  weightNext = 0;


  // Create the Xit in lower-right corner.
  Size worldSize = world->get_size();
  Size xitSize = Xit::get_size_max();
  Pos pos(worldSize.width - WSQUARE_WIDTH - xitSize.width * 2,
          worldSize.height - WSQUARE_HEIGHT - xitSize.height);
  PhysicalP p = new Xit(world,locator,pos);
  assert(p);
  locator->add(p);
  xitId = p->get_id();

  lTitleStr << "[" << level << "] Chicken Little" << ends;
  lStr << "[" << level << "] The sky is falling.\nFind the exit." << ends;
}



void LookOut::create_heavy(WorldP w,LocatorP l) {
  PhysicalP oldW = l->lookup(weights[weightNext]);
  // Kill off existing weight if it's still there.
  if (oldW) {
    oldW->kill_self();
  }

  // Decide whether to create Weight or Rock.
  const PhysicalContext* pc;
  if (Utils::coin_flip()) {
    pc = l->get_context(A_Weight);
  }
  else {
    pc = l->get_context(A_Rock);
  }

  Size worldSize = w->get_size();
  Size weightSize = pc->sizeMax;
  Size roomSize = w->get_room_size();
  // Don't create in the first(leftmost) room
  Pos pos(Utils::choose(worldSize.width - roomSize.width 
                        - weightSize.width - WSQUARE_WIDTH) 
          + roomSize.width,
          W_EMPTY_DEAD_ROWS * WSQUARE_HEIGHT + 2 * weightSize.height);

  PhysicalP newW = pc->create(pc->arg,w,l,pos);
  assert(newW);
  l->add(newW);
  weights[weightNext] = newW->get_id();

  // A little initial velocity upwards.
  Vel initVel(0,LOOK_OUT_INIT_VEL);
  ((MovingP)newW)->set_vel_next(initVel);

  // Kind of a hack.  On this level any Heavy object in the air can hurt.
  // So that ChopperBoys/Ninjas can't just cruise along on the top of the 
  // ceiling, only hitting the Heavies on the side and not taking damage.
  ((HeavyP)newW)->set_always_hurts_in_air();

  // Move to next spot.
  weightNext = (weightNext + 1) % WEIGHTS_MAX;
}
