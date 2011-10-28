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

#if X11
#ifndef NO_PRAGMAS
#pragma implementation "sound_cmn.h"
#endif
#endif

#include "stdafx.h"

#include "utils.h"
#include "streams.h"
#include "sound_cmn.h"
#if WIN32
#include "resource.h"
#endif

#include "bitmaps/sound_cmn/sound_cmn.bitmaps"


void SoundRequest::read(InStreamP in) {
  pos.read(in);
  soundName = (SoundName)in->read_int();
}



int SoundRequest::get_write_length() {
  return Pos::get_write_length() +
    sizeof(u_int);
}



void SoundRequest::write(OutStreamP out) const {
  pos.write(out);
  out->write_int((u_int)soundName);
}



// unnecessary
#if 0
SoundRequest &
SoundRequest::operator=(const SoundRequest& req) {
  id = req.id;
  soundName = req.soundName;
	return *this;
}
#endif



unsigned int SoundNames::lookup(SoundName name) {
#if X11
  // Avoid stupid compiler warnings.
  name = name;
#endif
#if WIN32
  if (name >= 0 && name < SOUND_MAX) {
    return names[name];
  }
#endif
  return 0;
}
