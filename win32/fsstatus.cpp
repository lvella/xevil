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

#include "stdafx.h"
#include "XEvil.h"//for resources
#include "XViewPort.h"
#include "xdata.h" //hresult stuff
#include "coord.h"
#include "wheel.h"
#include "fsstatus.h"

// Two rows of buttons are same height as the wheel.
#define BUTTON_HEIGHT (Wheel::WHEEL_ITEM_HEIGHT >> 1)
// Size of the area for displaying level-specific information.
#define LEVEL_WIDTH 215

 

/////////////////////////////////////////
//HEALTHBAR

BOOL
XEvilFSHealthBar::update(Xvars &xvars, LPDIRECTDRAWSURFACE p_buffer,CRect &p_clientarea,BOOL p_forceredraw)
{
  if ((m_pos!=m_postedpos)||(m_maxpos!=m_postedmaxpos)||(p_forceredraw))
  {
    m_pos=m_postedpos;
    m_maxpos=m_postedmaxpos;
    DrawItem(xvars, p_buffer);
    return TRUE;
  }
  return FALSE;
}



void
XEvilFSHealthBar::DrawItem(Xvars &xvars, LPDIRECTDRAWSURFACE p_buffer)
{
  if (m_maxpos<=1)
    return;
  HDC t_pixhdc;
  if (!xvars.GetDC(p_buffer,&t_pixhdc)){
    return;
  }

  // Erasing just text part???
  CDC *t_dc=CDC::FromHandle(t_pixhdc);
  CRect t_rect(m_clientpos);
  t_rect.right=t_rect.left+TEXTINDENT;

  CBrush *t_oldbrush=(CBrush *)t_dc->SelectStockObject(LTGRAY_BRUSH);
  t_dc->PatBlt(t_rect.left,t_rect.top,t_rect.Width(),t_rect .Height(),PATCOPY);
  t_dc->SelectObject(t_oldbrush);


  t_rect.OffsetRect(TEXTINDENT,0);
  //draw a square 
  char t_char[IT_STRING_LENGTH];
  
  t_rect.right=(int)(t_rect.left+((float)m_pos/(float)m_maxpos)*(m_clientpos.Width()-TEXTINDENT));
  CBrush t_solidbrush;
  float r=(float)m_pos/(float)m_maxpos;
  t_solidbrush.CreateSolidBrush(RGB(255*min(2-2*r,1),255*min(2*r,1),0));
  t_dc->SelectObject(&t_solidbrush);
  t_dc->PatBlt(t_rect.left,t_rect.top,t_rect.Width(),t_rect.Height(),PATCOPY);
  t_rect.left=t_rect.right+1;
  t_rect.right=m_clientpos.left+m_clientpos.right;
  if (t_rect.left<t_rect.right)
  {
    t_dc->SelectStockObject(LTGRAY_BRUSH);
    t_dc->PatBlt(t_rect.left,t_rect.top,t_rect.Width(),t_rect .Height(),PATCOPY);
  }
  t_dc->SelectObject(t_oldbrush);

  t_dc->SelectStockObject(BLACK_PEN);
	t_dc->SelectStockObject(ANSI_VAR_FONT);
	t_dc->SetBkMode(OPAQUE);
	t_dc->SetBkColor(RGB(192,192,192));
  CString t_string("Health: ");
  t_string+=itoa(m_health,t_char,10);
  t_rect=m_clientpos;
//  t_rect.OffsetRect(m_clientpos.left,m_clientpos.top);
  t_rect.left+=5;
	t_dc->DrawText(t_string,t_string.GetLength(),t_rect,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
  t_rect.left-=5;
  CPen *t_oldpen=(CPen *)t_dc->SelectStockObject(WHITE_PEN);
  t_dc->MoveTo(t_rect.right-1,t_rect.top);
  t_dc->LineTo(t_rect.left,t_rect.top);
  t_dc->LineTo(t_rect.left,t_rect.bottom-1);
//  t_rect=m_clientpos;
//  t_rect.OffsetRect(m_clientpos.left,m_clientpos.top);
  CPen t_newpen(PS_SOLID,1,RGB(96,96,96));
  t_dc->SelectObject(&t_newpen);
  t_dc->MoveTo(t_rect.right-1,t_rect.top);
  t_dc->LineTo(t_rect.right-1,t_rect.bottom-1);
  t_dc->LineTo(t_rect.left,t_rect.bottom-1);
  t_dc->SelectObject(t_oldpen);//put it back
  p_buffer->ReleaseDC(t_pixhdc);
}


  
/////////////////////////////////////////
//XEvilFSButton
BOOL
XEvilFSButton::update(Xvars &xvars, LPDIRECTDRAWSURFACE p_buffer,CRect &p_clientarea,BOOL p_forceredraw)
{
    if ((m_postedmessage!=m_message)||(p_forceredraw))
    {
        m_message=m_postedmessage;
        DrawItem(xvars, p_buffer,p_clientarea);
        return TRUE;//dirty
    }
    return FALSE; //not dirty
}



void
XEvilFSButton::DrawItem(Xvars &xvars, LPDIRECTDRAWSURFACE p_buffer,CRect &p_clientarea)
{
  HDC t_pixhdc;
  if (!xvars.GetDC(p_buffer,&t_pixhdc)){
    return;
  }

  CDC *t_dc=CDC::FromHandle(t_pixhdc);
  CRect t_rect(m_clientpos);

  t_rect.OffsetRect(p_clientarea.left,p_clientarea.top);

  CBrush *t_oldbrush=(CBrush *)t_dc->SelectStockObject(LTGRAY_BRUSH);
  t_dc->PatBlt(t_rect.left,t_rect.top,t_rect.Width(),t_rect .Height(),PATCOPY);
  t_dc->SelectObject(t_oldbrush);
  CPen *t_oldpen=(CPen *)t_dc->SelectStockObject(WHITE_PEN);
  t_dc->MoveTo(t_rect.right-1,t_rect.top);
  t_dc->LineTo(t_rect.left,t_rect.top);
  t_dc->LineTo(t_rect.left,t_rect.bottom-1);
  CPen t_newpen(PS_SOLID,1,RGB(96,96,96));
  t_dc->SelectObject(&t_newpen);
  t_dc->MoveTo(t_rect.right-1,t_rect.top);
  t_dc->LineTo(t_rect.right-1,t_rect.bottom-1);
  t_dc->LineTo(t_rect.left,t_rect.bottom-1);
  t_dc->SelectObject(t_oldpen);//put it back

	t_dc->SelectStockObject(BLACK_PEN);
	t_dc->SelectStockObject(ANSI_VAR_FONT);
	t_rect.left+=5;
	t_dc->SetBkMode(OPAQUE);
	t_dc->SetBkColor(RGB(192,192,192));
	t_dc->DrawText(m_message,m_message.GetLength(),t_rect,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
  p_buffer->ReleaseDC(t_pixhdc);
}



/////////////////////////////////////////
//XEvilFSStatusMessage
BOOL
XEvilFSStatusMessage::update(Xvars &xvars, LPDIRECTDRAWSURFACE p_buffer,CRect &p_clientarea,
                             BOOL p_forceredraw)
{
  if ((m_postedmessage!=m_message)||(p_forceredraw))
  {
    m_message=m_postedmessage;
    DrawItem(xvars, p_buffer,p_clientarea);
    return TRUE;//dirty
  }
  return FALSE; //not dirty
}



void
XEvilFSStatusMessage::DrawItem(Xvars &xvars, LPDIRECTDRAWSURFACE p_buffer,CRect &p_clientarea)
{
  // We should really get rid of the p_clientarea parameter all together.
  // The upper-left is always (0,0) anyway.

  const int textXIndent = 4;
  const int textYIndent = 0;
  const int lineHeight = m_clientpos.Height() >> 1;

  // Create string for each line.
	int t_index = m_message.Find('\n');
  CString t_string1;
  CString t_string2;
  if (t_index != -1) {
    // We have two lines.
		t_string1 += m_message.Mid(0,t_index);
		t_string2 += m_message.Mid(t_index + 1);
  }
  else {
    // Only one line, t_string2 stays empty.
    t_string1 += m_message;
  }

  // Rect for each line.
  CRect t_rect1(textXIndent,textYIndent,m_clientpos.Width() - textXIndent,lineHeight);
  t_rect1.OffsetRect(m_clientpos.left,m_clientpos.top);
  t_rect1.OffsetRect(p_clientarea.left,p_clientarea.top);
  CRect t_rect2(textXIndent,textYIndent + lineHeight,m_clientpos.Width() - textXIndent,2 * lineHeight);
  t_rect2.OffsetRect(m_clientpos.left,m_clientpos.top);
  t_rect2.OffsetRect(p_clientarea.left,p_clientarea.top);
  CRect t_rectboth = m_clientpos;
  t_rectboth.OffsetRect(p_clientarea.left,p_clientarea.top);

  // Get DC for p_buffer.
  HDC t_pixhdc;
  if (!xvars.GetDC(p_buffer,&t_pixhdc)){
    return;
  }
  CDC *t_dc = CDC::FromHandle(t_pixhdc);

  // Clear background.
  CBrush *t_oldbrush=(CBrush *)t_dc->SelectStockObject(LTGRAY_BRUSH);
  t_dc->PatBlt(t_rectboth.left,t_rectboth.top,t_rectboth.right,t_rectboth.bottom,PATCOPY);
  t_dc->SelectObject(t_oldbrush);

  // Draw 3d outline.
  CPen *t_oldpen=(CPen *)t_dc->SelectStockObject(WHITE_PEN);
  t_dc->MoveTo(t_rectboth.right-1,t_rectboth.top);
  t_dc->LineTo(t_rectboth.right-1,t_rectboth.bottom-1);
  t_dc->LineTo(t_rectboth.left,t_rectboth.bottom-1);
  CPen t_newpen(PS_SOLID,1,RGB(96,96,96));
  t_dc->SelectObject(&t_newpen);
  t_dc->LineTo(t_rectboth.left,t_rectboth.top);
  t_dc->LineTo(t_rectboth.right-1,t_rectboth.top);
  t_dc->SelectObject(t_oldpen);//put it back

  // Draw strings
	t_dc->SelectStockObject(ANSI_FIXED_FONT);
	t_dc->SetBkMode(TRANSPARENT);
  t_dc->SetTextColor(RGB(0,0,0));
  t_dc->DrawText(t_string1,t_string1.GetLength(),t_rect1,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
  if (t_index != -1) {
    t_dc->DrawText(t_string2,t_string2.GetLength(),t_rect2,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
  }

  p_buffer->ReleaseDC(t_pixhdc);  
}



/////////////////////////////////////////////////
//STATUS WINDOW 
/////////////////////////////////////////////////
StatusWnd::StatusWnd(LocatorP l,Xvars *xvars)
:m_weaponwheel(l),m_itemwheel(l) {
  m_plocator = l;
  m_pxvars = xvars;
  m_lpDD = m_pxvars->m_lpDD;
  m_buffer = NULL;
}



void
StatusWnd::updateHealth(Health p_stat,Health p_maxstat)
{
  m_healthbar.setMaxPos(p_maxstat);
  m_healthbar.setPos(p_stat);
  m_healthbar.setHealth(p_stat);

  // Probably redundant.
  m_dirty = TRUE;
}



void
StatusWnd::updateMass(Mass p_stat)
{
// Don't bother showing it.
}



void
StatusWnd::updateWeapon(const char p_stat[IT_STRING_LENGTH],ClassId weaponClassId,int p_ammo,BOOL p_ready)
{
  // Weapon
  m_weaponwheel.setIndexByClassId(weaponClassId);

  // Ammo
  m_weaponwheel.setValueAtIndex(p_ammo,m_weaponwheel.getIndex());

  // Ready
  m_weaponwheel.setReadyAtIndex(p_ready ? Wheel::READY_YES : Wheel::READY_NO,
                                m_weaponwheel.getIndex());

  // Probably redundant.
  m_dirty = TRUE;
  return;
}



void
StatusWnd::updateItem(const char p_stat[IT_STRING_LENGTH],ClassId itemClassId,int p_count)
{
  m_itemwheel.setIndexByClassId(itemClassId);
  m_itemwheel.setValueAtIndex(p_count,m_itemwheel.getIndex());

  // Probably redundant.
  m_dirty = TRUE;
}



void
StatusWnd::updateCharacterName(const char p_name[IT_STRING_LENGTH])
{
  CString t_string = "Name: ";
  t_string += p_name;
  m_charactername.setMessage(t_string);

  // Probably redundant.
  m_dirty = TRUE;
}



void
StatusWnd::updateCharacterClass(const char p_class[IT_STRING_LENGTH])
{
  CString t_string = "Class: ";
  t_string += p_class;
  m_characterclass.setMessage(t_string);

  // Probably redundant.
  m_dirty = TRUE;
}



void
StatusWnd::updateXtraLivesHumanKills(int p_stat,BOOL p_xtraLives)
{
	char t_char[IT_STRING_LENGTH];
	CString t_string;
  if (p_xtraLives) {
    t_string += "Extra Lives: ";
    if (p_stat == IT_INFINITE_LIVES) {
      t_string += "unlimited";
    }
    else {
      t_string += itoa(p_stat,t_char,10);
    }   
  }
  else {
    t_string += "Human Kills: ";
    t_string += itoa(p_stat,t_char,10);
  }
	m_liveshumankills.setMessage(t_string);

  // Probably redundant.
  m_dirty = TRUE;
}



void
StatusWnd::updateKillsEnemyKills(int p_stat,BOOL p_kills)
{
	char t_char[IT_STRING_LENGTH];
	CString t_string;
  if (p_kills) {
    t_string += "Kills: ";
  }
  else {
    t_string += "Machine Kills: ";
  }
	t_string+=itoa(p_stat,t_char,10);
  m_killsenemykills.setMessage(t_string);

  // Probably redundant.
  m_dirty = TRUE;
}



void
StatusWnd::updateHumansNum(int p_stat)
{
	char t_char[IT_STRING_LENGTH];
  // Used to be "Humans Playing" but font didn't fit on some displays
	CString t_string("Humans: ");  
	t_string+=itoa(p_stat,t_char,10);
	m_humansnum.setMessage(t_string);

  // Probably redundant.
  m_dirty = TRUE;
}



void
StatusWnd::updateEnemiesNum(int p_stat)
{
	char t_char[IT_STRING_LENGTH];
  // Used to be "Enemies Remaining" but font didn't fit on some displays
	CString t_string("Enemies: ");
	t_string+=itoa(p_stat,t_char,10);
	m_enemiesnum.setMessage(t_string);

  // Probably redundant.
  m_dirty = TRUE;
}



void
StatusWnd::updateLevel(const char* p_level)
{
  m_level.setMessage(CString(p_level));

  // Probably redundant.
  m_dirty = TRUE;
}



void
StatusWnd::updateStatus(const char* p_message)
{
  m_status.setMessage(CString(p_message));

  // Probably redundant.
  m_dirty = TRUE;
}



void
StatusWnd::setRects(const CRect &p_status)
{
  m_statusrect = p_status;
  m_bufferrect.SetRect(0,0,m_statusrect.Width(),m_statusrect.Height());

  const int nameClassWidth = m_statusrect.Width() / 6;
  const int healthWidth = m_statusrect.Width() - nameClassWidth - 2 * Wheel::WHEEL_ITEM_WIDTH;
  const int otherButtonWidth = healthWidth / 4;

  // Weapon wheel  
  Pos t_point;
  t_point.x = 0;
  t_point.y = 0;
  m_weaponwheel.set_pos(t_point);

  // Item wheel
  t_point.x += Wheel::WHEEL_ITEM_WIDTH;
  m_itemwheel.set_pos(t_point);

  // Character name.
  CRect t_rect;
  t_rect.left = t_point.x + Wheel::WHEEL_ITEM_WIDTH;
  t_rect.top = 0;
  t_rect.right = t_rect.left + nameClassWidth; 
  t_rect.bottom = BUTTON_HEIGHT;
  m_charactername.setClientPos(t_rect);

  // Character class.
  t_rect.OffsetRect(0,BUTTON_HEIGHT);
  m_characterclass.setClientPos(t_rect);

  t_rect.left += nameClassWidth;


  // Health bar
  t_rect.right = t_rect.left + healthWidth;
  t_rect.top = 0;
  t_rect.bottom = BUTTON_HEIGHT;
  m_healthbar.setClientPos(t_rect);

  // Lives/Human kills
  t_rect.right = t_rect.left + otherButtonWidth;
  t_rect.OffsetRect(0,BUTTON_HEIGHT);
  m_liveshumankills.setClientPos(t_rect);

  // Kills/Enemy kills
  t_rect.OffsetRect(otherButtonWidth,0);
  m_killsenemykills.setClientPos(t_rect);

  // Humans playing
  t_rect.OffsetRect(otherButtonWidth,0);
  m_humansnum.setClientPos(t_rect);
  
  // Enemies playing
  t_rect.OffsetRect(otherButtonWidth,0);
  // Make sure last one is flush against right edge.
  t_rect.right = m_statusrect.Width(); 
  m_enemiesnum.setClientPos(t_rect);

  // Status bar
  t_rect.SetRect(0,2 * BUTTON_HEIGHT,
                 m_statusrect.Width() - LEVEL_WIDTH,m_statusrect.Height());
  m_status.setClientPos(t_rect);

  // Level-information bar.
  t_rect.SetRect(m_statusrect.Width() - LEVEL_WIDTH,2 * BUTTON_HEIGHT,
                 m_statusrect.Width(),m_statusrect.Height());
  m_level.setClientPos(t_rect);

  // Just to be sure.
  m_dirty = TRUE;
}



BOOL
StatusWnd::Create(CRect &p_statusrect) {
  m_buffer = m_pxvars->create_surface(p_statusrect.Width(),p_statusrect.Height(),NULL);
  if (m_buffer == NULL) {
    return FALSE;
  }

  // Fill status window with black to start out with.
  m_pxvars->color_fill(m_buffer,CRect(0,0,p_statusrect.Width(),p_statusrect.Height()),
                      m_pxvars->m_black);

  setRects(p_statusrect);

  // Just put some initial value in.
  updateCharacterName("None");
  updateCharacterClass("None");
  updateHealth(0,100);
  updateMass(0);
  updateWeapon("None",A_None,0,FALSE);
  updateItem("None",A_None,0);
  updateXtraLivesHumanKills(0,TRUE);
  updateKillsEnemyKills(0,TRUE);
  updateHumansNum(0);
  updateEnemiesNum(0);

  m_dirty = TRUE;

  return TRUE;
}



void 
StatusWnd::update(LPDIRECTDRAWSURFACE p_buffer,CWnd *p_parent,BOOL p_forceredraw)
{
  if (!m_pxvars)
    return;
  m_dirty |= m_charactername.update(*m_pxvars, m_buffer,m_bufferrect,p_forceredraw);
  m_dirty |= m_characterclass.update(*m_pxvars, m_buffer,m_bufferrect,p_forceredraw);
  m_dirty |= m_liveshumankills.update(*m_pxvars, m_buffer,m_bufferrect,p_forceredraw);
  m_dirty |= m_killsenemykills.update(*m_pxvars, m_buffer,m_bufferrect,p_forceredraw);
  m_dirty |= m_humansnum.update(*m_pxvars, m_buffer,m_bufferrect,p_forceredraw);
  m_dirty |= m_enemiesnum.update(*m_pxvars, m_buffer,m_bufferrect,p_forceredraw);
  m_dirty |= m_healthbar.update(*m_pxvars, m_buffer,m_bufferrect,p_forceredraw);
  m_dirty |= m_weaponwheel.update(m_buffer,m_plocator,*m_pxvars,p_forceredraw);
  m_dirty |= m_itemwheel.update( m_buffer,m_plocator,*m_pxvars,p_forceredraw);
  m_dirty |= m_status.update(*m_pxvars, m_buffer,m_bufferrect,p_forceredraw);
  m_dirty |= m_level.update(*m_pxvars, m_buffer,m_bufferrect,p_forceredraw);

  if ((p_forceredraw) || (m_dirty)) {
    // Get dest rect in screen coordinates.
    // m_statusrect is in client coordinates
    CPoint t_topLeft = m_statusrect.TopLeft();
    CPoint t_bottomRight = m_statusrect.BottomRight();
    p_parent->ClientToScreen(&t_topLeft);
    p_parent->ClientToScreen(&t_bottomRight);
    CRect t_screenstatusrect(t_topLeft,t_bottomRight);

    // Actually do the blt to the screen.
    // Used to be a DHCHECKLOST.
    m_pxvars->Blt(p_buffer,t_screenstatusrect.left,t_screenstatusrect.top,m_buffer,m_bufferrect);
    m_dirty = FALSE;
  }
}



StatusWnd::~StatusWnd() {
}



void StatusWnd::clock() {
  m_weaponwheel.clock();
  m_itemwheel.clock();
}



void StatusWnd::init_x(LocatorP l,Xvars& xvars,IXCommand command,void* arg) {
  Wheel::init_x(l,xvars,command,arg);
}
