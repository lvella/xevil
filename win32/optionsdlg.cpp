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

// OptionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "XEvil.h"
#include "OptionsDlg.h"
#include "locator.h"
#include "game_style.h"
#include "role.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// OptionsDlg dialog


OptionsDlg::OptionsDlg(CWnd* pParent /*=NULL*/)
    : CDialog(OptionsDlg::IDD, pParent)
{
  //{{AFX_DATA_INIT(OptionsDlg)
  m_gamestyle = 0;
  m_nummachines = 0;
  m_roomshigh = 0;
  m_roomswide = 0;
  m_musictype = 2;
  m_soundonoff = FALSE;
  m_cooperative = FALSE;
  m_refillmachines = FALSE;
	m_human = FALSE;
	//}}AFX_DATA_INIT
}


void OptionsDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(OptionsDlg)
  DDX_Control(pDX, IDC_SOUNDTRACKVOL, m_soundtrackslider);
  DDX_Control(pDX, IDC_GAMESPEED, m_gamespeedslider);
  DDX_Control(pDX, IDC_EFFECTVOL, m_effectsslider);
  DDX_Radio(pDX, IDC_GAMESTYLE2, m_gamestyle);
  DDX_Text(pDX, IDC_NUMMACHINES, m_nummachines);
  DDV_MinMaxInt(pDX, m_nummachines, 0, 500);
  DDX_Text(pDX, IDC_ROOMHEIGHT, m_roomshigh);
  DDV_MinMaxInt(pDX, m_roomshigh, 0, 10);
  DDX_Text(pDX, IDC_ROOMWIDTH, m_roomswide);
  DDV_MinMaxInt(pDX, m_roomswide, 0, 10);
  DDX_Radio(pDX, IDC_SOUNDTYPE, m_musictype);
  DDX_Check(pDX, IDC_SOUNDONOFF, m_soundonoff);
  DDX_Check(pDX, IDC_COOPERATIVE, m_cooperative);
  DDX_Check(pDX, IDC_REFILLMACHINES, m_refillmachines);
	DDX_Check(pDX, IDC_HUMAN, m_human);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(OptionsDlg, CDialog)
  //{{AFX_MSG_MAP(OptionsDlg)
	ON_BN_CLICKED(IDC_STYLEDUEL, OnStyleChanged)
	ON_BN_CLICKED(IDC_STYLEEXTENDED, OnStyleChanged)
	ON_BN_CLICKED(IDC_STYLEKILL, OnStyleChanged)
	ON_BN_CLICKED(IDC_STYLELEVELS, OnStyleChanged)
	ON_BN_CLICKED(IDC_STYLESCENARIOS, OnStyleChanged)
	ON_BN_CLICKED(IDC_STYLETRAINING, OnStyleChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// OptionsDlg message handlers

BOOL OptionsDlg::OnInitDialog() 
{
  CDialog::OnInitDialog();
  CenterWindow();

  m_gamestyle = styleToIndex(m_Dstyle);

  m_nummachines=m_Dnummachines;
  m_refillmachines=m_Drefillmachines;

#if 0
  // Make sure the number is within the range for the UI, or
  // we won't be able to quit the dialog if humansNum is disabled.
  if (m_Dnumhumans >= 0 && m_Dnumhumans < 2) {
    m_numhumans=m_Dnumhumans;
  }
  else {
    m_numhumans=1;
  }
#endif
  m_human = m_Dhuman;

  m_musictype=m_Dmusictype;
  m_soundonoff=m_Dsoundonoff;
  m_cooperative=m_Dcooperative;
  m_roomshigh=m_Droomshigh;
  m_roomswide=m_Droomswide;
  m_effectsslider.SetRange(0,20);
  m_effectsslider.SetLineSize(1);
  m_effectsslider.SetPageSize(2);
  m_effectsslider.SetPos((5000+m_Deffectsvolume)/250);
  m_soundtrackslider.SetRange(0,20);
  m_soundtrackslider.SetLineSize(1);
  m_soundtrackslider.SetPageSize(2);
  m_soundtrackslider.SetPos((5000+m_Dsndtrkvolume)/250);
  m_gamespeedslider.SetRange(0,10);
  m_gamespeedslider.SetLineSize(1);
  m_gamespeedslider.SetPageSize(2);
  m_gamespeedslider.SetPos((100-m_Dgamespeed)/10);
  UpdateData(FALSE);

  SetEnabledByStyleAndRole(m_Dstyle,m_Droletype);

  return TRUE;    // return TRUE unless you set the focus to a control
}

void OptionsDlg::OnOK() {
  UpdateData(TRUE);
  m_Dstyle = indexToStyle(m_gamestyle);
  m_Dnummachines=m_nummachines;
  m_Drefillmachines=m_refillmachines;
  m_Dhuman=m_human;
  m_Droomswide=m_roomswide;
  m_Droomshigh=m_roomshigh;
  m_Dmusictype=(UIsettings::SOUNDTRACKTYPE)m_musictype;
  m_Dsoundonoff=m_soundonoff;
  m_Dcooperative=m_cooperative;

  m_Dgamespeed=100-10*m_gamespeedslider.GetPos();
  m_Deffectsvolume=250*m_effectsslider.GetPos()-5000;
  if (m_Deffectsvolume== -5000)
    m_Deffectsvolume= -10000;
  m_Dsndtrkvolume=250*m_soundtrackslider.GetPos()-5000;
  if (m_Dsndtrkvolume== -5000)
    m_Dsndtrkvolume= -10000;

#ifdef MODELESS_DIALOGS
  PostMessage(WM_COMMAND,IDOK,0);
#else
  CDialog::OnOK();
#endif
}

void OptionsDlg::OnCancel() 
{
#ifdef MODELESS_DIALOGS
  PostMessage(WM_COMMAND,IDCANCEL,0);
#else
  CDialog::OnCancel();
#endif
}

void OptionsDlg::OnStyleChanged() 
{
  UpdateData(TRUE);
  GameStyleType style = indexToStyle(m_gamestyle);
  SetEnabledByStyleAndRole(style,m_Droletype);
}

void OptionsDlg::SetEnabledByStyleAndRole(GameStyleType style,RoleType roleType) {
  // Ask the GameStyle and Role if the various UI items are used for this
  // this game style/role combination.
  Boolean enabled;

  // EnemiesNum
  CWnd* enemiesNum = GetDlgItem(IDC_NUMMACHINES);
  ASSERT(enemiesNum);
  enabled = Role::uses_enemies_num(roleType) &&
            GameStyle::uses_enemies_num(style);
  enemiesNum->EnableWindow(enabled);

  // EnemiesRefill
  CWnd* enemiesRefill = GetDlgItem(IDC_REFILLMACHINES);
  ASSERT(enemiesRefill);
  enabled = Role::uses_enemies_refill(roleType) &&
            GameStyle::uses_enemies_refill(style);
  enemiesRefill->EnableWindow(enabled);

  // GameStyle
  CWnd* levels = GetDlgItem(IDC_STYLELEVELS);
  CWnd* scenarios = GetDlgItem(IDC_STYLESCENARIOS);
  CWnd* kill = GetDlgItem(IDC_STYLEKILL);
  CWnd* duel = GetDlgItem(IDC_STYLEDUEL);
  CWnd* extended = GetDlgItem(IDC_STYLEEXTENDED);
  CWnd* training = GetDlgItem(IDC_STYLETRAINING);
  ASSERT(levels && scenarios && kill && duel && extended && training);
  enabled = Role::uses_game_style(roleType);
  levels->EnableWindow(enabled);
  scenarios->EnableWindow(enabled);
  kill->EnableWindow(enabled);
  duel->EnableWindow(enabled);
  extended->EnableWindow(enabled);
  training->EnableWindow(enabled);

  // HumansNum
  CWnd* humansNum = GetDlgItem(IDC_HUMAN);
  ASSERT(humansNum);
  humansNum->EnableWindow(Role::uses_humans_num(roleType));
  
  // Cooperative
  CWnd* cooperative = GetDlgItem(IDC_COOPERATIVE);
  ASSERT(cooperative);
  cooperative->EnableWindow(Role::uses_cooperative(roleType));
}

void OptionsDlg::setNumHumans(int p_int) {
  m_Dhuman = (p_int != 0);
}

int OptionsDlg::getNumHumans(){
  if (m_Dhuman) {
    return 1;
  }
  return 0;
}

GameStyleType OptionsDlg::indexToStyle(int index) {
  switch (index) {
    case 0: 
      return LEVELS;
    case 1: 
      return SCENARIOS;
    case 2: 
      return KILL;
    case 3: 
      return DUEL;
    case 4: 
      return EXTENDED;
    case 5: 
      return TRAINING;
    default:
      assert(FALSE);
      return LEVELS;
  }
}

int OptionsDlg::styleToIndex(GameStyleType style) {
  switch (style) {
  case LEVELS : 
    return 0;
  case SCENARIOS: 
    return 1;
  case KILL :
    return 2;
  case DUEL :
    return 3;
  case EXTENDED :
    return 4;
  case TRAINING :
    return 5;
  default:
    assert(FALSE);
    return 0;
  }
}

