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

// "locator.h" The object locator.  


/* Overview: 
The locator can be seen as the collection of all physical objects in
the game.  It draws all objects with the draw method.  The clock
method goes through all the phases of one turn for all objects.  This
is the only entity in the game that has a list of all the objects.
Objects can be mapped and/or collidable.  An unmapped object is
neither drawn nor collided.  An uncollidable object is drawn (if
mapped) but not collided.  There is a message queue that objects can
use to communicate with the ui object.  The locator does not need to
be clocked for the queue to work. 
The locator grid is the maximum size of the world.  I.e. it can be larger than
the world. */


#ifndef LOCATOR_H
#define LOCATOR_H

#if X11
	#ifndef NO_PRAGMAS
	#pragma interface
	#endif
#endif


// Include Files
#include <iostream.h>

#include "utils.h"
#include "coord.h"
#include "id.h"
#include "world.h"
#include "sound_cmn.h"
#include "xdata.h"

class Intel;
typedef Intel *IntelP;
class Physical;
typedef Physical *PhysicalP;
struct PhysicalContext;
class Human;
typedef Human *HumanP;
class Enemy;
typedef Enemy *EnemyP;
class Neutral;
typedef Neutral *NeutralP;
class SoundManager;
class SoundRequest;



// Maximum number of objects in the game.
// Should replace this static size array with the new PtrList class.
#define OL_LIST_MAX 2000 

#define OL_NEARBY_MAX OL_LIST_MAX
#define OL_MESSAGES_MAX 300


// Details of the object grid, each grid cell will have size 
// (OL_GRID_COL_MAX,OL_GRID_ROW_MAX) in WSQUARES.
#define OL_GRID_COL_MAX 4
#define OL_GRID_ROW_MAX 4
#define OL_GRID_WIDTH (OL_GRID_COL_MAX * WSQUARE_WIDTH) // In pixels.
#define OL_GRID_HEIGHT (OL_GRID_ROW_MAX * WSQUARE_HEIGHT) // In pixels.
#define OL_GRID_SIZE_MAX (max(OL_GRID_WIDTH,OL_GRID_HEIGHT))
#define OL_GRID_SIZE_MIN (min(OL_GRID_WIDTH,OL_GRID_HEIGHT))
// Max size of world in grid squares.
#define OL_GRID_HORIZ_MAX (W_COL_MAX_MAX / OL_GRID_COL_MAX + 1)
#define OL_GRID_VERT_MAX (W_ROW_MAX_MAX / OL_GRID_ROW_MAX + 1)


#define OL_RECENT_SOUNDS_MAX 30



// An entry for one object in the list of all objects currently in the game.
class OLentry {
  friend class Locator;
  friend class OLgridEntry; 
#if X11
  friend class OLshadowEntry;
#endif
  friend class PhysicalIter;

  
  // Only valid if mapped || flash.
  OLgridEntry *gridEntry; 
  // Set by OLgridEntry::insert.  Not nec. valid.
  GLoc gloc; 
#if X11
  // Only valid if mapped || flash.
  OLshadowEntry *shadowEntry; 
  // Set by OLshadowEntry::insert.  Not nec. valid.
  GLoc shadowGloc; 
#endif

  Boolean valid; 
  Boolean reserved; // Meaningful when !valid.
  PhysicalP physical; // Set when reserved.
  
  Boolean collided;
  Boolean mapped; // Redundant.  Must be same as in object. 
  Boolean flash; // Redundant.  Must be same as in object.
  Boolean collidable;

  // Used to control the drawing order for Composite objects, in draw_chain().
  // Only used one viewport at a time.
  Boolean alreadyDrawn; 
};



// The entry in the object grid, mostly a forwarding pointer to the OLentry
// for the object.
class OLgridEntry {
 public:
  OLgridEntry(OLentry *e);
  
  OLentry *get_entry() {return entry;}

  PhysicalP get_physical() {return entry->physical;}

  Boolean get_collided() {return entry->collided;}

  Boolean get_mapped() {return entry->mapped;}

