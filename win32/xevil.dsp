# Microsoft Developer Studio Project File - Name="xevil" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=xevil - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "xevil.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "xevil.mak" CFG="xevil - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "xevil - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "xevil - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "xevil - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "xevil___"
# PROP BASE Intermediate_Dir "xevil___"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 winmm.lib ole32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "xevil__0"
# PROP BASE Intermediate_Dir "xevil__0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winmm.lib ole32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "xevil - Win32 Release"
# Name "xevil - Win32 Debug"
# Begin Source File

SOURCE=".\res\!dog0_ru.bmp"
# End Source File
# Begin Source File

SOURCE=.\res\27a.bmp
# End Source File
# Begin Source File

SOURCE=.\res\27b.bmp
# End Source File
# Begin Source File

SOURCE=.\res\39a.bmp
# End Source File
# Begin Source File

SOURCE=.\res\aaanew.bmp
# End Source File
# Begin Source File

SOURCE=.\res\aaanew1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\aaanew2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\aaanew5.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Aaanew7.bmp
# End Source File
# Begin Source File

SOURCE=.\aboutdialog.cpp
# End Source File
# Begin Source File

SOURCE=..\cmn\actual.cpp
# End Source File
# Begin Source File

SOURCE=.\res\Aliennu.bmp
# End Source File
# Begin Source File

SOURCE=..\cmn\area.cpp
# End Source File
# Begin Source File

SOURCE=.\res\arrow.cur
# End Source File
# Begin Source File

SOURCE=.\res\arrow_dn.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Arrow_l.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Arrow_up.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Autolanc.bmp
# End Source File
# Begin Source File

SOURCE=.\res\babyseal.mid
# End Source File
# Begin Source File

SOURCE=.\res\Back.bmp
# End Source File
# Begin Source File

SOURCE=.\res\backg.BMP
# End Source File
# Begin Source File

SOURCE=.\res\bangbang.wav
# End Source File
# Begin Source File

SOURCE=.\res\Barrow_d.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Barrow_l.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Barrow_r.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Barrow_u.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bceagle.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Beat_dog.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bitmap2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bitmap3.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bitmap6.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bitmap7.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bitmap8.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Blok2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Blood13.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Blood25.bmp
# End Source File
# Begin Source File

SOURCE=.\res\blood39.bmp
# End Source File
# Begin Source File

SOURCE=.\res\blood4.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Blood40.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Blood9.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bloodg25.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bloodg39.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bloodg4.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bloodg40.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bloodg9.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00001.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00002.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00003.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00004.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00006.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00007.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\Bmp00010.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00011.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00012.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00013.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00014.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00015.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00016.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00017.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00018.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\Bmp00019.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00020.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00021.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00022.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\Bmp00023.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00024.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00026.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00027.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00028.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00030.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00031.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\Bmp00032.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00034.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\Bmp00035.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00038.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00039.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00040.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00041.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00042.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00043.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00044.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00045.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00046.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00047.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00048.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00049.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00050.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00051.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00052.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00053.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00054.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00055.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00056.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00057.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00058.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00059.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00060.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00061.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00062.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00064.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00066.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00067.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00068.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00069.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00070.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00071.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00072.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00073.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00074.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00075.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00081.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00084.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00085.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00091.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00092.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00093.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00094.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00096.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00099.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00101.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00103.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00105.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00107.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00109.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00111.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00113.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00123.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00125.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00127.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00129.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00131.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00132.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00134.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00136.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00138.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00140.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00151.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Bmp00154.bmp
# End Source File
# Begin Source File

SOURCE=.\res\breakdwn.wav
# End Source File
# Begin Source File

SOURCE=.\res\Bshield.bmp
# End Source File
# Begin Source File

SOURCE=.\res\c0.bmp
# End Source File
# Begin Source File

SOURCE=.\res\c1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\c2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\c3.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Cat.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Chainsaw.bmp
# End Source File
# Begin Source File

SOURCE=.\res\chainsw.wav
# End Source File
# Begin Source File

SOURCE=.\res\chicken0.bmp
# End Source File
# Begin Source File

SOURCE=.\res\chicken2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\chicken4.bmp
# End Source File
# Begin Source File

SOURCE=.\res\chicken5.bmp
# End Source File
# Begin Source File

SOURCE=.\res\chk0.bmp
# End Source File
# Begin Source File

SOURCE=.\res\chk0b.bmp
# End Source File
# Begin Source File

