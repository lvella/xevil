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

// "game.cpp"

#if X11
	#ifndef NO_PRAGMAS
	#pragma implementation "game.h"
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
#include <cstdlib>

#include "utils.h"
#include "coord.h"
#include "world.h"
#include "id.h"
#include "intel.h"
#include "physical.h"
#include "actual.h"
#include "locator.h"
#include "ui.h"
#include "game_style.h"
#include "game.h"
#include "sound.h"
#include "xetp.h"
#include "l_agreement.h"


#if WIN32
#include "uiplayer.h"
#include "uiserver.h"
#include "xviewport.h"
#include "resource.h"
#endif

#if X11
extern "C" {
#include <X11/Xresource.h>
}
#endif

using namespace std;

// Defines
#define VERSION "2.1 Beta 1"
// 2.0 Beta 4B is the RedHat Linux version.
// 2.02 is GPL

#ifdef XEVIL_KEYSET
#define KEYSET_DEFAULT XEVIL_KEYSET
#else
// Used to be UIdecmips, but now Linux is probably the most common.
// This corresponds to the standard IBM keyboard.
#define KEYSET_DEFAULT UIlinux   
#endif

#ifndef XEVIL_CLASS
#define XEVIL_CLASS "XEvil"
#endif

#define WORLD_ACROSS_MAX_DEFAULT 5 // 4 increase because made rooms smaller
#define WORLD_DOWN_MAX_DEFAULT 2

#define QUANTA_DEFAULT 40 // In milliseconds
#define QUANTA_MAX 500
#define EXTRA_TURNS 50
#define DEMO_EXTRA_TURNS EXTRA_TURNS
#define OTHER_INPUT_RESET 160
#define HUMANS_NUM_DEFAULT 1
#define ENEMIES_NUM_DEFAULT 0 // 3
#define REFILL_TIME 260
#define EXTRA_ITEMS_MULTIPLIER 8
#define LEVEL_TITLE_TIME 40
#define GET_BEARINGS_TIME UI_ARENA_MESSAGE_TIME
#define AWARD_BONUS_TIME UI_ARENA_MESSAGE_TIME
#define DEMO_CUTOFF_TIME 400
#define DRAWING_ALGORITHM_DEFAULT Locator::DRAW_MERGE_AREAS
#define OBJECT_WORLD_PERCENT_MIN .00019
#define NEW_LEVEL_TIME UI_ARENA_MESSAGE_TIME
#define PREPARE_CONNECT_TIME 2
#define AWARD_BONUS_TRIES 50
#define BONUS_CHICKENS_NUM 3
#define GAME_OVER_ARENA_MESSAGE_TIME 10000
#define LOG_FNAME_DEFAULT "./xevil.log"

// Chance there will be one this level
#define ALTAR_OF_SIN_CHANCE     8 


// Play sound
#define SOUNDONOFF_DEFAULT       True 


char *Game::wittySayings[Game::WITTY_SAYINGS_NUM] = {
    "If it moves it's a threat.  If it doesn't move it's a potential threat.",
    "Happy, happy.  Joy, joy.",
    "For the mother country!!!",
    "Hi ho.  Hi ho.  It's off to kill we go.",
    "Well, do you feel lucky, Punk?",
    "Wake up, time to die.",
    "Let's rock.",
    "I love the smell of napalm in the morning.",
    "How's it feel to be hunted?",
    "Go ahead, make my day.",
    "I'll be back.",
    "Won't you be my neighbor?",
    "3E.  We bring good things to death.",
    "We come in peace.  (Shoot to kill.  Shoot to kill.)",
    "Hey.  You can't do that.",
    "I'm here to chew bubble-gum and kick ass.  And I'm all out of bubble-gum.",
    "You're a useless asshole, Pogo-Man.",
    "Settle down, Beavis.",
    "Ho, ho, ho.  Now I have a machine gun.",
    "Aren't you dead yet?",
    "I don't give a fuck what you know.  I'm going to torture you anyway.",
    "I'm hungry.  Let's get a taco.",
    "I'm sorry, Dave.  I can't do that.",
    "Violence is the only true form of communication.",
    "Let the flames purify your soul!!!",
    "Fuck me gently with a chainsaw.",
    "He doesn't look like a quivering ball of white-hot canine fury.",
    "Hackito ergo sum.",
    "I'm singing in the rain...",
    "Washin' the dog!!  Washin' the dog!!",
    "ooooh.  That's gotta hurt.",
    "Kids, don't try this at home.  We're trained professionals.",
    "Oh, you want some of this?!  Fuck you!",
    "Well, he shoulda' armed himself!",
    "Fuck art.  Let's kill.",
    "If you can't eat it or fuck it, kill it.",
    "Skrotus lives in my fridge.",
    "I love you.  You love me.  We're a happy family.",
    "You always were an asshole, Gorman.",
    "Change it dude.  This sucks.",
    "The game of death, doom, and (more or less) ultimate destruction.",
    "I think I'll just Dieeeeeee.",
    "You know he'll feel that one in the morning.",
    "I've had this ice-cream bar since I was a child.",
    "Shut up or I will kill you.  I will physically kill you.",
    "Better stop before someone gets hurt.",
    "What??",
    "Whoa!!",
    "Jesus will love you if you give me money.",
    "Evil.  It's not just for breakfast anymore.",
    "XEvil.  The carpal tunnel game.",
    "Violence.  It's not just the best policy.  It's the only policy.",
    "Got ... to get ... to ... my arm.",
    "Who's the lucky one?",
    "Don't blink.  It would really suck if you blinked.",
    "Here kitty, kitty, kitty.",
    "Stay on target...",
    "Do not taunt Happy-Fun-Ball.",
    "Bring out the gimp.                            Gimp's sleepin.\nWell, I guess you'll just have to go wake him up now, won't you.",
    "That's easy.",
    "It's just a rabbit.",
    "Ignorance is the number one cause of happiness.",
    "What the heck's going on?  I thought I was a normal guy.",
    "Oh, no.  It's K-k-k-ken c-c-c-coming to k-k-k-kill me.",
    "You bloated sack of protoplasm.",
    "The weak exist only to amuse the strong.",
    "We're not hitch-hiking anymore.  We're riding.",
    "No, sir.  I didn't like it.",
    "Are we there yet?",
    "Kurt Cobain spoke for my generation.",
    "We're on a mission from God.",
    "Oh, I'm a lumberjack and I'm ok...",
    "I'm sorry.  You must have mistaken me for the clown that gives a damn.",
    "Another visitor.  Stay awhile.  STAY FOREVER!!!!",
    "I laughed.  I cried.  It was better than Cats.\nI'd see it again and again.",
    "SPOON!!!",
    "Only life can kill you.",
    "That which does not kill us, hurts.",
    "Hey, baby.  Wanna wrestle.",
    "Fuck you and the horse you rode in on.",
    "Spin, spin, spin, spin, spin, spin, spin...",
    "Dope will get you through times of no money\nbetter than money will get you through times of no dope.",
    "What he said.",
    "Like, take off, eh.",
    "Lord God, Leroy, that was the toughest meanest hippie I ever saw.",
    "Happy thoughts.  Happy thoughts. Happy thoughts.",
    "M*cr*s*ft:  Who need quality when you have marketing.",
    "The proof is left as an exercise.",
    "Ummm...  We have a problem.",
    "Come see the violence inherent in the system.\nHelp! Help! I'm being repressed.",
    "Hey, man, you a sniper?                                           Shut up.\nYou sure look like a sniper.                                      I said, shut up.",
    "Oh, I'm sorry.  Did that hurt?",
    "What me worry?",
    "Nobody expects the Spanish Inquisition!!!",
    "We are, we are, we are, we are, we are the engineers.\nWe can, we can, we can, we can demolish forty beers.",
    "When in doubt, shoot the lawyer.",
    "All that hate's gonna burn you up, boy.\nKeeps me warm.",
    "He lives in that piece of paper?\nNo, you idiot.  He lives in a regular house.",
    "I'll give you something to cry about.",
    "It's not a bug, it's a feature.",
    "Don't hate me because I'm beautiful.",
    "What a great game of cards this is.",
    "You're ugly and your mother dresses you funny.",
    "I can eat fifty eggs.",
    "What we have here is failure to communicate.",
    "Jesus loves me more than you.",
    "Are you threatening me!!!??",
    "I need TP for my bunghole.",
    "You're immune to both romance and mirth.  You must be a... a...\nThat's right.  I'm an engineer.",
    "Out Out!!  You demons of stupidity!!",
    "To infinity, and beyond!!!",
    "I love Bomb. It's nice.",
    "Marines!  We are leaving.",
    "Toad Sexing kicks ass.",
    "Let me try that again.",
    "I is a college graduate.",
    "FRIDAY!!! ... IN STEREO!!! ...",
    "I'll get you, my pretty.  And your little dog, too!",
    "I hold the keys to the kingdom!\nI'm the God!  I'm the God!",
    "We're going to need a bigger boat.",
    "Time for some righteous killing in the name of Our Lord and Savior Jesus Christ.",
    "The Rivers shall Run Red with the blood of the infidels!",
    "Oh, my God!  They killed Kenny!",
    "I'm not fat, I'm big boned",
    "Kick the baby.",
    "fnord",
    "Save the planet.  Kill yourself.",
    "I just do what the voices in my head tell me to.",
    "Here's to you, T-Bird.",
    "Whole new paradigm.  Same old shit.",
    "Teen suicide.  Just do it.",
    "It's the history eraser button, you fool!!",
    "XEvil is mother.  XEvil is father.",
    "You die.  She dies.  Everybody dies.",
    "Aaaaa!!  My soul is trapped in XEvil.  Send help!!",
    "Bugs Bugs Bugs Bugs Bugs Bugs Bugs Bugs Bugs Bugs\nBugs Bugs Bugs Bugs Bugs Bugs Bugs Bugs Bugs Bugs",
    "Fuck 'em if they can't take a joke.",
    "Nah, nah, nah, naaah.  You lose a plot of land.",
    "Crom!!!",
    "The enemy's gate is down.",
    "Just because you're paranoid, doesn't mean they're not out to get you.",
    "And it won't even run down your battery.",
    "How can you have any pudding if you don't eat your meat!!?",
    "You're my bitch now.",
    "Prepare to Qualify.",
    "I got a bad feeling about this drop.",
    "Cowboys never quit!",
    "XEvil.  The peak of abnormality.",
};



char *Game::intelNames[Game::INTEL_NAMES_NUM] = {
  "Dr. Pain",
  "Steve",
  "hardts",
  "James E. Tetazoo",
  "Arnold",
  "Victor the Cleaner",
  "Hudson",
  "Hicks",
  "Twinkie Boy",
  "Bob",
  "Ben Bitdiddle",
  "Beavis",
  "Butt-Head",
  "Fred",
  "Mulch-Man",
  "Mike",
  "Tim",
  "Peter",
  "Alan",
  "Neal",
  "Tony",
  "Asshole",
  "Crazy Hairy",
  "Spaceman Spiff",
  "Kitty-Wench",
  "Juker",
  "Shrieks",
  "Albert Lin",
  "Jim Gouldstone",
  "Steph",
  "Paining",
  "Dim",
  "The Man",  
  "Twinkie Girl",
  "Mr. Buzzcut",  
  "Sensei Steve",
  "Ren",
  "Stimpy",
  "Mr. Horse",
  "Dilbert",
  "Dogbert",
  "Gromit",
  "The Tick",
  "Fat Freddie",
  "Phineas",
  "Freewheelin Franklin",
  "Doctor Who",
  "Celery-Head-Man",
  "Roo",
  "Shwang",
  "Leo",
  "Smoothie",
  "Hawaii Chick",
  "Cowboy Steve",
  "Outlaw Jim",
  "Prick",
  "Smack",
  "Hello, Kitty",
  "Wedge",
  "Redshirt",
  "Mistress Leah",
  "e^x Man",
  "John Rusnak",
  "Mr. Bonk",
  "Pixie of Pass/Fail",
  "Josh the 10-yr Old",
  "Little Wooden Boy",
  "Brain",
  "Kenny",
  "Kyle",
  "Cartman",
  "Stan",
  "Mr. Hat",
  "Cid",
};



DifficultyLevel Game::difficultyLevels[DIFFICULTY_LEVELS_NUM] = {
  /* reflexes, enemiesInitial, enemiesIncr, enemiesMax, rankMultiplier, name */
  {  40,       2,              1,           10,         0.5f,           "trivial" },
  {  6,        3,              2,           50,         1.0f,           "normal" },
  {  4,        5,              5,           50,         1.5f,           "hard" },
  {  0,        20,             10,          1000000,    2.0f,           "bend-over" },
};



RankingSet Game::rankingSets[RANKING_SETS_NUM] = {
  /* minKills followed by the set of possible ranks. */
  { 0, {
    "Hell's Peg Boy",
    "Satan's Vibrating Sex Toy",
    "Hell's Semen Taster",
    "Satan's Enema",
    "Member of Hell's Children's Choir",
    NULL,
  }},
  { 10, {
    "Hell's Proctologist",
    "Hell's Fluffer",
    "Satan's Earwax Remover",
    "Prostitute in Hell's Brothel",
    "Fecal Sorter",
    NULL,
  }},
  { 25, {
    "Hell's Speedbump",
    "Hell's Janitor",
    "Cook in Hell's Cafeteria",
    "Hell's Shoe Salesman",
    "Hell's Chili Taster",
    "Satan's Shoehorn",
    NULL,
  }},
  { 40, {
    "IS Tech Support for Hell",
    "QA of Vomit Production",
    "Teacher at Beelzebub Jr. High",
    "Hell's Copier Repairman",
    "Hell's Plumber",
    NULL,
  }},
  { 60, {
    "Guidance Counselor at Mephistopheles High",
    "Hell's Doorman",
    "Hell's Aerobic Instructor",
    "Hell's Sysadmin",
    "Satan's Used Car Salesman",
    "Hell's Dentist",
    "Hell's Social Worker",
    NULL,
  }},
  // Middle management level.
  { 90, {
    "Manager, Soul Collection Department",
    "Manager, Hell Climate Control",
    "Manager, Medieval Torture Devices Maintenance",
    "Manager, Department of Temptation",
    "Manager, Department of Graft and Corruption",
    NULL,
  }},
  { 90, {
    "Hell's Chief Build Engineer",
    "Drill Sergeant for Hell's New Recruits",
    "Pimp in Hell's Brothel",
    "Postal Worker",
    NULL,
  }},
  { 130, {
    "Torture Methods Research Scientist",
    "VP of Hell's PR Department",
    "Strategic Planner, Plague and Pestilence",
    NULL,
  }},
  { 170, {
    "VP of Hell Marketing",
    "VP of Hell Legal",
    "VP of Soul Purchases",
    "VP of Worldly Corruption",
    NULL,
  }},
  { 250, {
    "High Priest of Torture",
    "Lead Software Engineer of Hell",
    NULL,
  }},
  { 666, {
    "Replace Bill as Satan's Right Hand Man",
    NULL,
  }},
  { 666666666, {
    "You are the new Satan",
    NULL,
  }},
};



