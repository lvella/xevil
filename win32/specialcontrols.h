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

#ifndef _SPECIALCONTROLS_H
#define _SPECIALCONTROLS_H

class CSpecialEdit:public CEdit
{
   DECLARE_DYNAMIC( CSpecialEdit )
public:
    CSpecialEdit(){};
    virtual void SpecialOnLButtonDown( UINT nFlags, CPoint &point ){CEdit::OnLButtonDown(nFlags,point);}
    virtual void SpecialOnLButtonUp( UINT nFlags, CPoint &point ){CEdit::OnLButtonUp(nFlags,point);}
    virtual void SpecialOnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags ){CEdit::OnKeyDown(nChar,nRepCnt,nFlags );}
    virtual void SpecialOnKeyUp( UINT nChar, UINT nRepCnt, UINT nFlags ){CEdit::OnKeyUp(nChar,nRepCnt,nFlags );}
    virtual void SpecialOnChar( UINT nChar, UINT nRepCnt, UINT nFlags ){CEdit::OnChar(nChar,nRepCnt,nFlags );}
protected:
};


class CSpecialButton :public CButton
{
   DECLARE_DYNAMIC( CSpecialButton )
public:
    CSpecialButton();
	//{{AFX_MSG(CPicturePushButton)
    afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
    afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
    afx_msg void OnMouseMove( UINT nFlags, CPoint point );
	//}}AFX_MSG
    virtual void SpecialOnLButtonDown( UINT nFlags, CPoint &point ){OnLButtonDown(nFlags,point);}
    virtual void SpecialOnLButtonUp( UINT nFlags, CPoint &point ){OnLButtonUp(nFlags,point);}
    virtual void SpecialOnMouseMove( UINT nFlags, CPoint &point ){OnMouseMove(nFlags,point);}
protected:
    BOOL m_state;
    DECLARE_MESSAGE_MAP( )
};

class CPicturePushButton :public CSpecialButton
{
public:
    CPicturePushButton();
    void setPictureId(UINT p_resourceid){m_pictureid=p_resourceid;}
	//{{AFX_MSG(CPicturePushButton)
    afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
    afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
    afx_msg void OnMouseMove( UINT nFlags, CPoint point );
	//}}AFX_MSG
    virtual void SpecialOnLButtonDown( UINT nFlags, CPoint &point ){OnLButtonDown(nFlags,point);}
    virtual void SpecialOnLButtonUp( UINT nFlags, CPoint &point ){OnLButtonUp(nFlags,point);}
    virtual void SpecialOnMouseMove( UINT nFlags, CPoint &point ){OnMouseMove(nFlags,point);}
private:
    virtual void DrawItem(LPDRAWITEMSTRUCT);
    UINT m_pictureid;
    CBitmap m_bitmap;
    BOOL m_init;
    DECLARE_MESSAGE_MAP( )
};

#endif //_SPECIALCONTROLS_H


