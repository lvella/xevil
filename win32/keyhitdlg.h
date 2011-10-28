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

// KeyHitDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// KeyHitDlg dialog

class KeyHitDlg : public CDialog
{
// Construction
public:
	KeyHitDlg(CWnd* pParent = NULL);   // standard constructor
    unsigned char getChar(){return m_Dchar;}
// Dialog Data
	//{{AFX_DATA(KeyHitDlg)
	enum { IDD = IDD_KEYHIT };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(KeyHitDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(KeyHitDlg)
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual BOOL OnInitDialog();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
    unsigned char m_Dchar;
    static LRESULT CALLBACK KeyhitKeyboardProc(int code,	WPARAM wParam, LPARAM lParam);
    void keyHit(WPARAM wParam, LPARAM lParam);
    static HHOOK m_hhk;
    static KeyHitDlg *m_hack;
};
