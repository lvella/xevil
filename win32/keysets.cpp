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

// KeySets.cpp : implementation file
//

#include "stdafx.h"
#include "xevil.h"
#include "KeySets.h"
#include "KeyHitDlg.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CKeySets dialog


CKeySets::CKeySets(CWnd* pParent /*=NULL*/)
	: CDialog(CKeySets::IDD, pParent)
{
	//{{AFX_DATA_INIT(CKeySets)
	m_playerstaticvalue = _T("");
	//}}AFX_DATA_INIT
  m_pbutton=NULL;
  m_pchar=NULL;
}


void CKeySets::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CKeySets)
	DDX_Control(pDX, IDC_CHAT, m_chatbutton);
	DDX_Control(pDX, IDC_WEAPONUSE, m_weaponusebutton);
	DDX_Control(pDX, IDC_WEAPONDROP, m_weapondropbutton);
	DDX_Control(pDX, IDC_WEAPONCHANGE, m_weaponchangebutton);
	DDX_Control(pDX, IDC_UR, m_upperrightbutton);
	DDX_Control(pDX, IDC_UP, m_upperbutton);
	DDX_Control(pDX, IDC_UL, m_upperleftbutton);
	DDX_Control(pDX, IDC_R, m_rightbutton);
	DDX_Control(pDX, IDC_LR, m_lowerrightbutton);
	DDX_Control(pDX, IDC_LL, m_lowerleftbutton);
	DDX_Control(pDX, IDC_L, m_leftbutton);
	DDX_Control(pDX, IDC_ITEMUSE, m_itemusebutton);
	DDX_Control(pDX, IDC_ITEMDROP, m_itemdropbutton);
	DDX_Control(pDX, IDC_ITEMCHANGE, m_itemchangebutton);
	DDX_Control(pDX, IDC_DOWN, m_downbutton);
	DDX_Control(pDX, IDC_CENTER, m_centerbutton);
	DDX_Text(pDX, IDC_PLAYERSTATIC, m_playerstaticvalue);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CKeySets, CDialog)
	//{{AFX_MSG_MAP(CKeySets)
	ON_BN_CLICKED(IDC_UP, OnUp)
	ON_BN_CLICKED(IDC_UR, OnUr)
	ON_BN_CLICKED(IDC_WEAPONCHANGE, OnWeaponchange)
	ON_BN_CLICKED(IDC_WEAPONUSE, OnWeaponuse)
	ON_BN_CLICKED(IDC_WEAPONDROP, OnWeapondrop)
	ON_BN_CLICKED(IDC_CENTER, OnCenter)
	ON_BN_CLICKED(IDC_DOWN, OnDown)
	ON_BN_CLICKED(IDC_ITEMCHANGE, OnItemchange)
	ON_BN_CLICKED(IDC_ITEMDROP, OnItemdrop)
	ON_BN_CLICKED(IDC_ITEMUSE, OnItemuse)
	ON_BN_CLICKED(IDC_L, OnL)
	ON_BN_CLICKED(IDC_LL, OnLl)
	ON_BN_CLICKED(IDC_LR, OnLr)
	ON_BN_CLICKED(IDC_R, OnR)
	ON_BN_CLICKED(IDC_UL, OnUl)
	ON_BN_CLICKED(IDC_CHAT, OnChat)
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKeySets message handlers