SOURCE=.\res\chk0c.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Choppr0.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Choppr0b.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Choppr0c.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Choppr0d.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Choppr0e.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Choppr0f.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Choppr2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Choppr25.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Choppr2b.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Choppr2c.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Choppr2d.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Choppr4.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Choppr4b.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Choppr4c.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Choppr4d.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Choppr5.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Chopprd.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Chopprdb.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Chopprdc.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Chopprdd.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Choppru.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Chopprub.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Choppruc.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Chopprud.bmp
# End Source File
# Begin Source File

SOURCE=.\res\chpdeath.wav
# End Source File
# Begin Source File

SOURCE=.\res\cloak.wav
# End Source File
# Begin Source File

SOURCE=.\connectdlg.cpp
# End Source File
# Begin Source File

SOURCE=..\cmn\coord.cpp
# End Source File
# Begin Source File

SOURCE=".\res\crawl-forward.bmp"
# End Source File
# Begin Source File

SOURCE=".\res\crawl-right-1.bmp"
# End Source File
# Begin Source File

SOURCE=".\res\crawl-right-2.bmp"
# End Source File
# Begin Source File

SOURCE=".\res\crawl-right-3.bmp"
# End Source File
# Begin Source File

SOURCE=".\res\crawl-right-4.bmp"
# End Source File
# Begin Source File

SOURCE=".\res\crawl-right-5.bmp"
# End Source File
# Begin Source File

SOURCE=".\res\crawl-right-6.bmp"
# End Source File
# Begin Source File

SOURCE=.\ddraw.lib
# End Source File
# Begin Source File

SOURCE=.\res\death.wav
# End Source File
# Begin Source File

SOURCE=.\res\Demonsum.bmp
# End Source File
# Begin Source File

SOURCE=.\res\dethmrch.mid
# End Source File
# Begin Source File

SOURCE=.\res\Dg5_a.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Dg5_b.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Dg_atd.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Dg_atu.bmp
# End Source File
# Begin Source File

SOURCE=.\difficulty.cpp
# End Source File
# Begin Source File

SOURCE=.\res\Displayb.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Dog_att0.bmp
# End Source File
# Begin Source File

SOURCE=.\res\dogatt.wav
# End Source File
# Begin Source File

SOURCE=.\res\dogdeath.wav
# End Source File
# Begin Source File

SOURCE=.\res\Door1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Doorbot.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Doortop.bmp
# End Source File
# Begin Source File

SOURCE=.\res\doppel.wav
# End Source File
# Begin Source File

SOURCE=.\res\Dragonde.bmp
# End Source File
# Begin Source File

SOURCE=.\draw.cpp
# End Source File
# Begin Source File

SOURCE=.\res\Drg_1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Drg_2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Drg_bdy.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Drg_head.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Drgbite.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Drop_sch.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Droplift.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\Drugs.bmp
# End Source File
# Begin Source File

SOURCE=.\dsound.lib
# End Source File
# Begin Source File

SOURCE=.\res\Egg.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Egg_open.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Empty.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Endblok.bmp
# End Source File
# Begin Source File

SOURCE=.\res\explosion.wav
# End Source File
# Begin Source File

SOURCE=.\res\feather2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\feather3.bmp
# End Source File
# Begin Source File

SOURCE=.\res\feather4.bmp
# End Source File
# Begin Source File

SOURCE=.\res\feather5.bmp
# End Source File
# Begin Source File

SOURCE=.\res\feather9.bmp
# End Source File
# Begin Source File

SOURCE=.\fileman.cpp
# End Source File
# Begin Source File

SOURCE=.\res\Fire.bmp
# End Source File
# Begin Source File

SOURCE=.\res\fire.mid
# End Source File
# Begin Source File

SOURCE=.\res\Fireball.bmp
# End Source File
# Begin Source File

SOURCE=.\res\fireball.wav
# End Source File
# Begin Source File

SOURCE=.\firepal.cpp
# End Source File
# Begin Source File

SOURCE=.\res\Fist.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Flame2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Flame3.bmp
# End Source File
# Begin Source File

SOURCE=.\fogpal.cpp
# End Source File
# Begin Source File

SOURCE=.\res\Frog0.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Frog2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Frog4.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Frog5.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Frog_dea.bmp
# End Source File
# Begin Source File

SOURCE=.\res\frogdeth.wav
# End Source File
# Begin Source File

SOURCE=.\res\Froggun.bmp
# End Source File
# Begin Source File

SOURCE=.\res\froggun.wav
# End Source File
# Begin Source File

SOURCE=.\fsstatus.cpp
# End Source File
# Begin Source File

