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
#include "specialcontrols.h"

/***************************************/
//SpecialEdit
IMPLEMENT_DYNAMIC( CSpecialEdit, CEdit)

/***************************************/
//CSpecialButton
IMPLEMENT_DYNAMIC( CSpecialButton, CButton)

BEGIN_MESSAGE_MAP(CSpecialButton, CButton)
	//{{AFX_MSG_MAP(CPicturePushButton)
	ON_WM_LBUTTONDOWN( )
	ON_WM_LBUTTONUP( )
    ON_WM_MOUSEMOVE( )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CSpecialButton::OnMouseMove( UINT p_uint, CPoint p_point)
{
}



void CSpecialButton::OnLButtonDown( UINT nFlags, CPoint point )
{
    m_state=TRUE;
    CButton::OnLButtonDown( nFlags, point );
    Invalidate();
    SetCapture();
}



CSpecialButton::CSpecialButton()
{
    m_state=FALSE;
    
}
void CSpecialButton::OnLButtonUp( UINT nFlags, CPoint point )
{
    CRect t_rect;
    GetClientRect(t_rect);
    if (t_rect.PtInRect(point))
    {
        if (m_state)
        {
            if (GetCheck()== 0)
                SetCheck(1);
            else 
                SetCheck(0);
        
            m_state=0;
            Invalidate();
            CButton::OnLButtonUp( nFlags, point );
            GetOwner()->PostMessage(WM_COMMAND,GetDlgCtrlID( ));
        }
    }
    ReleaseCapture();
}


/********************************************/
//CPicturePushButton
//CPicturePushButton
//CPicturePushButton
//CPicturePushButton



BEGIN_MESSAGE_MAP(CPicturePushButton, CSpecialButton)
	//{{AFX_MSG_MAP(CPicturePushButton)
	ON_WM_LBUTTONDOWN( )
	ON_WM_LBUTTONUP( )
    ON_WM_MOUSEMOVE( )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



void CPicturePushButton::OnMouseMove( UINT p_uint, CPoint p_point)
{
    CRect t_rect;
    BOOL t_state;
    GetClientRect(t_rect);
    if (p_uint==MK_LBUTTON)
    {
        if (!t_rect.PtInRect(p_point))
            t_state=0;
        else
            t_state=TRUE;
        if (t_state!=m_state)
        {
            m_state=t_state;
            Invalidate();

        }
    }
}



void CPicturePushButton::OnLButtonDown( UINT nFlags, CPoint point )
{
    m_state=TRUE;
    CButton::OnLButtonDown( nFlags, point );
    Invalidate();
    SetCapture();
}



CPicturePushButton::CPicturePushButton()
{
    m_init=FALSE;
}

void CPicturePushButton::OnLButtonUp( UINT nFlags, CPoint point )
{
    if (m_state)
    {
        CRect t_rect;
        m_state=0;
        GetClientRect(t_rect);
        if (t_rect.PtInRect(point))
        {
            Invalidate();
            CButton::OnLButtonUp( nFlags, point );
            GetOwner()->PostMessage(WM_COMMAND,GetDlgCtrlID( ));
        }
    }
    ReleaseCapture();
}



void
CPicturePushButton::DrawItem(LPDRAWITEMSTRUCT p_struct)
{
    CDC *t_dc=CDC::FromHandle(p_struct->hDC);
    CBrush *t_oldbrush=(CBrush *)t_dc->SelectStockObject(LTGRAY_BRUSH);
    CRect t_rect(p_struct->rcItem);
    t_dc->PatBlt(t_rect.left,t_rect.top,t_rect.Width(),t_rect .Height(),PATCOPY);
    t_dc->SelectObject(t_oldbrush);

    CPen *t_oldpen;
    if (m_state==TRUE)
        t_oldpen=(CPen *)t_dc->SelectStockObject(BLACK_PEN);
    else
        t_oldpen=(CPen *)t_dc->SelectStockObject(WHITE_PEN);
    t_dc->MoveTo(t_rect.right-1,0);
    t_dc->LineTo(0,0);
    t_dc->LineTo(0,t_rect.bottom-1);

    CPen t_newpen;
    if (m_state==TRUE)
        t_newpen.CreateStockObject(WHITE_PEN);
    else
        t_newpen.CreatePen(PS_SOLID,1,RGB(96,96,96));
    t_dc->SelectObject(&t_newpen);
    t_dc->MoveTo(t_rect.right-1,0);
    t_dc->LineTo(t_rect.right-1,t_rect.bottom-1);
    t_dc->LineTo(0,t_rect.bottom-1);
    t_dc->SelectObject(t_oldpen);//put it back

	CString t_string;
	GetWindowText(t_string.GetBuffer(255),255);
	t_string.ReleaseBuffer();
	t_dc->SelectStockObject(BLACK_PEN);
	t_dc->SelectStockObject(ANSI_VAR_FONT);
	t_rect.left+=5;
	t_dc->SetBkMode(OPAQUE);
	t_dc->SetBkColor(RGB(192,192,192));
	t_dc->DrawText(t_string,t_string.GetLength(),t_rect,DT_LEFT|DT_VCENTER|DT_SINGLELINE);

    if (!m_init)
    {
        if (!m_bitmap.LoadBitmap(m_pictureid))
        {
            TRACE("failed to load bitmap for CPicturePushButton continuing");
        }
        m_init=TRUE;
    }

	CDC dcImage;
	if (!dcImage.CreateCompatibleDC(t_dc))
		return;
    BITMAP bm;
    m_bitmap.GetBitmap(&bm);

    // Paint the image.
    CBitmap* pOldBitmap = dcImage.SelectObject(&m_bitmap);
    t_dc->BitBlt(2, 2, __min(bm.bmWidth,t_rect.Width()), __min(bm.bmHeight,t_rect.Height()), &dcImage, 0, 0, SRCCOPY);
    dcImage.SelectObject(pOldBitmap);

}
