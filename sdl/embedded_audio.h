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

// "embedded_audio.h"
// Manifest for the built-in sound assets (the "single-file build").
//
// The array of entries is DEFINED by the generated file
// (sdl/BUILD/embedded_audio_data.c, produced by sdl/gen_audio.py and never
// committed) and CONSUMED by the shared sound engine (x11/sound.cpp) when it is
// compiled with -DXEVIL_EMBEDDED_AUDIO.  This header is the single place the
// {name, ptr, len} layout lives, so the generator and the reader can never
// drift.  When XEVIL_EMBEDDED_AUDIO is NOT defined, nothing includes this file
// and the engine loads assets from the sounds/ directory exactly as before (the
// X11 build's behavior is unchanged).

#ifndef EMBEDDED_AUDIO_H
#define EMBEDDED_AUDIO_H

#ifdef __cplusplus
extern "C" {
#endif

// One embedded asset: its original filename (e.g. "chainsaw.wav"), a pointer to
// the raw file bytes compiled into the binary, and the true byte length.  NOTE:
// the byte array is initialised from a C string literal, so its sizeof is len+1
// (implicit trailing NUL); always use `len`, never sizeof, to read it.
struct XevilEmbeddedAudio {
  const char *name;
  const unsigned char *data;
  unsigned int len;
};

// Defined in the generated embedded_audio_data.c.
extern const struct XevilEmbeddedAudio xevil_embedded_audio[];
extern const int xevil_embedded_audio_count;

#ifdef __cplusplus
}
#endif

#endif // EMBEDDED_AUDIO_H
