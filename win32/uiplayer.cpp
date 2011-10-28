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

// "uiplayer.cpp"

#include "stdafx.h"
#include "utils.h"

#include <iostream.h>
#include <strstrea.h>
#include <iomanip.h>
#include <ctype.h>

#include "xdata.h"
#include "coord.h"
#include "world.h"
#include "locator.h"
#include "id.h"
#include "physical.h"
#include "ui.h"
#include "uiplayer.h"
#include "sound.h"
#include "role.h"

#include "resource.h"//windows resources
#include "xviewport.h"//viewport class declaration
#include "wheel.h"
#include "fsstatus.h"//fsstatus class declaration
#include "aboutdialog.h"
#include "optionsdlg.h"
#include "keysets.h"
#include "difficulty.h"
#include "runserverdlg.h"
#include "connectdlg.h"
#include "StoryDlg.h"
#include "s_man.h"
//for bitmap loading and writing
#include <direct.h>
#include "l_agreement.h"


#define mciNotifySuccessful 1


// Defines
// Shifting a viewport when no Intel is associated with it.  In pixels.
#define SHIFT_NO_INTEL (5 * WSQUARE_WIDTH)

#define ARENA_FONT_NAME "Arial" // Make it customizable someday.
#define ARENA_FONT_POINT_SIZE 18
#define FONT_3D_OFFSET 4

#define MESSAGE_TIME 13 

#define STATUS_WINDOW_INDENT 20

#define SCREENDIMX_FULLSCREEN 800
#define SCREENDIMY_FULLSCREEN 600

#define SCREENDIMX_WINDOWED SCREENDIMX_FULLSCREEN
#define SCREENDIMY_WINDOWED SCREENDIMY_FULLSCREEN

// window coords, size of the viewport
#define VIEW_SIZE_WIDTH 800
#define VIEW_SIZE_HEIGHT 512



#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)

// Arena message will be displayed this distance above
// the center of the screen.  This is so the text is not
// right on top of the player.
#define ARENA_MESSAGE_OFF_CENTER 25 // 40



ViewportInfo VInfoProvider::get_info() {
  // HACK, should use Xvars.unstretch*() instead of bit shift to divide by 2.
  Size visible;
  visible.set((VIEW_SIZE_WIDTH) >> 1,
              (VIEW_SIZE_HEIGHT) >> 1);
  // Smooth-scrolling ViewportInfo.
  ViewportInfo ret(IT_VISION_RANGE,visible);
  return ret;
}



int
UiPlayer::getWindowList(CTypedPtrList<CPtrList,CWnd *>&p_windowlist)
{
  p_windowlist.RemoveAll();
  for (int i=0;i<m_windowarray.GetSize();i++)
    p_windowlist.AddTail(m_windowarray[i]);
  if (xvars.screenmode==Xvars::WINDOW_SCREEN_MODE)
  {
#if 0
    for (i=0;i<m_windowstatsarray.GetSize();i++)
    {
      if (m_windowstatsarray[i])
        p_windowlist.AddTail((StatusWnd *)m_windowstatsarray[i]);
    }
#endif
  }
  if (m_pabout)
    p_windowlist.AddTail(m_pabout);
  if (m_pkeys)
    p_windowlist.AddTail(m_pkeys);
  if (m_poptions)
    p_windowlist.AddTail(m_poptions);
  if (m_pdifficulty)
    p_windowlist.AddTail(m_pdifficulty);
  if (m_pconnect)
    p_windowlist.AddTail(m_pconnect);
  if (m_prunserver)
    p_windowlist.AddTail(m_prunserver);
  
  return p_windowlist.GetCount();
}



// Functions
UiPlayer::UiPlayer(int *argc,char **argv, WorldP w, LocatorP l,char **,char *,
                   SoundManagerP s,DifficultyLevel dLevels[DIFFICULTY_LEVELS_NUM],
                   Xvars::SCREENMODE mode,const char* gameVersion) {
  xvars.screenmode=mode;
  world = w;
  locator = l;
  soundmanager = s;
  roomDim = world->get_room_dim();
  deactivated = FALSE;
  intelsSet[0] = FALSE;
  m_numenemiesplaying=0;
  inputsSet[0] = FALSE;
  m_gameversion = gameVersion;

  viewSize.width = VIEW_SIZE_WIDTH;
  viewSize.height = VIEW_SIZE_HEIGHT;

  // Means don't display a debugging page for the SurfaceManager.
  m_surfMgrPage = -1;

  m_chatOn = False;

  // No errors.
  m_recoveryState = RS_OK;

  // Moved -bmptransfer parsing to Game::parse_args(), changed it to
  // -bmp_match_colormap

  settingsChanges = UInone;
  otherInput = FALSE;

  //	init_x(); this is now done in add_viewport(0)

  // UiPlayer::world must be valid.

  viewportsNum = 0;

  neverReset = TRUE;

  Timer nTimer(MESSAGE_TIME);
  messageTimer = nTimer;

  redrawAll[0] = DRAW_ALL;
  arenaMessage[0] = NULL;
  
  m_pabout=NULL;
  m_pkeys=NULL;
  m_poptions=NULL;
  m_pdifficulty=NULL;
  m_pconnect=NULL;
  m_prunserver=NULL;
  m_pstory=NULL;  


  // Read in user-specified key controls.
  CWinApp* pApp = AfxGetApp();
  keycodes[0][IT_CENTER]=(char)pApp->GetProfileInt("Player1","Center",101);
  keycodes[0][IT_R]=(char)pApp->GetProfileInt("Player1","Right",102);
  keycodes[0][IT_DN_R]=(char)pApp->GetProfileInt("Player1","DownRight",99);
  keycodes[0][IT_DN]=(char)pApp->GetProfileInt("Player1","Down",98);
  keycodes[0][IT_DN_L]=(char)pApp->GetProfileInt("Player1","DownLeft",97);
  keycodes[0][IT_L]=(char)pApp->GetProfileInt("Player1","Left",100);
  keycodes[0][IT_UP_L]=(char)pApp->GetProfileInt("Player1","UpLeft",103);
  keycodes[0][IT_UP]=(char)pApp->GetProfileInt("Player1","Up",104);
  keycodes[0][IT_UP_R]=(char)pApp->GetProfileInt("Player1","UpRight",105);
  keycodes[0][IT_WEAPON_CHANGE]=(char)pApp->GetProfileInt("Player1","WeaponChange",36);
  keycodes[0][IT_WEAPON_DROP]=(char)pApp->GetProfileInt("Player1","WeaponDrop",33);
  keycodes[0][IT_ITEM_USE]=(char)pApp->GetProfileInt("Player1","ItemUse",46);
  keycodes[0][IT_ITEM_CHANGE]=(char)pApp->GetProfileInt("Player1","ItemChange",35);
  keycodes[0][IT_ITEM_DROP]=(char)pApp->GetProfileInt("Player1","ItemDrop",34);
  keycodes[0][IT_WEAPON_CENTER]=(char)pApp->GetProfileInt("Player1","WeaponFire",45);
  keycodes[0][IT_CHAT]=(char)pApp->GetProfileInt("Player1","Chat",VK_SPACE);

  keycodes[1][IT_CENTER]=(char)pApp->GetProfileInt("Player2","Center",'L');
  keycodes[1][IT_R]=(char)pApp->GetProfileInt("Player2","Right",186);
  keycodes[1][IT_DN_R]=(char)pApp->GetProfileInt("Player2","DownRight",191);
  keycodes[1][IT_DN]=(char)pApp->GetProfileInt("Player2","Down",190);
  keycodes[1][IT_DN_L]=(char)pApp->GetProfileInt("Player2","DownLeft",188);
  keycodes[1][IT_L]=(char)pApp->GetProfileInt("Player2","Left",'K');
  keycodes[1][IT_UP_L]=(char)pApp->GetProfileInt("Player2","UpLeft",'I');
  keycodes[1][IT_UP]=(char)pApp->GetProfileInt("Player2","Up",'O');
  keycodes[1][IT_UP_R]=(char)pApp->GetProfileInt("Player2","UpRight",'P');
  keycodes[1][IT_WEAPON_CHANGE]=(char)pApp->GetProfileInt("Player2","WeaponChange",'S');
  keycodes[1][IT_WEAPON_DROP]=(char)pApp->GetProfileInt("Player2","WeaponDrop",'D');
  keycodes[1][IT_ITEM_USE]=(char)pApp->GetProfileInt("Player2","ItemUse",'Z');
  keycodes[1][IT_ITEM_CHANGE]=(char)pApp->GetProfileInt("Player2","ItemChange",'X');
  keycodes[1][IT_ITEM_DROP]=(char)pApp->GetProfileInt("Player2","ItemDrop",'C');
  keycodes[1][IT_WEAPON_CENTER]=(char)pApp->GetProfileInt("Player2","WeaponFire",'A');
  keycodes[1][IT_CHAT]=(char)pApp->GetProfileInt("Player2","Chat",VK_SPACE);  // Not used.



  keyStyle = UI_NINE_KEY;
  
  xvars.valid = 0;
  difficulty = DIFF_NONE;

  m_roletype = R_NONE;
  m_numhumansplaying = 0;
  m_numenemiesplaying = 0;

  check_num_lock();
  add_viewport();

  // Tool for generating XPM images for the UNIX build.
  if (genXPMDir) {
    generate_xpm();
  }

  CString t_pathstring;
  if (GetTempPath(MAX_PATH,t_pathstring.GetBuffer(MAX_PATH))){
    t_pathstring.ReleaseBuffer();
    tempfilemanager.attachDirectory(t_pathstring);
    tempfilemanager.cleanup();
  }
  t_pathstring.ReleaseBuffer();
  if (soundmanager){
    soundmanager->setTempFileManager(&tempfilemanager);
  }
}



int UiPlayer::get_dpy_max() {
  return 1;
}



UiPlayer::~UiPlayer() {
  int i;

  xvars.m_lpDD->SetCooperativeLevel(*m_windowarray[0],DDSCL_NORMAL);
  // Should use del_viewport() instead of doing directly.
  if (xvars.screenmode==Xvars::FULL_SCREEN_MODE) {
    delete m_windowarray[0];
  }
  else {
    for(i=0;i<m_windowarray.GetSize();i++) {
      delete m_windowarray[i];
    }
  }
  for(i=0;i<m_windowstatsarray.GetSize();i++) {
    if (m_windowstatsarray[i]) {
     delete m_windowstatsarray[i];
    }
  }
  for (i=0;i<viewportsNum;i++) {
    if (arenaMessage[i]) {
      delete arenaMessage[i];
    }
  }

  // Really should get cleaned up in del_viewport().
  for (i = 0; i < keyDispatchers.length(); i++) {
    KeyDispatcher* kDis = (KeyDispatcher*)keyDispatchers.get(i);
    delete kDis;
  }

  soundmanager->stopMIDI();
}



