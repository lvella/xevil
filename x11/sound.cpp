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

// "sound.cpp"  X11 sound engine implementation (XEvil 2.5).
// All miniaudio contact is confined to this translation unit.

#include "utils.h"
#include "coord.h"
#include "sound_cmn.h"
#include "sound.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/stat.h>

// miniaudio's declarations.  MINIAUDIO_IMPLEMENTATION lives in
// vendor/miniaudio.c (compiled separately as C), so this only pulls in the
// (C-linkage) API prototypes.
#include "vendor/miniaudio.h"


// Number of concurrent effect voices.  Comfortably above the "at least 10
// simultaneous" requirement so overlapping gunfire never starves.
#define SOUND_VOICES_MAX 24

#define SOUND_PATH_MAX 1024


// SoundName -> asset filename.  Indexed directly by SoundName value; NULL means
// "no file" (index 0 reserved; the three WAV soundtracks 1..3 were disabled in
// 2.02 and were never extracted).  MUST stay in sync with the SoundNames enum
// in sound_cmn.h -- a runtime assert below checks the count.
static const char *const SOUND_FILES[] = {
  NULL,               // 0  (reserved)
  NULL,               // 1  SOUNDTRACK          (disabled WAV soundtrack)
  NULL,               // 2  SOUNDTRACK_LEVELS   (disabled WAV soundtrack)
  NULL,               // 3  SOUNDTRACK_SEAL     (disabled WAV soundtrack)
  "chainsaw.wav",     // 4  CHAINSAW_SOUND
  "flamethrower.wav", // 5  FLAMETHROWER
  "death.wav",        // 6  DEATH
  "seal_death.wav",   // 7  SEAL_DEATH
  "hugger_death.wav", // 8  HUGGER_DEATH
  "frog_death.wav",   // 9  FROG_DEATH
  "breakdown.wav",    // 10 BREAKDOWN
  "bang.wav",         // 11 BANG
  "pistol.wav",       // 12 PISTOL
  "mgun.wav",         // 13 MGUN
  "launcher.wav",     // 14 LAUNCHER
  "explosion.wav",    // 15 EXPLOSION
  "dog_death.wav",    // 16 DOG_DEATH
  "laser.wav",        // 17 LASER
  "hero_attack.wav",  // 18 HERO_ATTACK
  "ninja_attack.wav", // 19 NINJA_ATTACK
  "dog_attack.wav",   // 20 DOG_ATTACK
  "chop_death.wav",   // 21 CHOP_DEATH
  "doppel_use.wav",   // 22 DOPPEL_USE
  "cloak_use.wav",    // 23 CLOAK_USE
  "trans_use.wav",    // 24 TRANS_USE
  "shield_use.wav",   // 25 SHIELD_USE
  "ninja_death.wav",  // 26 NINJA_DEATH
  "froggun.wav",      // 27 FROGGUN
  "lancer.wav",       // 28 LANCER
  "swapper.wav",      // 29 SWAPPER
  "fire.mp3",         // 30 FIRE_SOUNDTRACK
  "hive.mp3",         // 31 HIVE_SOUNDTRACK
  "kill.mp3",         // 32 KILL_SOUNDTRACK
  "seal.mp3",         // 33 SEAL_SOUNDTRACK
  "zeepeeg.mp3",      // 34 ZEEPEEG_SOUNDTRACK
  "nightsky.mp3",     // 35 NIGHTSKY_SOUNDTRACK
  "sweetdark.mp3",    // 36 SWEETDARK_SOUNDTRACK
  "terraexm.mp3",     // 37 TERRAEXM_SOUNDTRACK  (restored in 2.5)
  "newsong.mp3",      // 38 NEWSONG_SOUNDTRACK
  "woob.wav",         // 39 WOOB  (GravityWell collapse; orphaned since 1997)
};

#define SOUND_FILES_NUM ((int)(sizeof(SOUND_FILES) / sizeof(SOUND_FILES[0])))

