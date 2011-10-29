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

#ifndef _XVARS_H
#define _XVARS_H

#include <iostream>
#include <strstrea.h>
#include "ddraw.h"

#include "utils.h"
#include "coord.h"
#include "s_man.h"


// Methinks we could get rid of one of these.
#define PALETTENTRIES 256
#define PALENTRIES 256

#define Xvars_BACKGROUND RGB(192,192,192)

typedef LPDIRECTDRAWSURFACE CMN_DRAWABLE;
typedef UINT CMN_BITS_ID;
typedef COLORREF CMN_COLOR;

typedef UINT XvarsValid;
#define XVARS_VALID_INIT 0



enum IXCommand {
  /* Add all surfaces to the SurfaceManager. */
  IX_ADD, 
  /* Initialize all graphics if they aren't already valid. */
  IX_INIT, 
  /* List all bitmaps used by the class.  Argument is a pointer to a PtrList.
     Fill the PtrList with newly allocated IXPixmapInfo structures.  Caller
     must free the IXPixmapInfo structs with delete. */
  IX_LIST, 
};



struct IXPixmapInfo {
  void* key;       // Key into the SurfaceManager.
  Dir dir;         // Direction corresponding to bitmap.
  int animNum;     // Number of this bitmap in animation sequence for dir.
};



struct CMN_IMAGEDATA {
  LPDIRECTDRAWSURFACE pixmap;
	LPDIRECTDRAWSURFACE mask;
  // offsets are stretched coordinates.
  Pos pixmapOffset;
  Pos maskOffset;
};



struct CMN_EVENTDATA {
  HWND m_parent;
  MSG m_event;
};



class FireXdata {
public:
  FireXdata() {valid = XVARS_VALID_INIT;}
  XvarsValid valid;
}; 



struct XEVIL_RGNDATA {
  RGNDATAHEADER rdh;
  RECT rect;
};



struct ColorPalette {
  WORD palversion;
  WORD numentries;
  PALETTEENTRY palentries[PALENTRIES];
};



// An object that knows how to try and recover from DirectDraw errors.
class IErrorRecovery {
public:
  virtual Boolean attemptRecovery() = 0;
  /* EFFECTS: Try to recover from a DirectDraw error.  Return whether graphics
     should still be enabled. */
};



// This class has become sufficiently complicated that we should really
// encapsulate all the data in it.  Way too many public fields.
class Xvars {
  // Kill this fucker.  Need to separate Xvars and UiPlayer.
  friend class UiPlayer;

public:
  Xvars();
  ~Xvars();

  Boolean is_valid(XvarsValid check) {return check == valid;}
  /* EFFECTS: Check whether some graphics-user is valid against the current
     Xvars. */

  void mark_valid(XvarsValid& val) {val = valid;}
  /* MODIFIES: val */
  /* EFFECTS: Set val to be valid with the current Xvars. */

  enum SCREENMODE {
    WINDOW_SCREEN_MODE=0,
    FULL_SCREEN_MODE=1
  };
  enum {
    DISPLAY_NAME_LENGTH = 80,
	  HUMAN_COLORS_NUM = 6
  };
  enum {
    BLACK = 0,
    RED,
    WHITE,
    MAXCOLORS
  };
  COLORREF backgroundColor;

  COLORREF humanColors[HUMAN_COLORS_NUM];
  CPen m_pens[MAXCOLORS];
  CBrush m_brushes[MAXCOLORS];
  LPDIRECTDRAW m_lpDD;
  LPDIRECTDRAWPALETTE m_ddpalette;
  ColorPalette m_palette;
  HPALETTE m_hpalette;
  SCREENMODE screenmode;
  int m_bpp; // Bits per pixel

  // Some stock colors color matched to the primary surface.
  // Should probably put this in an array of MAXCOLORS.
  DWORD m_black;
  DWORD m_white;
  DWORD m_red;
  DWORD m_green;

  SurfaceManager* m_surfaceManager;

