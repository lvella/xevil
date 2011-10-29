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


#ifndef NO_PRAGMAS
#pragma implementation "xdata.h"
#endif


// Include Files
#include "utils.h"
#include "xdata.h"
#include "area.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sysexits.h>

#include <cstdlib>
#include <iostream>
#include <X11/Xutil.h>
#include <X11/xpm.h>
#include <X11/Xatom.h>

using namespace std;

// Means always allocate some color, even if it's really, really bad.
#define XPM_CLOSENESS 65535




Window Xvars::create_toplevel_window(int argc,char** argv,
                                     int dpyNum,const Size& size,
                                     const char* title,
                                     long eventMask) {
  // Creates with 0 border width.
  Window toplevel = 
    XCreateSimpleWindow(dpy[dpyNum],root[dpyNum],0,0,
                        size.width,size.height,
                        0,windowBorder[dpyNum],
                        windowBg[dpyNum]);

  XSizeHints size_hints;
  size_hints.flags = PPosition | PSize | PMinSize | PMaxSize;
  size_hints.min_width = size.width;
  size_hints.min_height = size.height;
  size_hints.max_width = size.width;
  size_hints.max_height = size.height;

  XTextProperty windowName, iconName;
  // Should be safe to cast away const, XStringListToTextProperty doesn't 
  // mutate the string list argument.
  char *window_name = (char*)title;    // Will appear on window.
  char *icon_name = (char*)title;
  Status stat = XStringListToTextProperty(&window_name,1,&windowName);
  assert(stat);
  stat = XStringListToTextProperty(&icon_name,1,&iconName);
  assert(stat);
  
  // Hints to window manager.
  XWMHints wm_hints;
  wm_hints.initial_state = NormalState;
  wm_hints.input = True;
  wm_hints.flags = StateHint | InputHint;
  
  // Only set IconPixmap and IconMask hints if the pixmaps loaded 
  // correctly.
  if (iconPixmap[dpyNum]) {
    wm_hints.icon_pixmap = iconPixmap[dpyNum];
    wm_hints.flags |= IconPixmapHint;
    if (iconMask[dpyNum]) {
      wm_hints.icon_mask = iconMask[dpyNum];
      wm_hints.flags |= IconMaskHint;
    }
  }

  XClassHint class_hints;
  class_hints.res_name = argv[0];
  class_hints.res_class = "XEvil";

  XSetWMProperties(dpy[dpyNum],toplevel,
                   &windowName,&iconName,argv,argc,
                   &size_hints,&wm_hints,&class_hints);

  XSelectInput(dpy[dpyNum],toplevel,eventMask);

  // Add WM_DELETE_WINDOW protocol
  XChangeProperty(dpy[dpyNum],toplevel,
                  wmProtocols[dpyNum],XA_ATOM,
                  32,PropModePrepend,
                  (unsigned char *)&wmDeleteWindow[dpyNum],
                  1);

  return toplevel;
}



Pixel Xvars::alloc_named_color(int dpyNum,const char *name,Pixel def) const {
  XColor actual,database;

  // Check for monochrome display.
  // Hack, not supposed to look at c_class member of visual.
  Status status = 1;
  if (((visual[dpyNum]->c_class == PseudoColor)  ||
       (visual[dpyNum]->c_class == StaticColor) ||
       (visual[dpyNum]->c_class == DirectColor) ||
       (visual[dpyNum]->c_class == TrueColor)) && 
      (status = 
       XAllocNamedColor(dpy[dpyNum],cmap[dpyNum],(char *)name,
                        &actual,&database))) {
    return actual.pixel;
  }
  else {
    if (!status) {
      cerr << "Warning:: unable to allocate color " << ((char *)name) 
           << "." << endl;
    }
    return (def == (Pixel)-1) ? white[dpyNum] : def;
  }
}



