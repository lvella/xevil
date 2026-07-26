/*
 * XEvil(TM) Copyright (C) 1994,2000 Steve Hardt and Michael Judge
 * http://www.xevil.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.  See gpl.txt.
 */

// "dialog.h"  SDL port -- the modal first-run License Agreement dialog.
//
// The X11 build shows LAgreement::check_accepted() (x11/l_agreement_dlg.cpp)
// every launch unless -accept_agreement.  Windows/SDL users double-click an exe
// and never pass flags, so the SDL port shows the agreement ONCE (gated on the
// ~/.xevilrc acceptance marker, exactly the marker the shared cmn config writer
// uses) and remembers Accept forever.  Content, the Prev/Next paging, the
// Accept/Reject buttons and the three render toggles all mirror the X11 dialog.

#ifndef SDL_DIALOG_H
#define SDL_DIALOG_H

#include "utils.h"
#include "xdata.h"

// Result of the modal License Agreement dialog.
struct LicenseResult {
  Boolean accepted;        // False == user chose Reject / closed the window.
  Boolean largeViewport;   // Final value of the three render toggles.
  Boolean smoothScroll;
  Boolean drawBackground;  // (reduceDraw is the inverse of this.)
};

LicenseResult sdl_run_license_dialog(Xvars &xvars,int dpyNum,
                                     Boolean largeViewport,
                                     Boolean smoothScroll,
                                     Boolean drawBackground);
/* REQUIRES: xvars display dpyNum is open.  Resizes that window for the dialog.
   EFFECTS: Run the license agreement modally (its own SDL event pump) until the
   user Accepts or Rejects, and return the decision plus the final toggle
   values.  The caller restores the game window size afterwards. */

// ---- ~/.xevilrc acceptance marker (shared with cmn/game.cpp config) ----
Boolean sdl_agreement_marker_present();
/* EFFECTS: True iff ~/.xevilrc already exists with the acceptance marker line
   (i.e. the user accepted in a previous session, or any settings were saved). */

void sdl_agreement_write_marker();
/* EFFECTS: Ensure ~/.xevilrc exists carrying the acceptance marker as line 1,
   so the next launch skips the dialog.  No-op if the file already exists (the
   cmn config writer always writes the marker first). */

#endif
