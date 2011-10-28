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
#include "xdata.h"
#include <iostream.h>
#include <strstrea.h>
#include "ui.h"



struct XEvilBitmapFileStruct
{
    DWORD  biSize; //size of structure
    LONG   biWidth; 
    LONG   biHeight; 
    WORD   biPlanes; 
    WORD   biBitCount;
    DWORD  biCompression; 
    DWORD  biSizeImage; 
    LONG   biXPelsPerMeter; 
    LONG   biYPelsPerMeter; 
    DWORD  biClrUsed; 
    DWORD  biClrImportant; 
    PALETTEENTRY m_palette[PALENTRIES];
}; //used to load bitmaps from resource



class XPMSymbols {
public:
  XPMSymbols(u_int valuesNeeded);

  void* newSymbols();
  /* EFFECTS: Generate an encoding of a new set of symbols for an 
     XPM color.  Return value will never be NULL. */

  void getSymbols(u_char ret[3],void* key);
  /* MODIFIES: ret */
  /* EFFECTS: Make ret into a NULL-terminated string of length get_symbols_num()
     containing the symbols for the encoded XPM color. */

  int get_symbols_num() {assert(symbolsNum < 3); return symbolsNum;}
  /* EFFECTS: Return the number of symbols used to represent the range of 
     values passed in to the constructor.  Always 1 or 2. */

  static void* DUMMY;
  /* This is a special non-NULL value that is guaranteed never to be 
     returned by newSymbols(). */


private:
  void generate_potential_symbols();
  /* EFFECTS: Generate the list of potential symbols used for creating the
     XPM Colors. */

  // Either 1 or 2.
  int symbolsNum;
  // The next set of symbols to use, index into potentialSymbols
  int gen[2];
  // +1 is just to make debug printing easy.
  static u_char potSymbols[256 + 1]; 
  static int potSymbolsNum;
};
 


XPMSymbols::XPMSymbols(u_int valuesNeeded) {
  if (potSymbolsNum == -1) {
    generate_potential_symbols();
  }

  assert(valuesNeeded > 0 && (int)valuesNeeded <= potSymbolsNum * potSymbolsNum);


  // Can do it with only one symbol.
  if ((int)valuesNeeded <= potSymbolsNum) {
    symbolsNum = 1;
  }
  // We'll need two of them.
  else {
    symbolsNum = 2;
  } 

  // Initialize symbol generator.
  gen[0] = gen[1] = 0;
}



void* XPMSymbols::newSymbols() {
  // Doesn't assert if you ask for too many symbols.

  u_int ret = 0x0;

  if (symbolsNum == 1) {
    ret = (u_int)potSymbols[gen[0]];
    gen[0]++;
  }
  else if (symbolsNum == 2) {
    ret = (u_int)potSymbols[gen[1]] << 8 | 
          (u_int)potSymbols[gen[0]];
    
    // Do two-digit counting.
    if (gen[0] == 255) {
      gen[0] = 0;
      gen[1]++;
    }
    else {
      gen[0]++;
    }
  }
  else {
    assert(0);
  }

  assert(ret);
  return (void*)ret;
}  



void XPMSymbols::getSymbols(u_char ret[3],void* key) {
  u_int kkey = (u_int)key;
  ret[0] = (u_char)(kkey & 0xff);
  if (symbolsNum == 1) {
    ret[1] = '\0';
  }
  else if (symbolsNum == 2) {
    ret[1] = (u_char)(kkey & 0xff00);
    ret[2] = '\0';
  }
  else {
    assert(0);
  }

  // This wil never return XPMSymbols::DUMMY, which is 0xffffffff.
}



void XPMSymbols::generate_potential_symbols() {
  assert(potSymbolsNum == -1);
  potSymbolsNum = 0;

  int sym;
  for (sym = 0; sym < 256; sym++) {
    // Use all graphical symbols.
    if (isgraph(sym) && sym != '\"' && sym != '\\') {
      potSymbols[potSymbolsNum] = (u_char)sym;
      potSymbolsNum++;
    }
  }
  potSymbols[potSymbolsNum] = '\0';
  COUTRESULT("XPM Symbols list is");
  COUTRESULT((char*)potSymbols);
}



u_char XPMSymbols::potSymbols[256 + 1];



// Means not set yet.
int XPMSymbols::potSymbolsNum = -1;



void* XPMSymbols::DUMMY = (void*)0xffffffff;



Xvars::Xvars() {
  m_arenaFont = NULL;
  m_lpDD = NULL;
  m_surfaceManager = NULL;
  m_pWinPalette = NULL;
}	  



Xvars::~Xvars() {
  for(int i=0;i<MAXCOLORS;i++) {
    m_pens[i].DeleteObject();
    m_brushes[i].DeleteObject();
  }
  delete m_arenaFont;
  m_lpDD->Release();
  delete m_surfaceManager;
  if (m_pWinPalette){
    delete []m_pWinPalette;
  }
}



LPDIRECTDRAWSURFACE 
Xvars::create_surface(DWORD width,DWORD height,DDCOLORKEY *colorkey) {
  if (!graphicsEnabled) {
    return NULL;
  }

  DDSURFACEDESC       ddsd;
  HRESULT             ddrval;
  LPDIRECTDRAWSURFACE psurf;
  
  /*
   * fill in surface desc
   */
  memset( &ddsd, 0, sizeof( ddsd ) );
  ddsd.dwSize = sizeof( ddsd );
  ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
  ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
  
  ddsd.dwHeight = height;
  ddsd.dwWidth = width;

  ddrval = m_lpDD->CreateSurface( &ddsd, &psurf, NULL );
  DHRESULT(ddrval,return NULL);

  // Set color key if specified.
  if (colorkey) {
    ddrval = psurf->SetColorKey(DDCKEY_SRCBLT,colorkey);
    DHRESULT(ddrval,return NULL);
  }

  return psurf;
}



// Use GDI SetPixel to do the color match.
DWORD
Xvars::write_read_color_match(IDirectDrawSurface *pdds, COLORREF rgb) {
  if (!graphicsEnabled) {
    return 0;
  }

  COLORREF rgbT;
  HDC hdc;
  DWORD dw = CLR_INVALID;
  DDSURFACEDESC ddsd;
  HRESULT hres;

  //
  //  use GDI SetPixel to color match for us
  //
  if (rgb != CLR_INVALID && pdds->GetDC(&hdc) == DD_OK)
  {
      rgbT = GetPixel(hdc, 0, 0);             // save current pixel value
      SetPixel(hdc, 0, 0, rgb);               // set our value
      pdds->ReleaseDC(hdc);
  }

  //
  // now lock the surface so we can read back the converted color
  //
  ddsd.dwSize = sizeof(ddsd);
  while ((hres = pdds->Lock(NULL, &ddsd, 0, NULL)) == DDERR_WASSTILLDRAWING)
      ;

  if (hres == DD_OK)
  {
      dw  = *(DWORD *)ddsd.lpSurface;                     // get DWORD
      // For "True Color" we get an overflow with the 
      // following logic, but in this case we don't want
      // to mask out any bits anyway.  hardts
      if (ddsd.ddpfPixelFormat.dwRGBBitCount < 32) {
        dw &= (1 << ddsd.ddpfPixelFormat.dwRGBBitCount)-1;  // mask it to bpp
      }
      pdds->Unlock(NULL);
  }

  //
  //  now put the color that was there back.
  //
  if (rgb != CLR_INVALID && pdds->GetDC(&hdc) == DD_OK)
  {
      SetPixel(hdc, 0, 0, rgbT);
      pdds->ReleaseDC(hdc);
  }

  return dw;
}



