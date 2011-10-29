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

// "locator.C"

#if X11
#ifndef NO_PRAGMAS
#pragma implementation "locator.h"
#endif
#endif

// Include Files
#include "stdafx.h"

#if WIN32
#include "resource.h"
#endif

#include "utils.h"
#include "coord.h"
#include "id.h"
#include "area.h"
#include "physical.h"
#include "locator.h"
#include "actual.h"
#include "sound_cmn.h"
#include "sound.h"

#if WIN32
#include "resource.h"
#endif

#include "bitmaps/locator/locator.bitmaps" // for arrows

using namespace std;

// When we reach this, start deleting objects.
#define OL_LIST_WARN ((int)(OL_LIST_MAX * 0.9))



OLgridEntry::OLgridEntry(OLentry *e) {
  entry = e; 
  prev = next = NULL; 
#if X11
  wasDrawn = False;
#endif
}



const Area &OLgridEntry::get_area() {
  assert(prev);
  return entry->physical->get_area();
}



int OLgridEntry::get_drawing_level() {
  assert(entry->physical && prev); 
  return entry->physical->get_drawing_level();
}



void OLgridEntry::insert(OLgridEntry* grid[OL_GRID_VERT_MAX]
                         [OL_GRID_HORIZ_MAX],
                         const GLoc &gl) {
  assert((prev == NULL) && (next == NULL));

  assert(Locator::valid(gl));
  entry->gloc = gl;

  next = grid[gl.vert][gl.horiz]->next;
  prev = grid[gl.vert][gl.horiz];
  if (grid[gl.vert][gl.horiz]->next)
    grid[gl.vert][gl.horiz]->next->prev = this;
  grid[gl.vert][gl.horiz]->next = this;
}



void OLgridEntry::remove() {
  assert(prev);

  if (next)
    next->prev = prev;
  prev->next = next;
  prev = next = NULL;
}



#if X11
OLshadowEntry::OLshadowEntry() {
  entry = NULL; 
  prev = next = NULL; 
  orphaned = False; 
  wasDrawn = False;
}



OLshadowEntry::OLshadowEntry(const Area &a,OLentry *e) {
  area = a; 
  entry = e; 
  prev = next = NULL; 
  orphaned = False; 
  wasDrawn = False;
}



void OLshadowEntry::insert(OLshadowEntry * shadows[OL_GRID_VERT_MAX]
                           [OL_GRID_HORIZ_MAX],
                           const GLoc &gl) {
  assert((prev == NULL) && (next == NULL));

  entry->shadowGloc = gl;
  assert(Locator::valid(gl));

  next = shadows[gl.vert][gl.horiz]->next;
  prev = shadows[gl.vert][gl.horiz];
  if (shadows[gl.vert][gl.horiz]->next)
    shadows[gl.vert][gl.horiz]->next->prev = this;
  shadows[gl.vert][gl.horiz]->next = this;
}



void OLshadowEntry::remove()
{
  assert (prev);

  if (next)
    next->prev = prev;
  prev->next = next;
  prev = next = NULL;
}
#endif


HumanP Incarnator::operator () ()
{
  // First non-null human that wants to be reincarnated >= index n.
  while (n < locator->humansMax && 
         !(locator->humans[n] && 
           locator->reincarnating[n] &&
           locator->reincarnateTimers[n].ready())) {
    n++;
  }

  if (n == locator->humansMax) {
    return NULL;
  }
  else {
    // Extract human.
    locator->reincarnating[n] = False;
    return locator->humans[n];
  }
}



PhysicalP PhysicalIter::operator () ()
{
  while (n < locator->listMax && !locator->list[n].valid) {
    n++;
  }
  
  assert(n <= locator->listMax);
  if (n == locator->listMax) {
    return NULL;
  }
  else {
    n++;
    return locator->list[n-1].physical;
  }
}



ArenaMessageIter::ArenaMessageIter(Locator &l,const IntelId &i) {
  ptr = l.arenaHead; 
  locator = &l; 
  intelId = i;
}



char *ArenaMessageIter::next(Boolean &propagate) {
  while (ptr != locator->arenaTail) {
    int ret = -1;

    // If exclusive, intels must match.
    if (locator->arenaMessages[ptr].exclusive) {
      if (locator->arenaMessages[ptr].intelId == intelId) {
        ret = ptr;
      }
    }
    // Return first message found.
    else {
      ret = ptr;
    }
    
    // Increment pointer even if we found a match.
    ptr = (ptr + 1) % OL_MESSAGES_MAX;

    if (ret != -1) {
      // Set return value of propagate.
      propagate = locator->arenaMessages[ret].propagate;
      return locator->arenaMessages[ret].msg;
    }
  }

  // Didn't find a match.
  return NULL;
}



Locator::Locator(WorldP w, int drawAlg,SoundManager * s) {
  //  assert(TEAMS_MAX >= HUMANS_MAX * 2);  no longer true

  contextCount = 0;
  world  = w;
  drawingAlgorithm = drawAlg;
  drawRects = False;
  soundManager = s;

  register_contexts();

  uniqueGen = 0;

  listMax = 0;

  for (int c = 0; c < OL_GRID_HORIZ_MAX; c++) {
    for (int r = 0; r < OL_GRID_VERT_MAX; r++) {
      grid[r][c] = new OLgridEntry(NULL);
      assert(grid[r][c]);
#if X11
      shadows[r][c] = new OLshadowEntry();
      assert(shadows[r][c]);
#endif
    }
  }

  addNum = 0;
  useDelList = False;
  delNum = 0;

  rememberSounds = False;
  soundsNum = 0;

#if X11
  shadowDelNum = 0;
#endif

  xValid = XVARS_VALID_INIT;

  head = tail = 0;
  arenaHead = arenaTail = 0;
  messagesIgnore = False;

  humansMax = enemiesNum = neutralsNum = 0;
  for (int n = 0; n < HUMANS_MAX; n++) {
    Timer nTimer(REINCARNATE_TIME);
    reincarnateTimers[n] = nTimer;
  }

  persistentTeamsNum = teamsNum = 0;
}



Locator::~Locator()	
{
	reset();
	for (int c = 0; c < OL_GRID_HORIZ_MAX; c++)
		for (int r = 0; r < OL_GRID_VERT_MAX; r++)
	{
		assert(grid[r][c]);
		delete grid[r][c];
#if X11
		assert(shadows[r][c]);
		delete shadows[r][c];
#endif
	}
#if X11
	for (;shadowDelNum>0;shadowDelNum--)
	{
		assert(shadowDelList[shadowDelNum-1]);
		delete shadowDelList[shadowDelNum-1];
	}
#endif
}



void Locator::add(PhysicalP p) {
  if (addNum >= OL_LIST_MAX) {
    cerr << "Object locator is full.  Can only have " << OL_LIST_MAX 
         << " objects in the game." << endl;
    assert(0);
  }
  addList[addNum] = p;
  addNum++;
  p->set_id(reserve_list_entry(p));
}



void Locator::add_as(PhysicalP p,const Id &id) {
  assert(id.index >= 0 && id.index < OL_LIST_MAX);

  // Kill any object that might have been at that location.
  // clean up the grid, possibly add to the shadowDelList
  if (id.index < listMax && list[id.index].valid && list[id.index].physical) {
    // die, fucker
    del_now(list[id.index].physical);
  }
  
  // Stolen from Locator::add
  if (addNum >= OL_LIST_MAX)
    {
      cerr << "Object locator is full.  Can only have " << OL_LIST_MAX 
	   << " objects in the game." << endl;
      assert(0);
    }
  //  addList[addNum] = p;a
  //  addNum++;

  // Make all entries less than id.index invalid.
  while (listMax <= id.index) {
    list[listMax].valid = False;
    list[listMax].reserved = False;
    listMax++;
  }

  //  if (id.index >= listMax) {
    // Since Locator::del_now() may have decreased listMax
  //   listMax = id.index + 1;
  //}

  list[id.index].valid = False;
  list[id.index].reserved = True;
  list[id.index].physical = p;

  // Just to make sure.
  p->set_id(id);

  add_now(p);
}



