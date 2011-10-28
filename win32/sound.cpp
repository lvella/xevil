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
#include "sound_cmn.h"
#include "sound.h"
#include "locator.h"
#include "physical.h"

#define SOUND_TRACK_VOLUME_DEF 0
#define SOUND_EFFECT_VOLUME_DEF 0

#ifdef _GRDEBUG
#define DSRESULT(hresult,action) if (hresult!=DS_OK) \
	{ interpretDSHresult(hresult);\
		action;}
#else
#define DSRESULT(hresult,action) if (hresult!=DS_OK) action;
#endif


// Only those that can be chosen randomly.
unsigned int SoundManager::MIDI_SOUNDTRACK_IDS[MAX_MIDI_SOUNDTRACK]={
    SoundNames::FIRE_SOUNDTRACK,
    SoundNames::HIVE_SOUNDTRACK,
    SoundNames::KILL_SOUNDTRACK,
    SoundNames::ZEEPEEG_SOUNDTRACK,
    SoundNames::NIGHTSKY_SOUNDTRACK,
    SoundNames::SWEETDARK_SOUNDTRACK,
//    SoundNames::TERRAEXM_SOUNDTRACK,
    SoundNames::NEWSONG_SOUNDTRACK,
 };



class CMCIObject 
{
public:
    CMCIObject(HWND p_nofity);     
    ~CMCIObject();
    BOOL OpenFile(const char* pszFileName);
    void Close();
    void Play();
    void Stop();
    void setEffectsVolume(DWORD p_vol);
    DWORD GetPosition();
    DWORD GetLength();

// Implementation
protected:
    
private:
    void MCIError(DWORD dwErr);
    HWND m_HwndNotify;
    MCI_OPEN_PARMS m_OpenParams; 
};
  


BOOL
SoundManager::disable( void )
{
    if (m_lpDS == NULL)
    {
        return TRUE;
    }

    m_lpDS->Release();
    m_lpDS = NULL;

    return TRUE;

} /* DSDisable */



void 
SoundManager::loadRandomFilenames(const CString &p_directory)
{
    CString t_string=p_directory;
    t_string+="\\*.mid";
    WIN32_FIND_DATA c_file;
    /* find first .c file in current directory */
    HANDLE t_handle;
    for (int i=0;i<m_nummidifiles;i++)//delete all old files
        delete m_midifilenames[i];
    m_nummidifiles=0;
    if (t_handle=FindFirstFile( t_string.GetBuffer(0), &c_file) )
    {
        CString *t_newfile;
        if (c_file.cFileName)
        {
           t_newfile=new CString(p_directory + "\\");
           *t_newfile+= c_file.cFileName;
        }
        m_midifilenames.SetAtGrow(m_nummidifiles++,t_newfile);
        /* Find the rest of the .c files */
        while( FindNextFile( t_handle,&c_file )  )
        {
           if (c_file.cFileName)
           {
	           t_newfile=new CString(p_directory + "\\");
               *t_newfile+= c_file.cFileName;
           }
           m_midifilenames.SetAtGrow(m_nummidifiles++,t_newfile);
        }
    }
}


  /*
 * InitSound
 *
 * Sets up the DirectSound object and loads all sounds into secondary
 * DirectSound buffers.  Returns FALSE on error, or TRUE if successful
 */
BOOL
SoundManager::init( HWND hwndOwner )
{
    DSBUFFERDESC dsBD;
    IDirectSoundBuffer *lpPrimary;
	HRESULT t_result(NULL);
	CTypedPtrArray<CPtrArray,GUID *>t_guids;
	if (DirectSoundEnumerate((LPDSENUMCALLBACK)&dsEnumCallback,&t_guids))
		return FALSE;
	//WE MIGHT WANT TO ASK THEM WHICH SOUND CARD TO USE
	if (!t_guids.GetSize())
		return FALSE;
	t_result=DirectSoundCreate(t_guids[0],&m_lpDS,NULL);//for now we use 0 = primary device
	DSRESULT(t_result,return FALSE);
	t_result=m_lpDS->SetCooperativeLevel(hwndOwner,DSSCL_EXCLUSIVE);
	DSRESULT(t_result,return FALSE);

    /*
     * get the primary buffer and start it playing
     *
     * by playing the primary buffer, DirectSound knows to keep the
     * mixer active, even though we are not making any noise.
     */

    ZeroMemory( &dsBD, sizeof(DSBUFFERDESC) );
    dsBD.dwSize = sizeof(dsBD);
    dsBD.dwFlags = DSBCAPS_PRIMARYBUFFER;

    if (SUCCEEDED(t_result=m_lpDS->CreateSoundBuffer(&dsBD, &lpPrimary, NULL)))
    {
        t_result=lpPrimary->Play(0, 0, DSBPLAY_LOOPING);//play silence
		DSRESULT(t_result,;);
        lpPrimary->Release();
    }
    else
    {
		DSRESULT(t_result,return FALSE);
    }
	m_init=TRUE;
    m_windowowner=hwndOwner;
    m_basicsound=NULL;
//  m_midiloop=FALSE;  Was in xevil20t. hardts
    return TRUE;

} /* InitSound */