int
Xvars::single_color_match(PALETTEENTRY rgb, PALETTEENTRY *p_palette, int p_numentries) {
  if (!graphicsEnabled) {
    return 0;
  }

  int ret = -1;
  LONG minSum;
  for (int j = 0; j < p_numentries; j++) {
    LONG diff;
    LONG sumSquare = 0;
      
    // Switching components, Red <-> Blue.
    diff = abs(p_palette[j].peRed - rgb.peBlue);
    sumSquare += diff * diff;
    diff = abs(p_palette[j].peGreen - rgb.peGreen);
    sumSquare += diff * diff;
    diff = abs(p_palette[j].peBlue - rgb.peRed);
    sumSquare += diff * diff;

    // Found better match than existing one.
    if (ret == -1 || sumSquare < minSum) {
      ret = j;
      minSum = sumSquare;
    }
    // Perfect match.
    if (sumSquare == 0) {
      break;
    }
  }
  return ret;
}



DWORD Xvars::color_match(COLORREF rgb) {
  if (!graphicsEnabled) {
    return 0;
  }

  if (uses_palette()) {
    PALETTEENTRY t_entry;
    t_entry.peRed = GetRValue(rgb);
    t_entry.peGreen = GetGValue(rgb);
    t_entry.peBlue = GetBValue(rgb);
    return (DWORD)single_color_match(t_entry,m_palette.palentries,PALENTRIES);
  }
  // not color matched with palette, ignore palette
  else {
    // Changed from using m_lpprimarysurface to using m_lpbackbuffer because 
    // of problems locking primary surface on some win95 machines.
    return write_read_color_match(m_lpbackbuffer,rgb);
  }
}



BOOL 
Xvars::color_match_palette(PALETTEENTRY *p_source, PALETTEENTRY *p_dest, int p_numentries) {
  for (int i = 0;i < p_numentries;i++) {
    BYTE t_index = single_color_match(p_dest[i],p_source,p_numentries);
    p_dest[i].peRed = t_index;
  }
  return TRUE;
}



Boolean Xvars::load_surface(LPDIRECTDRAWSURFACE p_surface,const CPoint &p_dest,UINT p_bitmapid)
{
  if (!p_bitmapid || !p_surface) {
    return False;
  }

  if (uses_palette()) {
    return load_surface_palette(p_surface,p_dest,p_bitmapid);
  }
  else {
    return load_surface_no_palette(p_surface,p_dest,p_bitmapid,False);
  }
}



Boolean Xvars::load_surface_no_palette(LPDIRECTDRAWSURFACE p_surface,const CPoint &p_dest,
                                 UINT p_bitmapid,Boolean p_stretch) {
  if (!graphicsEnabled) {
    return False;
  }

  // Get DC to draw to  p_surface
  HDC t_surfacedc;
  if (!GetDC(p_surface, &t_surfacedc))
    return False;

  // Load bitmap and select it into t_dc
  CDC t_dc;
  t_dc.CreateCompatibleDC(CDC::FromHandle(t_surfacedc));
  CBitmap t_bittemp;
  if (!t_bittemp.LoadBitmap(p_bitmapid)) {
    COUTRESULT("Xvars::load_surface_no_palette, LoadBitmap fails");
    // should free t_pixmapdc
    p_surface->ReleaseDC(t_surfacedc);  
    return False;
  }
  BITMAP t_bitmapInfo;
  t_bittemp.GetBitmap(&t_bitmapInfo);
  
  CSize t_size(t_bitmapInfo.bmWidth,t_bitmapInfo.bmHeight);
  t_dc.SelectObject(&t_bittemp);

  // Copy bitmap onto t_surfacedc
  if (p_stretch) {
    CDC::FromHandle(t_surfacedc)->StretchBlt(
      p_dest.x,p_dest.y,
      stretch_x(t_size.cx),stretch_y(t_size.cy),
      &t_dc,
      0,0,
      t_size.cx,t_size.cy,
      SRCCOPY);
  }
  else {
    CDC::FromHandle(t_surfacedc)->BitBlt(
      p_dest.x,p_dest.y,
      t_size.cx,t_size.cy,
      &t_dc,0,0,SRCCOPY);
  }
  t_bittemp.DeleteObject();

  // Free DC for surface
  p_surface->ReleaseDC(t_surfacedc);  

  return TRUE;
}



Boolean Xvars::load_surface_palette(LPDIRECTDRAWSURFACE p_surface,const CPoint &p_dest,
                                    UINT p_bitmapid) {
  if (!graphicsEnabled) {
    return False;
  }

  // get a pointer to the bitmap resource.
  // then lock the surface using the destrect.
  HRSRC h = FindResource(NULL, MAKEINTRESOURCE(p_bitmapid), RT_BITMAP);
  if (!h) {
    return False;
  }

  // Load bitmap
  XEvilBitmapFileStruct *lpbi = 
    (XEvilBitmapFileStruct *)LockResource(LoadResource(NULL, h));
  BYTE *t_bitmapbits = ((BYTE *)lpbi) + sizeof(XEvilBitmapFileStruct);

  if (!m_pWinPalette && screenmode != Xvars::FULL_SCREEN_MODE) {
    m_pWinPalette =  new PALETTEENTRY[PALENTRIES];
    memcpy(m_pWinPalette,lpbi->m_palette,PALENTRIES * sizeof(PALETTEENTRY));

    // match palette and shove results into the t_palette[x].pRed
    Xvars::color_match_palette(m_palette.palentries,m_pWinPalette,PALENTRIES);
  }
  else if (m_pWinPalette && screenmode == Xvars::FULL_SCREEN_MODE){
      //must have switched modes. clean up old palette
      delete []m_pWinPalette;
      m_pWinPalette = NULL;
  }
  // each row is DWORD aligned. 32bit
  int t_widthbytes = lpbi->biWidth;
  while ((t_widthbytes & 3) != 0) {
	  t_widthbytes++;
  }

  // Loop over all rows, from bottom to top.
  for (int y = lpbi->biHeight - 1; y >= 0; y--) {
    // The destination rect on p_surface corresponding to 
    // one scan line.
    CRect t_destrect(p_dest.x,
                     p_dest.y + y,
                     p_dest.x + lpbi->biWidth,
                     p_dest.y + y + 1);

    // Lock the destination row on the surface
    DDSURFACEDESC t_sdesc;
    t_sdesc.dwSize = sizeof(t_sdesc);
    HRESULT t_result;
    t_result = p_surface->Lock(t_destrect,&t_sdesc,
                               DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT|DDLOCK_WRITEONLY,NULL);
//    t_result = p_surface->Lock(NULL,&t_sdesc,0,NULL);
    DHRESULT(t_result,return False);

    // Copy bits from t_bitmapbits to t_surfacemem.
    BYTE* t_surfacemem = (BYTE *)t_sdesc.lpSurface;

    if (screenmode == Xvars::FULL_SCREEN_MODE) {
      memcpy(t_surfacemem,t_bitmapbits,lpbi->biWidth);
    }
    else {
      for (int x = 0; x < lpbi->biWidth; x++) {
        t_surfacemem[x] = m_pWinPalette[*(t_bitmapbits + x)].peRed;
      }
    }
    // Unlock the destination row.
    t_result = p_surface->Unlock(NULL);
    DHRESULT(t_result,return False);

    // Advance to the next row.
    t_bitmapbits += t_widthbytes;
  }

  return TRUE;
}