void Locator::get_nearby(PhysicalP nearby[OL_NEARBY_MAX],int &nitems,
			                   PhysicalP p,int radius)
{
  int glocRadius = (int)ceil((double)((double)radius / OL_GRID_SIZE_MIN)); //stupid MS compiler gets ambiguous call without the recast to double
  int radius_2 = radius * radius;
  nitems = 0;

  const Id id = p->get_id();
  GLoc pGLoc = list[id.index].gloc;
  const Area &pArea = p->get_area();
  Pos pPos = pArea.get_middle();

  GLoc gloc;
  for (gloc.horiz = pGLoc.horiz - glocRadius; 
       gloc.horiz <= pGLoc.horiz + glocRadius; gloc.horiz++)
    for (gloc.vert = pGLoc.vert - glocRadius; 
      	 gloc.vert <= pGLoc.vert + glocRadius; 
      	 gloc.vert++)
      if (valid(gloc))
	    {
	      // Skip the header.
	      OLgridEntry *ge = grid[gloc.vert][gloc.horiz]->get_next(); 
	      while (ge)
	        {
	          PhysicalP p2 = ge->get_physical();
	          if (p != p2)
		        {
		          const Area& area = p2->get_area();
		          if (pPos.distance_2(area.get_middle()) <= radius_2)
		            {
		              assert(nitems < OL_NEARBY_MAX);
		              nearby[nitems] = p2;
		              nitems++;
		            }
		        }
	          ge = ge->get_next();
	        }
	    }
}



PhysicalP Locator::lookup(const Id &id,Boolean recentAddition) {
  if (id.index != Id::INVALID &&
      id.index < listMax &&
      (list[id.index].valid ||
       // recentAddition toggles whether recently added objects can be
       // looked up.
       (recentAddition && list[id.index].reserved))) {
    Id testId = list[id.index].physical->get_id();
    if (testId == id) {
      return list[id.index].physical;
    }
  }
  return NULL;
}



IntelP Locator::lookup(const IntelId &iId) {
  if (iId.index == IntelId::INVALID) {
    return NULL;
  }

  // Check humans.
  if (iId.index < humansMax && humans[iId.index]) {
    IntelId testId = humans[iId.index]->get_intel_id();
    if (testId == iId) {
      return humans[iId.index];
    }
  }

  // Check enemies.
  if (iId.index < enemiesNum && enemies[iId.index]->alive()) {
    IntelId testId = enemies[iId.index]->get_intel_id();
    if (testId == iId) {
      return enemies[iId.index];
    }
  }

  // Check neutrals.
  if (iId.index < neutralsNum && neutrals[iId.index]->alive()) {
    IntelId testId = neutrals[iId.index]->get_intel_id();
    if (testId == iId) {
      return neutrals[iId.index];
    }
  }

  return NULL;
}



// Used on both UNIX (LargeViewport only) and Windows.
void Locator::draw_directly(CMN_DRAWABLE window,Xvars &xvars,int dpyNum,
                            const Area &area) {
  if (!xvars.is_valid(xValid)) {
    init_x(xvars,IX_INIT,NULL);
  }
  
  // Get all the grid squares covering the area in question.
  GLoc gridStart; 
  GLoc gridFinish; 
  grid_covering_area(gridStart,gridFinish,area);

  // Avoid double drawing composite objects.
  clear_already_drawn_flag(gridStart,gridFinish);
  
  // Loop over all drawing levels, all grid squares, all objects in each 
  // grid square.
  GLoc gloc;
  for (int dLevel = 0; dLevel < DRAWING_LEVELS; dLevel++) {
    for (gloc.vert = gridStart.vert; 
         gloc.vert < gridFinish.vert; 
         gloc.vert++) {
      for (gloc.horiz = gridStart.horiz; 
           gloc.horiz < gridFinish.horiz; 
           gloc.horiz++) {
        if (!valid(gloc)) {
          continue;
        }
        OLgridEntry *ge = grid[gloc.vert][gloc.horiz]->get_next();
        
        while (ge) {
          OLentry *ent = ge->get_entry();
          assert (ent->gloc == gloc);
          
          // Must be visible this turn and be at the current drawing level.
          if ((ge->get_mapped() || ge->get_flash()) 
              && (ge->get_drawing_level() == dLevel)) {
            PhysicalP p = ge->get_physical();
            assert(p);
            
            // Usually only draws one object, except for Composite objects 
            // with drawing order.
            draw_chain(window,xvars,dpyNum,area,p);
          }
          ge = ge->get_next();
        } // while ge
      } // horiz
    } // vert
  } // dlevel
}



void Locator::draw_ticks(CMN_DRAWABLE window,Xvars &xvars,int dpyNum,
                         const Area &area,const Id &id,
                         ITickRenderer* renderer) {
  renderer->begin_draw(dpyNum);

  PhysicalP p = lookup(id);
  // Only draw tick marks if there is a Physical to center them on.
  if (p) {     
    // In window coords.
    Area winArea = xvars.stretch_area(area);
    Pos winPos = winArea.get_pos();
    Size winSize = winArea.get_size();

    // Get all objects within a certain range.
    // Same range as machine players.
    PhysicalP nearby[OL_NEARBY_MAX];
    int nitems;
    get_nearby(nearby,nitems,p,IT_VISION_RANGE);

    for (int n = 0; n < nitems; n++) {
      // TICK_MAX means don't draw tick.
      NetDataP netData = nearby[n]->get_net_data();
      TickType tt = netData->get_tick_type();

      // Use tick type specified by the NetData first.  If there is none,
      // let the Locator compute it.
      if (tt == TICK_MAX) {
        tt = compute_tick_type(nearby[n],p->get_intel());
      }

      // Should we draw a tick for the Physical
      if (tt != TICK_MAX) {
        const Area &checkArea = nearby[n]->get_area();

        // Only draw tick marks for objects off-screen.
        // The Client decides whether or not to draw ticks, even if the
        // Server decided what type of tick to draw.
        if (!checkArea.overlap(area)) {
          Pos middle = checkArea.get_middle();
          int windowX = xvars.stretch_x(middle.x) - winPos.x;
          int windowY = xvars.stretch_y(middle.y) - winPos.y;

          if (windowY < 0) {  // along top edge
            if (windowX < 0) {
              // upper left corner
              if (!renderer->draw_tick(tt,window,xvars,dpyNum,
                                       winSize,CO_UP_L,0)) {
                break;
              }
            } 
            else if (windowX < winSize.width) {
              // regular case
              if (!renderer->draw_tick(tt,window,xvars,dpyNum,
                                       winSize,CO_UP,windowX)) {
                break;
              }
            }
            else {
              // upper right
              if (!renderer->draw_tick(tt,window,xvars,dpyNum,
                                       winSize,CO_UP_R,0)) {
                break;
              }
            }
          }
          else if (windowY >= winSize.height) { // along bottom edge
            if (windowX < 0) {
              // bottom left corner
              if (!renderer->draw_tick(tt,window,xvars,dpyNum,
                                       winSize,CO_DN_L,0)) {
                break;
              }
            }
            else if (windowX < winSize.width) {
              // normal case
              if (!renderer->draw_tick(tt,window,xvars,dpyNum,
                                       winSize,CO_DN,windowX)) {
                break;
              }
            }
            else {
              // bottom right
              if (!renderer->draw_tick(tt,window,xvars,dpyNum,
                                       winSize,CO_DN_R,0)) {
                break;
              }
            }
          }
          else { // right or left side
            if (windowX < 0) {
              if (!renderer->draw_tick(tt,window,xvars,dpyNum,
                                       winSize,CO_L,windowY)) {
                break;
              }
            }
            else if (windowX >= winSize.width) {
              if (!renderer->draw_tick(tt,window,xvars,dpyNum,
                                       winSize,CO_R,windowY)) {
                break;
              }
            }
            else {
              assert(0);
            }
          }
        } // overlap
      }// not TICK_MAX
    } // for
  } // if (p)

  renderer->end_draw(dpyNum);
}