int //callback used for enumeration of sound devices.
SoundManager::dsEnumCallback(GUID * lpguid,LPSTR desc,LPSTR Module, LPVOID context)//call back, the context will be a CTypedPtrArrar of GUIDs *
{
	CTypedPtrArray<CPtrArray,GUID *> *t_guids=(CTypedPtrArray<CPtrArray,GUID *> *)context;	
	assert(t_guids);
	t_guids->Add((GUID *)lpguid);
	return TRUE;
}
  
  
  
/*
 * DestroySound
 *
 * Undoes everything that was done in a InitSound call
 */
BOOL 
SoundManager::destroyAllSound( void )
{
    int       idxKill;
    
    
	for( idxKill = m_soundbuffers.GetUpperBound(); idxKill >=0 ; idxKill-- )
        removeSoundbyIndex( idxKill );
	m_available.RemoveAll();
    disable(); //WHY THIS DONT WORK I DONT NO
    for (int i=0;i<m_nummidifiles;i++)
        delete m_midifilenames[i];
    m_nummidifiles=0;

    return TRUE;

} /* DestroySound */

/*
 * removeSound
 *
 * Frees up resources associated with a sound effect
 */
BOOL 
SoundManager::removeSound( UINT p_resourceid)
{
	if (!m_init)
		return FALSE;
	for (int i=m_soundbuffers.GetUpperBound();i>=0;i--)
	{
		if (m_soundbuffers[i]->getId()==p_resourceid)
			return removeSoundbyIndex(i);
    }
    return FALSE;

} /* SoundDestryEffect */

BOOL
SoundManager::removeSoundbyIndex(int p_index)
{
	if (!m_init)
		return FALSE;
	if ((p_index<0)||(p_index>m_soundbuffers.GetUpperBound()))
		return FALSE;
	if (!m_soundbuffers[p_index])
		return TRUE;
	IDirectSoundBuffer *t_buf=NULL;
	for (int i=0;i<MAX_CHANNELS_OVERLAY;i++)
	{
		if (t_buf!=(t_buf= m_soundbuffers[p_index]->getNext()))
		{
			t_buf->Stop();
			t_buf->Release();
		}
	}
	m_available.Add(p_index);
	delete m_soundbuffers[p_index];
	m_soundbuffers[p_index]=0;
	return TRUE;
}


int 
SoundManager::addSound( UINT p_resourceid,BOOL p_overridedup)
{
	IDirectSoundBuffer *t_soundbuffer=NULL;
	if (!m_init)
		return -1;
	if (!MAKEINTRESOURCE(p_resourceid))
	  return -1;

	int t_index;
	if (m_available.GetSize())
	{  
		t_index=m_available[m_available.GetUpperBound()];
		m_available.RemoveAt(m_available.GetUpperBound(),1);
	}
	else
	  t_index=m_soundbuffers.GetSize();
	SoundBuffer *t_sb=new SoundBuffer(p_resourceid);
	for (int i=0;i<MAX_CHANNELS_OVERLAY;i++)
		t_sb->setAt(i,NULL);
	m_soundbuffers.SetAtGrow(t_index,t_sb);
	if (!loadSound(t_index,MAKEINTRESOURCE(p_resourceid),p_overridedup))
		return -1;
	return t_index;
} /* SoundLoadEffect */

BOOL
SoundManager::loadSound(int p_index,LPSTR p_str,BOOL p_overridedup)
{
  if ((p_index<0)||(p_index>m_soundbuffers.GetUpperBound()))
  {
	assert(FALSE);
	return FALSE;
  }
  if (p_overridedup)
  {
	  m_soundbuffers[p_index]->reset();
	  m_soundbuffers[p_index]->setAt(0,DSLoadSoundBuffer(m_lpDS, p_str));
	  for (int i=1;i<MAX_CHANNELS_OVERLAY;i++)
		  m_soundbuffers[p_index]->setAt(i,m_soundbuffers[p_index]->getNext());
	  return (BOOL)m_soundbuffers[p_index]->getNext();
  }
  for (int i=0;i<MAX_CHANNELS_OVERLAY;i++)
	  m_soundbuffers[p_index]->setAt(i,DSLoadSoundBuffer(m_lpDS, p_str));
  return (BOOL)m_soundbuffers[p_index]->getNext();
}



// Lookup cross-platform sound name.
BOOL
SoundManager::playSound(SoundName name,int p_pan,int p_volume,
                        BOOL p_soundtrack,BOOL p_overridedup) {
  UINT soundid = SoundNames::lookup(name);
  if (soundid) {
      return playSoundById(soundid,p_pan,p_volume,p_soundtrack,p_overridedup);
  }
  return FALSE;
}