// The streaming soundtracks available to the X11 random rotation.  This is the
// full set of MIDI-derived tracks, including terraexm (26 years overdue).
static const SoundName SOUND_TRACKS[] = {
  SoundNames::FIRE_SOUNDTRACK,
  SoundNames::HIVE_SOUNDTRACK,
  SoundNames::KILL_SOUNDTRACK,
  SoundNames::SEAL_SOUNDTRACK,
  SoundNames::ZEEPEEG_SOUNDTRACK,
  SoundNames::NIGHTSKY_SOUNDTRACK,
  SoundNames::SWEETDARK_SOUNDTRACK,
  SoundNames::TERRAEXM_SOUNDTRACK,
  SoundNames::NEWSONG_SOUNDTRACK,
};

#define SOUND_TRACKS_NUM ((int)(sizeof(SOUND_TRACKS) / sizeof(SOUND_TRACKS[0])))


#ifdef XEVIL_EMBEDDED_AUDIO
// ------------------------------------------------------------------------- //
// Embedded-assets path (the "single-file build").
//
// When built with -DXEVIL_EMBEDDED_AUDIO, every asset is a byte array compiled
// into the binary (see sdl/gen_audio.py and the generated
// embedded_audio_data.c) and reached through the manifest in embedded_audio.h.
// A tiny in-RAM miniaudio VFS below feeds those bytes to the engine, so the
// existing ma_sound_init_from_file(...,"chainsaw.wav",...) calls decode straight
// from memory -- no sounds/ directory, no DLLs, no side files.  When the macro
// is undefined (the X11 build) none of this compiles and the engine loads from
// the sounds/ directory exactly as before.
// ------------------------------------------------------------------------- //
#include "embedded_audio.h"

// Resolve an asset name (matched on its basename, so a resolver that prepends a
// directory still hits) to its compiled-in bytes.  Returns False if unknown.
static Boolean embedded_audio_lookup(const char *name,
                                     const unsigned char **data,
                                     unsigned int *len) {
  const char *base = name;
  for (const char *p = name; *p; p++) {
    if (*p == '/' || *p == '\\') {
      base = p + 1;
    }
  }
  for (int i = 0; i < xevil_embedded_audio_count; i++) {
    if (!strcmp(xevil_embedded_audio[i].name, base)) {
      *data = xevil_embedded_audio[i].data;
      *len = xevil_embedded_audio[i].len;
      return True;
    }
  }
  return False;
}

// One open in-memory asset: a read-only cursor over a compiled-in byte array.
struct EmbeddedFile {
  const unsigned char *data;
  size_t size;
  size_t cursor;
};

// XEVIL_SOUND_DEBUG, cached.  Lets the VFS announce (once-cached lookup) that a
// play request was actually served from the compiled-in bytes -- the runtime
// proof that audio lives inside the binary, not in any sounds/ directory.
static Boolean embedded_vfs_debug() {
  static int v = -1;
  if (v < 0) {
    const char *e = getenv("XEVIL_SOUND_DEBUG");
    v = (e && !strcmp(e,"1")) ? 1 : 0;
  }
  return v ? True : False;
}

static ma_result embedded_vfs_open(ma_vfs *pVFS,const char *pFilePath,
                                   ma_uint32 openMode,ma_vfs_file *pFile) {
  (void)pVFS;
  if (openMode & MA_OPEN_MODE_WRITE) {
    return MA_INVALID_OPERATION;    // the embedded store is read-only
  }
  const unsigned char *data;
  unsigned int len;
  if (!embedded_audio_lookup(pFilePath,&data,&len)) {
    return MA_DOES_NOT_EXIST;
  }
  if (embedded_vfs_debug()) {
    fprintf(stderr,"XEVIL-SOUND: VFS serving embedded '%s' (%u bytes) from binary\n",
            pFilePath,len);
  }
  EmbeddedFile *h = (EmbeddedFile *)malloc(sizeof(EmbeddedFile));
  if (!h) {
    return MA_OUT_OF_MEMORY;
  }
  h->data = data;
  h->size = (size_t)len;
  h->cursor = 0;
  *pFile = (ma_vfs_file)h;
  return MA_SUCCESS;
}