SOURCE=.\res\fwbackgr.bmp
# End Source File
# Begin Source File

SOURCE=..\cmn\game.cpp
# End Source File
# Begin Source File

SOURCE=..\cmn\game_style.cpp
# End Source File
# Begin Source File

SOURCE=.\res\Garrow_d.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Garrow_l.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Garrow_r.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Garrow_u.bmp
# End Source File
# Begin Source File

SOURCE=.\glowpal.cpp
# End Source File
# Begin Source File

SOURCE=.\res\Handtoha.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Happy.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hell5.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hell_1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Henblok.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hero0.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hero0_at.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hero0b.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hero0c.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hero0d.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hero0e.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hero0f.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hero10.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hero10b.bmp
# End Source File
# Begin Source File

SOURCE=.\res\heroatt.wav
# End Source File
# Begin Source File

SOURCE=.\res\hhero18c.bmp
# End Source File
# Begin Source File

SOURCE=.\res\hhero18d.bmp
# End Source File
# Begin Source File

SOURCE=.\res\hhero18e.bmp
# End Source File
# Begin Source File

SOURCE=.\res\hhero18f.bmp
# End Source File
# Begin Source File

SOURCE=.\res\hhero18g.bmp
# End Source File
# Begin Source File

SOURCE=.\res\hhero21_.bmp
# End Source File
# Begin Source File

SOURCE=.\res\hhero_18.bmp
# End Source File
# Begin Source File

SOURCE=.\res\hi_alien.bmp
# End Source File
# Begin Source File

SOURCE=.\res\hialien0.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\Hialien1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\hialien2.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\hialien4.bmp
# End Source File
# Begin Source File

SOURCE=.\res\hialien5.bmp
# End Source File
# Begin Source File

SOURCE=.\res\hialien6.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\Hialtar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hiddog0.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hiddog0_.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hiddog0b.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hiddog2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hiddog21.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hiddog22.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hiddog2_.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hiddog5.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hiddog5b.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hiddogde.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hidog0.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hidog2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hidog21.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hidog21b.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hidog22.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hidog22b.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hidog2_a.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hidogdea.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hidoor2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hiflag.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hihero19.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hihero21.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hihero22.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hihero4.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hihero5.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hihero_d.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hihome.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hilaunch.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Himedkit.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hininja1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hininja2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hininja_.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hipistol.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hitextbl.bmp
# End Source File
# Begin Source File

SOURCE=.\res\hive.mid
# End Source File
# Begin Source File

SOURCE=.\res\Hiwalk18.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hiwalk2b.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hiwalk4.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hiwalk4b.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hiwalk4c.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hiwalk4d.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hiwalk5.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hiwalk5b.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hiwalk5c.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hiwalk5d.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hiwalk_0.bmp
# End Source File
# Begin Source File

SOURCE=.\res\hiwalk_0b.bmp
# End Source File
# Begin Source File

SOURCE=.\res\hiwalk_0c.bmp
# End Source File
# Begin Source File

SOURCE=.\res\hiwalk_0d.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hiwalk_2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hiwalk_d.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Hiwlk18b.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\Htrack.bmp
# End Source File
# Begin Source File

SOURCE=..\cmn\id.cpp
# End Source File
# Begin Source File

SOURCE=.\res\idr_xevi.ico
# End Source File
# Begin Source File

SOURCE=.\res\Ind1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Indt2.bmp
# End Source File
# Begin Source File

SOURCE=..\cmn\intel.cpp
# End Source File
# Begin Source File

SOURCE=.\res\Jump0.bmp
# End Source File
# Begin Source File

SOURCE=.\keyhitdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\keysets.cpp
# End Source File
# Begin Source File

SOURCE=.\res\kill_par.bmp
# End Source File
# Begin Source File

SOURCE=.\res\kkkill.mid
# End Source File
# Begin Source File

SOURCE=..\cmn\l_agreement.cpp
# End Source File
# Begin Source File

SOURCE=.\res\Ladder.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Ladder2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Lance_ho.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Lance_ve.bmp
# End Source File
# Begin Source File

SOURCE=.\res\lancer.wav
# End Source File
# Begin Source File

SOURCE=.\res\Lancer1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\laser.wav
# End Source File
# Begin Source File

SOURCE=.\res\Laser_ho.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Lasers.bmp
# End Source File
# Begin Source File

SOURCE=.\res\lem_trak.wav
# End Source File
# Begin Source File

SOURCE=.\res\lemdeth.wav
# End Source File
# Begin Source File

