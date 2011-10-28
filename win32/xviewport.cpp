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

 //XViewPort.cpp
#include "stdafx.h"
#include "XEvil.h"//for resources
#include "XViewPort.h"
#include "xdata.h"
#include "coord.h"

#define STATUSBAR_HEIGHT 36


BEGIN_MESSAGE_MAP( CXEvilWnd, CWnd )
	//{{AFX_MSG_MAP( CXEvilWnd )
    ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
    ON_WM_TIMER()
    ON_WM_ACTIVATEAPP()
    ON_WM_SETFOCUS()
    ON_WM_QUERYNEWPALETTE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP( )


ATOM CXEvilWnd::atom;

void CXEvilWnd::OnDestroy( )
{
	m_valid=FALSE;
}

void CXEvilWnd::OnClose( )
{
	m_valid=FALSE;
}
void CXEvilWnd::OnPaint( )
{
	CPaintDC t_dc(this);
	if (IsIconic())
	{
		CWinApp *t_app=AfxGetApp();
		assert(t_app);
		t_dc.DrawIcon(0,0,t_app->LoadIcon(IDR_XEVIL));
	}
}



void 
CXEvilWnd::OnTimer( UINT p_id)
{
    m_timerid=p_id;
}



CXEvilWnd::~CXEvilWnd()
{

}


void
CXEvilWnd::OnSetFocus(CWnd *p_oldwnd)
{
    HDC t_hdc=::GetDC(HWND_DESKTOP);
    UINT t_nummapped=::RealizePalette(t_hdc);
    ::ReleaseDC(HWND_DESKTOP,t_hdc);
    if (m_fullscreen)
        m_querynewpalette=TRUE;
    COUTRESULT("setfocus");
}



BOOL 
CXEvilWnd::OnQueryNewPalette()
{
    m_querynewpalette=TRUE;
    //return true;
    return CWnd::OnQueryNewPalette();  
}



void
CXEvilWnd::OnActivateApp( BOOL bActive, HTASK hTask )
{
    if (bActive)
    {
      COUTRESULT("ACTIVE");
      m_activated=WMACTIVATED;
      if (m_fullscreen) {
        Xvars::enable_graphics(True);
      }
    }
    else
    {
      COUTRESULT("INACTIVE");
      m_activated=WMINACTIVE;
      if (m_fullscreen) {
        Xvars::enable_graphics(False);
      }
    }
    if (m_fullscreen)
        m_querynewpalette=TRUE;
} 



BOOL
CXEvilWnd::Create(CRect &p_rect,CWnd *p_parent,unsigned char p_viewportnum,BOOL p_fullscreenmode)
{
    HWND t_hwnd(NULL);
    m_fullscreen=p_fullscreenmode;
    if (p_parent)
        t_hwnd=p_parent->m_hWnd;
    CMenu t_menu;
    VERIFY(t_menu.LoadMenu(IDR_XEVILMENU));
    if (p_fullscreenmode)
            t_menu.ModifyMenu(ID_FULLSCREEN,MF_BYCOMMAND|MF_STRING,ID_WINDOWSCREEN,"&Window Mode");
    //add on status bar size
    p_rect.bottom+=STATUSBAR_HEIGHT;

    WNDCLASSEX wndClass;
    if (!CXEvilWnd::atom)
    {
        wndClass.cbSize=sizeof(WNDCLASSEX);
        wndClass.style=CS_BYTEALIGNCLIENT|CS_VREDRAW | CS_HREDRAW;
        wndClass.lpfnWndProc=::DefWindowProc;
        wndClass.cbClsExtra=0;
        wndClass.cbWndExtra=0;
                wndClass.hInstance=AfxGetInstanceHandle();
        wndClass.hIcon=theApp.LoadIcon(IDR_XEVIL);
        wndClass.hCursor=theApp.LoadCursor(IDC_XEVIL);
        wndClass.hbrBackground=NULL;
        wndClass.lpszMenuName=NULL;
        wndClass.lpszClassName="XEvilViewPortClass";
        wndClass.hIconSm=theApp.LoadIcon(IDR_XEVIL_SMALL);
#if 0
        OSVERSIONINFO versionInfo;
        versionInfo.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
        VERIFY(GetVersionEx(&versionInfo));
/*
        char crap[255];
        ltoa(crap,versionInfo.dwMajorVersion*/
        if (VER_PLATFORM_WIN32_WINDOWS != versionInfo.dwPlatformId)
        {
                AfxMessageBox("Not Windows 95");
                wndClass.hIconSm=theApp.LoadIcon(IDR_XEVIL_SMALL);
        }
        else
                wndClass.hIconSm=NULL;
#endif //looking for solution to win95 problem, it was the hInstance
        CXEvilWnd::atom=::RegisterClassEx(&wndClass);
        if (!CXEvilWnd::atom)
        {
            assert(FALSE);
            AfxMessageBox("Failed to register window class, XEvilViewPort");
            return FALSE;
        }
    //        s_classname=AfxRegisterWndClass(CS_BYTEALIGNCLIENT|CS_VREDRAW | CS_HREDRAW,theApp.LoadCursor(IDC_XEVIL),0,theApp.LoadIcon(IDR_XEVIL));
    }
    BOOL t_return;

    // Window mode    
    if (! p_fullscreenmode)
    {
        t_return= CWnd::CreateEx(
            WS_EX_WINDOWEDGE,
            "XEvilViewPortClass",
            "XEvil",
            WS_MINIMIZEBOX |        // so we dont have to call ShowWindow
            WS_SYSMENU |        // so we get a icon in in our tray button
            WS_CAPTION|
            WS_POPUPWINDOW,
            p_rect.left,
            p_rect.top,
            p_rect.Width(),
            p_rect.Height(),
            t_hwnd/*parent*/,
            NULL/*DorHMenu*/,
            NULL/*lpParam*/);

        SetMenu(&t_menu);
        t_menu.Detach();
        CRect t_rect;
        GetClientRect(t_rect);
        t_rect.top=t_rect.bottom-STATUSBAR_HEIGHT;
//        m_statusbar.Create("status",WS_CHILD|WS_VISIBLE|BS_OWNERDRAW,t_rect,this,ID_STATUSBAR);
    }
    // Full screen mode.
    else
    {
        OSVERSIONINFO versionInfo;
        versionInfo.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
        VERIFY(GetVersionEx(&versionInfo));
        if (VER_PLATFORM_WIN32_WINDOWS != versionInfo.dwPlatformId)
        {
            t_return= CWnd::CreateEx(WS_EX_APPWINDOW|WS_EX_TOPMOST,"XEvilViewPortClass","XEvil",WS_POPUP|WS_VISIBLE
                    //WS_SYSMENU|WS_MINIMIZEBOX|WS_CAPTION|WS_POPUPWINDOW
            ,p_rect.left,p_rect.top,p_rect.Width(),p_rect.Height(),
            t_hwnd/*parent*/,NULL/*DorHMenu*/,NULL/*lpParam*/);
        }
        else
        {
            t_return= CWnd::CreateEx(WS_EX_APPWINDOW|WS_VISIBLE,"XEvilViewPortClass","XEvil",WS_POPUP
                    //WS_SYSMENU|WS_MINIMIZEBOX|WS_CAPTION|WS_POPUPWINDOW
            ,p_rect.left,p_rect.top,p_rect.Width(),p_rect.Height(),
            t_hwnd/*parent*/,NULL/*DorHMenu*/,NULL/*lpParam*/);
        }
        SetMenu(&t_menu);
        t_menu.Detach();

#if 0
        // Clear screen
        CRect t_rect;
        GetClientRect(t_rect);
        CDC *t_dc = GetDC();
        if (t_dc) {
          t_dc->FillSolidRect(t_rect,RGB(0,255,0));
          ReleaseDC(t_dc);
        }
        else {
          COUTRESULT("COULDNT GET DC");
        }
#endif
    }
    m_valid=t_return;
    return t_return;
}



