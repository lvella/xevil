/*
 * XEvil 2.5 SDL port -- in-memory XPM3 parser.
 *
 * The game's art stays compiled-in: every *.bitmaps file #includes XPM C
 * arrays (char* string arrays).  This parses such an array into a flat
 * RGBA8888 buffer.  It replaces the entire X11 pixmap+clip-mask machinery:
 * an XPM 'c None' color becomes alpha 0, every other color alpha 255, so the
 * mask is just the alpha channel.
 *
 * Header line understood:  "W H ncolors cpp [hotx hoty]"  (hotspot optional).
 * Color entries:  the pixel key (cpp chars) then key/value pairs among
 *   c / m / g4 / g / s ; the 'c' (color) value is used.  Values:
 *     None            -> transparent (alpha 0)
 *     #RGB/#RRGGBB/#RRRRGGGGBBBB (any case) -> that RGB, alpha 255
 *     a named color   -> baked table (see xpm.cpp), alpha 255
 * Unknown named colors fail loudly.
 */

#ifndef SDL_XPM_H
#define SDL_XPM_H

/* RGBA8888 image, row-major, 4 bytes/pixel in memory order R,G,B,A. */
struct XpmImage {
  int width;
  int height;
  int hotx;               /* hotspot x (0 if the XPM omits it) */
  int hoty;               /* hotspot y                         */
  unsigned char *rgba;    /* width*height*4 bytes, malloc'd     */

  XpmImage() : width(0), height(0), hotx(0), hoty(0), rgba(0) {}
  ~XpmImage();
  void reset();

 private:
  XpmImage(const XpmImage &);
  XpmImage &operator=(const XpmImage &);
};

namespace xpm {

/* Parse an in-memory XPM3 char** array.  Returns true on success; on failure
   prints a diagnostic to stderr and leaves `out` empty. */
bool parse(const char *const *data, XpmImage &out);

/* Look up a named X11 color from the baked table (case-insensitive, spaces
   ignored).  Returns true and fills r,g,b when known. */
bool lookup_named_color(const char *name,
                        unsigned char &r, unsigned char &g, unsigned char &b);

}  // namespace xpm

#endif
