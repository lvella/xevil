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

// "game.h"

#ifndef GAME_H
#define GAME_H

#if X11
	#ifndef NO_PRAGMAS
	#pragma interface
	#endif
#endif


// Include Files
extern "C" {
#include <time.h>
}
#include <iostream>
#include "utils.h"
#include "ui.h"
#include "world.h"
#include "locator.h"
#include "actual.h"
#include "role.h"
#include "xdata.h"
#include "sound.h"


// Class Declarations
class Game;
typedef Game *GameP;

enum RestartEnd {RE_NONE,RE_RESTART,RE_END};


class GameStats {
 public:
  GameStats() {reset();}
  long num_turns() {return numTurns;}
  void reset() {aveTime = 0; numTurns = 0;}
  void report();
  void clock();
  

 private:
  long numTurns;
  time_t prevTime;
  float aveTime; /* In seconds */
};



// New scalable, way using Locator::filter_contexts.
class GameObjects {
 public:
  GameObjects(WorldP, LocatorP);
  ~GameObjects();

  void game_reset(const char *oneItem,Boolean noItems,
		  Boolean oneEach);
  /* EFFECTS: Initialize  values for a new game.  Does not 
     actually create objects.  oneItem not empty means there will be only one 
     item in the world, the value of oneItem.  Otherwise, noItems means there 
     will be no items.  Otherwise regular items.  oneEach will cause
     one of each item to be used. */
  /* NOTE: Must be called once before GameObjects::refill. */
  /* REQUIRES: The memory for oneItem is not copied internally, so don't 
     ever delete it. */

  void level_reset(const Dim &worldDim,GameStyleP);
  /* EFFECTS: Choose how many of each class of objects to use for the 
     next level. */
  /* IMPLEMENTATION NOTE: Deal with memory for ids[]. */

  void refill();
  /* EFFECTS: Create and add objects as necessary. */
  /* REQUIRES: GameObjects::game_reset has already been called. */


private:
  void refill_helper(const PhysicalContext *cx,int which);
  /* EFFECTS:  Make it so that actuals[which] of the Item exist.  I.e. create
     (actuals[which] - <current number>) new Items. */

  void compute_actuals(const PhysicalContext *weapons[],int weaponsNum,
		       const PhysicalContext *oItems[],int oItemsNum);
  /* EFFECTS: Compute actuals from maximums.  weapons and oItems are the lists
   of potential weapons and potential otherItems. */

  Boolean is_available(const Id &id);
  /* EFFECTS: Decide whether to count id as still being available for players
     to use. */

  static Boolean potential_weapon_filter(const PhysicalContext*,void*);
  static Boolean potential_other_item_filter(const PhysicalContext*,void*);
  /* NOTE: Used for Locator::filter_context. */

  WorldP world;
  LocatorP locator;

  Boolean resetCalled;
  int maximums[A_CLASSES_NUM];  // Maximum num that could be created this level.
  int actuals[A_CLASSES_NUM];  // The actual num that will be here this level
  int levelAlready[A_CLASSES_NUM]; // Number already generated this level.
  int levelMaxTimes[A_CLASSES_NUM]; // Maximum number that can be generated this level. (0 == infinity)
  Id *ids[A_CLASSES_NUM];

  const char *oneItem;
  Boolean noItems;
  Boolean oneEach;
};



// A set of ranks that are of approximate equal worth.
struct RankingSet {
  enum {RANKING_SET_MAX = 8}; // Max number of rankings in a set.
  int killsMin; // Min number of kills to get this rank.
  char *rankings[RANKING_SET_MAX + 1]; // Extra for NULL.
};



class Game: public IPhysicalManager, public IGameManager {
 public:
  enum {
    // In turns.
    REPORT_TIME = 200
  };
  /* NOTE: Actually should be private.  Lame-ass Silicon Graphics compiler. */


  Game(int *argc,char **argv);
  ~Game();

  static const char* get_version_string();

#if WIN32
  int getWindowList(CTypedPtrList<CPtrList,CWnd *>&p_windowlist);
  /* EFFECTS: Get pointers to all windows created by UI.  For 
     event handling. */
#endif