BOOL CKeySets::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_weaponusebutton.SetWindowText(translateChar(m_Dweaponusechar));
	m_weapondropbutton.SetWindowText(translateChar(m_Dweapondropchar));
	m_weaponchangebutton.SetWindowText(translateChar(m_Dweaponchangechar));
	m_upperrightbutton.SetWindowText(translateChar(m_Dupperrightchar));
	m_upperbutton.SetWindowText(translateChar(m_Dupperchar));
	m_upperleftbutton.SetWindowText(translateChar(m_Dupperleftchar));
	m_rightbutton.SetWindowText(translateChar(m_Drightchar));
	m_lowerrightbutton.SetWindowText(translateChar(m_Dlowerrightchar));
	m_lowerleftbutton.SetWindowText(translateChar(m_Dlowerleftchar));
	m_leftbutton.SetWindowText(translateChar(m_Dleftchar));
	m_itemusebutton.SetWindowText(translateChar(m_Ditemusechar));
	m_itemdropbutton.SetWindowText(translateChar(m_Ditemdropchar));
	m_itemchangebutton.SetWindowText(translateChar(m_Ditemchangechar));
	m_downbutton.SetWindowText(translateChar(m_Ddownchar));
	m_centerbutton.SetWindowText(translateChar(m_Dcenterchar));
	m_chatbutton.SetWindowText(translateChar(m_Dchatchar));

  m_playerstaticvalue="";//m_Dplayerstaticvalue;
  SetWindowText(m_Dplayerstaticvalue);
  UpdateData(FALSE);
  m_changed=FALSE;
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CKeySets::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
#if 0
    // TODO: Add your message handler code here and/or call default
	if (m_pchar)
  {
    *m_pchar=nChar;
    m_pbutton->SetWindowText(translateChar(*m_pchar));
    m_pchar=NULL;
    m_pbutton=NULL;
  }
#endif
	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}



void CKeySets::OnOK() 
{
#ifdef MODELESS_DIALOGS
  PostMessage(WM_COMMAND,IDOK,0);
#else
	CDialog::OnOK();
#endif
}



void CKeySets::OnCancel() 
{
#ifdef MODELESS_DIALOGS
  PostMessage(WM_COMMAND,IDCANCEL,0);
#else
	CDialog::OnCancel();
#endif
}



void CKeySets::OnUp() 
{
	// TODO: Add your control notification handler code here
  KeyHitDlg t_dlg;
  t_dlg.DoModal();
  m_Dupperchar=t_dlg.getChar();
  m_upperbutton.SetWindowText(translateChar(m_Dupperchar));
  m_changed=TRUE;
}

void CKeySets::OnUr() 
{
	// TODO: Add your control notification handler code here
  KeyHitDlg t_dlg;
  t_dlg.DoModal();
  m_Dupperrightchar=t_dlg.getChar();
	m_upperrightbutton.SetWindowText(translateChar(m_Dupperrightchar));
  m_changed=TRUE;
}

void CKeySets::OnWeaponchange() 
{
	// TODO: Add your control notification handler code here
  KeyHitDlg t_dlg;
  t_dlg.DoModal();
  m_Dweaponchangechar=t_dlg.getChar();
  m_weaponchangebutton.SetWindowText(translateChar(m_Dweaponchangechar));
}

void CKeySets::OnWeaponuse() 
{
	// TODO: Add your control notification handler code here
    KeyHitDlg t_dlg;
    t_dlg.DoModal();
    m_Dweaponusechar=t_dlg.getChar();
    m_weaponusebutton.SetWindowText(translateChar(m_Dweaponusechar));
    m_changed=TRUE;
}

void CKeySets::OnWeapondrop() 
{
	// TODO: Add your control notification handler code here
    KeyHitDlg t_dlg;
    t_dlg.DoModal();
    m_Dweapondropchar=t_dlg.getChar();
    m_weapondropbutton.SetWindowText(translateChar(m_Dweapondropchar));
    m_changed=TRUE;
}

void CKeySets::OnCenter() 
{
	// TODO: Add your control notification handler code here
    KeyHitDlg t_dlg;
    t_dlg.DoModal();
    m_Dcenterchar=t_dlg.getChar();
    m_centerbutton.SetWindowText(translateChar(m_Dcenterchar));
    m_changed=TRUE;
}

void CKeySets::OnDown() 
{
	// TODO: Add your control notification handler code here
  KeyHitDlg t_dlg;
  t_dlg.DoModal();
  m_Ddownchar=t_dlg.getChar();
  m_downbutton.SetWindowText(translateChar(m_Ddownchar));
  m_changed=TRUE;
}