  Boolean get_flash() {return entry->flash;}

#if X11
  OLshadowEntry *get_shadow_entry() {return entry->shadowEntry;}
#endif

  const Area &get_area(); 

#if X11
  // Used for the merge_draw_areas() algorithm.
  // Completely different than the alreadyDrawn flag in the OLEntry.
  Boolean get_was_drawn() {return wasDrawn;}

  void set_was_drawn(Boolean val) {wasDrawn = val;}
#endif

  int get_drawing_level();

  Boolean get_collidable() {return entry->collidable;}

  OLgridEntry *get_next() {return next;}

  void set_collided(Boolean val) {entry->collided = val;}
  
  void insert(OLgridEntry *grid[OL_GRID_VERT_MAX][OL_GRID_HORIZ_MAX],
              const GLoc &gl);

  void remove();

  
 private:
  OLgridEntry *prev, *next;
  OLentry *entry;

#if X11
  // Used for the merge_draw_areas() algorithm.
  // Completely different than the alreadyDrawn flag in the OLEntry.
  Boolean wasDrawn;
#endif
};



#if X11
// An entry in the shadow grid.  An object's shadow represents where it was
// last turn, so we can clean up junk left on the window when we draw this
// turn.  Only used with Locator::draw_buffered().  Shadow's don't mean much
// when we are redrawing the entire screen each time.
class OLshadowEntry {
 public:
  OLshadowEntry();

  OLshadowEntry(const Area &a,OLentry *e);

  Boolean get_orphaned() {return orphaned;}

  OLentry *get_entry() {assert(entry); return entry;}

  OLgridEntry *get_grid_entry() {assert(entry); return entry->gridEntry;}

  const Area &get_area() {return area;}

  Boolean draw_self() {return drawSelf;}

  OLshadowEntry *get_next() {return next;}
  /* NOTE: Several require that the shadowEntry is not orphaned. */

  Boolean get_was_drawn() {return wasDrawn;}

  void set_area(const Area &a) {area = a;}
  void set_draw_self(Boolean val) {drawSelf = val;}
  void set_orphaned() {orphaned = True; entry = NULL;}
  void set_was_drawn(Boolean val) {wasDrawn = val;}

  void insert(OLshadowEntry *shadows[OL_GRID_VERT_MAX][OL_GRID_HORIZ_MAX],
	      const GLoc &gl);
  void remove();


 private:
  Boolean orphaned;
  Area area;
  Boolean drawSelf;
  OLentry *entry;
  OLshadowEntry *prev, *next;
  // Used for the merge_draw_areas() algorithm.
  // Completely different than the alreadyDrawn flag in the OLEntry.
  Boolean wasDrawn;
};
//typedef 
#endif



// Iterator to extract all humans to be reincarnated.
class Incarnator {
public:
  Incarnator(Locator &l) {n = 0; locator = &l;}
  
  HumanP operator () ();
  /* EFFECTS: Extract next object to be reincarnated.  Return NULL if none.
     The returned HumanP can only be extracted once. */

  
private:
  int n;
  Locator *locator;
};



class PhysicalIter {
 public:
  PhysicalIter(Locator &l) {n = 0; locator = &l;}

  PhysicalP operator() ();
  /* EFFECTS: Yield all Physicals managed by the Locator. */
  /* REQUIRES: Must be called outside of Locator::clock.  The Locator must 
     not be modified while the PhysicalIter is being used. */


 private:
  int n;
  Locator *locator;
};



class ArenaMessageIter {
 public:
  ArenaMessageIter(Locator &l,const IntelId &i);
  /* NOTES: Only return messages that are visible to Intel i. */

  char *next(Boolean &propagate);
  /* MODIFIES: propagate */
  /* EFFECTS: Yield all arena messages in the order they would be returned by
     arena_message_deq().  Memory returned is still the property of Locator, 
     do not free or hold on to it. */
  /* REQUIRES: Don't modify Locator while iterating. */


 private:
  int ptr;
  Locator *locator;
  IntelId intelId;
};



