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

#ifndef _UISERVER_H
#define _UISERVER_H

#include "xevilserverstatus.h"
// Class Declarations
class UiServer: public Ui {
public:
  enum {SOUNDTRACKTIMER_CD=666,SOUNDTRACKTIMER_MIDI=667};
  UiServer(LocatorP);  
  ~UiServer();

  void pre_clock();
  /* EFFECTS: Follows the object of the registered intel if any and redraws 
  the world and locator. */

  void post_clock();

  int getWindowList(CTypedPtrList<CPtrList,CWnd *>&p_windowlist);
  Boolean process_event(int dpyNum,CMN_EVENTDATA eventdata);//return if handled
  void set_level(const char *p_levelname);
private:
    CXEvilServerStatus m_serverwnd;
    LocatorP locator;
    CTime m_starttime;
};

#endif //_UISERVER_H

