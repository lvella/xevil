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

// RunServerDlg.h : header file
//
#include "utils.h"

/////////////////////////////////////////////////////////////////////////////
// CRunServerDlg dialog

class CRunServerDlg : public CDialog
{
// Construction
public:
	CRunServerDlg(CWnd* pParent = NULL);   // standard constructor

  Boolean get_local_human();
  CMN_PORT get_port() {return m_port;}
  const char* get_name() {return m_name;}

// Dialog Data
	//{{AFX_DATA(CRunServerDlg)
	enum { IDD = IDD_RUNSERVER };
	CEdit	m_nameControl;
	UINT	m_port;
	int		m_playerObserver;
	CString	m_name;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRunServerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
private:

	// Generated message map functions
	//{{AFX_MSG(CRunServerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPlayer();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

  void UpdateEnabled();
};