void Xvars::generate_pixmap_from_transform(int dpyNum,
                                           Drawable dest,Drawable src,
                                           const Size& srcSize,
                                           Drawable scratch,
                                           const TransformType* transforms,
                                           int tNum,
                                           int depth) {
  // Some cut and paste from the Windows version.
  // If we had more powerful graphics abstractions, we could make this
  // cross-platform code.  Not really worth doing it now.
  assert(tNum <= 2);
  Size scratchSize;

  switch(tNum) {
    // Just copy pixels src to dest.
    case 0:
      // Why doesn't this work???  Gives BadMatch.
#if 0
      XCopyArea(dpy[dpyNum],
                src,dest,gc[dpyNum],
                0,0,srcSize.width,srcSize.height,
                0,0);
#endif
      // Less efficient workaround.
      gen_pix_from_trans(dpyNum,dest,src,srcSize,
                         TR_NONE,depth);
      break;

    // Transform from src to dest.
    case 1:
      gen_pix_from_trans(dpyNum,dest,src,srcSize,
                         transforms[0],depth);
      break;

     // Transform from src to scratch, then scratch to dest.
    case 2: 
      gen_pix_from_trans(dpyNum,scratch,src,srcSize,transforms[0],depth);
      scratchSize = Transform2D::apply(transforms[0],srcSize);
      gen_pix_from_trans(dpyNum,dest,scratch,scratchSize,transforms[1],depth);
      break;

    default:
      assert(0);
  }
}



Boolean Xvars::load_pixmap(Drawable* pixmap,Drawable* mask,
                           int dpyNum,char** xpmBits) {
  return load_pixmap(pixmap,mask,dpyNum,xpmBits,is_stretched());
}



Boolean Xvars::load_pixmap(Drawable* pixmap,Drawable* mask,
                           int dpyNum,char** xpmBits,Boolean fullSize) {
  // Just load XPM as is.
  if (fullSize) {
    XpmAttributes attr;
    attr.valuemask = XpmCloseness;
    attr.closeness = XPM_CLOSENESS;
    attr.alloc_close_colors = True;

    int val = 
      XpmCreatePixmapFromData(dpy[dpyNum],root[dpyNum],
                              xpmBits,
                              pixmap,mask,
                              &attr);
    XpmFreeAttributes(&attr);

    Boolean ret = (val == XpmSuccess);
    if (!*pixmap || (mask && !*mask)) {
      ret = False;
    }

    return ret; 
  }


  //// Load image and mask into memory, reduce them and put them up to the
  //// display server as a new pixmap and mask

  XImage* srcImage;
  XImage* srcMask;
  Size srcSize;

  // Load in src image from supplied data.
  XpmAttributes attr;
  attr.valuemask = XpmReturnPixels | XpmCloseness;
  attr.closeness = XPM_CLOSENESS;
  attr.alloc_close_colors = True;

  // Perhaps should use XpmReturnAllocPixels, neither gives the transparent 
  // value.
  int val = 
    XpmCreateImageFromData(dpy[dpyNum],
                           xpmBits,
                           &srcImage,(mask ? &srcMask : (XImage**)NULL),
                           &attr);
  srcSize.width = attr.width;
  srcSize.height = attr.height;
  if (val != XpmSuccess) {
    XpmFreeAttributes(&attr);
    return False;
  }
  // Must be even size.
  assert((srcSize.width % 2 == 0) && 
         (srcSize.height % 2 == 0));
  int depth = srcImage->depth;
  int bitmap_pad = srcImage->bitmap_pad;

  // Size of reduced, destination image.
  Size destSize;
  destSize.set(srcSize.width / 2,srcSize.height / 2);
  
  // Create image for dest data.
  char* destData = 
    new_bytes_for_image(destSize,depth,bitmap_pad);
  XImage *destImage = 
    XCreateImage(dpy[dpyNum],visual[dpyNum],depth,ZPixmap,0,
                 destData,destSize.width,destSize.height,
                 bitmap_pad,0);
  assert(destImage);

  // Create dest mask if needed.
  XImage *destMask;
  if (mask) {
    assert(srcMask->depth == 1);
    char* destMaskData = 
      new_bytes_for_image(destSize,1,bitmap_pad);
    destMask = 
      XCreateImage(dpy[dpyNum],visual[dpyNum],1,ZPixmap,0,
                   destMaskData,destSize.width,destSize.height,
                   bitmap_pad,0);
    assert(destMask);
  }
    
  // Do the pixel reduction.
  //  unstretch_image(dpyNum,destImage,srcImage,attr.pixels,attr.npixels);
  unstretch_image(dpyNum,destImage,srcImage,
                  attr.pixels,attr.npixels);
  if (mask) {
    unstretch_image(dpyNum,destMask,srcMask,NULL,0);
  }

  // Kill src image and mask.
  XpmFreeAttributes(&attr);
  XDestroyImage(srcImage);  
  if (mask) {
    XDestroyImage(srcMask);
  }
      
  // Create dest pixmap and mask
  *pixmap = XCreatePixmap(dpy[dpyNum],root[dpyNum],
                          destSize.width,destSize.height,
                          depth);
  if (!*pixmap) {
    // Should do more cleanup.
    return False;
  }
  if (mask) {
    *mask = XCreatePixmap(dpy[dpyNum],root[dpyNum],
                          destSize.width,destSize.height,
                          1);
    if (!*mask) {
      // Should do more cleanup.
      return False;
    }
  }

  // Put dest image and mask up to the display server.
  put_image(dpyNum,*pixmap,destImage,destSize);
  if (mask) {
    put_image(dpyNum,*mask,destMask,destSize);
  }

  // Kill dest image and mask
  destroy_image(destImage);
  if (mask) {
    destroy_image(destMask);
  }
  return True;
}