// Functions     
void GameStats::report()
{
  if (numTurns % Game::REPORT_TIME == 0)
    cout << "Turn " << numTurns << ":  Average time of turn:  " << 
      aveTime << endl;
}



void GameStats::clock()
{
    time_t oldPrevTime = prevTime;
    time(&prevTime);    

    if (numTurns)
        aveTime = (numTurns * aveTime + (prevTime - oldPrevTime)) / 
        (numTurns + 1); 
    
    numTurns++;
}



GameObjects::GameObjects(WorldP w,LocatorP l)
: world(w),locator(l)
{
  resetCalled = False;
  for (int n = 0; n < A_CLASSES_NUM; n++) {
    maximums[n] = 0;
    actuals[n] = 0;
    ids[n] = NULL;
  }
}



GameObjects::~GameObjects() {
  for (int n = 0; n < A_CLASSES_NUM; n++) {
    if (ids[n])
      delete [] ids[n];
  }
}


void GameObjects::game_reset(const char *one_item,
			     Boolean no_items,Boolean one_each) {
  oneItem = one_item;
  noItems = no_items;
  oneEach = one_each;
  resetCalled = True;
  
//  level_reset(worldDim);
}



void GameObjects::level_reset(const Dim &worldDim,GameStyleP style) {
  // WARNING: GameObjects forgets about old items.  Not a memory leak, but 
  // this may end up with more and more items coming into existence.

  // We do want to kill off extra AltarOfSins, though.
  int n;
  for (n = 0; n < actuals[A_AltarOfSin]; n++) {
    PhysicalP p = locator->lookup(ids[A_AltarOfSin][n]);
    if (p) {
      Game::off_clock_kill(locator,p);
    }
  }


  // Initialize arrays.
  for (n = 0; n < A_CLASSES_NUM; n++) {
    maximums[n] = 0;
    actuals[n] = 0;
    levelAlready[n] = 0;
    levelMaxTimes[n] = 0; // infinity
    if (ids[n]) {
      delete [] ids[n];
      ids[n] = NULL;
    }
  }
  
  
  // Get arrays of potential weapons and potential otherItems.
  int weaponsNum;
  const PhysicalContext *weapons[A_CLASSES_NUM];
  weaponsNum = locator->filter_contexts(weapons,NULL,
                                        potential_weapon_filter,NULL);
  int oItemsNum;
  const PhysicalContext *oItems[A_CLASSES_NUM];
  oItemsNum = locator->filter_contexts(oItems,NULL,
                                       potential_other_item_filter,NULL);
  

  // Let Style filter out to specific weapons and items, if desired.
  // Must pass in arrays of length A_CLASSES_NUM.
  style->filter_weapons_and_other_items(locator,weaponsNum,weapons,oItemsNum,oItems);

  // Normal case.
  if (!strlen(oneItem) && !noItems && !oneEach) {
    // Area of the world.
    int areaFactor = worldDim.rowMax * worldDim.colMax;

    // Set maximums[n] for all potential weapons, n.
    for (n = 0; n < weaponsNum; n++) {
      // Check not already set.
      assert(maximums[weapons[n]->classId] == 0);

      // Don't allow objectWorldPercent values that are too small.
      float objWPercent = (float)fmax(weapons[n]->objectWorldPercent,
			      OBJECT_WORLD_PERCENT_MIN);

      maximums[weapons[n]->classId] = (int)ceil(areaFactor * objWPercent);
      // A limit may be placed on how many times the object comes back 
      // in a level.
      levelMaxTimes[weapons[n]->classId] = weapons[n]->levelMaxTimes;
    }

    // Set maximums[n] for all potential other items, n.
    for (n = 0; n < oItemsNum; n++) {
      // Check not already set.
      assert(maximums[oItems[n]->classId] == 0);
      float objWPercent = (float)fmax(oItems[n]->objectWorldPercent,
			      OBJECT_WORLD_PERCENT_MIN);

      maximums[oItems[n]->classId] = (int)ceil(areaFactor * objWPercent);
      levelMaxTimes[oItems[n]->classId] = oItems[n]->levelMaxTimes;
    }

    // Should we have an Altar this level.
    maximums[A_AltarOfSin] = (Utils::choose(ALTAR_OF_SIN_CHANCE) == 0);

    // Really should get it from PhysicalContext.
    levelMaxTimes[A_AltarOfSin] = 1; 
  }

  // If only one item in the Game.
  else if (strlen(oneItem)) {
    ClassId classId = Game::parse_class_name(locator,oneItem);
    if (classId != A_None) {
      maximums[classId] = 1;
    }
  }

  // One of each object.
  else if (oneEach) {
    for (n = 0; n < weaponsNum; n++) {
    	maximums[weapons[n]->classId] = 1;
    }

    for (n = 0; n < oItemsNum; n++) {
      maximums[oItems[n]->classId] = 1;
    }

    maximums[A_AltarOfSin] = 1;
  }

  // Compute actuals from maximums.
  compute_actuals(weapons,weaponsNum,oItems,oItemsNum);

  // Somewhat of a hack.  Always want maximum number when using oneItem or
  // oneEach.
  if (strlen(oneItem) || oneEach) {
    for (n = 0; n < A_CLASSES_NUM; n++) {
      actuals[n] = maximums[n];
    }
  }

  // Create the arrays of ids for each object type.
  // Only need space for actuals[n].
  for (n = 0; n < A_CLASSES_NUM; n++)
    if (actuals[n]) {
      Id invalid;
      ids[n] = new Id[actuals[n]];
      assert(ids[n]);
      for (int m = 0; m < actuals[n]; m++)
      	ids[n][m] = invalid;
      // Explicitly initialize ids because different C++ compilers follow
      // different rules about constructing elements of an array.
    }
}



void GameObjects::refill() {
  assert(resetCalled);

  int n;
  for (n = 0; n < A_CLASSES_NUM; n++) {
    if (actuals[n]) {
      // We know that the actuals is indexed by classId.
      const PhysicalContext *cx = locator->get_context((ClassId)n);
      assert(cx);
      refill_helper(cx,n);
    }
  }
}



Boolean GameObjects::is_available(const Id &id) {
  PhysicalP p = locator->lookup(id);
  if (!p) {
    return False;
  }

  // Only count items that aren't held by other players.
  if (p->is_item()) {
    return !((ItemP)p)->is_held();
  }

  return True;
}



// Ahh, no more #define REFILL_HELPER().  It's a real function now.
void GameObjects::refill_helper(const PhysicalContext *cx,int which) {
  int existsNum = 0;
  int m;
  for (m = 0; m < actuals[which]; m++) {
    if (is_available(ids[which][m])) {
      existsNum++;
    }
  }
  int newOnes = actuals[which] - existsNum;

  // Don't create object more that levelMaxTimes per level.  
  if (levelMaxTimes[which]) {  // 0 means infinity
    newOnes = Utils::minimum(newOnes,
			     levelMaxTimes[which] - levelAlready[which]);
  }
  
  // Now create the new ones.
  for (m = 0; newOnes > 0; m++) {
    assert(m < actuals[which]);
    if (!is_available(ids[which][m])) { 
      Pos pos = world->empty_rect(cx->sizeMax);
      assert(cx->create);
      PhysicalP p = cx->create(cx->arg,world,locator,pos);
      // Increment number of times created this level.
      levelAlready[which]++; 
      locator->add(p);
      ids[which][m] = p->get_id();
      newOnes--;
    }
  }
}



void GameObjects::compute_actuals(const PhysicalContext *weapons[],
                                  int weaponsNum,
                                  const PhysicalContext *oItems[],
                                  int oItemsNum) {
  // Initialize all actuals to 0.
  int n;       
  for (n = 0; n < A_CLASSES_NUM; n++)
    actuals[n] = 0;

  // Number of weapon and otherItems classes to include in this level.
  int weaponsLevel = 2 + Utils::choose(5);  // 2 <= weaponsLevel <= 6
  int oItemsLevel = 1 + Utils::choose(5);  // 1 <= oItemsLevel <= 5
  if (weaponsLevel > weaponsNum) {
    weaponsLevel = weaponsNum;
  }
  if (oItemsLevel > oItemsNum) {
    oItemsLevel = oItemsNum;
  }
  // assert(weaponsLevel <= weaponsNum && oItemsLevel <= oItemsNum);  Not since role can filter them.

  // Rank the weapon and other classes.
  int *weaponsRank = new int[weaponsNum];
  Utils::random_list(weaponsRank,weaponsNum);
  int *oItemsRank = new int[oItemsNum];
  Utils::random_list(oItemsRank,oItemsNum);

  // Set actuals to be non-zero for weaponsLevel number of classes.  Choose
  // those ranked highest according to weaponsRank.
  for (n = 0; n < weaponsLevel; n++) {
    int which = weapons[weaponsRank[n]]->classId;
    actuals[which] = 
	    maximums[which] ? (Utils::choose(maximums[which]) + 1) : 0;
    //      cout << "actuals[" << weapons[weaponsRank[n]]->className << "] = " 
    //	   << actuals[which] << " of " << maximums[which] << endl;
  }

  // Set actuals to be non-zero for oItemsLevel number of classes.  Choose
  // those ranked highest according to oItemsRank.
  for (n = 0; n < oItemsLevel; n++) {
    int which = oItems[oItemsRank[n]]->classId;
    actuals[which] = 
	    maximums[which] ? (Utils::choose(maximums[which]) + 1) : 0;
    //      cout << "actuals[" << oItems[oItemsRank[n]]->className << "] = " 
    //	   << actuals[which] << " of " << maximums[which] << endl;
  }

  //  cout << endl;

  delete [] weaponsRank;
  delete [] oItemsRank;

  // Altar of sin is special.
  if (maximums[A_AltarOfSin]) {
    actuals[A_AltarOfSin] = maximums[A_AltarOfSin];
  }
}



Boolean GameObjects::potential_weapon_filter(const PhysicalContext* cx,void*) {
  return cx->potentialWeapon;
}



Boolean GameObjects::potential_other_item_filter(const PhysicalContext* cx,
                                                 void*) {
  return cx->potentialOtherItem;
}



