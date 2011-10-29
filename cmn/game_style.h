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

// game_style.h

#ifndef GAME_STYLE_H
#define GAME_STYLE_H

#if X11 
	#ifndef NO_PRAGMAS
	#pragma interface
	#endif
#endif 

#if X11
#include <strstream>
#endif
#if WIN32
#include <strstrea.h>
#endif

#include "utils.h"
#include "coord.h"
#include "world.h"
#include "locator.h"
#include "intel.h"


#define SCENARIO_SEALS_TRAPDOORS 2
#define SCENARIO_BONUS_FROGS 12
#define SCENARIO_FLAG_FLAGS 15

struct IntelOptions;
class Human;
typedef Human *HumanP;

struct PhysicalContext;

class IPhysicalManager {
public:
  virtual PhysicalP create_enemy(PhysicalP p,Boolean addToLocator = True) = 0;
  /* EFFECTS: Create and return new enemy with intelligence.  If p is non-NULL, use it
     as the Physical.  Else choose Physical via enemy_physical(void). */

  virtual PhysicalP enemy_physical() = 0;
  virtual PhysicalP enemy_physical(ClassId classId) = 0;
  virtual PhysicalP enemy_physical(ClassId *classIds,int num) = 0;
  /* EFFECTS: Ways of creating a Physical for an enemy:
     1. Choose randomly.
     2. Specify exact class.
     3. Choose randomly from a set of classes.
   */

  virtual ITmask intel_options_for(IntelOptions &ops,ClassId classId) = 0;
  /* EFFECTS: Return appropriate IntelOptions for a specific Class. */
  
  virtual void kill_physical(PhysicalP p) = 0;
  /* EFFECTS: Remove p from the game silently. */
};
typedef IPhysicalManager *IPhysicalManagerP;



class Scenario;
typedef Scenario *ScenarioP;



class GameStyle {
public:
  GameStyle();
  virtual ~GameStyle();
  
  virtual GameStyle *clone() = 0;
  
  virtual void describe(std::ostrstream &) = 0;
  /* EFFECTS: Write a two-line description of the style to the stream. */

  virtual GameStyleType get_type() = 0;

  static GameStyle *by_type(GameStyleType);
  /* EFFECTS: Create a new style given the type. */

  static Boolean uses_enemies_num(GameStyleType);
  static Boolean uses_enemies_refill(GameStyleType);
  /* EFFECTS: Does the specific style use enemiesNum/enemiesRefill?
     Used for enabling/disabling UI. */
  /* NOTE: Sucks that this isn't extendable. */

  virtual Boolean class_friends();
  /* EFFECTS: Are Enemies classFriends with each other. */
  /* DEFAULT: True */

  virtual int human_initial_lives();
  /* DEFAULT: HUMAN_LIVES */

  virtual void set_human_data(HumanP,WorldP,LocatorP);
  /* DEFAULT: do nothing */

  virtual Pos human_initial_pos(WorldP,LocatorP,const Size &s);
  /* DEFAULT: world.empty_rect(s) */

  virtual Boolean can_refill_game_objects();
  /* DEFAULT: True */

  virtual Boolean need_difficulty(int enemiesNumNext);
  /* DEFAULT: True */

  virtual char *can_reset(int humansNumNext,int enemiesNumNext,
                          Boolean cooperative);
  /* EFFECTS: Can this GameStyle be used with the given settings.
     Return error message if not, must free return value. 
     DEFAULT: return NULL.
  */
    
  virtual void reset(WorldP,LocatorP,const DifficultyLevel &,
                     int enemiesNumNext) = 0;
  /* EFFECTS: Start a new game. */

  virtual int new_level_check(int enemiesPlaying,WorldP,LocatorP,
                              int level,
                              Boolean &lStrChanged,std::ostrstream &levelStr,
                              Timer &timer,
                              IPhysicalManagerP);
  /* MODIFIES: lStrChanged */
  /* EFFECTS: Return 
       -1   to continue with current level,
       0    to immediately start a new level,
       1    to start a new level after a few turns.
     Use levelStr to set Ui::set_level() and mark lStrChanged as True.
     lStrChanged will always be passed in as False. Timer is the game's Timer
     for the current state. level is the current level.
     DEFAULT: Return 0 if Timer has expired, else -1. */