void UiPlayer::reset_graphics(Xvars::SCREENMODE p_newmode){
  COUTRESULT("entering reset_graphics");

  //stop midi sound before destroying windows!
  soundmanager->pauseMIDI();

  // We could destroy xvars and create a new one.
  // Make the lifetime of Xvars correspond to graphics being 
  // initialized and destroyed.

  delete xvars.m_surfaceManager;
  xvars.m_surfaceManager = NULL;

  // Free up all the existing graphics.
  if (xvars.m_lpDD) {
    xvars.m_lpDD->Release();
  }

  // Should use del_viewport() instead of doing directly.
  for(int i = 0;i < m_windowarray.GetSize();i++) {
    delete m_windowarray[i];
  }
  for(i = 0;i < m_windowstatsarray.GetSize();i++) {
    if (m_windowstatsarray[i]) {
      delete m_windowstatsarray[i];
    }
  }
  int t_viewportsNum = viewportsNum;
  BOOL* t_intelsSet = NULL;
  if (t_viewportsNum) {
    t_intelsSet = new BOOL[t_viewportsNum];
    for (int i = 0;i < t_viewportsNum;i++) {
      t_intelsSet[i] = intelsSet[i];
    }
  }
  viewportsNum = 0;
  set_screen_mode(p_newmode);

  KeyDispatcher* kDis = (KeyDispatcher*)keyDispatchers.get(viewportsNum);
  delete kDis;
  keyDispatchers.del(viewportsNum);


  // Has side-effect of calling init_x().
  add_viewport();

  // Ick, force menus to reinitialize themselves for the current role type.
  RoleType roleType = m_roletype;
  // Sent to R_NONE, or set_role_type will do nothing.
  m_roletype = R_NONE;
  set_role_type(roleType);

  for (int vn = 0; vn < t_viewportsNum; vn++) {
    assert(vn <= get_viewports_num());
    if (vn == get_viewports_num()) {
      int vNum = add_viewport();
      assert(vn == vNum);
    }
    intelsSet[vn] = t_intelsSet[vn];
    update_statuses(vn,TRUE);
  }
  set_input(0,UI_KEYS_RIGHT);
  if (viewportsNum > 1) {
    set_input(1,UI_KEYS_LEFT);
  }
  if (t_intelsSet) {
    delete t_intelsSet;
  }
}



int UiPlayer::add_viewport() {
  if (viewportsNum==0) {
    // Hack, should really be in init_x, but need to call Xvars::stretch* functions.
    xvars.m_stretch = 2;
  }

  KeyDispatcher* kDis = new KeyDispatcher();
  assert(kDis);
  assert(keyDispatchers.length() == viewportsNum);
  keyDispatchers.add(kDis);

  assert(viewportsNum < UI_VIEWPORTS_MAX);

  inputsSet[viewportsNum] = FALSE;

  Pos tPos(0,0);
  Size t_size;
  t_size.width = viewSize.width;
  t_size.height = viewSize.height;
  Area tArea(AR_RECT,tPos,t_size);

  if (xvars.screenmode==Xvars::FULL_SCREEN_MODE) { 
    switch(viewportsNum)
    {
    case 0: 
      {
      }
      break;
    case 1: 
      {
        // Fix when we redo 2 viewport mode
        assert(0);
#if 0
        Size t_tempsize=viewdrawAreas[0].get_size();
        t_tempsize.width/=2;
        viewdrawAreas[1]=viewdrawAreas[0];
        viewdrawAreas[1].set_size(t_tempsize);

        viewdrawAreas[0]=Area(AR_RECT,Pos(viewsize.width/2,0),t_tempsize);

        t_size.width/=2;
        drawAreas[0]=Area(AR_RECT,Pos(0,0),t_size);
        drawAreas[1]=Area(AR_RECT,Pos(0,0),t_size);

        Pos t_middle=viewportAreas[0].get_middle();
        viewportAreas[0].set_size(xvars.unStretchSize(t_size));
        viewport_to_pos(0,t_middle);
        tArea.set_size(t_size);
#endif
      }
      break;
    default: 
      assert(0);
      return viewportsNum;
      break;
    }
  }
  // Windowed mode
  else {
  }

  //now in world coordinates
  viewportAreas[viewportsNum] = xvars.un_stretch_area(tArea);

  redrawAll[viewportsNum] = DRAW_ALL;

  //MAIN GAME VIEWPORT
  CXEvilWnd *t_wnd;
  CRect t_windowsizerect;
  if (xvars.screenmode==Xvars::WINDOW_SCREEN_MODE)
  {
    t_windowsizerect.SetRect(0,0,SCREENDIMX_WINDOWED,SCREENDIMY_WINDOWED); 
    t_wnd=new CXEvilWnd;
    t_wnd->Create(t_windowsizerect,NULL/*parent*/,viewportsNum,FALSE);
  }
  // full screen
  else
  {
    if (viewportsNum==1)
      t_wnd=m_windowarray[0];
    else
    {
      // Why aren't these SCREENDIMX_FULLSCREEN,SCREENDIMY_FULLSCREEN
      t_windowsizerect.SetRect(0,0,SCREENDIMX_WINDOWED,SCREENDIMY_WINDOWED);
      t_wnd=new CXEvilWnd;
      t_wnd->Create(t_windowsizerect,NULL/*parent*/,viewportsNum,TRUE);
    }
  }

  // Remove the debug menu if we are not debugging.
  // And remove the full-screen/window mode button
  // Perhaps this should be in XEvilViewport
  if (viewportsNum == 0) {
    CMenu* t_menu = t_wnd->GetMenu();
    assert(t_menu);
    if (!DebugInfo::on()) {
      t_menu->RemoveMenu(4,MF_BYPOSITION);
      if (xvars.screenmode==Xvars::WINDOW_SCREEN_MODE) {
        t_menu->RemoveMenu(ID_FULLSCREEN,MF_BYCOMMAND);
      }
      else {
        t_menu->RemoveMenu(ID_WINDOWSCREEN,MF_BYCOMMAND);
      }
    }
  }

  //t_windowsizerect will be increased by create to attach necessary room for controls
  t_wnd->setActivated(CXEvilWnd::WMNONE);
  m_windowarray.SetAtGrow(viewportsNum,t_wnd);

  // Call init_x().
  if (viewportsNum==0) {
    init_x(t_wnd->GetSafeHwnd());
    t_wnd->setActivated(CXEvilWnd::WMNONE);
  }

  StatusWnd *t_statswnd = NULL;
  if (xvars.screenmode==Xvars::WINDOW_SCREEN_MODE) {
    //moving window to center it!
    assert(t_wnd->m_hWnd);
    CRect t_clientarea;//this is the important part.
    t_wnd->GetClientRect(t_clientarea);
    int t_dx = t_windowsizerect.Width() - t_clientarea.Width();
    int t_dy = t_windowsizerect.Height() - t_clientarea.Height();
    t_windowsizerect.OffsetRect(t_dx,t_dy);//increase size because of border and menu
    SetWindowPos(NULL,0,0,t_windowsizerect.Width(),t_windowsizerect.Height(),
                 SWP_NOZORDER,SWP_NOMOVE|SWP_NOREDRAW);
    t_wnd->ShowWindow(SW_SHOWNORMAL);

    //must find the center of the screen to offset the play area
    CDC *t_dc=t_wnd->GetDC();
    CPoint t_position;
    t_position.x = (int)t_dc->GetDeviceCaps(HORZRES)/2;
    t_position.y = (int)t_dc->GetDeviceCaps(VERTRES)/3;
    t_wnd->ReleaseDC(t_dc);
    t_windowsizerect.OffsetRect(t_position.x-(t_windowsizerect.Width()/2),
                                t_position.y-(t_windowsizerect.Height()/2) );
    if (t_windowsizerect.top<0)
      t_windowsizerect.OffsetRect(0,-1 * t_windowsizerect.top);
    if (t_windowsizerect.left<0)
      t_windowsizerect.OffsetRect(-1 * t_windowsizerect.left,0);
    t_wnd->SetWindowPos(NULL,t_windowsizerect.left,t_windowsizerect.top,0,0,
                        SWP_NOZORDER|SWP_NOSIZE);


    // Putting FSStatusWnd in the windowed mode.
    t_wnd->GetClientRect(t_clientarea);
    t_statswnd = new StatusWnd(locator,&xvars);
    assert(t_statswnd);
    // In client coordinates
    CRect t_statusrect(0,viewSize.height,
                       t_clientarea.right,t_clientarea.bottom);
    if (!t_statswnd->Create(t_statusrect)) {
      COUTRESULT("failed to create status window!");
    }
  }
  else //FULL SCREEN
  {
    CRect t_clientarea;
    t_wnd->GetClientRect(t_clientarea);

    // In client coordinates.
    CRect t_statusrect;
    t_statusrect.left = 0;
    t_statusrect.top = viewSize.height;
    t_statusrect.right = t_clientarea.right;
    t_statusrect.bottom = t_clientarea.bottom;
    t_statswnd = new StatusWnd(locator,&xvars);
    assert(t_statswnd);

    if (viewportsNum==1)
    {
      // Fix me
      ASSERT(0);
      t_clientarea.left=t_clientarea.right/2;
      t_clientarea.right=SCREENDIMX_FULLSCREEN;
      if (m_windowstatsarray[0])
        m_windowstatsarray[0]->setRects(t_statusrect);
      t_clientarea.right=t_clientarea.left;
      t_clientarea.left=0;
    }

    if (t_statswnd) {
      if (!t_statswnd->Create(t_statusrect)) {
        COUTRESULT("failed to create status window!");
      }
    }

    COUTRESULT("left init_x, in add_viewport after creating statuswnd check backbuffer");
  }
  m_windowstatsarray.SetAtGrow(viewportsNum,t_statswnd);
  update_statuses(viewportsNum,TRUE);

  //
  // now create a DirectDrawClipper object.
  //
  //used to be only for windowed, not anymore?..
  // 
  // We should try taking away the clipper for FS mode.
  // Was only there to protect from crashing on Win95 if
  // we drew outside the window.
  //
  if (xvars.screenmode==Xvars::WINDOW_SCREEN_MODE) {
    HRESULT t_result;
    m_clipperarray.SetAtGrow(viewportsNum,NULL);
    t_result = xvars.m_lpDD->CreateClipper(0, &m_clipperarray[viewportsNum], NULL);

    if( t_result != DD_OK )
    {
      AfxMessageBox("Cant create clipper");
      return FALSE;
    }

    t_result = m_clipperarray[viewportsNum]->SetHWnd(0, t_wnd->GetSafeHwnd());

    if( t_result != DD_OK )
    {
      AfxMessageBox("Cant set clipper window handle");
      return FALSE;
    }
  }

  //activate proper window
  t_wnd->SetActiveWindow(); 
  Utils::freeif(arenaMessage[viewportsNum]);

  //needs to be incremented first to allow ::clock to draw the screen for init_x_all
  viewportsNum++; 

  // Call at the end because it needs to be able to call UiPlayer::clock to draw.
  // Disable with the -no_init_graphics command line.
  //
  // Only init_x_all() for the first viewport, viewportNum has already been incremented.
  if (viewportsNum == 1) {
    if (initGraphics) {
      init_x_all();
    }
  }

  return viewportsNum - 1;
}



void UiPlayer::set_screen_mode(Xvars::SCREENMODE p_mode) {
  xvars.screenmode=p_mode;
}



void UiPlayer::del_viewport() {
  viewportsNum--;

  KeyDispatcher* kDis = (KeyDispatcher*)keyDispatchers.get(viewportsNum);
  delete kDis;
  keyDispatchers.del(viewportsNum);

  if (xvars.screenmode==Xvars::WINDOW_SCREEN_MODE)
  {
    m_windowarray[viewportsNum]->DestroyWindow();
    if (m_windowstatsarray[viewportsNum])
      m_windowstatsarray[viewportsNum]->DestroyWindow();
  }
  else
  {
    // Not fixed up for two-viewport mode.
    assert(False);
#if 0
    if (viewportsNum!=1)
    {
      assert(FALSE);
      return;
    }
    Area t_viewarea(AR_RECT,Pos(0,0),viewsize);

    viewdrawAreas[0]=t_viewarea;
    drawAreas[0]=Area(AR_RECT,Pos(0,0),drawsize);

    CRect t_clientarea(0,0,0,0);
    //		m_windowarray[0]->GetClientRect(t_clientarea);
    m_windowarray[0]->ClientToScreen(t_clientarea);
    t_clientarea.bottom=SCREENDIMY_FULLSCREEN;
    t_clientarea.right=SCREENDIMX_FULLSCREEN;
    t_clientarea.left=0;
    t_clientarea.top+=viewsize.height;

    CRect t_statusrect;
    t_statusrect.top=0;
    t_statusrect.bottom=drawsize.height;
    t_statusrect.right=drawsize.width;
    t_statusrect.left=100;
    if (m_windowstatsarray[0])
      m_windowstatsarray[0]->setRects(t_statusrect);

    //    viewportAreas[0]=Area(AR_RECT,Pos(0,0),drawsize);
    viewportAreas[0].set_size(xvars.unStretchSize(viewsize));
    Pos t_middle=viewportAreas[0].get_middle();
    viewport_to_pos(0,t_middle);
#endif
  }
}