Game::Game(int *arg_c,char **arg_v)
  : world(), 
    soundManager(SOUNDONOFF_DEFAULT,&locator),
    locator(&world,DRAWING_ALGORITHM_DEFAULT,&soundManager),
    gameObjects(&world,&locator) { 
  // Move earlier, so it is before error messages.
  intro();


  // Call even for a non-network game.  Good to check our assumptions.	
  XETP::check_sizes();

  // Just for tracing in the debugger.
  char* version = Utils::get_OS_info();
  delete version;

  noUi = False;
  noNewLevel = False;
  observer = False;
  dedicatedServer = False;
  uiKeyset = UIunspecifiedKeyset; // an invalid value.
  fullScreen = False;

  // Now Ui is created after parse_args for X and Windows.
  ui = NULL;

  world.set_locator(&locator);

  // Always normal difficulty for demo.
  difficulty = DIFF_NORMAL;
#if WIN32
  // Since Windows has a good UI for setting difficulty, don't force
  // user to set it.
  difficultyNext = DIFF_NORMAL;
#endif
#if X11
  // DIFF_NONE will always ask for level of difficulty.
  difficultyNext = DIFF_NONE;
#endif

  Enemy::set_reflexes_time(difficultyLevels[difficulty].reflexes);

  quitGame = False;
  currentSoundName = 0;
#if WIN32
  musictype=UIsettings::MIDI;
#endif

  startImmediately = False;

  argc = arg_c;
  argv = arg_v;

  refillTimer.set(REFILL_TIME);
  
  noLevelTitle = False;

  pause = False;

  // Now style isn't set until after/during parse_args.
  style = NULL;

  humansNumNext = HUMANS_NUM_DEFAULT;

  // Before parse_args.
  for (int n = 0; n < Locator::HUMANS_MAX; n++) {
    humanNames[n] = Utils::strdup("");
  }
  
  // Must be before parse_args.
  enemiesNumNext = ENEMIES_NUM_DEFAULT;

  cooperative = cooperativeNext = False;

  enemiesRefill = enemiesRefillNext = False;

  quanta = -1;

  humansPlayingPrev = enemiesPlayingPrev = 0;
  enemyNameCount = 0;

  worldRooms.downMax = WORLD_DOWN_MAX_DEFAULT;
  worldRooms.acrossMax = WORLD_ACROSS_MAX_DEFAULT;

  intelHarmless = False;
  showStats = False;

  noItems = False;
  oneEach = False;
  oneItem = Utils::strdup("");

  humanClass = A_None;
  humanExtraAbility = AB_MAX; // no extra abilities

  noDemo = False;

  infiniteLives = False;

  worldFile = NULL;
  
  role = NULL;

  daemon = NULL;

//------------------------------ PARSE_ARGS ------------------------------
  parse_args(argc,argv);
//--------------------------- AFTER PARSE_ARGS ---------------------------

  // Put after parse_args() so DebugInfo()::turn_on() can be called.
  DebugInfo::initialize();


  // Wasn't assigned in parse_args, so use appropriate default.
  if (role == NULL) {
    if (dedicatedServer) {
      // -dedicated, but not -server
      role = new Server(False,NULL,&locator);
    }
    else {
      role = new StandAlone();
    }
  }
  assert(role);

  // Before check that role is ok().
  if (noUi && role->get_type() != R_SERVER) {
    cerr << "Can only set -no_ui if running as a server." << endl;
    noUi = False;
  }

  // If we are a server and we have a log file specified, redirect all
  // output to the log file.  Technically, we could have logging turned on
  // for a non-server, but why bother.
  if (role->get_type() == R_SERVER) {
    if (daemon == NULL) {
      daemon = new Daemon(LOG_FNAME_DEFAULT);
    }
    if (!DebugInfo::on()) {
      daemon->go();
    }
  }

  // Create the UI, this is now the only place the Ui is created.
#if X11
  if (!noUi) {
    ui = new Ui(arg_c,arg_v,&world,&locator,
                display_names(arg_c,arg_v),font_name(arg_c,arg_v),
                &soundManager,difficultyLevels,
                role->get_type());
    assert(ui);
  }
#endif
#if WIN32
  // noUi is meaningless on Windows.
  if (dedicatedServer) {
    ui = new UiServer(&locator);
    // Start immediately if a dedicated server.
    startImmediately = True;
  }
  else {
    ui = new UiPlayer(arg_c,arg_v,&world,&locator,
              display_names(arg_c,arg_v),font_name(arg_c,arg_v),
              &soundManager,difficultyLevels,
              fullScreen ? Xvars::FULL_SCREEN_MODE : 
                           Xvars::WINDOW_SCREEN_MODE,
              VERSION);
  }              
  assert(ui);
#endif


  if (ui) {
    // Moved out of parse_args, because Ui might not exist.
    if (uiKeyset != UIunspecifiedKeyset) {
      for (int dpyNum = 0; dpyNum < ui->get_dpy_max(); dpyNum++) {
        ui->set_keyset(dpyNum,uiKeyset);
      }
    }

    ui->set_world_rooms(worldRooms);
    ui->set_enemies_num(enemiesNumNext);
    ui->set_enemies_refill(enemiesRefillNext);
    ui->set_cooperative(cooperativeNext);
    ui->set_quanta(quanta);    
    ui->set_difficulty(difficultyNext);

#if WIN32
    ui->set_role_type(role->get_type());
    ui->set_musictype(musictype);
    ui->set_sound_onoff(soundManager.isSoundOn());
    ui->set_track_volume(soundManager.getTrackVolume());
    ui->set_sound_volume(soundManager.getEffectsVolume());
#endif
  }

  // If we aren't displaying a UI, start the game immediately.
  if (!ui) {
    startImmediately = True;
  }

  // Depends on the role
  humansNumNext = limit_humans_num(humansNumNext);

  // Not good that we are calling methods on role before role->connect().
  // This specific case is ok.
  role->set_humans_num(humansNumNext);
  if (ui) {
    ui->set_humans_num(role->get_humans_num());
  }

  // If quanta wasn't set in parse_args.
  if (quanta == -1) {
    quanta = QUANTA_DEFAULT;
  }
  if (ui) {
    ui->set_quanta(quanta);
  }
   
  // If style hasn't already been set.
  if (!style) {
    GameStyleType gsType = role->get_default_game_style();
    style = GameStyle::by_type(gsType);
    assert(style);
  }

  // Can't set this earlier, because isn't guaranteed to exist until now.
  if (ui) {
    ui->set_style(style->get_type());
  }


  // After parse_args
  state = noDemo ? demoOff : demoOn;

  // Delay connecting to server until after Ui has initialized graphics on
  // Windows, ui->add_viewport().
  if (role->get_type() == R_CLIENT) {
    state = prepareConnect;
    timer.set(PREPARE_CONNECT_TIME); 
  } 
  // Tell Server to start running.  We can't just do everything in the Server
  // constructor because (UNIX only) the constructor of Ui may block for
  // arbitrary amount of time waiting for the License Agreement Dialog.
  if (role->get_type() == R_SERVER) {
    ((ServerP)role)->run();
  }

  
  // By default, next game will be the same style as this one.
  styleNext = style->clone();

  // world.set_rooms_next(worldRooms);
  // Now in Game::reset.

  // Must be before first call to Ui::set_input.
  if (ui) {
    process_x_resources(argc,argv);			
    ui->set_input(0,UI_KEYS_RIGHT);
    ui->set_enemies_num(enemiesNumNext);
  }
  
  locator.set_messages_ignore(True); // Must be after intro().

  // World title map needs to know the dimensions of the viewport.
  // If no Ui, don't really care about the title/demo screen.
  if (ui) {
    // The IViewportInfo is valid after the ui is constructed. 
    world.title_map(Ui::get_viewport_info());
  }

  if (role->get_type() != R_CLIENT) {
    if (state == demoOn) {
      timer.set(DEMO_CUTOFF_TIME);
      demo_setup();
    }
  }

  // randomize the list of in the constructor just to be sure.
  Utils::random_list(intelNamesIndices,INTEL_NAMES_NUM);

  levelTitleStored = NULL;

#if WIN32
  start_soundtrack();
#endif
}



Game::~Game() {
  if (oneItem) {
    delete oneItem;
  }

  delete ui;
  delete style;
  delete styleNext;
  delete role;
  delete daemon;

#if WIN32
  soundManager.stopCD();
  soundManager.stopMIDI();
#endif

  delete levelTitleStored;

  for (int n = 0; n < Locator::HUMANS_MAX; n++) {
    Utils::freeif(humanNames[n]);
  }
}



const char* Game::get_version_string() {
  return VERSION;
}



static RestartEnd restartEnd;



void Game::pre_clock() {
  restartEnd = RE_NONE;
  ui_settings_check(restartEnd);
  
  // Must be before ui->clock().
  // Start new game because of user input.
  if (!pause && 
      state == gameOver && 
      timer.ready() && 
      ui && ui->other_input()) {
    restartEnd = RE_RESTART;
  }

  // Start new demo.
  if (!pause &&
      state == demoOn && 
      ui && ui->other_input()) {
    timer.set(0);
  }

  // Must be after Client::yield() and before clearRegisteredSounds()
  if (role->get_type() == R_CLIENT) {
    // check for soundplay
    play_sounds();
  }

  soundManager.clearRegisteredSounds();
  

  // Put this right before Ui clears them out.
  role->handle_messages(this,&locator);
  if (ui) {
    // Drawing happens here.
    ui->pre_clock();
  }
  else {
    // Still want to nuke the messages each turn.
    locator.clear_all_messages();
  }
}



void Game::post_clock() {
  // Put before everything else, to interpret keys at last possible moment.
  if (ui) {
    ui->post_clock();
  }

  // Keep polling role to make sure it is ok.  e.g. fail for Client 
  // if Server goes down.
  if (state != roleFailed && state != prepareConnect && !role->ok()) {
    state = roleFailed;
    // timer.set(ROLE_FAILED_TIME);
  }

  if (role->get_type() == R_CLIENT) {
    // Finish connecting to server.
    if (state == prepareConnect) {
      if (timer.ready()) {
        // Clear out anything that might be hanging around before
        // connecting to server.
        // We don't use Game::reset() when role is Client.
        if (ui) {
          ui->reset();
        }
        locator.reset();        
        world.reset();

        assert(role && role->get_type() == R_CLIENT);
        ((ClientP)role)->connect_server();
        // Doesn't really matter.  Don't use gameOver because then 
        // any key press causes the game to reset.
        state = gameOn; 
      }
    }

    // All the Locator pseudo-clocking stuff happens in Client::yield.

    role->clock(this,&world,&locator);
    if (restartEnd == RE_RESTART) {
      // Will tell server to start new game for this client.
      ((ClientP)role)->request_new_human();
    }
    // This really shouldn't be in two places.
    timer.clock();
    return;
  }
  // Sure would be nice to move the above code somehow into Role.  Game really
  // shouldn't have to do role->get_type().  Bad OO programming.


  if (!pause) {
    /* Put (reset) after ui->clock so doesn't draw screen twice on reset.  
 	  Intel needs to be clocked to get id so that ui can follow the id of 
 	  the intel. */

    if (restartEnd == RE_RESTART) {
      new_game();
	    // If it was set.
	    if (ui) {
        ui->unset_prompt_difficulty();
      }
    }
    else if (restartEnd == RE_END) {
      end_game(False);
      // I bet this is unnecessary.
      if (ui) {
        ui->unset_prompt_difficulty();
      }
    }
      
    // The big-ass state machine.  Maybe we should have a IGameState inteface 
    // with separate implementations for each state.
    switch (state) {
    case demoOff:
      if (startImmediately) {
        // Start a new game now.
        new_game();
      }
    break;

    case demoOn: 
    case demoExtra: {
      if (startImmediately) {
        // Start a new game now.
        new_game();
      }
      else if (timer.ready()) {
        // Start new demo.
        state = demoOn;
        timer.set(DEMO_CUTOFF_TIME);
        locator.reset();

        if (ui) {
          ui->demo_reset();
        }

        world.demo_reset();
        demo_setup();
      }
      else if (state == demoOn && locator.enemies_alive() <= 1) {
        state = demoExtra;
        timer.set(DEMO_EXTRA_TURNS);
      }
      world.clock();
      locator.clock();
    }
    break;
      
    case newGame: {
      if (timer.ready()) {
        // Check if we have to ask the user for a difficulty setting.
        if (difficultyNext == DIFF_NONE && // not set yet
            styleNext->need_difficulty(enemiesNumNext)) {
            if (ui) {
              // need to get difficulty level from user.
              state = getDifficulty;
              ui->set_prompt_difficulty();
            }
            else {
              // No way to ask user for difficulty, assume DIFF_NORMAL
              difficultyNext = DIFF_NORMAL;
              reset();
            }
	      }
	      else {
          // leave difficulty alone.  Start game.
          reset();
        }
      }
    }
    break;
	
    case getDifficulty: {
      // Check if user has given a difficulty value.
      assert(ui != NULL);
      int uiDiff = ui->get_difficulty();
      if (uiDiff != DIFF_NONE) {
	      difficultyNext = uiDiff;
	      ui->unset_prompt_difficulty();
        ui->set_difficulty(difficultyNext);  // Make sure UI has new value.
        // reset();
        new_game();
      }
      // else stay in getDifficulty until User supplies a value.
    }
    break;

    case getBearings: {
      if (timer.ready()) {
        style->new_level_set_timer(timer);
        state = gameOn;
      }
    }
    break;
	  
    case awardBonus: {
      if (timer.ready()) {
        assert(levelTitleStored);
        // Pass ownership of levelTitleStored to get_bearings().
        get_bearings(levelTitleStored);
        levelTitleStored = NULL;
      }
    }
    break;
	  
    case gameOn: 
    case gameExtra:
    case levelExtra: {
      world.clock();
      locator.clock();  
      reincarnations_check();
	      
      int humansPlaying = locator.humans_playing();
      int enemiesPlaying = locator.enemies_alive();
	          
      if (humansPlaying != humansPlayingPrev) {
	      humansPlayingPrev = humansPlaying;
          if (ui) {
            ui->set_humans_playing(humansPlaying);
          }
          role->set_humans_playing(humansPlaying);
      }
	          
      if (enemiesPlaying != enemiesPlayingPrev) {
	      enemiesPlayingPrev = enemiesPlaying;
        if (ui) {
          ui->set_enemies_playing(enemiesPlaying);
        }
        role->set_enemies_playing(enemiesPlaying);
      }

      if (state == gameExtra) {
	      // Game is really over, stop clocking.
	      if (timer.ready()) {
	        end_game(True);
  	    }
      }
      else {
        assert(state == gameOn || state == levelExtra);
        game_over_check(humansPlaying,enemiesPlaying);
        
        // Check state because game_over_check() may have changed it.
        if (state == gameOn) {
          new_level_check(enemiesPlaying);
        }
        if (state == levelExtra) {
          // Extra time after level end has run out, start new level.
          if (timer.ready()) {
            if (!noNewLevel) {
              new_level();
            }
          }
        }
        if (state == gameOn || state == levelExtra) {
          refill_check();
        }
      }
	    
      if (showStats) {
	      stats.report();
      }
      stats.clock();
    }
    break;

    // handled at top of Game::clock().
    case gameOver:
	  break;

    case roleFailed:
      // Do nothing util user starts a new game.
      break;

    default:
      assert(0);
    } // switch


    // All-purpose timer, different uses for different states.
    timer.clock();
  }  // if (!pause)
  
  // check for soundplay
  play_sounds();

  role->clock(this,&world,&locator);
}



void Game::yield(CMN_TIME time,int quanta,ITurnStarterP turnStarter) {
  assert(role); 
  role->yield(time,quanta,this,turnStarter,&world,&locator);
}



ClassId Game::parse_class_name(LocatorP l,const char *name) {
  ClassId n;
  for (n = 0; n < A_CLASSES_NUM; n++) {
    const PhysicalContext *context = l->get_context(n);
    if (context && !strcmp(context->className,name))
      return n;
  }
  return A_None;
}