// Windows resource-manager paths may arrive wide; our asset names are pure
// ASCII, so narrow each code unit and reuse the open above.  (Never taken on
// the Linux build, but keeps the future mingw build working regardless of which
// path miniaudio chooses.)
static ma_result embedded_vfs_open_w(ma_vfs *pVFS,const wchar_t *pFilePath,
                                     ma_uint32 openMode,ma_vfs_file *pFile) {
  char narrow[SOUND_PATH_MAX];
  size_t i = 0;
  for (; pFilePath[i] && i < sizeof(narrow) - 1; i++) {
    narrow[i] = (char)pFilePath[i];
  }
  narrow[i] = '\0';
  return embedded_vfs_open(pVFS,narrow,openMode,pFile);
}

static ma_result embedded_vfs_close(ma_vfs *pVFS,ma_vfs_file file) {
  (void)pVFS;
  free((void *)file);
  return MA_SUCCESS;
}

static ma_result embedded_vfs_read(ma_vfs *pVFS,ma_vfs_file file,void *pDst,
                                   size_t sizeInBytes,size_t *pBytesRead) {
  (void)pVFS;
  EmbeddedFile *h = (EmbeddedFile *)file;
  size_t remaining = h->size - h->cursor;
  size_t toRead = (sizeInBytes < remaining) ? sizeInBytes : remaining;
  if (toRead > 0) {
    memcpy(pDst,h->data + h->cursor,toRead);
    h->cursor += toRead;
  }
  if (pBytesRead) {
    *pBytesRead = toRead;
  }
  // Mirror ma_default_vfs: end-of-file only when nothing at all could be read.
  if (toRead == 0 && sizeInBytes > 0) {
    return MA_AT_END;
  }
  return MA_SUCCESS;
}

static ma_result embedded_vfs_write(ma_vfs *pVFS,ma_vfs_file file,
                                    const void *pSrc,size_t sizeInBytes,
                                    size_t *pBytesWritten) {
  (void)pVFS;
  (void)file;
  (void)pSrc;
  (void)sizeInBytes;
  if (pBytesWritten) {
    *pBytesWritten = 0;
  }
  return MA_NOT_IMPLEMENTED;         // assets are read-only
}

static ma_result embedded_vfs_seek(ma_vfs *pVFS,ma_vfs_file file,
                                   ma_int64 offset,ma_seek_origin origin) {
  (void)pVFS;
  EmbeddedFile *h = (EmbeddedFile *)file;
  ma_int64 base;
  if (origin == ma_seek_origin_current) {
    base = (ma_int64)h->cursor;
  } else if (origin == ma_seek_origin_end) {
    base = (ma_int64)h->size;
  } else {
    base = 0;                        // ma_seek_origin_start
  }
  ma_int64 target = base + offset;
  if (target < 0) {
    return MA_BAD_SEEK;
  }
  if (target > (ma_int64)h->size) {
    target = (ma_int64)h->size;
  }
  h->cursor = (size_t)target;
  return MA_SUCCESS;
}

static ma_result embedded_vfs_tell(ma_vfs *pVFS,ma_vfs_file file,
                                   ma_int64 *pCursor) {
  (void)pVFS;
  EmbeddedFile *h = (EmbeddedFile *)file;
  *pCursor = (ma_int64)h->cursor;
  return MA_SUCCESS;
}

static ma_result embedded_vfs_info(ma_vfs *pVFS,ma_vfs_file file,
                                   ma_file_info *pInfo) {
  (void)pVFS;
  EmbeddedFile *h = (EmbeddedFile *)file;
  pInfo->sizeInBytes = (ma_uint64)h->size;
  return MA_SUCCESS;
}
#endif // XEVIL_EMBEDDED_AUDIO


