# XEvil Sound Assets

The original XEvil 2.02 Windows build shipped its audio embedded as PE resources
inside `xevil.exe`. The X11/UNIX build never had sound at all
(`SoundNames::lookup()` in `cmn/sound_cmn.cpp` was a stub returning 0).

These files were recovered from the original `xevil-windows.exe` (2.02,
2000-01-19) distributed at http://www.xevil.com/download, extracted with
`wrestool --raw` and renamed per the resource-ID tables in
`win32/resource.h` and `cmn/bitmaps/sound_cmn/sound_cmn.bitmaps`.

## Sound effects (WAV, mostly 8-bit mono 11 kHz — authentic 90s)

| File | SoundName | Resource |
|---|---|---|
| chainsaw.wav | CHAINSAW_SOUND | IDW 818 |
| flamethrower.wav | FLAMETHROWER | IDW 819 |
| death.wav | DEATH | IDW 820 |
| seal_death.wav | SEAL_DEATH | IDW 821 |
| hugger_death.wav | HUGGER_DEATH | IDW 822 |
| frog_death.wav | FROG_DEATH | IDW 823 |
| breakdown.wav | BREAKDOWN | IDW 826 |
| bang.wav | BANG | IDW 827 |
| pistol.wav | PISTOL | IDW 832 |
| mgun.wav | MGUN | IDW 833 |
| launcher.wav | LAUNCHER | IDW 834 |
| explosion.wav | EXPLOSION | IDW 835 |
| dog_death.wav | DOG_DEATH | IDW 877 |
| laser.wav | LASER | IDW 878 |
| hero_attack.wav | HERO_ATTACK | IDW 879 |
| ninja_attack.wav | NINJA_ATTACK | IDW 880 |
| dog_attack.wav | DOG_ATTACK | IDW 881 |
| chop_death.wav | CHOP_DEATH | IDW 882 |
| doppel_use.wav | DOPPEL_USE | IDW 883 |
| cloak_use.wav | CLOAK_USE | IDW 884 |
| trans_use.wav | TRANS_USE | IDW 885 |
| shield_use.wav | SHIELD_USE | IDW 886 |
| ninja_death.wav | NINJA_DEATH | IDW 887 |
| froggun.wav | FROGGUN | IDW 888 |
| lancer.wav | LANCER | IDW 889 |
| swapper.wav | SWAPPER | IDW 890 |

(IDW 824/830/831 wave soundtracks were already disabled in 2.02 and are not
present in the exe.)

## Music (original MIDI + pre-rendered MP3)

MP3s rendered with FluidSynth (FluidR3 GM soundfont) at 44.1 kHz / 128 kbps so
the game can stream them without a MIDI synth. The .mid originals are kept for
authenticity.

| File | SoundName | Resource | Notes |
|---|---|---|---|
| fire.mid/.mp3 | FIRE_SOUNDTRACK | IDM 1632 | |
| hive.mid/.mp3 | HIVE_SOUNDTRACK | IDM 1633 | |
| kill.mid/.mp3 | KILL_SOUNDTRACK | IDM 1634 | Kill'em All mode |
| seal.mid/.mp3 | SEAL_SOUNDTRACK | IDM 1630 | |
| zeepeeg.mid/.mp3 | ZEEPEEG_SOUNDTRACK | IDM 1635 | |
| nightsky.mid/.mp3 | NIGHTSKY_SOUNDTRACK | IDM 1636 | |
| sweetdark.mid/.mp3 | SWEETDARK_SOUNDTRACK | IDM 1637 | |
| terraexm.mid/.mp3 | TERRAEXM_SOUNDTRACK | IDM 1638 | commented out in 2.02 ("sounds really painful on cheap sound cards"); re-enabled |
| newsong.mid/.mp3 | NEWSONG_SOUNDTRACK | IDM 1639 | |

There is a tenth soundtrack that got away: `win32/resource.h:1009` defines
`IDM_DEATHMARCHSOUNDTRACK 1631` — the only surviving trace of a **"Death
March"** track. No `.mid`, no `.mp3`, and no code reference exists; the resource
is absent from the shipped exe, and the ID `1631` was even recycled for a wall
bitmap (`IDB_MD4WALL`). Truly lost unless an older 1.x / 2.0-beta build ever
surfaces.

XEvil is GPL v2; these assets ship with the official free distribution.