void Game::ui_settings_check(RestartEnd &restartEnd) {
  if (ui == NULL) {
    return;
  }

  if (ui->settings_changed()) {
    UIsettings settings;
    UImask mask = ui->get_settings(settings);
    
    if (mask & UInewGame) {
      restartEnd = RE_RESTART;
    }
    
    if (mask & UIquit) {
      print_stats();
      // main quits the game before next clock
      quit();
    }
#if WIN32
    if (mask & UItrackvol) {
	    soundManager.setTrackVolume(settings.trackvol);
    }
    if (mask & UIsoundvol) {
	    soundManager.setEffectsVolume(settings.soundvol);
    }
    if (mask & UIsound) {
      soundManager.turnOnoff(settings.sound);
      if (settings.sound) {
        start_soundtrack();
      }
      else {
        stop_soundtrack();
      }
    }
    if (mask & UImusictype) {
      stop_soundtrack();
      musictype = settings.musictype;
      start_soundtrack();
    }
#endif
    if (mask & UIrooms) {
      worldRooms = settings.worldRooms;
    }
    if (mask & UIhumansNum) {
      humansNumNext = limit_humans_num(settings.humansNum);
      ui->set_humans_num(humansNumNext);
    }
    
    if (mask & UIenemiesNum) {
      enemiesNumNext = Utils::minimum(settings.enemiesNum,
                                      Locator::ENEMIES_MAX);
      ui->set_enemies_num(enemiesNumNext);
    }
    
    if (mask & UIenemiesRefill) {
      enemiesRefillNext = settings.enemiesRefill;
      ui->set_enemies_refill(enemiesRefillNext);
    }
    
    if (mask & UIpause) {
	    pause = settings.pause;
	    ui->set_pause(pause);
    }
    
    if (mask & UIstyle) {
      if (settings.style != styleNext->get_type()) {
        // If set.
        ui->unset_prompt_difficulty();
        set_style_next(settings.style);
      }
    }
    
    if (mask & UIquanta) {
      quanta = settings.quanta;
      if (quanta < 0)
          quanta = 0;
      if (quanta > QUANTA_MAX)
          quanta = QUANTA_MAX;
      ui->set_quanta(quanta);
    }

#if WIN32
    if (mask & UIscreenmode) {
      change_screen_mode((Xvars::SCREENMODE)settings.screenmode);
    }
#endif

    if (mask & UIcooperative) {
      cooperativeNext = settings.cooperative;
      ui->set_cooperative(cooperativeNext);
    }
#if WIN32
    // Set from the "Difficulty..." menu item.
    // Won't apply until next game.
    if (mask & UIdifficulty) {
      difficultyNext = settings.difficulty;
      ui->set_difficulty(difficultyNext);
    }
#endif

    if (mask & UIconnectServer) {
      delete role;
      ostrstream portName;
      portName << settings.connectPort << ends;
      IViewportInfo* vInfo = Ui::get_viewport_info();
      ClientP client = new Client(settings.connectHostname,portName.str(),
                                  0,settings.humanName,vInfo,
                                  Connection::ADJUST_SKIP,&locator);
      delete portName.str();
      assert(client);
      role = client;
      ui->set_role_type(role->get_type());
      set_style_next(role->get_default_game_style());

      // Want to give a turn to let Client put a message up on the window.
      state = prepareConnect;
      timer.set(PREPARE_CONNECT_TIME); 
    }

    else if ((mask & UIdisconnectServer) ||
             (mask & UIstopServer)) {
      RoleType oldRoleType = role->get_type();
      delete role;
      role = new StandAlone();
      assert(role);
      restartEnd = RE_END;
      ui->set_role_type(role->get_type());
      set_style_next(role->get_default_game_style());
      if (oldRoleType == R_CLIENT) {
        locator.arena_message_enq(Utils::strdup("Disconnected from server. Reverting to stand-alone game."));
      }
      else if (oldRoleType == R_SERVER) {
        locator.arena_message_enq(Utils::strdup("Server shut down. Reverting to stand-alone game."));
      }
      // else ignore it
    }

    else if (mask & UIrunServer) {
      // A little funny the way we mutate humanNames[] here, but it's
      // just the same as if the user specified -name on the commmand line.
      if (strlen(settings.humanName)) {
        Utils::freeif(humanNames[0]);
        humanNames[0] = Utils::strdup(settings.humanName);
      }

      delete role;
      ostrstream portName;
      portName << settings.serverPort << ends;
      role = new Server(settings.localHuman,portName.str(),&locator);
      delete portName.str();
      assert(role);
      restartEnd = RE_RESTART;
      ui->set_role_type(role->get_type());
      set_style_next(role->get_default_game_style());
      ((ServerP)role)->run();
    }
    
    else if (mask & UIchatRequest) {
      // Just pass the chat request on to the role.
      role->send_chat_request(&locator,settings.chatReceiver,
                              settings.chatMessage);
    }
  
  } // if settings changed
}



void Game::set_style_next(GameStyleType styleType) {
  delete styleNext;  
  styleNext = GameStyle::by_type(styleType);
  if (ui) {
    ui->set_style(styleType);
  }

  ostrstream str;
  styleNext->describe(str);
  locator.message_enq(str.str());
}



void Game::reincarnations_check() {
  Incarnator iter(locator);
  HumanP human;
  while (human = iter()) {
    PhysicalP obj = human_physical();
    // human_physical now adds to locator.
    human->reincarnate();
    obj->set_intel(human);
      
    ostrstream msg;
    msg << human->get_name() << " is back from the dead." << ends;
    locator.message_enq(msg.str());
  }
}



void Game::game_over_check(int humansPlaying,int enemiesPlaying) {
  int hNum = (role->get_type() == R_SERVER) ? HU_UNSPECIFIED : humansPlaying;
  if (cooperative) {
    // Pretend there are at most one player, so will end game if 
    // just humans left.
    hNum = Utils::minimum(hNum,1);
  }  
  if (style->game_over_check(role->get_humans_num(),hNum,enemiesPlaying)) {
    state = gameExtra;
    timer.set(EXTRA_TURNS);
  }
}



void Game::new_level_check(int enemiesPlaying) {
  assert(state == gameOn);

  ostrstream str;
  Boolean lStrChanged = False;
  int val = style->new_level_check(enemiesPlaying,&world,&locator,
                                   level,lStrChanged,str,timer,
                                   (IPhysicalManagerP)this);

  // start new level immediately.
  if (val == 0) {
    timer.set(0);
    if (!noNewLevel) {
      new_level();
    }
    // Perhaps shouldn't return here, maybe should still do the 
    // lStrChanged logic.
    return;
  }
  // start new level in a little bit
  else if (val == 1) {
    state = levelExtra;
    timer.set(NEW_LEVEL_TIME);  
  } 
  // continue with current level
  else {
    assert(val == -1);
  }

  // Now use lStrChanged flag explicitly, because we can't effectively check
  // if str has anything in it.  Fucking Linux compilers.
  if (lStrChanged) {
    if (ui) {
      ui->set_level(str.str());
    }
    // Ok to delete even if ui is NULL, str.str() will allocate the memory to
    // kill off.
    delete str.str();
  }
}  



void Game::refill_check() {
  if (refillTimer.ready()) {
    if (style->can_refill_game_objects()) {
      gameObjects.refill();
    }    
    style->refill_enemies(enemiesRefill,&world,&locator,
                          (IPhysicalManagerP)this);
    refillTimer.set(REFILL_TIME);
  }

  refillTimer.clock();
}



Boolean Game::potential_human_filter(const PhysicalContext* pc,void*) {
  return pc->potentialHuman;
}



// New version which uses Locator::filter_contexts.
// More general and easy to extend.
PhysicalP Game::human_physical(int humanNum) {
  // The class of the human to be created.
  const PhysicalContext *theContext;

  if (humanClass == A_None ||
      // Nonzero numbered humans are always chosen randomly.
      // Careful with the Dragon hack below, look at humansNum.
      humanNum != 0) {
    // Choose class randomly.
    
    // Get list of all classes that are potential Human classes.
    const PhysicalContext *list[A_CLASSES_NUM];
    int size = locator.filter_contexts(list,NULL,potential_human_filter,NULL);
    assert(size);
    theContext = list[Utils::choose(size)];
  }
  else {
    // Class has already been chosen.
    theContext = locator.get_context(humanClass);
  }
  

  // Create an object of the class corresponding to theContext.
  Pos pos = style->human_initial_pos(&world,&locator,theContext->sizeMax);
  assert(theContext->create);
  PhysicalP obj = NULL;

  // Hack for Dragon, need a more general mechanism.
  if (humanClass == A_Dragon && humanNum == 0) {
    PtrList ret;
    Segmented::create_and_add_composite(ret,&world,&locator,
                                        Dragon::SEGMENTS_NUM,pos,
                                        theContext->create,theContext->arg);
    obj = (PhysicalP)ret.get(0);
  }
  else {
    obj = theContext->create(theContext->arg,&world,&locator,pos);
    locator.add(obj);
  }
  assert(obj);


  // Add extra abilities
  if (obj->is_creature() && humanExtraAbility != AB_MAX) {
    AbilityP a = NULL;
    switch(humanExtraAbility) {
      case AB_Sticky:
        if (Sticky::has_required_frames(
              ((MovingP)obj)->get_moving_context())) {
          a = new Sticky(Ninja::stickyContext,Sticky::defaultXdata);
        }
        else {
          cerr << obj->get_class_name() 
               << " does not have all the animation frames to be a Sticky." 
               << endl;
        }
      break;

      case AB_Flying:
        a = new Flying(ChopperBoy::flyingContext,Flying::defaultXdata);
      break;

      case AB_Hopping:
        a = new Hopping(Frog::hoppingContext,Hopping::defaultXdata);
      break;

      case AB_OnFire:
        a = new OnFire(FireDemon::onFireContext,OnFire::defaultXdata);
      break;

      default:
        cerr << "unknown ability" << endl;
    }

    if (a) {
      // Can only have one locomotion.
      if (a->is_locomotion()) {
        LocomotionP l = obj->get_locomotion();
        if (l) {
          ((CreatureP)obj)->remove_ability(l->get_ability_id());
        }
      }
      ((CreatureP)obj)->add_ability(a);
    }
  }

  return obj;
}



Boolean Game::potential_enemy_filter(const PhysicalContext* pc,void*) {
  return pc->potentialEnemy;
}



PhysicalP Game::enemy_physical() {
  return enemy_physical(NULL,0);
}



// New version which uses Locator::filter_contexts.
// More general and easy to extend.
PhysicalP Game::enemy_physical(ClassId *choosable,int choosableNum) {
  ClassId localChoosable[A_CLASSES_NUM];

  if (choosableNum == 0) {
    // Ask the Locator for potential enemies since none were passed in.
    assert(!choosable);
    choosable = localChoosable;
    choosableNum = locator.filter_contexts(NULL,choosable,
                                           potential_enemy_filter,NULL);
  }

  // Fill in weights.
  int weights[A_CLASSES_NUM];
  int n;
  for (n = 0; n < choosableNum; n++) {
    const PhysicalContext *context = locator.get_context(choosable[n]);
    assert(context);
    weights[n] = context->enemyWeight;
  }

  // Sum of all weights.
  int weightsSum = 0;
  for (n = 0; n < choosableNum; n++) {
    weightsSum += weights[n];
  }
  
  // Choose random weight in [0,weightsSun).
  int choice = Utils::choose(weightsSum);

  // weightBelow is always the total amount of weight of the classes from
  // 0 to n.  When this exceeds the chosen weight, create a Physical of 
  // class n.
  int weightBelow = 0;
  for (n = 0; n < choosableNum; n++) {
    weightBelow += weights[n];
    if (choice < weightBelow) {
      return enemy_physical(choosable[n]);
    }
  }
  assert(0);
  return NULL;
}



// New more general version.
PhysicalP Game::enemy_physical(ClassId classId) {
  const PhysicalContext *context = locator.get_context(classId);
  assert(context);

  Pos pos = world.empty_rect(context->sizeMax);
  assert(context->create);
  PhysicalP obj = context->create(context->arg,&world,&locator,pos);
  return obj;
}



char *Game::choose_ranking(int rawKills) {
  // Adjust the number of kills according to the level of difficulty.
  int kills = (int)(rawKills * difficultyLevels[difficulty].rankMultiplier);

  // Figure out the ranking set, they are listed in order.
  int setNum = 0;
  assert(kills >= 0 && rankingSets[0].killsMin == 0);
  while (setNum < RANKING_SETS_NUM - 1) {
    if (kills >= rankingSets[setNum + 1].killsMin) {
      // We have the min number of kills to go to the next set.
      setNum++;
    }
    else {
      // We are at the highest set we can be at.
      break;
    }
  }
    
  // Determine size of this RankingSet.
  int cardinality = 0;
  // RankingSet::rankings must be NULL-terminated.
  while(rankingSets[setNum].rankings[cardinality]) {
    cardinality++;
  }
  assert(cardinality > 0);
  int which = Utils::choose(cardinality);
  return rankingSets[setNum].rankings[which];
}



void Game::end_game(Boolean showMessages) {
  state = gameOver;
  timer.set(OTHER_INPUT_RESET);

  if (!showMessages) {
    return;
  }

  // I believe role->get_humans_num() and locator.humans_registered() 
  // should always be the same.
  
  int humansNum = locator.humans_registered();

  // Special case, where no humans were playing the game.
  if (humansNum == 0) {
    ostrstream msg;
    if (locator.enemies_alive() == 0) {
      msg << "Total Devastation: Everyone is Dead";
    }
    else {
      msg << "One Machine Player Survived";
    }
    msg << ends;
    locator.arena_message_enq(msg.str(),NULL,GAME_OVER_ARENA_MESSAGE_TIME);
    return;
  }

  // Normal case, tell each human player individually how he did.
  for (int n = 0; n < humansNum; n++) {
    HumanP human = locator.get_human(n);
    ostrstream msg;
    
    // Soups are only taken into account if you have unlimited lives.
    int totalKills;
    if (human->get_lives() == IT_INFINITE_LIVES) {
      // A Human should only have infinite lives for the EXTENDED game style.
      // If this changes, consider changing the logic in the Ui for 
      // displaying
      // number of human kills.
      assert(style->get_type() == EXTENDED);
      
      totalKills = human->get_human_kills() 
        - human->get_soups() + human->get_enemy_kills();
    }
    else {
      totalKills = human->get_human_kills() + human->get_enemy_kills();
    }
    
    char *ranking = choose_ranking(totalKills);
    msg << totalKills << (totalKills == 1 ? "Kill" : " Kills") 
        << ", Rank: " << ranking << ends;
    IntelId humanIntelId = human->get_intel_id();
    locator.arena_message_enq(msg.str(),&humanIntelId,
                              GAME_OVER_ARENA_MESSAGE_TIME);
  }
}



void Game::new_level() {
  // Always reset to be sure, even if we are going to set it below.
  world.set_special_map_next(NULL);
  world.set_rooms_next(worldRooms);
  // After the previous two calls, so it takes precedence.
  if (worldFile) {
    world.set_file_next(worldFile);
  }
    
  timer.set(0);

  // Will clean out non-persistent teams.
  locator.level_reset();

  ostrstream lStr;       // For level box on the side of the ui->
  ostrstream lTitleStr;  // For Ui title screen.
  Boolean doBonus;

  // Possibly increment level count number.
  if (style->advance_level()) {
    level++;
    if (level > levelHighest) {
      levelHighest = level;
    }
    // See if we should award a bonus for work on the previous level.
    doBonus = style->award_bonus();
  }
  else {
    // Never get a bonus if you can't advance to the next level.
    doBonus = False; 
  }

  // Ask style to start new level
  // Every style MUST set something for lStr and lTitleStr.
  style->new_level(level,&world,&locator,difficultyLevels[difficulty],
                   lStr,lTitleStr,
                   (IPhysicalManagerP)this,role->get_humans_num());

#if WIN32
  start_soundtrack();
#endif

  // Must be after style set to new_level.
  gameObjects.level_reset(world.get_dim(),style);
  
  // Refill gameObjects immediately, but not style->refill().
  refillTimer.set(REFILL_TIME);
  if (style->can_refill_game_objects()) {
    gameObjects.refill();
  }

  // Go to awardBonus state.
  if (doBonus) {
    delete levelTitleStored; // If it already exists.
    // Store levelTitle string for later use.
    levelTitleStored = lTitleStr.str();

    // Tell user about the bonus.
    award_bonuses_now();
    timer.set(AWARD_BONUS_TIME);
      
    state = awardBonus;
  }
  // Go to getBearings state.
  else {
    get_bearings(lTitleStr.str());
  }
  
  if (ui) {
    ui->set_level(lStr.str());
  }
  delete lStr.str();

  // Will pass in more info later.
  role->new_level(this,&world,&locator);
}