void CKeySets::OnItemchange() 
{
	// TODO: Add your control notification handler code here
  KeyHitDlg t_dlg;
  t_dlg.DoModal();
  m_Ditemchangechar=t_dlg.getChar();
  m_itemchangebutton.SetWindowText(translateChar(m_Ditemchangechar));
  m_changed=TRUE;
}

void CKeySets::OnItemdrop() 
{
	// TODO: Add your control notification handler code here
  KeyHitDlg t_dlg;
  t_dlg.DoModal();
  m_Ditemdropchar=t_dlg.getChar();
  m_itemdropbutton.SetWindowText(translateChar(m_Ditemdropchar));
  m_changed=TRUE;
}

void CKeySets::OnItemuse() 
{
	// TODO: Add your control notification handler code here
  KeyHitDlg t_dlg;
  t_dlg.DoModal();
  m_Ditemusechar=t_dlg.getChar();
  m_itemusebutton.SetWindowText(translateChar(m_Ditemusechar));
  m_changed=TRUE;
}

void CKeySets::OnL() 
{
	// TODO: Add your control notification handler code here
  KeyHitDlg t_dlg;
  t_dlg.DoModal();
  m_Dleftchar=t_dlg.getChar();
  m_leftbutton.SetWindowText(translateChar(m_Dleftchar));
  m_changed=TRUE;
}

void CKeySets::OnLl() 
{
	// TODO: Add your control notification handler code here
  KeyHitDlg t_dlg;
  t_dlg.DoModal();
  m_Dlowerleftchar=t_dlg.getChar();
  m_lowerleftbutton.SetWindowText(translateChar(m_Dlowerleftchar));
  m_changed=TRUE;
}

void CKeySets::OnLr() 
{
	// TODO: Add your control notification handler code here
  KeyHitDlg t_dlg;
  t_dlg.DoModal();
  m_Dlowerrightchar=t_dlg.getChar();
  m_lowerrightbutton.SetWindowText(translateChar(m_Dlowerrightchar));
  m_changed=TRUE;
}

void CKeySets::OnR() 
{
	// TODO: Add your control notification handler code here
  KeyHitDlg t_dlg;
  t_dlg.DoModal();
  m_Drightchar=t_dlg.getChar();
  m_rightbutton.SetWindowText(translateChar(m_Drightchar));
  m_changed=TRUE;
}

void CKeySets::OnUl() 
{
	// TODO: Add your control notification handler code here
  KeyHitDlg t_dlg;
  t_dlg.DoModal();
  m_Dupperleftchar=t_dlg.getChar();
  m_upperleftbutton.SetWindowText(translateChar(m_Dupperleftchar));
  m_changed=TRUE;
}


void CKeySets::OnChat() 
{
	// TODO: Add your control notification handler code here
  KeyHitDlg t_dlg;
  t_dlg.DoModal();
  m_Dchatchar=t_dlg.getChar();
  m_chatbutton.SetWindowText(translateChar(m_Dchatchar));
  m_changed=TRUE;	
}