void UiPlayer::set_humans_playing(int val) {
  for (int i=0;i<viewportsNum;i++)
  {
    StatusWnd *t_wnd=m_windowstatsarray[i];
    if (t_wnd)
    {
      assert(t_wnd);
      t_wnd->updateHumansNum(val);
      m_numhumansplaying=val;
    }
  }
}



void UiPlayer::set_enemies_playing(int val) {
  for (int i = 0;i < viewportsNum;i++) {
    StatusWnd *t_wnd = m_windowstatsarray[i];
    if (t_wnd) {
      assert(t_wnd);
      t_wnd->updateEnemiesNum(val);
      m_numenemiesplaying=val;
    }
  }
}



void UiPlayer::set_level(const char* val) {
  for (int i = 0;i < viewportsNum;i++) {
    StatusWnd *t_wnd = m_windowstatsarray[i];
    if (t_wnd) {
      assert(t_wnd);
      t_wnd->updateLevel(val);
      m_level = val;
    }
  }
}



Boolean UiPlayer::other_input() {
  return otherInput;
}

  
  
void UiPlayer::set_input(int n,UIinput input) {
  assert(n >= 0 && n < viewportsNum);
  inputs[n] = input; 
  inputsSet[n] = TRUE;
}




void UiPlayer::set_difficulty(int d) {
  difficulty = d;
}



void UiPlayer::set_role_type(RoleType roleType) {
  if (roleType == m_roletype) {
    // Nothing to do.
    return;
  }

  assert(roleType != R_NONE);
  m_roletype = roleType;  

  // Update the menus for client and server.
  for (int n = 0; n < m_windowarray.GetSize(); n++) {
    if (!m_windowarray[n] || !m_windowarray[n]->GetMenu()) {
      continue;
    }
    CMenu *t_menu = m_windowarray[n]->GetMenu();
    assert(t_menu);


    // Change the "Network" item to "Network", "Client", or "Server".
    const char* menuTitle;
    switch (roleType) {
      case R_STAND_ALONE:
        menuTitle = "Network";
        break;
      case R_CLIENT:
        menuTitle = "Client";
        break;
      case R_SERVER:
        menuTitle = "Server";
        break;
      default:
        assert(0);        
    }
    t_menu->ModifyMenu(2,MF_BYPOSITION | MF_STRING,0,menuTitle);


    // Enable/Disable difficulty menu.
    if (Role::uses_difficulty(roleType)) {
      t_menu->EnableMenuItem(ID_DIFFICULTY,MF_BYCOMMAND|MF_ENABLED);
    }
    else {
      t_menu->EnableMenuItem(ID_DIFFICULTY,MF_BYCOMMAND|MF_GRAYED);
    }

    // CONNECTSERVER / DISCONNECTSERVER
    Boolean connServerExists = 
      (t_menu->GetMenuState(ID_CONNECTSERVER,MF_BYCOMMAND) != -1);
    if (roleType == R_STAND_ALONE ||
        roleType == R_SERVER) {
      // Change ID_DISCONNECTSERVER to ID_CONNECTSERVER
      if (!connServerExists) {
        t_menu->ModifyMenu(ID_DISCONNECTSERVER,MF_BYCOMMAND|MF_STRING,
                           ID_CONNECTSERVER,"&Connect to Server");
      }
      if (roleType == R_SERVER) {
        t_menu->EnableMenuItem(ID_CONNECTSERVER,MF_BYCOMMAND|MF_GRAYED);
      }
      else {
        t_menu->EnableMenuItem(ID_CONNECTSERVER,MF_BYCOMMAND|MF_ENABLED);
      }      
    }
    else {
      assert(roleType == R_CLIENT);
      // Change ID_CONNECTSERVER to ID_DISCONNECTSERVER
      if (connServerExists) {
        t_menu->ModifyMenu(ID_CONNECTSERVER,MF_BYCOMMAND|MF_STRING,
                           ID_DISCONNECTSERVER,"&Disconnect Server");      
      }
      t_menu->EnableMenuItem(ID_DISCONNECTSERVER,MF_BYCOMMAND|MF_ENABLED);
    }


    // RUNSERVER / STOPSERVER
    Boolean runServerExists = 
      (t_menu->GetMenuState(ID_RUNSERVER,MF_BYCOMMAND) != -1);
    if (roleType == R_STAND_ALONE ||
        roleType == R_CLIENT) {
      // Change ID_STOPSERVER to ID_RUNSERVER
      if (!runServerExists) {
        t_menu->ModifyMenu(ID_STOPSERVER,MF_BYCOMMAND|MF_STRING,
                           ID_RUNSERVER,"&Run as Server");
      }
      if (roleType == R_CLIENT) {
        t_menu->EnableMenuItem(ID_RUNSERVER,MF_BYCOMMAND|MF_GRAYED);
      }
      else {
        t_menu->EnableMenuItem(ID_RUNSERVER,MF_BYCOMMAND|MF_ENABLED);
      }      
    }
    else {
      assert(roleType == R_SERVER);
      // Change ID_RUNSERVER to ID_STOPSERVER
      if (runServerExists) {
        t_menu->ModifyMenu(ID_RUNSERVER,MF_BYCOMMAND|MF_STRING,
                           ID_STOPSERVER,"&Stop Server");      
      }
      t_menu->EnableMenuItem(ID_STOPSERVER,MF_BYCOMMAND|MF_ENABLED);
    }

    // This might not be the best way to force menu to redraw.
    m_windowarray[n]->SetMenu(t_menu);
  }
}



void UiPlayer::register_intel(int n, IntelP intel) {
  assert(n >= 0 && n < viewportsNum);

  intelsSet[n] = TRUE;
  intels[n] = intel;
}



void UiPlayer::unregister_intel(int n) {
  assert((n >= 0) && (n < UI_VIEWPORTS_MAX));
  intelsSet[n] = FALSE;
}



IntelP UiPlayer::get_intel(int n) {
  assert((n >= 0) && (n < UI_VIEWPORTS_MAX));
  return intels[n];
}



void UiPlayer::demo_reset() {
  for (int n = 0; n < viewportsNum; n++) {
    intelsSet[n] = FALSE;
    redrawAll[n] = DRAW_ALL;
    Utils::freeif(arenaMessage[n]);  // Actual message being displayed.
    Pos tPos(0,0);
    viewportAreas[n].set_pos(tPos);

    update_statuses(n,TRUE);
  }

  // In case the user was in the middle of sending a message.
  m_chatOn = False;
}



void UiPlayer::reset() {
  demo_reset();
  neverReset = FALSE;
}



void UiPlayer::set_redraw_arena() {
  for (int n = 0; n < viewportsNum; n++) {
    redrawAll[n] = DRAW_ALL;
  }
}



CWnd *
UiPlayer::findModalessWindow(HWND p_hwnd) {
  if (m_pabout && m_pabout->m_hWnd == p_hwnd)
    return m_pabout;
  if (m_pkeys && m_pkeys->m_hWnd == p_hwnd)
    return m_pkeys;
  if (m_poptions && m_poptions->m_hWnd == p_hwnd)
    return m_poptions;
  if (m_pdifficulty && m_pdifficulty->m_hWnd == p_hwnd)
    return m_pdifficulty;
  if (m_pconnect && m_pconnect->m_hWnd == p_hwnd)
    return m_pconnect;
  if (m_prunserver && m_prunserver->m_hWnd == p_hwnd)
    return m_prunserver;
             
  return NULL;
}



void
UiPlayer::deleteModalessWnd(CWnd *t_wnd)
{
  if (m_pabout && m_pabout->m_hWnd == t_wnd->m_hWnd)
  {
    delete m_pabout;
    m_pabout=NULL;
  }
  if (m_pkeys && m_pkeys->m_hWnd == t_wnd->m_hWnd)
  {
    delete m_pkeys;
    m_pkeys=NULL;
  }
  if (m_poptions && m_poptions->m_hWnd == t_wnd->m_hWnd)
  {
    delete m_poptions;
    m_poptions=NULL;
  }
  if (m_pdifficulty && m_pdifficulty->m_hWnd == t_wnd->m_hWnd)
  {
    delete m_pdifficulty;
    m_pdifficulty=NULL;
  }
  if (m_pconnect && m_pconnect->m_hWnd == t_wnd->m_hWnd)
  {
    delete m_pconnect;
    m_pconnect=NULL;
  }
  if (m_prunserver && m_prunserver->m_hWnd == t_wnd->m_hWnd)
  {
    delete m_prunserver;
    m_prunserver=NULL;
  }
}