struct ArenaMessage {
  char *msg;
  IntelId intelId; // Meaningful iff exclusive.
  Boolean exclusive; // Only show message to a specific viewport.
  Quanta time; // -1 means use default.
  Boolean propagate; // for Server, should this message go to all clients.
};



class TeamOptions {
  public:

  // Constructor give default values.
  TeamOptions() {
    membersDontCollide = True;
    deleteClosure = False;
  }

  Boolean deleteClosure;
  Boolean membersDontCollide;
};
typedef TeamOptions *TeamOptionsP;



// Used by Locator::draw_ticks() to render the tick marks.
// Not much point on Windows, but on UNIX we have two different 
// methods of drawing the tick marks.
//
// USE:  For each display
//         begin_draw();
//         draw_tick for each tick on the display
//         end_draw()
// Do not overlap drawing for different displays, different renderers.
class ITickRenderer {
public:
  virtual void begin_draw(int dpyNum) = 0;
  /* EFFECTS: Get ready to draw the tick marks for given display. */

  virtual void end_draw(int dpyNum) = 0;
  /* EFFECTS: Finished drawing all the tick marks, make sure they are
     visible on the screen. */

  virtual Boolean draw_tick(TickType tt,CMN_DRAWABLE window,
                            Xvars &xvars,int dpyNum,
                            const Size &arenaSize,
                            Dir tickDir,int offset) = 0;
  /* EFFECTS: Draw one tick mark.  The arena has size windowSize.  Offset is 
     X or Y depending on the direction given by tickDir. Return whether 
     successful.  All values are stretched. */
  /* NOTE: window does not necessarily have size arenaSize.  Window is 
     whatever was passed into Locator::draw_ticks(). */
};



// Internal to Locator.
// Each team is more of a "rule" that says that two Physicals are
// on the same team.
struct OLTeam {
  TeamId teamId;
  TeamOptions options;
  Boolean (*same_team)(LocatorP,PhysicalP,PhysicalP,void *);
  void *closure;
};



struct OLItemInfo {
  Boolean isItem;
  Boolean isWeapon;
  CMN_BITS_ID iconId;
};



class Locator: public ITickRenderer {
  friend class Incarnator;
  friend class PhysicalIter;
  friend class ArenaMessageIter;


 public:
  enum { 
    // HUMANS_MAX currently limited by number of available names.
    HUMANS_MAX = 70,  
    ENEMIES_MAX = 500, 
    NEUTRALS_MAX = 400, 
    DRAWING_LEVELS = 3,
    // For set_drawing_algorithm().
    DRAW_MERGE_AREAS,
    DRAW_NO_MERGE
  };

  Locator(WorldP world,int drawingAlgorithm,SoundManager *soundManager);
  ~Locator();
  /* EFFECTS: Create a new object locator with no managed objects. */

  static Boolean valid(const GLoc &gl) 
    {return ((gl.vert >= 0) && (gl.horiz >= 0) && 
	     (gl.vert < OL_GRID_VERT_MAX) && (gl.horiz < OL_GRID_HORIZ_MAX));} 
  /* EFFECTS:  Is gl a loc in the grid. */

  void add(PhysicalP p);
  /* REQUIRES: p is not already managed. */
  /* EFFECTS: p will be added to the list of managed physicals at the beginning
     of the next clock.  p is initially mapped.  canCollide is set from p.
     If called inside clock, object will not be added until beginning of next
     clock.  You should not add something twice.  p is given a valid Id 
     immediately (not at the next clock). */

  void add_as(PhysicalP p,const Id &);
  /* EFFECTS: Add as the given id.  Used by the Client. */

  void get_nearby(PhysicalP nearby[OL_NEARBY_MAX],int &nitems,
		  PhysicalP p,int radius);
  /* MODIFIES: nearby,nitems. */
  /* EFFECTS:  Returns all objects that have their middles within radius of 
     p's middle.  (I.e <= ) */  
  /* NOTE: Expensive. */

  PhysicalP lookup(const Id &id,Boolean recentAdditions = False);
  /* EFFECTS: Return the physical with the given id if it still exists, else
     NULL.  If recentAdditions is False, will return NULL if Locator has 
     not been clocked since the add(), i.e. the object is not in the grid 
     yet. */