  // The global error recovery object.
  static IErrorRecovery* errorRecovery;


  int bytes_per_pixel() {return m_bpp >> 3;}

  BOOL uses_palette() {return m_bpp == 8;}

  LPDIRECTDRAWSURFACE create_surface(DWORD width,DWORD height,DDCOLORKEY *colorkey);
  /* EFFECTS: Create an offscreen surface of given dimensions.  
     Stretched coordinates.  Will set source colorkey if colorkey is non-NULL. */

  DWORD color_match(COLORREF rgb);
  /* EFFECTS: Match a colorref to a value in the primary surface/backbuffer. 
     Works in both palette and non-palette modes. */

  static void read_palette_from_id(UINT p_palid, PALETTEENTRY ppe[PALETTENTRIES]);
  /* EFFECTS: Read in a palette resource. */


  // Color Key
  DDCOLORKEY XVARS_COLORKEY;
  DDCOLORKEY XVARS_COLORKEY_MASKS ;
  enum {
    COLORKEY_DEF = RGB(255,0,255),
    MASKCOLORKEY_DEF = RGB(255,255,255)
  };

  // This now loads surfaces for all bit depths.
  // p_dest should be in stretched coordinates.
  Boolean load_surface(LPDIRECTDRAWSURFACE p_surface,const CPoint &p_dest,UINT p_bitmapid);
  // Similar to load_surface.  But, will stretch p_bitmapid to double size.
  // p_dest is still in stretched coordinates on p_surface.
  Boolean load_stretch_surface(LPDIRECTDRAWSURFACE p_surface,const CPoint &p_dest,UINT p_bitmapid);

  //used to translate bitmaps with one resource 
  //read a FILE and write the correct palette
  Boolean read_bitmap_write_file(FILE *p_input, FILE *p_output);

  // Stretch methods.
  int stretch_x(int x) {return x * m_stretch;}
  int stretch_y(int y) {return y * m_stretch;}
  Area stretch_area(const Area &);
  CRect stretch_rect(const CRect &rect)
    {return CRect(rect.left*m_stretch,rect.top*m_stretch,rect.right*m_stretch,rect.bottom*m_stretch);}
  Size stretch_size(const Size &s) 
    {Size ret; ret.set(s.width*m_stretch,s.height*m_stretch); return ret;}
  CPoint stretch_point(const CPoint &point)
    {return CPoint(point.x * m_stretch,point.y * m_stretch);}
  Pos stretch_pos(const Pos &pos)
    {return Pos(pos.x * m_stretch,pos.y * m_stretch);}

  //contain methods
  CRect contain_rect(const CRect &rect, int maxwidth, int maxheight, int minwidth =0, int minheight =0)
    {return CRect(min(max(minwidth,rect.left),maxwidth),min(max(minheight,rect.top),maxheight)
    ,min(max(minwidth,rect.right),maxwidth),min(max(minheight,rect.bottom),maxheight));}

  static BOOL color_match_palette(PALETTEENTRY *p_source, PALETTEENTRY *p_dest, int p_numentries);

  BOOL hresultSuccess(HRESULT); 
  // if fails, increments idnum and returns False

  static void interpretDHresult(HRESULT p_result);
  // If DebugInfo is on, print a message appropriate for p_result.  Return whether

  static void DHresultRecover(HRESULT p_result);
  // Print out error message and try to recover from the error, if not DD_OK.

  // Really need some way of returning an error.
  void generate_mask_from_pixmap(LPDIRECTDRAWSURFACE mask,const Pos& maskoffset,
                                 LPDIRECTDRAWSURFACE pixmap,const Pos& pixmapoffset,
                                 const Size& size);
  /* EFFECTS: Generate mask from pixmap.  All values are stretched coordinates. */

  void generate_pixmap_from_transform(LPDIRECTDRAWSURFACE dest,const Pos& destOffset,
                                      LPDIRECTDRAWSURFACE src,const Pos& srcOffset,
                                      const Size& srcSize,
                                      LPDIRECTDRAWSURFACE scratch,
                                      const TransformType* transforms,int tNum);
  /* REQUIRES: scratch is at least as big as the max of src and dest */
  /* EFFECTS: Generate one pixmap from another based on some transformations.
     All values are stretched coordinates. */