Boolean Xvars::read_bitmap_write_file(FILE *p_input, FILE *p_output) {
  if (!graphicsEnabled) {
    return False;
  }

  BITMAPFILEHEADER bmpfileheader_struct;
  unsigned char *bmpfileheader=(unsigned char *)&bmpfileheader_struct;
  BITMAPINFOHEADER bmpinfoheader_struct;
  unsigned char *bmpinfoheader=(unsigned char *)&bmpinfoheader_struct;
#define GET_2B(array,offset)  ((unsigned short) (unsigned char)(array[offset]) + \
			       (((unsigned short) (unsigned char)(array[offset+1])) << 8))
#define GET_4B(array,offset)  ((long) (unsigned char)(array[offset]) + \
			       (((long) (unsigned char)(array[offset+1])) << 8) + \
			       (((long) (unsigned char)(array[offset+2])) << 16) + \
			       (((long) (unsigned char)(array[offset+3])) << 24))
  long biWidth = 0;		/* initialize to avoid compiler warning */
  int bPad;
  DWORD t_index=0;/*index to stream*/
  if (!read_param(p_input,bmpfileheader,14)) {
  	return False;/*cant read fileheader*/
  }

  //write new file
  fwrite(bmpfileheader,sizeof(BITMAPFILEHEADER),1,p_output);

  /* Read and verify the bitmap file header */
  if (GET_2B(bmpfileheader,0) != 0x4D42) {/* 'BM' */
  	return False;
  }
  if (! read_param(p_input, bmpinfoheader, 4)) {
	  return False;//cant read 4 bytes
  }
  if (bmpinfoheader_struct.biSize < 12 || bmpinfoheader_struct.biSize > 64) {
  	return False;
  }
  if (! read_param(
      p_input, 
      bmpinfoheader+4,
      (short)(bmpinfoheader_struct.biSize-4))) {/*casting here is ok beacause of check for >64 above*/
    return False;
  }

  fwrite(bmpinfoheader, 1 ,bmpinfoheader_struct.biSize ,p_output);
  RGBQUAD      t_rgbquad[PALENTRIES];
  for (int i=0;i<PALENTRIES;i++) {
    t_rgbquad[i].rgbRed = m_palette.palentries[i].peBlue;
    t_rgbquad[i].rgbGreen = m_palette.palentries[i].peGreen;
    t_rgbquad[i].rgbBlue = m_palette.palentries[i].peRed;
    t_rgbquad[i].rgbReserved = 0;
  }
  fwrite(&t_rgbquad,sizeof(RGBQUAD),PALENTRIES,p_output);
  
  bPad = bmpfileheader_struct.bfOffBits - (bmpinfoheader_struct.biSize + 14);
  bPad -= sizeof(PALETTEENTRY) * PALENTRIES;
  if (bPad < 0) {
    return False; //bad pad data
  }

  PALETTEENTRY t_palette[PALENTRIES];
  read_param(p_input,t_palette,PALENTRIES * sizeof(PALETTEENTRY)); 
  //we just read the wrong thing. we need to swap red and blue!
  //in the file are RGBQUADS not PALETTEENTRY. god bless windows and little Endian
  for (i=0;i<PALENTRIES;i++) {
    int t_temp=t_palette[i].peRed;
    t_palette[i].peRed = t_palette[i].peBlue;
    t_palette[i].peBlue = t_temp;
  }
  // each row is DWORD aligned. 32bit

  // match palette and shove results into the t_palette[x].pRed
  Xvars::color_match_palette(m_palette.palentries,t_palette,PALENTRIES);

  if (bPad) {
    unsigned char *t_dummydata = new unsigned char[bPad];
    read_param(p_input, t_dummydata, bPad);
    fwrite(t_dummydata,1,bPad,p_output);
    delete []t_dummydata;
  }

  // Loop over all rows, from bottom to top.
  biWidth = bmpinfoheader_struct.biWidth;
  while (( biWidth & 3) != 0) 
	biWidth++;
  unsigned char *t_row = new unsigned char[biWidth];
  for (int y = bmpinfoheader_struct.biHeight - 1; y >= 0; y--) {
    
    if (!read_param(p_input,t_row,(unsigned short)biWidth)) {
      /*cant read fileheader*/
      delete [] t_row;
      return False;
    }

    for (int x = 0; x < bmpinfoheader_struct.biWidth; x++) {
      t_row[x] = t_palette[t_row[x]].peRed;
    }
    fwrite(t_row,1,biWidth,p_output);
  }
  delete [] t_row;
  return TRUE;
}



short Xvars::read_param(FILE *p_file,void *p_dest,unsigned short p_bytecount) {
  assert(p_file && p_dest);
  return fread(p_dest,1,p_bytecount,p_file);
}



Boolean Xvars::load_stretch_surface(LPDIRECTDRAWSURFACE p_surface,
                                    const CPoint &p_dest,UINT p_bitmapid) {
  // Does this fail for palettized stretched surfaces?
  return load_surface_no_palette(p_surface,p_dest,p_bitmapid,TRUE);
}



