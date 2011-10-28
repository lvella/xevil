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

#ifndef _FSSTATUSH
#define _FSSTATUSH
#include "locator.h"
#include "intel.h"
class XEvilFSButton
{
private:
    CRect m_clientpos;
    CString m_message;
    CString m_postedmessage;
    void DrawItem(Xvars &xvars, LPDIRECTDRAWSURFACE p_buffer,CRect &p_clientarea);
public:
    XEvilFSButton(){m_message="default";}
    XEvilFSButton(CRect p_pos){m_clientpos=p_pos;}
    ~XEvilFSButton(){}
    
    void setMessage(CString &p_string){m_postedmessage=p_string;}
    BOOL update(Xvars &xvars, LPDIRECTDRAWSURFACE p_buffer,CRect &p_clientarea,BOOL p_forceredraw=FALSE); 
    // returns TRUE if it drew something.
    void setClientPos(const CRect &p_rect){m_clientpos=p_rect;}
};



class XEvilFSStatusMessage 
{
private:
  CRect m_clientpos;
  CString m_message;  // Being displayed
  CString m_postedmessage; // Recently set
  void DrawItem(Xvars &xvars, LPDIRECTDRAWSURFACE p_buffer,CRect &p_clientarea);

public:
  XEvilFSStatusMessage() {}
  ~XEvilFSStatusMessage() {}
  void setMessage(CString &p_string){m_postedmessage=p_string;}
  void setClientPos(const CRect &p_rect){m_clientpos = p_rect;}
  BOOL update(Xvars &xvars, LPDIRECTDRAWSURFACE p_buffer,CRect &p_clientarea,BOOL p_forceredraw=FALSE); 
};



class XEvilFSHealthBar
{
private:
    CRect m_clientpos;
    int m_pos;
    int m_maxpos;
    int m_postedpos;
    int m_postedmaxpos;
    int m_health;
    void DrawItem(Xvars &xvars, LPDIRECTDRAWSURFACE p_buffer);
    enum {BORDER=2,TEXTINDENT=100};

public:
    XEvilFSHealthBar(){m_pos=1;m_maxpos=1;m_postedpos=0;m_postedmaxpos=2;m_health=0;}
    XEvilFSHealthBar(CRect p_pos){m_clientpos=p_pos;}
    ~XEvilFSHealthBar(){}
    
    void setPos(int p_int){m_postedpos=p_int;}
    void setMaxPos(int p_int){m_postedmaxpos=p_int;}
    void setHealth(int p_int){m_health=p_int;}
    BOOL update(Xvars &xvars, LPDIRECTDRAWSURFACE p_buffer,CRect &p_clientarea,BOOL p_forceredraw=FALSE);  //return true if actually drawn
    void setClientPos(const CRect &p_rect){m_clientpos=p_rect;}
};



class StatusWnd
{
private:
    void drawStatusMessage();

    // The rectangle of the statusWnd in the coordinates
    // of its parent.
    CRect m_statusrect;
    // Same size as status rect.  Upper-left is always (0,0).
    CRect m_bufferrect; 

    XEvilFSHealthBar m_healthbar;
    Wheel m_weaponwheel;
    Wheel m_itemwheel;
	  XEvilFSButton m_charactername;
	  XEvilFSButton m_characterclass;
	  XEvilFSButton m_liveshumankills;
	  XEvilFSButton m_killsenemykills;
	  XEvilFSButton m_humansnum;    
	  XEvilFSButton m_enemiesnum;
    XEvilFSStatusMessage m_status;
    XEvilFSStatusMessage m_level;

    LPDIRECTDRAWSURFACE m_buffer;
    LPDIRECTDRAW m_lpDD;
    BOOL m_dirty;
    Xvars *m_pxvars;
    Locator *m_plocator;


public:
    StatusWnd(LocatorP,Xvars *xvars);
    ~StatusWnd();
  	BOOL Create(CRect &p_statusrect);


    BOOL DestroyWindow(){return TRUE;}
	  LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam){return NULL;}
    LRESULT SendMessage( UINT message, WPARAM wParam = 0, LPARAM lParam = 0 ){return NULL;}
	  BOOL needRedraw(){return FALSE;}
	  void setRedraw(BOOL p_bool){}
    // Can we just kill all these methods?


    void updateCharacterName(const char p_stat[IT_STRING_LENGTH]);
    void updateCharacterClass(const char p_stat[IT_STRING_LENGTH]);
    void updateHealth(Health p_stat,Health p_maxstat);
    void updateMass(Mass p_stat);
    void updateWeapon(const char p_stat[IT_STRING_LENGTH],ClassId weaponClassId,int p_ammo,BOOL p_ready);
    void updateItem(const char p_stat[IT_STRING_LENGTH],ClassId itemClassId,int p_count);
  	void updateXtraLivesHumanKills(int p_stat,BOOL p_xtraLives);
  	void updateKillsEnemyKills(int p_stat,BOOL p_kills);
    void updateHumansNum(int p_stat);
    void updateEnemiesNum(int p_stat);
    void updateLevel(const char*);
    void updateStatus(const char*);

    void update(LPDIRECTDRAWSURFACE p_buffer,CWnd *p_parent,BOOL p_forceredraw = FALSE);
    /* EFFECTS: Draw the StatusWnd to p_buffer if dirty. */

    void setRects(const CRect &p_status);
    static void init_x(LocatorP,Xvars &,IXCommand,void*);

    void clock();
    /* NOTE: Needed for animation. */
};


#endif //_FSSTATUSH