  CFont* select_arena_font(CDC *);
  /* EFFECTS: Select the font appropriate for drawing in the arena into the CDC. 
     return prev selected object. */
  /* NOTE: Perhaps we'll need a version that takes a HDC. */

  HRESULT color_fill(LPDIRECTDRAWSURFACE,const CRect &,DWORD color,Boolean fast = False);
  /* EFFECTS: Fill the specified rect on the surface in the given color.
     Basically the same as DDBLT_COLORFILL.  If fast is True, use directdraw to fill even
     if we aren't sure whether it works on the current machine. */

  HRESULT Blt(LPDIRECTDRAWSURFACE dest,
              DWORD destX,DWORD destY,
              LPDIRECTDRAWSURFACE src, 
              LPRECT srcRect,
              Boolean srcColorKey = False,
              LPDIRECTDRAWCLIPPER = NULL);
  HRESULT Blt(LPDIRECTDRAWSURFACE dest,
              const Pos &destPos,
              LPDIRECTDRAWSURFACE src,
              const Pos &srcPos,
              const Size &size,
              Boolean srcColorKey = False,
              LPDIRECTDRAWCLIPPER = NULL);
  /* EFFECTS: Copy a rectangular region from one surface to another. 
     Stretched coordinates. */
  /* NOTE: Xvars::Blt now does clipping against negative numbers for
     dest position.  That's the only clipping it does, however. */

  Boolean GetDC(LPDIRECTDRAWSURFACE surf, HDC *dc);
  /* EFFECTS: Gets the device context of the passed in surface are returns is in *dc.
     will return False if null surf or surface GetDC gives error. */ 
  
  
  Boolean write_xpm_file(const char* filename,const char* varName,
                         LPDIRECTDRAWSURFACE surf,
                         const Area& area,const DDCOLORKEY* cKey,
                         Boolean halfSize);
  /* REQUIRES: The directory for filename exists and is writable. */
  /* EFFECTS: Write a directdraw surface to a file in the XPM format.  Return whether
     successful.  area gives, in stretched coordinates, the rectangular area of the 
     source pixmap on surf.  varName gives the name to use as the C/C++ static 
     variable defined by the XPM file.  If ckey is non-NULL it gives the color key 
     for surf, to generate transparent pixels in the XPM file.  If halfSize, write
     out an XPM file that is half the size of the source bitmap. */

  static Boolean get_reduce_draw() {return reduceDraw;}

  static void set_reduce_draw(Boolean val) {reduceDraw = val;}

  static void enable_graphics(Boolean val){graphicsEnabled = val;}
  
private:
  static DWORD write_read_color_match(IDirectDrawSurface *pdds, COLORREF rgb);
  static int single_color_match(PALETTEENTRY rgb, PALETTEENTRY *p_palette, int p_numentries);

  // Only uiplayer uses this.
  Area un_stretch_area(const Area &area);

  inline BOOL pixel_equals(BYTE*,BYTE*,int bytesPerPixel);
  inline void pixel_set(BYTE*dest,BYTE*src,int bytesPerPixel);

  // The guts of load_surface().
  Boolean load_surface_palette(LPDIRECTDRAWSURFACE p_surface,const CPoint &p_destrect,UINT p_bitmapid);
  Boolean load_surface_no_palette(LPDIRECTDRAWSURFACE p_surface,const CPoint &p_destrect,UINT p_bitmapid,BOOL p_stretch = False);

  void gen_pix_from_trans(LPDIRECTDRAWSURFACE dest,const Pos& destOffset,
                          LPDIRECTDRAWSURFACE src,const Pos& srcOffset,
                          const Size& srcSize,
                          TransformType transform);
  /* REQUIRES: dest is large enough for the transformed version of src. */
  /* EFFECTS: Helper for generate_pixmap_from_transform.  Do one transformation, 
     from source to dest.  Coords are stretched. */

