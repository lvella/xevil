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

#include "stdafx.h"
#include "fogpal.h"

FogPalette::FogPalette(ColorPalette &p_palette)
:PaletteModifier(p_palette)
{
  int t_int=0;
  m_amount=0;
  m_dir=-1*FOG_AMOUNT;
  for (int i=0;i<PALENTRIES;i++)//first 10 and last 10 are reserved
  {

    t_int=p_palette.palentries[i].peRed*p_palette.palentries[i].peRed;
    t_int+=p_palette.palentries[i].peGreen*p_palette.palentries[i].peGreen;
    t_int+=p_palette.palentries[i].peBlue*p_palette.palentries[i].peBlue;
    t_int= t_int/3;
    t_int= (int)sqrt((float)t_int);
    m_palette.palentries[i].peRed=t_int;

    m_palette.palentries[i].peGreen=t_int;
    m_palette.palentries[i].peBlue=t_int;
  }

  m_palette.palversion=0x300; //default of windows
  m_palette.numentries=PALENTRIES;//256
  m_fogholdingpalette=m_palette;
  m_hpal= ::CreatePalette((LOGPALETTE *)&m_palette);//create a global palette
}



void FogPalette::clock()
{
  m_palette=  m_fogholdingpalette;
  m_amount+=m_dir;
  if (m_amount>FOG_UPPERLIMIT)
  {
    m_amount=FOG_UPPERLIMIT;
    m_dir=-1*FOG_AMOUNT;
  }
  if (m_amount<FOG_LOWERLIMIT)
  {
    m_amount=FOG_LOWERLIMIT;
    m_dir=FOG_AMOUNT;
  }

  for (int i=0;i<PALENTRIES;i++)
  {
    if (((int)m_palette.palentries[i].peRed+m_amount)>255)
      m_palette.palentries[i].peRed=255;
    else
      if (((int)m_palette.palentries[i].peRed+m_amount)<0)
        m_palette.palentries[i].peRed=0;
    else
      m_palette.palentries[i].peRed+=m_amount;
    if (((int)m_palette.palentries[i].peGreen+m_amount)>255)
      m_palette.palentries[i].peGreen=255;
    else
      if (((int)m_palette.palentries[i].peGreen+m_amount)<0)
        m_palette.palentries[i].peGreen=0;
    else
      m_palette.palentries[i].peGreen+=m_amount;
    if (((int)m_palette.palentries[i].peBlue+m_amount)>255)
      m_palette.palentries[i].peBlue=255;
    else
      if (((int)m_palette.palentries[i].peBlue+m_amount)<0)
        m_palette.palentries[i].peBlue=0;
    else
      m_palette.palentries[i].peBlue+=m_amount;
  }
  m_ddpalette->SetEntries(0,0,255,m_palette.palentries);
}