  IntelP lookup(const IntelId &iId);
  /* EFFECTS: Return the human or machine (enemy or neutral) Intel for iId if 
     it exists, is registered, and is alive or reincarnating.  (Note that only 
     Humans can be reincarnating.)  Otherwise, return NULL. */

#if X11
  void draw_buffered(CMN_DRAWABLE window,Xvars &xvars,int dpyNum,
                     const Box &box);
  /* REQUIRES: win is the same size as Box, in WSQUAREs.  
     Either 1) window has not been changed since last call to draw_buffered.
     or     2) window has just been freshly draw with the world. */
  /* EFFECTS: Draw all objects overlapping box (world coordinates) onto 
     window.  Uses a floating-backbuffer algorithm, only drawing small patches
     that have changed.  (0,0) of window corresponds to upper-left of box in 
     the world. */     
#endif

  void draw_directly(CMN_DRAWABLE window,Xvars &xvars,int dpyNum,
                     const Area &area);
  /* REQUIRES: win is the same size as area, in pixels. */
  /* EFFECTS: Draw all objects overlapping area (world coordinates) directly
     onto window.  This method does no buffering.  Of course, the caller
     can use draw_directly() to implement its own back buffer.  (0,0) of 
     window corresponds to upper-left of area in the world. */     

  void draw_ticks(CMN_DRAWABLE window,Xvars &xvars,int dpyNum,
                  const Area &area, const Id &id,
                  ITickRenderer* renderer);
  /* EFFECTS: Draw tick marks for all objects near id.  Use renderer to do 
     the actual drawing.  area is in world coordinates. */
  
  void reset();
  /* EFFECTS: Prepare for a new game, kill all Physicals, Intels, Teams,
     Humans, Enemies, Neutrals, and unprocessed messages. */

  void level_reset();
  /* EFFECTS: Right now, this just cleans out the non-persistent teams. */

  void clock();

  void client_pre_clock();
  void client_post_clock();
  /* EFFECTS: Clock everything except the objects.  Used by Client. 
     The client may change objects between pre and post clock. */


  //---------------------- Message Service ------------------------//
  void message_enq(char *msg);
  /* EFFECTS:  Enqueue msg in the queue of messages for the ui.  Locator will 
     free msg when it is done with it. Message will appear in the 
     message bar at the bottom. */

  char *message_deq();
  /* EFFECTS: Dequeue the next message off the appropriate message queue or 
     return NULL if the queue is empty. */

  char *peek_most_recent_message();
  /* EFFECTS: Return the message most recently enqued.  Memory is still the 
     property of the Locator and may be deleted later.  Return NULL if no
     messages in the queue. */

  void arena_message_enq(char *msg,IntelId *intelId = NULL,
                         Quanta time = -1,Boolean propagate = True);
  /* EFFECTS: Put msg in the queue to be drawn on the Arena, shoved in the
     user's face.  intelId is non-NULL if only that specific viewport should
     display the message.  If time is not -1, it gives the number of clock
     ticks for which the message should be displayed.  propagate indicates 
     whether this message should be propagated to clients if playing 
     over a network. */

  void arena_message_enq(char *msg,PhysicalP p);
  /* EFFECTS: Convience function.  If p has an intel, enq a message for the
     intel.  Else, throw message away. */

  Boolean arena_message_deq(char **msg,IntelId &intelId,
                            Quanta &time,Boolean &propagate);
  /* MODIFIES: msg, intelId, time, propagate */
  /* EFFECTS: Dequeue the next message off the appropriate message queue or 
     return NULL if the queue is empty.  If returns TRUE, only set the message
     for the viewport of the given IntelId.  On FALSE, set on all 
     viewports.  Time will be set to the number of clock ticks for which to
     display the message, or -1 if the default should be used. */

  void set_messages_ignore(Boolean msgIg) {messagesIgnore = msgIg;}
  /* EFFECTS: While set to True, all enques messages will be immediately 
     discarded.  Default is False. */

