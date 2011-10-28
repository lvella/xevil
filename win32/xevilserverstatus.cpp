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

// XEvilServerStatus.cpp : implementation file
//

#include "stdafx.h"
#include "xevil.h"
#include <afxtempl.h>
#include "XEvilServerStatus.h"



ATOM CXEvilServerStatus::atom;


CXEvilServerStatus::CXEvilServerStatus()
{
    m_close=FALSE;
    m_togglestate=SMALL;
}




IMPLEMENT_DYNAMIC( CXEvilServerStatus, CWnd)

BEGIN_MESSAGE_MAP(CXEvilServerStatus, CWnd)
	//{{AFX_MSG_MAP(CXEvilServerStatus)
	ON_BN_CLICKED(IDC_DISPLAY, OnDisplay)
	ON_BN_CLICKED(IDC_STARTSERVER, OnStartserver)
	ON_BN_CLICKED(IDC_STOPSERVER, OnStopserver)
	ON_BN_CLICKED(IDC_XEVIL3, OnSettings)
	ON_WM_CLOSE()
    ON_WM_PAINT()
    ON_WM_SETCURSOR( )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CXEvilServerStatus message handlers

void CXEvilServerStatus::OnLButtonDown( UINT nFlags, CPoint point )
{
}



void CXEvilServerStatus::OnLButtonUp( UINT nFlags, CPoint point )
{
}



void CXEvilServerStatus::OnDisplay() 
{
	// TODO: Add your control notification handler code here
	
}

void CXEvilServerStatus::OnStartserver() 
{
	// TODO: Add your control notification handler code here
	
}

void CXEvilServerStatus::OnStopserver() 
{
	// TODO: Add your control notification handler code here
	
}

void CXEvilServerStatus::OnSettings() 
{
	// TODO: Add your control notification handler code here
	
}


void CXEvilServerStatus::OnClose( )
{
	m_close=TRUE;
    CWnd::OnClose();
}