SOURCE=..\cmn\locator.cpp
# End Source File
# Begin Source File

SOURCE=.\res\Logob5.bmp
# End Source File
# Begin Source File

SOURCE=.\res\machgun.wav
# End Source File
# Begin Source File

SOURCE=.\Res\Md2door1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Md2door2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Md2ladde.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Md2mover.bmp
# End Source File
# Begin Source File

SOURCE=.\res\md2outsi.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\Md2updow.bmp
# End Source File
# Begin Source File

SOURCE=.\res\md4backg.bmp
# End Source File
# Begin Source File

SOURCE=.\res\md4door1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Md4door2.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\Md4horiz.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Md4ladde.bmp
# End Source File
# Begin Source File

SOURCE=.\res\md4outsi.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Md4updow.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Md4vertm.bmp
# End Source File
# Begin Source File

SOURCE=.\res\md4wall.bmp
# End Source File
# Begin Source File

SOURCE=.\res\md5backg.bmp
# End Source File
# Begin Source File

SOURCE=.\res\md5door1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\md5door2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\md5horiz.bmp
# End Source File
# Begin Source File

SOURCE=.\res\md5ladde.bmp
# End Source File
# Begin Source File

SOURCE=.\res\md5mover.bmp
# End Source File
# Begin Source File

SOURCE=.\res\md5outsi.bmp
# End Source File
# Begin Source File

SOURCE=.\res\md5updow.bmp
# End Source File
# Begin Source File

SOURCE=.\res\md5vertm.bmp
# End Source File
# Begin Source File

SOURCE=.\res\md5wall.bmp
# End Source File
# Begin Source File

SOURCE=.\res\mdoutsid.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Menunewg.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Mgun1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\missile.wav
# End Source File
# Begin Source File

SOURCE=.\res\Missle25.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Missle39.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Missle40.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Mover.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Moversq.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Moversqu.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Napalm1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Napalm3.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Napalm4.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Napalms.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Newsong.mid
# End Source File
# Begin Source File

SOURCE=.\res\Nightsky.mid
# End Source File
# Begin Source File

SOURCE=.\res\Nin0.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Nin0b.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Nin0c.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Nin0d.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Nin0e.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Nin0f.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Nin5.bmp
# End Source File
# Begin Source File

SOURCE=.\res\nin_kick0jmp.bmp
# End Source File
# Begin Source File

SOURCE=.\res\nin_kick0low.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Nin_lo0.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Nin_lo0b.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Nin_lo0c.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Nin_lo0d.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Nin_lo0e.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Nin_lo0f.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ninatt_down.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Ninj0a.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Ninj0b.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Ninj0c.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Ninj0d.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ninjaatt.wav
# End Source File
# Begin Source File

SOURCE=.\res\ninjadth.wav
# End Source File
# Begin Source File

SOURCE=.\res\ninjump_down1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ninjump_down2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ninjump_up.bmp
# End Source File
# Begin Source File

SOURCE=.\res\nlift.BMP
# End Source File
# Begin Source File

SOURCE=.\res\None.bmp
# End Source File
# Begin Source File

SOURCE=.\res\None1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Oil25.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Oil39.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Oil4.bmp
# End Source File
# Begin Source File

SOURCE=.\res\oil40.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Oil9.bmp
# End Source File
# Begin Source File

SOURCE=.\optionsdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\res\Outa.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Outb.bmp
# End Source File
# Begin Source File

SOURCE=.\res\outs.BMP
# End Source File
# Begin Source File

SOURCE=.\res\Outside.bmp
# End Source File
# Begin Source File

SOURCE=.\palmod.cpp
# End Source File
# Begin Source File

SOURCE=.\res\Parents.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Pent.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Pepper1.bmp
# End Source File
# Begin Source File

SOURCE=..\cmn\physical.cpp
# End Source File
# Begin Source File

SOURCE=.\res\Pills.bmp
# End Source File
# Begin Source File

SOURCE=.\res\pistol.wav
# End Source File
# Begin Source File

SOURCE=.\res\Pyramid.bmp
# End Source File
# Begin Source File

SOURCE=.\res\r0.bmp
# End Source File
# Begin Source File

SOURCE=.\res\r1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\r2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\r3.bmp
# End Source File
# Begin Source File

SOURCE=.\res\r4.bmp
# End Source File
# Begin Source File

SOURCE=.\res\r5.bmp
# End Source File
# Begin Source File

SOURCE=.\res\rarrow_r.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Rockhi.bmp
# End Source File
# Begin Source File