  virtual Boolean game_over_check(int humansNum,int humansPlaying,
                                  int enemiesPlaying);
  /* EFFECTS: Return True if the game is over.  humansNum is the total number
     of humans involved in the game for this Role.  humansPlaying,
     enemiesPlaying give the number that are still alive or have extra lives
     left. */
  /* NOTE: Caller will set humansPlaying=HU_UNSPECIFIED to indicate that
     the number of humans playing in the game may change, see coord.h.
     Mostly for client/server games. */
  /* DEFAULT: True if humansPlaying == 0. */

  virtual Boolean advance_level();
  /* EFFECTS: Should we increment the level number counter when we 
     start the next level. */
  /* DEFAUlT: True */

  virtual Boolean award_bonus();
  /* EFFECTS: Should we award all humans a bonus when they beat this level.  
     A bonus
     will never be awarded if advance_level() returns False. */
  /* DEFAULT: False */

  virtual void new_level(int level,WorldP,LocatorP,
                         const DifficultyLevel &dLevel,
                         std::ostrstream &levelStr,std::ostrstream &levelTitleStr,
                         IPhysicalManagerP,int humansNum) = 0;
  /* MODIFIES: level */
  /* EFFECTS: Start a new level.  Use levelStr levelTitleStr to 
     set the messages for the user. dontAdvance is the result passed out of
     new_level_check().  Must call world.reset() and clean out any 
     Physicals hanging 
     around from last level. For the very first level, level will be -1 
     at the start
     of this call. So, the first level played will be level 0. */

  virtual void new_level_set_timer(Timer &timer);
  /* MODIFIES: timer */
  /* EFFECTS: After getBearings time, set a timer for the level if desired. */
  /* DEFAULT: timer.set(LEVEL_TIME) */

  virtual void refill_enemies(Boolean enemiesRefill,WorldP,LocatorP,
                              IPhysicalManagerP) = 0;
  /* EFFECTS: Refill enemies in the Game.  enemiesRefill is the preference
     setting. */

  virtual void filter_weapons_and_other_items(LocatorP,int &weaponsNum,
                                              const PhysicalContext *
                                              weapons[A_CLASSES_NUM],
                                              int &oItemsNum,
                                              const PhysicalContext *
                                              oItems[A_CLASSES_NUM]);
  /* REQUIRES: weapons[] and oItems[] both have space for A_CLASSES_NUM 
     elements. */
  /* EFFECTS: The complete set of possible weapons and other items is
     passed in.
     Change the lists to determine which weapons/items are acceptable 
     candidates 
     for the level. */
  /* MODIFIES: weaponsNum,weapons,oItemsNum,oItems */
  /* DEFAULT: Do nothing. */

  virtual unsigned int get_soundtrack();
  /* DEFAULT: IDW_SOUNDTRACK */

  virtual SoundName get_midisoundtrack();
  /* DEFAULT: SOUND_RANDOM */


#ifndef PROTECTED_IS_PUBLIC
protected:
#endif
  virtual void clean_physicals(Boolean doMinimum,WorldP,LocatorP,
                               IPhysicalManagerP);
  /* EFFECTS: Deal with existing Physicals in the locator, possibly 
     killing them or 
     moving them to stay inside the world. doMinimum=True means try 
     to leave everything alone,
     just relocate players if necessary.  doMimimum=False means kill 
     everything except human 
     players, relocate humans if necessary. */


private:
  enum {KILL_OFF,RELOCATE,LEAVE_ALONE};

  int partition(PhysicalP p,Boolean doMinimum,WorldP,LocatorP);
  /* EFFECTS: Return KILL_OFF, RELOCATE, or LEAVE_ALONE to determine 
     what should happen to the Physical during clean_physicals().  
     Does not deal with followers. */

  void get_followers_recursive(PtrList& leaveAlone,PhysicalP p);
  /* MODIFIES: leaveAlone */
  /* EFFECTS: Append to leaveAlone all followers of p and of p's 
     followers.  (Does not append p itself.) */
};
typedef GameStyle *GameStyleP;



// Levels, except put in a scenario every few levels.
class Normal : public GameStyle {
public:
  Normal();
  ~Normal();

  virtual GameStyleP clone();

  virtual void describe(std::ostrstream &);
  
  virtual GameStyleType get_type();

  virtual int human_initial_lives();

  virtual void set_human_data(HumanP,WorldP,LocatorP);

  virtual Pos human_initial_pos(WorldP,LocatorP,const Size &s);