// For Locator's implementation of ITickRenderer.  Does nothing.
void Locator::begin_draw(int) {
}



// For Locator's implementation of ITickRenderer.  Does nothing.
void Locator::end_draw(int) {
}



void Locator::draw_chain(CMN_DRAWABLE window,Xvars &xvars,int dpyNum,
                         const Area &area,PhysicalP p) {
  Id id = p->get_id();
  assert(lookup(id) == p);
  
  // Don't draw multiple times.
  // Since p was drawn, we can be sure that p's draw_before_me() objects 
  // were drawn.
  if (list[id.index].alreadyDrawn) {
    return;
  }

  // Can't reject p if it is outside area yet.  We must call all the way down
  // p's chain because p may have some draw-before that does overlap area.

  // First draw any objects that must be drawn before p.
  CompositeP comp = p->get_composite();
  if (comp) {
    // We are relying on get_draw_before_me() not to get in an infinite loop.
    PhysicalP q = comp->get_draw_before_me();
    if (q) {
      draw_chain(window,xvars,dpyNum,area,q);
    }
  }

  // Now we can reject p if it is outside area.
  const Area &areaP = p->get_area();  
  if (areaP.overlap(area)) {
    // Physical::draw() will do the correct offset.
    // Pass in area, not areaP.
    p->draw(window,xvars,dpyNum,area);
    list[id.index].alreadyDrawn = True;
  }
}



void Locator::reset()
{
  // Destroy the addList.
  int n;
  for (n = 0; n < addNum; n++) {
    delete addList[n];
  }
  addNum = 0;

  for (n = 0; n < listMax; n++) {
    if (list[n].valid) {
      del_now(list[n].physical);
    }
  }
  listMax = 0;

  // Don't reset this, because the client may be getting old packets.
  //  uniqueGen = 0;
  
  // Check grid integrity.  There should only be the headers remaining.
  for (int c = 0; c < OL_GRID_HORIZ_MAX; c++) {
    for (int r = 0; r < OL_GRID_VERT_MAX; r++) {
      if (!grid[r][c]) {
        cerr << "Error in grid integrity.  grid[" << r << "][" << c << "]"
             << " is null" << endl;
      } 
      else if (grid[r][c]->get_next() != NULL) {
        cerr << "Error in grid integrity.  grid[" << r << "][" << c << "]"
             << " has a next entry." << endl;
      }
    }
  }

  clear_all_messages();

  for (n = 0; n < humansMax; n++) {
    // May be null.
    delete humans[n];
  }

  for (n = 0; n < enemiesNum; n++) {
    delete enemies[n];
  }

  for (n = 0; n < neutralsNum; n++) {
    delete neutrals[n];
  }

  humansMax = enemiesNum = neutralsNum = 0;
  
  for (n = 0; n < teamsNum; n++) {
    delete_team_data(teams[n]);
  }
  teamsNum = 0;
  for (n = 0; n < persistentTeamsNum; n++) {
    delete_team_data(persistentTeams[n]);
  }
  persistentTeamsNum = 0;
}



void Locator::clear_all_messages() {
  // Delete all messages remaining in both queues.
  char *msg;
  while (msg = message_deq()) {
    delete msg;
  }
  
  IntelId dummy;
  Quanta dummy2 = 0;
  Boolean dummy3 = False;
  do {
    arena_message_deq(&msg,dummy,dummy2,dummy3);
    delete msg;
  } while (msg);
}



void Locator::level_reset() {
  // Kill off non-persistent teams.
  for (int n = 0; n < teamsNum; n++) {
    delete_team_data(teams[n]);
  }
  teamsNum = 0;
}



void Locator::clock() {
#if X11
  kill_shadow_del_list();

  shadows_follow_physicals();
#endif
  
  collision_checks();
  
  intelligence_and_act();

  kill_outside_world();

  kill_excess_objects();

  check_for_death();
  
  update_phase();
  
  grid_follows_physicals();
  // Also sets list[n].collided = False.

  add_new_objects();
  
  delete_dead();
  
  reincarnate_clock();
}



void Locator::client_pre_clock() {
#if X11
  kill_shadow_del_list();
  
  shadows_follow_physicals();
#endif
}



void Locator::client_post_clock() {  
  grid_follows_physicals();
  // Also sets list[n].collided = False.

  //  add_new_objects();
  
  //  delete_dead();
  
  //  reincarnate_clock();
}



#if X11
void Locator::kill_shadow_del_list() {
  // Kill shadow delete list.
  int n;       
  for (n = 0; n < shadowDelNum; n++) {
    shadowDelList[n]->remove();
    delete shadowDelList[n];
  }
  shadowDelNum = 0;
}



void Locator::shadows_follow_physicals() {
  int n;       

  // Move shadows to previous area.
  for (n = 0; n < listMax; n++) {
    if (list[n].valid && (list[n].mapped || list[n].flash)) {
      list[n].shadowEntry->set_area(list[n].physical->get_area());
      GLoc realGloc = compute_gloc(list[n].shadowEntry->get_area());
      
      if (list[n].shadowGloc != realGloc) {
	    list[n].shadowEntry->remove();
	    list[n].shadowEntry->insert(shadows,realGloc);
	  }
    }
  }
}
#endif



void Locator::intelligence_and_act() {
  /* Intelligence phase and action phase.  Set next variables.  Externally 
     visible state should not change. 
     Exceptions to this: canTake. */
  int n;
  for (n = 0; n < listMax; n++) {
    if (list[n].valid) {
      // Think before you act.
      list[n].physical->intelligence();
      list[n].physical->act();
    }
  }
}



void Locator::kill_outside_world() {
  int n;
  for (n = 0; n < listMax; n++) {
    if (list[n].valid && (list[n].mapped || list[n].flash)) {
      const Area &area = list[n].physical->get_area_next();
      if (!world->inside(area.middle_wsquare())) {
        // (&& !list[n].physical->delete_me())
        list[n].physical->set_quiet_death();
        list[n].physical->kill_self();
        arena_message_enq(Utils::strdup("Knocked Out Of World"),
                          list[n].physical);
      }
    }
  }
}



void Locator::check_for_death() {
  // Check for death.  I.e. healthNext < 0.
  /* NOTE: If using kill_self in this phase, an object must manually 
     call die and check for die_called().  Hack in Frog::die. */
  int n;
  for (n = 0; n < listMax; n++) {
    if (list[n].valid &&
        list[n].physical->get_health_next() < 0 && 
        list[n].physical->get_health() >= 0 &&
        !list[n].physical->die_called()) {
      list[n].physical->die();
    }
  }
}



