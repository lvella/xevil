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

// "id.h" Object locator id.

#ifndef ID_H
#define ID_H

#if X11
	#ifndef NO_PRAGMAS
	#pragma interface
	#endif
#endif

#include "streams.h"

class Identifier {
 public:
  void read(InStreamP in);
  static int get_write_length();
  void write(OutStreamP out) const;

  Boolean operator == (const Identifier &other) const;
  Boolean operator != (const Identifier &other) const;
  /* NOTE: Two INVALID Identifiers are not equal to each other. */

  void invalidate() {index = INVALID;}
  /* EFFECTS: Guarantees that the Identifier is invalid. */


#ifndef PROTECTED_IS_PUBLIC 
protected:  
#endif
  Identifier();
  Identifier(InStreamP in) {read(in);}

  enum {INVALID = -1};
  int index; 
  int unique;
};



class Id: public Identifier {
  friend class Locator;
public:
  Id() {}
  Id(InStreamP in) : Identifier(in) {}
};



class IntelId: public Identifier {
  friend class Locator;
public:
  IntelId() {}
  IntelId(InStreamP in) : Identifier(in) {}
};



class MoverId: public Identifier {
  friend class World;
public:
  MoverId() {}
  MoverId(InStreamP in) : Identifier(in) {}
};

typedef u_int CompositeId;
#define COMPOSITE_ID_NONE 0xffffffff;

#endif