// Lookup cross-platform sound name.
void
SoundManager::playMidi(SoundName name, BOOL loop, int delay) {
  if (name == SoundNames::SOUND_RANDOM) {
    name = MIDI_SOUNDTRACK_IDS[Utils::choose(MAX_MIDI_SOUNDTRACK)];
  }
  UINT soundid = SoundNames::lookup(name);
  if (soundid) {
    requestMIDItrack(soundid,loop, delay);
  }
  return;
}



/*
 * SoundPlayEffect
 *
 * Plays the sound effect specified.  
 * Returns TRUE if succeeded.
 */
BOOL 
SoundManager::playSoundById( UINT p_resourceid ,int p_pan,int p_volume,BOOL p_soundtrack,BOOL p_overridedup)//p_soundtrack, loops the sound
{
	if ((!m_soundon)||(p_volume<= -10000))//volume of -10000= no sound
		return FALSE;
    if (p_soundtrack&&(m_trackvol<= -10000))
        return FALSE;
    if (!p_soundtrack&&(m_effectsvol<= -10000))
        return FALSE;
	if ((!m_init)||(!p_resourceid))
		return FALSE;
	HRESULT     dsrval;
	IDirectSoundBuffer *pdsb = NULL;
	for (int i=0;i<m_soundbuffers.GetSize();i++)
	{
		if (m_soundbuffers[i]->getId()==p_resourceid)
		{
			pdsb=m_soundbuffers[i]->getNext();
			break;
		}
	}
	if (!pdsb)
	{
		int t_index=addSound(p_resourceid,p_overridedup);
		assert(t_index>=0);
		pdsb=m_soundbuffers[t_index]->getNext();;
	}
	if( !m_lpDS || !pdsb )
	{
		return FALSE;
	}
	
	/*
	* Rewind the play cursor to the start of the effect, and play
	*/
	pdsb->SetCurrentPosition(0);
	pdsb->SetPan(p_pan);
	if (p_soundtrack)
		pdsb->SetVolume(p_volume+m_trackvol);
	else
		pdsb->SetVolume(p_volume+m_effectsvol);
	dsrval = pdsb->Play( 0, 0, p_soundtrack);//p_soundtrack, loops the sound

	if (dsrval == DSERR_BUFFERLOST)
	{

		dsrval = pdsb->Restore();

		if (dsrval == DS_OK)
		{
			if (DSReloadSoundBuffer(pdsb, MAKEINTRESOURCE(p_resourceid)))
			{
				pdsb->SetCurrentPosition( 0);
				dsrval = pdsb->Play(0, 0, p_soundtrack); //p_soundtrack, loops the sound
			}
			else
			{
				dsrval = E_FAIL;
			}
		}
	}
	DSRESULT(dsrval,return FALSE);
	return (dsrval == DS_OK);

} /* playSound */

	/*
	* stopSound
	*
	* Stops the sound effect specified.
	* Returns TRUE if succeeded.
*/
BOOL 
SoundManager::stopSound(SoundName p_name) {
  if (!m_init) {
		return FALSE;
  }

  // Lookup the SoundName to get resource.
  UINT resourceid = SoundNames::lookup(p_name);
  if (!resourceid) {
    return FALSE;
  }

  HRESULT     dsrval;
	int t_index;
 	for (int i=0;i<m_soundbuffers.GetSize();i++) {
		if (m_soundbuffers[i]->getId() == resourceid) {
      t_index = i;
      break;
		}
	}
	if (i==m_soundbuffers.GetSize()) {//failed ok just return false
		return FALSE;
  }

  if( !m_lpDS || !m_soundbuffers[t_index] ) {
      return FALSE;
  }

  for (i=0;i<MAX_CHANNELS_OVERLAY;i++) {
		dsrval = m_soundbuffers[t_index]->getNext()->Stop();
		DSRESULT(dsrval,return FALSE);
	}

  return TRUE;
} /* SoundStopEffect */


#ifdef _GRDEBUG
void interpretDSHresult(HRESULT p_result)
{
  CString t_string;
  switch (p_result)
  {
  case DS_OK: t_string="DS_OK" ;
  case DSERR_ALLOCATED: t_string= "DSERR_ALLOCATED";
  case DSERR_CONTROLUNAVAIL: t_string= "DSERR_CONTROLUNAVAIL";
  case DSERR_INVALIDPARAM: t_string= "DSERR_INVALIDPARAM";
  case DSERR_INVALIDCALL: t_string= "DSERR_INVALIDCALL";
  case DSERR_GENERIC: t_string= "DSERR_GENERIC";
  case DSERR_PRIOLEVELNEEDED: t_string= "DSERR_PRIOLEVELNEEDED";
  case DSERR_OUTOFMEMORY: t_string= "DSERR_OUTOFMEMORY";
  case DSERR_BADFORMAT: t_string= "DSERR_BADFORMAT";
  case DSERR_UNSUPPORTED: t_string= "DSERR_UNSUPPORTED";
  case DSERR_NODRIVER: t_string= "DSERR_NODRIVER";
  case DSERR_ALREADYINITIALIZED: t_string= "DSERR_ALREADYINITIALIZED";
  case DSERR_NOAGGREGATION: t_string= "DSERR_NOAGGREGATION";
  case DSERR_BUFFERLOST: t_string= "DSERR_BUFFERLOST";
  case DSERR_OTHERAPPHASPRIO: t_string= "DSERR_OTHERAPPHASPRIO";
  case DSERR_UNINITIALIZED: t_string= "DSERR_UNINITIALIZED";
  default: 
	t_string="UNKNOWN DSERR";
	break;
  }
  COUTRESULT(t_string);
}
#endif //_GRDEBUG