void Locator::update_phase() {
  // Update externally visable state.
  int n;
  for (n = 0; n < listMax; n++) {
    if (list[n].valid) {
      list[n].physical->update();
    }
  }
}



void Locator::grid_follows_physicals() {
  /* Update the locator grid, shifting gridEntries around if necessary. */
  int n;
  for (n = 0; n < listMax; n++) {
    if (list[n].valid) {
      allign_flash_and_mapped(n);
      if (list[n].mapped || list[n].flash) {
        const Area &area = list[n].physical->get_area();
        if (world->inside(area.middle_wsquare())) {
          GLoc realGLoc = compute_gloc(list[n].physical);

          if (list[n].gloc != realGLoc) {
            list[n].gridEntry->remove();
            list[n].gridEntry->insert(grid,realGLoc);
          }
        }
        else {
          if (!(list[n].physical->die_called() && 
                list[n].physical->delete_me())) {
            // For Server and StandAlone, we should have already cleaned up 
            // in kill_outside_world.  
            // For Client, we do it here.
            del_now(list[n].physical);
            /*
              cerr << "item knocked out of world, but die " <<
              list[n].physical->die_called() << " and delete " <<
              list[n].physical->delete_me() << " not both called " << endl;
              */
          }
        } // inside
	    } // mapped || flash
      
      list[n].collided = False;
    } // if valid
  } // for n
}



void Locator::add_new_objects() {
  // Add all objects on the list.
  int n;
  for (n = 0; n < addNum; n++) {
    add_now(addList[n]);
  }
  addNum = 0;
}



void Locator::delete_dead() {
  // Delete objects that are dead.
  int n;
  for (n = 0; n < listMax; n++) {
    if (list[n].valid && list[n].physical->delete_me()) {
      del_now(list[n].physical);
    }
  }
}



void Locator::message_enq(char *msg) {
  if (messagesIgnore) {
      delete msg;
      return;
    }

  if ((tail + 1) % OL_MESSAGES_MAX != head) {
      messages[tail] = msg;
      tail = (tail + 1) % OL_MESSAGES_MAX;
    }
  else {
    delete msg;
//  cout << "Warning: Message queue overflow.  Discarding message." << endl;
  }
}



char *Locator::message_deq() {
  if (head == tail)
    return NULL;

  int ret = head;
  head = (head + 1) % OL_MESSAGES_MAX;
  
  return messages[ret];
}



char *Locator::peek_most_recent_message() {
  if (head == tail) {
    return NULL;
  }

  return messages[Utils::mod(tail - 1,OL_MESSAGES_MAX)];
}



void Locator::arena_message_enq(char *msg,IntelId *intelId,
                                Quanta time,Boolean propagate) {
  // Don't disable arenaMessages.
  //  if (messagesIgnore) {
  //  delete msg;
  //  return;
  //}

  if ((arenaTail + 1) % OL_MESSAGES_MAX != arenaHead) {
    // Fill ArenaMessage structure.
    arenaMessages[arenaTail].msg = msg;
    arenaMessages[arenaTail].exclusive = (intelId != NULL);
    arenaMessages[arenaTail].time = time;
    arenaMessages[arenaTail].propagate = propagate;
    if (intelId) {
      arenaMessages[arenaTail].intelId = *intelId;
    }
    arenaTail = (arenaTail + 1) % OL_MESSAGES_MAX;
  }
  else {
    delete msg;
//  cout << "Warning: Message queue overflow.  Discarding message." << endl;
  }
}



void Locator::arena_message_enq(char *msg,PhysicalP p) {
  IntelP intel = p->get_intel();
  if (intel) {
    IntelId intelId = intel->get_intel_id();
    // Will always do default amount of time.
    arena_message_enq(msg,&intelId);
  }
  else {
    delete msg;
  }
}



Boolean Locator::arena_message_deq(char **msg,IntelId &intelId,
                                   Quanta &time,Boolean &propagate) {
  assert(msg);
  
  if (arenaHead == arenaTail) {
    *msg = NULL;
    time = -1;
    return False;
  }

  int ret = arenaHead;
  arenaHead = (arenaHead + 1) % OL_MESSAGES_MAX;
  
  *msg = arenaMessages[ret].msg;
  time = arenaMessages[ret].time; // may be -1
  propagate = arenaMessages[ret].propagate;
  if (arenaMessages[ret].exclusive) {
    intelId = arenaMessages[ret].intelId;
    return True;
  }
  else {
    return False;
  }
}



void Locator::register_human(HumanP h)
{
  // find an empty spot
  int n;
  for (n = 0; n < humansMax; n++) {
    if (!humans[n]) {
      break;
    }
  }

  // Increase size of array.
  if (n == humansMax) {
    assert(humansMax < HUMANS_MAX);
    humansMax++;
  }

  // n is now a valid spot to insert the human.
  
  // Assign h an IntelId.
  IntelId iId = h->get_intel_id();
  assert(iId.index == IntelId::INVALID);
  iId.index = n;
  iId.unique = uniqueGen;
  uniqueGen++;
  h->set_intel_id(iId);

  // Add to the array of Humans.
  humans[n] = h;
  reincarnating[n] = False;
  reincarnateTimers[n].reset();
}



void Locator::register_human_as(HumanP h) {
  IntelId iId = h->get_intel_id();
  assert(iId.index != IntelId::INVALID && iId.index < HUMANS_MAX);
  
  if (iId.index < humansMax && humans[iId.index]) {
    // kill human off if one was already there.
    delete humans[iId.index];
  }
  // Increment humansMax
  while (humansMax <= iId.index) {
    humans[humansMax] = NULL;
    reincarnating[humansMax] = False;
    reincarnateTimers[humansMax].reset();
    humansMax++;
  }
  humans[iId.index] = h;

  // These values don't matter.
  reincarnating[iId.index] = False;
  reincarnateTimers[iId.index].reset();
}



void Locator::unregister_human(const IntelId &iId) {
  IntelP intel = lookup(iId);
  if (!(intel && intel->is_human())) {
    return;
  }

  delete humans[iId.index];
  humans[iId.index] = NULL;

  // Decrease effective size of list.
  int n = iId.index;
  if (n == humansMax - 1) {
    // Scan back til we hit a valid entry.
    while(n > 0 && !humans[n - 1]) {
      n--;
    }
    humansMax = n;
  }
}



void Locator::register_enemy(EnemyP m)
{
  IntelId iId = m->get_intel_id();
  assert(iId.index == IntelId::INVALID);
  iId.unique = uniqueGen;
  uniqueGen++;

  // Try inserting in current array.
  for (int n = 0; n < enemiesNum; n++) {
    if (!enemies[n]->alive()) {
      delete enemies[n];
      enemies[n] = m;
      iId.index = n;
      m->set_intel_id(iId);
      return;
    }
  }

  // Else add to array.
  assert(enemiesNum < ENEMIES_MAX);
  enemies[enemiesNum] = m;
  iId.index = enemiesNum;
  m->set_intel_id(iId);
  enemiesNum++;
}



void Locator::register_neutral(NeutralP m) {
  IntelId iId = m->get_intel_id();
  assert(iId.index == IntelId::INVALID);
  iId.unique = uniqueGen;
  uniqueGen++;

  // Try inserting in current array.
  for (int n = 0; n < neutralsNum; n++) {
    if (!neutrals[n]->alive()) {
      delete neutrals[n];
      neutrals[n] = m;
      iId.index = n;
      m->set_intel_id(iId);
      return;
    }
  }
  
  // Else add to array.
  assert(neutralsNum < NEUTRALS_MAX);
  neutrals[neutralsNum] = m;
  iId.index = neutralsNum;
  m->set_intel_id(iId);
  neutralsNum++;
}



