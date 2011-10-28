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

// RunServerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "xevil.h"
#include "RunServerDlg.h"
#include "role.h"
#include "xetp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRunServerDlg dialog


CRunServerDlg::CRunServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRunServerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRunServerDlg)
  m_port = XETP::DEFAULT_PORT;
	m_playerObserver = -1;
	m_name = _T("");
	//}}AFX_DATA_INIT
}



Boolean CRunServerDlg::get_local_human() {
  return m_playerObserver == 0;
}

void CRunServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRunServerDlg)
	DDX_Control(pDX, IDC_EDIT3, m_nameControl);
	DDX_Text(pDX, IDC_EDIT1, m_port);
	DDV_MinMaxUInt(pDX, m_port, 1, 65535);
	DDX_Radio(pDX, IDC_PLAYER, m_playerObserver);
	DDX_Text(pDX, IDC_EDIT3, m_name);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRunServerDlg, CDialog)
	//{{AFX_MSG_MAP(CRunServerDlg)
	ON_BN_CLICKED(IDC_PLAYER, OnPlayer)
	ON_BN_CLICKED(IDC_OBSERVER, OnPlayer)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRunServerDlg message handlers

BOOL CRunServerDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  // Default is player.
  m_playerObserver = 0;
  m_name = "";

  UpdateEnabled();
  UpdateData(FALSE);  

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRunServerDlg::OnPlayer() 
{
  UpdateData(TRUE);
  UpdateEnabled();
}

void CRunServerDlg::UpdateEnabled()
{
  // Only enable the name field if in "Player" mode.
  BOOL enabled = (m_playerObserver == 0);
  m_nameControl.EnableWindow(enabled);
}
