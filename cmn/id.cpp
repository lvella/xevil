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

// "id.cpp"

#if X11
	#ifndef NO_PRAGMAS
	#pragma implementation "id.h"
	#endif
#endif
#include "stdafx.h"


#include <limits.h>

#include "utils.h"
#include "id.h"



// Lame-o HP compiler won't let this be inline.
Identifier::Identifier() {
  invalidate();
}



void Identifier::read(InStreamP in) {
  u_short val = in->read_short(); 
  if (val == 0xffff) {
    index = INVALID;
  }
  else {
    index = val;
  }
  unique = in->read_int();
}



int Identifier::get_write_length() {
  return sizeof(short) + sizeof(int);
}



void Identifier::write(OutStreamP out) const {
  u_short val;
  if (index == INVALID) {
    val = 0xffff;
  }
  else {
    assert(index < USHRT_MAX);  // strictly less than to avoid 0xffff
    val = (u_short)index;
  }

  out->write_short(val); 
  out->write_int(unique);
}



Boolean Identifier::operator == (const Identifier &other) const {
  return (index != INVALID) && (other.index == index) 
    && (other.unique == unique);
}



Boolean Identifier::operator != (const Identifier &other) const {
  return (index == INVALID) || (other.index != index) 
    || (other.unique != unique);
}