int Locator::humans_playing() {
  int n,ret;
  for (n = 0, ret = 0; n < humansMax; n++) {
    if (humans[n] && (humans[n]->alive() || humans[n]->reincarnate_me())) {
      ret++;
    }
  }
  return ret;
}



int Locator::enemies_alive() {
  int n,ret;
  for (n = 0, ret = 0; n < enemiesNum; n++) {
    if (enemies[n]->alive()) {
      ret++;
    }
  }
  return ret;
}



int Locator::neutrals_alive()
{
  int n,ret;
  for (n = 0, ret = 0; n < neutralsNum; n++)
    if (neutrals[n]->alive())
      ret++;
  return ret;
}



int Locator::humans_registered() {
  // Count elements of humans[] that are non-null.
  int ret = 0;
  for (int n = 0; n < humansMax; n++) {
    if (humans[n]) {
      ret++;
    }
  }
  return ret;
}



HumanP Locator::get_human(int which) {
  int humansBefore = 0;
  for (int n = 0; n < humansMax; n++) {
    if (humans[n]) {
      if (which == humansBefore) {
        return humans[n];
      }
      // Count number of valid humans before n.
      humansBefore++;
    }
  }
  // Asked for a human that doesn't exist.
  assert(0);
  return NULL;
}



void Locator::set_remember_deleted(Boolean val) {
  if (val == useDelList) {
    return;
  }

  if (val) {
    assert(delNum == 0);
    useDelList = True;
  }
  else {
    // Clear out existing requests.
    delNum = 0;
    useDelList = False;        
  }
}



void Locator::set_remember_sounds(Boolean val) {
  if (val == rememberSounds) {
    return;
  }

  if (val) {
    assert(soundsNum == 0);
    rememberSounds = True;
  }
  else {
    // Clear out existing requests.
    soundsNum = 0;
    rememberSounds = False;
  }
}



TeamId Locator::add_persistent_team(Boolean (*same_team)
                                    (LocatorP,PhysicalP,PhysicalP,void *),
                                    void *closure,TeamOptionsP ops) {
  // Code dupd in add_team()
  assert(persistentTeamsNum < TEAMS_MAX);
  
  // same unique number generator as regular teams.
  TeamId ret = uniqueGen;
//  ret.index = persistentTeamsNum;
// ret.unique = uniqueGen;
  uniqueGen++;

  persistentTeams[persistentTeamsNum].teamId = ret;
  if (ops) {
    persistentTeams[persistentTeamsNum].options = *ops;    
  } 
  else {
    TeamOptions defaultOps;
    persistentTeams[persistentTeamsNum].options = defaultOps;    
  }
  persistentTeams[persistentTeamsNum].same_team = same_team;
  persistentTeams[persistentTeamsNum].closure = closure;
  persistentTeamsNum++;  
  return ret;
}



TeamId Locator::add_team(Boolean (*same_team)(LocatorP,PhysicalP,PhysicalP,void *),
                         void *closure,TeamOptionsP ops) {
  // Code dupd in add_persistent_team()
  assert(teamsNum < TEAMS_MAX);
  
  TeamId ret = uniqueGen;
//  ret.index = teamsNum;
//  ret.unique = uniqueGen;
  uniqueGen++;

  teams[teamsNum].teamId = ret;
  if (ops) {
    teams[teamsNum].options = *ops;    
  } 
  else {
    TeamOptions defaultOps;
    teams[teamsNum].options = defaultOps;    
  }
  teams[teamsNum].same_team = same_team;
  teams[teamsNum].closure = closure;
  teamsNum++;  
  return ret;
}



#if 0  
Boolean Locator::team_member(PhysicalP p,const TeamId &teamId) {
  if (teamId.index != Id::INVALID) {
    return False;
  }

  // Check regular teams.
  if (teamId.index < teamsNum &&
      teamId == teams[teamId.index].teamId) {
    Boolean ret = 
      teams[teamId.index].member(this,p,teams[teamId.index].closure);
    return ret;
  }

  // Check persistent teams.
  if (teamId.index < persistentTeamsNum &&
      teamId == persistentTeams[teamId.index].teamId) {
    Boolean ret = 
      persistentTeams[teamId.index].member(this,p,persistentTeams[teamId.index].closure);
    return ret;
  }

  // Invalid team, so not a member.
  return False;
}
#endif

  

Boolean Locator::same_team(TeamOptions &ops,PhysicalP p1,PhysicalP p2) {
  // Is there a better way to do this?
  int n;

  // regular teams
  for (n = 0; n < teamsNum; n++) {
    if (teams[n].same_team(this,p1,p2,teams[n].closure)) {
      // Both are on the same team according to rule n.
      ops = teams[n].options;
      return True;
    }
  }

  // persistent teams
  for (n = 0; n < persistentTeamsNum; n++) {
    if (persistentTeams[n].same_team(this,p1,p2,persistentTeams[n].closure)) {
      // Both are on team n.
      ops = persistentTeams[n].options;
      return True;
    }
  }

  // Tried all the teams.
  return False;
}



void Locator::delete_team_data(OLTeam &team) {
  if (team.options.deleteClosure) {
    delete team.closure;
  }
}



const PhysicalContext *Locator::get_context(ClassId classId) {
  if (classId >= 0 && classId < A_CLASSES_NUM) {
    return contexts[classId];
  }
  return NULL;
}



int Locator::filter_contexts(const PhysicalContext *contextList[A_CLASSES_NUM],
			     ClassId idList[A_CLASSES_NUM],
			     Boolean (* filter)(const PhysicalContext *,void *),
                 void *closure) {
  assert(list);
  int numFound = 0;
  
  int n;
  for (n = 0; n < A_CLASSES_NUM; n++)
    if (contexts[n] && filter(contexts[n],closure)) {
      assert(numFound < A_CLASSES_NUM);
      if (contextList) {
        contextList[numFound] = contexts[n];
      }
      if (idList) {
        // We guarantee that the index of a context in Locator::contexts
        // is the ClassId of that context.
        idList[numFound] = (ClassId)n;
      }
      numFound++;
  }
  return numFound;
}



void Locator::get_item_info(Boolean &isItem,Boolean &isWeapon,
                            CMN_BITS_ID &iconId,ClassId cId) {
  assert(cId >= 0 && cId < A_None);
  // A regular class.
  if (cId < A_CLASSES_NUM) {
    if (contexts[cId] && contexts[cId]->get_item_info != NULL) {
      contexts[cId]->get_item_info(isItem,isWeapon,iconId);
    }
    else {
      isItem = False;
      isWeapon = False;
      iconId = (CMN_BITS_ID)0;
    }
  }
  // One of the "non-existent" classes, see coord.h
  else {
    const OLItemInfo &iInfo = nonExistentClassesItemInfo[cId - A_CLASSES_NUM];
    isItem = iInfo.isItem;
    isWeapon = iInfo.isWeapon;
    iconId = iInfo.iconId;
  }
}