// break in event handling. False == stop checking
Boolean 
UiPlayer::process_event(int,CMN_EVENTDATA eventdata)
{
  int vNum = get_viewport_num(eventdata.m_parent);
  CWnd *t_paintwnd;
  CPaintDC *t_dc;
  CRect t_updaterect;
  Boolean t_returnvalue=TRUE;

  CWnd *t_wnd;
  t_wnd= findModalessWindow(eventdata.m_parent);
  if (t_wnd)
  {
    switch(eventdata.m_event.message) 
    {
    case WM_DESTROY: case WM_CLOSE:
      deleteModalessWnd(t_wnd);
      m_windowarray[0]->SetFocus();
      break;
    case WM_KEYDOWN:case WM_KEYUP: case 0x118:
      ::SendMessage(eventdata.m_event.hwnd,eventdata.m_event.message,eventdata.m_event.wParam,eventdata.m_event.lParam);
      break;
    case WM_COMMAND:
      switch (LOWORD(eventdata.m_event.wParam))
      {
      case IDOK:
        if (m_pkeys && t_wnd->m_hWnd==m_pkeys->m_hWnd)
          retrieveKeys();
        if (m_poptions && t_wnd->m_hWnd==m_poptions->m_hWnd)
          retrieveOptions();
        if (m_pdifficulty && t_wnd->m_hWnd==m_pdifficulty->m_hWnd)
          retrieveDifficulty();
        if (m_pconnect && t_wnd->m_hWnd==m_pconnect->m_hWnd)
          retrieveConnect();
        if (m_prunserver && t_wnd->m_hWnd==m_prunserver->m_hWnd)
          retrieveRunServer();
                   
      case IDCANCEL:
          m_windowarray[0]->SetFocus();
          deleteModalessWnd(t_wnd);
          break;
      }
      break;
    default:
      if (eventdata.m_parent==t_wnd->m_hWnd)
        ::SendMessage(eventdata.m_event.hwnd,eventdata.m_event.message,eventdata.m_event.wParam,eventdata.m_event.lParam);
      else
      //  ::SendMessage(eventdata.m_event.hwnd,eventdata.m_event.message,eventdata.m_event.wParam,eventdata.m_event.lParam);

      ::DefWindowProc(eventdata.m_event.hwnd, eventdata.m_event.message, eventdata.m_event.wParam, eventdata.m_event.lParam);
    }
    return TRUE;
  }

  if (-1 != vNum) //check main window
  {
    switch(eventdata.m_event.message) 
    {
    case WM_TIMER:
      if (eventdata.m_event.wParam==Ui::SOUNDTRACKTIMER_CD)
        soundmanager->playCDtrack();
      if (eventdata.m_event.wParam==Ui::SOUNDTRACKTIMER_MIDI)
        soundmanager->playMIDItrack();
      break;
    case MM_MCINOTIFY:
      if (eventdata.m_event.wParam == mciNotifySuccessful)
        soundmanager->playMIDItrack();
      return TRUE;
      break;
    case WM_PAINT:
      if (IsIconic(eventdata.m_parent))
      {
        SendMessage(eventdata.m_parent,WM_PAINT,NULL,NULL);
        return TRUE;
      }
      t_paintwnd=CWnd::FromHandle(eventdata.m_event.hwnd);
      assert(t_paintwnd);
      if (eventdata.m_event.hwnd==eventdata.m_parent)
      {
        t_dc=new CPaintDC(t_paintwnd);
        assert(t_dc);
        delete t_dc;
      }
      else
        t_paintwnd->SendMessage(WM_PAINT,eventdata.m_event.wParam,eventdata.m_event.lParam);
      break;
      viewport_expose(vNum);
      break;
    case WM_SETFOCUS:
      {
        if (viewportsNum)//if we have a viewport
        {
          CXEvilWnd *t_wnd=m_windowarray[0];
          assert(t_wnd);
          m_windowarray[viewportsNum]->SetFocus();
        }
      }
      break;
    case WM_ACTIVATEAPP:
      {
        WORD fActive = (BOOL)eventdata.m_event.wParam; 
        if (fActive == TRUE)
        {
          COUTRESULT("WM_ACTIVATEAPP");
          doActivate(WA_ACTIVE);
        }
        break;
      }
    case WM_DESTROY:    
      settingsChanges |= UIquit;
      break;
    case WM_CHAR:
      // Quit chat mode.
      if (m_chatOn) {
        chat_process_char((char)eventdata.m_event.wParam);
      }
      // Confirm exiting program.
      else {
        if ((char)eventdata.m_event.wParam == VK_ESCAPE) {
          int val = AfxMessageBox("Quit XEvil?",MB_OKCANCEL);
          if (val == IDOK) {
            settingsChanges |= UIquit;
          }
        }
      }
      break;
    case WM_COMMAND:
      switch (LOWORD(eventdata.m_event.wParam))
      {
      case ID_NEWGAME:
        settingsChanges |= UInewGame;
        break;
      case ID_OPTIONS:
        {
          doOptions();
          break;			
        }
      case ID_CONNECTSERVER :
        {
          doConnect();
        }
        break;
      case ID_DISCONNECTSERVER :
        {
          settingsChanges |= UIdisconnectServer;
        }
        break;
      case ID_STOPSERVER :
        {
          settingsChanges |= UIstopServer;
        }
        break;
      case ID_RUNSERVER :
        {
          doRunServer();
        }
        break;
      case ID_STORY :
        {
          doStory();
        }
        break;
      case ID_DIFFICULTY :
        {
          doDifficulty();
          break;
        }      
      case ID_APP_EXIT :
        settingsChanges |= UIquit;
        break;
      case ID_APP_ABOUT :
        {
#ifdef MODELESS_DIALOGS
          m_pabout=new AboutDialog;
          m_pabout->Create(AboutDialog::IDD,m_windowarray[0]);
          m_pabout->ShowWindow(SW_NORMAL);
          m_pabout->SetFocus();
#else
          AboutDialog t_dlg;
          t_dlg.setLicenseText(LAgreement::get_text());
          t_dlg.setVersionString(m_gameversion);
          t_dlg.setReject(FALSE);
          t_dlg.DoModal();
#endif
          break;
        }
      case ID_KEYS : case ID_KEYS2 :
        {
          doKeys(LOWORD(eventdata.m_event.wParam));
          break;
        }
      case ID_RESETGFX:
        {
          int val = AfxMessageBox("Reset Graphics?\n\n"
              "Use this to attempt recovery from DirectDraw errors.\n"
              "May cause unpredictable behavior on some machines.",
              MB_OKCANCEL);
          if (val == IDOK) {
            reset_graphics(xvars.screenmode);
          }
          break;
        }
      case ID_FULLSCREEN:
        {
          if (xvars.screenmode!=Xvars::FULL_SCREEN_MODE)
          {
            settings.screenmode=Xvars::FULL_SCREEN_MODE;
            settingsChanges |= UIscreenmode;
          }
          break;
        }
      case ID_WINDOWSCREEN:
        {
          if (xvars.screenmode!=Xvars::WINDOW_SCREEN_MODE)
          {
            settings.screenmode=Xvars::WINDOW_SCREEN_MODE;
            settingsChanges |= UIscreenmode;
          }
          break;
        }
      case ID_NETHELP:
        {
          int val = AfxMessageBox("This will exit XEvil and run a web browser.",
                                  MB_OKCANCEL);
          if (val == IDOK) {
            CString t_string;
            t_string.LoadString(IDS_NETHELPURL);
            CXEvilWnd *t_wnd = get_viewport_wnd(vNum);
            if (t_wnd && xvars.screenmode!=Xvars::FULL_SCREEN_MODE) {
              t_wnd->SendMessage(WM_SYSCOMMAND,SC_MINIMIZE,0);
            }
            ShellExecute(NULL,"open",t_string,"","",SW_SHOWDEFAULT);
            settingsChanges |= UIquit;     
          }
          break;
        }
      case ID_SURFACEMGR:
        {
          int numPages = xvars.m_surfaceManager->debug_pages_num(viewSize);
          // -1 becomes 0 which is what we want.
          m_surfMgrPage++;
          if (m_surfMgrPage == numPages) {
            // All pages have been displayed revert to normal viewing.
            m_surfMgrPage = -1;

            float prcnt = 100.0f * (float)xvars.m_surfaceManager->pixels_used() / 
              (float)xvars.m_surfaceManager->pixels_allocated();
            strstream str;
            str << setprecision(4)
              << "Used " << xvars.m_surfaceManager->pixels_used() << " pixels out of " 
              << xvars.m_surfaceManager->pixels_allocated() 
              << " (" << prcnt << "%)" << ends;
            locator->message_enq(Utils::strdup(str.str()));
            locator->arena_message_enq(str.str(),NULL,250,False);
          }          
          break;
        }
      default :
        assert(FALSE);
      return t_returnvalue;
      }
    default:
      ::DefWindowProc(eventdata.m_event.hwnd, eventdata.m_event.message, eventdata.m_event.wParam, eventdata.m_event.lParam);
        break;
    }
  }
  return TRUE;
}



void
UiPlayer::update_statuses(int viewportNum,BOOL forcestatuschanged)
{
  StatusWnd *t_wnd = m_windowstatsarray[viewportNum];
  if (intelsSet[viewportNum])
  {
    if (forcestatuschanged || intels[viewportNum]->intel_status_changed())
    {
      if (t_wnd)
      {
        assert(t_wnd);
        const IntelStatus *status = intels[viewportNum]->get_intel_status();
        t_wnd->updateCharacterName(status->name);
        t_wnd->updateCharacterClass(status->className);
        t_wnd->updateHealth(status->health,status->healthmax);
        t_wnd->updateMass(status->mass);
        t_wnd->updateWeapon(status->weapon,status->weaponClassId,status->ammo,status->weaponReady);
        t_wnd->updateItem(status->item,status->itemClassId,status->itemCount);
        t_wnd->updateHumansNum(m_numhumansplaying);
        t_wnd->updateEnemiesNum(m_numenemiesplaying);
        t_wnd->updateLevel(m_level);

        // Display different messages in the same fields for EXTENDED vs. other
        // GameStyles.  This logic is duplicated in the front ends, should be in
        // XP code.
        if (settings.style == EXTENDED) {
          // Use humanKills
          t_wnd->updateXtraLivesHumanKills(status->humanKills - status->soups,FALSE);
          // Use machineKills
          t_wnd->updateKillsEnemyKills(status->enemyKills,FALSE);        
        }
        else {
          // Use Extra Lives
          t_wnd->updateXtraLivesHumanKills(status->lives,TRUE);
          // Use Kills
          t_wnd->updateKillsEnemyKills(status->humanKills + status->enemyKills,TRUE);        
        }
      }
    }
  }

  // Clock viewports for animation.
  // Right now we are clocking right before the draw, so
  // we always have the current state drawn.
  // Want to clock after all the update* calls are made.
  StatusWnd *t_stats = m_windowstatsarray[viewportNum];
  ASSERT(t_stats);
  t_stats->clock();

  // This does its own dirty logic, and will only actually draw if necessary.
  t_wnd->update(xvars.m_lpprimarysurface,m_windowarray[0],forcestatuschanged);
}



void UiPlayer::interpret_key_state() {
  // We now do the key repeat check inside Human

  // If the user is typing in a chat message, don't dispatch key events.
  if (m_chatOn) {
    return;
  }

  // Space bar will start a new game.
  // This should probably be triggered by any key press like on
  // UNIX.
  if (KEY_DOWN(VK_SPACE)) {
    otherInput = TRUE;
  }

  // ignore all keypress that is the same
  // check all previous keypress to see if it is up now.
  for (int viewportNum = 0; viewportNum < viewportsNum; viewportNum++) {
    if (!inputsSet[viewportNum]) {
      continue;
    }
    KeyDispatcher* kDis = (KeyDispatcher*)keyDispatchers.get(viewportNum);
    assert(kDis);
    kDis->clock(this,this,(void*)viewportNum);
  }
}



// Glue method.
Boolean UiPlayer::key_down(int key,void* closure) {
  int viewportNum = (int)closure;
  return KEY_DOWN(keycodes[viewportNum][key]);
}



// Glue method.
void UiPlayer::dispatch(ITcommand command,void* closure) {
  int viewportNum = (int)closure;
  dispatch(viewportNum,command);
}



void UiPlayer::pre_clock() {
  // Make attempt to recover from error condition.
  if (m_recoveryState == RS_MUST_RECOVER) {
    COUTRESULT("Attempting error recovery, reset graphics.");
    m_recoveryState = RS_IN_RECOVERY;

    // Set screenmode to current mode.
    reset_graphics(xvars.screenmode);
  }

  otherInput = False;

  interpret_key_state();

  // Display new arena messages.
  char *arenaMsg;
  Boolean exclusive;
  // Loop, empty the queue.
  do {
    IntelId msgTarget;
    Quanta time;
    Boolean propagate; // not used here
    exclusive = locator->arena_message_deq(&arenaMsg,msgTarget,time,propagate);
    if (arenaMsg) {
      // Go through all viewports, setting message on appropriate ones.
      for (int n = 0; n < viewportsNum; n++) {
        if ((exclusive && intelsSet[n] && 
             intels[n]->get_intel_id() == msgTarget) ||
            !exclusive) {
          // Either this is the target for the exclusive message or 
          // the message goes to all viewports.
          Utils::freeif(arenaMessage[n]);
          arenaMessage[n] = Utils::strdup(arenaMsg);

          if (time == -1) {
            // default value.
            time = UI_ARENA_MESSAGE_TIME;
          }
          arenaMessageTimer[n].set(time);
          redrawAll[n] = DRAW_ALL;  // This correct? hardts
          
          // Dont' need to redraw all, because we'll redraw the message
          // every turn.
          if (exclusive) {
            break;
          }
        }
      }
      delete arenaMsg;
    }
  } while(arenaMsg);
  // end arena messages


  // Make soundmanager follow middle of viewportarea
  for (short i = 0; i < viewportsNum; i++) {
    soundmanager->setKeyPosition(i,viewportAreas[i].get_middle());
  }


  // Follow intel and draw.
  for (int n = 0; n < viewportsNum; n++) {
    // Before check for death.
    CXEvilWnd *t_wnd = get_viewport_wnd(n);
    assert(t_wnd);
    update_statuses(n);

    switch (t_wnd->getActivated()) {
    case CXEvilWnd::WMACTIVATED :
      doActivate(WA_ACTIVE);
      t_wnd->setActivated(CXEvilWnd::WMNONE);
      break;
    case CXEvilWnd::WMINACTIVE :
      doActivate(WA_INACTIVE);
      t_wnd->setActivated(CXEvilWnd::WMNONE);
      break;
    case CXEvilWnd::WMNONE :
    default:
      break;
    }
    if (!deactivated && t_wnd->getQueryNewPalette() && (xvars.uses_palette()))
    {//resetpalette!
      t_wnd->setQueryNewPalette(FALSE);
      HRESULT t_result=xvars.m_lpprimarysurface->SetPalette( xvars.m_ddpalette );
      DHRESULT(t_result,"aftersetpalette");
    }
    if (t_wnd->getTimerId()==UiPlayer::SOUNDTRACKTIMER_CD)
    {
      t_wnd->setTimerId(-1);
      soundmanager->playCDtrack();
    }
    if (t_wnd->getTimerId()==UiPlayer::SOUNDTRACKTIMER_MIDI)
    {
      t_wnd->setTimerId(-1);
      soundmanager->playMIDItrack();
    }
    if (xvars.screenmode==Xvars::WINDOW_SCREEN_MODE)
    {
      StatusWnd *t_statswnd=get_stats_wnd(n);
      assert(t_statswnd);
      if (t_statswnd->needRedraw())
      {
        t_statswnd->setRedraw(FALSE);
        t_statswnd->SendMessage(WM_PAINT,0,0);
      }
    }

    // Follow intel.
    if (intelsSet[n] && intels[n]->is_playing()) {
      PhysicalP p;
      if (p = locator->lookup(intels[n]->get_id())) {
        Area a = p->get_area();
        if (viewport_to_pos(n,a.get_middle())) {
          redrawAll[n] = DRAW_ALL;
        }
      }
#if 0
      else if (!intels[n]->alive() && intels[n]->get_lives() == 0) {
        unregister_intel(n);
      }
#endif
    }

    if (!deactivated) {
      draw(n,redrawAll[n]);
    }

    arenaMessageTimer[n].clock();
    // Clear arenaMessage being displayed if time is up.
    if (arenaMessage[n] && arenaMessageTimer[n].ready()) {
      Utils::freeif(arenaMessage[n]);
      // Clean up the message.
      redrawAll[n] = DRAW_ALL;  // This correct? hardts
    }
  }    

  // Clock message bar.
  if (messageTimer.ready())
  {
    char *msg;
    if (msg = locator->message_deq())
    {
      set_message(msg);

      delete msg;
      messageTimer.set();
    }
  }
  messageTimer.clock();
}