BOOL
CXEvilServerStatus::Create(CRect &p_rect,CWnd *p_parent)
{
	HWND t_hwnd(NULL);
	if (p_parent)
		t_hwnd=p_parent->m_hWnd;

	WNDCLASSEX wndClass;
    m_cursor=theApp.LoadCursor(IDC_XEVIL_SERVER);
	if (!CXEvilServerStatus::atom)
	{
		wndClass.cbSize=sizeof(WNDCLASSEX);
		wndClass.style=CS_BYTEALIGNCLIENT|CS_VREDRAW | CS_HREDRAW;
		wndClass.lpfnWndProc=::DefWindowProc;
		wndClass.cbClsExtra=0;
		wndClass.cbWndExtra=0;
        wndClass.hInstance=AfxGetInstanceHandle();
		wndClass.hIcon=theApp.LoadIcon(IDR_XEVIL);
		wndClass.hCursor=m_cursor;
		wndClass.hbrBackground=NULL;
		wndClass.lpszMenuName=NULL;
		wndClass.lpszClassName="XEvilServerViewPortClass";
        wndClass.hIconSm=theApp.LoadIcon(IDR_XEVIL_SMALL);
		CXEvilServerStatus::atom=::RegisterClassEx(&wndClass);
		if (!CXEvilServerStatus::atom)
		{
			assert(FALSE);
			AfxMessageBox("Failed to register window class, XEvilServerViewPort");
			return FALSE;
		}
	}
	BOOL t_return;
    t_return= CWnd::CreateEx(
        WS_EX_WINDOWEDGE,
        "XEvilServerViewPortClass",
        "XEvil Server",
        WS_MINIMIZEBOX |    // so we dont have to call ShowWindow
        WS_SYSMENU |    // so we get a icon in in our tray button
        WS_CAPTION|
        WS_EX_CLIENTEDGE|
        WS_THICKFRAME|
        WS_POPUPWINDOW,
		p_rect.left,
        p_rect.top,
        SERVER_VIEW_WIDTH,
        SERVER_VIEW_HEIGHT,
		t_hwnd/*parent*/,
        NULL/*DorHMenu*/,
        NULL/*lpParam*/);

    m_startserver.setPictureId(IDB_XEVIL);
    m_stopserver.setPictureId(IDB_STOP);
    m_settings.setPictureId(IDB_SERVEROPTIONS);

    m_font.CreateFont(-10,0,0,0,FW_THIN,0,0,0,0,OUT_TT_PRECIS,0,DEFAULT_QUALITY,DEFAULT_PITCH,NULL);

    CRect t_rect(0,0,36,36);
	m_startserver.Create("",WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_OWNERDRAW,t_rect,this,IDC_STARTSERVER);
    t_rect.OffsetRect(36,0);
	m_stopserver.Create("",WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_OWNERDRAW,t_rect,this,IDC_STOPSERVER);
    t_rect.OffsetRect(36,0);
	m_settings.Create("",WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_OWNERDRAW,t_rect,this,IDC_SETTINGS);

    m_startserver.SetCursor(NULL);
    m_stopserver.SetCursor(NULL);
    m_settings.SetCursor(NULL);

    t_rect.OffsetRect(2+36,0);
    t_rect.bottom=t_rect.top+16;
    t_rect.right=t_rect.left+50;
    t_return|=m_static1.Create("# Players:",WS_VISIBLE|WS_CHILD|SS_SIMPLE,t_rect,this,IDC_STATIC);
    CRect t_rect2=t_rect;

    t_rect.OffsetRect(0,2+16);
    t_rect.right=t_rect.left+30;
    t_return|=m_static2.Create("Time:",WS_VISIBLE|WS_CHILD|SS_SIMPLE,t_rect,this,IDC_STATIC);

    t_rect.OffsetRect(32,0);
    t_rect.right=t_rect.left+63;
    t_return|=m_timeconnected.Create("365:24:60:99",WS_VISIBLE|WS_CHILD|SS_RIGHT,t_rect,this,IDC_STATIC);
    

    t_rect2.left=t_rect.right-32;
    t_rect2.right=t_rect2.left+32;
    t_return|=m_numberofplayers.Create("99",WS_VISIBLE|WS_CHILD|SS_RIGHT,t_rect2,this,IDC_STATIC);

    
    m_static1.SetFont(&m_font);
    m_static2.SetFont(&m_font);
    m_timeconnected.SetFont(&m_font);
    m_numberofplayers.SetFont(&m_font);

    //CHECKBOX
    t_rect.SetRect(0,0,10,12);
    t_rect.OffsetRect(SERVER_VIEW_WIDTH-12,SERVER_VIEW_HEIGHT-14);
    t_return|=m_moredisplaycontrol.Create("",WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,t_rect,this,IDC_DISPLAY);
    m_moredisplaycontrol.SetCheck(0);

    //EDITBOX
    t_rect.SetRect(0,44,SERVER_VIEW_WIDTH,SERVER_VIEW_HEIGHT_BIG-24);
    t_return|=m_editcontrol.Create(WS_BORDER|WS_CHILD|WS_VISIBLE|ES_READONLY|ES_AUTOVSCROLL|ES_MULTILINE|ES_WANTRETURN|ES_AUTOHSCROLL|WS_HSCROLL|WS_VSCROLL
        ,t_rect,this,IDC_MESSAGEDISPLAY);
    m_editcontrol.SetFont(&m_font);
    m_editcontrol.SetWindowText("");
    t_rect.top=t_rect.bottom+2;
    t_rect.bottom=t_rect.top+20;
    t_return|=m_messagecontrol.Create(WS_BORDER|WS_CHILD|WS_VISIBLE|ES_AUTOVSCROLL|ES_LEFT
        ,t_rect,this,IDC_MESSAGEDISPLAY);
    m_messagecontrol.SetFont(&m_font);
    return t_return;
}



int
CXEvilServerStatus::getWindowList(CTypedPtrList<CPtrList,CWnd *>&p_windowlist)
{
    p_windowlist.AddTail(&m_startserver);
    p_windowlist.AddTail(&m_stopserver);
    p_windowlist.AddTail(&m_settings);
/*    p_windowlist.AddTail(&m_numberofplayers);
    p_windowlist.AddTail(&m_timeconnected);
    p_windowlist.AddTail(&m_moredisplaycontrol);
    p_windowlist.AddTail(&m_editcontrol);
    p_windowlist.AddTail(&m_messagecontrol);*/

    return p_windowlist.GetCount();
}