  void clear_all_messages();
  /* EFFECTS: Wipes out all messages in both message queues. */
  //---------------------- Message Service ------------------------//


  void set_drawing_algorithm(int alg) {drawingAlgorithm = alg;}
  /* EFFECTS: Choose between  DRAW_MERGE_AREAS and DRAW_NO_MERGE. */

  void set_draw_rects(Boolean val) {drawRects = val;}
  /* EFFECTS: Draw rectangles around areas when copying to screen, for
     debugging drawing algorithm. */

  // Player registry service.  Takes responsibility for reincarnation.
  void register_human(HumanP); // Kept until reset.
  void register_enemy(EnemyP); // Deleted as needed.
  void register_neutral(NeutralP); // Deleted as needed.

  void register_human_as(HumanP); // Kept until reset.
  /* EFFECTS: Register human using existing intelId of human.  Used by 
     Client. */

  void unregister_human(const IntelId &);
  
  int humans_playing();
  int enemies_alive();
  int neutrals_alive();
  
  int humans_registered();
  /* IMPLEMENTATION NOTE: No longer the same as humansMax. */

  HumanP get_human(int n);
  /* REQUIRES: n < the number of humans registered */
  /* NOTE: Does not guarantee any particular order. */


  //---------------------- Recently Deleted Service ------------------------//
  // Recently deleted service also records recently unmapped objects. */

  void set_remember_deleted(Boolean);
  /* EFFECTS: Turn on remembering recently deleted objects.  Must be called 
     before using any of these functions.  If turned on, must keep calling
     reset_recently_deleted() or the delList will overflow. */
  
  int get_recently_deleted_num() {assert(useDelList); return delNum;}
  
  Id get_recently_deleted(int val) 
  {assert(useDelList && val >= 0 && val < delNum); return delList[val];}

  void reset_recently_deleted() {assert(useDelList); delNum = 0;}
  /* EFFECTS: Gives all the objects that were just deleted in the last 
     turn.  Call reset_just_deleted() to start counting for next time. */
  /* NOTE: The reason we don't just do this in Locator::clock() is that 
     clock() gets called in funny places a few times to get objects inserted
     at the right time. */
  //---------------------- Recently Deleted Service ------------------------//


  //------------------------ Recent Sounds Service -------------------------//
  void set_remember_sounds(Boolean);
  /* EFFECTS: Turn on remembering recently sent sounds.  Must be called 
     before using any of these functions.  If OL_RECENT_SOUNDS_MAX is exceeded,
     will start dropping sounds. */
  
  int get_sounds_num() {assert(rememberSounds); return soundsNum;}
  
  void get_sound(SoundRequest &request,int index) 
  {assert(rememberSounds && index >= 0 && index < soundsNum); 
  request = sounds[index];}
  /* MODIFIES: request */

  void reset_sounds() {assert(rememberSounds); soundsNum = 0;}
  /* We could do this in Locator::clock() as it is not that big a deal if we 
     lose a sound or two. */
  //------------------------ Recent Sounds Service -------------------------//


  TeamId add_persistent_team(Boolean (*same_team)(LocatorP,PhysicalP,
                                                  PhysicalP,void *),
                             void *closure,TeamOptionsP);
  /* EFFECTS: Persistent teams will last through Locator::level_reset(), 
     will only be removed at Locator::reset(). */
  
  TeamId add_team(Boolean (*same_team)(LocatorP,PhysicalP,PhysicalP,void *),
                  void *closure,TeamOptionsP);
  /* EFFECTS: Add new team defined by the member function.  Teams are not 
     mutually exclusive, a Physical can belong to multiple teams.  All
     non-persistent teams are cleared out at reset() or level_reset(). */
  /* NOTE: TeamId is no longer used, since we shuffle the arrays around in 
     delete_persistent_team(). */

  Boolean same_team(PhysicalP p1,PhysicalP p2)
  {TeamOptions dummy; return same_team(dummy,p1,p2); }
  /* EFFECTS: Are the two Physicals on the same team, for any of the teams. */