void Game::award_bonuses_now() {
  int hNum = locator.humans_playing();
  for (int n = 0; n < hNum; n++) {
    // Grab the Physical for human number n.
    HumanP h = locator.get_human(n);
    PhysicalP p = NULL;
    if (h) {
      p = locator.lookup(h->get_id());
    }
    if (!p || !p->alive() || p->get_intel() != h) {
      continue;
    }

    char* awardMsg = NULL;
    int count = 0;
    while (!awardMsg && count < AWARD_BONUS_TRIES) {
      count++;

      // Decide what bonus to give.
      int which = Utils::choose(6);
      switch (which) {
      // Grant full health.
      case 0:
        if (p->get_health() < (int)(.7 * p->get_health_max())) {
          p->heal();
          awardMsg = "Bonus: Full Health";
        }
        break;

      // AutoLancer
      case 1: {
          // Make sure p is a User and that p can hold more weapons.
          UserP user = (UserP)p->get_ability(AB_User);
          if (user && user->get_weapons_num() < PH_WEAPONS_MAX) {
            const Area& area = p->get_area();
            Pos newPos = area.get_middle() - 0.5 * AutoLancer::get_size_max();
            PhysicalP weap = new AutoLancer(&world,&locator,newPos);
            assert(weap);
            locator.add(weap);
            awardMsg = "Bonus: Auto-Lancer";
          }
        }
        break;

      // DemonSummoner
      // Should share code with AutoLancer.
      case 2: {
          // Make sure p is a User and that p can hold more weapons.
          UserP user = (UserP)p->get_ability(AB_User);
          if (user && user->get_weapons_num() < PH_WEAPONS_MAX) {
            const Area& area = p->get_area();
            Pos newPos = area.get_middle() - 0.5 * DemonSummoner::get_size_max();
            PhysicalP weap = new DemonSummoner(&world,&locator,newPos);
            assert(weap);
            locator.add(weap);
            awardMsg = "Bonus: Demon-Summoner";
          }
        }
        break;

        // Lasers
        case 3:  {     
          // Already has BuiltIn weapon
          if (!p->get_ability(AB_BuiltIn)) {
            // Don't want to be BuiltIn and Fighter at the same time.
            AbilityP fighter = p->get_ability(AB_Fighter);
            if (fighter) {
              ((CreatureP)p)->remove_ability(AB_Fighter);
            }

            AbilityP a = new BuiltIn(Walker::builtInContext,BuiltIn::defaultXdata);
            assert(a);
            ((CreatureP)p)->add_ability(a);
            awardMsg = "Bonus: Built-In Lasers";
          }
        }
        break;

        // XProtection
        case 4: {
          if (p->is_moving()) {
            PhysicalP prevProt = locator.lookup(((MovingP)p)->get_protection());
            if (prevProt) {
              // Don't give XProtection if p already has it.
              if (prevProt->get_class_id() == A_XProtection) {
                break;
              }
              prevProt->kill_self();
            }
            ProtectionP xProt = new XProtection(&world,&locator,p->get_area());
            assert(xProt);
            locator.add(xProt);
            ((MovingP)p)->set_protection(xProt->get_id());
            awardMsg = "Bonus: X-Shield";
          }
        }
        break;

        // Grant a psycho-chicken.
        case 5: {
          const Area& area = p->get_area();
          Pos p;
          Size s;
          area.get_rect(p,s);
          Size cSize = Chicken::get_size_max();
          for (int n = 0; n < BONUS_CHICKENS_NUM; n++) {
            // Position of the new Chicken.
            Pos cPos(p.x + (s.width << 1) - (cSize.width << 1),
                     p.y + s.height - cSize.height);
            PhysicalP c = new Chicken(&world,&locator,cPos);
            assert(c);
            // Create psychotic pet inteligence.
            IntelOptions ops;
            ops.psychotic = True;
            NeutralP pet = new Pet(&world,&locator,"psycho-chicken",
                                 &ops,ITpsychotic,h);
            assert(pet);

            c->set_intel(pet);
            locator.add(c);
            locator.register_neutral(pet);
          }
          awardMsg = "Bonus: Psycho-Chicken";
        }
        break;

        default:
          assert(0);
      } // switch
    } // while     
    if (!awardMsg) {
      awardMsg = "Cannot Award Bonus Now";
    }

    IntelId hId = h->get_intel_id();
    locator.arena_message_enq(Utils::strdup(awardMsg),&hId,AWARD_BONUS_TIME);    
  } // for humans num
}



// This method takes ownership for the memory of levelTitleStr.
void Game::get_bearings(char *levelTitleStr) {
  if (!noLevelTitle) {
    // Give user a few turns to figure out what's going on before starting the new level.
    state = getBearings;
    timer.set(GET_BEARINGS_TIME);
    locator.arena_message_enq(levelTitleStr,NULL,GET_BEARINGS_TIME);
  }
  else {
    // start immediately, no level title or time to get bearings.
    state = getBearings;
    timer.set(0);
    delete levelTitleStr;
  }
}



void Game::reset() {
  assert(role->get_type() != R_CLIENT);
  
  if (styleNext->need_difficulty(enemiesNumNext)) {
    assert(difficultyNext != DIFF_NONE);  // should be specified by now.
    difficulty = difficultyNext;
  }
  
  int hNumNext = (role->get_type() == R_SERVER) ? 
    HU_UNSPECIFIED : 
    humansNumNext;
  char *err = styleNext->can_reset(hNumNext,enemiesNumNext,cooperativeNext);
  // The style may not allow us to play with the given settings.
  if (err) {
    locator.arena_message_enq(err);

    // Force the game to end.
    state = gameOver;
    timer.set(OTHER_INPUT_RESET);
    return;
  }


  // Get ready for a new game.

  Stats::enable();
  locator.set_messages_ignore(False);

  if (state == gameOn || state == gameExtra) {
    end_game(False);
  }

  enemyNameCount = 0;
  enemiesRefill = enemiesRefillNext;

  cooperative = cooperativeNext;

  // Would be nice to have a more general mechanism.
  Composite::reset_id_generator();


  ///////////////// Before here, switch on styleNext.
  delete style;
  style = styleNext;
  styleNext = style->clone();
  ///////////////// After this point, switch on style


  // Role may ignore the value passed in.
  role->set_humans_num(humansNumNext);
  // Just to be sure.
  if (ui) {
    ui->set_humans_num(role->get_humans_num());
  }

  level = 0; // So will start out at 1.
  levelHighest = 0;

  // Set the enemies reflexes according to the new level of difficulty.
  Enemy::set_reflexes_time(difficultyLevels[difficulty].reflexes);

  // ui->reset() used to be after Locator::reset()
  if (ui) {
    ui->reset(); 
  }
  locator.reset();
  
  // Moved resetting world rooms to new_level(); 

  ostrstream msg;
  msg << wittySayings[Utils::choose(WITTY_SAYINGS_NUM)] << ends;
  locator.message_enq(msg.str());

  /* Don't need to call Ui::set_* because the new values originally came 
     from ui-> */

  gameObjects.game_reset(oneItem,noItems,oneEach);

  style->reset(&world,&locator,difficultyLevels[difficulty],enemiesNumNext);

  // Used to be after new_level().  Shouldn't hurt anything to be here.
  // Needs to be after style is set to new value.
  role->reset(this);

  // NOTE: Humans haven't been added to the Locator yet.
  new_level();
  
  // After new_level because world must be reset.
  // After role->set_humans_num()
  humans_reset(); 

  // clock once to put objects in game.
  // This has to be after humans_reset().
  assert(world.just_reset());
  world.clock();
  locator.clock();

  // Force Game::clock to set Ui::set_intels_playing().
  humansPlayingPrev = enemiesPlayingPrev = -1;  // Used to be 0.
}



void Game::new_game() {
  state = newGame;
  timer.set(0);
}



void Game::process_x_resources(int *,char **)
{
  // Should have already checked for UI before coming here.
  assert(ui != NULL);

#if X11
  for (int dpyNum = 0; dpyNum < ui->get_dpy_max(); dpyNum++)
    if (!ui->keyset_set(dpyNum)) {
      const char* const *keysNames = ui->get_keys_names();
      
      KeySym right[UI_KEYS_MAX][2],left[UI_KEYS_MAX][2];
      
      for (int n = 0; n < UI_KEYS_MAX; n++)
        for (int which = 0; which < 2; which++) {
          { // Right keys.
            right[n][which] = 0;
            
            ostrstream strm;
            if (which == 0)
              strm << "right_" << keysNames[n] << ends;
            else
              strm << "right_" << keysNames[n] << "_2" << ends;
            char *option = strm.str();
            
            // Should we free value??
            char *value = XGetDefault(ui->get_dpy(0),XEVIL_CLASS,option);
            if (value) {
              KeySym keysym = XStringToKeysym(value);
              if (keysym != NoSymbol)
                right[n][which] = keysym;
            }
            delete option;
	      }
	      
	      { // Left Keys.
            left[n][which] = 0;
            
            ostrstream strm;
            if (which == 0)
              strm << "left_" << keysNames[n] << ends;
            else
              strm << "left_" << keysNames[n] << "_2" << ends;
            char *option = strm.str();
            
            // Should we free value??
            char *value = XGetDefault(ui->get_dpy(0),XEVIL_CLASS,option);
            if (value) {
              KeySym keysym = XStringToKeysym(value);
              if (keysym != NoSymbol)
                left[n][which] = keysym;
            }
            delete option;
	      }
	    }

      // This sets the default, even if there are no resources found.
      ui->set_keyset(dpyNum,KEYSET_DEFAULT,right,left);
    }
#endif
}