// All miniaudio state, hidden from the shared header so its 4MB include never
// leaks into the rest of the build.
struct SoundImpl {
  ma_engine engine;
  ma_sound voices[SOUND_VOICES_MAX];
  Boolean voiceUsed[SOUND_VOICES_MAX];
  ma_sound music;
  Boolean musicActive;
  char soundDir[SOUND_PATH_MAX];
#ifdef XEVIL_EMBEDDED_AUDIO
  // VFS object handed to the engine's resource manager: a bare callbacks table
  // (miniaudio casts the ma_vfs* straight to ma_vfs_callbacks*).
  ma_vfs_callbacks embeddedVfs;
#endif

  SoundImpl() {
    musicActive = False;
    soundDir[0] = '\0';
    for (int i = 0; i < SOUND_VOICES_MAX; i++) {
      voiceUsed[i] = False;
    }
#ifdef XEVIL_EMBEDDED_AUDIO
    memset(&embeddedVfs,0,sizeof(embeddedVfs));
    embeddedVfs.onOpen  = embedded_vfs_open;
    embeddedVfs.onOpenW = embedded_vfs_open_w;
    embeddedVfs.onClose = embedded_vfs_close;
    embeddedVfs.onRead  = embedded_vfs_read;
    embeddedVfs.onWrite = embedded_vfs_write;
    embeddedVfs.onSeek  = embedded_vfs_seek;
    embeddedVfs.onTell  = embedded_vfs_tell;
    embeddedVfs.onInfo  = embedded_vfs_info;
#endif
  }
};



SoundManager::SoundManager(Boolean onoff,Locator *locator) {
  m_soundOn = onoff;
  m_disabled = False;
  m_triedInit = False;
  m_engineOk = False;
  m_haveAssets = False;
  m_warned = False;
  m_effectsVol = 70;   // -sound_volume default
  m_trackVol = 50;     // -music_volume default
  m_numKeyPositions = 0;
  m_locator = locator;
  m_currentTrack = 0;
  m_impl = NULL;

  m_debug = (getenv("XEVIL_SOUND_DEBUG") &&
             !strcmp(getenv("XEVIL_SOUND_DEBUG"),"1")) ? True : False;

  const char *noSound = getenv("XEVIL_NO_SOUND");
  if (noSound && !strcmp(noSound,"1")) {
    m_disabled = True;
  }
}



SoundManager::~SoundManager() {
  if (m_impl) {
    if (m_engineOk) {
      if (m_impl->musicActive) {
        ma_sound_uninit(&m_impl->music);
        m_impl->musicActive = False;
      }
      for (int i = 0; i < SOUND_VOICES_MAX; i++) {
        if (m_impl->voiceUsed[i]) {
          ma_sound_uninit(&m_impl->voices[i]);
          m_impl->voiceUsed[i] = False;
        }
      }
      ma_engine_uninit(&m_impl->engine);
    }
    delete m_impl;
    m_impl = NULL;
  }
}



void SoundManager::warn_once(const char *msg) {
  if (!m_warned) {
    m_warned = True;
    fprintf(stderr,"XEVIL sound: %s\n",msg);
  }
}



void SoundManager::disable() {
  // Hard, permanent disable (-no_sound / XEVIL_NO_SOUND).
  m_disabled = True;
  // A hard-disabled manager is definitively "not on"; keep isSoundOn() honest
  // so callers (e.g. the network sound-relay request) see the real state.
  m_soundOn = False;
  if (m_impl && m_engineOk) {
    if (m_impl->musicActive) {
      ma_sound_uninit(&m_impl->music);
      m_impl->musicActive = False;
    }
    ma_engine_uninit(&m_impl->engine);
    m_engineOk = False;
  }
}



void SoundManager::setEffectsVolume(int p_int) {
  if (p_int < 0) p_int = 0;
  if (p_int > 100) p_int = 100;
  m_effectsVol = p_int;
}