void
Xvars::read_palette_from_id( UINT p_palid, PALETTEENTRY ppe[PALETTENTRIES] ) {
  // Don't to graphicsEnabled check, since no DirectDraw calls in here.

  WORD i;
  RGBQUAD   pRgb;

  HRSRC hResInfo;
  HGLOBAL hResData;
  BYTE *pvRes;
	
  hResInfo = FindResource(AfxGetInstanceHandle(), MAKEINTRESOURCE(p_palid), "PALETTE"); 
  if (( hResInfo != NULL) &&
      ((hResData = LoadResource(AfxGetInstanceHandle(), hResInfo)) != NULL) &&
      ((pvRes = (BYTE *)LockResource(hResData)) != NULL)) {	
    pvRes = pvRes+24; //we're heeeere!

    for( i=0; i < 256; i++ ) {
      memcpy(&pRgb,pvRes+i*4,4);
      ppe[i].peRed = pRgb.rgbBlue;
      ppe[i].peGreen = pRgb.rgbGreen;
      ppe[i].peBlue = pRgb.rgbRed;
    }
	}
	else {
		assert(False);
  }
}



Area Xvars::un_stretch_area(const Area &area) {
  Pos p = area.get_pos();
  Size s = area.get_size();
  Size newSize;
  newSize.set(s.width/m_stretch,s.height/m_stretch);
  Area ret(AR_RECT,Pos(p.x/m_stretch,p.y/m_stretch),newSize);
  return ret;
}



Area Xvars::stretch_area(const Area &area) {
  Pos p = area.get_pos();
  Size s = area.get_size();
  Size newSize;
  newSize.set(s.width * m_stretch,s.height * m_stretch);
  Area ret(AR_RECT,Pos(p.x * m_stretch,p.y * m_stretch),newSize);
  return ret;
}



void Xvars::generate_mask_from_pixmap(LPDIRECTDRAWSURFACE maskSurf,const Pos& maskoffset,
                                      LPDIRECTDRAWSURFACE pixmapSurf,const Pos& pixmapoffset,
                                      const Size& size) {
  if (!graphicsEnabled) {
    return;
  }

  // Lock mask surface
  DDSURFACEDESC maskDesc;
  maskDesc.dwSize = sizeof(maskDesc);
  HRESULT hres;
  hres = maskSurf->Lock(NULL,&maskDesc,0,NULL);

  // Lock pixmap surface
  DDSURFACEDESC pixmapDesc;
  pixmapDesc.dwSize = sizeof(pixmapDesc);
  if (hres == DD_OK) {
    hres = pixmapSurf->Lock(NULL,&pixmapDesc,0,NULL);
    if (hres != DD_OK) {
      // Still want to unlock the mask
      maskSurf->Unlock(NULL);
      COUTRESULT("failed to lock pixmap");
    }
  }
  else {
    COUTRESULT("failed to lock mask");
  }
  DHRESULT(hres,return);


  // Start at top of rectangle.
  BYTE *maskBits = (BYTE*)maskDesc.lpSurface + 
                   maskDesc.lPitch * maskoffset.y + 
                   maskoffset.x * bytes_per_pixel();
  BYTE *pixmapBits = (BYTE*)pixmapDesc.lpSurface + 
                     pixmapDesc.lPitch * pixmapoffset.y +
                     pixmapoffset.x * bytes_per_pixel();

  for (int y = 0; y < size.height; y++) {
    // Start at beginning of this row.
    BYTE *maskP = maskBits;
    BYTE *pixmapP = pixmapBits;
    for (int x = 0; x < size.width; x++) {
      // Copy pink as pink.
      if (pixel_equals(pixmapP,(BYTE*)&XVARS_COLORKEY.dwColorSpaceLowValue,
                       bytes_per_pixel())) {
        pixel_set(maskP,(BYTE*)&XVARS_COLORKEY.dwColorSpaceLowValue,bytes_per_pixel());
      }
      // Copy all other real data as white.
      else {
        pixel_set(maskP,(BYTE*)&XVARS_COLORKEY_MASKS.dwColorSpaceLowValue,bytes_per_pixel());
      }
      maskP += bytes_per_pixel();
      pixmapP += bytes_per_pixel();
    }
    maskBits += maskDesc.lPitch;
    pixmapBits += pixmapDesc.lPitch;
  }
  
  maskSurf->Unlock(NULL);
  pixmapSurf->Unlock(NULL);
}



void Xvars::generate_pixmap_from_transform(LPDIRECTDRAWSURFACE dest,const Pos& destOffset,
                                           LPDIRECTDRAWSURFACE src,const Pos& srcOffset,
                                           const Size& srcSize,
                                           LPDIRECTDRAWSURFACE scratch,
                                           const TransformType* transforms,int tNum) {
  if (!graphicsEnabled) {
    return;
  }

  assert(tNum <= 2);

  switch(tNum) {
    // Just copy pixels src to dest.
    case 0:
      Blt(dest,destOffset,
          src,srcOffset,
          srcSize);
      break;

    // Transform from src to dest.
    case 1:
      gen_pix_from_trans(dest,destOffset,src,srcOffset,srcSize,transforms[0]);
      break;

    // Transform from src to scratch, then scratch to dest.
    case 2:
      gen_pix_from_trans(scratch,Pos(0,0),src,srcOffset,srcSize,transforms[0]);
      Size scratchSize = Transform2D::apply(transforms[0],srcSize);
      gen_pix_from_trans(dest,destOffset,scratch,Pos(0,0),scratchSize,transforms[1]);
      break;
    default:
      assert(0);
  }
}
                     


void Xvars::gen_pix_from_trans(LPDIRECTDRAWSURFACE dest,const Pos& destOffset,
                               LPDIRECTDRAWSURFACE src,const Pos& srcOffset,
                               const Size& srcSize,
                               TransformType transform) {
  if (!graphicsEnabled) {
    return;
  }

  HRESULT hres;

  // Lock source surface.
  DDSURFACEDESC srcDesc;
  srcDesc.dwSize = sizeof(srcDesc);
  hres = src->Lock(NULL,
                   &srcDesc,
                   DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT,NULL);
  DHRESULT(hres,return);

  // Lock dest surface if different than source, else reuse source pointer.
  BYTE *destRoot = NULL;
  LONG destlPitch = 0;
  if (src != dest) {
    DDSURFACEDESC destDesc;
    destDesc.dwSize = sizeof(destDesc);
    hres = dest->Lock(NULL,
                      &destDesc,
                      DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT,NULL);
    // Careful to free src if dest lock fails.
    DHRESULT(hres,src->Unlock(NULL); return);
    destRoot = (BYTE*)destDesc.lpSurface;
    destlPitch = destDesc.lPitch;
  }
  else {
    destRoot = (BYTE*)srcDesc.lpSurface;
    destlPitch = srcDesc.lPitch;
  }


  // Stretched coordinates.  We really should use a different type than "Pos".
  Pos srcPos; 
  for (srcPos.y = 0; srcPos.y < srcSize.height; srcPos.y++) {
    // Start at beginning of this row.
    BYTE* srcPix = (BYTE*)srcDesc.lpSurface + 
                   srcDesc.lPitch * (srcOffset.y + srcPos.y) + 
                   bytes_per_pixel() * srcOffset.x;
    
    for (srcPos.x = 0; srcPos.x < srcSize.width; srcPos.x++) {
      Pos destPos = Transform2D::apply(transform,srcPos,srcSize);

      BYTE* destPix = destRoot + 
                      destlPitch * (destOffset.y + destPos.y) +
                      bytes_per_pixel() * (destOffset.x + destPos.x);
      pixel_set(destPix,srcPix,bytes_per_pixel());

      srcPix += bytes_per_pixel();
    }
  }


  // Unlock surfaces.
  src->Unlock(NULL);
  if (src != dest) {
    dest->Unlock(NULL);
  }
}