void Game::parse_args(int *argc,char **argv) {
  // Create a bunch of "-name<x>" strings for comparing with command-line 
  // args.
  ostrstream dashName[Locator::HUMANS_MAX];
  int n;
  for (n = 0; n < Locator::HUMANS_MAX; n++) {
    dashName[n] << "-name" << n << ends;
  }

  // Defaults
  int skip = Connection::ADJUST_SKIP;
  CMN_PORT clientPort = 0;  // Means not specified


  // Take two passes at the arguments because the constructors for Roles need
  // certain arguments to already be parsed.
  //////////// First Pass ///////////

  // Get -name so we have it for the constructor of Client.
  for (n = 0; n < *argc; n++) {
    // -name is the same as -name0
    if (!strcmp("-ability",argv[n]) && (n + 1 < *argc)) {
      n++;
      if (!strcmp(argv[n],"flying")) {
        humanExtraAbility = AB_Flying;
      }
      else if (!strcmp(argv[n],"on-fire")) {
        humanExtraAbility = AB_OnFire;
      }
      else if (!strcmp(argv[n],"sticky")) {
        humanExtraAbility = AB_Sticky;
      }
      else if (!strcmp(argv[n],"hopping")) {
        humanExtraAbility = AB_Hopping;
      }
    }
    else if (!strcmp("-accept_agreement",argv[n])) {
      LAgreement::comm_line_accepted();
    }
    else if (!strcmp("-client_port",argv[n]) && (n + 1 < *argc)) {
      clientPort = atoi(argv[n+1]);
      n++;
    }
    else if (! strcmp("-dedicated",argv[n])) {
      dedicatedServer = True;
    }
    else if (!strcmp("-name",argv[n]) && (n + 1 < *argc)) {
      humanNames[0] = Utils::strdup(argv[n+1]);
      n++;
    }
    else if (! strcmp("-no_ui",argv[n])) {
      noUi = True;
    }
    else if (!strcmp("-observer",argv[n])) {
      observer = True;
    }
    else if (!strcmp("-skip",argv[n]) && (n + 1 < *argc)) {
      skip = Utils::maximum(Connection::ADJUST_SKIP,atoi(argv[n + 1]));
      n++;
    }
    // Check "-name<x>" arguments.
    else {
      for (int j = 0; j < Locator::HUMANS_MAX; j++) {
        if ((! strcmp(dashName[j].str(),argv[n])) && (n + 1 < *argc)) {
          humanNames[j] = Utils::strdup(argv[n+1]);
          n++;
        }
      }
    }
  }
 

  //////////// Second Pass ///////////

  for (n = 0; n < *argc; n++) {
    if (! strcmp("-buggy",argv[n])) {
      // For simulating a bad network connection.
      GenericStream::set_buggy(True);
    }
#if WIN32
#if _DEBUG
    // Used to be called -bmptransfer
    else if (!strcmp("-bmp_match_palette",argv[n]) && (n + 2 < *argc)) {
      UiPlayer::bmp_match_palette(argv[n+1],argv[n+2]);
      // suck off two extra arguments
      n += 2;
    }
#endif
#endif
    else if (role == NULL && 
             !strcmp("-connect",argv[n]) && (n + 1 < *argc)) {
      IViewportInfo* vInfo = Ui::get_viewport_info();

      // -connect <server> <port>
      if (n + 2 < *argc && argv[n+2][0] != '-') {
        // skip is a local variable from above
        role = new Client(argv[n+1],argv[n+2],clientPort,
                          humanNames[0],vInfo,skip,&locator);
        // suck off two extra arguments
        n += 2;
      }
      // -connect <server>
      else {
        // skip is a local variable from above
        role = new Client(argv[n+1],NULL,clientPort,
                          humanNames[0],vInfo,skip,&locator);
        // suck off one extra argument
        n++;
      }
      assert(role);
      // Wait until Client::connect_server to check role->ok()
    }
    else if (! strcmp("-cooperative",argv[n])) {
      cooperativeNext = True;
    }
    else if (! strcmp("-debug_info",argv[n])) {
      DebugInfo::turn_on();
    }
    else if (!strcmp("-difficulty",argv[n]) && (n + 1 < *argc)) {
      n++;
      for (int m = 0; m < DIFFICULTY_LEVELS_NUM; m++) {
        if (!strcmp(difficultyLevels[m].name,argv[n]))
          difficultyNext = m;
      }
    }
    else if (!strcmp("-disconnect_time",argv[n]) && (n + 1 < *argc)) {
      n++;
      Server::set_no_activity_disconnect(atoi(argv[n]));
    }
    else if (! strcmp("-draw_rects",argv[n])) {
      locator.set_draw_rects(True);
    }
    else if (! strcmp("-duel",argv[n])) {
      delete style;
      style = GameStyle::by_type(DUEL);
    }
    else if (! strcmp("-dummy_arg",argv[n])) {
      // Do nothing this is just a placeholder.
    }
    // "-enemies" is down with "-machines"
    else if (! strcmp("-extended",argv[n])) {
      delete style;
      style = GameStyle::by_type(EXTENDED);
    }
#if WIN32
    else if (!strcmp("-full_screen",argv[n])) {
      fullScreen = True;
    }
    // Auto-generate XPM images for UNIX from the Windows bitmaps.
    else if (!strcmp("-gen_xpm",argv[n]) && (n + 1 < *argc)) {
      n++;
      UiPlayer::set_generate_xpm(argv[n]);
    }
    else if (!strcmp("-cd",argv[n])) {
      musictype = UIsettings::CD;
    }
    else if (!strcmp("-midi",argv[n])) {
      musictype = UIsettings::MIDI;
    }
    else if (!strcmp("-no_soundtrack",argv[n])) {
      musictype = UIsettings::NONE;
    }
#endif
    else if (!strcmp("-h",argv[n])
             || !strcmp("-help",argv[n])) {
      cout 
        << endl
        << "See http://www.xevil.com/docs/instructions.html for full description." << endl 
        << "usage: " << argv[0] << " <argument list>" << endl
        << endl
        << "Common arguments:" << endl
        << "-display <displayname> or -d <displayname>" << endl
        << "    set the X display for all players in the game" << endl
        << "-display<N> <displayname> or -d<N> <displayname>" << endl
        << "    set the X display for a specific player, player number N, starts with '0'" << endl 
        << "-font <fontname> or -fn <fontname>" << endl
        << "    set the font" << endl
        << "-h or -help" << endl
        << "    print help message" << endl
        << "-info" << endl
        << "    print license agreement" << endl
        << "-v or -version" << endl
        << "    print version info" << endl
        << "-world <worldfile>" << endl
        << "    use the worldfile to specify the map" << endl
        << endl
        << "-connect <servername> {serverport}" << endl
        << "    connect as a client to an XEvil server, serverport is optional" << endl
        << "-name <playername>" << endl
        << "    only with -connnect, specify your player name" << endl
        << "-no_dead_reckoning" << endl
        << "    only with connect, disable dead reckoning" << endl
        << endl
        << "-server {port}" << endl
        << "    run an XEvil server, port is optional" << endl
        << "-no_ui" << endl
        << "    only with -server, run as a command-line server, implies -observer" << endl
        << "-observer" << endl
        << "    only with -server, run server with no human player on the server machine" << endl
        << "-disconnect_time" << endl
        << "    only with -server, specify time (ms) before disconnecting clients that do not respond" << endl
        << "-no_disconnect" << endl
        << "    only with -server, don't disconnect clients that do not respond" << endl;

      // Just exit, don't want to deal with Ui being created, etc, if we
      // use Game::quit().
      exit(1);
    }
    else if (! strcmp("-harmless",argv[n])) {
      intelHarmless = True;
    }
    else if (!strcmp("-human_class",argv[n]) && (n + 1 < *argc)) {
      assert(humanClass == A_None);
      n++;
      
      // If not found, humanClass will just remain as A_None.
      humanClass = parse_class_name(&locator,argv[n]);
    }
    else if (! strcmp("-human_reflexes",argv[n]) && (n + 1 < *argc)) {
      n++;
      int val = atoi(argv[n]);
      Client::set_human_reflexes(val);
      cout << "Human reflexes set to " << val << " milliseconds." << endl;
    }
    else if ((! strcmp("-humans",argv[n])) && (n + 1 < *argc)) {
      humansNumNext = atoi(argv[n+1]);
      n++;
    }
    else if (! strcmp("-infinity",argv[n])) {
      infiniteLives = True;
    }
    else if (! strcmp("-info",argv[n])) {
      // Print the license agreement out to standard out.
      Line::set_text_columns(70);
      Page thePage(NULL,LAgreement::get_text());
      const PtrList& lines = thePage.get_lines();
      for (int nn = 0; nn < lines.length(); nn++) {
        char* txt = ((Line*)lines.get(nn))->alloc_text();
        cout << txt << endl;
        delete [] txt;
      }      
      exit(0);
    }
    else if (! strcmp("-kill",argv[n])) {
      delete style;
      style = GameStyle::by_type(KILL);
    }

#if X11
    else if ((! strcmp("-keys",argv[n])) && (n + 1 < *argc)) {
	  // Sets the keyset for all displays to the same value.
	  n++;
      if (! strcmp("alpha",argv[n]))
        uiKeyset = UIalpha;
      if (! strcmp("decmips",argv[n]))
        uiKeyset = UIdecmips;
      if (! strcmp("iris",argv[n]))
        uiKeyset = UIiris;
      if (! strcmp("mac",argv[n]))
        uiKeyset = UImac;
      if (! strcmp("ncd",argv[n]))
        uiKeyset = UIncd;
      if (! strcmp("rsaix",argv[n]))
		uiKeyset = UIrsaix;
      if (! strcmp("sun3",argv[n]))
        uiKeyset = UIsun3;
      if (! strcmp("sun4",argv[n]))
        uiKeyset = UIsun4;
      if (! strcmp("sun4_sparc",argv[n]))
        uiKeyset = UIsun4_sparc;
      if (! strcmp("tektronix",argv[n]))
        uiKeyset = UItektronix;
      if (! strcmp("linux",argv[n]))
        uiKeyset = UIlinux;
	}
    else if (!strcmp("-large_viewport",argv[n])) {
      Ui::set_large_viewport(True);
    }
#endif // X11
    else if (!strcmp("-levels",argv[n])) {
      delete style;
      style = GameStyle::by_type(LEVELS);
    }
    else if (!strcmp("-log_file",argv[n]) && (n + 1 < *argc)) {
      delete daemon; // May be NULL.
      daemon = new Daemon(argv[n+1]);
      n++;
    }
    else if ((!strcmp("-machines",argv[n]) || !strcmp("-enemies",argv[n]))
             && (n + 1 < *argc)) {
      enemiesNumNext = atoi(argv[n+1]);
      enemiesNumNext = Utils::minimum(enemiesNumNext,Locator::ENEMIES_MAX);
      n++;
    }
    else if (! strcmp("-map",argv[n])) {
      world.set_map_print(True);
    }
// Should this be commented
#if WIN32
    else if (! strcmp("-midi_location",argv[n]) && (n + 1 < *argc)) {
      mididirectory = argv[n+1];
      n++;
    }
#endif
    else if (! strcmp("-net_stats",argv[n])) {
      Role::show_stats();
    }
#if 0
    else if (! strcmp("-no_adjust_skip",argv[n])) {
      Connection::disable_adjust_skip();
    }
#endif
    else if (! strcmp("-no_demo",argv[n])) {
      noDemo = True;
    }
    else if (!strcmp("-no_disconnect",argv[n])) {
      Server::disable_auto_disconnect();
    }
    else if (!strcmp("-no_dead_reckoning",argv[n])) {
      Client::disable_dead_reckoning();
    }
#if WIN32
    else if (! strcmp("-no_init_graphics",argv[n])) {
      Ui::no_init_graphics();
    }
#endif
    else if (! strcmp("-no_items",argv[n])) {
      noItems = True;
    }
    else if (! strcmp("-no_level_title",argv[n])) {
      noLevelTitle = True;
    }
    else if (! strcmp("-no_movers",argv[n])) {
      World::disable_movers();
    }
    else if (! strcmp("-no_new_level",argv[n])) {
      noNewLevel = True;
    }
#if X11
    else if (! strcmp("-no_smooth_scroll",argv[n])) {
      Ui::set_smooth_scroll(False);
    }
#endif
    else if (! strcmp("-old_draw",argv[n])) {
      locator.set_drawing_algorithm(Locator::DRAW_NO_MERGE);
    }
    else if (! strcmp("-one_each",argv[n])) {
      oneEach = True;
    }
    else if ((! strcmp("-one_item",argv[n])) && (n + 1 < *argc)) {
	  n++;
	  oneItem = Utils::strdup(argv[n]);
    }
    else if (! strcmp("-echo_ping_pong",argv[n])) {
      Role::echo_ping_pong();
    }
    else if (! strcmp("-echo_turn_windows",argv[n])) {
      Connection::enable_echo_turn_windows();
    }
    // Used to be just -regenerate_machines.
    else if (!strcmp("-regenerate_machines",argv[n]) || 
             !strcmp("-regenerate_enemies",argv[n])) {
	    enemiesRefillNext = True;
    }
    else if (! strcmp("-reduce_draw",argv[n])) {
      Ui::set_reduce_draw(True);
    }
    else if (! strcmp("-rooms",argv[n]) && (n + 1 < *argc)) {
      Rooms temp;
      if (sscanf(argv[n+1],"%dx%d",
                 &temp.acrossMax,&temp.downMax) == 2)
        worldRooms = temp;
      n++;
    }
    else if ((! strcmp("-scenario",argv[n])) && (n + 1 < *argc)) {
      n++;
      Scenarios::set_override(argv[n]);
    }
    else if (!strcmp("-scenarios",argv[n])) {
      delete style;
      style = GameStyle::by_type(SCENARIOS);
    }
    else if (role == NULL && 
             (!strcmp("-server",argv[n]))) {
      // Server has human player unless we are an observer or we have no Ui 
      // or we are a dedicated server
      Boolean localHuman = !(observer || noUi || dedicatedServer);
      // Note that -dedicated without -server is handled after parse_args in Game's 
      // constructor.

      // -server <port>
      if (n + 1 < *argc && argv[n+1][0] != '-') {
        role = new Server(localHuman,argv[n+1],&locator);
        // suck off extra argument.
        n++;
      }
      else {
        role = new Server(localHuman,NULL,&locator);
      }
      assert(role);
    }
#if X11
    else if (!strcmp("-small_viewport",argv[n])) {
      Ui::set_large_viewport(False);
    }
    else if (! strcmp("-smooth_scroll",argv[n])) {
      Ui::set_smooth_scroll(True);
    }
#endif
    else if ((! strcmp("-speed",argv[n])) && (n + 1 < *argc)) {
      quanta = atoi(argv[n+1]);
      n++;
    }
    else if (! strcmp("-stats",argv[n])) {
      showStats = True;
    }
#if X11
    else if (! strcmp("-synchronous",argv[n])) {
      Ui::set_synchronous();
    }
#endif
    else if ((! strcmp("-target_delay",argv[n])) && (n + 1 < *argc)) {
      n++;
      Turn val = Utils::maximum(1,atoi(argv[n]));
      Connection::set_target_delay(val);
    }
    else if (! strcmp("-training",argv[n])) {
      delete style;
      style = GameStyle::by_type(TRAINING);
    }
#if X11
    else if (! strcmp("-use_averaging",argv[n])) {
      Xvars::set_use_averaging(True);
    }
    else if (! strcmp("-use_buffer",argv[n])) {
      Ui::set_use_buffer(True);
    }
    else if (! strcmp("-no_buffer",argv[n])) {
      Ui::set_use_buffer(False);
    }
#endif
    else if (!strcmp("-v",argv[n])
             || !strcmp("-version",argv[n])) {
      // Don't have to do anything, info was already printed.

      // Just exit, don't want to deal with Ui being created, etc, if we
      // use Game::quit().
      exit(1);
    }
    else if (! strcmp("-world",argv[n]) && (n + 1 < *argc)) {
      worldFile = argv[n+1];
      n++;
    }
  } // for
  
  
  // Delete memory for "-name" strings.
  for (n = 0; n < Locator::HUMANS_MAX; n++) {
    delete dashName[n].str();
  }
}



char **Game::display_names(int *argc,char **argv) {

#if X11
  ostrstream dashDisplay[UI_VIEWPORTS_MAX][2];
  char **displayNames = new charP [UI_VIEWPORTS_MAX];

  int n;
  for (n = 0; n < UI_VIEWPORTS_MAX; n++) {
    displayNames[n] = new char [Xvars::DISPLAY_NAME_LENGTH];
    strcpy(displayNames[n],"");
    dashDisplay[n][0] << "-display" << n << ends;
    dashDisplay[n][1] << "-d" << n << ends;
  }

  // Loop through all command line arguments.
  for (n = 0; n < *argc - 1 ; n++) {
    // Set display name for all viewports.
    if (!strcmp(argv[n],"-display") || !strcmp(argv[n],"-d")) {
      assert(strlen(argv[n+1]) < Xvars::DISPLAY_NAME_LENGTH);
      for (int vNum = 0; vNum < UI_VIEWPORTS_MAX; vNum++) {
        strcpy(displayNames[vNum],argv[n+1]);
      }
    }
      
    // Set display name for one viewport.
    for (int m = 0; m < UI_VIEWPORTS_MAX; m++) {
      for (int which = 0; which < 2; which++) {
        if (!strcmp(argv[n],dashDisplay[m][which].str())) {
          assert(strlen(argv[n+1]) < Xvars::DISPLAY_NAME_LENGTH);
          strcpy(displayNames[m],argv[n+1]);
        }
      }
    }
  }

  for (n = 0; n < UI_VIEWPORTS_MAX; n++) {
    for (int which = 0; which < 2; which++) {
      delete dashDisplay[n][which].str();
    }
  }
  return displayNames;
#endif

#if WIN32
  return NULL;
#endif
}



char *Game::font_name(int *argc,char **argv) {
  // Loop through all command line arguments.
  for (int n = 0; n < *argc - 1 ; n++) {
    if (!strcmp("-font",argv[n]) || !strcmp("-fn",argv[n])) {
      return argv[n+1];
    }
  }
  
  return NULL;
}

  

