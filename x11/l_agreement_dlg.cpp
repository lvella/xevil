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

// "l_agreement_dlg.cpp"

// No corresponding header file, the only entry point is through
// LAgreement::check_accpted().


#include "utils.h"
extern "C" {
#include <X11/Xutil.h>
#include <X11/Xatom.h>
}

#include <strstream.h>

#include "xdata.h"
#include "panel.h"
#include "l_agreement.h"


#define PREV_LABEL "Prev"
#define NEXT_LABEL "Next"
#define ACCEPT_LABEL "Accept"
#define REJECT_LABEL "Reject"



class LAgreementDlg {
public:
  enum {
    ACCEPTED,REJECTED,UNDECIDED,
  };

  LAgreementDlg(Xvars &,int dpyNum,int argc,char** argv,
                Boolean lgViewportVal,Boolean smoothScrollVal,
                Boolean reduceDrawVal);

  ~LAgreementDlg();

  void process_event(int dpyNum,XEvent *event);

  int get_status() {return status;}
  /* EFFECTS: Return ACCEPTED, REJECTED or ,UNDECIDED. */

  Boolean get_large_viewport() 
    {return optionToggles[LARGE_VIEWPORT]->get_value();}
  /* EFFECTS: Return whether we should use large viewports for XEvil. */

  Boolean get_smooth_scroll() 
    {return optionToggles[SMOOTH_SCROLL]->get_value();}

  Boolean get_reduce_draw() 
    {return !optionToggles[DRAW_BACKGROUNDS]->get_value();}
  /* NOTE: The opposite of DRAW_BACKGROUNDS. */


private:
  enum {SHAREWARE_1,PREV,NEXT,ACCEPT,REJECT,SHAREWARE_2,BUTTONS_MAX};
  enum {LARGE_VIEWPORT,SMOOTH_SCROLL,DRAW_BACKGROUNDS,OPTIONS_MAX};
  static const char* buttonLabels[BUTTONS_MAX];
  static const char* optionToggleLabels[OPTIONS_MAX];
  static const char* optionTextText[OPTIONS_MAX];

  enum {
    TEXT_PADDING = 3,
    BUTTON_VERT_PADDING = 6,
    OPTION_HORIZ_PADDING = 8,
    BUTTONS_EDGE_PADDING = 10, // On outside edges.
  };

  void check_viewed_before();
  /* EFFECTS: Check for an .xevilrc  Sets viewedBefore and rcFilename. */

  void mark_as_viewed();
  /* EFFECTS: Create a .xevilrc to mark that the user has viewed the
     dialog at least once. */

  void redraw();

  void parse(const char*);

  void compute_button_pos(Pos buttonPos[BUTTONS_MAX],
                          Size buttonSizes[BUTTONS_MAX],
                          int widthMax,
                          int top);
  /* MODIFIES: buttonPos */
  /* EFFECTS: Compute the location of all the "buttons" including the 
     shareware labels.  widthMax is the available space.
     top is the very top of the buttons. */

  static void button_callback(Panel*,void*,void*);
  void _button_callback(Panel*);
  Boolean get_button_sensitive(int);


  Xvars* xvars;
  int separatorY_1;
  int separatorY_2;
  int dpyNum;
  int status;

  // Little hack here, not all LAgreementDlg::buttons are really buttons,
  // also used for the obnoxious "SHAREWARE" labels.
  TextPanel* buttons[BUTTONS_MAX];

  PtrList pages;
  int currentPage;

  Size windowSize;
  Size fontSize;
  Window window;

  TogglePanel* optionToggles[OPTIONS_MAX];
  TextPanel* optionText[OPTIONS_MAX];

  // Has the User already seen the license agreement dialog in some
  // previous use of XEvil.
  Boolean viewedBefore;
  // The absolute filename of the .xevilrc file.
  char* rcFilename;
};