  virtual Boolean can_refill_game_objects();

  virtual char *can_reset(int humansNumNext,int enemiesNumNext,
                          Boolean cooperative);

  virtual void reset(WorldP,LocatorP,const DifficultyLevel &,
                     int enemiesNumNext);

  virtual int new_level_check(int enemiesPlaying,WorldP,LocatorP,
                              int level,Boolean &lStrChanged,
                              std::ostrstream &levelStr,Timer &timer,
                              IPhysicalManagerP);

  virtual Boolean advance_level();

  virtual Boolean award_bonus();

  virtual void new_level(int level,WorldP,LocatorP,const DifficultyLevel &dLevel,
                         std::ostrstream &levelStr,std::ostrstream &levelTitleStr,
                         IPhysicalManagerP,int humansNum);

  virtual void new_level_set_timer(Timer &timer);

  virtual void refill_enemies(Boolean enemiesRefill,WorldP,LocatorP,IPhysicalManagerP);
  
  virtual unsigned int get_soundtrack();

  virtual SoundName get_midisoundtrack();


private:
  GameStyleP get_delegate() {if (playScenarios) return scenarios; else return levels;}

  Boolean playScenarios;
  GameStyleP levels;
  GameStyleP scenarios;  
};



class Levels : public GameStyle {
public:
  Levels();
  
  virtual GameStyleP clone();

  virtual void describe(std::ostrstream &);
  
  virtual GameStyleType get_type();

  virtual char *can_reset(int humansNumNext,int enemiesNumNext,Boolean cooperative);

  virtual void reset(WorldP,LocatorP,const DifficultyLevel &,int enemiesNumNext);

  virtual int new_level_check(int enemiesPlaying,WorldP,LocatorP,
                              int level,Boolean &lStrChanged,std::ostrstream &levelStr,Timer &timer,
                              IPhysicalManagerP);

  virtual void new_level(int level,WorldP,LocatorP,const DifficultyLevel &dLevel,
                         std::ostrstream &levelStr,std::ostrstream &levelTitleStr,
                         IPhysicalManagerP,int humansNum);

  virtual void new_level_set_timer(Timer &timer);

  virtual void refill_enemies(Boolean enemiesRefill,WorldP,LocatorP,IPhysicalManagerP);
  
  virtual unsigned int get_soundtrack();


private:
  int enemiesNum; // Incremented each level.
};



class KillKillKill : public GameStyle {
public:
  KillKillKill();

  virtual GameStyleP clone();

  virtual void describe(std::ostrstream &);

  virtual GameStyleType get_type();

  virtual Boolean class_friends();

  virtual Boolean need_difficulty(int enemiesNumNext);

  virtual char *can_reset(int humansNumNext,int enemiesNumNext,Boolean cooperative);

  virtual void reset(WorldP,LocatorP,const DifficultyLevel &dLevel,int );

  virtual Boolean game_over_check(int humansNum,int humansPlaying,
                                  int enemiesPlaying);

  virtual void new_level(int level,WorldP,LocatorP,const DifficultyLevel &dLevel,
                         std::ostrstream &levelStr,std::ostrstream &levelTitleStr,
                         IPhysicalManagerP,int humansNum);

  virtual void refill_enemies(Boolean enemiesRefill,WorldP,LocatorP,IPhysicalManagerP);

  virtual SoundName get_midisoundtrack();


private:  
  Boolean justReset;  // After reset(), before new_level().
  int enemiesNum; // Stays constant.
};



class Duel : public GameStyle {
public:
  Duel();
  
  virtual GameStyleP clone();

  virtual void describe(std::ostrstream &);

  virtual GameStyleType get_type();

  virtual Boolean need_difficulty(int enemiesNumNext);

  virtual char *can_reset(int humansNumNext,int enemiesNumNext,Boolean cooperative);

  virtual void reset(WorldP,LocatorP,const DifficultyLevel &dLevel,int );

  virtual Boolean game_over_check(int humansNum,int humansPlaying,
                                  int enemiesPlaying);

  virtual void new_level(int level,WorldP,LocatorP,const DifficultyLevel &dLevel,
                         std::ostrstream &levelStr,std::ostrstream &levelTitleStr,
                         IPhysicalManagerP,int humansNum);