TickType Locator::compute_tick_type(PhysicalP p,IntelP relativeTo) {
  TickType tt = TICK_MAX;

  IntelP intel = p->get_intel();
  // Have intel and not be invisible to draw tick marks.
  if (intel && p->is_moving() && !((MovingP)p)->is_invisible()) {
    // Choose appropriate type of tick.
    if (intel->is_human()) {
      tt = TICK_HUMAN;
    }
    else if (intel->is_enemy()) {
      tt = TICK_ENEMY;
    }
    else {
      tt = TICK_NEUTRAL;

      // Slaves of other humans should have the tick mark of a human.
      IntelP pMaster = lookup(((MachineP)intel)->get_master_intel_id());
      if (pMaster) {
        assert(pMaster->is_human());
        if (pMaster != relativeTo) {
          tt = TICK_HUMAN;
        }
      }
    }
  }
  // Draw nearby objects of great importance, e.g Flag, Xit, AltarOfSin
  else {
    // Could use PhysicalContext, but why bother.
    if (p->get_class_id() == A_Flag ||
        p->get_class_id() == A_Xit ||
        p->get_class_id() == A_AltarOfSin) {
      tt = TICK_OTHER;
    } 
  }
  return tt;
}



void Locator::add_now(PhysicalP p) {
  Id id = p->get_id();
  int n = id.index;

  assert (list[n].physical == p);
  list[n].valid = True;
  list[n].collided = False;
  list[n].collidable = p->collidable();
  list[n].mapped = p->get_mapped();
  list[n].flash = p->get_flash();
  list[n].alreadyDrawn = False;

  // Maybe we should set the reserved flag to False?

  if (list[n].mapped || list[n].flash) {
    // Create and add shadow and grid entries.
    list[n].gridEntry = new OLgridEntry(&list[n]);
    assert(list[n].gridEntry);
    GLoc gloc = compute_gloc(list[n].physical);
    list[n].gridEntry->insert(grid,gloc);
    
#if X11
    list[n].shadowEntry = new OLshadowEntry(p->get_area(),&list[n]);
    assert(list[n].shadowEntry);
    list[n].shadowEntry->insert(shadows,gloc);
#endif
  }
}



void Locator::del_now(PhysicalP p) {
  Id id = p->get_id();
  
  // Object has been added and is in locator, add it to the delete list and
  // kill its OLEntry and shadowEntry
  if (list[id.index].valid) {
    // Add to the delete list.
    // This code copied in allign_flash_and_mapped.
    if (useDelList) {
      if (delNum < OL_LIST_MAX) {
        delList[delNum] = id;
        delNum++;
      }
      else {
        cerr << "List of recently deleted objects has overflown." << endl;
      }
    }
    
    list[id.index].valid = False;
    list[id.index].reserved = False;
    
    if (list[id.index].mapped || list[id.index].flash) {
      list[id.index].gridEntry->remove();
      delete list[id.index].gridEntry;
#if X11
      shadowDelList[shadowDelNum] = list[id.index].shadowEntry;
      list[id.index].shadowEntry->set_orphaned();
      shadowDelNum++;
#endif
    }
  }
  // Object has been added, but we are deleting it before Locator has a chance
  // to clock and complete the process.  So, remove from the add list and 
  // unreserve the entry.
  else {
    if (!list[id.index].reserved) {
      cerr << "Locator::del_now() bad ID when deleting object" << endl;
      return;
    }
    list[id.index].reserved = False;
    // Already know that valid is False.

    // Delete p from the addList.
    int n;
    Boolean found = False;
    for (n = 0; n < addNum; n++) {
      if (addList[n] == p) {
        addList[n] = addList[addNum - 1];
        addList[addNum - 1] = NULL;
        addNum--;
        found = True;
        break;
      }
    }
    // Make sure the Physical was actually in the add list.
    assert(found);
  }

  // Decrease effective size of list.
  if (id.index == listMax - 1) {
    // Scan back til we hit a valid entry.
    while(id.index > 0 && !list[id.index-1].valid) {
      id.index--;
    }
    listMax = id.index;
  }

  // We now make sure to delete p.
  delete p;
}



void Locator::allign_flash_and_mapped(int n)
{
  Boolean flash = list[n].physical->get_flash();
  Boolean mapped = list[n].physical->get_mapped();

  // Add new OLgridEntry and shadowEntry.
  if (!(list[n].mapped || list[n].flash) && (mapped || flash))
    {
      list[n].gridEntry = new OLgridEntry(&list[n]);
      assert(list[n].gridEntry);
      GLoc gloc = compute_gloc(list[n].physical);
      list[n].gridEntry->insert(grid,gloc);      
#if X11
      list[n].shadowEntry = 
	new OLshadowEntry(list[n].physical->get_area(),&list[n]);
      assert(list[n].shadowEntry);
      list[n].shadowEntry->insert(shadows,gloc);
#endif
    }

  // Destroy grid entry and mark shadow entry for deletion.
  if ((list[n].mapped || list[n].flash) && !(mapped || flash)) {
    list[n].gridEntry->remove();
    delete list[n].gridEntry;
#if X11
    shadowDelList[shadowDelNum] = list[n].shadowEntry;
    list[n].shadowEntry->set_orphaned();
    shadowDelNum++;
#endif

    // kind of a hack using delList for recently unmapped objects.
    if (useDelList) {
      if (delNum < OL_LIST_MAX) {
        delList[delNum] = list[n].physical->get_id();
        delNum++;
      }
      else {
        cerr << "List of recently deleted objects has overflown." << endl;
      }
    }    
  }
  
  list[n].mapped = mapped;
  list[n].flash = flash;
}



Id Locator::reserve_list_entry(PhysicalP p) {
  // Must use this form of get_id(), since this is one of the few places
  // where it is valid for the id not to be set.
  Id dummy;

  for (int n = 0; ; n++) {
    assert(n <= listMax); 
    
    // Increase effective size of list.
    if (n == listMax) {
      // If Locator list is full, we are in deep shit.  This will 
      // probably cause an assertion somewhere.
      if (n >= OL_LIST_MAX) {
        cerr << "ERROR: Exceeded maximum number of " << OL_LIST_MAX
        << " objects in the game." << endl;
        // Return an invalid Id.
        return dummy;
      }
      list[n].valid = False;
      list[n].reserved = False;
      listMax++;
    }
    
    // Found an empty entry.
    if (!list[n].valid && !list[n].reserved) {
      Id ret;
      ret.index = n;
      ret.unique = uniqueGen;
      uniqueGen++;
      list[n].reserved = True;
      list[n].physical = p;
      return ret;
    }
  }
}



/* Collision phase.  Physical::collide will be called at most once for any
   object. */