BYTE Xvars::xpm_map_pixels(BYTE* srcPix,int srcPixNum,Boolean) {
  // Just sample upper-left corner.
  assert(srcPixNum > 0);
  return srcPix[0];
}



CFont* Xvars::select_arena_font(CDC *p_cdc) {
  if (m_arenaFont) {
    return p_cdc->SelectObject(m_arenaFont);
  }
  else {
    return (CFont*)p_cdc->SelectStockObject(ANSI_VAR_FONT);
  }
}



HRESULT Xvars::color_fill(LPDIRECTDRAWSURFACE surface,const CRect &rect,
                          DWORD color,Boolean fast) {
  if (!graphicsEnabled) {
    return DD_OK;
  }

  // Use DDBLT_COLORFILL
  if (fast) {
    DDBLTFX t_DDBltFx; 
    memset(&t_DDBltFx,0,sizeof(DDBLTFX));
    t_DDBltFx.dwSize = sizeof(DDBLTFX);
    t_DDBltFx.dwFillColor = color;
    HRESULT t_result = 
      m_lpbackbuffer->Blt((RECT*)(const RECT*)rect,NULL,NULL,DDBLT_COLORFILL,&t_DDBltFx);
    DHRESULT(t_result,;);
    return t_result;
  }
  // else do it ourselves because DDBLT_COLORFILL doesn't work on some machines.


  DDSURFACEDESC surfDesc;
  surfDesc.dwSize = sizeof(surfDesc);

  // Lock entire surface, could just lock rect.
  HRESULT t_result;
  t_result = surface->Lock(NULL,&surfDesc,
                             DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT|DDLOCK_WRITEONLY,
                             NULL);
  DHRESULT(t_result,return t_result);
  int bytesPerPixel = bytes_per_pixel();

  for (int y = rect.top; y < rect.bottom; y++) {
    int x = rect.left;
    BYTE *p = (BYTE*)surfDesc.lpSurface + y * surfDesc.lPitch + x * bytesPerPixel;
    for (; x < rect.right; x++, p += bytesPerPixel) {
      pixel_set(p,(BYTE*)&color,bytesPerPixel);
    }
  }

  surface->Unlock(NULL);
  return DD_OK;
}



HRESULT Xvars::Blt(LPDIRECTDRAWSURFACE dest,
                   DWORD destX,DWORD destY,
                   LPDIRECTDRAWSURFACE src, 
                   LPRECT srcRect,
                   Boolean requestSrcColorKey,
                   LPDIRECTDRAWCLIPPER clipper) {
  if (!graphicsEnabled) {
    return DD_OK;
  }

  Boolean doSrcColorKey = requestSrcColorKey;

// Renenable this code to turn off src key bltting in reduceDraw mode.
#if 0
  if (get_reduce_draw()) {
    doSrcColorKey = False;
  }
#endif

  // Some bounds checking.  
  // Only checking dest, and only checking for negative numbers.
  // DirectDraw seems to work ok for going too far off to the
  // right or bottom on the dest surfaces.  
  // (We should probably check this as well.)
  if (destX < 0) {
    srcRect->left -= destX;
    if (srcRect->left >= srcRect->right) {
      // Rectangle completely clipped.
      return DD_OK;
    }
    destX = 0;
  }

  if (destY < 0) {
    srcRect->top -= destY;
    if (srcRect->top >= srcRect->bottom) {
      // Rectangle completely clipped.
      return DD_OK;
    }
    destY = 0;
  }

  // Set clipper if supplied.
  if (clipper) {
    HRESULT result = dest->SetClipper(clipper);
    if (result != DD_OK) {
      return result;
    }
  }

  // Call IDirectDrawSurface::Blt, perhaps we should use BltFast instead.
  HRESULT t_result = dest->Blt(CRect(destX,destY,
                         destX + srcRect->right - srcRect->left,
                         destY + srcRect->bottom - srcRect->top),
                         src,srcRect,
                         doSrcColorKey ? DDBLT_KEYSRC : NULL,
                         NULL);
  if (t_result != DD_OK)
  {
    return t_result;
  }
  return t_result;
}



HRESULT Xvars::Blt(LPDIRECTDRAWSURFACE dest,
                   const Pos &destPos,
                   LPDIRECTDRAWSURFACE src,
                   const Pos &srcPos,
                   const Size &size,
                   Boolean srcColorKey,
                   LPDIRECTDRAWCLIPPER clipper) {
  return Blt(dest,destPos.x,destPos.y,src,
             CRect(srcPos.x,srcPos.y,srcPos.x + size.width,srcPos.y + size.height),
             srcColorKey,clipper);
}



Boolean Xvars::GetDC(LPDIRECTDRAWSURFACE surf, HDC *dc)
{
  if (!surf || !dc || !graphicsEnabled)
    return False;
  HRESULT t_result = surf->GetDC(dc);
  DHRESULT(t_result,return False);
  return True;
}



