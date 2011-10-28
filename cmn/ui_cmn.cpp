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

// "ui_cmn.cpp"

#if X11
#ifndef NO_PRAGMAS
#pragma implementation "ui_cmn.h"
#endif
#endif

#include "stdafx.h"
#include "utils.h"
#include "ui_cmn.h"



KeyDispatcher::KeyDispatcher() {
  weaponKeyDown = False;
  weaponCommandDefault = IT_CENTER;
  dispatchCalled = False;
}



void KeyDispatcher::clock(IKeyState* keyState,IDispatcher* dispatcher,
                          void* closure) {
  ITcommand command = IT_NO_COMMAND;
  
  // Weirdness for firing weapons.
  // If user holds down WEAPON_CENTER and presses a direction, 
  // a shot is fired in that direction.    
  // If user presses and releases WEAPON_CENTER, a shot is fired
  // in the direction the user most recently moved or fired.
  if (keyState->key_down(IT_WEAPON_CENTER,closure)) {
    if (!weaponKeyDown) {
      weaponKeyDown = True;
      dispatchCalled = False;
    }
  }
  if (!keyState->key_down(IT_WEAPON_CENTER,closure) &&
      weaponKeyDown == True) {
    // Fire weapon in last direction if weapon_center was pressed and 
    // released without sending any other command.
    if (!dispatchCalled) {
      command = weaponCommandDefault;
    }
    weaponKeyDown = False;
  }
  
  
  // Movement
  if (keyState->key_down(IT_R,closure)) {
    command = weaponKeyDown ? IT_WEAPON_R : IT_R;
    weaponCommandDefault = IT_WEAPON_R;
  }
  if (keyState->key_down(IT_L,closure)) {
    if (command == IT_WEAPON_R || command == IT_R) {
      // If left and right pressed, consider it to be center.
      command = IT_CENTER;
      // Does not effect weaponCommandDefault.
    }
    else {
      command = weaponKeyDown ? IT_WEAPON_L : IT_L;
      weaponCommandDefault = IT_WEAPON_L;
    }
  }
  if (keyState->key_down(IT_DN,closure)) {
    if (keyState->key_down(IT_UP,closure)) {
      // If up and down pressed, do nothing
    }
    if ((command == IT_R)||(command == IT_WEAPON_R)) {
      command = weaponKeyDown ? IT_WEAPON_DN_R : IT_DN_R;
      weaponCommandDefault = IT_WEAPON_DN_R;
    }
    else if ((command == IT_L)||(command == IT_WEAPON_L)) {
      command = weaponKeyDown ? IT_WEAPON_DN_L : IT_DN_L;
      weaponCommandDefault = IT_WEAPON_DN_L;
    }
    else {
      command = weaponKeyDown ? IT_WEAPON_DN : IT_DN;
      weaponCommandDefault = IT_WEAPON_DN;
    }
  }
  else if (keyState->key_down(IT_UP,closure)) {
    if ((command == IT_R)||(command == IT_WEAPON_R)) {
      command = weaponKeyDown ? IT_WEAPON_UP_R : IT_UP_R;
      weaponCommandDefault = IT_WEAPON_UP_R;
    }
    else if ((command == IT_L)||(command == IT_WEAPON_L)) {
      command = weaponKeyDown ? IT_WEAPON_UP_L : IT_UP_L;
      weaponCommandDefault = IT_WEAPON_UP_L;
    }
    else {
      command = weaponKeyDown ? IT_WEAPON_UP : IT_UP;
      weaponCommandDefault = IT_WEAPON_UP;
    }
  }
  
  if (keyState->key_down(IT_CENTER,closure)) {  
    command = IT_CENTER;
    // Does not effect weaponCommandDefault.
  }
  if (keyState->key_down(IT_UP_R,closure)) {
    command = weaponKeyDown ? IT_WEAPON_UP_R : IT_UP_R;
    weaponCommandDefault = IT_WEAPON_UP_R;
  }
  if (keyState->key_down(IT_UP_L,closure)) {
    command = weaponKeyDown ? IT_WEAPON_UP_L : IT_UP_L;
    weaponCommandDefault = IT_WEAPON_UP_L;
  }
  if (keyState->key_down(IT_DN_L,closure)) {
    command = weaponKeyDown ? IT_WEAPON_DN_L : IT_DN_L;
    weaponCommandDefault = IT_WEAPON_DN_L;
  }
  if (keyState->key_down(IT_DN_R,closure)) {
    command = weaponKeyDown ? IT_WEAPON_DN_R : IT_DN_R;
    weaponCommandDefault = IT_WEAPON_DN_R;
  }
  
  
  // Rest of weapons and items.
  if (keyState->key_down(IT_WEAPON_CHANGE,closure)) {
    command = IT_WEAPON_CHANGE;
  }
  else if (keyState->key_down(IT_WEAPON_DROP,closure)) {
    command = IT_WEAPON_DROP;
  }
  else if (keyState->key_down(IT_ITEM_USE,closure)) {
    command = IT_ITEM_USE;
  }
  else if (keyState->key_down(IT_ITEM_CHANGE,closure)) {
    command = IT_ITEM_CHANGE;
  }
  else if (keyState->key_down(IT_ITEM_DROP,closure)) {
    command = IT_ITEM_DROP;
  }
  else if (keyState->key_down(IT_CHAT,closure)) {
    command = IT_CHAT;
  }
  
  
  // Only call dispatch() if we have an interesting command.
  if (command != IT_NO_COMMAND) {
    dispatcher->dispatch(command,closure);
    dispatchCalled = True;
  }	  
}