void
CXEvilButton::DrawItem(LPDRAWITEMSTRUCT p_struct)
{
        if (!p_struct->itemAction==ODA_DRAWENTIRE)
                return;
        CDC *t_dc=CDC::FromHandle(p_struct->hDC);
        CBrush *t_oldbrush=(CBrush *)t_dc->SelectStockObject(LTGRAY_BRUSH);
        CRect t_rect(p_struct->rcItem);
        t_dc->PatBlt(t_rect.left,t_rect.top,t_rect.Width(),t_rect .Height(),PATCOPY);
        t_dc->SelectObject(t_oldbrush);
        CPen *t_oldpen=(CPen *)t_dc->SelectStockObject(WHITE_PEN);
        t_dc->MoveTo(t_rect.right-1,0);
        t_dc->LineTo(0,0);
        t_dc->LineTo(0,t_rect.bottom-1);
        CPen t_newpen(PS_SOLID,1,RGB(96,96,96));
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
}

//note! there may be a '\n' in the string, find it and put the text on the next line!
void
CXEvilIndentedButton::DrawItem(LPDRAWITEMSTRUCT p_struct)
{
        if (!p_struct->itemAction==ODA_DRAWENTIRE)
                return;
        CPen t_newpen(PS_SOLID,1,RGB(96,96,96));

        CDC *t_dc=CDC::FromHandle(p_struct->hDC);
        CBrush *t_oldbrush=(CBrush *)t_dc->SelectStockObject(LTGRAY_BRUSH);
        CRect t_rect(p_struct->rcItem);
        t_dc->PatBlt(t_rect.left,t_rect.top,t_rect.Width(),t_rect .Height(),PATCOPY);
        t_dc->SelectObject(t_oldbrush);
        t_dc->SelectObject(&t_newpen);
        t_dc->MoveTo(t_rect.right-2,2);
        t_dc->LineTo(1,2);
        t_dc->LineTo(1,t_rect.bottom-2);

        CPen *t_oldpen=(CPen *)t_dc->SelectStockObject(WHITE_PEN);
        t_dc->MoveTo(t_rect.right-2,2);
        t_dc->LineTo(t_rect.right-2,t_rect.bottom-2);
        t_dc->LineTo(1,t_rect.bottom-2);
        t_dc->SelectObject(t_oldpen);//put it back

		CString t_string;
		CString t_string2;
		GetWindowText(t_string.GetBuffer(255),255);
		t_string.ReleaseBuffer();
//find the /n
		int t_index=t_string.Find('\n');
		if (-1!=t_index)
		{
		  t_string2=t_string.Mid(t_index+1);
		  t_string=t_string.Mid(0,t_index);
		}

		t_dc->SelectStockObject(BLACK_PEN);
		t_rect.left+=3;
		if (t_string2.GetLength())
		{
		  t_rect.bottom=t_rect.bottom/2;
		  t_rect.top+=3;
		  t_rect.OffsetRect(0,1);
		}
		t_dc->SelectStockObject(ANSI_VAR_FONT);
		t_dc->SetBkMode(OPAQUE);
		t_dc->SetBkColor(RGB(192,192,192));
		t_dc->DrawText(t_string,t_string.GetLength(),t_rect,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
		t_rect.OffsetRect(0,t_rect.bottom-3);
		t_rect.top-=3;
		t_rect.bottom-=1;
		if (t_string2.GetLength())
		  t_dc->DrawText(t_string2,t_string2.GetLength(),t_rect,DT_LEFT|DT_VCENTER|DT_SINGLELINE);

}