BOOL
SoundManager::submitRequest(SoundRequest p_req)
{
//search all viewports for Pos
  unsigned int soundid = SoundNames::lookup(p_req.get_sound_name());
  if (!soundid) {
    return False;
  }

	Pos t_pos = p_req.get_pos();
	int t_distance=m_keypositions[0].distance_2(t_pos);
	for (int i=1;i<m_numkeypositions;i++)
	{
		t_distance=min(t_distance,m_keypositions[i].distance_2(t_pos));
	}
	for (i=0;i<MAX_CHANNELS;i++)
	{
		if ((!m_soundevents[i].m_init)||
			(t_distance<m_soundevents[i].m_distance))
		{
      m_soundevents[i].position=t_pos;
      m_soundevents[i].soundid = soundid;      
//      m_soundevents[i].locatorid=p_req.get_id();
			m_soundevents[i].m_init=TRUE;
			m_soundevents[i].m_distance=t_distance;
			return TRUE;
		}
	}
	return FALSE;
}


void
SoundManager::clearRegisteredSounds()
{
	memset(m_soundevents,0,sizeof(SoundEvent)*MAX_CHANNELS);
} //set all m_inits to false

SoundEvent
SoundManager::getEvent(int p_index)
{
	SoundEvent t_event;
	memset(&t_event,0,sizeof(SoundEvent));
	if ((p_index<0)||(p_index>MAX_CHANNELS_OVERLAY))
		return t_event;
	return m_soundevents[p_index];
}


SoundManager::SoundManager(BOOL p_onoff,LocatorP locator)
{
	m_numloopingsounds=0;
	m_soundon=p_onoff;
	m_lpGUID=NULL;
	m_lpDS=NULL;
	m_init=FALSE;
	clearRegisteredSounds();
	m_numkeypositions=1;
	m_locator=locator;
	m_effectsvol=SOUND_EFFECT_VOLUME_DEF;
	m_trackvol=SOUND_TRACK_VOLUME_DEF;
    m_cdinitialized=FALSE;
    m_basicsound=NULL;
    m_midiinitialized=FALSE;
    m_mediapresent=FALSE;
    m_randomdirectory="";
    m_nummidifiles=0;
    m_pausedflag=FALSE;
    tempfilemanager=NULL;
    tempmidifilename=NULL;
}


IDirectSoundBuffer *
SoundBuffer::getNext()
{
	BYTE t_index=m_index%MAX_CHANNELS_OVERLAY;
	m_index++;
	return m_array[t_index];
}

void
SoundBuffer::setAt(BYTE p_index,IDirectSoundBuffer *p_buff)
{
	if ((p_index<0)||(p_index>MAX_CHANNELS_OVERLAY))
	{
		assert(FALSE);
		return;
	}
	m_array[p_index]=p_buff;
}



int
SoundManager::getTrackCount()
{
    if (!m_cdinitialized)
        if (!init_cd(m_windowowner))
            return 0;
    if (!m_mediapresent)
        if (!media_check())
            return 0;
    return m_nTrackCount;
}



Boolean
SoundManager::media_check()
{
    m_mediapresent=FALSE;
    m_nTrackCount=0;
    //
    // Determine whether there is a CD in the drive.
    //
    MCIStatus.dwItem = MCI_STATUS_MEDIA_PRESENT;
    mciSendCommand (MCIOpen.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM |
        MCI_WAIT, (DWORD) (LPVOID) &MCIStatus);
    if (MCIStatus.dwReturn) 
    {
        MCIStatus.dwItem = MCI_STATUS_NUMBER_OF_TRACKS;
        if (mciSendCommand (MCIOpen.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM |
            MCI_WAIT, (DWORD) (LPVOID) &MCIStatus))
            return FALSE;
        m_nTrackCount = min ((int) MCIStatus.dwReturn, 99);
    }
    if (m_nTrackCount>0)
        m_mediapresent=TRUE;
    return m_mediapresent;
}