void Locator::collision_checks() {
  for (int n = 0; n < listMax; n++) {
    if (list[n].valid && 
        !list[n].collided && 
        list[n].mapped && 
        list[n].collidable &&
        list[n].physical->alive()) {  // So corpses don't collide, see below.
      PhysicalP p1 = list[n].physical;
      GLoc gloc;

	    for (gloc.vert = list[n].gloc.vert - 1; 
	         gloc.vert <= list[n].gloc.vert + 1; 
	         gloc.vert++) {
	      for (gloc.horiz = list[n].gloc.horiz - 1; 
	           gloc.horiz <= list[n].gloc.horiz + 1; 
	           gloc.horiz++) {
	        if (valid(gloc) && possible_overlap(p1->get_area(),gloc)) {
            // Skip the header.
            OLgridEntry *ge = grid[gloc.vert][gloc.horiz]->get_next(); 
            while (ge) {
	            PhysicalP p2 = ge->get_physical(); 
	            assert(p2);
	            IntelP intel1 = p1->get_intel();
	            IntelP intel2 = p2->get_intel();
                TeamOptions ops;
                
	            if (
                  !ge->get_collided() &&
                  ge->get_mapped() &&
                  ge->get_collidable() &&

                  p1 != p2 &&  // Don't collide with self.

                  // Make sure neither physical is set not to 
                  // collide with the other.
                  p2->get_dont_collide() != p1->get_id() &&
                  p1->get_dont_collide() != p2->get_id() &&

                  // Teammates don't collide with each other
                  !(same_team(ops,p1,p2) && ops.membersDontCollide) &&

                  // If both have intels, make sure neither intel 
                  // is set not to collide with the other.
                  // If they don't both have intels, continue.
                  (! (intel1 && intel2) ||  
                   (intel1->get_dont_collide() != intel2->get_intel_id() &&
	                  intel2->get_dont_collide() != intel1->get_intel_id())
                   ) &&

                  p2->alive() // So corpses don't collide, see below.
                 ) { 

                  // Composite objects don't collide with each other.
                  // We could do this with teams.
                  CompositeP c1 = p1->get_composite();
                  CompositeP c2 = p2->get_composite();
                  if (c1 && c2 && 
                      c1->get_composite_id() == c2->get_composite_id()) {
                    ge = ge->get_next();
                    continue;
                  }

                  /// Check for the dont_collide_composite() flag
                  if (c1 && c1->get_composite_id() == p2->get_dont_collide_composite()) {
                    ge = ge->get_next();
                    continue;
                  }
                  if (c2 && c2->get_composite_id() == p1->get_dont_collide_composite()) {
                    ge = ge->get_next();
                    continue;
                  }

                  const Area &a1 = p1->get_area();
                  if (a1.overlap(p2->get_area())) {
                    if (p1->get_mass() > p2->get_mass()) {
	                    p2->avoid(p1);
                    }
                    else if (p2->get_mass() > p1->get_mass()) {
	                    p1->avoid(p2);
                    }
                    else if (Utils::coin_flip()) {
	                    p1->avoid(p2);
                    }
                    else {
	                    p2->avoid(p1);
                    }

                    p1->collide(p2);
                    p2->collide(p1);
                    list[n].collided = True;
                    ge->set_collided(True);
                  } // overlap
              } // big-ass if


              // Also called above to "continue"
              ge = ge->get_next();
            } // while ge
          } // valid gloc
        } // gloc horiz
	    } // gloc vert
    } // if list valid
  } // for n
}



GLoc Locator::compute_gloc(PhysicalP p)
{
  const Area &area = p->get_area();
#ifndef NDEBUG
  Size size = area.get_size();
  assert(size.width <= OL_GRID_WIDTH && size.height <= OL_GRID_HEIGHT);
#endif

  Pos middle = area.get_middle(); 
  GLoc gloc;
  gloc.horiz = middle.x / OL_GRID_WIDTH;
  gloc.vert = middle.y / OL_GRID_HEIGHT;
  return gloc;
}



GLoc Locator::compute_gloc(const Area &area)
{
  Pos middle = area.get_middle(); 
  GLoc gloc;
  gloc.horiz = middle.x / OL_GRID_WIDTH;
  gloc.vert = middle.y / OL_GRID_HEIGHT;
  return gloc;
}



Boolean Locator::possible_overlap(const Area &area,const GLoc &gloc) {
  //static int tr;
  //static int fa;

  Pos pos(gloc.horiz * OL_GRID_WIDTH - OL_GRID_WIDTH / 2,
	  gloc.vert * OL_GRID_HEIGHT - OL_GRID_HEIGHT / 2);
  Size size;
  size.set(2 * OL_GRID_WIDTH,2 * OL_GRID_HEIGHT);
  Area virtGLoc(AR_RECT,pos,size);
  if (virtGLoc.overlap(area)) {
    //tr++;
    //cout << "true/false: " << tr << "/" << fa << " ratio: " << ((float)tr / (float)fa) << endl;
    return True;
  }
  else {
    //fa++;
    //cout << "true/false: " << tr << "/" << fa << " ratio: " << ((float)tr / (float)fa) << endl;
    return False;
  }
}



void Locator::reincarnate_clock(){
  for (int n = 0; n < humansMax; n++) {
    if (humans[n]) {
      if (!reincarnating[n] && humans[n]->reincarnate_me()) {
        reincarnating[n] = True;
        reincarnateTimers[n].set();
      }
      reincarnateTimers[n].clock();
    }
  }
}



void Locator::kill_excess_objects() {
  // Count number of existing objects.
  int count = 0;
  int n;
  for (n = 0; n < listMax; n++) {
    if (list[n].valid || list[n].reserved) {
      count++;
    }
  } 

  // If we decide to trim off the excess objects.
  if (count > OL_LIST_WARN) {
    cerr << "Approaching critical number of objects in XEvil."
         << "  Trim some of the excess." << endl;
    
    // Kill unheld items.
    for (n = 0; n < listMax; n++) {
      if (list[n].valid) {
        PhysicalP p = list[n].physical;
        if (p->is_item() && !((ItemP)p)->is_held()) {
          p->set_quiet_death();
          p->kill_self();
        }  
      }
    }

    // Count remaining
    int postCount = 0;
    for (n = 0; n < listMax; n++) {
      if (list[n].valid || list[n].reserved) {
        postCount++;
      }
    } 
    cerr << "Was " << count << " objects, there are now " 
         << postCount << " objects." << endl;
  }
}



void Locator::clear_already_drawn_flag(const GLoc &gridStart,
                                       const GLoc &gridFinish) {
  GLoc gloc;
  for (gloc.vert = gridStart.vert; gloc.vert < gridFinish.vert; gloc.vert++) {
    for (gloc.horiz = gridStart.horiz; gloc.horiz < gridFinish.horiz; 
         gloc.horiz++) {
      if (valid(gloc)) {
        OLgridEntry *ge = grid[gloc.vert][gloc.horiz]->get_next();
        
        while (ge) {
  	      OLentry *ent = ge->get_entry();
          // Clear the flag.
          ent->alreadyDrawn = False;
          ge = ge->get_next();
        }
      }
    }
  }
}



void Locator::grid_covering_area(GLoc& start,GLoc& finish,const Area& area) {
  Pos aPos;
  Size aSize;
  area.get_rect(aPos,aSize);

  start.horiz =
    (int)floor((aPos.x / WSQUARE_WIDTH)/(float)OL_GRID_COL_MAX) - 1;
  start.vert =
    (int)floor((aPos.y / WSQUARE_HEIGHT)/(float)OL_GRID_ROW_MAX) - 1; 
  
  finish.horiz = 
    (int)ceil(ceil((float)(aPos.x + aSize.width) / (float)WSQUARE_WIDTH)/ 
              (float) OL_GRID_COL_MAX) + 1;
  finish.vert = 
    (int)ceil(ceil((float)(aPos.y + aSize.height)/(float)WSQUARE_HEIGHT)/
              (float) OL_GRID_ROW_MAX) + 1;
}



