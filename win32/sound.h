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

#ifndef _SOUND_H
#define _SOUND_H
#include <afxtempl.h>
#include "mmsystem.h"
#include "dsound.h"
#include "coord.h"
#include "ui.h"

#include "fileman.h"

#define MAX_CHANNELS 10 //CANNOT BE 0
#define MAX_CHANNELS_OVERLAY 4 //CANNOT BE 0


struct SoundEvent
{
//	Id locatorid;
	Pos position; //because the item may have just died!
	UINT soundid;
	Boolean m_init;
	int m_distance;
};



class CMCIObject;
  



class SoundBuffer
{
	UINT m_soundid;
	IDirectSoundBuffer *m_array[MAX_CHANNELS_OVERLAY];
	BYTE m_index;
public:
	SoundBuffer(UINT p_soundid){m_index=0;m_soundid=p_soundid;}
	UINT getId(){return m_soundid;}
	void reset(){m_index=0;}
	IDirectSoundBuffer *getNext();
	void setAt(BYTE p_index,IDirectSoundBuffer *p_buf);
};



class Locator;



class SoundManager
{
  Boolean m_soundon;
  int m_effectsvol;
  int m_trackvol;
  Pos m_keypositions[UI_VIEWPORTS_MAX];
  short m_numkeypositions;
  LPDIRECTSOUND m_lpDS;
  GUID *m_lpGUID;
  CTypedPtrArray<CPtrArray,SoundBuffer *>m_soundbuffers;
  HWND m_windowowner;
  CWordArray m_available;
  Boolean m_init;
  SoundEvent m_soundevents[MAX_CHANNELS];
  SoundEvent m_loopingsounds[MAX_CHANNELS];
  int m_numloopingsounds;
  Locator *m_locator;
  MCI_OPEN_PARMS MCIOpen;                     // MCI_OPEN structure
  MCI_PLAY_PARMS MCIPlay;                     // MCI_PLAY structure
  MCI_SET_PARMS MCISet;                       // MCI_SET structure
  MCI_STATUS_PARMS MCIStatus;                 // MCI_STATUS structure

  Boolean m_cdinitialized;
  Boolean m_mediapresent;
  Boolean m_midiinitialized;
  int m_nTrackCount;
  int byTrackNumber;
  BOOL m_midiloop;
  CString m_midifilename;
  HMIDIOUT m_hmidiout;
  CString m_randomdirectory;
  int m_nummidifiles;
  BOOL m_pausedflag; //was the midi file paused??
  CTypedPtrArray<CPtrArray,CString *>m_midifilenames;
  
  CMCIObject *m_basicsound;
  TempFileManager *tempfilemanager;
  const char *tempmidifilename;

public:
  SoundManager(Boolean p_onoff,Locator *locator);
  ~SoundManager(){destroyAllSound();}

  Boolean isSoundOn(){return m_soundon;}
  void turnOnoff(Boolean p_bool){m_soundon=p_bool;} 
  void setTrackVolume(int p_int){m_trackvol=p_int;}
  void setEffectsVolume(int p_int){m_effectsvol=p_int;}
  int getTrackVolume(){return m_trackvol;}
  int getEffectsVolume(){return m_effectsvol;}

  Boolean init(HWND hwndOwner);//initializes SoundManager with the HWND as owner
  Boolean removeSound(UINT p_index);//removes sound at index p_index
  Boolean playSound(SoundName p_resourceid,int p_pan,int p_volume,Boolean p_loop,Boolean p_overridedup=False);
  /* Play sound using cross platform SoundName. */

  void playMidi(SoundName name, BOOL loop, int delay);
  /* Play midi using cross platform SoundName. */

  Boolean playSoundById(UINT p_resourceid,int p_pan,int p_volume,Boolean p_loop,Boolean p_overridedup=False);
  /* Play sound using platform-specific resource id. */

  Boolean stopSound(SoundName p_resourceid);
  Boolean destroyAllSound();

  Boolean submitRequest(SoundRequest p_req);
  SoundEvent getEvent(int p_index);
  void clearRegisteredSounds();
  void requestCDtrack(BYTE p_byTrackNumber,int p_delay);//plays CD track,delay=seconds
  int playCDtrack ();//restarts current tracknumber
  void stopCD();
  int getTrackCount();
  void requestMIDItrack(const CString &p_request,BOOL p_loop,int p_delay);
  void requestMIDItrack(UINT p_id,BOOL p_loop,int p_delay);//-1 == random
  void requestRandomMIDItrack(const CString &p_directory,BOOL p_loop,int p_delay);
  void playMIDItrack();
  void playMIDItrack(const CString & p_filename, BOOL p_loop);
  void stopMIDI();
  void pauseMIDI();
  void resumeMIDI();
  void setTempFileManager(TempFileManager *tfm){tempfilemanager = tfm;}

  Boolean init_cd(HWND p_wnd);
  Boolean media_check();
  Boolean init_midi(HWND p_wnd);

  void setKeyPosition(short p_index,Pos p_pos){m_keypositions[p_index]=p_pos;}
  Pos getKeyPosition(short p_index){return m_keypositions[p_index];}
  void setNumKeyPositions(short p_index){m_numkeypositions=p_index;}
  short getNumKeyPositions(){return m_numkeypositions;}
  
private://private methods
  Boolean removeSoundbyIndex(int p_index);
  int addSound(UINT p_resourceid,Boolean p_overridedup);//gets resource id and returns sound index used for later playing //override dup tells manager not to make MAX_CHANNELS_OVERLAY many DSBUFS
  Boolean DSParseWaveResource(void *pvRes,WAVEFORMATEX **ppWaveHeader,BYTE **ppbWaveData,DWORD *pcbWaveSize);
  IDirectSoundBuffer * DSLoadSoundBuffer(IDirectSound *pDS, LPCTSTR lpName);
  Boolean DSReloadSoundBuffer(IDirectSoundBuffer *pDSB, LPCTSTR lpName);
  Boolean DSGetWaveResource(HMODULE hModule, LPCTSTR lpName,
	    WAVEFORMATEX **ppWaveHeader, BYTE **ppbWaveData, DWORD *pcbWaveSize);//from an id get the wave data
  Boolean DSFillSoundBuffer(IDirectSoundBuffer *pDSB, BYTE *pbWaveData, DWORD cbWaveSize);//loads wave data into a direct sound buffer
  Boolean disable();//Releases the DirectSound device
  Boolean loadSound(int p_index,LPSTR p_str,Boolean p_overridedup);//loads m_soundbuffers[p_index] from m_soundstringarray[p_index]
  void loadRandomFilenames(const CString &p_directory);//used to load midi files from a firectory
  static int __stdcall dsEnumCallback(GUID * lpguid,LPSTR desc,LPSTR Module, LPVOID context);//call back, the context will be a CTypedPtrArrar of GUIDs *

  enum {
    MAX_MIDI_SOUNDTRACK = 7
  };
  static unsigned int MIDI_SOUNDTRACK_IDS[MAX_MIDI_SOUNDTRACK];
};

#ifdef _GRDEBUG
void interpretDSHresult(HRESULT p_result);
#endif
typedef SoundManager * SoundManagerP;

#endif //_SOUND_H
