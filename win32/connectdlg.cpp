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

// ConnectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "xevil.h"
#include "ConnectDlg.h"
#include "role.h"
#include "xetp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConnectDlg dialog


CConnectDlg::CConnectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CConnectDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConnectDlg)
	m_hostname = _T("");
  m_port = XETP::DEFAULT_PORT;
	m_name = _T("");
	//}}AFX_DATA_INIT
}


void CConnectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConnectDlg)
	DDX_Text(pDX, IDC_EDIT1, m_hostname);
	DDX_Text(pDX, IDC_EDIT2, m_port);
	DDV_MinMaxUInt(pDX, m_port, 1, 65535);
	DDX_Text(pDX, IDC_EDIT3, m_name);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConnectDlg, CDialog)
	//{{AFX_MSG_MAP(CConnectDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConnectDlg message handlers

void CConnectDlg::OnOK() 
{
#ifdef MODELESS_DIALOGS
  PostMessage(WM_COMMAND,IDOK,0);
#else
	CDialog::OnOK();
#endif
}



void CConnectDlg::OnCancel() 
{
#ifdef MODELESS_DIALOGS
  PostMessage(WM_COMMAND,IDCANCEL,0);
#else
	CDialog::OnCancel();
#endif
}
