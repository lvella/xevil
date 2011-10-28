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

// "l_agreement.h"

#ifndef L_AGREEMENT_H 
#define L_AGREEMENT_H

#if X11
#ifndef NO_PRAGMAS
#pragma interface
#endif
#endif

#include "utils.h"
class Xvars;

// Holds one line of text.
class Line {
public:
  Line(const char **textRet,const char *text);
  /* EFFECTS: Parse text into a line of text, set textRet to be the next
     char in the "input stream". */
  
  const char* get_text(int &lngth) {lngth = length; return start;}
  /* MODIFIES: length */
  /* EFFECTS: Return the line of text.  Not NULL terminated, use "length"
     instead. */

  char* alloc_text();
  /* EFFECTS: Returns a new copy of the text for this line, null-terminated.
     Free memory with "delete []" when done. */

  static void set_text_columns(int val) {textColumns = val;}
  static int get_text_columns() {return textColumns;}


private:
  // Format to this number of columns.
  static int textColumns; 

  const char *start;
  int length;
};



// One page of the license agreement.
class Page {
public:
  Page(const char **textRet,const char *text);
  /* EFFECTS: Parse text into a page of lines, set textRet to be the next
     char in the "input stream".  If textRet is NULL, put all text in 
     one page. */

  ~Page();

  static void set_text_rows(int val) {textRows = val;}
  static int get_text_rows() {return textRows;}

  const PtrList&get_lines() {return lines;}
  /* EFFECTS: Return the list of lines in this page. */


private:
  PtrList lines;
  static int textRows;
};



class LAgreement {
public:
#if X11
  static Boolean check_accepted(Boolean& largeViewport,
                                Boolean& smoothScroll,
                                Boolean& reduceDraw,
                                Xvars&,int dpyNum,
                                int argc,char** argv);
  /* MODIFIES: largeViewport, smoothScroll, reduceDraw */
  /* EFFECTS: Run modal dialog to force the user to read the license agreement
     and accept/reject it.  Initialize checkbox with value of largeViewport,
     smoothScroll.  On return, set largeViewport, smoothScroll to the final 
     value of the checkbox. */
  /* NOTE: Body of this function lives in l_agreement_dlg.cpp */
#endif

  static void comm_line_accepted() 
    {commLineAccepted = True;}
  /* EFFECTS: User accepted license agreement with the command line. */

  static const char *get_text();
  /* EFFECTS: Return the text of the license agreement.  Guaranteed to be 
     in static memory. */


private:
  static Boolean commLineAccepted;
};

#endif