void
CXEvilServerStatus::OnPaint()
{
    CDC *t_dc;
    BOOL t_beginpaint=FALSE;
    PAINTSTRUCT t_struct;
    if (GetUpdateRect(NULL))
    {
        t_beginpaint=TRUE;
        t_dc=BeginPaint(&t_struct);
    }
    else
        t_dc=GetDC();

    CBrush *t_oldbrush=(CBrush *)t_dc->SelectStockObject(LTGRAY_BRUSH);
    CRect t_rect(t_struct.rcPaint);
    t_dc->PatBlt(t_rect.left,t_rect.top,t_rect.Width(),t_rect .Height(),PATCOPY);
    t_dc->SelectObject(t_oldbrush);

    if (t_beginpaint)
        EndPaint(&t_struct);
}


BOOL
CXEvilServerStatus::OnSetCursor( CWnd *p_wnd, UINT, UINT )
{
    POINT t_point;
    CRect t_rect;
    ::GetCursorPos(&t_point);
    GetWindowRect(t_rect);
    if (t_rect.PtInRect(t_point))
    {
        if (::GetCursor()!=m_cursor)
                m_oldcursor=::SetCursor(m_cursor);
    }
    else
    {
        if (::GetCursor()==m_cursor)
                ::SetCursor(m_oldcursor);
    }

    return TRUE;
}


void
CXEvilServerStatus::toggleDisplay()
{
    CRect t_crect;
    CRect t_wrect;
    int t_ydiff;
    int t_xdiff;
    GetClientRect(t_crect);
    GetWindowRect(t_wrect);
    t_xdiff=t_wrect.Width()-SERVER_VIEW_WIDTH;
    switch(m_togglestate)
    {
    case SMALL:
        m_togglestate=BIG;
        t_ydiff=t_wrect.Height()-SERVER_VIEW_HEIGHT;
        t_wrect.SetRect(t_wrect.left,t_wrect.top,t_wrect.left+SERVER_VIEW_WIDTH+t_xdiff,t_wrect.top+SERVER_VIEW_HEIGHT_BIG+t_ydiff);
        break;
    case BIG:
        m_togglestate=SMALL;
        t_ydiff=t_wrect.Height()-SERVER_VIEW_HEIGHT_BIG;
        t_wrect.SetRect(t_wrect.left,t_wrect.top,t_wrect.left+SERVER_VIEW_WIDTH+t_xdiff,t_wrect.top+SERVER_VIEW_HEIGHT+t_ydiff);
        break;
    default:
        assert(0);
    }
    MoveWindow(t_wrect,TRUE);
    Invalidate();
}



void
CXEvilServerStatus::getMessage(CString &p_string)
{
    m_messagecontrol.GetWindowText(p_string);
}



void
CXEvilServerStatus::clearMessage()
{
    m_messagecontrol.SetWindowText("");
}


void 
CXEvilServerStatus::addLog(const CString &p_string)
{
    CString t_string;
    m_editcontrol.GetWindowText(t_string);
    t_string+=p_string;
    if (t_string.GetLength()>12000)
        t_string=t_string.Right(6000);
    t_string+="\r\n";
    m_editcontrol.SetWindowText(t_string);
    int t_int=m_editcontrol.GetLineCount();
    int t_int2=m_editcontrol.GetFirstVisibleLine();
    m_editcontrol.LineScroll(t_int-t_int2);

}



void 
CXEvilServerStatus::clearLog()
{
    m_editcontrol.SetWindowText("");
    int t_int2=m_editcontrol.GetFirstVisibleLine();
    m_editcontrol.LineScroll(-1*t_int2);
}



void 
CXEvilServerStatus::setTimeSpan(CString &p_string)
{
  m_timeconnected.SetWindowText(p_string);
}



void 
CXEvilServerStatus::setNumPlayers(int p_numplayers)
{
  CString t_string;
  itoa(p_numplayers,t_string.GetBuffer(255),10);
  t_string.ReleaseBuffer();
  m_numberofplayers.SetWindowText(t_string);
}