  Boolean quit_game(){return quitGame;}
  /* NOTE: Role may instruct game to exit. */

#if X11
  Display *get_dpy(int dpyNum) 
  {return ui ? ui->get_dpy(dpyNum) : (Display *)NULL;}
#endif

  int get_dpy_max() {return ui ? ui->get_dpy_max() : 0;}

  Boolean has_ui() {return (ui != NULL);}
  /* NOTE: We may be playing as Server with no ui. */

  Quanta get_quanta() {return quanta;}
  /* EFFECTS: How often should the game be clocked. (milliseconds) */

  // It might be better to hide all the role-specific code inside Game. 
  RoleP get_role() {return role;}

  Boolean show_stats() {return showStats;}
  
  void pre_clock();
  /* EFFECTS: Portion of one cycle of game play prior to reading all the
     events for this turn. */

  void post_clock();
  /* EFFECTS: Portion of one cycle of game play after reading the events
     from the queue. */

  void yield(CMN_TIME time,int quanta,ITurnStarterP);
  /* EFFECTS: Yield given amount of time to processor.  Process any sockets,
     etc. */

  Boolean process_event(int dpyNum,CMN_EVENTDATA event) 
#if WIN32
  {return ui->process_event(dpyNum,event);}
#endif
#if X11
  {if (ui) ui->process_event(dpyNum,event); return False;}
#endif
  /* WINDOWS NOTE: If didnt process, return False. */
  /* UNIX NOTE: Return value is meaningless. */
  
  static ClassId parse_class_name(LocatorP l,const char *name);
  /* EFFECTS: Given the class name corresponding to a registered 
     PhysicalContext, return the ClassId of the PhysicalContext, or 
     A_None if not found. */
  /* NOTE: Pass in Locator so that we can export this as a static function. */


/****************** Implementation for IPhysicalManager ******************/

  virtual PhysicalP create_enemy(PhysicalP p,Boolean addToLocator = True);
  /* EFFECTS: Create enemy player, give it a Physical, register_enemy
     and add to locator.  The Physical will be p if non-NULL.  If p is NULL,
     use enemy_physical() to create a enemy.  Return the Physical. */

  virtual PhysicalP enemy_physical();
  /* EFFECTS: Randmoly choose a Physical for an Enemy. */

  virtual PhysicalP enemy_physical(ClassId classId);
  /* EFFECTS: Create a Physical of class classId. */

  virtual PhysicalP enemy_physical(ClassId *classIds,int num);
  /* EFFECTS: Create a Physical of a class in classIds or choose randomly if
     num is zero.  Otherwise, num is the length of classIds. */

  virtual ITmask intel_options_for(IntelOptions &ops,ClassId classId);
  /* EFFECTS: Return the default IntelOptions and mask for an (Enemy) 
     Intelligence in a body of class classId. */
  /* MODIFIES: ops */

  virtual void kill_physical(PhysicalP p);

/************************* End IPhysicalManager **************************/



/****************** Implementation for IGameManager ******************/

  virtual void world_changed();
  
  virtual void manager_reset(GameStyleType);

  virtual void register_intel(int num,IntelP intel);
  /* EFFECTS: Associate intel with viewport num. */

  virtual void humans_num_incremented();

  virtual void humans_num_decremented(const IntelId &);

  virtual void quit();

  virtual void set_humans_playing(int);

  virtual void set_enemies_playing(int);

  virtual Quanta manager_get_quanta();

  virtual GameStyleType get_game_style_type();
  
/************************* End IGameManager **************************/
			     
 
  
private:
  enum { 
    WITTY_SAYINGS_NUM = 148,
    INTEL_NAMES_NUM  = 74,
    RANKING_SETS_NUM = 12,
  };

  void ui_settings_check(RestartEnd &);
  /* EFFECTS: Process any commands that the Ui has received. */
  
  void set_style_next(GameStyleType);
  /* EFFECTS: Sets the style in Game and Ui, and send appropriate message. */

  void reincarnations_check();
  
  void game_over_check(int humansPlaying,int enemiesPlaying);
  /* EFFECTS: Set extraTimer if the game over conditions for the current style
     have been met.  Call end_game() when the Timer expires. */
  
  void new_level_check(int enemiesPlaying);
  /* EFFECTS: Called in clock().  Sets newLevelTimer if the level has been 
     completed.  Call new_level() when the Timer expires. */