// This method is way too big, split it up a bit.
LAgreementDlg::LAgreementDlg(Xvars &x_vars,int dpy_num,int argc,char** argv,
                             Boolean lgViewportVal,Boolean smoothScrollVal,
                             Boolean reduceDrawVal) {
  xvars = &x_vars;
  dpyNum = dpy_num;
  status = UNDECIDED; // User hasn't said yes or no yet.

  // Initialize this up at the top, so we get the right button states.
  currentPage = 0;
  
  // Create content model, the list of pages.
  parse(LAgreement::get_text());


  // We should really just put this in Xvars.
  fontSize = xvars->fontSize[dpyNum];

  // Compute sizes for the buttons.
  Size buttonSizes[BUTTONS_MAX];
  int n;
  for (n = 0; n < BUTTONS_MAX; n++) {
    buttonSizes[n] = 
      TextPanel::get_unit(xvars->font[dpyNum],
                          Utils::strlen(buttonLabels[n]));
  }
  // Just used for the height, for two rows.
  Size optionTextUnit = 
    TextPanel::get_unit(xvars->font[dpyNum],1,2);


  // Create window
  windowSize.width = 
    fontSize.width * Line::get_text_columns() 
    + 2 * TEXT_PADDING;
  windowSize.height = 
    fontSize.height * Page::get_text_rows()
    + 2 * TEXT_PADDING // around the text in the main area
    + 1 // separator
    + (OPTIONS_MAX + 1) * TEXT_PADDING
    + OPTIONS_MAX * optionTextUnit.height
    + 1 // separator
    + 2 * BUTTON_VERT_PADDING
    + buttonSizes[0].height;

  window = 
    xvars->create_toplevel_window(argc,argv,dpyNum,windowSize,
                                  "XEvil License Agreement",ExposureMask);


  // Compute location of the top separator line.
  separatorY_1 = 
    fontSize.height * Page::get_text_rows()
    + 2 * TEXT_PADDING;


  // Compute max width of the option buttons.
  int optionsMaxWidth = 0;
  for (n = 0; n < OPTIONS_MAX; n++) {
    Size opSize = 
      TogglePanel::get_unit(xvars->font[dpyNum],
                            Utils::strlen(optionToggleLabels[n]),1);
    optionsMaxWidth = Utils::maximum(optionsMaxWidth,opSize.width);
  }
    

  // Now create the toggle buttons and corresponding text.
  Pos optionPos(OPTION_HORIZ_PADDING,separatorY_1 + 1 + TEXT_PADDING);
  for (n = 0; n < OPTIONS_MAX; n++) {
    // Create toggle button.
    Size optionToggleUnit = 
      TogglePanel::get_unit(xvars->font[dpyNum],
                            strlen(optionToggleLabels[n]),1);
    Pos togglePos(OPTION_HORIZ_PADDING,
                  optionPos.y + optionTextUnit.height / 2 
                  - optionToggleUnit.height / 2);
    optionToggles[n] = 
      new TogglePanel(dpyNum,*xvars,window,
                      togglePos,optionToggleUnit,
                      // Don't care about callback.  We will poll result when 
                      // dialog is popped down.
                      NULL,NULL,
                      optionToggleLabels[n]);
    assert(optionToggles[n]);

    // Create text for toggle button.
    Pos textPos(2 * OPTION_HORIZ_PADDING + optionsMaxWidth,
                optionPos.y);
    Size textUnit = 
      TextPanel::get_unit(xvars->font[dpyNum],strlen(optionTextText[n]),2);
    optionText[n] = 
      new TextPanel(dpyNum,*xvars,window,
                    textPos,textUnit,
                    NULL,NULL,
                    optionTextText[n],
                    // Don't want a border.
                    TEXT_PANEL_EVENT_MASK,False);
    assert(optionText[n]);

    optionPos.y += optionTextUnit.height;
  }

  // Set initial values
  optionToggles[LARGE_VIEWPORT]->set_value(lgViewportVal);
  optionToggles[SMOOTH_SCROLL]->set_value(smoothScrollVal);
  optionToggles[DRAW_BACKGROUNDS]->set_value(!reduceDrawVal);
  

  // Location of the bottom separator line.
  separatorY_2 =
    separatorY_1 + 1 
    + (OPTIONS_MAX + 1) * TEXT_PADDING
    + OPTIONS_MAX * optionTextUnit.height;


  // Before creating the buttons.
  // Sets initial value of viewedBefore.
  check_viewed_before();


  // Figure out where to place the buttons.
  Pos buttonPos[BUTTONS_MAX];
  compute_button_pos(buttonPos,buttonSizes,
                     windowSize.width,
                     separatorY_2 + 1 + BUTTON_VERT_PADDING);

  // Create the buttons
  for (n = 0; n < BUTTONS_MAX; n++) {

    // Shareware labels.
    if (n == SHAREWARE_1 || n == SHAREWARE_2) {
      buttons[n] = 
        new TextPanel(dpyNum,*xvars,window,
                      buttonPos[n],buttonSizes[n],
                      NULL,NULL,buttonLabels[n],
                      TEXT_PANEL_EVENT_MASK,
                      False);
    }
    // Normal buttons.
    else {
      buttons[n] = 
        new ButtonPanel(dpyNum,*xvars,window,
                        buttonPos[n],buttonSizes[n],
                        button_callback,(void*)this,
                        buttonLabels[n]);
    }
    assert(buttons[n]);
    buttons[n]->set_sensitive(get_button_sensitive(n));
  }
  
  // Pop up the window.
  XMapWindow(xvars->dpy[dpyNum],window);
}