void UiPlayer::post_clock() {
  // Does nothing on Windows, only for compatibility with UNIX.
}



void UiPlayer::set_message(const char *message) {
  // Ignore all status messages while the status area is used for chat input.
  if (m_chatOn) {
    return;
  }

  // BUG.  Won't work when we have two Viewports.
  // Should move status message outside of the Viewport if we want it to be
  // shared between the two viewports.

  if (m_windowstatsarray[0]) {
    m_windowstatsarray[0]->updateStatus(message);
  }  
}



void translate_bitmaps(Xvars &xvars,CString &p_src,CString &p_dest)
{
  FILE *t_src;
  FILE *t_dest;
  _chdir(p_src);
  CFileFind finder;
  BOOL bWorking = finder.FindFile("*.bmp");
  CString t_newfilename;
  CString t_oldfilename;
  while (bWorking)
  {
    bWorking = finder.FindNextFile();
    t_newfilename = p_dest + "\\" + finder.GetFileName();
    t_oldfilename = finder.GetFilePath();
    t_src=fopen(t_oldfilename.GetBuffer(0),"rb");
    t_dest=fopen(t_newfilename.GetBuffer(0),"wb");
    xvars.read_bitmap_write_file(t_src,t_dest);
    fclose(t_src);
    fclose(t_dest);
  }
}



void UiPlayer::draw(int viewportNum,DRAWTYPE changedOnly) {
  assert((viewportNum >= 0) && (viewportNum < viewportsNum));

  CXEvilWnd *t_wnd = (CXEvilWnd *)m_windowarray[viewportNum];
  assert(t_wnd);
  if (!t_wnd->isValid()) {
    settingsChanges |= UIquit;
    return;
  }
  if (t_wnd->IsIconic()) {
    CPaintDC t_dc(t_wnd);
    CWinApp *t_app=AfxGetApp();
    assert(t_app);
    t_dc.DrawIcon(0,0,t_app->LoadIcon(IDR_XEVIL));
    return;
  }

  PAINTSTRUCT t_paintstruct;
  if (xvars.screenmode == Xvars::WINDOW_SCREEN_MODE) {
    if (DRAW_EXPOSE == changedOnly) {
      // Don't need the returned DC.
      ::BeginPaint(t_wnd->m_hWnd, &t_paintstruct);
    }
  }

  // Refresh status window.
  if (DRAW_ALL==changedOnly) {
    StatusWnd *t_wnd = m_windowstatsarray[viewportNum];
    ASSERT(t_wnd);
    t_wnd->update(xvars.m_lpprimarysurface,m_windowarray[0]);
  }

  // Normal drawing.
  if (m_surfMgrPage == -1) {
    draw_backbuffer(viewportNum);
  }
  // Draw debug page for SurfaceManager.
  else {
    xvars.m_surfaceManager->draw_debug_page(xvars,xvars.m_lpbackbuffer,
                                            viewSize,m_surfMgrPage);
  }

  CRect t_clientrect;
  HRESULT t_result = DD_OK;
  Size t_size = viewSize;
  Pos t_pos;

  // Merged this section of code for windowed and full-screen modes.
  t_clientrect.SetRect(0,0,viewSize.width,viewSize.height);
  CRect t_offset(t_pos.x,t_pos.y,t_pos.x+t_size.width,t_pos.y+t_size.height);
  t_wnd->ClientToScreen(&t_offset);

  // Only clipper for window mode.
  LPDIRECTDRAWCLIPPER t_lpclipper = NULL;
  if (xvars.screenmode==Xvars::WINDOW_SCREEN_MODE) {
    t_lpclipper = m_clipperarray[viewportNum];
  }

  t_result = 
    xvars.Blt(xvars.m_lpprimarysurface,t_offset.left,t_offset.top,xvars.m_lpbackbuffer,
              t_clientrect,False,t_lpclipper);

  DHRESULT(t_result,;);


  // What does this mean? hardts
  if (DRAW_NOTALL!=changedOnly) {
    redrawAll[viewportNum] = DRAW_NOTALL;
  }

  // Shouldn't this be just for window mode?  hardts
  if (xvars.screenmode == Xvars::WINDOW_SCREEN_MODE) {
    if (DRAW_EXPOSE==changedOnly) {
      ::EndPaint(t_wnd->m_hWnd,&t_paintstruct);
    }
  }

  // Tool used for matching palettes.
  if (bmpSrcDir && bmpDestDir) {
    translate_bitmaps(xvars,CString(bmpSrcDir),CString(bmpDestDir));
    exit(0);
  }

  // Successful recovery from an error, yea!
  // We got through an entire cycle of graphics with out blowing up, so
  // mark things as ok.
  if (m_recoveryState == RS_IN_RECOVERY) {
    m_recoveryState = RS_OK;        
    COUTRESULT("Successful error recovery.");
  }
}



