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

// "sound_cmn.h"  Sound code common to Windows and X.

#ifndef CMN_SOUND_H
#define CMN_SOUND_H

#if X11
	#ifndef NO_PRAGMAS
	#pragma interface
	#endif
#endif

#include "utils.h"
#include "coord.h"
#include "streams.h"
#include "id.h"
#include "area.h"



class SoundRequest {
public:
  SoundRequest(SoundName s,const Area &area) 
    {pos = area.get_middle(); soundName = s;}

  SoundRequest(){}

  void read(InStreamP in);

  static int get_write_length();

  void write(OutStreamP out) const;

  Pos get_pos() {return pos;}
  // Use pos instead of Id, so it will still work over the network. */

  SoundName get_sound_name() {return soundName;}


private:
  Pos pos;
  SoundName soundName;
};



class SoundNames {
public:
  enum {
    SOUNDTRACK = 1, // start at 1, 0 is reserved.
    SOUNDTRACK_LEVELS,
    SOUNDTRACK_SEAL,
    CHAINSAW_SOUND,
    FLAMETHROWER,
    DEATH,
    SEAL_DEATH,
    HUGGER_DEATH,
    FROG_DEATH,
    BREAKDOWN,
    BANG,
    PISTOL,
    MGUN,
    LAUNCHER,
    EXPLOSION,
    DOG_DEATH,
    LASER,
    HERO_ATTACK,
    NINJA_ATTACK,
    DOG_ATTACK,
    CHOP_DEATH,
    DOPPEL_USE,
    CLOAK_USE,
    TRANS_USE,
    SHIELD_USE,
    NINJA_DEATH,
    FROGGUN,
    LANCER,
    SWAPPER,
    FIRE_SOUNDTRACK,
    HIVE_SOUNDTRACK,
    KILL_SOUNDTRACK,
    SEAL_SOUNDTRACK,
    ZEEPEEG_SOUNDTRACK,
    NIGHTSKY_SOUNDTRACK,
    SWEETDARK_SOUNDTRACK,
//    TERRAEXM_SOUNDTRACK,
    NEWSONG_SOUNDTRACK,    

    // Don't change the following.
    SOUND_MAX,
    SOUND_RANDOM = -1,
  };

  static unsigned int lookup(SoundName);
  /* EFFECTS: Return the platform-specific resource name for the SoundName or 0
     if not found. */


private:
#if WIN32
  static unsigned int names[SOUND_MAX];
#endif  
};

#endif
