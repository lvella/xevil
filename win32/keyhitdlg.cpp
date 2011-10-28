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

// KeyHitDlg.cpp : implementation file


#include "stdafx.h"
#include "xevil.h"
#include "KeyHitDlg.h"

/*#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif*/

HHOOK KeyHitDlg::m_hhk;
KeyHitDlg * KeyHitDlg::m_hack;

/////////////////////////////////////////////////////////////////////////////
// KeyHitDlg dialog


KeyHitDlg::KeyHitDlg(CWnd* pParent /*=NULL*/)
: CDialog(KeyHitDlg::IDD, pParent)
{
  //{{AFX_DATA_INIT(KeyHitDlg)
  //}}AFX_DATA_INIT
}


void KeyHitDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(KeyHitDlg)
  //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(KeyHitDlg, CDialog)
//{{AFX_MSG_MAP(KeyHitDlg)
ON_WM_KEYUP()
ON_WM_KEYDOWN()
ON_WM_CHAR()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// KeyHitDlg message handlers

void KeyHitDlg::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  // TODO: Add your message handler code here and/or call default
  m_Dchar=nChar;
  CDialog::OnKeyUp(nChar, nRepCnt, nFlags);
  EndDialog(IDOK);
}

void KeyHitDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  // TODO: Add your message handler code here and/or call default
  m_Dchar=nChar;
  EndDialog(IDOK);
  CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL KeyHitDlg::OnInitDialog() 
{
  CDialog::OnInitDialog();

  // TODO: Add extra initialization here
  SetFocus();
  SetCapture();
  m_hack=this;
  m_hhk=SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)KeyhitKeyboardProc,
    AfxGetInstanceHandle(),AfxGetThread()->m_nThreadID);
 

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

void KeyHitDlg::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  // TODO: Add your message handler code here and/or call default
  m_Dchar=nChar;
  EndDialog(IDOK);
  CDialog::OnChar(nChar, nRepCnt, nFlags);
}



void KeyHitDlg::keyHit(WPARAM wParam, LPARAM lParam)
{
  m_Dchar=(unsigned char)wParam;
  EndDialog(IDOK);
}



LRESULT CALLBACK 
KeyHitDlg::KeyhitKeyboardProc(int code,	WPARAM wParam, LPARAM lParam)
{
  UnhookWindowsHookEx(m_hhk);
  m_hack->keyHit(wParam,lParam);
  return ::CallNextHookEx(m_hhk,code,	wParam,lParam); 
}