void Game::humans_reset() {
  assert(role->get_type() != R_CLIENT);

  int humansNum = role->get_humans_num();

//  assert(humansNum <= Xvars::HUMAN_COLORS_NUM);  no longer true.
  assert(humansNum <= Locator::HUMANS_MAX);
  assert(humansNum <= INTEL_NAMES_NUM);

  // randomize the list of names here.
  Utils::random_list(intelNamesIndices,INTEL_NAMES_NUM);
  

  if (cooperative) {
    // Each human contributes to the pool of lives.
    int livesOne = style->human_initial_lives();
    if (livesOne != IT_INFINITE_LIVES) {
      sharedLives =  livesOne * humansNum;
    }
    else {
      sharedLives = livesOne;
    }
    
    // BUG: sharedLives doesn't make a lot of sense for CLIENT/SERVER

    // Must be after setting cooperative to cooperativeNext.
    locator.add_persistent_team(humans_team,NULL,NULL);
  }
  else {
    locator.add_persistent_team(one_human_team,NULL,NULL);
  }
  

  // Create humans along with their viewports, if necessary.
  for (int h = 0; h < humansNum; h++) {
    // Don't create extra viewports on server.
    if (h < role->num_viewports_needed()) {

      if (ui) {
        assert(h <= ui->get_viewports_num());
        if (h == ui->get_viewports_num()) {
          int vNum = ui->add_viewport();
          assert(h == vNum);
#if X11
          int v = ui->get_viewports_num_on_dpy(ui->get_dpy_num(vNum));
#endif
#if WIN32
          int v = vNum;
#endif
          if (v == 1) {
            ui->set_input(vNum,UI_KEYS_RIGHT);
          }
          else if (v == 2) {
            ui->set_input(vNum,UI_KEYS_LEFT);
          }
        }
      } // if (ui)
    } // for h

    create_human_and_physical(h);
  }


  // Delete extra viewports.
  if (ui && 
      role->get_type() != R_SERVER) {
    int delNum = ui->get_viewports_num() - humansNum;
    for (int n = 0; n < delNum; n++) {
      if (ui->get_viewports_num() > 1) {
        ui->del_viewport();
      }
    }
  }


  // else was done for each human in create_human_and_physical
}



void Game::create_human_and_physical(int h) {
  if (h >= Locator::HUMANS_MAX) {
    cerr << "Maximum number of humans already reached." << endl;
    return;
  }

  assert(role->get_type() != R_CLIENT);

  // Choose random name if -name was not specified for this player.
  char *nameNonPc = 
    strlen(humanNames[h]) ? 
    humanNames[h] :
    intelNames[intelNamesIndices[h % INTEL_NAMES_NUM]];
  
  // Give infinite lives for UIsettings::EXTENDED.
  // Not infinite lives for Uisettings::DUEL.
  int lives = style->human_initial_lives();

  if (infiniteLives) {
    lives = IT_INFINITE_LIVES;
  }
  
  int *shLives = NULL;
  if (cooperative) {
    // Humans will share Game::sharedLives for the number of lives.
    shLives = &sharedLives;
  }
  
  HumanP human = new Human(&world,&locator,nameNonPc,lives,shLives,
                           h % Xvars::HUMAN_COLORS_NUM);
  
  assert(human);
  style->set_human_data(human,&world,&locator);
  
  locator.register_human(human);
  if (ui && h < role->num_viewports_needed()) {
    ui->register_intel(h,human);
  }
  
  // Actual objects for humans.
  PhysicalP obj = human_physical(h);
  // human_physical now adds to locator.
  obj->set_intel(human);
  
  role->human_created(this,human,h,&world,&locator);

// No longer necessary to add a team for each human.
}



#if WIN32
void Game::change_screen_mode(Xvars::SCREENMODE p_newmode)
{
  if (!ui) {
    return;
  }
  ui->reset_graphics(p_newmode);
}
#endif



void Game::intro() {
  // If you are thinking about changing the text here, I suggest you contact 
  // the XEvil authors at satan@xevil.com first.
  // You may NOT change the copyright portion.
  // Even if you are making changes to XEvil, you do not have copyright 
  // for XEvil.

  
  // Put message in the status bar when the game starts up.
  // Use \n in string for locator.message_enq.
  ostrstream msg;
  msg 
    << "XEvil(TM) " << VERSION 
    << "  http://www.xevil.com  satan@xevil.com  " << XETP::versionStr << "\n"
    << "Copyright(C) 1994,2000 Steve Hardt and Michael Judge"
    << ends;
  locator.message_enq(msg.str());


  // Print message to standard out.  Doesn't really do anything on Windows.
  // Use endl for cout.
  cout 
    << "XEvil(TM) version " << VERSION << "  http://www.xevil.com  satan@xevil.com  " 
    << XETP::versionStr << endl
    << "Copyright(C) 1994,2000 Steve Hardt and Michael Judge" << endl
    << endl;  
  cout 
    << "XEvil is free software under the Gnu General Public License." << endl
    << "XEvil comes with absolutely no warranty." << endl;  
  cout 
    << "Type 'xevil -info' for license information and information on no warranty." << endl
    << "     'xevil -help' for usage and network-play instructions." << endl;
}



// Used in Game::print_stats().
Boolean Game::stats_creations_filter(const PhysicalContext* cx,void*) {
  return cx->statsCreations;
}



Boolean Game::stats_uses_filter(const PhysicalContext* cx,void*) {
  return cx->statsUses;
}



Boolean Game::stats_deaths_filter(const PhysicalContext* cx,void*) {
  return cx->statsDeaths;
}



// New way using Locator::filter_contexts.
void Game::print_stats()
{
  // Fucking HP compiler crashes.
#ifndef NO_SETPRECISION
  cout << setprecision(3);
#endif
  cout << endl
  << "-----------------------STATISTICS-----------------------" << endl;


  int contextsNum,n;
  const PhysicalContext *contexts[A_CLASSES_NUM];


  // Creations.
  contextsNum = 
    locator.filter_contexts(contexts,NULL,stats_creations_filter,NULL);

  for (n = 0; n < contextsNum; n++) {
    const Stats &stats = contexts[n]->get_stats(contexts[n]->arg);
    cout << contexts[n]->className << ":  created:  " 
    << stats.get_creations() << endl;
  }
  cout << endl;


  // Uses.
  contextsNum = 
    locator.filter_contexts(contexts,NULL,stats_uses_filter,NULL);

  for (n = 0; n < contextsNum; n++) {
    const Stats &stats = contexts[n]->get_stats(contexts[n]->arg);
    cout << contexts[n]->className << ":  used: " 
    << stats.get_uses() << endl;
  }
  cout << endl;


  // Deaths.
  contextsNum = 
    locator.filter_contexts(contexts,NULL,stats_deaths_filter,NULL);

  for (n = 0; n < contextsNum; n++) {
    const Stats &stats = contexts[n]->get_stats(contexts[n]->arg);
    cout << contexts[n]->className << ":  number killed: "
    << stats.get_deaths() 
    << "  average lifespan: " << stats.get_ave_lifespan() << " seconds" 
    << endl;
  }


  // Total for all Creatures.
  const Stats &creature = Creature::get_stats();
  cout 
  << "Total creatures killed: " 
  << creature.get_deaths() 
  << "  average lifespan: " << creature.get_ave_lifespan() << " seconds" 
  << endl << endl;


  // Figure this one out yourself.
  cout 
  << "Highest level: " << levelHighest << endl;
}



PhysicalP Game::create_enemy(PhysicalP obj,Boolean addToLocator) {
  if (!obj) {
    obj = enemy_physical();
  }

  IntelOptions ops;
  ITmask opMask = intel_options_for(ops,obj->get_class_id());

  ostrstream name;
  name << "Machine-" << (enemyNameCount++) << ends;
  EnemyP enemy = new Enemy(&world,&locator,name.str(),&ops,opMask);
  assert(enemy);
  delete name.str();
  locator.register_enemy(enemy);

  if (addToLocator) {
    locator.add(obj);
  }
  obj->set_intel(enemy);

  return obj;
}



ITmask Game::intel_options_for(IntelOptions &ops,ClassId classId) {
  ops.harmless = intelHarmless;

  ops.classFriends = style->class_friends();

  // Should be in the PhysicalContext.
  // REALLY should be in the PhysicalContext, I just got bit.
  ops.psychotic = 
    (classId == A_Enforcer || classId == A_Alien || 
     classId == A_FireDemon || classId == A_Dog || classId == A_Mutt ||
     classId == A_Dragon || classId == A_Yeti ||
     classId == A_RedHugger || classId == A_GreenHugger);
  
  return ITharmless | ITclassFriends | ITpsychotic;
}



void Game::kill_physical(PhysicalP p) {
  off_clock_kill(&locator,p);
}



void Game::world_changed() {
  locator.set_messages_ignore(False);
  if (ui) {
    ui->set_redraw_arena();
  }
#if WIN32
  start_soundtrack();
#endif
}



void Game::manager_reset(GameStyleType style) {
  if (ui) {
    ui->set_style(style);
    ui->reset();
  }
}



void Game::register_intel(int num,IntelP intel) {
  if (ui) {
    ui->register_intel(num,intel);
  }
}



void Game::humans_num_incremented() {
  // Perhaps should also be looking for when a connection is lost.

  // These are the states where it is legal to join in a game.
  if (state == getBearings ||
      state == gameOn ||
      state == levelExtra ||
      state == gameExtra || 
      state == awardBonus) {
    int h = locator.humans_registered();
    
    // This will push the new human to the appropriate connection.
    create_human_and_physical(h);
  }
}



void Game::humans_num_decremented(const IntelId &iId) {
  IntelP intel = locator.lookup(iId);
  if (!intel) {
    return;
  }

  // Why are we using Locator::explicit_die() instead of 
  // Game::off_clock_kill().  Should be consistent about this.

  // Find and kill the physical for the human and all of its 
  // followers, chainsaws, protection, etc.
  Id id = intel->get_id();
  PhysicalP p = locator.lookup(id);
  if (p) {
    PtrList list;
    p->get_followers(list);
    locator.explicit_die(p);
    for (int n = 0; n < list.length(); n++) {
      PhysicalP follower = (PhysicalP)list.get(n);
      locator.explicit_die(follower);
    }
  }

  locator.unregister_human(iId);
}



void Game::quit() {
  if (role) {
    role->game_quitting();
  }
  quitGame = True;
}



void Game::set_humans_playing(int val) {
  if (ui) {
    ui->set_humans_playing(val);
  }
}



void Game::set_enemies_playing(int val) {
  if (ui) {
    ui->set_enemies_playing(val);
  }
}



Quanta Game::manager_get_quanta() {
  return quanta;
}



GameStyleType Game::get_game_style_type() {
  assert(style);
  return style->get_type();
}