Area Xvars::stretch_area(const Area& area) {
  Area ret(stretch_pos(area.get_pos()),
           stretch_size(area.get_size()));
  return ret;
}



void Xvars::gen_pix_from_trans(int dpyNum,Drawable dest,Drawable src,
                               const Size& srcSize,TransformType transform,
                               int depth) {
  // Get source image into client memory.
  XImage* srcImage = 
    XGetImage(dpy[dpyNum],src,0,0,srcSize.width,srcSize.height,
              AllPlanes,ZPixmap);
  if (!srcImage) {
    cerr << "Could not get image in Xvars::gen_pix_from_trans." << endl;
    return;
  }

  // Size of destination image.
  Size destSize = Transform2D::apply(transform,srcSize);

  // Allocate memory for the image.
  char* destData = new_bytes_for_image(destSize,depth,srcImage->bitmap_pad);

  // Create XImage for the transformed data.
  XImage* destImage =
    XCreateImage(dpy[dpyNum],visual[dpyNum],depth,ZPixmap,0,
                 destData,destSize.width,destSize.height,
                 srcImage->bitmap_pad,0);
  assert(destImage);
  assert(destData == destImage->data);
  

  // Traverse coordinates of src, compute corresponding position on 
  // dest and copy the pixel.
  Pos srcPos; 
  for (srcPos.y = 0; srcPos.y < srcSize.height; srcPos.y++) {
    for (srcPos.x = 0; srcPos.x < srcSize.width; srcPos.x++) {
      Pos destPos = Transform2D::apply(transform,srcPos,srcSize);
      unsigned long pix = XGetPixel(srcImage,srcPos.x,srcPos.y);
      XPutPixel(destImage,destPos.x,destPos.y,pix);
    }
  }

  put_image(dpyNum,dest,destImage,destSize);

  // Make sure to use new/delete for freeing the memory we allocated.
  destroy_image(destImage);

  XDestroyImage(srcImage);
}