SOURCE=..\cmn\role.cpp
# End Source File
# Begin Source File

SOURCE=.\runserverdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\s_man.cpp
# End Source File
# Begin Source File

SOURCE=.\res\Seal0.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Seal0b.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Shell.bmp
# End Source File
# Begin Source File

SOURCE=.\res\shield.wav
# End Source File
# Begin Source File

SOURCE=.\res\Sider.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Skull.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Small.ico
# End Source File
# Begin Source File

SOURCE=.\res\sndtrak.wav
# End Source File
# Begin Source File

SOURCE=.\sound.cpp
# End Source File
# Begin Source File

SOURCE=..\cmn\sound_cmn.cpp
# End Source File
# Begin Source File

SOURCE=.\specialcontrols.cpp
# End Source File
# Begin Source File

SOURCE=.\res\Squanch_.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Star.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Starb.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Stars.bmp
# End Source File
# Begin Source File

SOURCE=.\stdafx.cpp
# End Source File
# Begin Source File

SOURCE=.\storydlg.cpp
# End Source File
# Begin Source File

SOURCE=..\cmn\streams.cpp
# End Source File
# Begin Source File

SOURCE=.\res\Suicideb.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Swapper.bmp
# End Source File
# Begin Source File

SOURCE=.\res\swapper.wav
# End Source File
# Begin Source File

SOURCE=.\res\Sweetdar.mid
# End Source File
# Begin Source File

SOURCE=.\res\Take_can.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Terraexm.mid
# End Source File
# Begin Source File

SOURCE=.\timedlg.cpp
# End Source File
# Begin Source File

SOURCE=.\res\Track.wav
# End Source File
# Begin Source File

SOURCE=.\res\transmog.wav
# End Source File
# Begin Source File

SOURCE=.\res\trapopen.bmp
# End Source File
# Begin Source File

SOURCE=.\res\troll0.bmp
# End Source File
# Begin Source File

SOURCE=.\res\troll0b.bmp
# End Source File
# Begin Source File

SOURCE=.\res\troll0c.bmp
# End Source File
# Begin Source File

SOURCE=.\res\troll0d.bmp
# End Source File
# Begin Source File

SOURCE=.\res\troll1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\troll18.bmp
# End Source File
# Begin Source File

SOURCE=.\res\troll18b.bmp
# End Source File
# Begin Source File

SOURCE=.\res\troll25.bmp
# End Source File
# Begin Source File

SOURCE=.\res\troll4.bmp
# End Source File
# Begin Source File

SOURCE=.\res\troll5.bmp
# End Source File
# Begin Source File

SOURCE=.\ui.cpp
# End Source File
# Begin Source File

SOURCE=..\cmn\ui_cmn.cpp
# End Source File
# Begin Source File

SOURCE=.\uiplayer.cpp
# End Source File
# Begin Source File

SOURCE=.\uiserver.cpp
# End Source File
# Begin Source File

SOURCE=.\Res\Updn.bmp
# End Source File
# Begin Source File

SOURCE=..\cmn\utils.cpp
# End Source File
# Begin Source File

SOURCE=.\res\vege2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Vegetabl.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Vertmove.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\Vtrack.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\Wall.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Weight2.bmp
# End Source File
# Begin Source File

SOURCE=.\wheel.cpp
# End Source File
# Begin Source File

SOURCE=..\cmn\world.cpp
# End Source File
# Begin Source File

SOURCE=.\res\Wsqaure.bmp
# End Source File
# Begin Source File

SOURCE=.\xdata.cpp
# End Source File
# Begin Source File

SOURCE=..\cmn\xetp.cpp
# End Source File
# Begin Source File

SOURCE=..\cmn\xetp_basic.cpp
# End Source File
# Begin Source File

SOURCE=.\xevil.cpp
# End Source File
# Begin Source File

SOURCE=.\res\xevil.pal
# End Source File
# Begin Source File

SOURCE=.\xevil.rc
# End Source File
# Begin Source File

SOURCE=.\res\Xevil1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\xevil1.cur
# End Source File
# Begin Source File

SOURCE=.\xevilserverstatus.cpp
# End Source File
# Begin Source File

SOURCE=.\res\xit.bmp
# End Source File
# Begin Source File

SOURCE=.\xviewport.cpp
# End Source File
# Begin Source File

SOURCE=.\res\Yarrow_d.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Yarrow_l.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Yarrow_r.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Yarrow_u.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Yshield.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Zeepeeg.mid
# End Source File
# End Target
# End Project