  CMN_DRAWABLE get_scratch_buffer(Pos &pos,Xvars&,int dpyNum);
  /* MODIFIES: pos */
  /* EFFECTS: Return the scratch buffer.  Set pos to the offset in the
     returned CMN_DRAWABLE where the scratch area exists.  On UNIX, this is
     always (0,0).
     The scratch buffer is exported by the locator for any drawing tricks
     that objects may want to use to draw themselves.  E.g. Cloaks use it. 
     Guaranteed to be as large as the largest object. */
  /* TODO: CMN_DRAWABLE on Windows should be made to include the Pos.  Or make
     a new CMN_* type. */

  Boolean submitSoundRequest(SoundRequest req);
  

  //----------------------- Class Registry Service -------------------------//
  // Note: To iterate over all classes, you loop from 0 to 
  // A_CLASSES_NUM, calling get_context() on each.  Make sure to test for 
  // NULL.
  const PhysicalContext *get_context(ClassId classId);
  /* EFFECTS: Return the PhyscialContext of the class with the given 
     ClassId. */
  
  int filter_contexts(const PhysicalContext* contextList[A_CLASSES_NUM],
		      ClassId idList[A_CLASSES_NUM],
		      Boolean (* filter)(const PhysicalContext*,void * closure),
              void *closure);
  /* MODIFIES: contextList, idList */
  /* EFFECTS: Create a list of all the classes/ids of object for which filter
     returns True.  list should be an array of size A_CLASSES_NUM.  
     Returns the size of list.  Either of contextList or idList can be NULL,
     if one is, it will not be filled in. */

  void get_item_info(Boolean &isItem,Boolean &isWeapon,
                     CMN_BITS_ID &iconId,ClassId cId);
  /* MODIFIES: isItem, isWeapon, bitsId */
  /* EFFECTS: Gives weapon/item info about the class corresponding to cId.
     Don't just use get_context(), because that won't work for "non-existent" 
     classes like A_HandToHand.  get_item_info() will handle them. isItem and 
     isWeapon tell whether the class derives from Item and Weapon, 
     respectively.  iconId gives an icon appropriate for displaying in a 
     status window (WIN32 only for now). 
     iconId will only be meaninful if isItem or isWeapon is True. */
  //----------------------- Class Registry Service -------------------------//


  void explicit_die(PhysicalP p) {del_now(p);}
  /* EFFECTS: Kill all traces of p from the Locator, and delete p. */
  /* NOTE: Will delete enemy or neutral associated with p if registered 
     with the Locator. */

  void init_x(Xvars &,IXCommand,void*);
  /* EFFECTS: Initialize or reinitialize all graphics info */

  TickType compute_tick_type(PhysicalP p,IntelP relativeTo);
  /* EFFECTS: What type of tick, if any, should be displayed for this Physical.
     The tick type of p from the perspective of "relativeTo".
     Return TICK_MAX if none. */

#if WIN32
  Boolean generate_xpm(Xvars& xvars,const char* genDir,Boolean halfSize);
  /* EFFECTS: Output all bitmaps as XPM files to the directory 
     (genDir + "world/").  Return if success. */
#endif


  //----------------------- ITickRenderer -------------------------//
  // The locator implements ITickRenderer by drawing tick marks on the
  // inside edges of the window passed to draw_tick().  This is the only
  // implementation on WIN32.  On UNIX, there is another implementation that
  // draws tick marks in the border around the window.
  //
  // NOTE: We should really move this implementation of 
  // ITickRenderer into the Viewport object (UiPlayer on Win32).

  virtual void begin_draw(int);

  virtual void end_draw(int);

  virtual Boolean draw_tick(TickType tt,CMN_DRAWABLE window,Xvars &xvars,int dpyNum,
                            const Size &windowSize,
                            Dir tickDir,int offset);
  //----------------------- ITickRenderer -------------------------//


  
private:
  enum {
    REINCARNATE_TIME = 70, 
    TEAMS_MAX = 3, // Arbitrary, don't need a team for each human anymore.
  }; 