static void Xvars_unstretch_subsample(XImage* dest,XImage* src) {
  // Real simple, sample every other pixel in ever other row.
  Pos destPos;
  for (destPos.y = 0; destPos.y < dest->height; destPos.y++) {
    for (destPos.x = 0; destPos.x < dest->width; destPos.x++) {
      unsigned long pix = XGetPixel(src,destPos.x * 2,destPos.y * 2);
      XPutPixel(dest,destPos.x,destPos.y,pix);
    }
  }
}



void Xvars::unstretch_image(int dpyNum,
                            XImage* dest,XImage* src,
                            Pixel* pixels,int pixelsNum) {
  assert(src->width == 2 * dest->width &&
         src->height == 2 * dest->height);

  // Don't use color info at all, just subsample the pixels.
  //
  // Do this if 1) useAveraging is turned off,
  //            2) there is no interesting pixel information, i.e. masks.
  if (!useAveraging || pixels == NULL) {
    Xvars_unstretch_subsample(dest,src);
    return;
  }

  // First get list of RGB values for the available pixels.
  // colors[] will be indexed parallel to pixels[].
  XColor* colors = new XColor[pixelsNum];
  assert(colors);
  int n;
  for (n = 0; n < pixelsNum; n++) {
    colors[n].pixel = pixels[n];
  }
  XQueryColors(dpy[dpyNum],cmap[dpyNum],colors,pixelsNum);

  // Create hashtable to map from pixels to RGB values.
  // Safe to put pointers to colors[], since that array is guaranteed to
  // live as long as the HashTable.
  IDictionary* pixel2RGB = HashTable_factory();
  for (n = 0; n < pixelsNum; n++) {
    pixel2RGB->put((void*)colors[n].pixel,(void*)&colors[n]);
  }

  // Real simple, sample every other pixel.
  // We could make it look much better with a better algorithm, e.g. 
  // average every block of four pixels.
  Pos destPos;
  for (destPos.y = 0; destPos.y < dest->height; destPos.y++) {
    for (destPos.x = 0; destPos.x < dest->width; destPos.x++) {
      // Use int not XColor, since members of XColor are short.
      int r = 0, g = 0, b = 0;

      // Search four pixels of src.
      Pos srcPos;
      for (srcPos.y = 2 * destPos.y; 
           srcPos.y < 2 * (destPos.y + 1); 
           srcPos.y++) {
        for (srcPos.x = 2 * destPos.x;
             srcPos.x < 2 * (destPos.x + 1);
             srcPos.x++) {
          unsigned long pix = XGetPixel(src,srcPos.x,srcPos.y);
          XColor* color = (XColor*)pixel2RGB->get((void*)pix);
          // The XImage, src, shouldn't have any pixels that aren't in the
          // list of pixels passed in.
          if (color) {
            r += color->red;
            g += color->green;
            b += color->blue;
          }
          else {
            if (pix != 0) {
              cerr << "Xvars::unstretch_image() found pixel " << pix 
                   << " that is not in the pixel list." << endl;
            }
          }
        } // x
      } // y

      // Divide by 4, since we are averaging 4 pixels in the source.
      XColor destColor;
      destColor.red = (r >> 2);
      destColor.green = (g >> 2);
      destColor.blue = (b >> 2);
      
      // Find the  color in colors closest to the RGB values in destColor.
      int destIndex = color_match(&destColor,colors,pixelsNum);
      Pixel destPixel = colors[destIndex].pixel;

      // Put the pixel in the dest image.
      XPutPixel(dest,destPos.x,destPos.y,destPixel);
    }
  }

  delete pixel2RGB;
  delete [] colors;
}



// This is a bloody bottleneck, simple linear search.  Need something better,
// like a BSP tree of the color space.
int Xvars::color_match(XColor* color,XColor* palette,int paletteNum) {
  // Means none found yet.
  int ret = -1;
  float retFit;
  for (int n = 0; n < paletteNum; n++) {
    float rDiff = palette[n].red - color->red;
    float gDiff = palette[n].green - color->green;
    float bDiff = palette[n].blue - color->blue;
    float fit = rDiff * rDiff + gDiff * gDiff + bDiff * bDiff;

    // Perfect fit, we're done.
    if (fit == 0.0f) {      
      return n;
    }

    // First checked, or best so far.
    if (ret == -1 || fit < retFit) {
      ret = n;
      retFit = fit;
    }
  }
  assert(ret != -1);
  return ret;
}