void Game::demo_setup() {
  // Choose between different demos.
  switch (Utils::choose(8)) {
    case 0: { // A bunch of Heros and an Alien.
    	for (int n = 0; n < 10; n++) {
        ostrstream name;
        name << "Enemy-" << n << ends;
        IntelOptions ops;
        ops.harmless = True;
        EnemyP enemy = new Enemy(&world,&locator,name.str(),
			         &ops,ITharmless);
        assert(enemy);
        delete name.str();
        locator.register_enemy(enemy);

        Pos pos = world.empty_rect(Hero::get_size_max());
        PhysicalP obj = new Hero(&world,&locator,pos);
        assert(obj);
        locator.add(obj);
        obj->set_intel(enemy);
  	  }

      IntelOptions ops;
	    ops.psychotic = True;
	    EnemyP enemy = new Enemy(&world,&locator,"killer",&ops,ITpsychotic);
	    assert(enemy);
	    locator.register_enemy(enemy);
	    Pos pos = world.empty_rect(Alien::get_size_max());
	    PhysicalP obj = new Alien(&world,&locator,pos);
	    assert(obj);
	    locator.add(obj);
	    obj->set_intel(enemy);
    }
    break;

    
    case 1: { // Hero, FThrower, and a bunch of Frogs (does not mean Frenchmen).
	    for (int n = 0; n < 15; n++) {
	      ostrstream name;
	      name << "Enemy-" << n << ends;
	      IntelOptions ops;
	      ops.psychotic = Utils::coin_flip();
	      EnemyP enemy = new Enemy(&world,&locator,name.str(),
					     &ops,ITpsychotic);
	      assert(enemy);
	      delete name.str();
	      locator.register_enemy(enemy);
	      
	      Pos pos = world.empty_rect(Frog::get_size_max());
	      PhysicalP obj = new Frog(&world,&locator,pos);
	      assert(obj);
	      locator.add(obj);
	      obj->set_intel(enemy);
      }

	    EnemyP enemy = new Enemy(&world,&locator,"killer",NULL,0);
	    assert(enemy);
	    locator.register_enemy(enemy);
	    Pos pos = world.empty_rect(Hero::get_size_max());
	    PhysicalP obj = new Hero(&world,&locator,pos);
	    assert(obj);
	    locator.add(obj);
	    obj->set_intel(enemy);
	    
      //	pos = world.empty_rect(FThrower::get_size_max());
	    PhysicalP fThrower = new FThrower(&world,&locator,pos);
	    locator.add(fThrower);
    }
    break;


    case 2: { // A bunch of Enforcers.
      for (int n = 0; n < 10; n++)
	      {
	        ostrstream name;
	        name << "Enemy-" << n << ends;
	        IntelOptions ops;
	        ops.classFriends = False;
	        ops.psychotic = True;
	        EnemyP enemy = new Enemy(&world,&locator,name.str(),
					       &ops,ITclassFriends|ITpsychotic);
	        assert(enemy);
	        delete name.str();
	        locator.register_enemy(enemy);
	        
	        Pos pos = world.empty_rect(Enforcer::get_size_max());
	        PhysicalP obj = new Enforcer(&world,&locator,pos);
	        assert(obj);
	        locator.add(obj);
	        obj->set_intel(enemy);
	      }
    }
    break;


    case 3: { // A bunch of Ninjas and a chainsaw.
      for (int n = 0; n < 10; n++) {
	      ostrstream name;
	      name << "Enemy-" << n << ends;
	      IntelOptions ops;
	      ops.classFriends = False;
	      EnemyP enemy = new Enemy(&world,&locator,name.str(),
					     &ops,ITclassFriends);
	      assert(enemy);
	      delete name.str();
	      locator.register_enemy(enemy);
	      
	      Pos pos = world.empty_rect(Ninja::get_size_max());
	      PhysicalP obj = new Ninja(&world,&locator,pos);
	      assert(obj);
	      locator.add(obj);
	      obj->set_intel(enemy);
      }
      Pos pos = world.empty_rect(Chainsaw::get_size_max());
      PhysicalP obj = new Chainsaw(&world,&locator,pos);
      locator.add(obj);
    }
    break;

    
    case 4: { // drop weights 
      Dim worldDim= world.get_dim();

      // x is in title space
      for (int x = WSQUARE_WIDTH * 1;
           // 2 = 1 for the last wsquare plus 1 for the weight/bomb itself
           x < (worldDim.colMax - 1) * WSQUARE_WIDTH; 
           x += WSQUARE_WIDTH) {
        Pos pos(x,2 * WSQUARE_HEIGHT);
        Pos pos2(x,WSQUARE_HEIGHT);
        PhysicalP p = new Weight(&world,&locator,pos);
        PhysicalP p2 = new Bomb(&world,&locator,pos2);

        // Vel vel(0,-13);
        // ((MovingP)p)->set_vel_next(vel);
        // ((MovingP)p2)->set_vel_next(vel);
        locator.add(p);
        locator.add(p2);
        // moved after the Locator::add(), hardts
        ((ItemP)p2)->use(NULL);
      }	    

      for (int m = 0; m < 10; m++) {
        ostrstream name;
        name << "Enemy-" << m << ends;
        EnemyP enemy = new Enemy(&world,&locator,name.str(),
                                 NULL,ITnone);
        assert(enemy);
        delete name.str();
        locator.register_enemy(enemy);
        
        Pos pos = world.empty_rect(Hero::get_size_max());
        PhysicalP obj;
	      switch (Utils::choose(3)) {
	      case 0:
	        obj = new Ninja(&world,&locator,pos);
	        break;
	      case 1:
	        obj = new Hero(&world,&locator,pos);
	        break;
	      case 2: {
            obj = new Frog(&world,&locator,pos);
	      }
          break;
	      };
	      assert(obj);
	      locator.add(obj);
	      obj->set_intel(enemy);
      }
    }
    break;


    case 5: { // Ninjas and ChopperBoys.
      int n;
      for (n = 0; n < 10; n++) {
	      ostrstream name;
	      name << "Enemy-" << n << ends;
	      IntelOptions ops;
	      ops.classFriends = False;
	      EnemyP enemy = new Enemy(&world,&locator,name.str(),
					     &ops,ITclassFriends);
	      assert(enemy);
	      delete name.str();
	      locator.register_enemy(enemy);
	        
	      PhysicalP obj;
	      Pos pos;
	      if (n % 2) {
	        pos = world.empty_rect(Ninja::get_size_max());
	        obj = new Ninja(&world,&locator,pos);
        }
        else {
	        pos = world.empty_rect(ChopperBoy::get_size_max());
	        obj = new ChopperBoy(&world,&locator,pos);
        }
        assert(obj);
	      locator.add(obj);
	      obj->set_intel(enemy);
	    }

      for (n = 0; n < 5; n++) {
	      Pos pos = world.empty_rect(Launcher::get_size_max());
	      PhysicalP obj = new Launcher(&world,&locator,pos);
	      assert(obj);
	      locator.add(obj);
      }

    }
    break;

    case 6: { // pack o' dogs
      int n;
      locator.add_team(Scenarios::class_team,(void*)A_Dog,NULL);

      // Don't use create_enemy() as that eventually calls Scenarios::intel_options_for()
      // and the scenario isn't set up yet.

      for (n = 0; n < 9; n++) {
        ostrstream name;
        name << "Dog-" << n << ends;
        EnemyP intel = new Enemy(&world,&locator,name.str(),NULL,ITnone);
        assert(intel);
        delete name.str();
        locator.register_enemy(intel);

        Pos pos = world.empty_rect(Dog::get_size_max());
        PhysicalP obj = new Dog(&world,&locator,pos);
        assert(obj);
        locator.add(obj);
        obj->set_intel(intel);
  	  }

      for (n = 0; n < 3; n++) {
        ostrstream name;
        name << "Enemy-" << n << ends;
        IntelOptions ops;
        ops.harmless = True;
        EnemyP enemy = new Enemy(&world,&locator,name.str(),&ops,ITharmless);
        assert(enemy);
        delete name.str();
        locator.register_enemy(enemy);

        PhysicalP obj;
        if (n == 0) {
          Pos pos = world.empty_rect(Ninja::get_size_max());
          obj = new Ninja(&world,&locator,pos);
        }
        else {
          Pos pos = world.empty_rect(Hero::get_size_max());
          obj = new Hero(&world,&locator,pos);
        }
        assert(obj);
        locator.add(obj);
        obj->set_intel(enemy);
  	  }
    }
    break;


    // One dragon.
    case 7: {
      // So dragons don't collide.      
      locator.add_team(universal_team,NULL,NULL);

      for (int n = 0; n < 1; n++) {
        Pos pos = world.empty_rect(Dragon::get_size_max());
        PtrList ret;
        Segmented::create_and_add_composite(ret,&world,&locator,Dragon::SEGMENTS_NUM,pos,
                                            Dragon::create,NULL);
        for (int m = 0; m < ret.length(); m++) {
          ostrstream name;
          name << "Enemy-" << n << ends;
          IntelOptions ops;
          ops.harmless = True;
          EnemyP enemy = new Enemy(&world,&locator,name.str(),&ops,ITharmless);
          assert(enemy);
          delete name.str();
          locator.register_enemy(enemy);
          PhysicalP p = (PhysicalP)ret.get(m);
          p->set_intel(enemy);
          
          // Make 'em a bit faster.
          ModifierP modifiers = ((CreatureP)p)->get_modifiers();
          assert(modifiers);
          ModifierP ds = new DoubleSpeed();
          modifiers->append_unique(ds);
        }
      }      
    }
    break;


    default:
      assert(0);
  } // switch
}



Boolean Game::universal_team(LocatorP,PhysicalP,PhysicalP,void *) {
  return True;
}
  


void Game::off_clock_kill(LocatorP /*locator*/,PhysicalP p) {
  // Kill off a Physical.  Normally, during Locator::clock() we just
  // call p->kill_self().  But, we need this extra stuff for it to
  // work outside of Locator::clock(). 
  //
  // Looking at this code a second time it makes me a bit nervous, 
  // We don't call Locator::explicit_die.  So, the object exists in 
  // the Locator for another turn and, I believe, gets nuked in 
  // Locator::delete_dead().  Don't like the idea of the object 
  // existing for another turn.  Should always use 
  // Game::off_clock_kill() or always Locator::explicit_die(), not
  // some combination.

  p->set_quiet_death();

  {
    const Area &area = p->get_area();
    const Area &areaNext = p->get_area_next();
    if (!(area == areaNext)) {
      // Surpress warning message.  Really should look into why this 
      // happens.  Does happen for Chainsaw.
#if 0
      cerr << "off_clock_kill(1), area does not equal area next for a " 
           << p->get_class_name()
           << endl;
#endif
    }
  }
  if (!p->die_called()) {
    p->kill_self();
    p->die();
  }
  // In case die modifies the next variables.
  p->hack_update_area_next();
  {
    const Area &area = p->get_area();
    const Area &areaNext = p->get_area_next();
    if (!(area == areaNext)) {
      cerr << "off_clock_kill(2), area does not equal area next for a " 
           << p->get_class_name()
           << endl;
    }
  }
}



/***    

  '*' means set in humans_team()
  '1' means set in one_human_team()
  
                                           P2

P1              |Human | Slave to P1 | Slave to P1's master | Slave to some human, not P1
                |      |             |                      | or p1's master
                +------+-------------+----------------------+---------------------------+
Human           | *    |  * 1        |        null          |     *                     |
                +------+-------------+----------------------+---------------------------+
Slave to P2     | * 1  |  null       |        null          |     null                  |
                +------+-------------+----------------------+---------------------------+
Slave to P2's   |  null|  null       |        * 1           |     null                  |
master          |      |             |                      |                           |
                +------+-------------+----------------------+---------------------------+
Slave to some   |      |  null       |        null          |                           |
other human, not| *    |             |                      |      *                    |
P2 or P2s master|      |             |                      |                           |
                +------+-------------+----------------------+---------------------------+
***/

Boolean Game::humans_team(LocatorP l,PhysicalP p1,PhysicalP p2,void *) {
  // Both are Human.
  IntelP p1Intel = p1->get_intel();
  IntelP p2Intel = p2->get_intel();
  if (!p1Intel || !p2Intel) {
    return False;
  }
  
  if (p1Intel->is_human() && p2Intel->is_human()) {
    return True;
  }

  Boolean p1HumanSlave = False;
  if (!p1Intel->is_human()) {
    IntelId masterId = ((MachineP)p1Intel)->get_master_intel_id();
    IntelP master = l->lookup(masterId);
    if (master && master->is_human()) {
      p1HumanSlave = True;
    }
  }

  Boolean p2HumanSlave = False;
  if (!p2Intel->is_human()) {
    IntelId masterId = ((MachineP)p2Intel)->get_master_intel_id();
    IntelP master = l->lookup(masterId);
    if (master && master->is_human()) {
      p2HumanSlave = True;
    }
  }

  if (p1Intel->is_human() && p2HumanSlave) {
    return True;
  }

  if (p1HumanSlave && p2Intel->is_human()) {
    return True;
  }

  if (p1HumanSlave && p2HumanSlave) {
    return True;
  }
  
  return False;
}



// Human and his slaves are on the same team.
Boolean Game::one_human_team(LocatorP,PhysicalP p1,PhysicalP p2,void *) {
  IntelP p1Intel = p1->get_intel();
  IntelP p2Intel = p2->get_intel();
  if (!p1Intel || !p2Intel) {
    return False;
  }

  // P1 is Human, P2 is P1's slave.
  if (p1Intel->is_human()) {
    if (!p2Intel->is_human()) {            
      if (((MachineP)p2Intel)->get_master_intel_id() == 
          p1Intel->get_intel_id()) {
        return True;
      }
    }
  }

  // P2 is Human, P1 is P2's slave.
  if (p2Intel->is_human()) {
    if (!p1Intel->is_human()) {            
      if (((MachineP)p1Intel)->get_master_intel_id() == 
          p2Intel->get_intel_id()) {
        return True;
      }
    }
  }

  // P1 and P2 are slaves of the same master.
  if (!p1Intel->is_human() && !p2Intel->is_human()) {
    if (((MachineP)p1Intel)->get_master_intel_id() ==
        ((MachineP)p2Intel)->get_master_intel_id()) {
      return True;
    }
  }

  return False;
}



#if WIN32
int
Game::getWindowList(CTypedPtrList<CPtrList,CWnd *>&p_windowlist) {
  if (ui) {
    return ui->getWindowList(p_windowlist);
  }
  else {
    return -1;  // means no UI
  }
}
#endif



void
Game::play_sounds()
{
#if WIN32
  SoundEvent t_req;
  int t_pan=0;
  int t_vol;

  Pos t_pos=soundManager.getKeyPosition(0);
  for (int i=0;i<MAX_CHANNELS;i++)
  {
      t_req=soundManager.getEvent(i);
			if (t_req.m_init==True)
			{
				if (soundManager.getNumKeyPositions()>1)
				{
					soundManager.playSoundById(t_req.soundid,0,0,False);
				}
				else
				{
					t_pan=(int)(((float)t_req.position.x-(float)t_pos.x) /
							        (float)(((float)W_ROOM_COL_MAX/2)*(float)WSQUARE_WIDTH)*(float)10000);
//					t_pan=(int)((float)t_req.position.x-(float)t_pos.x)
//							/(float)(((float)W_ROOM_COL_MAX/2)*(float)WSQUARE_WIDTH ) *(float)10000;
					if (t_pan>10000)
						t_pan=10000;
					if (t_pan< -10000)
						t_pan= -10000;
					if (t_pan>2000)
						t_pan=10000;
					if (t_pan<-2000)
						t_pan=-10000;
					t_vol= (int)(-1000*  (t_req.position.distance(t_pos))/ (((float)W_ACROSS_MAX_MAX*2)*(float)WSQUARE_WIDTH ));
					if (t_vol>0)
						t_vol=0;
					soundManager.playSoundById(t_req.soundid,t_pan,t_vol,False);
				}
			}
  }
#endif
}


#if WIN32
void
Game::start_soundtrack() {
  switch (musictype) {
    case UIsettings::NONE : 
    break;

    case UIsettings::WAV : {
// WAV files disabled.
#if 0
      soundManager.stopSound(currentSoundName);
      currentSoundName = style->get_soundtrack();      
      soundManager.playSound(currentSoundName,0,0,True,True);      
#endif
    }
    break;

    case UIsettings::CD : {
      if (soundManager.getTrackCount()) {
        int t_choose=Utils::choose(soundManager.getTrackCount());
        soundManager.stopCD();
        soundManager.requestCDtrack(t_choose,1);
      }
    }
    break;

    case UIsettings::MIDI : {
      soundManager.stopMIDI();
      if (!mididirectory.GetLength()) {
        currentSoundName = style->get_midisoundtrack();      
        soundManager.playMidi(currentSoundName,TRUE,1);
      }
      else {
        soundManager.requestRandomMIDItrack(mididirectory,TRUE,1);
      }
    }
    break;

    default :
    break;
  }
}



void Game::stop_soundtrack() {
  switch (musictype) {
    case UIsettings::CD :   
      soundManager.stopCD();
      break;
    case UIsettings::WAV :  
      soundManager.stopSound(currentSoundName);
      break;
    case UIsettings::MIDI : 
      soundManager.stopMIDI();
      break;
  }
}
#endif



int Game::limit_humans_num(int h) {
  int ret = Utils::minimum(h,Locator::HUMANS_MAX);

  // Can only have more humans than viewports if we are a server.
  if (role->get_type() != R_SERVER) {
    ret = Utils::minimum(ret,UI_VIEWPORTS_MAX);
  }
  return ret;
}