// This method is huge.  Cut it into smaller pieces, e.g. write_xpm_header, 
// write_xpm_pixels, etc.
Boolean Xvars::write_xpm_file(const char* filename,const char* varName,
                              LPDIRECTDRAWSURFACE surf,
                              const Area& area,const DDCOLORKEY* cKey,
                              Boolean halfSize) {
  if (!graphicsEnabled) {
    return False;
  }

  // Check that we are in 8-bit, indexed color mode.
  assert(uses_palette());

  // Only handle one transparent color.
  if (cKey) {
    assert(cKey->dwColorSpaceLowValue == cKey->dwColorSpaceHighValue);
  }

  Boolean ret = True;
  IDictIterator* iter = NULL;
  const int srcPixMax = 2 * 2;

  // Lock source DirectDraw surface.
  Pos offset = area.get_pos();
  Size size = area.get_size();
  CRect srcRect(offset.x,offset.y,offset.x + size.width,offset.y + size.height);
  DDSURFACEDESC sDesc;
  memset(&sDesc,0,sizeof(sDesc));
  sDesc.dwSize = sizeof(sDesc);
  HRESULT result = surf->Lock(NULL,&sDesc,0,NULL);
  DHRESULT(result,return False);

  // Hash from (color index in DirectDraw surface) -->
  //           (void* generated from XPMSymbols::newSymbols())
  IDictionary* hash = HashTable_factory();

  // First walk over the rectangle and get a list of all the colors used, put
  // a dummy entry in the hash table for each color.  Can't put a real
  // XPMSymbols symbol encoding  as the value yet, since we don't know whether 
  // we need 1 or 2 symbols per color yet.
  Pos srcPos;
  // If we have a color key, make sure the XPM file has a color entry for the
  // transparent color ("None"), even if the transparent color is never 
  // actually used in the pixmap, e.g. blood_4.  This is necessary on X11 to 
  // make sure the mask can be generated.
  if (cKey) {
    hash->put((void*)cKey->dwColorSpaceLowValue,XPMSymbols::DUMMY);
  }
  // Code duplication, should make some sort of iterator for walking over the 
  // pixels of a rectangle in a DirectDraw surface.
  for (srcPos.y = 0; srcPos.y < size.height; srcPos.y++) {
    // Start at beginning of this row.
    BYTE* srcPix = (BYTE*)sDesc.lpSurface + 
                   sDesc.lPitch * (offset.y + srcPos.y) + 
                   bytes_per_pixel() * offset.x;
    for (srcPos.x = 0; srcPos.x < size.width; srcPos.x++) {
      void* key = (void*)*srcPix;

      // Ok to put value over existing value, see IDictionary::put().
      // The hash table will take care of duplicates for us.
      hash->put(key,XPMSymbols::DUMMY);

      srcPix += bytes_per_pixel();  // Will always be 1.
    }
  }

  // Count number of distinct colors actually used.
  // This lets us know how many symbols needed per color in the XPM file.
  int numColors = hash->length();

  // Keeps track of all the symbols used in the XPM image we are creating.
  XPMSymbols symbols(numColors);
  

  // Open output text file.
  FILE* fp = fopen(filename,"w");
  if (fp == NULL) {
    ret = False;
    goto writeXpmCleanup;
  }

  // We are being real cheap here, only checking that fprintf works for the
  // first and last lines written.  Not too bad a hack since nothing bad 
  // really happens with the extra calls to fprintf. 

  // Write header info.
  if (fprintf(fp,"/* XPM */\n") < 0) {
    ret = False;
    goto writeXpmCleanup;
  }
  fprintf(fp,"static char *%s[] = {\n",varName);
  fprintf(fp,"  /* width height ncolors chars_per_pixel */\n");
  // Write half the size if halfSize is True.
  fprintf(fp,"  \"%d %d %d %d 0 0\",\n",
          halfSize ? (size.width >> 1) : size.width,
          halfSize ? (size.height >> 1) : size.height,
          numColors,symbols.get_symbols_num());
  // TODO: If halfSize is True, we might not be using all the colors, 
  // could optimize.


  // Write out all the colors.
  fprintf(fp,"  /* colors */\n");
  iter = hash->iterate();
  void* key;
  void* value;
  while (value = iter->next(key)) {
    // The index into the palette.
    BYTE index = (BYTE)key;

    // Create a unique set of symbols for the color if it doesn't already
    // have one.  A little funny to modify the hash table while we are 
    // iterating its entries, but we're safe because we are just modifying
    // an existing <key,value> association.
    if (value == XPMSymbols::DUMMY) {
      value = symbols.newSymbols();
      void* oldValue = hash->put(key,value);
      assert(oldValue == XPMSymbols::DUMMY);
    }

    // Get the symbols for the color.
    u_char syms[3]; 
    symbols.getSymbols(syms,value);

    // Write the line for one color in the XPM file.
    //
    // Transparent color.
    if (cKey && index == cKey->dwColorSpaceLowValue) {
      fprintf(fp,"  \"%s c None\",\n",syms);
    }
    // Normal color, write RGB triplet.
    else {      
      PALETTEENTRY* pEntry = &m_palette.palentries[index];
      fprintf(fp,"  \"%s c #%02x%02x%02x\",\n",syms,
              pEntry->peRed,pEntry->peGreen,pEntry->peBlue);
    }
  }
  delete iter;

  
  // If writing bitmap of half the size, jump by a 2x2 square at a time.
  Size delta;
  if (halfSize) {
    assert(size.width % 2 == 0 && size.height % 2 == 0);
    delta.set(2,2);
  }
  else {
    delta.set(1,1);
  }


  // Write out the pixel data.
  fprintf(fp,"  /* pixels */\n");
  // Code duplication, should make some sort of iterator for walking over the 
  // pixels of a rectangle in a DirectDraw surface.
  for (srcPos.y = 0; srcPos.y < size.height; srcPos.y += delta.height) {
    fprintf(fp,"  \"");

    // Start at beginning of this row.
    BYTE* srcPix = (BYTE*)sDesc.lpSurface + 
                   sDesc.lPitch * (offset.y + srcPos.y) + 
                   bytes_per_pixel() * offset.x;
    for (srcPos.x = 0; srcPos.x < size.width; srcPos.x += delta.width) {      
      // Gather all the pixels in a square of size delta into srcPixels.
      BYTE srcPixels[srcPixMax];
      int srcPixNum = 0;
      Size seekPos;
      for (seekPos.height = 0; seekPos.height < delta.height; seekPos.height++) {
        BYTE* seekPix = srcPix + sDesc.lPitch * seekPos.height;
        for (seekPos.width = 0; seekPos.width < delta.width; seekPos.width++) {
          srcPixels[srcPixNum] = *seekPix;
          srcPixNum++;
          seekPix += bytes_per_pixel();
        }
      }

      // Possibly reduce the pixels
      BYTE mappedPix = xpm_map_pixels(srcPixels,srcPixNum,halfSize);

      // Lookup the symbols for the destination pixel.
      void* value = hash->get((void*)mappedPix);
      assert(value);

      // Write out one pixel.
      u_char syms[3];
      symbols.getSymbols(syms,value);
      fprintf(fp,"%s",syms);

      srcPix += delta.width * bytes_per_pixel();
    }
    // Done with this row of pixels.
    fprintf(fp,"\",\n");
  }


  // Finish up writing XPM file.
  if (fprintf(fp,"};\n") < 0) {
    ret = False; 
    goto writeXpmCleanup;
  }
  fclose(fp);


  // Clean up.
  // Using goto as poor-man's exception handling.
writeXpmCleanup:
  delete hash;
  surf->Unlock(NULL);  
  return ret;
}

 

// if fails, increments valid and returns False
BOOL Xvars::hresultSuccess(HRESULT p_result) {
  interpretDHresult(p_result);
  if (DD_OK == p_result) {
    return TRUE;
  }
  if (DDERR_SURFACELOST == p_result) {
    valid++;
  }
  return False;
}