LAgreementDlg::~LAgreementDlg() {
  Utils::freeif(rcFilename);
  
  int n;
  for (n = 0; n < BUTTONS_MAX; n++) {
    delete buttons[n];
  }

  for (n = 0; n < OPTIONS_MAX; n++) {
    delete optionToggles[n];
    delete optionText[n];
  }

  XDestroyWindow(xvars->dpy[dpyNum],window);

  for (n = 0; n < pages.length(); n++) {
    delete (Page*)pages.get(n);
  }
}



void LAgreementDlg::process_event(int dpyNum,XEvent *event) {
  // Received WM_DELETE_WINDOW.
  if (event->type == ClientMessage && event->xclient.format == 32 &&
      event->xclient.data.l[0] == xvars->wmDeleteWindow[dpyNum] &&
      event->xany.window == window) {
    status = REJECTED;
  }

  // Get expose, window close, and delegate to children
  if (event->type == Expose 
      && event->xexpose.window == window) {
    redraw();
    return;
  }

  // See if the toggle buttons or toggle labels want it.
  int n;
  for (n = 0; n < OPTIONS_MAX; n++) {
    if (optionToggles[n]->process_event(dpyNum,event)) {
      return;
    }
    if (optionText[n]->process_event(dpyNum,event)) {
      return;
    }
  }
    
  // Give event to first button that wants it.
  for (n = 0; n < BUTTONS_MAX; n++) {    
    if (buttons[n]->process_event(dpyNum,event)) {
      return;
    }
  }
}



void LAgreementDlg::check_viewed_before() {
  // Now that XEvil is GPL, we don't force the user to page through the
  // license agreement the first time XEvil is run, i.e. no more .xevilrc.
  viewedBefore = True;
  rcFilename = NULL;

#if 0
  const char* home = Utils::getenv("HOME");
  if (!home || !*home) {
    viewedBefore = False;
    rcFilename = NULL;
    return;
  }

  ostrstream str;
  str << home;
  if (home[strlen(home)-1] != '/') {
    str << "/";
  }
  str << ".xevilrc" << ends;

  // Keep rcFilename around even if the file doesn't exist yet.
  rcFilename = str.str();

  FILE* fp = fopen(rcFilename,"r");
  if (!fp) {
    viewedBefore = False;
    return;
  }
  else {
    viewedBefore = True;
    fclose(fp);
  }
#endif
}



void LAgreementDlg::mark_as_viewed() {
  if (rcFilename) {
    FILE* fp = fopen(rcFilename,"w");
    if (fp) {
      fprintf(fp,"XEvil is your friend.  Trust XEvil.\n");
      fclose(fp);
    }
  }
  // else HOME env variable is not available, so give up.
}



void LAgreementDlg::redraw() {
  XClearWindow(xvars->dpy[dpyNum],window);

  // Loop over all lines on current page.
  Page* page = (Page*)pages.get(currentPage);
  const PtrList& lines = page->get_lines();  
  for (int n = 0; n < lines.length(); n++) {
    int length;
    const char* text = ((Line*)lines.get(n))->get_text(length);
    if (length > 0) {
      XDrawString(xvars->dpy[dpyNum],window,xvars->gc[dpyNum],
                  TEXT_PADDING,
                  TEXT_PADDING 
                  + xvars->font[dpyNum]->max_bounds.ascent 
                  + fontSize.height * n,
                  text,length);                           
    }
  }

  // Draw separators.
  XDrawLine(xvars->dpy[dpyNum],window,xvars->gc[dpyNum],
            0,separatorY_1,windowSize.width - 1,separatorY_1);
  XDrawLine(xvars->dpy[dpyNum],window,xvars->gc[dpyNum],
            0,separatorY_2,windowSize.width - 1,separatorY_2);

  // Buttons will redraw themselves.
}
 

 
void LAgreementDlg::parse(const char *text) {
  const char *p = text;

  Line::set_text_columns(70);
  Page::set_text_rows(24);
  
  while (*p) {
    Page* page = new Page(&p,p);
    pages.add(page);
  }
}