CString
CKeySets::translateChar(unsigned char p_char)
{
/*  if (IsCharAlphaNumeric(p_char))
  {
    return CString(p_char);
  }*/
  switch (p_char)
  {
    case 8:   return "BK SPACE";
    case 9:   return "TAB";
    case 13:  return "Enter";
    case 16:  return "SHIFT";
    case 17:  return "CNTRL";
    case 27:  return "ESC";
    case 32:  return "SPACE";
    case 33:  return "PgUp";
    case 34:  return "PgDn";
    case 35:  return "End";
    case 36:  return "Home";
    case 37:  return "Lt Arrow";
    case 38:  return "Up Arrow";
    case 39:  return "Rt Arrow";
    case 40:  return "Dn Arrow";
    case 45:  return "Insert";
    case 46:  return "Del";
    case 96:  return "Num 0";
    case 97:  return "Num 1";
    case 98:  return "Num 2";
    case 99:  return "Num 3";
    case 100: return "Num 4";
    case 101: return "Num 5";
    case 102: return "Num 6";
    case 103: return "Num 7";
    case 104: return "Num 8";
    case 105: return "Num 9";
    case 107: return "Num +";
    case 109: return "Num -";
    case 110: return "Num .";
    case 111: return "Num /";
    case 112: return "F1";
    case 113: return "F2";
    case 114: return "F3";
    case 115: return "F4";
    case 116: return "F5";
    case 117: return "F6";
    case 118: return "F7";
    case 119: return "F8";
    case 120: return "F9";
    case 121: return "F10";
    case 122: return "F11";
    case 123: return "F12";
    case 170: return '.';       //'.' is NOT 190!
    case 186: return "'";
    case 187: return "=";
    case 188: return ",";
    case 189: return "-";
    case 190: return ".";
    case 191: return "/";
    case 192: return "~";
    case 220: return "\\";
    case 221: return "]";
    case 222: return "[";
    case 18:case 44:case 145: case 19:  return "Invalid";
  }

  return CString(p_char);
}



void 
CKeySets::setUpChar(char p_char)
{
  m_Dupperchar=p_char;
}



void CKeySets::setUpLeftChar(char p_char)
{
  m_Dupperleftchar=p_char;
}



void CKeySets::setUpRightChar(char p_char)
{
  m_Dupperrightchar=p_char;
}



void CKeySets::setLeftChar(char p_char)
{
  m_Dleftchar=p_char;
}



void CKeySets::setRightChar(char p_char)
{
  m_Drightchar=p_char;
}



void CKeySets::setCenterChar(char p_char)
{
  m_Dcenterchar=p_char;
}



void CKeySets::setDownChar(char p_char)
{
  m_Ddownchar=p_char;
}



void CKeySets::setDownLeftChar(char p_char)
{
  m_Dlowerleftchar=p_char;
}



void CKeySets::setDownRightChar(char p_char)
{
  m_Dlowerrightchar=p_char;
}




void CKeySets::setWeaponUse(char p_char)
{
  m_Dweaponusechar=p_char;
}



void CKeySets::setWeaponChange(char p_char)
{
  m_Dweaponchangechar=p_char;
}



void CKeySets::setWeaponDrop(char p_char)
{
  m_Dweapondropchar=p_char;
}



void CKeySets::setItemUse(char p_char)
{
  m_Ditemusechar=p_char;
}



void CKeySets::setItemChange(char p_char)
{
  m_Ditemchangechar=p_char;
}



void CKeySets::setItemDrop(char p_char)
{
  m_Ditemdropchar=p_char;
}

  
  
void CKeySets::setChat(char p_char)
{
  m_Dchatchar=p_char;
}



char CKeySets::getUpChar()
{
  return m_Dupperchar;
}



char CKeySets::getUpLeftChar()
{
  return m_Dupperleftchar;
}



char CKeySets::getUpRightChar()
{
  return m_Dupperrightchar;
}



char CKeySets::getLeftChar()
{
  return m_Dleftchar;
}



char CKeySets::getRightChar()
{
  return m_Drightchar;
}



char CKeySets::getCenterChar()
{
  return m_Dcenterchar;
}



char CKeySets::getDownChar()
{
  return m_Ddownchar;
}



char CKeySets::getDownLeftChar()
{
  return m_Dlowerleftchar;
}



char CKeySets::getDownRightChar()
{
  return m_Dlowerrightchar;
}



char CKeySets::getWeaponUse()
{
  return m_Dweaponusechar;
}



char CKeySets::getWeaponChange()
{
  return m_Dweaponchangechar;
}



char CKeySets::getWeaponDrop()
{
  return m_Dweapondropchar;
}



char CKeySets::getItemUse()
{
  return m_Ditemusechar;
}



char CKeySets::getItemChange()
{
  return m_Ditemchangechar;
}



char CKeySets::getItemDrop()
{
  return m_Ditemdropchar;
}



char CKeySets::getChat()
{
  return m_Dchatchar;
}