void SoundManager::setTrackVolume(int p_int) {
  if (p_int < 0) p_int = 0;
  if (p_int > 100) p_int = 100;
  m_trackVol = p_int;
  // Adjust any currently-playing track live.
  if (m_impl && m_engineOk && m_impl->musicActive) {
    ma_sound_set_volume(&m_impl->music,(float)m_trackVol / 100.0f);
  }
}



void SoundManager::ensure_init() {
  if (m_triedInit) {
    return;
  }
  m_triedInit = True;

  if (m_disabled) {
    return;
  }

  m_impl = new SoundImpl();
  if (!m_impl) {
    return;
  }

#ifdef XEVIL_EMBEDDED_AUDIO
  // Assets are compiled into the binary; route the engine's resource manager
  // through the in-RAM VFS so ma_sound_init_from_file() reads them by name.
  ma_engine_config engineCfg = ma_engine_config_init();
  engineCfg.pResourceManagerVFS = &m_impl->embeddedVfs;
  ma_result initRes = ma_engine_init(&engineCfg,&m_impl->engine);
#else
  ma_result initRes = ma_engine_init(NULL,&m_impl->engine);
#endif
  if (initRes != MA_SUCCESS) {
    // Headless / no audio device.  Leave debug logging on, but produce no
    // audio.  This is the path that runs in CI.
    delete m_impl;
    m_impl = NULL;
    m_engineOk = False;
    warn_once("no audio device available, running silently");
    return;
  }
  m_engineOk = True;

#ifdef XEVIL_EMBEDDED_AUDIO
  // Nothing to locate: the assets live inside the binary.
  m_impl->soundDir[0] = '\0';
  m_haveAssets = True;

  // One-time sweep: warn ONCE (not per file) if any expected asset is absent
  // from the compiled-in manifest.
  int missing = 0;
  for (int name = 1; name < SOUND_FILES_NUM; name++) {
    if (!SOUND_FILES[name]) {
      continue;
    }
    const unsigned char *d;
    unsigned int l;
    if (!embedded_audio_lookup(SOUND_FILES[name],&d,&l)) {
      missing++;
    }
  }
  if (missing > 0) {
    warn_once("some embedded sound assets are missing; those effects are disabled");
  }
  if (m_debug) {
    fprintf(stderr,"XEVIL-SOUND: %d embedded audio assets available\n",
            xevil_embedded_audio_count);
  }
#else
  // Locate the assets directory: first existing of the candidates.
  const char *envDir = getenv("XEVIL_SOUND_DIR");
  const char *candidates[4];
  candidates[0] = envDir;   // may be NULL
  candidates[1] = "./sounds";
  candidates[2] = "/usr/local/share/xevil/sounds";
  candidates[3] = "/usr/share/xevil/sounds";

  for (int c = 0; c < 4 && !m_haveAssets; c++) {
    if (!candidates[c]) {
      continue;
    }
    struct stat st;
    if (stat(candidates[c],&st) == 0 && S_ISDIR(st.st_mode)) {
      strncpy(m_impl->soundDir,candidates[c],SOUND_PATH_MAX - 1);
      m_impl->soundDir[SOUND_PATH_MAX - 1] = '\0';
      m_haveAssets = True;
    }
  }

  if (!m_haveAssets) {
    warn_once("no 'sounds' directory found (set XEVIL_SOUND_DIR); silent");
    return;
  }

  // One-time sweep: warn ONCE (not per file) if any expected asset is missing.
  int missing = 0;
  for (int name = 1; name < SOUND_FILES_NUM; name++) {
    if (!SOUND_FILES[name]) {
      continue;
    }
    char path[SOUND_PATH_MAX];
    snprintf(path,sizeof(path),"%s/%s",m_impl->soundDir,SOUND_FILES[name]);
    if (access(path,R_OK) != 0) {
      missing++;
    }
  }
  if (missing > 0) {
    warn_once("some sound files are missing; those effects are disabled");
  }
#endif
}