  void refill_check();
  /* EFFECTS: Refill items in the game.  Depends on style and scenario. */
  
  static Boolean potential_human_filter(const PhysicalContext* pc,void*);
  /* EFFECTS: Filter those classes whose potentialHuman flag is True. */
  
  PhysicalP human_physical(int humanNum = 0);
  /* USES: Game::humanClass */
  /* EFFECTS: Create a Physical appropriate for a Human. */
  /* NOTE: humanNum is something of a hack. */
  /* NOTE: Also adds the object(s) to the Locator.  Needed so that 
     human_physical
     can create and add composite objects. */
  
  static Boolean potential_enemy_filter(const PhysicalContext* pc,void*);
  /* EFFECTS: Filter those classes whose potentialEnemy flag is True. */

  char *choose_ranking(int kills);
  /* EFFECTS: Choose a rank for the player at the end of game based on the
     number of kills. */
  
  void end_game(Boolean showMessages);
  /* EFFECTS: End the game immediately. showMessages specifies whether to 
     report ranking/kills/etc. */

  void new_level();
  /* EFFECTS: Clean up after previous level and start a new level. */

  void award_bonuses_now();
  /* EFFECTS: Award a bonus to all human players.  Put a message up on the screen for each. */

  void get_bearings(char* levelTitleStr);
  /* MODIFIES: levelTitleStr */
  /* EFFECTS: Set state to getBearings for the appropriate amount of time.  levelTitleStr
     is the message to display while the user waits for the next level to start. */
  /* NOTE: get_bearings() takes ownership for the memory of levelTitleStr. */

  void reset();
  /* EFFECTS: End the game and start a new one. */

  void new_game();
  /* EFFECTS: Before reset(), sets NEW_GAME_MESSAGE. */

  void process_x_resources(int *argc,char **argv);
  /* EFFECTS: Read in x resources. Sets default keyset for the Ui. */

  void parse_args(int *argc,char **argv);
  /* EFFECTS: Parses the command line, setting all options except
   politically correct. */

  char **display_names(int *argc,char **argv);
  /* EFFECTS: Returns an array of length UI_VIEWPORTS_MAX of strings 
     containing the display names for all possible viewports.  
     "" means use the local display.  Must delete both the array and the 
     strings contained in it when done. */

  char *font_name(int *argc,char **argv);
  /* EFFECTS: Check for the font name in the argument list. */

  void humans_reset();
  /* EFFECTS: Create new human players and create/destroy viewports and 
     register the human playsers as necessary. */

  void create_human_and_physical(int h);
  /* EFFECTS: Create human number h and an appropriate physical. */

#if WIN32
  void Game::change_screen_mode(Xvars::SCREENMODE p_newmode);
  /* EFFECTS: destroys current ui and initializes another with p_newmode
    keeps game state exactly as it was. */
#endif

  void intro();
  /* EFFECTS: Put info messages on cout and on the Locator message
     queue. */ 

  static Boolean stats_creations_filter(const PhysicalContext*,void*);
  static Boolean stats_uses_filter(const PhysicalContext*,void*);
  static Boolean stats_deaths_filter(const PhysicalContext*,void*);
  /* NOTE: Helpers for print_stats(). */

  void print_stats();
  /* EFFECTS: Print out current statistics for the Game to cout. */

  void demo_setup();
  /* EFFECTS: Create enemy players for the demo. */

  static Boolean universal_team(LocatorP,PhysicalP,PhysicalP,void *);
  /* EFFECTS: Put all objects on the same team, used for demo. */
  
#if WIN32
  void start_soundtrack();
  /* EFFECTS: Starts soundtrack, sets up soundManager.  Will stop
     current soundtrack first, before starting new one. */

  void stop_soundtrack();
  /* EFFECTS: Stop current soundtrack. */  
#endif

  // public for GameObjects, probably we should give GameObjects a pointer to
  // a IPhysicalManager.
public:
  static void off_clock_kill(LocatorP,PhysicalP);
  /* EFFECTS: Kill a Physical when control is not in Locator::clock().
     I.e. not during regular game play. */
private:
	
  static Boolean humans_team(LocatorP,PhysicalP,PhysicalP,void *);
  /* EFFECTS: All humans and their slaves are on the same team. */