  void add_now(PhysicalP);
  void del_now(PhysicalP);

  Boolean same_team(TeamOptions &ops,PhysicalP p1,PhysicalP p2);
  /* MODIFIES: ops */
  /* NOTES: Private version of same_team(), except returns the TeamOptions of 
     the team if found. May set ops even if False is returned. */

  void allign_flash_and_mapped(int n);
  /* EFFECTS: Checks to see if physical n has changed its mapped or flash 
     attribute.  If so, adjust the mapped attribute in *this. */
  /* REQUIRES: n is a valid list index. */

  Id reserve_list_entry(PhysicalP p);
  /* EFFECTS: Reserve an entry in the list for p and return an Id for the
     entry. */
  
  void collision_checks();
  /* EFFECTS: Do collision checks between all registered objects. */

  GLoc compute_gloc(PhysicalP);
  GLoc compute_gloc(const Area &);
  /* EFFECTS: Compute the location in the grid for a physical or an area. */
  /* NOTE: Ok to return invalid gloc. */

#if X11
  void merge_draw_area(CMN_DRAWABLE window,Xvars &xvars,int dpyNum,
		       const Box &room,const Area &area,const GLoc &gloc);
  /* REQUIRES: area is centered at gloc. window is the size of room*/  
  /* EFFECTS: Crux of the DRAW_MERGE_AREAS drawing algorithm.  window 
     corresponds to the room.  Opportunistically merge area with any nearby
     areas that need to be drawn, enlarging area to encompass them.  Then
     draw the merged area and mark the was_rawn() flag. */

  void draw_area(CMN_DRAWABLE window,Xvars &xvars,int dpyNum,const Box &room,
                 const Area &area,const GLoc &gloc);
  /* REQUIRES: area is centered at gloc. window is the size of room*/
  /* EFFECTS: Draws world and all objects overlapping area. */
  /* NOTE: box does not have to be valid. */
#endif

  void draw_chain(CMN_DRAWABLE window,Xvars &xvars,int dpyNum,
                  const Area &area,PhysicalP p);
  /* EFFECTS: Draw p and any objects that must be drawn before it.  
     e.g. Composite objects. */

  Boolean possible_overlap(const Area &area,const GLoc &gloc);
  /* EFFECTS: Returns whether area could overlap any of the areas in gloc. */

  void delete_team_data(OLTeam &);
  /* NOTE: Only frees team data, does not affect size of list. */

  // Class registry service.
  void register_contexts();
  void register_context(const PhysicalContext *context); 
  /* EFFECTS: All classes of Physical objects must be registered with
     register_context in order to be seen by the Game. */


  //// phases of clock()
#if X11
  void kill_shadow_del_list();
  /* NOTE: Needed for the X11 drawing algorithm. */
  /* EFFECTS: Don't want to actually delete the shadows until the 
     beginning of the next turn and we are sure nothing is still
     pointing to them. */

  void shadows_follow_physicals();
  /* EFFECTS: The 'shadows' follow one turn behind the current location
     of the Physicals.  So, we know where the physical was last time
     to redraw the dirty areas. */
#endif

  void intelligence_and_act();
  /* EFFECTS: Intelligence phase and action phase.  Set next variables.  
     Externally visible state should not change (although there are
     some exceptions). */

  void kill_outside_world();
  /* EFFECTS: If a Physical gets knocked so far through the wall it ends up
     outside the world, kill it. */

  void kill_excess_objects();
  /* EFFECTS: If we get close to the maximum number of objects in the game,
     start killing some off. */
  /* NOTE: There is no real good reason for having a static fixed-size array 
     of objects in the Locator.  Now that we have PtrList available, we 
     should use it instead. */

  void check_for_death();
  /* EFFECTS: Check for death.  I.e. healthNext < 0. */
  /* NOTE: If using kill_self in this phase, an object must manually 
     call die and check for die_called(). */

  void update_phase();
  /* EFFECTS: Update externally visable state.  Set current variables to the 
     values of the next variables. */

  void grid_follows_physicals();
  /* EFFECTS: Update the locator grid to reflect the new positions of the 
     Physicals.  Shift gridEntries around as necessary. */