Boolean
SoundManager::init_cd(HWND p_wnd)
{
    MCIOpen.lpstrDeviceType = (LPCSTR) MCI_DEVTYPE_CD_AUDIO;
    if (mciSendCommand (NULL, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID,
        (DWORD) (LPVOID) &MCIOpen))

        if (mciSendCommand (NULL, MCI_OPEN, MCI_OPEN_TYPE |
            MCI_OPEN_TYPE_ID | MCI_OPEN_SHAREABLE,
            (DWORD) (LPVOID) &MCIOpen)) {

            MessageBeep (MB_ICONEXCLAMATION);
            return FALSE;
        }

    MCISet.dwTimeFormat = MCI_FORMAT_TMSF;
    if (mciSendCommand (MCIOpen.wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT,
        (DWORD) (LPVOID) &MCISet))
        return FALSE;
    m_cdinitialized=TRUE;
    return m_cdinitialized;
}



void
SoundManager::requestCDtrack(BYTE p_byTrackNumber,int p_delay)
{
    ::KillTimer(m_windowowner,Ui::SOUNDTRACKTIMER_CD);
    byTrackNumber=p_byTrackNumber;
    ::SetTimer(m_windowowner,Ui::SOUNDTRACKTIMER_CD,p_delay*1000,NULL);
}



int //time in miliseconds 
SoundManager::playCDtrack ()
{
    ::KillTimer(m_windowowner,Ui::SOUNDTRACKTIMER_CD);
    if (!m_cdinitialized)
        if (!init_cd(m_windowowner))
            return -1;
    if (!m_mediapresent)
        if (!media_check())
            return -1;
    if (byTrackNumber<=0)
        return -1;
    //
    // Send an MCI_PLAY command to the CD Audio driver.
    //
    MCIPlay.dwCallback = (DWORD) m_windowowner;
    MCIPlay.dwFrom =MCI_MAKE_TMSF (byTrackNumber, 0, 0, 0);
    MCIPlay.dwTo= MCI_MAKE_TMSF (byTrackNumber+1, 0, 0, 0);
    if (MMSYSERR_NOERROR!=mciSendCommand (MCIOpen.wDeviceID, MCI_PLAY,  MCI_FROM|MCI_TO,
        (DWORD) (LPVOID) &MCIPlay))
    {
        m_mediapresent=FALSE;
        return -1;
    }
    MCIStatus.dwItem = MCI_STATUS_LENGTH;
    MCIStatus.dwTrack = (DWORD) byTrackNumber;
    if (MMSYSERR_NOERROR!=mciSendCommand (MCIOpen.wDeviceID, MCI_STATUS,
                    MCI_TRACK | MCI_STATUS_ITEM | 
                    MCI_WAIT, 
                    (DWORD) (LPVOID) &MCIStatus))
    {
        m_mediapresent=FALSE;
        return -1;
    }

    int t_milis=1000*(
      ((int) MCI_MSF_MINUTE (MCIStatus.dwReturn) * 60) +
       (int) MCI_MSF_SECOND (MCIStatus.dwReturn));
    ::SetTimer(m_windowowner,Ui::SOUNDTRACKTIMER_CD,t_milis,NULL);
    return t_milis;
}



void
SoundManager::stopCD()
{
    if (!m_cdinitialized)
        return;
    ::KillTimer(m_windowowner,Ui::SOUNDTRACKTIMER_CD); // added hardts
    if (MMSYSERR_NOERROR!=mciSendCommand (MCIOpen.wDeviceID, MCI_STOP, NULL, NULL))
        m_mediapresent=FALSE;
}



Boolean
SoundManager::init_midi(HWND p_wnd)
{
#if 0 //dont need to open the out, the open file will do that!
  MMRESULT t_result=midiOutOpen(&m_hmidiout,0, (DWORD)p_wnd,NULL, CALLBACK_WINDOW);
    if (MMSYSERR_NOERROR==t_result)
    {
        m_midiinitialized=TRUE;
        return TRUE;
    }
    return FALSE;
#endif
  return TRUE;
}



void
SoundManager::requestRandomMIDItrack(const CString &p_directory,BOOL p_loop,int p_delay)
{
    if (!m_midiinitialized)
    {
        assert(FALSE);
        return;
    }
    if (p_directory!= m_randomdirectory)
        loadRandomFilenames(p_directory);
    if (!m_nummidifiles)
        return;
    int t_choose=Utils::choose(m_nummidifiles);

    m_midifilename=*m_midifilenames[t_choose];
    if (!m_midifilename.GetLength())
    {
        assert(FALSE);
        return;
    }
    m_midiloop=p_loop;
    playMIDItrack();
}



void
SoundManager::requestMIDItrack(const CString &p_filename,BOOL p_loop,int p_delay)
{
  stopMIDI();  
  m_midifilename=p_filename;
  if (!m_midifilename.GetLength()){
      assert(FALSE);
      return;
  }
  m_midiloop=p_loop;
  playMIDItrack();
}



