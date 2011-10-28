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

// OptionsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// OptionsDlg dialog
#include "ui.h"

class OptionsDlg : public CDialog
{
// Construction
public:
  OptionsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
  //{{AFX_DATA(OptionsDlg)
	enum { IDD = IDD_OPTIONS };
  CSliderCtrl	m_soundtrackslider;
  CSliderCtrl	m_gamespeedslider;
  CSliderCtrl	m_effectsslider;
  int	    m_gamestyle;
  int	    m_nummachines;
  int	    m_roomshigh;
  int	    m_roomswide;
  int     m_musictype;
  BOOL    m_soundonoff;
  BOOL    m_cooperative;
  BOOL    m_refillmachines;
	BOOL	m_human;
	//}}AFX_DATA


// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(OptionsDlg)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:

  // Generated message map functions
  //{{AFX_MSG(OptionsDlg)
  virtual BOOL OnInitDialog();
  virtual void OnOK();
  virtual void OnCancel();
	afx_msg void OnStyleChanged();
	//}}AFX_MSG
  DECLARE_MESSAGE_MAP()
private:
  // To convert between GameStyleType and the index used for the
  // dialog's game style radio boxes.
  GameStyleType indexToStyle(int index);
  int styleToIndex(GameStyleType);

  // Set fields to be enabled/disabled according to the game style.
  void SetEnabledByStyleAndRole(GameStyleType,RoleType);

  BOOL m_Dhuman;
  int m_Dnummachines;
  BOOL m_Drefillmachines;
  GameStyleType m_Dstyle;
  RoleType m_Droletype;
  int m_Droomshigh;
  int m_Droomswide;
  BOOL m_Dsoundonoff;
  BOOL m_Dcooperative;
  int m_Deffectsvolume;
  int m_Dsndtrkvolume;
  long m_Dgamespeed;
  UIsettings::SOUNDTRACKTYPE m_Dmusictype;

public:
//accessmethods
  void setNumHumans(int p_int);
  void setNumMachines(int p_int){m_Dnummachines=p_int;}
  void setRefillMachines(BOOL p_bool){m_Drefillmachines=p_bool;}
  void setSndTrkVolume(int p_uint){m_Dsndtrkvolume=p_uint;}
  void setEffectsVolume(int p_uint){m_Deffectsvolume=p_uint;}
  void setStyle(GameStyleType p_style){m_Dstyle=p_style;}
  void setRoleType(RoleType p_roletype){m_Droletype=p_roletype;}
  void setRoomsWide(int p_int){m_Droomswide=p_int;}
  void setRoomsHigh(int p_int){m_Droomshigh=p_int;}
  void setGameSpeed(long p_short){m_Dgamespeed=p_short;}
  void setMusicType(UIsettings::SOUNDTRACKTYPE p_type){m_Dmusictype=p_type;}
  void setSoundOnOff(BOOL p_bool){m_Dsoundonoff=p_bool;}
  void setCooperative(BOOL p_bool){m_Dcooperative=p_bool;}
  int getSoundOnOff(){return m_Dsoundonoff;}
  int getNumHumans();
  int getNumMachines(){return m_Dnummachines;}
  BOOL getRefillMachines(){return m_Drefillmachines;}
  GameStyleType getStyle(){return m_Dstyle;}
  int getRoomsWide(){return m_Droomswide;}
  int getRoomsHigh(){return m_Droomshigh;}
  int getSndTrkVolume(){return m_Dsndtrkvolume;}
  int getEffectsVolume(){return m_Deffectsvolume;}
  long getGameSpeed(){return m_Dgamespeed;}
  BOOL getCooperative(){return m_Dcooperative;}
  UIsettings::SOUNDTRACKTYPE getMusicType(){return m_Dmusictype;}
};