void LAgreementDlg::compute_button_pos(Pos buttonPos[BUTTONS_MAX],
                                       Size buttonSizes[BUTTONS_MAX],
                                       int widthMax,
                                       int top) {
  buttonPos[SHAREWARE_1].x = BUTTONS_EDGE_PADDING;
  buttonPos[SHAREWARE_1].y = top;
  buttonPos[SHAREWARE_2].x = 
    widthMax - BUTTONS_EDGE_PADDING - buttonSizes[SHAREWARE_2].width;
  buttonPos[SHAREWARE_2].y = top;

  int bWidth = widthMax - 2 * BUTTONS_EDGE_PADDING - 
    buttonSizes[SHAREWARE_1].width - buttonSizes[SHAREWARE_2].width;

  for (int n = SHAREWARE_1 + 1; n < SHAREWARE_2; n++) {
    buttonPos[n].x = 
      buttonPos[SHAREWARE_1].x + buttonSizes[SHAREWARE_1].width +
      (int)((float)n / (float)(BUTTONS_MAX - 1) * (float)bWidth)
      - (buttonSizes[n].width / 2);
    buttonPos[n].y = top;
  }
#if 0
    Pos pos((int)((n + 1.0f) / (float)(BUTTONS_MAX + 1) 
                  * windowSize.width) 
            - (buttonSizes[n].width / 2),
            );
#endif
}



void LAgreementDlg::button_callback(Panel* panel,void*,void* closure) {
  LAgreementDlg* dlg = (LAgreementDlg*)closure;
  dlg->_button_callback(panel);
}

 

void LAgreementDlg::_button_callback(Panel* panel) {
  // Which button was pressed.  Search through four buttons, big deal.
  int button;
  for (button = 0; button < BUTTONS_MAX; button++) {
    if (buttons[button] == panel) {
      break;
    }
  }
  assert(button < BUTTONS_MAX);


  switch (button) {
  case PREV:
    assert(currentPage > 0);
    currentPage--;
    break;
  case NEXT:
    assert(currentPage < pages.length() - 1);
    currentPage++;
    break;
  case ACCEPT:
    // So user won't have to click through it again.
    mark_as_viewed(); 
    status = ACCEPTED;
    // Don't bother with a redraw.
    return;
  case REJECT:
    status = REJECTED;
    // Don't bother with a redraw.
    return;
  default:
    assert(0);
  }

  // Update whether buttons are sensitive or not.
  for (int n = 0; n < BUTTONS_MAX; n++) {
    buttons[n]->set_sensitive(get_button_sensitive(n));
  }
  redraw();
}




Boolean LAgreementDlg::get_button_sensitive(int button) {
  switch (button) {
  case PREV:
    // If not the first page.
    return currentPage > 0;
  case NEXT:
    // If not the last page.
    return currentPage < pages.length() - 1;
  case ACCEPT:
  case REJECT:
    // If the last page or if user has already seen the dialog at least
    // once.
    return viewedBefore || (currentPage == pages.length() - 1);
  case SHAREWARE_1:
  case SHAREWARE_2:
    return True;
  default:
    assert(0);
    return False;
  }
}

   

// Don't write "SHAREWARE".  XEvil is now GPL.
const char* LAgreementDlg::buttonLabels[BUTTONS_MAX] = {
  "_",    //  "SHAREWARE",
  "Prev",
  "Next",
  "Accept",
  "Reject",
  "_",    //  "SHAREWARE",
};



const char* LAgreementDlg::optionToggleLabels[OPTIONS_MAX] = {
  "Large Viewport",
  "Smooth Scroll",
  "Draw Background",
};



const char* LAgreementDlg::optionTextText[OPTIONS_MAX] = {
  "Unselect [Large Viewport] to speed up drawing,\n"
  "or to play two-player on one machine.",
  
  "Use [Smooth Scroll] if you have a machine\n"
  "with fast graphics.",

  "Only meaningful for smooth scroll.\n"
  "Unselect [Draw Background] to speed up drawing."
};



Boolean LAgreement::check_accepted(Boolean& largeViewport,
                                   Boolean& smoothScroll,
                                   Boolean& reduceDraw,
                                   Xvars &xvars,
                                   int dpyNum,
                                   int argc,char** argv) {
  // User ran "xevil -accept_agreement".
  if (commLineAccepted) {
    return True;
  }

  LAgreementDlg dlg(xvars,dpyNum,argc,argv,
                    largeViewport,smoothScroll,reduceDraw);

  // Run the dialog modally until the user chooses one way or the other.
  while (1) {
    // Grab X events.
    XEvent event;
    XNextEvent(xvars.dpy[dpyNum],&event);

    // Let the dialog handle it, return if a decision has been made.
    dlg.process_event(dpyNum,&event);
    int val = dlg.get_status();

    // Poll value of the option toggle buttons.
    largeViewport = dlg.get_large_viewport();
    smoothScroll = dlg.get_smooth_scroll();
    reduceDraw = dlg.get_reduce_draw();

    if (val == LAgreementDlg::REJECTED) {
      return False;
    }
    else if (val == LAgreementDlg::ACCEPTED) {
      return True;
    }
  }

  // LAgreementDlg destructor will pop down window.
}
