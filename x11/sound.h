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

// "sound.h"
// X11/UNIX sound engine.  Historically this header held only dummy no-ops and
// the UNIX build was silent; it now drives a real audio engine backed by
// miniaudio (see x11/vendor/miniaudio.h).  That dependency is confined to
// sound.cpp -- this header exposes the same public API shape as the old stub
// and the Win32 SoundManager so cmn/ code compiles unchanged.  The audio device
// is opened LAZILY on first use; if no device is available (e.g. a headless
// machine) the manager marks itself disabled forever and every method becomes a
// cheap no-op that never blocks or crashes.

#ifndef SOUND_H
#define SOUND_H

#include "utils.h"
#include "sound_cmn.h"

// Number of simultaneous positional effect events tracked per turn, mirroring
// the Win32 MAX_CHANNELS.  play_sounds() drains this each turn.
#define SOUND_CHANNELS_MAX 10

// Max listener "key positions" (one per local viewport).  >= UI_VIEWPORTS_MAX,
// but defined locally so this header never has to include ui.h.
#define SOUND_KEYPOS_MAX 8


// One queued positional effect.  Layout mirrors the Win32 SoundEvent so that
// Game::play_sounds() reads the same fields on both platforms.
struct SoundEvent {
  SoundEvent() {soundid = 0; m_init = False; m_distance = 0;}
  Pos position;      // World position of the sound (may outlive the emitter).
  unsigned int soundid;
  Boolean m_init;    // True if this slot holds a real event this turn.
  int m_distance;    // distance_2 to nearest listener, for slot priority.
};


class Locator;
struct SoundImpl;   // miniaudio state, defined only in sound.cpp (PIMPL).


class SoundManager {
public:
  SoundManager(Boolean onoff,Locator *locator);
  ~SoundManager();

  Boolean isSoundOn() {return m_soundOn;}
  void turnOnoff(Boolean p_bool) {m_soundOn = p_bool;}

  void setTrackVolume(int p_int);
  void setEffectsVolume(int p_int);
  int getTrackVolume() {return m_trackVol;}
  int getEffectsVolume() {return m_effectsVol;}

  void disable();
  /* EFFECTS: Permanently and completely disable sound (-no_sound /
     XEVIL_NO_SOUND).  Unlike turnOnoff(False) this suppresses even debug
     logging and can never be undone. */

  Boolean submitRequest(SoundRequest p_req);
  /* EFFECTS: Accept a positional effect request.  Logs it (if
     XEVIL_SOUND_DEBUG) and queues it for the next play_sounds() drain. */

  SoundEvent getEvent(int p_index);
  void clearRegisteredSounds();

  Boolean playSoundById(unsigned int p_soundid,int p_pan,int p_volume,
                        Boolean p_loop);
  /* EFFECTS: Play a one-shot effect by SoundName-valued id, with Win32-style
     pan (-10000..10000) and attenuation (hundredths of dB, <= 0). */

  void playMidi(SoundName p_name,Boolean p_loop,int p_delay);
  /* EFFECTS: Start a streaming soundtrack.  SOUND_RANDOM picks one at random. */

  void stopMIDI();

  void setKeyPosition(short p_index,Pos p_pos);
  Pos getKeyPosition(short p_index);
  void setNumKeyPositions(short p_num);
  short getNumKeyPositions() {return m_numKeyPositions;}

  // Retained (as no-ops) for API compatibility with the old stub / Win32.
  // These are only referenced from Win32-only code paths.
  Boolean removeSound(unsigned int) {return False;}
  Boolean playSound(unsigned int,int,int,Boolean,Boolean = False) {return False;}
  Boolean stopSound(unsigned int) {return False;}
  Boolean destroyAllSound() {return False;}

private:
  void ensure_init();
  /* EFFECTS: Lazily open the audio device and locate assets on first use.
     On failure marks the engine unavailable but leaves debug logging on. */

  void warn_once(const char *msg);
  SoundName pick_random_track();

  Boolean m_soundOn;        // User on/off toggle.
  Boolean m_disabled;       // Hard-disabled by -no_sound / XEVIL_NO_SOUND.
  Boolean m_debug;          // XEVIL_SOUND_DEBUG=1: log flow to stderr.
  Boolean m_triedInit;      // Have we attempted device init yet?
  Boolean m_engineOk;       // Did device init succeed?
  Boolean m_haveAssets;     // Did we find the sounds directory?
  Boolean m_warned;         // Have we already printed our one warning?
  int m_effectsVol;         // 0..100
  int m_trackVol;           // 0..100

  Pos m_keyPositions[SOUND_KEYPOS_MAX];
  short m_numKeyPositions;
  SoundEvent m_soundEvents[SOUND_CHANNELS_MAX];

  Locator *m_locator;
  SoundName m_currentTrack;
  SoundImpl *m_impl;        // miniaudio engine + voices, NULL until init.
};

typedef SoundManager *SoundManagerP;

#endif