void UiPlayer::draw_backbuffer(int viewportNum) {
  // Need to clear the screen for reduced drawing.
  if (Xvars::get_reduce_draw()) {
    // Make sure to use fast (hardware) color filling here.
    xvars.color_fill(xvars.m_lpbackbuffer,
                     CRect(0,0,viewSize.width,viewSize.height),
                     xvars.m_black,True);
  }

  // Draw World.
  // World::draw() now takes care of background and outside.
  // We always use background3D on Windows, i.e. background scrolls slower
  // than foreground.
  world->draw(xvars.m_lpbackbuffer,xvars,0,
              viewportAreas[viewportNum],Xvars::get_reduce_draw(),True); 

  // Draw all objects in the game.
  locator->draw_directly(xvars.m_lpbackbuffer,xvars,0,
                         viewportAreas[viewportNum]);  

  // Draw tick marks on edge of screen.
  if (intelsSet[viewportNum]) {
      assert(intels[viewportNum]);
      locator->draw_ticks(xvars.m_lpbackbuffer,xvars,0,
                          viewportAreas[viewportNum],
                          intels[viewportNum]->get_id(),
                          locator);  
  }

  // Draw arena Message.
  if (arenaMessage[viewportNum]) {
    // Draw in center of screen.
    Size vSize = viewportAreas[viewportNum].get_size();
    if (vSize.height > 2 * ARENA_MESSAGE_OFF_CENTER) {
      CRect t_rect(0,0,vSize.width,vSize.height - 2 * ARENA_MESSAGE_OFF_CENTER);
      HDC t_temphdc;
      if (!xvars.GetDC(xvars.m_lpbackbuffer,&t_temphdc)){
        return;
      }

      CDC *t_dc=CDC::FromHandle(t_temphdc);
      t_dc->SelectStockObject(BLACK_PEN);
      CFont *t_oldFont = xvars.select_arena_font(t_dc);
      t_dc->SetBkMode(TRANSPARENT);
      t_dc->SetBkColor(RGB(192,192,192));

      CRect t_winrect = xvars.stretch_rect(t_rect);
      t_dc->SetTextColor(RGB(0,0,0));
      t_dc->DrawText(arenaMessage[viewportNum],-1,t_winrect,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
      // Shift to up and left
      t_winrect.right -= FONT_3D_OFFSET;
      t_winrect.bottom -= FONT_3D_OFFSET;
      t_dc->SetTextColor(RGB(255,0,50));
      t_dc->DrawText(arenaMessage[viewportNum],-1,t_winrect,DT_CENTER|DT_VCENTER|DT_SINGLELINE);

      t_dc->SelectObject(t_oldFont); // So we can free t_temphdc
      xvars.m_lpbackbuffer->ReleaseDC(t_temphdc);
    }
  }
}



void UiPlayer::full_screen_init_x(HWND p_primarywindow) {
  COUTRESULT("full_screen2");
  HRESULT t_result=xvars.m_lpDD->SetCooperativeLevel(p_primarywindow,DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN|DDSCL_ALLOWMODEX);
  DHRESULT(t_result,;);
  COUTRESULT("after set cooperative");
  t_result=xvars.m_lpDD->SetDisplayMode(SCREENDIMX_FULLSCREEN,SCREENDIMY_FULLSCREEN,8);
  COUTRESULT("after set display mode");
  //dont know why I have to do this twice but it was necessary due to lost surface
  t_result=xvars.m_lpDD->SetCooperativeLevel(p_primarywindow,DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN|DDSCL_ALLOWMODEX);
  COUTRESULT("after set cooperative2");
  DHRESULT(t_result,;);
  // Create the primary surface with 1 back buffer
  DDSURFACEDESC t_ddsd;
  t_ddsd.dwSize = sizeof( t_ddsd );

  t_ddsd.dwFlags = DDSD_CAPS ;
  t_ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

  COUTRESULT("full_screen: about to create surface");
  t_result = xvars.m_lpDD->CreateSurface( &t_ddsd, &(xvars.m_lpprimarysurface), NULL );

  COUTRESULT("full_screen created surface?\n");
  DHRESULT(t_result,return);
  COUTRESULT("full_screen created not failed\n");

  // Create back buffer
  xvars.m_lpbackbuffer = xvars.create_surface(viewSize.width,viewSize.height,NULL);

  t_result=xvars.m_lpDD->CreatePalette( DDPCAPS_ALLOW256|DDPCAPS_8BIT, xvars.m_palette.palentries,&xvars.m_ddpalette,NULL);
  t_result=xvars.m_lpprimarysurface->SetPalette( xvars.m_ddpalette );
  xvars.m_bpp = 8;
  ::ShowWindow(p_primarywindow,SW_MAXIMIZE);
  ::SendMessage(p_primarywindow,WM_SETFOCUS,0,0);
}



void UiPlayer::window_screen_init_x(HWND p_primarywindow) {
  HRESULT t_result=xvars.m_lpDD->SetCooperativeLevel(p_primarywindow,DDSCL_NORMAL );
  // Create the primary surface with 1 back buffer
  // Create the primary surface with 1 back buffer
  DDSURFACEDESC t_ddsd;
  t_ddsd.dwSize = sizeof( t_ddsd );

  t_ddsd.dwFlags = DDSD_CAPS ;
  t_ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

  t_result = xvars.m_lpDD->CreateSurface( &t_ddsd, &(xvars.m_lpprimarysurface), NULL );
  DHRESULT(t_result,_exit(33););

  DDSURFACEDESC t_describedesc;
  t_describedesc.dwSize = sizeof( t_ddsd );
  t_result = xvars.m_lpDD->GetDisplayMode( &t_describedesc);
  DHRESULT(t_result,_exit(33););
  xvars.m_bpp = t_describedesc.ddpfPixelFormat.dwRGBBitCount;

  xvars.m_lpbackbuffer = xvars.create_surface(viewSize.width,viewSize.height,NULL);

  if (xvars.uses_palette()) {
    t_result=xvars.m_lpDD->CreatePalette( DDPCAPS_ALLOW256|DDPCAPS_8BIT, xvars.m_palette.palentries,&xvars.m_ddpalette,NULL);
    DHRESULT(t_result,_exit(33););
    t_result=xvars.m_lpprimarysurface->SetPalette( xvars.m_ddpalette );
    DHRESULT(t_result,_exit(33););
  }
}



void UiPlayer::init_x(HWND p_primarywindow) {
  COUTRESULT("entering init_x");

  // Only really needs to be set once, but who cares.
  xvars.errorRecovery = this;

  // Call at the beginning of init_x(), so we can use the Xvars methods.
  xvars.graphicsEnabled = True;


  xvars.m_lpDD = NULL;
  Xvars::read_palette_from_id(IDR_XEVILPAL,xvars.m_palette.palentries);


  // Create some useful pens.
  if (!xvars.m_pens[Xvars::BLACK].m_hObject) {
    xvars.m_pens[Xvars::BLACK].CreatePen(PS_SOLID,1,RGB(0,0,0));
  }
  if (!xvars.m_pens[Xvars::RED].m_hObject) {
    xvars.m_pens[Xvars::RED].CreatePen(PS_SOLID,1,RGB(255,0,0));
  }
  if (!xvars.m_pens[Xvars::WHITE].m_hObject) {
    xvars.m_pens[Xvars::WHITE].CreatePen(PS_SOLID,1,RGB(255,255,255));
  }
 
  // Create some useful brushes.
  if (!xvars.m_brushes[Xvars::BLACK].m_hObject) {
    xvars.m_brushes[Xvars::BLACK].CreateSolidBrush(RGB(0,0,0));
  }
  if (!xvars.m_brushes[Xvars::RED].m_hObject) {
    xvars.m_brushes[Xvars::RED].CreateSolidBrush(RGB(255,0,0));
  }
  if (!xvars.m_brushes[Xvars::WHITE].m_hObject) {
    xvars.m_brushes[Xvars::WHITE].CreateSolidBrush(RGB(255,255,255));
  }


  HRESULT t_result = DirectDrawCreate(NULL,&xvars.m_lpDD,NULL);
  DHRESULT(t_result,_exit(33););

  xvars.m_palette.palversion = 0x300; //default of windows
  xvars.m_palette.numentries = PALENTRIES;//256
  HDC t_hdc=::GetDC(p_primarywindow);
  if (xvars.screenmode != Xvars::FULL_SCREEN_MODE ) {
    xvars.m_hpalette= ::CreatePalette((LOGPALETTE *)&xvars.m_palette);//create a global palette
    ::SelectPalette(t_hdc,xvars.m_hpalette,FALSE);//false is to force us to use our own palette!
    UINT t_uint=::RealizePalette(t_hdc);
    t_uint=::GetSystemPaletteEntries(t_hdc,0,PALENTRIES,xvars.m_palette.palentries);
  }
  // Will be used below, compute now while we have a DC to play with.
  CDC* t_cdc = CDC::FromHandle(t_hdc);
  int t_arenaFontHeight = -((t_cdc->GetDeviceCaps(LOGPIXELSY) * ARENA_FONT_POINT_SIZE) / 72);
  ::ReleaseDC(p_primarywindow,t_hdc);
  if (xvars.screenmode==Xvars::FULL_SCREEN_MODE ) {
    full_screen_init_x(p_primarywindow);
  }
  else {
    window_screen_init_x(p_primarywindow);
  }

  // Set up the color keys.
  xvars.XVARS_COLORKEY.dwColorSpaceLowValue = 
    xvars.color_match(Xvars::COLORKEY_DEF);
  xvars.XVARS_COLORKEY.dwColorSpaceHighValue = 
    xvars.XVARS_COLORKEY.dwColorSpaceLowValue;
  xvars.XVARS_COLORKEY_MASKS.dwColorSpaceLowValue = 
    xvars.color_match(Xvars::MASKCOLORKEY_DEF);
  xvars.XVARS_COLORKEY_MASKS.dwColorSpaceHighValue = 
    xvars.XVARS_COLORKEY_MASKS.dwColorSpaceLowValue;

  // Stock colors.
  xvars.m_black = xvars.color_match(RGB(0x0,0x0,0x0));
  xvars.m_white = xvars.color_match(RGB(0x255,0x255,0x255));
  xvars.m_red = xvars.color_match(RGB(0x0,0x0,0x255));
  xvars.m_green = xvars.color_match(RGB(0x0,0x255,0x0));

  // Set up fonts.
  if (xvars.m_arenaFont) {
    delete xvars.m_arenaFont;
  }
  xvars.m_arenaFont = new CFont();
  if (!xvars.m_arenaFont->CreateFont(t_arenaFontHeight,0,0,0,FW_BOLD,0,0,0,
        DEFAULT_CHARSET,OUT_CHARACTER_PRECIS,CLIP_CHARACTER_PRECIS,
        DEFAULT_QUALITY,DEFAULT_PITCH | FF_DONTCARE,ARENA_FONT_NAME)) {
    delete xvars.m_arenaFont;
    xvars.m_arenaFont = NULL;  // Means use default font
  }

  // Clipper.
  xvars.m_rgndata.rdh.dwSize=sizeof(xvars.m_rgndata.rdh);
  xvars.m_rgndata.rdh.iType=RDH_RECTANGLES;
  xvars.m_rgndata.rdh.nCount=1;
  xvars.m_rgndata.rdh.nRgnSize=sizeof(RECT);
  xvars.m_rgndata.rdh.rcBound.left=0;
  xvars.m_rgndata.rdh.rcBound.right=viewSize.width;
  xvars.m_rgndata.rdh.rcBound.top=0;
  xvars.m_rgndata.rdh.rcBound.bottom=viewSize.height;
  xvars.m_rgndata.rect.left=0;
  xvars.m_rgndata.rect.right=viewSize.width;
  xvars.m_rgndata.rect.top=0;
  xvars.m_rgndata.rect.bottom=viewSize.height;

  t_result = xvars.m_lpDD->CreateClipper(0, &xvars.m_backclipper, NULL);
  DHRESULT(t_result,return);
  t_result = xvars.m_backclipper->SetClipList((LPRGNDATA)&xvars.m_rgndata,0);
  DHRESULT(t_result,return);
  t_result = xvars.m_lpbackbuffer->SetClipper(xvars.m_backclipper);
  DHRESULT(t_result,return);  

  // Initialize sound.
  soundmanager->init(p_primarywindow);

  // Gets incremented when things mess up, then incremented again when the
  // graphics are reinitialized.
  xvars.valid++;

  // Create new SurfaceManager.
  // Add all surfaces to the surface manager regardless of whether we will call init_x_all().
  assert(!xvars.m_surfaceManager);
  xvars.m_surfaceManager = new SurfaceManager(viewSize.height);
  assert(xvars.m_surfaceManager);
  // Reserve space on the SurfaceManager for all surfaces immediately.  We will actully fill the
  // rectangles later.
  add_surfaces_all();
  t_result = xvars.m_surfaceManager->add_complete(xvars);
  DHRESULT(t_result,return);

  COUTRESULT("leaving init_x");
}



BOOL UiPlayer::viewport_to_pos(int n,const Pos &p) 
{
  Pos posOld = viewportAreas[n].get_pos();
  Pos posNew = p - 0.5f * viewportAreas[n].get_size();
  //	posNew.x = (posNew.x / WSQUARE_WIDTH) * WSQUARE_WIDTH;
  //	posNew.y = (posNew.y / WSQUARE_HEIGHT) * WSQUARE_HEIGHT;
  viewportAreas[n].set_pos(posNew);

  return !(posOld == posNew);
}


void UiPlayer::dispatch(int viewportNum,ITcommand command) {
  if (neverReset) {
    return;
  }

  // Can chat even if no associated human.  For Server in observer mode.
  if (command == IT_CHAT) {
    // Only enable Chat mode if the current Role supports it.
    if (Role::uses_chat(m_roletype)) {
      m_chatMessage = "";
      m_chatOn = True;
      update_chat_message();
    }
    return;
  }

  // Normal game-play command.
  if (intelsSet[viewportNum] && intels[viewportNum]->is_playing()) {  
    // Only give the command to a human.
    if (intels[viewportNum]->is_human()) {
      HumanP human = (HumanP)intels[viewportNum];
      human->set_command(command);
    }
    return;
  }

  // No associated intel, so scroll with the keyset. 
  Size size = Intel::command_to_size(command,SHIFT_NO_INTEL);
  Area areaNew = viewportAreas[viewportNum] + size;
  
  if (size.abs_2() != 0 &&  // size != 0
      // Still covering the world a bit
      world->overlap(areaNew.get_box())) {
    viewportAreas[viewportNum] = areaNew;
    redrawAll[viewportNum] = DRAW_ALL;
  }
}



int UiPlayer::get_viewport_num(HWND window) {
  for (int i=0;i<m_windowarray.GetSize();i++) {
    CXEvilWnd *t_view=m_windowarray[i];
    if (window==(t_view->m_hWnd))
      return i;
  }
  return -1;
}



int UiPlayer::get_statsviewport_num(HWND window)
{
#if 0
  if (xvars.screenmode==Xvars::WINDOW_SCREEN_MODE)
  {
    for (int i=0;i<m_windowstatsarray.GetSize();i++)
    {
      XEvilWindowStatsWnd *t_view=(XEvilWindowStatsWnd *)m_windowstatsarray[i];
      if (t_view &&(window==(t_view->m_hWnd)))
        return i;
    }
  }
#endif
  return -1;
}



CXEvilWnd *
UiPlayer::get_viewport_wnd(int p_viewport)
{
  if ((p_viewport<0)||(p_viewport>=m_windowarray.GetSize()))
  {	
    assert(FALSE);
    return NULL;
  }
  return m_windowarray[p_viewport];
}



StatusWnd *
UiPlayer::get_stats_wnd(int p_statsport)
{
  if ((p_statsport<0)||(p_statsport>=m_windowstatsarray.GetSize()))
  {	
    assert(FALSE);
    return NULL;
  }
  return m_windowstatsarray[p_statsport];
}



void UiPlayer::viewport_expose(int viewportNum)
{  
  ASSERT(xvars.screenmode == Xvars::WINDOW_SCREEN_MODE);

  // Redraw arena.
  draw(viewportNum,DRAW_EXPOSE);

}



void UiPlayer::set_prompt_difficulty() {
  // Not used in Windows.
  ASSERT(0);
}



void UiPlayer::doOptions() {
  m_poptions= new OptionsDlg;
  m_poptions->setStyle(settings.style);
  m_poptions->setRoleType(m_roletype);
  m_poptions->setNumHumans(settings.humansNum);
  m_poptions->setNumMachines(settings.enemiesNum);
  m_poptions->setRefillMachines(settings.enemiesRefill);
  m_poptions->setRoomsWide(settings.worldRooms.acrossMax);
  m_poptions->setRoomsHigh(settings.worldRooms.downMax);
  m_poptions->setCooperative(settings.cooperative);
  if (settings.quanta>100) {
    settings.quanta=100;
  }
  m_poptions->setSndTrkVolume(settings.trackvol);
  m_poptions->setEffectsVolume(settings.soundvol);
  m_poptions->setGameSpeed(settings.quanta);
  m_poptions->setMusicType(settings.musictype);
  m_poptions->setSoundOnOff(settings.sound);

#ifdef MODELESS_DIALOGS
  m_poptions->Create(OptionsDlg::IDD,m_windowarray[0]);
  m_poptions->ShowWindow(SW_NORMAL);
  m_poptions->SetFocus();
#else
  if (m_poptions->DoModal() == IDOK) {
    retrieveOptions();    
  }
  delete m_poptions;
  m_poptions = NULL;
#endif
}



void
UiPlayer::retrieveOptions()
{
  if (!m_poptions)
    return;
  if (m_poptions->getStyle()!= settings.style)
    settingsChanges |= UIstyle;
  if (m_poptions->getNumMachines()!= settings.enemiesNum)
    settingsChanges |= UIenemiesNum;
  if (m_poptions->getRefillMachines()!= settings.enemiesRefill)
    settingsChanges |= UIenemiesRefill;
  if (m_poptions->getNumHumans()!= settings.humansNum)
    settingsChanges |= UIhumansNum;
  if (m_poptions->getRoomsWide()!= settings.worldRooms.acrossMax)
    settingsChanges |= UIrooms;
  if (m_poptions->getRoomsHigh()!= settings.worldRooms.downMax)
    settingsChanges |= UIrooms;
  if (m_poptions->getGameSpeed()!= settings.quanta)
    settingsChanges |= UIquanta;
  if (m_poptions->getEffectsVolume()!= settings.soundvol)
    settingsChanges |= UIsoundvol;
  if (m_poptions->getSndTrkVolume()!= settings.trackvol)
    settingsChanges |= UItrackvol;
  if (m_poptions->getMusicType()!= settings.musictype)
    settingsChanges |= UImusictype;
  if (m_poptions->getSoundOnOff() != settings.sound)
    settingsChanges |= UIsound;
  if (m_poptions->getCooperative() != settings.cooperative)
    settingsChanges |= UIcooperative;
  settings.sound = m_poptions->getSoundOnOff();
  settings.style = m_poptions->getStyle();
  settings.humansNum=m_poptions->getNumHumans();
  settings.enemiesNum=m_poptions->getNumMachines();
  settings.enemiesRefill=m_poptions->getRefillMachines();
  settings.worldRooms.acrossMax=m_poptions->getRoomsWide();
  settings.worldRooms.downMax=m_poptions->getRoomsHigh();
  settings.quanta=m_poptions->getGameSpeed();/*0-100*/
  settings.soundvol=m_poptions->getEffectsVolume();
  settings.trackvol=m_poptions->getSndTrkVolume();
  settings.musictype=m_poptions->getMusicType();
  settings.cooperative=m_poptions->getCooperative();  
}



void UiPlayer::retrieveKeys() {
  if (!m_pkeys) {
    return;
  }

  if (m_pkeys->getChanged()) {
    int t_index=m_pkeys->getIndex();
    keycodes[t_index][IT_UP]=m_pkeys->getUpChar();
    keycodes[t_index][IT_UP_L]=m_pkeys->getUpLeftChar();
    keycodes[t_index][IT_UP_R]=m_pkeys->getUpRightChar();
    keycodes[t_index][IT_L]=m_pkeys->getLeftChar();
    keycodes[t_index][IT_R]=m_pkeys->getRightChar();
    keycodes[t_index][IT_DN]=m_pkeys->getDownChar();
    keycodes[t_index][IT_CENTER]=m_pkeys->getCenterChar();
    keycodes[t_index][IT_DN_L]=m_pkeys->getDownLeftChar();
    keycodes[t_index][IT_DN_R]=m_pkeys->getDownRightChar();
    keycodes[t_index][IT_WEAPON_CENTER]=m_pkeys->getWeaponUse();
    keycodes[t_index][IT_WEAPON_CHANGE]=m_pkeys->getWeaponChange();
    keycodes[t_index][IT_WEAPON_DROP]=m_pkeys->getWeaponDrop();
    keycodes[t_index][IT_ITEM_USE]=m_pkeys->getItemUse();
    keycodes[t_index][IT_ITEM_CHANGE]=m_pkeys->getItemChange();
    keycodes[t_index][IT_ITEM_DROP]=m_pkeys->getItemDrop();
    keycodes[t_index][IT_CHAT]=m_pkeys->getChat();

    CString t_plstring("Player1");
    if (t_index==1) {
      t_plstring="Player2";
    }

    CWinApp* pApp = AfxGetApp();
    pApp->WriteProfileInt(t_plstring,"Center",(char)keycodes[t_index][IT_CENTER]);
    pApp->WriteProfileInt(t_plstring,"Right",(char)keycodes[t_index][IT_R]);
    pApp->WriteProfileInt(t_plstring,"DownRight",(char)keycodes[t_index][IT_DN_R]);
    pApp->WriteProfileInt(t_plstring,"Down",(char)keycodes[t_index][IT_DN]);
    pApp->WriteProfileInt(t_plstring,"DownLeft",(char)keycodes[t_index][IT_DN_L]);
    pApp->WriteProfileInt(t_plstring,"Left",(char)keycodes[t_index][IT_L]);
    pApp->WriteProfileInt(t_plstring,"UpLeft",(char)keycodes[t_index][IT_UP_L]);
    pApp->WriteProfileInt(t_plstring,"Up",(char)keycodes[t_index][IT_UP]);
    pApp->WriteProfileInt(t_plstring,"UpRight",(char)keycodes[t_index][IT_UP_R]);
    pApp->WriteProfileInt(t_plstring,"WeaponFire",(char)keycodes[t_index][IT_WEAPON_CENTER]);
    pApp->WriteProfileInt(t_plstring,"WeaponChange",(char)keycodes[t_index][IT_WEAPON_CHANGE]);
    pApp->WriteProfileInt(t_plstring,"WeaponDrop",(char)keycodes[t_index][IT_WEAPON_DROP]);
    pApp->WriteProfileInt(t_plstring,"ItemUse",(char)keycodes[t_index][IT_ITEM_USE]);
    pApp->WriteProfileInt(t_plstring,"ItemChange",(char)keycodes[t_index][IT_ITEM_CHANGE]);
    pApp->WriteProfileInt(t_plstring,"ItemDrop",(char)keycodes[t_index][IT_ITEM_DROP]);
    pApp->WriteProfileInt(t_plstring,"Chat",(char)keycodes[t_index][IT_CHAT]);
  }
}



void UiPlayer::doKeys(UINT p_commandid)
{
  if (m_pkeys)
    return;
  m_pkeys=new CKeySets;
  int t_index;
  if (p_commandid==ID_KEYS)
  {
    m_pkeys->setText(CString("Player 1 keys:"));
    m_pkeys->setIndex(0);
    t_index=0;
  }
  else
  {
    m_pkeys->setText(CString("Player 2 keys:"));
    m_pkeys->setIndex(1);
    t_index=1;
  }
  m_pkeys->setUpChar(keycodes[t_index][IT_UP]);
  m_pkeys->setUpLeftChar(keycodes[t_index][IT_UP_L]);
  m_pkeys->setUpRightChar(keycodes[t_index][IT_UP_R]);
  m_pkeys->setLeftChar(keycodes[t_index][IT_L]);
  m_pkeys->setRightChar(keycodes[t_index][IT_R]);
  m_pkeys->setCenterChar(keycodes[t_index][IT_CENTER]);
  m_pkeys->setDownChar(keycodes[t_index][IT_DN]);
  m_pkeys->setDownLeftChar(keycodes[t_index][IT_DN_L]);
  m_pkeys->setDownRightChar(keycodes[t_index][IT_DN_R]);
  m_pkeys->setWeaponUse(keycodes[t_index][IT_WEAPON_CENTER]);
  m_pkeys->setWeaponChange(keycodes[t_index][IT_WEAPON_CHANGE]);
  m_pkeys->setWeaponDrop(keycodes[t_index][IT_WEAPON_DROP]);
  m_pkeys->setItemUse(keycodes[t_index][IT_ITEM_USE]);
  m_pkeys->setItemChange(keycodes[t_index][IT_ITEM_CHANGE]);
  m_pkeys->setItemDrop(keycodes[t_index][IT_ITEM_DROP]);
  m_pkeys->setChat(keycodes[t_index][IT_CHAT]);
#ifdef MODELESS_DIALOGS
  m_pkeys->Create(CKeySets::IDD,m_windowarray[0]);
  m_pkeys->ShowWindow(SW_NORMAL);
  m_pkeys->SetFocus();
#else
  if (m_pkeys->DoModal() == IDOK) {
    retrieveKeys();
  }
  delete m_pkeys;
  m_pkeys = NULL;
#endif
}



void UiPlayer::doDifficulty()
{
  if (m_pdifficulty)
    return;
  m_pdifficulty = new CDifficulty;
  // Default to NORMAL if none specified yet.
  if (DIFF_NONE== difficulty) {
    difficulty = DIFF_NORMAL;
  }
  m_pdifficulty->setDifficulty(difficulty);
#ifdef MODELESS_DIALOGS
  m_pdifficulty->Create(CDifficulty::IDD,m_windowarray[0]);
  m_pdifficulty->ShowWindow(SW_NORMAL);
  m_pdifficulty->SetFocus();
#else
  if (m_pdifficulty->DoModal() == IDOK) {
    retrieveDifficulty();    
  }
  delete m_pdifficulty;
  m_pdifficulty = NULL;
#endif
}



void UiPlayer::retrieveDifficulty()
{
  if (!m_pdifficulty)
    return;
  // Tell Game the new difficulty.
  settingsChanges |= UIdifficulty;
  settings.difficulty = m_pdifficulty->getDifficulty();
  difficulty = m_pdifficulty->getDifficulty(); // redundant, Game will set it.
}



void UiPlayer::doConnect()
{
  m_pconnect=new CConnectDlg;
#ifdef MODELESS_DIALOGS
  // Running as modeless dialog doesn't fill in the getHostname() and getName() fields.
  m_pconnect->Create(CConnectDlg::IDD,m_windowarray[0]);
  m_pconnect->ShowWindow(SW_NORMAL);
  m_pconnect->SetFocus();
#else
  if (m_pconnect->DoModal() == IDOK) {
    retrieveConnect();    
  }
  delete m_pconnect;
  m_pconnect = NULL;
#endif
}



void UiPlayer::doStory()
{
  m_pstory = new CStoryDlg;
  m_pstory->DoModal();
  delete m_pstory;
  m_pstory = NULL;
}



void UiPlayer::retrieveConnect()
{
  settingsChanges |= UIconnectServer;
  strncpy(settings.connectHostname,m_pconnect->getHostname(),R_NAME_MAX - 1);
  settings.connectHostname[R_NAME_MAX - 1] = '\0';
  settings.connectPort = m_pconnect->getPort();
  strncpy(settings.humanName,m_pconnect->getName(),IT_STRING_LENGTH - 1);
  settings.humanName[IT_STRING_LENGTH - 1] = '\0';
}

  
  
void UiPlayer::doRunServer()
{
  m_prunserver=new CRunServerDlg;
//  m_prunserver->Create(CRunServerDlg::IDD,m_windowarray[0]);
//  m_prunserver->ShowWindow(SW_NORMAL);
//  m_prunserver->SetFocus();
  if (m_prunserver->DoModal() == IDOK) {
    retrieveRunServer();    
  }
  delete m_prunserver;
  m_prunserver = NULL;
}



void UiPlayer::retrieveRunServer()
{
  settingsChanges |= UIrunServer;
  settings.serverPort = m_prunserver->get_port();
  settings.localHuman = m_prunserver->get_local_human();

  // Unspecified name.
  strncpy(settings.humanName,m_prunserver->get_name(),IT_STRING_LENGTH - 1);
  settings.humanName[IT_STRING_LENGTH - 1] = '\0';
}



void UiPlayer::doActivate(WORD p_fActive)
{
  if (p_fActive==WA_ACTIVE)
  {
    COUTRESULT("entering doActivate, check backbuffer (WA_ACTIVE)");
    if (deactivated&&(xvars.screenmode==Xvars::FULL_SCREEN_MODE))
    {
      reset_graphics(xvars.screenmode);
      deactivated=FALSE;
    }
  }
  else if (p_fActive==WA_INACTIVE)
  {
    COUTRESULT("entering doActivate, check backbuffer (WA_INACTIVE)");
    if (xvars.screenmode==Xvars::FULL_SCREEN_MODE)
    {
      deactivated=TRUE;
    }
  }
}



void UiPlayer::add_surfaces_all() {
  world->init_x(xvars,IX_ADD,NULL);
  locator->init_x(xvars,IX_ADD,NULL);
  StatusWnd::init_x(locator,xvars,IX_ADD,NULL);
  
  ClassId cId;
  for (cId = 0; cId <  A_CLASSES_NUM; cId++) {
    const PhysicalContext *pc = locator->get_context(cId);
    if (pc) { 
      assert(pc->init_x);
      // Call init_x() with addOnly set to True.
      // This means just add the surface requests to the SurfaceManager.
      pc->init_x(xvars,IX_ADD,NULL);
    }
  }
}



// Initialize world, locator, StatusWnd (WeaponWheel), and all objects.
void UiPlayer::init_x_all() {
  world->init_x(xvars,IX_INIT,NULL);
  locator->init_x(xvars,IX_INIT,NULL);
  StatusWnd::init_x(locator,xvars,IX_INIT,NULL);
 
  ClassId cId;
  for (cId = 0; cId <  A_CLASSES_NUM; cId++) {
    const PhysicalContext *pc = locator->get_context(cId);
    if (pc) { 
      ostrstream str;
      str << pc->className << " graphics loading " << " (" << 
        (cId + 1) << " of " << (int)A_CLASSES_NUM << ")" << ends;
      // Redraw the screen while we go.
      display_arena_message_sync(str.str());
      COUTRESULT(str.str());
      delete str.str();

      assert(pc->init_x);
      pc->init_x(xvars,IX_INIT,NULL);
    }
  }
}



// e.g. <2,3> ---> "2c"
static CString dir_anim_num_suffix(int dir,int animNum) {
  CString ret;

  // Suffix for the direction number.
  ostrstream dirStr;
  dirStr << dir << ends;

  ret += dirStr.str();
  delete dirStr.str();

  // Suffix indicating the animation frame, no suffix for the 
  // first frame.
  if (animNum > 0) {
    ret += (char)('a' + animNum);
  }
  return ret;
}



void UiPlayer::generate_xpm() {
  // Might be nice to implement this w/o using CString.  Increase future 
  // portability.

  assert(genXPMDir);
  if (!xvars.uses_palette()) {
    AfxMessageBox("Must be in palette mode to generate XPM images.");
    genXPMDir = NULL;
    return;
  }

  if (!*genXPMDir) {
    AfxMessageBox("Empty dest directory name for XPM images.");
    genXPMDir = NULL;
    return;
  }

  // Create CString for easier string manipulation.
  CString genDir(genXPMDir);

  // So we don't go reentrant when we call clock() to print debug info.
  genXPMDir = NULL;

  // Make sure ends in '\'.
  if (genDir[genDir.GetLength() - 1] != '\\') {
    genDir += '\\';
  }
  // E.g. genDir = "c:\somedir\"


  // Make sure top-level destination directory for creating bitmaps exists.
  if (!Utils::is_dir(genDir)) {
    Boolean ret = Utils::mkdir(genDir);
    if (!ret) {
      AfxMessageBox("Could not create top-level directory for XPM images.");
      genXPMDir = NULL;
      return;
    }
  }

  const Boolean halfSize = False;


  // Ask the world to output all its bitmaps.
  display_arena_message_sync("Writing XPM images for World");
  if (!world->generate_xpm(xvars,genDir,halfSize)) {
    AfxMessageBox("Failed to write XPM images for world.");
    return;
  }


  // Ask the Locator to output all its bitmaps.
  display_arena_message_sync("Writing XPM images for Locator");
  if (!locator->generate_xpm(xvars,genDir,halfSize)) {
    AfxMessageBox("Failed to write XPM images for locator.");
    return;
  }


  // Loop over all Physical objects registered with the Locator.
  for (int cNum = 0; cNum < A_CLASSES_NUM; cNum++) {
    const PhysicalContext* cx = locator->get_context(cNum);
    if (!cx) {
      continue;
    }

    ostrstream str;
    str << "Writing XPM images for " << cx->className << ends;
    COUTRESULT(str.str());
    display_arena_message_sync(str.str());
    delete str.str();

    // Replace dashes with underscores for filenames.
    // E.g. "chopper_boy"
    CString fName = cx->className;
    int n;
    for (n = 0; n < fName.GetLength(); n++) {
      if (fName[n] == '-') {
        fName.SetAt(n,'_');
      }
    }

    // E.g. "c:\somedir\chopper_boy\"
    CString classDir = genDir + fName + '\\';

    // Make sure class-specific destination bitmap directory exists.
    if (!Utils::is_dir(classDir)) {
      Boolean ret = Utils::mkdir(classDir);
      if (!ret) {
        AfxMessageBox("Could not create class-specfic directory for XPM images.");
        genXPMDir = NULL;
        return;
      }
    }

    // E.g. "c:\somedir\chopper_boy\chopper_boy_"
    CString fullNameRoot = classDir + fName;

    // Ask PhysicalContext for the list of pixmaps.
    assert(cx->init_x);
    PtrList infoList;
    cx->init_x(xvars,IX_LIST,(void*)&infoList);

    // Look for duplicate pixmaps in infoList.
    // Stores the keys that are used for the SurfaceManager.
    IDictionary* dups = HashTable_factory();

    // Go through list of all pixmaps, writing each to an XPM file.
    // Don't write the same pixmap twice, instead write a warning message.
    for (n = 0; n < infoList.length(); n++) {
      IXPixmapInfo* pInfo = (IXPixmapInfo*)infoList.get(n);
      assert(pInfo);

      // See if we already wrote out this pixmap.
      void* prevEntry = dups->get(pInfo->key);
      if (prevEntry != NULL) {
        // Write file with name describing the duplication to aid developer
        // making the .bitmaps files.
        IXPixmapInfo* pOriginal = (IXPixmapInfo*)prevEntry;

        // E.g. "23_duplicate_of_18"
        CString filename(classDir);
        filename += dir_anim_num_suffix(pInfo->dir,pInfo->animNum) 
                 +  "_duplicate_of_" 
                 +  dir_anim_num_suffix(pOriginal->dir,pOriginal->animNum);

        // Touch the file.
        FILE* fp = fopen(filename,"w");
        fclose(fp);

        // Go to next pixmap in infoList.
        continue;
      }
  

      // Store association between SurfaceManger key and pInfo.
      // The IXPixmapInfo will later tell us what the original 
      // frame when we run into a duplicate.
      // Safe to alias pInfo here because we don't delete the list 
      // of IXPixmapInfo until after dups is gone.
      dups->put(pInfo->key,pInfo);

      // The suffix for direction and animation frame number.
      CString dirANum = dir_anim_num_suffix(pInfo->dir,pInfo->animNum);

      // The complete filename, e.g. "c:\somedir\chopper_boy\chopper_boy_1c"
      CString ffName = fullNameRoot + '_' + dirANum + ".xpm";

      // The variable name to store in the XPM file, e.g. "chopper_boy_1c"
      CString varName = fName + '_' + dirANum;

      Area area;
      const DDCOLORKEY* cKey;
      LPDIRECTDRAWSURFACE surf = 
        xvars.m_surfaceManager->lookup(area,cKey,pInfo->key);

      // Finally.  Write the bastard.
      Boolean val = 
        xvars.write_xpm_file(ffName,varName,surf,area,cKey,halfSize);

      // If error, abort writing all XPM files.
      if (!val) {
        AfxMessageBox("Could not write XPM image.");        
        // To end the outer loop.
        cNum = (A_CLASSES_NUM - 1);
        // To end the inner loop.
        break;
      }        
    } // for n

    delete dups;

    // Delete contents of list, even if we abort.
    for (n = 0; n < infoList.length(); n++) {
      IXPixmapInfo* pInfo = (IXPixmapInfo*)infoList.get(n);
      delete pInfo;
    }
  } // for cNum

  genXPMDir = NULL;
  exit(0);
}



void UiPlayer::display_arena_message_sync(const char* message) {
  for (int n = 0; n < viewportsNum; n++) {
    Utils::freeif(arenaMessage[n]);
    arenaMessage[n] = Utils::strdup(message);
    redrawAll[n] = DRAW_ALL;
  }
  pre_clock();
  post_clock();  
}



void UiPlayer::update_chat_message() {
  // Will clear the message if chat is not turned on.

  CString message;
  if (m_chatOn) {
    message += "CHAT <<";
    message += m_chatMessage;
    message += "\nEnter to send, Esc to cancel.";
  }
  get_stats_wnd(0)->updateStatus(message);
}



void UiPlayer::chat_process_char(char charPressed) {
  assert(m_chatOn);

  // Commit the chat message.
  if (charPressed == VK_RETURN) {
    // Don't bother sending empty messages.
    if (!m_chatMessage.IsEmpty()) {
      settingsChanges |= UIchatRequest;
    
      // No UI for specifying the receiver yet.
      settings.chatReceiver[0] = '\0';

      // Copy over the message.
      Utils::strncpy(settings.chatMessage,m_chatMessage,UI_CHAT_MESSAGE_MAX);
      settings.chatMessage[UI_CHAT_MESSAGE_MAX] = '\0';
    }

    // Turn off chat mode.
    m_chatOn = False;
    update_chat_message();
  }  

  // Abort the chat message.
  else if (charPressed == VK_ESCAPE) {
    m_chatOn = False;
    update_chat_message();
  }

  // Delete prev char.
  else if (charPressed == VK_BACK) {  
    // VK_DELETE doesn't seem to come in a WM_CHAR message.

    if (!m_chatMessage.IsEmpty()) {
      // There must be a better way to remove the last character.
      m_chatMessage = m_chatMessage.Left(m_chatMessage.GetLength() - 1);
    }
    update_chat_message();
  }

  // Only add printable characters to the chat message.
  else if (isprint(charPressed)) {
    // Don't add white space to the beginning of the message.
    if (!m_chatMessage.IsEmpty() || !isspace(charPressed)) {
      m_chatMessage += charPressed;
      update_chat_message();      
    }
  }
}



Boolean UiPlayer::attemptRecovery() {
  switch (m_recoveryState) {
    case RS_OK:
      // Attempt recovery at the soonest possible time.
      m_recoveryState = RS_MUST_RECOVER;
      COUTRESULT("Graphics Error: Disabling graphics and setting the error-recovery flag.");
      // Disable graphics to avoid more errors before we attempt the recovery.
      return False;
      break;

    case RS_MUST_RECOVER:
      // Flag already set, do nothing.  Keep graphics disabled until error
      // recovery time.
      return False;
      break;

    // Don't attempt recovery again unless we are sure the last recovery
    // attempt was successful.  Don't get into an infinite loop trying to
    // recover over and over.
    //
    // MS DirectDraw fucks us again.  Application will probably crash soon.
    case RS_IN_RECOVERY:
      m_recoveryState = RS_RECOVERY_FAILED;
      COUTRESULT("Error while trying to recover from error, give up error recovery.");
      // Keep graphics enabled.  We're screwed, but might as well keep trying
      // to draw.
      return True;
      break;

    // Things are messed up.  We will never leave this state.  As before, we
    // might as well keep trying to draw.  Can't make it worse.
    case RS_RECOVERY_FAILED:
      return True;

    default:
      assert(0);
      return False;
  }
}



const char* UiPlayer::genXPMDir = NULL;



const char* UiPlayer::bmpSrcDir = NULL;



const char* UiPlayer::bmpDestDir = NULL;



VInfoProvider UiPlayer::viewInfo;