#if 0
// Round val up to the nearest multiple of unit.
static int Utils_int_ceil(int val,int unit) {
  if (val % unit == 0) {
    return val;
  }
  else {
    return val + (unit - val % unit);
  }
}
#endif



char* Xvars::new_bytes_for_image(const Size& size,int depth,int bitmap_pad) {
  // Calc bytes for one row.
  int rowBits = size.width * bitmap_pad;
  // Now, I originally thought I should have (size.width * depth) rounded up
  // to the nearest increment of bitmap_pad.  The X documentation sure sounds
  // that way.  But, crashes in 24 bit mode, e.g. depth==24, bitmap_pad==32.  
  // So, here I'm allocating bitmap_pad bits for every pixel.  May be 
  // wasteful, but won't crash, goddamn it.
  // (We could really afford to be more wasteful here, if we needed to.  The 
  //  XImages are always only temporary.  And, usually only allocated one or
  //  two at a time.)

  int rowBytes = rowBits / 8;

  // Bytes for all rows.
  int bytesNum = rowBytes * size.height;  

  // Allocate data for dest image.
  char* data = new char[bytesNum];
  assert(data);
  return data;
}



void Xvars::destroy_image(XImage* image) {
  delete [] image->data;
  image->data = NULL;
  XDestroyImage(image);
}



void Xvars::put_image(int dpyNum,Drawable dest,XImage* src,const Size& size) {
  // Create temp GC to set foreground and background in case we are
  // dealing with a mask.
  GC gc;
  XGCValues values;
  values.foreground = 1;
  values.background = 0;
  gc = XCreateGC(dpy[dpyNum],dest,
                 GCForeground | GCBackground,&values);  

  XPutImage(dpy[dpyNum],dest,gc,src,
            0,0,0,0,size.width,size.height);
  
  // Kill temp GC.
  XFreeGC(dpy[dpyNum],gc);  
}



const char *Xvars::humanColorNames[Xvars::HUMAN_COLORS_NUM] = {
  "blue",
  "brown",
  "black",
  "purple",
  "green4",
  "pink3",
};



Boolean Xvars::useAveraging = False;



Daemon::Daemon(const char* filename) {
  fname = Utils::strdup(filename);
  fd = 0;
}



Daemon::~Daemon() {
  delete fname;
  if (fd != 0) {
    ::close(fd);
  }
}



const char* Daemon::get_file_name() {
  return fname;
}



void Daemon::daemonize() {
  pid_t new_pid = fork();
  
  // Error
  if ( new_pid < 0 ) {
    cerr << "Could not fork background process." << endl;
  }

  // Child
  else if ( new_pid == 0 ) {
    close( 0 );  // stdin
    // stdout and stderr closed below

    setsid();
    
    chdir("/tmp");
  }

  // Parent
  else {
    cout << "Started [pid " << new_pid << ']' << endl;
    exit( EX_OK );
  }
}



void Daemon::go() {
  int fDesc;
  fDesc = ::open(fname,
                 O_WRONLY | O_CREAT | O_APPEND,
                 S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
  if (fDesc == -1) {
    cerr << "Could not open log file " << fname << endl;
    return;
  }
  cout << "Logging all output to " << fname << endl;

  daemonize();

  
  // Redirect stdout and stderr.
  int val1 = dup2(fDesc,1);
  int val2 = dup2(fDesc,2);
  if (val1 == -1 || val2 == -1) {
    cerr << "Unable to redirect output to log file " << fname << endl;
    return;
  }  

  fd = fDesc;
}