/*
whole basis of this function is to play a midi track that is located in our resource file.
HACK: we will make a "temporary file" to play the midi track since I am too stupid to find out how to
play a midi track from a resource id. they are small and we will clean up when we leave.
*/
void
SoundManager::requestMIDItrack(UINT p_trackresourceid,BOOL p_loop,int p_delay)
{
    if (!p_trackresourceid || !tempfilemanager)
        return;
    HRSRC hResInfo;
    HGLOBAL hResData;
    void *pvRes;
    if (!tempmidifilename)
        tempmidifilename = tempfilemanager->newTempFileName("mid");


    LPSTR t_ptr = MAKEINTRESOURCE(p_trackresourceid); //make int into string for find resource

    hResInfo=FindResource(NULL, t_ptr, "MIDI"); //null module to default to main app
    if (( hResInfo != NULL) &&
        ((hResData = LoadResource(NULL, hResInfo)) != NULL) &&
        ((pvRes = LockResource(hResData)) != NULL) )
    {
        DWORD t_size = SizeofResource(NULL,hResInfo);
        try {
            CFile t_file(tempmidifilename,CFile::modeCreate|CFile::modeWrite|CFile::typeBinary);
            t_file.Write(pvRes, t_size);
            t_file.Close();
        }
        catch (CFileException *)
        {
            assert(0);
            return;
        }

    }
    else
        return;
    
    CString t_string(tempmidifilename);
    requestMIDItrack(t_string,p_loop,p_delay);
}



void
SoundManager::playMIDItrack()
{
    if (!m_midifilename.GetLength())
    {
        assert(FALSE);
        return;
    }
    stopMIDI();
    playMIDItrack(m_midifilename,m_midiloop);
}



void
SoundManager::playMIDItrack(const CString &p_filename,BOOL p_loop)//plays imediately
{
    if (!m_midiinitialized || !p_filename.GetLength())
        if (!init_midi(m_windowowner))
            return;
    try
    {
        stopMIDI();
        m_basicsound=new CMCIObject(m_windowowner);
        m_basicsound->OpenFile(CString(p_filename));
        m_basicsound->Play();
        m_basicsound->setEffectsVolume((m_trackvol/-10000)*0xFFFF);
    }
    catch ( CFileException e)
    {
    }
}



void
SoundManager::pauseMIDI()
{
  if (m_basicsound)
  {
    stopMIDI();
    m_pausedflag=TRUE;
  }
}



void 
SoundManager::resumeMIDI()
{
  if (m_pausedflag)
  {
    playMIDItrack();
    m_pausedflag=FALSE;
  }
}



void
SoundManager::stopMIDI()
{
  if (m_basicsound){

    m_basicsound->Stop();
    delete m_basicsound;
    m_basicsound=NULL;
   }
}



////////////////////////////////
/////TECHIE SOUND CRAP

///////////////////////////////////////////////////////////////////////////////
//
// DSLoadSoundBuffer    Loads an IDirectSoundBuffer from a Win32 resource in
//                      the current application.
//
// Params:
//  pDS         -- Pointer to an IDirectSound that will be used to create
//                 the buffer.
//
//  lpName      -- Name of WAV resource to load the data from.  Can be a
//                 resource id specified using the MAKEINTRESOURCE macro.
//
// Returns an IDirectSoundBuffer containing the wave data or NULL on error.
//
// example:
//  in the application's resource script (.RC file)
//      Turtle WAV turtle.wav
//
//  some code in the application:
//      IDirectSoundBuffer *pDSB = DSLoadSoundBuffer(pDS, "Turtle");
//
//      if (pDSB)
//      {
//          IDirectSoundBuffer_Play(pDSB, 0, 0, DSBPLAY_TOEND);
//          /* ... */
//
///////////////////////////////////////////////////////////////////////////////
IDirectSoundBuffer *
SoundManager::DSLoadSoundBuffer(IDirectSound *pDS, LPCTSTR lpName)
{
    IDirectSoundBuffer *pDSB = NULL;
    DSBUFFERDESC dsBD = {0};
    BYTE *pbWaveData;
	HRESULT t_result;
    if (DSGetWaveResource(AfxGetInstanceHandle(), lpName, &dsBD.lpwfxFormat, &pbWaveData, &dsBD.dwBufferBytes))
    {
        dsBD.dwSize = sizeof(dsBD);
        dsBD.dwFlags = DSBCAPS_STATIC | DSBCAPS_CTRLDEFAULT;

        if (SUCCEEDED(t_result=pDS->CreateSoundBuffer(&dsBD, &pDSB, NULL)))
        {
            if (!DSFillSoundBuffer(pDSB, pbWaveData, dsBD.dwBufferBytes))
            {
                pDSB->Release();
                pDSB = NULL;
            }
        }
        else
        {
			DSRESULT(t_result,return NULL);
        }
    }

    return pDSB;
}
///////////////////////////////////////////////////////////////////////////////
//
// DSReloadSoundBuffer
//
///////////////////////////////////////////////////////////////////////////////