void Xvars::DHresultRecover(HRESULT p_result) {
  if (!graphicsEnabled) {
    COUTRESULT("Warning: DHRESULT called when graphics are disabled.");
  }

  if (p_result == DD_OK) {
    return;
  }

  // Print out the error.
  interpretDHresult(p_result);
  // Should have been set by now.
  assert(errorRecovery);


// Error recovery logic disabled.
#if 0
  // Attempt to recover, ask the errorRecovery object if we should disable 
  // graphics or force it to be enabled.
  graphicsEnabled = errorRecovery->attemptRecovery();
#endif
}



void Xvars::interpretDHresult(HRESULT p_result) {
  if (!DebugInfo::on()) {
    return;
  }

  CString t_string;
  switch (p_result)
  {
  case DD_OK:
	  return;
  case DDERR_ALREADYINITIALIZED:
	  t_string="DDERR_ALREADYINITIALIZED";
	  break;
  case DDERR_CANNOTDETACHSURFACE:
	  t_string="DDERR_CANNOTDETACHSURFACE";
	  break;
  case DDERR_CURRENTLYNOTAVAIL:
	  t_string="DDERR_CURRENTLYNOTAVAIL";
	  break;
  case DDERR_EXCEPTION:
	  t_string="DDERR_EXCEPTION";
	  break;
  case DDERR_GENERIC:
	  t_string="DDERR_GENERIC";
	  break;
  case DDERR_HEIGHTALIGN			:
	  t_string="DDERR_HEIGHTALIGN			";
	  break;
  case DDERR_INCOMPATIBLEPRIMARY		:
	  t_string="DDERR_INCOMPATIBLEPRIMARY		";
	  break;
  case DDERR_INVALIDCAPS			:
	  t_string="DDERR_INVALIDCAPS			";
	  break;
  case DDERR_INVALIDCLIPLIST			:
	  t_string="DDERR_INVALIDCLIPLIST			";
	  break;
  case DDERR_INVALIDMODE			:
	  t_string="DDERR_INVALIDMODE			";
	  break;
  case DDERR_INVALIDOBJECT			:
	  t_string="DDERR_INVALIDOBJECT			";
	  break;
  case DDERR_INVALIDPARAMS			:
	  t_string="DDERR_INVALIDPARAMS			";
	  break;
  case DDERR_INVALIDPIXELFORMAT		:
	  t_string="DDERR_INVALIDPIXELFORMAT		";
	  break;
  case DDERR_INVALIDRECT			:
	  t_string="DDERR_INVALIDRECT			";
	  break;
  case DDERR_LOCKEDSURFACES			:
	  t_string="DDERR_LOCKEDSURFACES			";
	  break;
  case DDERR_NO3D				:
	  t_string="DDERR_NO3D				";
	  break;
  case DDERR_NOALPHAHW				:
	  t_string="DDERR_NOALPHAHW				";
	  break;
  case DDERR_NOCLIPLIST			:
	  t_string="DDERR_NOCLIPLIST			";
	  break;
  case DDERR_NOCOLORCONVHW			:
	  t_string="DDERR_NOCOLORCONVHW			";
	  break;
  case DDERR_NOCOOPERATIVELEVELSET		:
	  t_string="DDERR_NOCOOPERATIVELEVELSET		";
	  break;
  case DDERR_NOCOLORKEY			:
	  t_string="DDERR_NOCOLORKEY			";
	  break;
  case DDERR_NOCOLORKEYHW			:
	  t_string="DDERR_NOCOLORKEYHW			";
	  break;
  case DDERR_NODIRECTDRAWSUPPORT		:
	  t_string="DDERR_NODIRECTDRAWSUPPORT		";
	  break;
  case DDERR_NOEXCLUSIVEMODE			:
	  t_string="DDERR_NOEXCLUSIVEMODE			";
	  break;
  case DDERR_NOFLIPHW				:
	  t_string="DDERR_NOFLIPHW				";
	  break;
  case DDERR_NOGDI				:
	  t_string="DDERR_NOGDI				";
	  break;
  case DDERR_NOMIRRORHW			:
	  t_string="DDERR_NOMIRRORHW			";
	  break;
  case DDERR_NOTFOUND				:
	  t_string="DDERR_NOTFOUND				";
	  break;
  case DDERR_NOOVERLAYHW			:
	  t_string="DDERR_NOOVERLAYHW			";
	  break;
  case DDERR_NORASTEROPHW			:
	  t_string="DDERR_NORASTEROPHW			";
	  break;
  case DDERR_NOROTATIONHW			:
	  t_string="DDERR_NOROTATIONHW			";
	  break;
  case DDERR_NOSTRETCHHW			:
	  t_string="DDERR_NOSTRETCHHW			";
	  break;
  case DDERR_NOT4BITCOLOR			:
	  t_string="DDERR_NOT4BITCOLOR			";
	  break;
  case DDERR_NOT4BITCOLORINDEX			:
	  t_string="DDERR_NOT4BITCOLORINDEX			";
	  break;
  case DDERR_NOT8BITCOLOR			:
	  t_string="DDERR_NOT8BITCOLOR			";
	  break;
  case DDERR_NOTEXTUREHW			:
	  t_string="DDERR_NOTEXTUREHW			";
	  break;
  case DDERR_NOVSYNCHW				:
	  t_string="DDERR_NOVSYNCHW				";
	  break;
  case DDERR_NOZBUFFERHW			:
	  t_string="DDERR_NOZBUFFERHW			";
	  break;
  case DDERR_NOZOVERLAYHW			:
	  t_string="DDERR_NOZOVERLAYHW			";
	  break;
  case DDERR_OUTOFCAPS				:
	  t_string="DDERR_OUTOFCAPS				";
	  break;
  case DDERR_OUTOFMEMORY			:
	  t_string="DDERR_OUTOFMEMORY			";
	  break;
  case DDERR_OUTOFVIDEOMEMORY			:
	  t_string="DDERR_OUTOFVIDEOMEMORY			";
	  break;
  case DDERR_OVERLAYCANTCLIP			:
	  t_string="DDERR_OVERLAYCANTCLIP			";
	  break;
  case DDERR_OVERLAYCOLORKEYONLYONEACTIVE	:
	  t_string="DDERR_OVERLAYCOLORKEYONLYONEACTIVE	";
	  break;
  case DDERR_PALETTEBUSY			:
	  t_string="DDERR_PALETTEBUSY			";
	  break;
  case DDERR_COLORKEYNOTSET			:
	  t_string="DDERR_COLORKEYNOTSET			";
	  break;
  case DDERR_SURFACEALREADYATTACHED		:
	  t_string="DDERR_SURFACEALREADYATTACHED		";
	  break;
  case DDERR_SURFACEALREADYDEPENDENT		:
	  t_string="DDERR_SURFACEALREADYDEPENDENT		";
	  break;
  case DDERR_SURFACEBUSY			:
	  t_string="DDERR_SURFACEBUSY			";
	  break;
  case DDERR_CANTLOCKSURFACE                   :
	  t_string="DDERR_CANTLOCKSURFACE                   ";
	  break;
  case DDERR_SURFACEISOBSCURED			:
	  t_string="DDERR_SURFACEISOBSCURED			";
	  break;
  case DDERR_SURFACELOST			:
	  t_string="DDERR_SURFACELOST			";
	  break;
  case DDERR_SURFACENOTATTACHED		:
	  t_string="DDERR_SURFACENOTATTACHED		";
	  break;
  case DDERR_TOOBIGHEIGHT			:
	  t_string="DDERR_TOOBIGHEIGHT			";
	  break;
  case DDERR_TOOBIGSIZE			:
	  t_string="DDERR_TOOBIGSIZE			";
	  break;
  case DDERR_TOOBIGWIDTH			:
	  t_string="DDERR_TOOBIGWIDTH			";
	  break;
  case DDERR_UNSUPPORTED			:
	  t_string="DDERR_UNSUPPORTED			";
	  break;
  case DDERR_UNSUPPORTEDFORMAT			:
	  t_string="DDERR_UNSUPPORTEDFORMAT			";
	  break;
  case DDERR_UNSUPPORTEDMASK			:
	  t_string="DDERR_UNSUPPORTEDMASK			";
	  break;
  case DDERR_VERTICALBLANKINPROGRESS		:
	  t_string="DDERR_VERTICALBLANKINPROGRESS		";
	  break;
  case DDERR_WASSTILLDRAWING			:
	  t_string="DDERR_WASSTILLDRAWING			";
	  break;
  case DDERR_XALIGN				:
	  t_string="DDERR_XALIGN				";
	  break;
  case DDERR_INVALIDDIRECTDRAWGUID		:
	  t_string="DDERR_INVALIDDIRECTDRAWGUID		";
	  break;
  case DDERR_DIRECTDRAWALREADYCREATED		:
	  t_string="DDERR_DIRECTDRAWALREADYCREATED		";
	  break;
  case DDERR_NODIRECTDRAWHW			:
	  t_string="DDERR_NODIRECTDRAWHW			";
	  break;
  case DDERR_PRIMARYSURFACEALREADYEXISTS	:
	  t_string="DDERR_PRIMARYSURFACEALREADYEXISTS	";
	  break;
  case DDERR_NOEMULATION			:
	  t_string="DDERR_NOEMULATION			";
	  break;
  case DDERR_REGIONTOOSMALL			:
	  t_string="DDERR_REGIONTOOSMALL			";
	  break;
  case DDERR_CLIPPERISUSINGHWND		:
	  t_string="DDERR_CLIPPERISUSINGHWND		";
	  break;
  case DDERR_NOCLIPPERATTACHED			:
	  t_string="DDERR_NOCLIPPERATTACHED			";
	  break;
  case DDERR_NOHWND				:
	  t_string="DDERR_NOHWND				";
	  break;
  case DDERR_HWNDSUBCLASSED			:
	  t_string="DDERR_HWNDSUBCLASSED			";
	  break;
  case DDERR_HWNDALREADYSET			:
	  t_string="DDERR_HWNDALREADYSET			";
	  break;
  case DDERR_NOPALETTEATTACHED			:
	  t_string="DDERR_NOPALETTEATTACHED			";
	  break;
  case DDERR_NOPALETTEHW			:
	  t_string="DDERR_NOPALETTEHW			";
	  break;
  case DDERR_BLTFASTCANTCLIP			:
	  t_string="DDERR_BLTFASTCANTCLIP			";
	  break;
  case DDERR_NOBLTHW				:
	  t_string="DDERR_NOBLTHW				";
	  break;
  case DDERR_NODDROPSHW			:
	  t_string="DDERR_NODDROPSHW			";
	  break;
  case DDERR_OVERLAYNOTVISIBLE			:
	  t_string="DDERR_OVERLAYNOTVISIBLE			";
	  break;
  case DDERR_NOOVERLAYDEST			:
	  t_string="DDERR_NOOVERLAYDEST			";
	  break;
  case DDERR_INVALIDPOSITION			:
	  t_string="DDERR_INVALIDPOSITION			";
	  break;
  case DDERR_NOTAOVERLAYSURFACE		:
	  t_string="DDERR_NOTAOVERLAYSURFACE		";
	  break;
  case DDERR_EXCLUSIVEMODEALREADYSET		:
	  t_string="DDERR_EXCLUSIVEMODEALREADYSET		";
	  break;
  case DDERR_NOTFLIPPABLE			:
	  t_string="DDERR_NOTFLIPPABLE			";
	  break;
  case DDERR_CANTDUPLICATE			:
	  t_string="DDERR_CANTDUPLICATE			";
	  break;
  case DDERR_NOTLOCKED				:
	  t_string="DDERR_NOTLOCKED				";
	  break;
  case DDERR_CANTCREATEDC			:
	  t_string="DDERR_CANTCREATEDC			";
	  break;
  case DDERR_NODC				:
	  t_string="DDERR_NODC				";
	  break;
  case DDERR_WRONGMODE				:
	  t_string="DDERR_WRONGMODE				";
	  break;
  case DDERR_IMPLICITLYCREATED			:
	  t_string="DDERR_IMPLICITLYCREATED			";
	  break;
  case DDERR_NOTPALETTIZED			:
	  t_string="DDERR_NOTPALETTIZED			";
	  break;
  case DDERR_UNSUPPORTEDMODE			:
	  t_string="DDERR_UNSUPPORTEDMODE			";
	  break;
  case DDERR_NOMIPMAPHW			:
	  t_string="DDERR_NOMIPMAPHW			";
	  break;
  case DDERR_INVALIDSURFACETYPE                :
	  t_string="DDERR_INVALIDSURFACETYPE                ";
	  break;
  case DDERR_DCALREADYCREATED			:
	  t_string="DDERR_DCALREADYCREATED			";
	  break;
  case DDERR_CANTPAGELOCK			:
	  t_string="DDERR_CANTPAGELOCK			";
	  break;
  case DDERR_CANTPAGEUNLOCK			:
	  t_string="DDERR_CANTPAGEUNLOCK			";
	  break;
  case DDERR_NOTPAGELOCKED			:
	  t_string="DDERR_NOTPAGELOCKED			";
	  break;
  }
  DebugInfo::print(t_string);
}



Boolean Xvars::reduceDraw = False;



IErrorRecovery* Xvars::errorRecovery = NULL;


Boolean Xvars::graphicsEnabled = False;
