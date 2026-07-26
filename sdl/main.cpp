/*
 * XEvil(TM) Copyright (C) 1994,2000 Steve Hardt and Michael Judge
 * http://www.xevil.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.  See gpl.txt.
 */

// "main.cpp"  SDL port of the outer loop (cf. x11/main.cpp).  Same
// pre_clock/process-events/post_clock/yield turn structure, but the per-turn
// event drain uses SDL_PollEvent instead of the Xlib event queue.

#include <SDL2/SDL.h>

#include <cstdlib>
#include <iostream>

extern "C" {
#include <sys/time.h>
}

#include "utils.h"
#include "neth.h"
#include "game.h"

using namespace std;


class TurnStarter: public ITurnStarter {
public:
  TurnStarter(struct timeval *timer) : m_timer(timer) {}

  virtual void start_turn() {
    if (gettimeofday(m_timer,NULL) != 0) {
      cerr << "Error with gettimeofday()." << endl;
    }
  }

private:
  struct timeval *m_timer;
};



int main(int argc,char **argv) {
  Utils::seed_random();

  GameP game = new Game(&argc,argv);

  struct timeval startTime;
  TurnStarter turnStarter(&startTime);
  turnStarter.start_turn();

  while (True) {
    Quanta quanta = game->get_quanta();

    game->pre_clock();

    // Drain this turn's SDL events (window close, keys) and feed them to Game.
    if (game->has_ui()) {
      SDL_Event event;
      while (SDL_PollEvent(&event)) {
        game->process_event(0,&event);
      }
    }

    game->post_clock();

    if (game->quit_game()) {
      delete game;
      exit(0);
    }

    game->yield(startTime,quanta,&turnStarter);
  }
}