  BYTE xpm_map_pixels(BYTE* srcPix,int srcPixNum,Boolean halfSize);

  short read_param(FILE *p_file,void *p_dest,unsigned short p_bytecount);


  // Stretching methods/data
  int m_stretch;
  inline Size unStretchSize(const Size &size){Size ret; ret.set(size.width/m_stretch,size.height/m_stretch); return ret;}

  // Use select_arena_font to use arenaFont.
  CFont *m_arenaFont; // Font for drawing in the middle of the screen

  LPDIRECTDRAWSURFACE m_lpprimarysurface;
  LPDIRECTDRAWSURFACE m_lpbackbuffer;
  XEVIL_RGNDATA m_rgndata;
  LPDIRECTDRAWCLIPPER m_backclipper;

  static Boolean reduceDraw;

  //this keeps the windowed pallete fresh in xdata for the next 
  //bitmap. Only one color_matching is necessary
  PALETTEENTRY *m_pWinPalette;
  // Use this to tell if specific Xdata's are valid or not.
  // increment it to mark everything invalid.
  XvarsValid valid;

  // If False, disable all graphics operations.
  // Ick, making this static.
  static Boolean graphicsEnabled;
};



inline BOOL Xvars::pixel_equals(BYTE*p1,BYTE*p2,int bytesPerPixel) {
  for (int n = 0; n < bytesPerPixel; n++) {
    if (p1[n] != p2[n]) {
      return False;
    }
  }
  return TRUE;
}



inline void Xvars::pixel_set(BYTE*dest,BYTE*src,int bytesPerPixel) {
  for (int n = 0; n < bytesPerPixel; n++) {
    dest[n] = src[n];
  }
}



// Debugging macros.
#define COUTRESULT(x)  DebugInfo::print(x);

// Attempt error recovery if not DD_OK.
#define DHRESULT(hresult,action) if (hresult!=DD_OK) \
  { Xvars::DHresultRecover(hresult);\
    action;}



struct Wxdata {
//  LPDIRECTDRAWSURFACE blocksSurface;
  // In unstretch coords.
//  CRect blocks[W_ALL_BLOCKS_NUM];
//  CRect doors[W_ALL_DOORS_NUM];
//  CRect moverSquare[W_ALL_MOVER_SQUARES_NUM];
//  CRect movers[W_ALL_MOVERS_NUM];
  
//  LPDIRECTDRAWSURFACE backgroundSurfaces[W_ALL_BACKGROUNDS_NUM];
//  LPDIRECTDRAWSURFACE outsideSurfaces[W_ALL_OUTSIDES_NUM];
//  LPDIRECTDRAWSURFACE posterSurfaces[W_ALL_POSTERS_NUM];
};



struct OLxdata {
//  LPDIRECTDRAWSURFACE buffer;
//  LPDIRECTDRAWSURFACE scratchBuffer;
//  LPDIRECTDRAWSURFACE ticks;
//  int tickOffsets[TICK_MAX][CO_DIR_HALF_PURE]; // in window coords.
};



class ProtectionXdata {
public:
  ProtectionXdata() {valid = XVARS_VALID_INIT;}

  CPen color;
  XvarsValid valid;
};



class MovingXdata {
public:
  MovingXdata() {valid = XVARS_VALID_INIT; offsetsValid = False;}
  XvarsValid valid;
  // Ripped everything out, now in SurfaceManager, only thing left is the flag
  // that says whether we've been initialized or not.
  // Could keep a copy of the hash keys here.

  // Hack, This snippet of data is completely independent of init_x(), etc.
  Size offsets[CO_DIR_MAX];  
  Boolean offsetsValid;
};

// All dummy on Windows.
class LogFile {
public:
  LogFile() {}
  ~LogFile() {}

  void set_file_name(const char*) {}

  void enable();

  void disable();
};

#endif