void Locator::register_contexts() {
  // As good a place as any to put this call.
  Ability::register_abilities();

  // Set all contexts to NULL.
  int n;
  for (n = 0; n < A_CLASSES_NUM; n++) {
    contexts[n] = NULL;
  }

  // Here we go...
  register_context(&Explosion::context);
  register_context(&Fire::context);
  register_context(&FireExplosion::context);
  register_context(&NProtection::context.physicalContext);
  register_context(&TProtection::context.physicalContext);
  register_context(&XProtection::context.physicalContext);
  register_context(&Trapdoor::context.physicalContext);
  register_context(&Home::context.physicalContext);
  register_context(&Shell::context.movingContext.physicalContext);
  register_context(&SwapShell::context.movingContext.physicalContext);
  register_context(&Lance::context.movingContext.physicalContext);
  register_context(&Laser::context.movingContext.physicalContext);
  register_context(&FrogShell::context.movingContext.physicalContext);
  register_context(&Fireball::context.movingContext.physicalContext);
  register_context(&Missile::context.movingContext.physicalContext);
  register_context(&Star::context.movingContext.physicalContext);
  register_context(&Blood::context.movingContext.physicalContext);
  register_context(&GreenBlood::context.movingContext.physicalContext);
  register_context(&OilDroplet::context.movingContext.physicalContext);
  register_context(&Feather::context.movingContext.physicalContext);
  register_context(&Grenade::context.movingContext.physicalContext);
  register_context(&Napalm::context.movingContext.physicalContext);
  register_context(&Egg::context.movingContext.physicalContext);
  register_context(&Xit::context.fallingContext.movingContext.physicalContext);
  register_context(&Flag::context.fallingContext.movingContext.physicalContext);
  register_context(&Rock::context.heavyContext.fallingContext.movingContext.physicalContext);
  register_context(&Weight::context.heavyContext.fallingContext.movingContext.physicalContext);
  register_context(&AltarOfSin::context.fallingContext.movingContext.physicalContext);
  register_context(&Doppel::context.itemContext.fallingContext.
		   movingContext.physicalContext);
  register_context(&Cloak::context.fallingContext.
		   movingContext.physicalContext);
  register_context(&Transmogifier::context.itemContext.fallingContext.
		   movingContext.physicalContext);
  register_context(&MedKit::context.itemContext.fallingContext.
		   movingContext.physicalContext);
  register_context(&CrackPipe::context.autoUseContext.itemContext.fallingContext.
		   movingContext.physicalContext);
  register_context(&Caffine::context.autoUseContext.itemContext.fallingContext.
		   movingContext.physicalContext);
  register_context(&PCP::context.autoUseContext.itemContext.fallingContext.
		   movingContext.physicalContext);
  register_context(&NShield::context.autoUseContext.itemContext.fallingContext.
		   movingContext.physicalContext);
  register_context(&TShield::context.autoUseContext.itemContext.fallingContext.
		   movingContext.physicalContext);
  register_context(&Bomb::context.itemContext.fallingContext.
		   movingContext.physicalContext);
  register_context(&Chainsaw::context.weaponContext.itemContext.fallingContext.
		   movingContext.physicalContext);
  register_context(&Pistol::context.weaponContext.itemContext.fallingContext.
		   movingContext.physicalContext);
  register_context(&MGun::context.weaponContext.itemContext.fallingContext.
		   movingContext.physicalContext);
  register_context(&Lancer::context.weaponContext.itemContext.fallingContext.
		   movingContext.physicalContext);
  register_context(&AutoLancer::context.weaponContext.itemContext.fallingContext.
		   movingContext.physicalContext);
  register_context(&FThrower::context.weaponContext.itemContext.fallingContext.
		   movingContext.physicalContext);
  register_context(&Launcher::context.weaponContext.itemContext.fallingContext.
		   movingContext.physicalContext);
  register_context(&Grenades::context.weaponContext.itemContext.fallingContext.
		   movingContext.physicalContext);
  register_context(&Napalms::context.weaponContext.itemContext.fallingContext.
		   movingContext.physicalContext);
  register_context(&Stars::context.weaponContext.itemContext.fallingContext.
		   movingContext.physicalContext);
  register_context(&Swapper::context.gunContext.weaponContext.itemContext.
		   fallingContext.movingContext.physicalContext);
  register_context(&FrogGun::context.gunContext.weaponContext.itemContext.
		   fallingContext.movingContext.physicalContext);
  register_context(&DogWhistle::context.weaponContext.itemContext.
		   fallingContext.movingContext.physicalContext);
  register_context(&DemonSummoner::context.weaponContext.itemContext.
		   fallingContext.movingContext.physicalContext);
  register_context(&Enforcer::creatureContext.movingContext.physicalContext);
  register_context(&Frog::creatureContext.movingContext.physicalContext);
  register_context(&Hero::creatureContext.movingContext.physicalContext);
  register_context(&Zombie::creatureContext.movingContext.physicalContext);
  register_context(&Ninja::creatureContext.movingContext.physicalContext);
  register_context(&Alien::creatureContext.movingContext.physicalContext);
  register_context(&RedHugger::creatureContext.
		   movingContext.physicalContext);
  register_context(&GreenHugger::creatureContext.
		   movingContext.physicalContext);
  register_context(&ChopperBoy::creatureContext.movingContext.physicalContext);
  register_context(&Seal::creatureContext.movingContext.physicalContext);
  register_context(&FireDemon::creatureContext.movingContext.physicalContext);
  register_context(&Dragon::creatureContext.movingContext.physicalContext);
  register_context(&Walker::creatureContext.movingContext.physicalContext);
  register_context(&Dog::creatureContext.movingContext.physicalContext);
  register_context(&Mutt::creatureContext.movingContext.physicalContext);
  register_context(&Yeti::creatureContext.movingContext.physicalContext);
  register_context(&Chicken::creatureContext.movingContext.physicalContext);
  register_context(&PhysMover::context);
  

  assert(contextCount <= A_CLASSES_NUM);

  // Some sanity checking, see if something is wrong with the
  // newly programmed PhysicalContexts.
  for (n = 0; n < A_CLASSES_NUM; n++) {
    const PhysicalContext *cx = contexts[n];
    if (cx) {
      // Make sure that the index of a context is the same as it's classId.
      assert(cx->classId == n);

      // If potentialEnemy, enemyWeight is greater than zero.
      // Not the converse, see Seals.
      if (cx->potentialEnemy) {
        assert(cx->enemyWeight > 0);
      }
      assert(cx->enemyWeight >= 0);

      assert(cx->create);

      // Should have a get_stats() method iff one of the stats* flags is 
      // set.
      assert((cx->statsCreations || cx->statsUses || cx->statsDeaths) ==
	           (cx->get_stats != NULL));
      
      // Can't act as both a weapon and otherItem.
      assert(!(cx->potentialWeapon && cx->potentialOtherItem));

      // If Physical could be created as either a weapon or otherItem, make
      // sure objectWorldPercent is greater than zero.
      assert((cx->potentialWeapon || cx->potentialOtherItem) ==
	           (cx->objectWorldPercent > 0));

      assert(cx->objectWorldPercent >= 0);
    }
  }


  if (contextCount < A_CLASSES_NUM) {
    cout << "ERROR: Locator::register_contexts(): Not all classes have been "
      << "registered" << endl;
  }
}



void Locator::register_context(const PhysicalContext *context)
{
  contexts[context->classId] = context;
  //  int cCountPre = contextCount;
  contextCount++;
  //  int cCountPost = contextCount;
  // cerr << "cCountPre is " << cCountPre << " post is " << cCountPost << 
  //  " contextCount@ " << &contextCount << " is " << contextCount << endl;
}



Boolean 
Locator::submitSoundRequest(SoundRequest req) {
  // Don't bother submitting or remembering the sound request if no sound.
  SoundName name = req.get_sound_name();
  // Don't use lookup, because we want to submit the sound if we are
  // a Server sending to Client.
  if (!(name > 0 && name < SoundNames::SOUND_MAX)) {
    return False;
  }
  
  // Server will rember recent sounds so it can send them to the Client.
  if (rememberSounds && soundsNum < OL_RECENT_SOUNDS_MAX) {
    sounds[soundsNum] = req;
    soundsNum++;
  }
  
  if (!SoundNames::lookup(name)) {
    return False;
  }

  return soundManager->submitRequest(req);
}
