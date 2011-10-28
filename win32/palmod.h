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

#ifndef PALMOD_H
#define PALMOD_H

#include "xdata.h"

class PaletteModifier
{
public:
    PaletteModifier(ColorPalette &p_palette);
    PaletteModifier();
    virtual ~PaletteModifier();
    virtual HRESULT applyPalette(HDC p_hdc, LPDIRECTDRAWSURFACE p_suface, LPDIRECTDRAW p_lpDD);
    virtual HRESULT removePalette();
    virtual void clock()=0;
protected:
    LPDIRECTDRAWPALETTE m_ddpalette;
    LPDIRECTDRAWPALETTE m_oldpalette;
    LPDIRECTDRAWSURFACE m_surface;
    HPALETTE m_hpal;
    ColorPalette m_palette;
    BOOL m_applied;
};

/*passed a palette,  does some modification on it and makes a lpdirectdrawpalette
keeps pointer to old palette to return it to normal.*/

class DefaultModifier:public PaletteModifier
{
public:
    DefaultModifier(){}
    virtual ~DefaultModifier(){}
    virtual HRESULT applyPalette(HDC p_hdc, LPDIRECTDRAWSURFACE p_suface, LPDIRECTDRAW p_lpDD){return DD_OK;}
    virtual HRESULT removePalette(){return DD_OK;}
    virtual void clock(){}
};

#endif //PALMOD_H

