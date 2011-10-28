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
#include "palmod.h"
#include "xdata.h"


/*passed a palette,  does some modification on it and makes a lpdirectdrawpalette
keeps pointer to old palette to return it to normal.*/

PaletteModifier::PaletteModifier(ColorPalette &p_palette)
{
  m_applied=FALSE;
  m_ddpalette=NULL;
  m_oldpalette=NULL;
  m_surface=NULL;
  m_hpal=NULL;
}



PaletteModifier::PaletteModifier()
{
  m_applied=FALSE;
  m_ddpalette=NULL;
  m_oldpalette=NULL;
  m_surface=NULL;
  m_hpal=NULL;
}



PaletteModifier::~PaletteModifier()
{
    HRESULT t_result;
    if (m_applied)
        t_result=m_surface->SetPalette( m_oldpalette);

    if (m_ddpalette)
        m_ddpalette->Release();
    if (m_hpal)
      ::DeleteObject(m_hpal);

}



HRESULT
PaletteModifier::applyPalette(HDC p_hdc,LPDIRECTDRAWSURFACE p_surface, LPDIRECTDRAW p_lpDD)
{
    if (m_applied)
        return DD_OK;

    
/*    ::SelectPalette(p_hdc,m_hpal,FALSE);//false is to force us to use our own palette!
    UINT t_uint=::RealizePalette(p_hdc);
    t_uint=::GetSystemPaletteEntries(p_hdc,0,PALENTRIES,m_palette.palentries);*/

    HRESULT t_result=p_lpDD->CreatePalette( DDPCAPS_ALLOW256|DDPCAPS_8BIT, m_palette.palentries, &m_ddpalette,NULL);
    DHRESULT(t_result,return t_result);


    m_surface=p_surface;
    t_result=m_surface->GetPalette( &m_oldpalette);
    DHRESULT(t_result,return t_result);
    t_result=m_surface->SetPalette( m_ddpalette);
    DHRESULT(t_result,return t_result);
    m_applied=TRUE;
    return DD_OK;
}



HRESULT
PaletteModifier::removePalette()
{
    if (!m_applied)
        return DD_OK;
    HRESULT t_result=DD_OK;
    if (m_applied)
        t_result=m_surface->SetPalette( m_oldpalette);
    m_applied=FALSE;
    return t_result;
}