Boolean SoundManager::submitRequest(SoundRequest p_req) {
  if (m_disabled) {
    return False;
  }

  SoundName name = p_req.get_sound_name();
  if (!(name > 0 && name < SoundNames::SOUND_MAX)) {
    return False;
  }

  Pos pos = p_req.get_pos();

  // Respect the on/off toggle before anything else observable, so that turning
  // sound off (menu toggle or persisted config) genuinely silences the engine.
  if (!m_soundOn) {
    return False;
  }

  if (m_debug) {
    // Proves end-to-end flow even with no audio device (but not when sound is
    // off, whether via -no_sound or the on/off toggle).
    fprintf(stderr,"XEVIL-SOUND: %d at %d,%d\n",(int)name,pos.x,pos.y);
  }

  ensure_init();
  if (!m_engineOk || !m_haveAssets) {
    return False;
  }

  // Distance to the nearest listener, for channel priority (mirrors Win32).
  int distance = 0;
  if (m_numKeyPositions > 0) {
    distance = m_keyPositions[0].distance_2(pos);
    for (int i = 1; i < m_numKeyPositions; i++) {
      int d = m_keyPositions[i].distance_2(pos);
      if (d < distance) {
        distance = d;
      }
    }
  }

  // Claim a free slot, or evict the farthest-away queued sound.
  for (int i = 0; i < SOUND_CHANNELS_MAX; i++) {
    if (!m_soundEvents[i].m_init || distance < m_soundEvents[i].m_distance) {
      m_soundEvents[i].position = pos;
      m_soundEvents[i].soundid = (unsigned int)name;
      m_soundEvents[i].m_init = True;
      m_soundEvents[i].m_distance = distance;
      return True;
    }
  }
  return False;
}



void SoundManager::clearRegisteredSounds() {
  for (int i = 0; i < SOUND_CHANNELS_MAX; i++) {
    m_soundEvents[i].m_init = False;
    m_soundEvents[i].m_distance = 0;
    m_soundEvents[i].soundid = 0;
  }
}



SoundEvent SoundManager::getEvent(int p_index) {
  SoundEvent ret;
  if (p_index < 0 || p_index >= SOUND_CHANNELS_MAX) {
    return ret;
  }
  return m_soundEvents[p_index];
}



Boolean SoundManager::playSoundById(unsigned int p_soundid,int p_pan,
                                    int p_volume,Boolean p_loop) {
  if (m_disabled || !m_soundOn) {
    return False;
  }
  ensure_init();
  if (!m_engineOk || !m_haveAssets) {
    return False;
  }
  if (p_soundid <= 0 || (int)p_soundid >= SOUND_FILES_NUM ||
      !SOUND_FILES[p_soundid]) {
    return False;
  }

  char path[SOUND_PATH_MAX];
#ifdef XEVIL_EMBEDDED_AUDIO
  // Bare asset name; the in-RAM VFS resolves it to the compiled-in bytes.
  snprintf(path,sizeof(path),"%s",SOUND_FILES[p_soundid]);
#else
  snprintf(path,sizeof(path),"%s/%s",m_impl->soundDir,SOUND_FILES[p_soundid]);
#endif

  // Reclaim finished voices.
  for (int i = 0; i < SOUND_VOICES_MAX; i++) {
    if (m_impl->voiceUsed[i] && !ma_sound_is_playing(&m_impl->voices[i])) {
      ma_sound_uninit(&m_impl->voices[i]);
      m_impl->voiceUsed[i] = False;
    }
  }

  int slot = -1;
  for (int i = 0; i < SOUND_VOICES_MAX; i++) {
    if (!m_impl->voiceUsed[i]) {
      slot = i;
      break;
    }
  }
  if (slot < 0) {
    // All voices busy; drop this shot rather than cut off another.
    return False;
  }

  ma_uint32 flags = MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION;
  if (ma_sound_init_from_file(&m_impl->engine,path,flags,NULL,NULL,
                              &m_impl->voices[slot]) != MA_SUCCESS) {
    // Missing/undecodable file: silently skip (already warned once at init).
    return False;
  }
  m_impl->voiceUsed[slot] = True;

  // Win32 pan is -10000..10000 (left..right); miniaudio wants -1..1.
  float pan = (float)p_pan / 10000.0f;
  if (pan < -1.0f) pan = -1.0f;
  if (pan > 1.0f) pan = 1.0f;
  ma_sound_set_pan(&m_impl->voices[slot],pan);

  // Win32 volume is attenuation in hundredths of a dB (<= 0); convert to a
  // linear gain and scale by the effects-volume setting.
  float gain = powf(10.0f,(float)p_volume / 2000.0f);
  gain *= (float)m_effectsVol / 100.0f;
  if (gain < 0.0f) gain = 0.0f;
  ma_sound_set_volume(&m_impl->voices[slot],gain);

  ma_sound_set_looping(&m_impl->voices[slot],p_loop ? MA_TRUE : MA_FALSE);
  ma_sound_start(&m_impl->voices[slot]);
  return True;
}