BOOL 
SoundManager::DSReloadSoundBuffer(IDirectSoundBuffer *pDSB, LPCTSTR lpName)
{
    BOOL result=FALSE;
    BYTE *pbWaveData;
    DWORD cbWaveSize;
	HRESULT t_result(NULL);
    if (DSGetWaveResource(AfxGetInstanceHandle(), lpName, NULL, &pbWaveData, &cbWaveSize))
    {
        if (SUCCEEDED(t_result=pDSB->Restore()) &&
            DSFillSoundBuffer(pDSB, pbWaveData, cbWaveSize))
        {
            result = TRUE;
        }
		else
		{
			DSRESULT(t_result,return FALSE);
		}
    }

    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//
// DSGetWaveResource
//
///////////////////////////////////////////////////////////////////////////////

BOOL 
SoundManager::DSGetWaveResource(HMODULE hModule, LPCTSTR lpName,
    WAVEFORMATEX **ppWaveHeader, BYTE **ppbWaveData, DWORD *pcbWaveSize)
{
    HRSRC hResInfo;
    HGLOBAL hResData;
    void *pvRes;
	hResInfo=FindResource(hModule, lpName, "WAVE"); 
    if (( hResInfo != NULL) &&
        ((hResData = LoadResource(hModule, hResInfo)) != NULL) &&
        ((pvRes = LockResource(hResData)) != NULL) &&
        DSParseWaveResource(pvRes, ppWaveHeader, ppbWaveData, pcbWaveSize))
    {
        return TRUE;
    }

    return FALSE;
}


BOOL 
SoundManager::DSFillSoundBuffer(IDirectSoundBuffer *pDSB, BYTE *pbWaveData, DWORD cbWaveSize)
{
    if (pDSB && pbWaveData && cbWaveSize)
    {
        LPVOID pMem1, pMem2;
        DWORD dwSize1, dwSize2;
		HRESULT t_result;
        if (SUCCEEDED(t_result=pDSB->Lock( 0, cbWaveSize,
            &pMem1, &dwSize1, &pMem2, &dwSize2, 0)))
        {
            CopyMemory(pMem1, pbWaveData, dwSize1);

            if ( 0 != dwSize2 )
                CopyMemory(pMem2, pbWaveData+dwSize1, dwSize2);

            pDSB->Unlock( pMem1, dwSize1, pMem2, dwSize2);
            return TRUE;
        }
		else
		{
			DSRESULT(t_result,return FALSE);
		}
    }

    return FALSE;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

BOOL 
SoundManager::DSParseWaveResource(void *pvRes, WAVEFORMATEX **ppWaveHeader, BYTE **ppbWaveData,DWORD *pcbWaveSize)
{
    DWORD *pdw;
    DWORD *pdwEnd;
    DWORD dwRiff;
    DWORD dwType;
    DWORD dwLength;

    if (ppWaveHeader)
        *ppWaveHeader = NULL;

    if (ppbWaveData)
        *ppbWaveData = NULL;

    if (pcbWaveSize)
        *pcbWaveSize = 0;

    pdw = (DWORD *)pvRes;
    dwRiff = *pdw++;
    dwLength = *pdw++;
    dwType = *pdw++;

    if (dwRiff != mmioFOURCC('R', 'I', 'F', 'F'))
        goto exit;      // not even RIFF

    if (dwType != mmioFOURCC('W', 'A', 'V', 'E'))
        goto exit;      // not a WAV

    pdwEnd = (DWORD *)((BYTE *)pdw + dwLength-4);

    while (pdw < pdwEnd)
    {
        dwType = *pdw++;
        dwLength = *pdw++;

        switch (dwType)
        {
        case mmioFOURCC('f', 'm', 't', ' '):
            if (ppWaveHeader && !*ppWaveHeader)
            {
                if (dwLength < sizeof(WAVEFORMAT))
                    goto exit;      // not a WAV

                *ppWaveHeader = (WAVEFORMATEX *)pdw;

                if ((!ppbWaveData || *ppbWaveData) &&
                    (!pcbWaveSize || *pcbWaveSize))
                {
                    return TRUE;
                }
            }
            break;

        case mmioFOURCC('d', 'a', 't', 'a'):
            if ((ppbWaveData && !*ppbWaveData) ||
                (pcbWaveSize && !*pcbWaveSize))
            {
                if (ppbWaveData)
                    *ppbWaveData = (LPBYTE)pdw;

                if (pcbWaveSize)
                    *pcbWaveSize = dwLength;

                if (!ppWaveHeader || *ppWaveHeader)
                    return TRUE;
            }
            break;
        }

        pdw = (DWORD *)((BYTE *)pdw + ((dwLength+1)&~1));
    }

exit:
    return FALSE;
}



CMCIObject::CMCIObject(HWND p_hwndnotify)
{
    m_OpenParams.dwCallback = 0;
    m_OpenParams.wDeviceID = 0;
    m_OpenParams.lpstrDeviceType = NULL;
    m_OpenParams.lpstrElementName = NULL;
    m_OpenParams.lpstrAlias = NULL;
    m_HwndNotify= p_hwndnotify;
}

CMCIObject::~CMCIObject()
{
    // Make sure the object is not in use.
    if (m_OpenParams.wDeviceID != 0) {
        Close();
    }
    ASSERT(m_OpenParams.wDeviceID == 0);
}
  
BOOL CMCIObject::OpenFile(const char* pszFileName)
{
    DWORD dwResult;

    if (m_OpenParams.wDeviceID != 0) {
        Close();
    }
    ASSERT(m_OpenParams.wDeviceID == 0);
    m_OpenParams.lpstrDeviceType = NULL;
    m_OpenParams.lpstrElementName = pszFileName;
    dwResult = mciSendCommand(0,
                              MCI_OPEN,
                              MCI_WAIT | MCI_OPEN_ELEMENT,
                              (DWORD)(LPVOID)&m_OpenParams);
    if (dwResult != 0) {
        COUTRESULT("CMCIObject::OpenFile1");
        return FALSE;
    }

    // Set the time format to milliseconds.
    MCI_SET_PARMS set;
    set.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
    dwResult = mciSendCommand(m_OpenParams.wDeviceID,
                              MCI_SET,
                              MCI_WAIT | MCI_SET_TIME_FORMAT,
                              (DWORD)(LPVOID)&set);
    if (dwResult != 0) {
       COUTRESULT("CMCIObject::OpenFile2");
        return FALSE;
    }
    return TRUE;
}
  
  
void CMCIObject::Close()
{
    MCI_GENERIC_PARMS gp;
    DWORD dwResult;

    if (m_OpenParams.wDeviceID == 0) return; // Already closed
    Stop(); // Just in case
    dwResult = mciSendCommand(m_OpenParams.wDeviceID,
                              MCI_CLOSE,
                              MCI_WAIT,
                              (DWORD)(LPVOID)&gp);
    if (dwResult != 0) {
        COUTRESULT("CMCIObject::close");
    }
    m_OpenParams.wDeviceID = 0;
}
  
  
void CMCIObject::Play()
{
    MCI_PLAY_PARMS play;
    DWORD dwResult;
    memset(&play, 0, sizeof(play)); //initialize play to 0

    dwResult = mciSendCommand(m_OpenParams.wDeviceID,
                   MCI_SEEK,
                   MCI_WAIT | MCI_SEEK_TO_START,
                   0);
    if (dwResult != 0) {
      CString t_string;
      mciGetErrorString(dwResult,t_string.GetBuffer(100), 100);
      t_string.ReleaseBuffer();
      COUTRESULT("CMCIObject::Play error");
      COUTRESULT(t_string.GetBuffer(0));
    }

    play.dwCallback = (DWORD) m_HwndNotify;
    dwResult = mciSendCommand(m_OpenParams.wDeviceID,
                              MCI_PLAY,
                              MCI_NOTIFY,
                              (DWORD)(LPVOID)&play);
    if (dwResult != 0) {
      CString t_string;
      mciGetErrorString(dwResult,t_string.GetBuffer(100), 100);
      t_string.ReleaseBuffer();
      COUTRESULT("CMCIObject::Play error");
      COUTRESULT(t_string.GetBuffer(0));
    }
}
  
  
void CMCIObject::Stop()
{
    DWORD dwResult;
    MCI_GENERIC_PARMS t_genparms;

    dwResult = mciSendCommand(m_OpenParams.wDeviceID,
                              MCI_STOP,
                              MCI_WAIT,
                              (DWORD)(LPVOID)NULL);
    if (dwResult != 0) {
        COUTRESULT("CMCIObject::Stop");
    }

    dwResult = mciSendCommand(m_OpenParams.wDeviceID, MCI_CLOSE, 
    NULL, (DWORD) (LPMCI_GENERIC_PARMS) &t_genparms);    
    if (dwResult != 0) {
        COUTRESULT("CMCIObject::Stop");
    }
}
  
DWORD CMCIObject::GetPosition()
{
    MCI_STATUS_PARMS status;
    status.dwItem = MCI_STATUS_POSITION;
    if (mciSendCommand(m_OpenParams.wDeviceID,
                       MCI_STATUS,
                       MCI_WAIT | MCI_STATUS_ITEM,
                       (DWORD)(LPVOID)&status) != 0) {
        return 0; // Some error
    }
    return status.dwReturn;
}

DWORD CMCIObject::GetLength()
{
    MCI_STATUS_PARMS status;
    status.dwItem = MCI_STATUS_LENGTH;
    if (mciSendCommand(m_OpenParams.wDeviceID,
                       MCI_STATUS,
                       MCI_WAIT | MCI_STATUS_ITEM,
                       (DWORD)(LPVOID)&status) != 0) {
        return 0; // Some error
    }
    return status.dwReturn;
}
  
void
CMCIObject::setEffectsVolume(DWORD p_vol)
{
     // Set the time format to milliseconds.
    MCI_SET_PARMS set;
    set.dwAudio=p_vol;
    mciSendCommand(m_OpenParams.wDeviceID,
                              MCI_SET,
                              MCI_WAIT | MCI_SET_AUDIO_ALL,
                              (DWORD)(LPVOID)&set);
}
