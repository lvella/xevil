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

// XEvil.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "XEvil.h"
#include "game.h"
#include "mmsystem.h"

/*
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
*/

#include "aboutdialog.h"
#include "timedlg.h"
#include "l_agreement.h"

/////////////////////////////////////////////////////////////////////////////
// CXEvilApp


BEGIN_MESSAGE_MAP(CXEvilApp, CWinApp)
	//{{AFX_MSG_MAP(CXEvilApp)
	//}}AFX_MSG_MAP
	// Standard file based document commands
//	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
//	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CXEvilApp construction

CXEvilApp::CXEvilApp()
:m_game(NULL)
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}
CXEvilApp::~CXEvilApp()
{
	if (m_game)
		delete m_game;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CXEvilApp object

CXEvilApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CXEvilApp initialization
#define MAX_COMMANDLINEPARAMETERS 50

void 
CXEvilApp::prepareCommandLine(const CString &p_commandline,int &p_argc, char **&p_argv)
{
	p_argv=new char *[MAX_COMMANDLINEPARAMETERS];
	p_argc=1;
	p_argv[0]="";//leave first one empty
	int t_index;
	CString t_string;
	for (t_index=0;t_index<p_commandline.GetLength();t_index++)
	{
		t_string="";
		if (p_commandline[t_index]!=' ')
		{//process string
			while((t_index<p_commandline.GetLength())&&(p_commandline[t_index]!=' '))
				t_string+=p_commandline[t_index++];
			p_argv[p_argc]=new char[t_string.GetLength()+1];
			strcpy(p_argv[p_argc],t_string.GetBuffer(0));
            p_argc++;
            if (p_argc==MAX_COMMANDLINEPARAMETERS)
                return;
		}
	}
}


// The timebomb
BOOL CXEvilApp::timeStampcheck() {
// Disabled for final release.
#if 0
  CTime current = CTime::GetCurrentTime();

// Beta 7
  CTime earliest(1998,12,1,0,0,0);  // Dec 1, 1998
  CTime latest(1999,4,1,0,0,0);    // Apr 1, 1999

// Beta 6
//  CTime earliest(1998,1,1,0,0,0);  // Jan 1, 1998
//  CTime latest(1999,1,1,0,0,0);    // Jan 1, 1999

// Beta 5
//  CTime earliest(1997,7,1,0,0,0);  // July 1, 1997
//  CTime latest(1998,7,1,0,0,0);    // July 1, 1998

  if (current <= earliest) {
    return FALSE;
  }
  if (current >= latest) {
    return FALSE;
  }  
#endif
  return TRUE;
}



class TurnStarter: public ITurnStarter {
public:
  TurnStarter(int &timer) 
    : m_timer(timer) {
  }
  /* EFFECTS: Set timer to be the start time for the turn when 
     startTurn() is called. */
  
  virtual void start_turn() {
    m_timer = timeGetTime();
  }


private:
  int &m_timer;
};



BOOL CXEvilApp::InitInstance() {
  // Standard initialization
  // If you are not using these features and wish to reduce the size
  //  of your final executable, you should remove from the following
  //  the specific initialization routines you do not need.
  int t_argc;
  char **t_argv;
  prepareCommandLine(m_lpCmdLine,t_argc,t_argv);
  int t_fullscreencmdlineindex=-1;
  BOOL t_fullscreenmode=TRUE;
  int t_reducedrawindex=-1;
  if (FAILED(CoInitialize(NULL)))
  {
    AfxMessageBox("Error loading COM");
    return FALSE;
  }

  for (int i=0;i<t_argc;i++)
  {
    if (!strcmp(t_argv[i],"-full_screen"))
    {
      t_fullscreencmdlineindex=i;
    }
    else if (!strcmp(t_argv[i],"-window_screen"))
    {
      t_fullscreenmode=FALSE;
    }
    else if (!strcmp(t_argv[i],"-reduce_draw"))
    {
      t_reducedrawindex=i;;
    }
  }

  AboutDialog t_dlg;
  t_dlg.enableFullScreenButton(TRUE);
  t_dlg.setFullScreenMode(t_fullscreenmode);
  t_dlg.enableReduceDraw(TRUE);
  t_dlg.setReduceDraw(t_reducedrawindex != -1);
  t_dlg.setVersionString(Game::get_version_string());
  t_dlg.setLicenseText(LAgreement::get_text());
  if (t_dlg.DoModal()!= IDOK) {
    exit(0);
  }
  
  t_fullscreenmode=t_dlg.getFullScreenMode();
  if (t_fullscreenmode&&(t_fullscreencmdlineindex== -1)) {
    t_argv[t_argc++]=_strdup("-full_screen");
  }
  if (!t_fullscreenmode&&(t_fullscreencmdlineindex!= -1)) {
    t_argv[t_fullscreencmdlineindex]=_strdup("-window_screen");
  }
  if (t_dlg.getReduceDraw() && t_reducedrawindex == -1) {
    t_argv[t_argc++]=_strdup("-reduce_draw");
  }
  if (!t_dlg.getReduceDraw() && t_reducedrawindex != -1) {
    t_argv[t_reducedrawindex]=_strdup("-dummy_arg");
  }

  BOOL t_timecheck=timeStampcheck();
  if (!t_timecheck)
  {
    TimeStampFailedDlg t_timefailed;
    t_timefailed.DoModal();
    return FALSE;
  }
  Utils::seed_random();

#ifdef _AFXDLL
  Enable3dControls();			// Call this when using MFC in a shared DLL
#else
  Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

  LoadStdProfileSettings(0);  // Load standard INI file options (including MRU)

  // Register the application's document templates.  Document templates
  //  serve as the connection between documents, frame windows and views.

  CTypedPtrList<CPtrList,CWnd *> t_wndlist;
  srand((unsigned int)time(NULL));
  WORD wVersionRequested;  
  WSADATA wsaData; 
  int err; 
  wVersionRequested = MAKEWORD(2, 0); 

  err = WSAStartup(wVersionRequested, &wsaData); 

  m_game = new Game(&t_argc,t_argv);
  m_game->getWindowList(t_wndlist);
  if (clock()== -1)
	  assert (FALSE);
  long total = 0;
  long events = 0;
  CMN_EVENTDATA t_event;//for messasge reception
  int t_timer;
  BOOL t_gameover=FALSE;


  // TurnStarter exists so Game::yield() can decide when to start timing the next turn.
  TurnStarter turnStarter(t_timer);
  // Start the first turn.
  turnStarter.start_turn();

  // Main loop
  while (!t_gameover)
  {
    // wait till it's time to wake up
    int t_wndCount = m_game->getWindowList(t_wndlist);
    Quanta quanta = m_game->get_quanta(); 

    // Clock the game
    total++;
    POSITION t_pos;

    m_game->pre_clock();
    // If there is no UI at all, don't check for events.
    if (t_wndCount != -1) {
      for( t_pos = t_wndlist.GetHeadPosition(); t_pos != NULL; )
      {
	      CWnd *t_wnd=t_wndlist.GetNext(t_pos);
	      assert(t_wnd);							
        t_event.m_parent=t_wnd->m_hWnd;
	      while (PeekMessage(&t_event.m_event,t_wnd->m_hWnd,NULL,NULL,PM_REMOVE))
	      { 
          if (TranslateMessage(&t_event.m_event))
            break;
		      if (!m_game->process_event(0,t_event))
            break;
	      }
      }
    }
    m_game->post_clock();
    if (m_game->show_stats() && !(total % Game::REPORT_TIME)) {
	    cout << "total:" << total << " events:" << events << 
	    " percent:" << ((float)events / (float) total) << endl; 
    }

    if (m_game->quit_game()) {
      t_gameover = TRUE;
      break;
    }
    m_game->yield(t_timer,quanta,&turnStarter);
  } 

  for (i=1;i<t_argc;i++) {
    delete t_argv[i];
  }
  delete t_argv;
  return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About


/////////////////////////////////////////////////////////////////////////////
// CXEvilApp commands




int CXEvilApp::ExitInstance() 
{
	// TODO: Add your specialized code here and/or call the base class
  CoUninitialize();
	return CWinApp::ExitInstance();
}

 
