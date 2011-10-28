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

#include "palmod.h"
#include "utils.h"

class FogPalette: public PaletteModifier
{
public:
  enum {FOG_UPPERLIMIT=50,FOG_LOWERLIMIT=-50, FOG_AMOUNT=5};
  FogPalette::FogPalette(ColorPalette &p_palette);
  virtual ~FogPalette(){}
  virtual void clock();
  ColorPalette m_fogholdingpalette;
  int m_amount;
  char m_dir;
};