  static Boolean one_human_team(LocatorP,PhysicalP,PhysicalP,void *);
  /* EFFECTS: Team for one human and his slaves. */

  int limit_humans_num(int);

  void play_sounds();
  /* EFFECTS: plays all sounds located in the sound registry. */

  
  SoundManager soundManager;
  SoundName currentSoundName;
  //keeps track of current sound track being played: can be 0

  static char *wittySayings[WITTY_SAYINGS_NUM];
  static char *intelNames[INTEL_NAMES_NUM];
  static DifficultyLevel difficultyLevels[DIFFICULTY_LEVELS_NUM];
  static RankingSet rankingSets[RANKING_SETS_NUM];

  enum GameState {
    demoOff, // Before game starts, just showing title screen. 
    demoOn, // Before game starts, playing a demo.
    demoExtra, // Few extra turns after end of one demo before start of next.
    newGame, // Starting a new game.
    getDifficulty, // UNIX only, waiting for user to enter difficulty level.
		getBearings, // Beginning of new level, let the user see what's going on before starting.
    gameOn, // Normal game play.
    levelExtra, // Few extra turns after level is beat, but before starting new level.
    awardBonus, // Few turns before getBearings to tell users about awarded bonuses.
    gameExtra, // Few extra turns after game is over.
    gameOver, // Game is over, everything is frozen.
    roleFailed, // Role (Client or Server) failed for some reason.
    prepareConnect, // Client is in the process of connecting to a server.
  };

  Quanta quanta;  // Time between each turn in milliseconds.
  Boolean showStats;
  World world;
  Rooms worldRooms; //Value in UI.
  char *worldFile; // From parse_args().
  Locator locator; // Initialized after world.
  Ui *ui; // Must be initialized after the other two.
  GameStats stats;
  int *argc;
  char **argv; // Careful, pointer to original argv, not a copy.
  int viewportsNum;
  int humansNumNext; // Next is mostly just a reflection of the UI.
  // From -name command line argument.  Each will be chosen randomly if "".  
  char *humanNames[Locator::HUMANS_MAX]; 
  int enemiesNumNext; // What appears on the Ui.
#if WIN32
  UIsettings::SOUNDTRACKTYPE musictype; //play tracks from cd? or MIDI or wav
  CString mididirectory; 
#endif
  Boolean cooperative,cooperativeNext;
  int sharedLives; // Value shared by all Humans if playing cooperatively.

  Boolean pause;  // Is the game paused.
  GameState state;
  Timer timer; // For state transitions.

  Boolean intelHarmless;
  int humansPlayingPrev; // Just used for displaying info.
  int enemiesPlayingPrev;
  Boolean noItems; // Mostly for testing.
  Boolean oneEach;
  char *oneItem; // Only initialize one item if not "".
  ClassId humanClass; // Choose randomly if A_None.
  AbilityId humanExtraAbility;
  Boolean infiniteLives; // the -infinity command-line option

  Boolean enemiesRefill,enemiesRefillNext; // Keep regenerating enemies.
  GameObjects gameObjects;
  Timer refillTimer;
  GameStyle *style, *styleNext;

  int enemyNameCount; // Number for next enemy name.  E.g. "enemy-8"

  Boolean quitGame; //set when application must close

  int level; // Current level.
  int levelHighest; // Over all games.
  Boolean noDemo; // Don't do anything during the demo.
  Boolean startImmediately; // skip the demo completely, start automatically
  Boolean noLevelTitle; // From command line args.
  Boolean noUi; // implies observer is True
  Boolean observer; // No human player for this Role, only for Server.
  Boolean dedicatedServer; // implies observer is True
  UIkeyset uiKeyset;
  Boolean noNewLevel;
  Boolean fullScreen;

  int difficulty; // Should never be DIFF_NONE.
  int difficultyNext; // DIFF_NONE means not specified yet.

  RoleP role; // client/server/standalone

  // For choosing names for humans.
  int intelNamesIndices[INTEL_NAMES_NUM];

  // Only used in the awardBonus state.
  // Stores the string to display on the Ui when the 
  // next level starts.
  char *levelTitleStored;

  // For logging server output to a file.
  // UNIX-only for now.
  Daemon* daemon;
};
#endif


