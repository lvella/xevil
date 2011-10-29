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

// "main.cpp"

// Include Files
#include "utils.h"

extern "C" {
#include <X11/Xutil.h>

#ifdef OPENWOUND_XOS_STRLEN_HACK
#define _strings_h
#endif
#include <X11/Xos.h>
}

#include <cstdlib>
#include <iostream>

#include "utils.h"
#include "neth.h"
#include "game.h"

using namespace std;


class TurnStarter: public ITurnStarter {
public:
  TurnStarter(struct timeval *timer) 
    : m_timer(timer) {
  }
  /* EFFECTS: Set timer to be the start time for the turn when 
     startTurn() is called. */
  
  virtual void start_turn() {
    if (gettimeofday(m_timer,NULL) != 0) {
      cerr << "Error with gettimeofday()." << endl;
    }
  }


private:
  struct timeval *m_timer;
};


// main's job is very simple.
// Create the Game, then run the outer loop, one iteration per turn.
// Empty the event queue and feed it to Game every turn.
int main(int argc, char **argv) {
  Utils::seed_random();

  GameP game = new Game(&argc,argv);

  //  assert (clock() != -1);

  long total = 0;
  long events = 0;

  // TurnStarter exists so Game::yield() can decide when to start timing 
  // the next turn.
  struct timeval startTime;
  TurnStarter turnStarter(&startTime);
  // Start the first turn.
  turnStarter.start_turn();

  while (True) {
    Quanta quanta = game->get_quanta(); 
    
    // Clock the game
    total++;
    
    game->pre_clock();

    if (game->has_ui()) {
      for (int dpyNum = 0; dpyNum < game->get_dpy_max(); dpyNum++) {
        int eventsNum;
        if (eventsNum = 
            XEventsQueued(game->get_dpy(dpyNum),QueuedAfterReading))
          for (int m = 0; m < eventsNum; m++) {
            XEvent event;
            XNextEvent(game->get_dpy(dpyNum),&event);
            game->process_event(dpyNum,&event);
            events++;
          }
      }
    }
    
    game->post_clock();
    if (game->show_stats() && !(total % Game::REPORT_TIME))
      cout << "total:" << total << " events:" << events << 
	  " percent:" << ((float)events / (float) total) << endl; 
    
    // This should be the only place exit() is called.
    // (But it isn't, ui.cpp calls if license agreement is rejected 
    //  and game.cpp calls if "xevil -info".)
    if (game->quit_game()) {
      delete game;
      exit(1);
    }
    
    // Let game decide how to throw time away.
    game->yield(startTime,quanta,&turnStarter);
  }
}

