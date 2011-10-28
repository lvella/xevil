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

// "sound.h"
// Dummy classes, only implemented on Windows. 
// A place holder if we ever implement sound on X.

#ifndef SOUND_H
#define SOUND_H

#include "utils.h"
#include "sound_cmn.h"

struct SoundEvent {
  SoundEvent() {dummy = 0;}
  int dummy;
};

class SoundManager {
public:
  SoundManager(Boolean,Locator *) {}
  ~SoundManager() {}
  
  Boolean isSoundOn(){return False;}
  void turnOnoff(Boolean){} 
  void setTrackVolume(int) {}
  void setEffectsVolume(int) {}
  int getTrackVolume(){return 0;}
  int getEffectsVolume(){return 0;}

  //  Boolean init(HWND hwndOwner);
  Boolean removeSound(unsigned int){return False;}
  Boolean playSound(unsigned int,int,int,Boolean,Boolean = False) 
  {return False;}
  Boolean stopSound(unsigned int) {return False;}
  Boolean destroyAllSound() {return False;}

  Boolean submitRequest(SoundRequest) {return False;}
  SoundEvent getEvent(int) {SoundEvent ret; return ret;}
  void clearRegisteredSounds() {}

  void setKeyPosition(short,Pos){}
  Pos getKeyPosition(short){Pos ret; return ret;}
  void setNumKeyPositions(short){}
  short getNumKeyPositions(){return 0;}
};

#endif