  virtual void refill_enemies(Boolean enemiesRefill,WorldP,LocatorP,IPhysicalManagerP);

  
private:
  Boolean justReset;  // After reset(), before new_level().
  int enemiesNum; // Stays constant.
};



class ExtendedDuel : public GameStyle {
public:
  ExtendedDuel();
  
  virtual GameStyleP clone();

  virtual void describe(std::ostrstream &);

  virtual GameStyleType get_type();

  virtual int human_initial_lives();

  virtual Boolean need_difficulty(int enemiesNumNext);

  virtual char *can_reset(int humansNumNext,int enemiesNumNext,Boolean cooperative);

  virtual void reset(WorldP,LocatorP,const DifficultyLevel &dLevel,int );

  virtual Boolean game_over_check(int humansNum,int humansPlaying,
                                  int enemiesPlaying);

  virtual void new_level(int level,WorldP,LocatorP,const DifficultyLevel &dLevel,
                         std::ostrstream &levelStr,std::ostrstream &levelTitleStr,
                         IPhysicalManagerP,int humansNum);

  virtual void refill_enemies(Boolean enemiesRefill,WorldP,LocatorP,IPhysicalManagerP);


private:
  Boolean justReset;  // After reset(), before new_level().
  int enemiesNum; // Stays constant.
};



class Training : public GameStyle {
public:
  Training();

  virtual GameStyleP clone();

  virtual void describe(std::ostrstream &);

  virtual GameStyleType get_type();

  virtual Boolean need_difficulty(int enemiesNumNext);

  virtual void reset(WorldP,LocatorP,const DifficultyLevel &dLevel,int);

  virtual void new_level(int level,WorldP,LocatorP,const DifficultyLevel &dLevel,
                         std::ostrstream &levelStr,std::ostrstream &levelTitleStr,
                         IPhysicalManagerP,int humansNum);

  virtual void refill_enemies(Boolean enemiesRefill,WorldP,LocatorP,IPhysicalManagerP);
};



class Scenarios : public GameStyle {
public:
  Scenarios();
  virtual ~Scenarios();

  virtual GameStyleP clone();

  virtual void describe(std::ostrstream &);

  virtual GameStyleType get_type();

  virtual Boolean class_friends();

  virtual int human_initial_lives();

  virtual void set_human_data(HumanP,WorldP,LocatorP);

  virtual Pos human_initial_pos(WorldP,LocatorP,const Size &s);

  virtual Boolean can_refill_game_objects();

  virtual char *can_reset(int humansNumNext,int enemiesNumNext,Boolean cooperative);

  virtual void reset(WorldP,LocatorP,const DifficultyLevel &,int enemiesNumNext);

  virtual int new_level_check(int enemiesPlaying,WorldP,LocatorP,
                              int level,Boolean &lStrChanged,std::ostrstream &levelStr,Timer &timer,
                              IPhysicalManagerP);

  virtual Boolean advance_level();

  virtual Boolean award_bonus();

  virtual void new_level(int level,WorldP,LocatorP,const DifficultyLevel &dLevel,
                         std::ostrstream &levelStr,std::ostrstream &levelTitleStr,
                         IPhysicalManagerP manager,int humansNum);

  virtual void new_level_set_timer(Timer &timer);

  virtual void refill_enemies(Boolean enemiesRefill,WorldP,LocatorP,IPhysicalManagerP);

  static void set_override(char *val);
  /* EFFECTS: If set, always choose the given scenario. 
     Don't ever delete the passed in value, not copied. */

  virtual void filter_weapons_and_other_items(LocatorP,int &weaponsNum,
                                              const PhysicalContext *weapons[],
                                              int &oItemsNum,
                                              const PhysicalContext *oItems[]);

  virtual unsigned int get_soundtrack();


  virtual SoundName get_midisoundtrack();

  static Boolean class_team(LocatorP,PhysicalP p1,PhysicalP p2,void* closure);
  /* EFFECTS: Team composed of all enemies of a single class. 
     The classId will be passed in as the closure. */
  /* NOTE: Left public so that Game can reuse this team definition.  
     Not the best place to put it, but slightly better than just making a
     global function. */


private:
  static Boolean class_team_member(PhysicalP,ClassId);
  /* EFFECTS: Helper for class_team. */

  void choose_scenario();
  /* EFFECTS: Delete old scenario and create a new one.  Works even if 
     scenario is NULL. */

  ScenarioP scenario;
  static char *override;
};
#endif