  void add_new_objects();
  /* EFFECTS: Add all objects on the add list, i.e. everything just added in
     this the previous act/update phase. */

  void delete_dead();
  /* EFFECTS: Actually remove the objects that are dead.  Don't want to do this
     until after the update phase. */

  void reincarnate_clock();
  /* EFFECTS: Clock all the reincarnate timers. */
  //// end phases of clock()


  void clear_already_drawn_flag(const GLoc &gridStart,const GLoc &gridFinish);
  /* EFFECTS: Clear the already_drawn() flag in all OLGridEntries in the
     grid squares between gridStart and gridFinish, exclusive. */

  void grid_covering_area(GLoc& start,GLoc& finish,const Area& area);
  /* MODIFIES: start, finish */
  /* EFFECTS: Set start and finish to be the corners of the rectangle of
     grid squares completely covering area.  I.e. loop from start.{vert,horiz}
     to < finish.{vert,horiz} */

#if WIN32
  static void* compute_tick_key(int tickType,Dir dir);
  /* REQUIRES: Dir should be a "pure" direction, not "half-pure". */
  /* EFFECTS: Compute the SurfaceManager key for storing the tick marks.  */     
#endif


  ///////////////// Data.
  WorldP world;
  SoundManager *soundManager;
  int uniqueGen;
  // All valid entries in the list have index < listMax.
  int listMax; 
  OLentry list[OL_LIST_MAX];
  OLgridEntry* grid[OL_GRID_VERT_MAX][OL_GRID_HORIZ_MAX];
#if X11
  OLshadowEntry* shadows[OL_GRID_VERT_MAX][OL_GRID_HORIZ_MAX];
  OLshadowEntry* shadowDelList[OL_LIST_MAX];
  int shadowDelNum;
#endif

  PhysicalP addList[OL_LIST_MAX];
  int addNum;

  // Remember everything that has been deleted.
  Boolean useDelList;
  Id delList[OL_LIST_MAX];
  int delNum;

  // Remember recent sounds
  Boolean rememberSounds;
  SoundRequest sounds[OL_RECENT_SOUNDS_MAX];
  int soundsNum;

  XvarsValid xValid;
  OLxdata xdata;

  // Message service.
  char *messages[OL_MESSAGES_MAX];
  int head,tail; 
  ArenaMessage arenaMessages[OL_MESSAGES_MAX];
  int arenaHead,arenaTail; 
  Boolean messagesIgnore;

  // Player registry service.
  HumanP humans[HUMANS_MAX];
  EnemyP enemies[ENEMIES_MAX];
  NeutralP neutrals[NEUTRALS_MAX];
  int humansMax;
  int enemiesNum;
  int neutralsNum;
  Boolean reincarnating[HUMANS_MAX]; // Just for humans.
  Timer reincarnateTimers[HUMANS_MAX]; // Just for humans.

  // teams and persistent teams now independent.
  OLTeam teams[TEAMS_MAX];
  int teamsNum;
  OLTeam persistentTeams[TEAMS_MAX];
  int persistentTeamsNum; 

  int drawingAlgorithm;
  Boolean drawRects;

  // Arrows for tick marks
  const static CMN_BITS_ID tickPixmapBits[TICK_MAX][CO_DIR_HALF_PURE];
  // unstretched coordinates.
  const static Size tickSizes[CO_DIR_HALF_PURE];

  // contextCount is for verification purposes, make sure all the classes are
  // registered.
  int contextCount;

  // List of all classes of Physicals.  Indexed by the values for ClassId.
  // E.g. contexts[A_ChopperBoy] is the PhysicalContext for the class
  // ChopperBoy.
  const PhysicalContext *contexts[A_CLASSES_NUM];

  // Item info for the "non-existent" classes, like A_HandToHand.
  // Index starts at ClassId, A_CLASSES_NUM.
  const static OLItemInfo nonExistentClassesItemInfo[A_None - A_CLASSES_NUM];
};
typedef Locator *LocatorP;
#endif
