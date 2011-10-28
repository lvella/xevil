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

// "ui.h"  Code common to all implementations of Ui.

#ifndef UI_CMN_H
#define UI_CMN_H

#if X11
#ifndef NO_PRAGMAS
#pragma interface
#endif
#endif

#include "utils.h"
#include "intel.h"

#define UI_KEYS_MAX IT_WEAPON_R



typedef unsigned long UImask;
// UNIX and Windows.
#define UInone 0L
#define UInewGame (1L<<0)
#define UIquit (1L<<1)
#define UIhumansNum (1L<<2)
#define UIenemiesNum (1L<<3)
#define UIenemiesRefill (1L<<4)
#define UIpause (1L<<6)
#define UIstyle (1L<<7)
#define UIquanta (1L<<8)
// Windows only.
#define UIrooms (1L<<9)
#define UIsoundvol (1L<<10)
#define UItrackvol (1L<<11)
#define UImusictype (1L<<12)
#define UIsound (1L<<13)
#define UIscreenmode (1L<<14)
// UNIX and Windows.
#define UIcooperative (1L<<15)
// Windows only.
#define UIdifficulty (1L<<16)
#define UIconnectServer (1L<<17)
#define UIdisconnectServer (1L<<18)
#define UIrunServer (1L<<19)
#define UIstopServer (1L<<20)
// UNIX and Windows.
#define UIchatRequest (1L<<21)


// This is the only bottleneck restricting the size of the passed message.
// Should make this an arbitrary length.
#define UI_CHAT_MESSAGE_MAX 160


class IKeyState {
public:
  virtual Boolean key_down(int key,void* closure) = 0;
  /* REQUIRES: 0 <= key < UI_KEYS_MAX */
  /* EFFECTS: Is the specified key currently down. */
};



class IDispatcher {
public:
  virtual void dispatch(ITcommand command,void* closure) = 0;
  /* EFFECTS: Cause the specified command to be sent. */
};



// The job of this class is to map the state of the keyboard into
// ITcommands to be executed.  Most of the logic is devoted to handling
// weirdness with weapon keys, e.g. holding down the WEAPON_USE key and
// pressing to the right causes IT_WEAPON_R to be dispatched.
class KeyDispatcher {
public:
  KeyDispatcher();

  void clock(IKeyState*,IDispatcher*,void* closure);
  /* NOTE: closure is a hack, the data should be in the implementation of 
     IKeyState and IDispatcher.  When I get around to making Ui have a
     list of Viewport instead of just a bunch of arrays, Viewport will
     implement these interfaces instead of Ui and we can get rid of this
     C-style "closure" bullshit.  (Did it on UNIX, now to do it on 
     Windows.) */
  

private:
  Boolean weaponKeyDown;
  ITcommand weaponCommandDefault;
  // Was dispatch() called since the last weapon key down.
  Boolean dispatchCalled;
};

  
#endif