SoundName SoundManager::pick_random_track() {
  return SOUND_TRACKS[Utils::choose(SOUND_TRACKS_NUM)];
}



void SoundManager::playMidi(SoundName p_name,Boolean p_loop,int /*p_delay*/) {
  if (m_disabled) {
    return;
  }

  SoundName resolved = p_name;
  if (resolved == SoundNames::SOUND_RANDOM) {
    resolved = pick_random_track();
  }

  const char *file = NULL;
  if (resolved > 0 && resolved < SOUND_FILES_NUM) {
    file = SOUND_FILES[resolved];
  }

  if (m_debug) {
    fprintf(stderr,"XEVIL-MUSIC: %d %s\n",(int)resolved,file ? file : "(none)");
  }

  m_currentTrack = resolved;

  if (!m_soundOn || !file) {
    return;
  }
  ensure_init();
  if (!m_engineOk || !m_haveAssets) {
    return;
  }

  // Stop any previous track.
  if (m_impl->musicActive) {
    ma_sound_uninit(&m_impl->music);
    m_impl->musicActive = False;
  }

  char path[SOUND_PATH_MAX];
#ifdef XEVIL_EMBEDDED_AUDIO
  // Bare asset name; the in-RAM VFS resolves it to the compiled-in bytes.
  snprintf(path,sizeof(path),"%s",file);
#else
  snprintf(path,sizeof(path),"%s/%s",m_impl->soundDir,file);
#endif

  ma_uint32 flags = MA_SOUND_FLAG_STREAM | MA_SOUND_FLAG_NO_SPATIALIZATION;
  if (ma_sound_init_from_file(&m_impl->engine,path,flags,NULL,NULL,
                              &m_impl->music) != MA_SUCCESS) {
    return;
  }
  m_impl->musicActive = True;
  ma_sound_set_looping(&m_impl->music,p_loop ? MA_TRUE : MA_FALSE);
  ma_sound_set_volume(&m_impl->music,(float)m_trackVol / 100.0f);
  ma_sound_start(&m_impl->music);
}



void SoundManager::stopMIDI() {
  if (m_impl && m_engineOk && m_impl->musicActive) {
    ma_sound_uninit(&m_impl->music);
    m_impl->musicActive = False;
  }
  m_currentTrack = 0;
}



void SoundManager::setKeyPosition(short p_index,Pos p_pos) {
  if (p_index >= 0 && p_index < SOUND_KEYPOS_MAX) {
    m_keyPositions[p_index] = p_pos;
  }
}



Pos SoundManager::getKeyPosition(short p_index) {
  if (p_index >= 0 && p_index < SOUND_KEYPOS_MAX) {
    return m_keyPositions[p_index];
  }
  Pos ret;
  return ret;
}



void SoundManager::setNumKeyPositions(short p_num) {
  if (p_num < 0) {
    p_num = 0;
  }
  if (p_num > SOUND_KEYPOS_MAX) {
    p_num = SOUND_KEYPOS_MAX;
  }
  m_numKeyPositions = p_num;
}
