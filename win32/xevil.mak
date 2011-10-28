# Microsoft Developer Studio Generated NMAKE File, Based on xevil.dsp
!IF "$(CFG)" == ""
CFG=xevil - Win32 Debug
!MESSAGE No configuration specified. Defaulting to xevil - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "xevil - Win32 Release" && "$(CFG)" != "xevil - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "xevil - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\xevil.exe"

!ELSE 

ALL : "$(OUTDIR)\xevil.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\aboutdialog.obj"
	-@erase "$(INTDIR)\actual.obj"
	-@erase "$(INTDIR)\area.obj"
	-@erase "$(INTDIR)\connectdlg.obj"
	-@erase "$(INTDIR)\coord.obj"
	-@erase "$(INTDIR)\difficulty.obj"
	-@erase "$(INTDIR)\draw.obj"
	-@erase "$(INTDIR)\fileman.obj"
	-@erase "$(INTDIR)\firepal.obj"
	-@erase "$(INTDIR)\fogpal.obj"
	-@erase "$(INTDIR)\fsstatus.obj"
	-@erase "$(INTDIR)\game.obj"
	-@erase "$(INTDIR)\game_style.obj"
	-@erase "$(INTDIR)\glowpal.obj"
	-@erase "$(INTDIR)\id.obj"
	-@erase "$(INTDIR)\intel.obj"
	-@erase "$(INTDIR)\keyhitdlg.obj"
	-@erase "$(INTDIR)\keysets.obj"
	-@erase "$(INTDIR)\l_agreement.obj"
	-@erase "$(INTDIR)\locator.obj"
	-@erase "$(INTDIR)\optionsdlg.obj"
	-@erase "$(INTDIR)\palmod.obj"
	-@erase "$(INTDIR)\physical.obj"
	-@erase "$(INTDIR)\role.obj"
	-@erase "$(INTDIR)\runserverdlg.obj"
	-@erase "$(INTDIR)\s_man.obj"
	-@erase "$(INTDIR)\sound.obj"
	-@erase "$(INTDIR)\sound_cmn.obj"
	-@erase "$(INTDIR)\specialcontrols.obj"
	-@erase "$(INTDIR)\stdafx.obj"
	-@erase "$(INTDIR)\storydlg.obj"
	-@erase "$(INTDIR)\streams.obj"
	-@erase "$(INTDIR)\timedlg.obj"
	-@erase "$(INTDIR)\ui.obj"
	-@erase "$(INTDIR)\ui_cmn.obj"
	-@erase "$(INTDIR)\uiplayer.obj"
	-@erase "$(INTDIR)\uiserver.obj"
	-@erase "$(INTDIR)\utils.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\wheel.obj"
	-@erase "$(INTDIR)\world.obj"
	-@erase "$(INTDIR)\xdata.obj"
	-@erase "$(INTDIR)\xetp.obj"
	-@erase "$(INTDIR)\xetp_basic.obj"
	-@erase "$(INTDIR)\xevil.obj"
	-@erase "$(INTDIR)\xevil.res"
	-@erase "$(INTDIR)\xevilserverstatus.obj"
	-@erase "$(INTDIR)\xviewport.obj"
	-@erase "$(OUTDIR)\xevil.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)\xevil.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Release/
CPP_SBRS=.

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o NUL /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\xevil.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\xevil.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=winmm.lib ole32.lib /nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)\xevil.pdb" /machine:I386 /out:"$(OUTDIR)\xevil.exe" 
LINK32_OBJS= \
	"$(INTDIR)\aboutdialog.obj" \
	"$(INTDIR)\actual.obj" \
	"$(INTDIR)\area.obj" \
	"$(INTDIR)\connectdlg.obj" \
	"$(INTDIR)\coord.obj" \
	"$(INTDIR)\difficulty.obj" \
	"$(INTDIR)\draw.obj" \
	"$(INTDIR)\fileman.obj" \
	"$(INTDIR)\firepal.obj" \
	"$(INTDIR)\fogpal.obj" \
	"$(INTDIR)\fsstatus.obj" \
	"$(INTDIR)\game.obj" \
	"$(INTDIR)\game_style.obj" \
	"$(INTDIR)\glowpal.obj" \
	"$(INTDIR)\id.obj" \
	"$(INTDIR)\intel.obj" \
	"$(INTDIR)\keyhitdlg.obj" \
	"$(INTDIR)\keysets.obj" \
	"$(INTDIR)\l_agreement.obj" \
	"$(INTDIR)\locator.obj" \
	"$(INTDIR)\optionsdlg.obj" \
	"$(INTDIR)\palmod.obj" \
	"$(INTDIR)\physical.obj" \
	"$(INTDIR)\role.obj" \
	"$(INTDIR)\runserverdlg.obj" \
	"$(INTDIR)\s_man.obj" \
	"$(INTDIR)\sound.obj" \
	"$(INTDIR)\sound_cmn.obj" \
	"$(INTDIR)\specialcontrols.obj" \
	"$(INTDIR)\stdafx.obj" \
	"$(INTDIR)\storydlg.obj" \
	"$(INTDIR)\streams.obj" \
	"$(INTDIR)\timedlg.obj" \
	"$(INTDIR)\ui.obj" \
	"$(INTDIR)\ui_cmn.obj" \
	"$(INTDIR)\uiplayer.obj" \
	"$(INTDIR)\uiserver.obj" \
	"$(INTDIR)\utils.obj" \
	"$(INTDIR)\wheel.obj" \
	"$(INTDIR)\world.obj" \
	"$(INTDIR)\xdata.obj" \
	"$(INTDIR)\xetp.obj" \
	"$(INTDIR)\xetp_basic.obj" \
	"$(INTDIR)\xevil.obj" \
	"$(INTDIR)\xevil.res" \
	"$(INTDIR)\xevilserverstatus.obj" \
	"$(INTDIR)\xviewport.obj" \
	".\ddraw.lib" \
	".\dsound.lib"

"$(OUTDIR)\xevil.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\xevil.exe"

!ELSE 

ALL : "$(OUTDIR)\xevil.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\aboutdialog.obj"
	-@erase "$(INTDIR)\actual.obj"
	-@erase "$(INTDIR)\area.obj"
	-@erase "$(INTDIR)\connectdlg.obj"
	-@erase "$(INTDIR)\coord.obj"
	-@erase "$(INTDIR)\difficulty.obj"
	-@erase "$(INTDIR)\draw.obj"
	-@erase "$(INTDIR)\fileman.obj"
	-@erase "$(INTDIR)\firepal.obj"
	-@erase "$(INTDIR)\fogpal.obj"
	-@erase "$(INTDIR)\fsstatus.obj"
	-@erase "$(INTDIR)\game.obj"
	-@erase "$(INTDIR)\game_style.obj"
	-@erase "$(INTDIR)\glowpal.obj"
	-@erase "$(INTDIR)\id.obj"
	-@erase "$(INTDIR)\intel.obj"
	-@erase "$(INTDIR)\keyhitdlg.obj"
	-@erase "$(INTDIR)\keysets.obj"
	-@erase "$(INTDIR)\l_agreement.obj"
	-@erase "$(INTDIR)\locator.obj"
	-@erase "$(INTDIR)\optionsdlg.obj"
	-@erase "$(INTDIR)\palmod.obj"
	-@erase "$(INTDIR)\physical.obj"
	-@erase "$(INTDIR)\role.obj"
	-@erase "$(INTDIR)\runserverdlg.obj"
	-@erase "$(INTDIR)\s_man.obj"
	-@erase "$(INTDIR)\sound.obj"
	-@erase "$(INTDIR)\sound_cmn.obj"
	-@erase "$(INTDIR)\specialcontrols.obj"
	-@erase "$(INTDIR)\stdafx.obj"
	-@erase "$(INTDIR)\storydlg.obj"
	-@erase "$(INTDIR)\streams.obj"
	-@erase "$(INTDIR)\timedlg.obj"
	-@erase "$(INTDIR)\ui.obj"
	-@erase "$(INTDIR)\ui_cmn.obj"
	-@erase "$(INTDIR)\uiplayer.obj"
	-@erase "$(INTDIR)\uiserver.obj"
	-@erase "$(INTDIR)\utils.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(INTDIR)\wheel.obj"
	-@erase "$(INTDIR)\world.obj"
	-@erase "$(INTDIR)\xdata.obj"
	-@erase "$(INTDIR)\xetp.obj"
	-@erase "$(INTDIR)\xetp_basic.obj"
	-@erase "$(INTDIR)\xevil.obj"
	-@erase "$(INTDIR)\xevil.res"
	-@erase "$(INTDIR)\xevilserverstatus.obj"
	-@erase "$(INTDIR)\xviewport.obj"
	-@erase "$(OUTDIR)\xevil.exe"
	-@erase "$(OUTDIR)\xevil.ilk"
	-@erase "$(OUTDIR)\xevil.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)\xevil.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o NUL /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\xevil.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\xevil.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=winmm.lib ole32.lib /nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)\xevil.pdb" /debug /machine:I386 /out:"$(OUTDIR)\xevil.exe"\
 /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\aboutdialog.obj" \
	"$(INTDIR)\actual.obj" \
	"$(INTDIR)\area.obj" \
	"$(INTDIR)\connectdlg.obj" \
	"$(INTDIR)\coord.obj" \
	"$(INTDIR)\difficulty.obj" \
	"$(INTDIR)\draw.obj" \
	"$(INTDIR)\fileman.obj" \
	"$(INTDIR)\firepal.obj" \
	"$(INTDIR)\fogpal.obj" \
	"$(INTDIR)\fsstatus.obj" \
	"$(INTDIR)\game.obj" \
	"$(INTDIR)\game_style.obj" \
	"$(INTDIR)\glowpal.obj" \
	"$(INTDIR)\id.obj" \
	"$(INTDIR)\intel.obj" \
	"$(INTDIR)\keyhitdlg.obj" \
	"$(INTDIR)\keysets.obj" \
	"$(INTDIR)\l_agreement.obj" \
	"$(INTDIR)\locator.obj" \
	"$(INTDIR)\optionsdlg.obj" \
	"$(INTDIR)\palmod.obj" \
	"$(INTDIR)\physical.obj" \
	"$(INTDIR)\role.obj" \
	"$(INTDIR)\runserverdlg.obj" \
	"$(INTDIR)\s_man.obj" \
	"$(INTDIR)\sound.obj" \
	"$(INTDIR)\sound_cmn.obj" \
	"$(INTDIR)\specialcontrols.obj" \
	"$(INTDIR)\stdafx.obj" \
	"$(INTDIR)\storydlg.obj" \
	"$(INTDIR)\streams.obj" \
	"$(INTDIR)\timedlg.obj" \
	"$(INTDIR)\ui.obj" \
	"$(INTDIR)\ui_cmn.obj" \
	"$(INTDIR)\uiplayer.obj" \
	"$(INTDIR)\uiserver.obj" \
	"$(INTDIR)\utils.obj" \
	"$(INTDIR)\wheel.obj" \
	"$(INTDIR)\world.obj" \
	"$(INTDIR)\xdata.obj" \
	"$(INTDIR)\xetp.obj" \
	"$(INTDIR)\xetp_basic.obj" \
	"$(INTDIR)\xevil.obj" \
	"$(INTDIR)\xevil.res" \
	"$(INTDIR)\xevilserverstatus.obj" \
	"$(INTDIR)\xviewport.obj" \
	".\ddraw.lib" \
	".\dsound.lib"

"$(OUTDIR)\xevil.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "xevil - Win32 Release" || "$(CFG)" == "xevil - Win32 Debug"
SOURCE=.\aboutdialog.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_ABOUT=\
	".\aboutdialog.h"\
	".\stdafx.h"\
	".\xevil.h"\
	{$(INCLUDE)}"l_agreement.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"utils.h"\
	

"$(INTDIR)\aboutdialog.obj" : $(SOURCE) $(DEP_CPP_ABOUT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_ABOUT=\
	".\aboutdialog.h"\
	".\stdafx.h"\
	".\xevil.h"\
	{$(INCLUDE)}"l_agreement.h"\
	{$(INCLUDE)}"utils.h"\
	

"$(INTDIR)\aboutdialog.obj" : $(SOURCE) $(DEP_CPP_ABOUT) "$(INTDIR)"


!ENDIF 

SOURCE=..\cmn\actual.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_ACTUA=\
	"..\cmn\bitmaps\alien\alien.bitmaps"\
	"..\cmn\bitmaps\altar_of_sin\altar_of_sin.bitmaps"\
	"..\cmn\bitmaps\auto_lancer\auto_lancer.bitmaps"\
	"..\cmn\bitmaps\blood\blood.bitmaps"\
	"..\cmn\bitmaps\bomb\bomb.bitmaps"\
	"..\cmn\bitmaps\caffine\caffine.bitmaps"\
	"..\cmn\bitmaps\chainsaw\chainsaw.bitmaps"\
	"..\cmn\bitmaps\chicken\chicken.bitmaps"\
	"..\cmn\bitmaps\chopper_boy\chopper_boy.bitmaps"\
	"..\cmn\bitmaps\cloak\cloak.bitmaps"\
	"..\cmn\bitmaps\crack_pipe\crack_pipe.bitmaps"\
	"..\cmn\bitmaps\demon_summoner\demon_summoner.bitmaps"\
	"..\cmn\bitmaps\dog\dog.bitmaps"\
	"..\cmn\bitmaps\dog_whistle\dog_whistle.bitmaps"\
	"..\cmn\bitmaps\doppel\doppel.bitmaps"\
	"..\cmn\bitmaps\dragon\dragon.bitmaps"\
	"..\cmn\bitmaps\egg\egg.bitmaps"\
	"..\cmn\bitmaps\enforcer\enforcer.bitmaps"\
	"..\cmn\bitmaps\explosion\explosion.bitmaps"\
	"..\cmn\bitmaps\f_thrower\f_thrower.bitmaps"\
	"..\cmn\bitmaps\feather\feather.bitmaps"\
	"..\cmn\bitmaps\fire\fire.bitmaps"\
	"..\cmn\bitmaps\fire_demon\fire_demon.bitmaps"\
	"..\cmn\bitmaps\fire_explosion\fire_explosion.bitmaps"\
	"..\cmn\bitmaps\fireball\fireball.bitmaps"\
	"..\cmn\bitmaps\flag\flag.bitmaps"\
	"..\cmn\bitmaps\frog\frog.bitmaps"\
	"..\cmn\bitmaps\frog_gun\frog_gun.bitmaps"\
	"..\cmn\bitmaps\frog_shell\frog_shell.bitmaps"\
	"..\cmn\bitmaps\green_blood\green_blood.bitmaps"\
	"..\cmn\bitmaps\grenade\grenade.bitmaps"\
	"..\cmn\bitmaps\grenades\grenades.bitmaps"\
	"..\cmn\bitmaps\hero\hero.bitmaps"\
	"..\cmn\bitmaps\home\home.bitmaps"\
	"..\cmn\bitmaps\hugger\hugger.bitmaps"\
	"..\cmn\bitmaps\lance\lance.bitmaps"\
	"..\cmn\bitmaps\lancer\lancer.bitmaps"\
	"..\cmn\bitmaps\laser\laser.bitmaps"\
	"..\cmn\bitmaps\launcher\launcher.bitmaps"\
	"..\cmn\bitmaps\m_gun\m_gun.bitmaps"\
	"..\cmn\bitmaps\med_kit\med_kit.bitmaps"\
	"..\cmn\bitmaps\missile\missile.bitmaps"\
	"..\cmn\bitmaps\n_protection\n_protection.bitmaps"\
	"..\cmn\bitmaps\n_shield\n_shield.bitmaps"\
	"..\cmn\bitmaps\napalm\napalm.bitmaps"\
	"..\cmn\bitmaps\napalms\napalms.bitmaps"\
	"..\cmn\bitmaps\ninja\ninja.bitmaps"\
	"..\cmn\bitmaps\oil_droplet\oil_droplet.bitmaps"\
	"..\cmn\bitmaps\p_c_p\p_c_p.bitmaps"\
	"..\cmn\bitmaps\phys_mover\phys_mover.bitmaps"\
	"..\cmn\bitmaps\pistol\pistol.bitmaps"\
	"..\cmn\bitmaps\rock\rock.bitmaps"\
	"..\cmn\bitmaps\seal\seal.bitmaps"\
	"..\cmn\bitmaps\shell\shell.bitmaps"\
	"..\cmn\bitmaps\star\star.bitmaps"\
	"..\cmn\bitmaps\stars\stars.bitmaps"\
	"..\cmn\bitmaps\swap_shell\swap_shell.bitmaps"\
	"..\cmn\bitmaps\swapper\swapper.bitmaps"\
	"..\cmn\bitmaps\t_protection\t_protection.bitmaps"\
	"..\cmn\bitmaps\t_shield\t_shield.bitmaps"\
	"..\cmn\bitmaps\transmogifier\transmogifier.bitmaps"\
	"..\cmn\bitmaps\trapdoor\trapdoor.bitmaps"\
	"..\cmn\bitmaps\walker\walker.bitmaps"\
	"..\cmn\bitmaps\weight\weight.bitmaps"\
	"..\cmn\bitmaps\x_protection\x_protection.bitmaps"\
	"..\cmn\bitmaps\xit\xit.bitmaps"\
	"..\cmn\bitmaps\yeti\yeti.bitmaps"\
	"..\cmn\streams.h"\
	".\s_man.h"\
	".\stdafx.h"\
	".\xdata.h"\
	{$(INCLUDE)}"actual.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"physical.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	
NODEP_CPP_ACTUA=\
	"..\cmn\bitmaps\alien\gen_xpm\alien\alien_0.xpm"\
	"..\cmn\bitmaps\alien\gen_xpm\alien\alien_0b.xpm"\
	"..\cmn\bitmaps\alien\gen_xpm\alien\alien_0c.xpm"\
	"..\cmn\bitmaps\alien\gen_xpm\alien\alien_0d.xpm"\
	"..\cmn\bitmaps\alien\gen_xpm\alien\alien_0e.xpm"\
	"..\cmn\bitmaps\alien\gen_xpm\alien\alien_0f.xpm"\
	"..\cmn\bitmaps\alien\gen_xpm\alien\alien_10b.xpm"\
	"..\cmn\bitmaps\alien\gen_xpm\alien\alien_10c.xpm"\
	"..\cmn\bitmaps\alien\gen_xpm\alien\alien_10d.xpm"\
	"..\cmn\bitmaps\alien\gen_xpm\alien\alien_18.xpm"\
	"..\cmn\bitmaps\alien\gen_xpm\alien\alien_2.xpm"\
	"..\cmn\bitmaps\alien\gen_xpm\alien\alien_20b.xpm"\
	"..\cmn\bitmaps\alien\gen_xpm\alien\alien_20c.xpm"\
	"..\cmn\bitmaps\alien\gen_xpm\alien\alien_25.xpm"\
	"..\cmn\bitmaps\alien\gen_xpm\alien\alien_4.xpm"\
	"..\cmn\bitmaps\alien\gen_xpm\alien\alien_5.xpm"\
	"..\cmn\bitmaps\alien\gen_xpm\alien\alien_9.xpm"\
	"..\cmn\bitmaps\altar_of_sin\gen_xpm\altar_of_sin\altar_of_sin_4.xpm"\
	"..\cmn\bitmaps\altar_of_sin\gen_xpm\altar_of_sin\altar_of_sin_4b.xpm"\
	"..\cmn\bitmaps\altar_of_sin\gen_xpm\altar_of_sin\altar_of_sin_4c.xpm"\
	"..\cmn\bitmaps\auto_lancer\gen_xpm\auto_lancer\auto_lancer_4.xpm"\
	"..\cmn\bitmaps\blood\gen_xpm\blood\blood_25.xpm"\
	"..\cmn\bitmaps\blood\gen_xpm\blood\blood_39.xpm"\
	"..\cmn\bitmaps\blood\gen_xpm\blood\blood_4.xpm"\
	"..\cmn\bitmaps\blood\gen_xpm\blood\blood_40.xpm"\
	"..\cmn\bitmaps\blood\gen_xpm\blood\blood_9.xpm"\
	"..\cmn\bitmaps\bomb\gen_xpm\bomb\bomb_25.xpm"\
	"..\cmn\bitmaps\bomb\gen_xpm\bomb\bomb_25b.xpm"\
	"..\cmn\bitmaps\bomb\gen_xpm\bomb\bomb_26.xpm"\
	"..\cmn\bitmaps\bomb\gen_xpm\bomb\bomb_27.xpm"\
	"..\cmn\bitmaps\bomb\gen_xpm\bomb\bomb_28.xpm"\
	"..\cmn\bitmaps\bomb\gen_xpm\bomb\bomb_29.xpm"\
	"..\cmn\bitmaps\bomb\gen_xpm\bomb\bomb_30.xpm"\
	"..\cmn\bitmaps\bomb\gen_xpm\bomb\bomb_31.xpm"\
	"..\cmn\bitmaps\caffine\gen_xpm\caffine\caffine_4.xpm"\
	"..\cmn\bitmaps\chainsaw\gen_xpm\chainsaw\chainsaw_0.xpm"\
	"..\cmn\bitmaps\chainsaw\gen_xpm\chainsaw\chainsaw_0b.xpm"\
	"..\cmn\bitmaps\chainsaw\gen_xpm\chainsaw\chainsaw_0c.xpm"\
	"..\cmn\bitmaps\chainsaw\gen_xpm\chainsaw\chainsaw_1.xpm"\
	"..\cmn\bitmaps\chainsaw\gen_xpm\chainsaw\chainsaw_1b.xpm"\
	"..\cmn\bitmaps\chainsaw\gen_xpm\chainsaw\chainsaw_1c.xpm"\
	"..\cmn\bitmaps\chainsaw\gen_xpm\chainsaw\chainsaw_4.xpm"\
	"..\cmn\bitmaps\chainsaw\gen_xpm\chainsaw\chainsaw_4b.xpm"\
	"..\cmn\bitmaps\chainsaw\gen_xpm\chainsaw\chainsaw_5.xpm"\
	"..\cmn\bitmaps\chainsaw\gen_xpm\chainsaw\chainsaw_5b.xpm"\
	"..\cmn\bitmaps\chainsaw\gen_xpm\chainsaw\chainsaw_7.xpm"\
	"..\cmn\bitmaps\chainsaw\gen_xpm\chainsaw\chainsaw_7b.xpm"\
	"..\cmn\bitmaps\chicken\gen_xpm\chicken\chicken_0.xpm"\
	"..\cmn\bitmaps\chicken\gen_xpm\chicken\chicken_0b.xpm"\
	"..\cmn\bitmaps\chicken\gen_xpm\chicken\chicken_19.xpm"\
	"..\cmn\bitmaps\chicken\gen_xpm\chicken\chicken_19b.xpm"\
	"..\cmn\bitmaps\chicken\gen_xpm\chicken\chicken_2.xpm"\
	"..\cmn\bitmaps\chicken\gen_xpm\chicken\chicken_20b.xpm"\
	"..\cmn\bitmaps\chicken\gen_xpm\chicken\chicken_25.xpm"\
	"..\cmn\bitmaps\chicken\gen_xpm\chicken\chicken_26.xpm"\
	"..\cmn\bitmaps\chicken\gen_xpm\chicken\chicken_28.xpm"\
	"..\cmn\bitmaps\chicken\gen_xpm\chicken\chicken_38.xpm"\
	"..\cmn\bitmaps\chicken\gen_xpm\chicken\chicken_39.xpm"\
	"..\cmn\bitmaps\chicken\gen_xpm\chicken\chicken_4.xpm"\
	"..\cmn\bitmaps\chicken\gen_xpm\chicken\chicken_5.xpm"\
	"..\cmn\bitmaps\chicken\gen_xpm\chicken\chicken_5b.xpm"\
	"..\cmn\bitmaps\chopper_boy\gen_xpm\chopper_boy\chopper_boy_0.xpm"\
	"..\cmn\bitmaps\chopper_boy\gen_xpm\chopper_boy\chopper_boy_0b.xpm"\
	"..\cmn\bitmaps\chopper_boy\gen_xpm\chopper_boy\chopper_boy_0c.xpm"\
	"..\cmn\bitmaps\chopper_boy\gen_xpm\chopper_boy\chopper_boy_0d.xpm"\
	"..\cmn\bitmaps\chopper_boy\gen_xpm\chopper_boy\chopper_boy_0e.xpm"\
	"..\cmn\bitmaps\chopper_boy\gen_xpm\chopper_boy\chopper_boy_0f.xpm"\
	"..\cmn\bitmaps\chopper_boy\gen_xpm\chopper_boy\chopper_boy_2.xpm"\
	"..\cmn\bitmaps\chopper_boy\gen_xpm\chopper_boy\chopper_boy_21.xpm"\
	"..\cmn\bitmaps\chopper_boy\gen_xpm\chopper_boy\chopper_boy_21b.xpm"\
	"..\cmn\bitmaps\chopper_boy\gen_xpm\chopper_boy\chopper_boy_21c.xpm"\
	"..\cmn\bitmaps\chopper_boy\gen_xpm\chopper_boy\chopper_boy_21d.xpm"\
	"..\cmn\bitmaps\chopper_boy\gen_xpm\chopper_boy\chopper_boy_22.xpm"\
	"..\cmn\bitmaps\chopper_boy\gen_xpm\chopper_boy\chopper_boy_22b.xpm"\
	"..\cmn\bitmaps\chopper_boy\gen_xpm\chopper_boy\chopper_boy_22c.xpm"\
	"..\cmn\bitmaps\chopper_boy\gen_xpm\chopper_boy\chopper_boy_22d.xpm"\
	"..\cmn\bitmaps\chopper_boy\gen_xpm\chopper_boy\chopper_boy_25.xpm"\
	"..\cmn\bitmaps\chopper_boy\gen_xpm\chopper_boy\chopper_boy_2b.xpm"\
	"..\cmn\bitmaps\chopper_boy\gen_xpm\chopper_boy\chopper_boy_2c.xpm"\
	"..\cmn\bitmaps\chopper_boy\gen_xpm\chopper_boy\chopper_boy_2d.xpm"\
	"..\cmn\bitmaps\chopper_boy\gen_xpm\chopper_boy\chopper_boy_4.xpm"\
	"..\cmn\bitmaps\chopper_boy\gen_xpm\chopper_boy\chopper_boy_4b.xpm"\
	"..\cmn\bitmaps\chopper_boy\gen_xpm\chopper_boy\chopper_boy_4c.xpm"\
	"..\cmn\bitmaps\chopper_boy\gen_xpm\chopper_boy\chopper_boy_4d.xpm"\
	"..\cmn\bitmaps\chopper_boy\gen_xpm\chopper_boy\chopper_boy_5.xpm"\
	"..\cmn\bitmaps\cloak\gen_xpm\cloak\cloak_4.xpm"\
	"..\cmn\bitmaps\cloak\gen_xpm\cloak\cloak_4b.xpm"\
	"..\cmn\bitmaps\cloak\gen_xpm\cloak\cloak_4c.xpm"\
	"..\cmn\bitmaps\cloak\gen_xpm\cloak\cloak_4d.xpm"\
	"..\cmn\bitmaps\cloak\gen_xpm\cloak\cloak_4e.xpm"\
	"..\cmn\bitmaps\cloak\gen_xpm\cloak\cloak_4f.xpm"\
	"..\cmn\bitmaps\crack_pipe\gen_xpm\crack_pipe\crack_pipe_4.xpm"\
	"..\cmn\bitmaps\demon_summoner\gen_xpm\demon_summoner\demon_summoner_4.xpm"\
	"..\cmn\bitmaps\dog\gen_xpm\dog\dog_0.xpm"\
	"..\cmn\bitmaps\dog\gen_xpm\dog\dog_0b.xpm"\
	"..\cmn\bitmaps\dog\gen_xpm\dog\dog_19.xpm"\
	"..\cmn\bitmaps\dog\gen_xpm\dog\dog_19b.xpm"\
	"..\cmn\bitmaps\dog\gen_xpm\dog\dog_2.xpm"\
	"..\cmn\bitmaps\dog\gen_xpm\dog\dog_20b.xpm"\
	"..\cmn\bitmaps\dog\gen_xpm\dog\dog_25.xpm"\
	"..\cmn\bitmaps\dog\gen_xpm\dog\dog_26.xpm"\
	"..\cmn\bitmaps\dog\gen_xpm\dog\dog_28.xpm"\
	"..\cmn\bitmaps\dog\gen_xpm\dog\dog_38.xpm"\
	"..\cmn\bitmaps\dog\gen_xpm\dog\dog_39.xpm"\
	"..\cmn\bitmaps\dog\gen_xpm\dog\dog_4.xpm"\
	"..\cmn\bitmaps\dog\gen_xpm\dog\dog_5.xpm"\
	"..\cmn\bitmaps\dog\gen_xpm\dog\dog_5b.xpm"\
	"..\cmn\bitmaps\dog\gen_xpm\mutt\mutt_0.xpm"\
	"..\cmn\bitmaps\dog\gen_xpm\mutt\mutt_0b.xpm"\
	"..\cmn\bitmaps\dog\gen_xpm\mutt\mutt_19.xpm"\
	"..\cmn\bitmaps\dog\gen_xpm\mutt\mutt_19b.xpm"\
	"..\cmn\bitmaps\dog\gen_xpm\mutt\mutt_2.xpm"\
	"..\cmn\bitmaps\dog\gen_xpm\mutt\mutt_20b.xpm"\
	"..\cmn\bitmaps\dog\gen_xpm\mutt\mutt_25.xpm"\
	"..\cmn\bitmaps\dog\gen_xpm\mutt\mutt_26.xpm"\
	"..\cmn\bitmaps\dog\gen_xpm\mutt\mutt_28.xpm"\
	"..\cmn\bitmaps\dog\gen_xpm\mutt\mutt_38.xpm"\
	"..\cmn\bitmaps\dog\gen_xpm\mutt\mutt_39.xpm"\
	"..\cmn\bitmaps\dog\gen_xpm\mutt\mutt_4.xpm"\
	"..\cmn\bitmaps\dog\gen_xpm\mutt\mutt_5.xpm"\
	"..\cmn\bitmaps\dog\gen_xpm\mutt\mutt_5b.xpm"\
	"..\cmn\bitmaps\dog_whistle\gen_xpm\dog_whistle\dog_whistle_4.xpm"\
	"..\cmn\bitmaps\doppel\gen_xpm\doppelganger\doppelganger_4.xpm"\
	"..\cmn\bitmaps\doppel\gen_xpm\doppelganger\doppelganger_4b.xpm"\
	"..\cmn\bitmaps\dragon\gen_xpm\dragon\dragon_0.xpm"\
	"..\cmn\bitmaps\dragon\gen_xpm\dragon\dragon_25.xpm"\
	"..\cmn\bitmaps\dragon\gen_xpm\dragon\dragon_26.xpm"\
	"..\cmn\bitmaps\dragon\gen_xpm\dragon\dragon_26b.xpm"\
	"..\cmn\bitmaps\dragon\gen_xpm\dragon\dragon_26c.xpm"\
	"..\cmn\bitmaps\dragon\gen_xpm\dragon\dragon_26d.xpm"\
	"..\cmn\bitmaps\egg\gen_xpm\egg\egg_4.xpm"\
	"..\cmn\bitmaps\egg\gen_xpm\egg\egg_5.xpm"\
	"..\cmn\bitmaps\enforcer\gen_xpm\enforcer\enforcer_0.xpm"\
	"..\cmn\bitmaps\enforcer\gen_xpm\enforcer\enforcer_0b.xpm"\
	"..\cmn\bitmaps\enforcer\gen_xpm\enforcer\enforcer_0c.xpm"\
	"..\cmn\bitmaps\enforcer\gen_xpm\enforcer\enforcer_25.xpm"\
	"..\cmn\bitmaps\enforcer\gen_xpm\enforcer\enforcer_4.xpm"\
	"..\cmn\bitmaps\f_thrower\gen_xpm\flame_thrower\flame_thrower_4.xpm"\
	"..\cmn\bitmaps\feather\gen_xpm\blood\blood_25.xpm"\
	"..\cmn\bitmaps\feather\gen_xpm\blood\blood_39.xpm"\
	"..\cmn\bitmaps\feather\gen_xpm\blood\blood_4.xpm"\
	"..\cmn\bitmaps\feather\gen_xpm\blood\blood_40.xpm"\
	"..\cmn\bitmaps\feather\gen_xpm\blood\blood_9.xpm"\
	"..\cmn\bitmaps\fire\gen_xpm\fire\fire_4.xpm"\
	"..\cmn\bitmaps\fire_demon\gen_xpm\fire_demon\fire_demon_0.xpm"\
	"..\cmn\bitmaps\fire_demon\gen_xpm\fire_demon\fire_demon_0b.xpm"\
	"..\cmn\bitmaps\fire_demon\gen_xpm\fire_demon\fire_demon_2.xpm"\
	"..\cmn\bitmaps\fire_demon\gen_xpm\fire_demon\fire_demon_22.xpm"\
	"..\cmn\bitmaps\fire_demon\gen_xpm\fire_demon\fire_demon_25.xpm"\
	"..\cmn\bitmaps\fire_demon\gen_xpm\fire_demon\fire_demon_4.xpm"\
	"..\cmn\bitmaps\fire_demon\gen_xpm\fire_demon\fire_demon_5.xpm"\
	"..\cmn\bitmaps\fire_demon\gen_xpm\fire_demon\fire_demon_5b.xpm"\
	"..\cmn\bitmaps\fireball\gen_xpm\fireball\fireball_4.xpm"\
	"..\cmn\bitmaps\fireball\gen_xpm\fireball\fireball_4b.xpm"\
	"..\cmn\bitmaps\fireball\gen_xpm\fireball\fireball_4c.xpm"\
	"..\cmn\bitmaps\flag\gen_xpm\flag\flag_4.xpm"\
	"..\cmn\bitmaps\frog\gen_xpm\frog\frog_0.xpm"\
	"..\cmn\bitmaps\frog\gen_xpm\frog\frog_2.xpm"\
	"..\cmn\bitmaps\frog\gen_xpm\frog\frog_24.xpm"\
	"..\cmn\bitmaps\frog\gen_xpm\frog\frog_25.xpm"\
	"..\cmn\bitmaps\frog\gen_xpm\frog\frog_4.xpm"\
	"..\cmn\bitmaps\frog\gen_xpm\frog\frog_5.xpm"\
	"..\cmn\bitmaps\frog_gun\gen_xpm\frog_gun\frog_gun_4.xpm"\
	"..\cmn\bitmaps\frog_shell\gen_xpm\frog_shell\frog_shell_4.xpm"\
	"..\cmn\bitmaps\frog_shell\gen_xpm\frog_shell\frog_shell_4b.xpm"\
	"..\cmn\bitmaps\green_blood\gen_xpm\green_blood\green_blood_25.xpm"\
	"..\cmn\bitmaps\green_blood\gen_xpm\green_blood\green_blood_39.xpm"\
	"..\cmn\bitmaps\green_blood\gen_xpm\green_blood\green_blood_4.xpm"\
	"..\cmn\bitmaps\green_blood\gen_xpm\green_blood\green_blood_40.xpm"\
	"..\cmn\bitmaps\green_blood\gen_xpm\green_blood\green_blood_9.xpm"\
	"..\cmn\bitmaps\grenade\gen_xpm\grenade\grenade_4.xpm"\
	"..\cmn\bitmaps\grenade\gen_xpm\grenade\grenade_4b.xpm"\
	"..\cmn\bitmaps\grenade\gen_xpm\grenade\grenade_4c.xpm"\
	"..\cmn\bitmaps\grenade\gen_xpm\grenade\grenade_4d.xpm"\
	"..\cmn\bitmaps\grenades\gen_xpm\grenades\grenades_4.xpm"\
	"..\cmn\bitmaps\hero\gen_xpm\hero\hero_0.xpm"\
	"..\cmn\bitmaps\hero\gen_xpm\hero\hero_0b.xpm"\
	"..\cmn\bitmaps\hero\gen_xpm\hero\hero_0c.xpm"\
	"..\cmn\bitmaps\hero\gen_xpm\hero\hero_0d.xpm"\
	"..\cmn\bitmaps\hero\gen_xpm\hero\hero_0e.xpm"\
	"..\cmn\bitmaps\hero\gen_xpm\hero\hero_0f.xpm"\
	"..\cmn\bitmaps\hero\gen_xpm\hero\hero_10.xpm"\
	"..\cmn\bitmaps\hero\gen_xpm\hero\hero_18.xpm"\
	"..\cmn\bitmaps\hero\gen_xpm\hero\hero_2.xpm"\
	"..\cmn\bitmaps\hero\gen_xpm\hero\hero_20.xpm"\
	"..\cmn\bitmaps\hero\gen_xpm\hero\hero_20c.xpm"\
	"..\cmn\bitmaps\hero\gen_xpm\hero\hero_20d.xpm"\
	"..\cmn\bitmaps\hero\gen_xpm\hero\hero_20e.xpm"\
	"..\cmn\bitmaps\hero\gen_xpm\hero\hero_20f.xpm"\
	"..\cmn\bitmaps\hero\gen_xpm\hero\hero_25.xpm"\
	"..\cmn\bitmaps\hero\gen_xpm\hero\hero_26.xpm"\
	"..\cmn\bitmaps\hero\gen_xpm\hero\hero_28.xpm"\
	"..\cmn\bitmaps\hero\gen_xpm\hero\hero_32.xpm"\
	"..\cmn\bitmaps\hero\gen_xpm\hero\hero_38.xpm"\
	"..\cmn\bitmaps\hero\gen_xpm\hero\hero_39.xpm"\
	"..\cmn\bitmaps\hero\gen_xpm\hero\hero_4.xpm"\
	"..\cmn\bitmaps\hero\gen_xpm\hero\hero_5.xpm"\
	"..\cmn\bitmaps\hero\gen_xpm\hero\hero_9.xpm"\
	"..\cmn\bitmaps\home\gen_xpm\home\home_4.xpm"\
	"..\cmn\bitmaps\hugger\gen_xpm\green_hugger\green_hugger_0.xpm"\
	"..\cmn\bitmaps\hugger\gen_xpm\green_hugger\green_hugger_0b.xpm"\
	"..\cmn\bitmaps\hugger\gen_xpm\green_hugger\green_hugger_2.xpm"\
	"..\cmn\bitmaps\hugger\gen_xpm\green_hugger\green_hugger_20b.xpm"\
	"..\cmn\bitmaps\hugger\gen_xpm\green_hugger\green_hugger_25.xpm"\
	"..\cmn\bitmaps\hugger\gen_xpm\green_hugger\green_hugger_4.xpm"\
	"..\cmn\bitmaps\hugger\gen_xpm\green_hugger\green_hugger_5.xpm"\
	"..\cmn\bitmaps\hugger\gen_xpm\red_hugger\red_hugger_0.xpm"\
	"..\cmn\bitmaps\hugger\gen_xpm\red_hugger\red_hugger_0b.xpm"\
	"..\cmn\bitmaps\hugger\gen_xpm\red_hugger\red_hugger_2.xpm"\
	"..\cmn\bitmaps\hugger\gen_xpm\red_hugger\red_hugger_20b.xpm"\
	"..\cmn\bitmaps\hugger\gen_xpm\red_hugger\red_hugger_25.xpm"\
	"..\cmn\bitmaps\hugger\gen_xpm\red_hugger\red_hugger_4.xpm"\
	"..\cmn\bitmaps\hugger\gen_xpm\red_hugger\red_hugger_5.xpm"\
	"..\cmn\bitmaps\lance\gen_xpm\lance\lance_25.xpm"\
	"..\cmn\bitmaps\lance\gen_xpm\lance\lance_28.xpm"\
	"..\cmn\bitmaps\lancer\gen_xpm\lancer\lancer_4.xpm"\
	"..\cmn\bitmaps\laser\gen_xpm\laser\laser_25.xpm"\
	"..\cmn\bitmaps\laser\gen_xpm\laser\laser_28.xpm"\
	"..\cmn\bitmaps\launcher\gen_xpm\launcher\launcher_4.xpm"\
	"..\cmn\bitmaps\m_gun\gen_xpm\machine_gun\machine_gun_4.xpm"\
	"..\cmn\bitmaps\med_kit\gen_xpm\med_kit\med_kit_4.xpm"\
	"..\cmn\bitmaps\missile\gen_xpm\missile\missile_25.xpm"\
	"..\cmn\bitmaps\missile\gen_xpm\missile\missile_39.xpm"\
	"..\cmn\bitmaps\missile\gen_xpm\missile\missile_40.xpm"\
	"..\cmn\bitmaps\n_shield\gen_xpm\n_shield\n_shield_4.xpm"\
	"..\cmn\bitmaps\napalm\gen_xpm\napalm_grenade\napalm_grenade_4.xpm"\
	"..\cmn\bitmaps\napalm\gen_xpm\napalm_grenade\napalm_grenade_4b.xpm"\
	"..\cmn\bitmaps\napalm\gen_xpm\napalm_grenade\napalm_grenade_4c.xpm"\
	"..\cmn\bitmaps\napalm\gen_xpm\napalm_grenade\napalm_grenade_4d.xpm"\
	"..\cmn\bitmaps\napalms\gen_xpm\napalm_grenades\napalm_grenades_4.xpm"\
	"..\cmn\bitmaps\ninja\gen_xpm\ninja\ninja_0.xpm"\
	"..\cmn\bitmaps\ninja\gen_xpm\ninja\ninja_0b.xpm"\
	"..\cmn\bitmaps\ninja\gen_xpm\ninja\ninja_0c.xpm"\
	"..\cmn\bitmaps\ninja\gen_xpm\ninja\ninja_0d.xpm"\
	"..\cmn\bitmaps\ninja\gen_xpm\ninja\ninja_0e.xpm"\
	"..\cmn\bitmaps\ninja\gen_xpm\ninja\ninja_0f.xpm"\
	"..\cmn\bitmaps\ninja\gen_xpm\ninja\ninja_10b.xpm"\
	"..\cmn\bitmaps\ninja\gen_xpm\ninja\ninja_10c.xpm"\
	"..\cmn\bitmaps\ninja\gen_xpm\ninja\ninja_10d.xpm"\
	"..\cmn\bitmaps\ninja\gen_xpm\ninja\ninja_10e.xpm"\
	"..\cmn\bitmaps\ninja\gen_xpm\ninja\ninja_10f.xpm"\
	"..\cmn\bitmaps\ninja\gen_xpm\ninja\ninja_18.xpm"\
	"..\cmn\bitmaps\ninja\gen_xpm\ninja\ninja_2.xpm"\
	"..\cmn\bitmaps\ninja\gen_xpm\ninja\ninja_20.xpm"\
	"..\cmn\bitmaps\ninja\gen_xpm\ninja\ninja_20c.xpm"\
	"..\cmn\bitmaps\ninja\gen_xpm\ninja\ninja_20d.xpm"\
	"..\cmn\bitmaps\ninja\gen_xpm\ninja\ninja_20e.xpm"\
	"..\cmn\bitmaps\ninja\gen_xpm\ninja\ninja_20f.xpm"\
	"..\cmn\bitmaps\ninja\gen_xpm\ninja\ninja_22.xpm"\
	"..\cmn\bitmaps\ninja\gen_xpm\ninja\ninja_22b.xpm"\
	"..\cmn\bitmaps\ninja\gen_xpm\ninja\ninja_25.xpm"\
	"..\cmn\bitmaps\ninja\gen_xpm\ninja\ninja_26.xpm"\
	"..\cmn\bitmaps\ninja\gen_xpm\ninja\ninja_28.xpm"\
	"..\cmn\bitmaps\ninja\gen_xpm\ninja\ninja_2b.xpm"\
	"..\cmn\bitmaps\ninja\gen_xpm\ninja\ninja_2c.xpm"\
	"..\cmn\bitmaps\ninja\gen_xpm\ninja\ninja_2d.xpm"\
	"..\cmn\bitmaps\ninja\gen_xpm\ninja\ninja_32.xpm"\
	"..\cmn\bitmaps\ninja\gen_xpm\ninja\ninja_38.xpm"\
	"..\cmn\bitmaps\ninja\gen_xpm\ninja\ninja_39.xpm"\
	"..\cmn\bitmaps\ninja\gen_xpm\ninja\ninja_4.xpm"\
	"..\cmn\bitmaps\ninja\gen_xpm\ninja\ninja_5.xpm"\
	"..\cmn\bitmaps\ninja\gen_xpm\ninja\ninja_9.xpm"\
	"..\cmn\bitmaps\oil_droplet\gen_xpm\oil_droplet\oil_droplet_25.xpm"\
	"..\cmn\bitmaps\oil_droplet\gen_xpm\oil_droplet\oil_droplet_39.xpm"\
	"..\cmn\bitmaps\oil_droplet\gen_xpm\oil_droplet\oil_droplet_4.xpm"\
	"..\cmn\bitmaps\oil_droplet\gen_xpm\oil_droplet\oil_droplet_40.xpm"\
	"..\cmn\bitmaps\oil_droplet\gen_xpm\oil_droplet\oil_droplet_9.xpm"\
	"..\cmn\bitmaps\p_c_p\gen_xpm\PCP\PCP_4.xpm"\
	"..\cmn\bitmaps\pistol\gen_xpm\pistol\pistol_4.xpm"\
	"..\cmn\bitmaps\rock\gen_xpm\rock\rock_4.xpm"\
	"..\cmn\bitmaps\seal\gen_xpm\baby_seal\baby_seal_0.xpm"\
	"..\cmn\bitmaps\seal\gen_xpm\baby_seal\baby_seal_0b.xpm"\
	"..\cmn\bitmaps\seal\gen_xpm\baby_seal\baby_seal_25.xpm"\
	"..\cmn\bitmaps\seal\gen_xpm\baby_seal\baby_seal_4.xpm"\
	"..\cmn\bitmaps\shell\gen_xpm\shell\shell_4.xpm"\
	"..\cmn\bitmaps\star\gen_xpm\star\star_4.xpm"\
	"..\cmn\bitmaps\star\gen_xpm\star\star_4b.xpm"\
	"..\cmn\bitmaps\stars\gen_xpm\stars\stars_4.xpm"\
	"..\cmn\bitmaps\swap_shell\gen_xpm\swap_shell\swap_shell_4.xpm"\
	"..\cmn\bitmaps\swap_shell\gen_xpm\swap_shell\swap_shell_4b.xpm"\
	"..\cmn\bitmaps\swapper\gen_xpm\soul_swapper\soul_swapper_4.xpm"\
	"..\cmn\bitmaps\t_shield\gen_xpm\t_shield\t_shield_4.xpm"\
	"..\cmn\bitmaps\transmogifier\gen_xpm\transmogifier\transmogifier_4.xpm"\
	"..\cmn\bitmaps\trapdoor\gen_xpm\trapdoor\trapdoor_4.xpm"\
	"..\cmn\bitmaps\walker\gen_xpm\walker\walker_0.xpm"\
	"..\cmn\bitmaps\walker\gen_xpm\walker\walker_0b.xpm"\
	"..\cmn\bitmaps\walker\gen_xpm\walker\walker_0c.xpm"\
	"..\cmn\bitmaps\walker\gen_xpm\walker\walker_0d.xpm"\
	"..\cmn\bitmaps\walker\gen_xpm\walker\walker_18.xpm"\
	"..\cmn\bitmaps\walker\gen_xpm\walker\walker_2.xpm"\
	"..\cmn\bitmaps\walker\gen_xpm\walker\walker_20b.xpm"\
	"..\cmn\bitmaps\walker\gen_xpm\walker\walker_25.xpm"\
	"..\cmn\bitmaps\walker\gen_xpm\walker\walker_2b.xpm"\
	"..\cmn\bitmaps\walker\gen_xpm\walker\walker_4.xpm"\
	"..\cmn\bitmaps\walker\gen_xpm\walker\walker_4b.xpm"\
	"..\cmn\bitmaps\walker\gen_xpm\walker\walker_4c.xpm"\
	"..\cmn\bitmaps\walker\gen_xpm\walker\walker_4d.xpm"\
	"..\cmn\bitmaps\walker\gen_xpm\walker\walker_5.xpm"\
	"..\cmn\bitmaps\walker\gen_xpm\walker\walker_5b.xpm"\
	"..\cmn\bitmaps\walker\gen_xpm\walker\walker_5c.xpm"\
	"..\cmn\bitmaps\walker\gen_xpm\walker\walker_5d.xpm"\
	"..\cmn\bitmaps\weight\gen_xpm\weight\weight_4.xpm"\
	"..\cmn\bitmaps\xit\gen_xpm\exit\exit_4.xpm"\
	"..\cmn\bitmaps\yeti\gen_xpm\yeti\yeti_0.xpm"\
	"..\cmn\bitmaps\yeti\gen_xpm\yeti\yeti_0b.xpm"\
	"..\cmn\bitmaps\yeti\gen_xpm\yeti\yeti_0c.xpm"\
	"..\cmn\bitmaps\yeti\gen_xpm\yeti\yeti_0d.xpm"\
	"..\cmn\bitmaps\yeti\gen_xpm\yeti\yeti_10.xpm"\
	"..\cmn\bitmaps\yeti\gen_xpm\yeti\yeti_10b.xpm"\
	"..\cmn\bitmaps\yeti\gen_xpm\yeti\yeti_10c.xpm"\
	"..\cmn\bitmaps\yeti\gen_xpm\yeti\yeti_10d.xpm"\
	"..\cmn\bitmaps\yeti\gen_xpm\yeti\yeti_10e.xpm"\
	"..\cmn\bitmaps\yeti\gen_xpm\yeti\yeti_10f.xpm"\
	"..\cmn\bitmaps\yeti\gen_xpm\yeti\yeti_18.xpm"\
	"..\cmn\bitmaps\yeti\gen_xpm\yeti\yeti_2.xpm"\
	"..\cmn\bitmaps\yeti\gen_xpm\yeti\yeti_20b.xpm"\
	"..\cmn\bitmaps\yeti\gen_xpm\yeti\yeti_25.xpm"\
	"..\cmn\bitmaps\yeti\gen_xpm\yeti\yeti_4.xpm"\
	"..\cmn\bitmaps\yeti\gen_xpm\yeti\yeti_5.xpm"\
	"..\cmn\bitmaps\yeti\gen_xpm\yeti\yeti_9.xpm"\
	

"$(INTDIR)\actual.obj" : $(SOURCE) $(DEP_CPP_ACTUA) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_ACTUA=\
	"..\cmn\bitmaps\alien\alien.bitmaps"\
	"..\cmn\bitmaps\altar_of_sin\altar_of_sin.bitmaps"\
	"..\cmn\bitmaps\auto_lancer\auto_lancer.bitmaps"\
	"..\cmn\bitmaps\blood\blood.bitmaps"\
	"..\cmn\bitmaps\bomb\bomb.bitmaps"\
	"..\cmn\bitmaps\caffine\caffine.bitmaps"\
	"..\cmn\bitmaps\chainsaw\chainsaw.bitmaps"\
	"..\cmn\bitmaps\chicken\chicken.bitmaps"\
	"..\cmn\bitmaps\chopper_boy\chopper_boy.bitmaps"\
	"..\cmn\bitmaps\cloak\cloak.bitmaps"\
	"..\cmn\bitmaps\crack_pipe\crack_pipe.bitmaps"\
	"..\cmn\bitmaps\demon_summoner\demon_summoner.bitmaps"\
	"..\cmn\bitmaps\dog\dog.bitmaps"\
	"..\cmn\bitmaps\dog_whistle\dog_whistle.bitmaps"\
	"..\cmn\bitmaps\doppel\doppel.bitmaps"\
	"..\cmn\bitmaps\dragon\dragon.bitmaps"\
	"..\cmn\bitmaps\egg\egg.bitmaps"\
	"..\cmn\bitmaps\enforcer\enforcer.bitmaps"\
	"..\cmn\bitmaps\explosion\explosion.bitmaps"\
	"..\cmn\bitmaps\f_thrower\f_thrower.bitmaps"\
	"..\cmn\bitmaps\feather\feather.bitmaps"\
	"..\cmn\bitmaps\fire\fire.bitmaps"\
	"..\cmn\bitmaps\fire_demon\fire_demon.bitmaps"\
	"..\cmn\bitmaps\fire_explosion\fire_explosion.bitmaps"\
	"..\cmn\bitmaps\fireball\fireball.bitmaps"\
	"..\cmn\bitmaps\flag\flag.bitmaps"\
	"..\cmn\bitmaps\frog\frog.bitmaps"\
	"..\cmn\bitmaps\frog_gun\frog_gun.bitmaps"\
	"..\cmn\bitmaps\frog_shell\frog_shell.bitmaps"\
	"..\cmn\bitmaps\green_blood\green_blood.bitmaps"\
	"..\cmn\bitmaps\grenade\grenade.bitmaps"\
	"..\cmn\bitmaps\grenades\grenades.bitmaps"\
	"..\cmn\bitmaps\hero\hero.bitmaps"\
	"..\cmn\bitmaps\home\home.bitmaps"\
	"..\cmn\bitmaps\hugger\hugger.bitmaps"\
	"..\cmn\bitmaps\lance\lance.bitmaps"\
	"..\cmn\bitmaps\lancer\lancer.bitmaps"\
	"..\cmn\bitmaps\laser\laser.bitmaps"\
	"..\cmn\bitmaps\launcher\launcher.bitmaps"\
	"..\cmn\bitmaps\m_gun\m_gun.bitmaps"\
	"..\cmn\bitmaps\med_kit\med_kit.bitmaps"\
	"..\cmn\bitmaps\missile\missile.bitmaps"\
	"..\cmn\bitmaps\n_protection\n_protection.bitmaps"\
	"..\cmn\bitmaps\n_shield\n_shield.bitmaps"\
	"..\cmn\bitmaps\napalm\napalm.bitmaps"\
	"..\cmn\bitmaps\napalms\napalms.bitmaps"\
	"..\cmn\bitmaps\ninja\ninja.bitmaps"\
	"..\cmn\bitmaps\oil_droplet\oil_droplet.bitmaps"\
	"..\cmn\bitmaps\p_c_p\p_c_p.bitmaps"\
	"..\cmn\bitmaps\phys_mover\phys_mover.bitmaps"\
	"..\cmn\bitmaps\pistol\pistol.bitmaps"\
	"..\cmn\bitmaps\rock\rock.bitmaps"\
	"..\cmn\bitmaps\seal\seal.bitmaps"\
	"..\cmn\bitmaps\shell\shell.bitmaps"\
	"..\cmn\bitmaps\star\star.bitmaps"\
	"..\cmn\bitmaps\stars\stars.bitmaps"\
	"..\cmn\bitmaps\swap_shell\swap_shell.bitmaps"\
	"..\cmn\bitmaps\swapper\swapper.bitmaps"\
	"..\cmn\bitmaps\t_protection\t_protection.bitmaps"\
	"..\cmn\bitmaps\t_shield\t_shield.bitmaps"\
	"..\cmn\bitmaps\transmogifier\transmogifier.bitmaps"\
	"..\cmn\bitmaps\trapdoor\trapdoor.bitmaps"\
	"..\cmn\bitmaps\walker\walker.bitmaps"\
	"..\cmn\bitmaps\weight\weight.bitmaps"\
	"..\cmn\bitmaps\x_protection\x_protection.bitmaps"\
	"..\cmn\bitmaps\xit\xit.bitmaps"\
	"..\cmn\bitmaps\yeti\yeti.bitmaps"\
	"..\cmn\streams.h"\
	".\s_man.h"\
	".\stdafx.h"\
	".\xdata.h"\
	{$(INCLUDE)}"actual.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"physical.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	

"$(INTDIR)\actual.obj" : $(SOURCE) $(DEP_CPP_ACTUA) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\cmn\area.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_AREA_=\
	"..\cmn\streams.h"\
	".\stdafx.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"utils.h"\
	

"$(INTDIR)\area.obj" : $(SOURCE) $(DEP_CPP_AREA_) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_AREA_=\
	"..\cmn\streams.h"\
	".\stdafx.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"utils.h"\
	

"$(INTDIR)\area.obj" : $(SOURCE) $(DEP_CPP_AREA_) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\connectdlg.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_CONNE=\
	"..\cmn\streams.h"\
	".\connectdlg.h"\
	".\fileman.h"\
	".\firepal.h"\
	".\fogpal.h"\
	".\glowpal.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\sound.h"\
	".\stdafx.h"\
	".\ui.h"\
	".\xdata.h"\
	".\xevil.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"game_style.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"role.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"ui_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	{$(INCLUDE)}"xetp.h"\
	{$(INCLUDE)}"xetp_basic.h"\
	

"$(INTDIR)\connectdlg.obj" : $(SOURCE) $(DEP_CPP_CONNE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_CONNE=\
	"..\cmn\streams.h"\
	".\connectdlg.h"\
	".\fileman.h"\
	".\firepal.h"\
	".\fogpal.h"\
	".\glowpal.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\sound.h"\
	".\stdafx.h"\
	".\ui.h"\
	".\xdata.h"\
	".\xevil.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"game_style.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"role.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"ui_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	{$(INCLUDE)}"xetp.h"\
	{$(INCLUDE)}"xetp_basic.h"\
	

"$(INTDIR)\connectdlg.obj" : $(SOURCE) $(DEP_CPP_CONNE) "$(INTDIR)"


!ENDIF 

SOURCE=..\cmn\coord.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_COORD=\
	"..\cmn\streams.h"\
	".\stdafx.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"utils.h"\
	

"$(INTDIR)\coord.obj" : $(SOURCE) $(DEP_CPP_COORD) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_COORD=\
	"..\cmn\streams.h"\
	".\stdafx.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"utils.h"\
	

"$(INTDIR)\coord.obj" : $(SOURCE) $(DEP_CPP_COORD) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\difficulty.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_DIFFI=\
	"..\cmn\streams.h"\
	".\difficulty.h"\
	".\stdafx.h"\
	".\xevil.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"utils.h"\
	

"$(INTDIR)\difficulty.obj" : $(SOURCE) $(DEP_CPP_DIFFI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_DIFFI=\
	"..\cmn\streams.h"\
	".\difficulty.h"\
	".\stdafx.h"\
	".\xevil.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"utils.h"\
	

"$(INTDIR)\difficulty.obj" : $(SOURCE) $(DEP_CPP_DIFFI) "$(INTDIR)"


!ENDIF 

SOURCE=.\draw.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_DRAW_=\
	"..\cmn\streams.h"\
	".\firepal.h"\
	".\fogpal.h"\
	".\glowpal.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\stdafx.h"\
	".\ui.h"\
	".\xdata.h"\
	{$(INCLUDE)}"actual.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"game_style.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"physical.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"ui_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	

"$(INTDIR)\draw.obj" : $(SOURCE) $(DEP_CPP_DRAW_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_DRAW_=\
	"..\cmn\streams.h"\
	".\firepal.h"\
	".\fogpal.h"\
	".\glowpal.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\stdafx.h"\
	".\ui.h"\
	".\xdata.h"\
	{$(INCLUDE)}"actual.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"game_style.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"physical.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"ui_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	

"$(INTDIR)\draw.obj" : $(SOURCE) $(DEP_CPP_DRAW_) "$(INTDIR)"


!ENDIF 

SOURCE=.\fileman.cpp
DEP_CPP_FILEM=\
	".\fileman.h"\
	".\stdafx.h"\
	

"$(INTDIR)\fileman.obj" : $(SOURCE) $(DEP_CPP_FILEM) "$(INTDIR)"


SOURCE=.\firepal.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_FIREP=\
	"..\cmn\streams.h"\
	".\firepal.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\stdafx.h"\
	".\xdata.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"utils.h"\
	

"$(INTDIR)\firepal.obj" : $(SOURCE) $(DEP_CPP_FIREP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_FIREP=\
	"..\cmn\streams.h"\
	".\firepal.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\stdafx.h"\
	".\xdata.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"utils.h"\
	

"$(INTDIR)\firepal.obj" : $(SOURCE) $(DEP_CPP_FIREP) "$(INTDIR)"


!ENDIF 

SOURCE=.\fogpal.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_FOGPA=\
	"..\cmn\streams.h"\
	".\fogpal.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\stdafx.h"\
	".\xdata.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"utils.h"\
	

"$(INTDIR)\fogpal.obj" : $(SOURCE) $(DEP_CPP_FOGPA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_FOGPA=\
	"..\cmn\streams.h"\
	".\fogpal.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\stdafx.h"\
	".\xdata.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"utils.h"\
	

"$(INTDIR)\fogpal.obj" : $(SOURCE) $(DEP_CPP_FOGPA) "$(INTDIR)"


!ENDIF 

SOURCE=.\fsstatus.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_FSSTA=\
	"..\cmn\streams.h"\
	".\fsstatus.h"\
	".\s_man.h"\
	".\stdafx.h"\
	".\wheel.h"\
	".\xdata.h"\
	".\xevil.h"\
	".\xviewport.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	

"$(INTDIR)\fsstatus.obj" : $(SOURCE) $(DEP_CPP_FSSTA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_FSSTA=\
	"..\cmn\streams.h"\
	".\fsstatus.h"\
	".\s_man.h"\
	".\stdafx.h"\
	".\wheel.h"\
	".\xdata.h"\
	".\xevil.h"\
	".\xviewport.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	

"$(INTDIR)\fsstatus.obj" : $(SOURCE) $(DEP_CPP_FSSTA) "$(INTDIR)"


!ENDIF 

SOURCE=..\cmn\game.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_GAME_=\
	"..\cmn\streams.h"\
	".\fileman.h"\
	".\firepal.h"\
	".\fogpal.h"\
	".\glowpal.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\sound.h"\
	".\specialcontrols.h"\
	".\stdafx.h"\
	".\ui.h"\
	".\uiplayer.h"\
	".\uiserver.h"\
	".\xdata.h"\
	".\xevilserverstatus.h"\
	".\xviewport.h"\
	{$(INCLUDE)}"actual.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"game.h"\
	{$(INCLUDE)}"game_style.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"l_agreement.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"physical.h"\
	{$(INCLUDE)}"role.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"ui_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	{$(INCLUDE)}"xetp.h"\
	{$(INCLUDE)}"xetp_basic.h"\
	

"$(INTDIR)\game.obj" : $(SOURCE) $(DEP_CPP_GAME_) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_GAME_=\
	"..\cmn\streams.h"\
	".\fileman.h"\
	".\firepal.h"\
	".\fogpal.h"\
	".\glowpal.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\sound.h"\
	".\specialcontrols.h"\
	".\stdafx.h"\
	".\ui.h"\
	".\uiplayer.h"\
	".\uiserver.h"\
	".\xdata.h"\
	".\xevilserverstatus.h"\
	".\xviewport.h"\
	{$(INCLUDE)}"actual.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"game.h"\
	{$(INCLUDE)}"game_style.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"l_agreement.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"physical.h"\
	{$(INCLUDE)}"role.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"ui_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	{$(INCLUDE)}"xetp.h"\
	{$(INCLUDE)}"xetp_basic.h"\
	

"$(INTDIR)\game.obj" : $(SOURCE) $(DEP_CPP_GAME_) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\cmn\game_style.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_GAME_S=\
	"..\cmn\streams.h"\
	".\s_man.h"\
	".\stdafx.h"\
	".\xdata.h"\
	{$(INCLUDE)}"actual.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"game_style.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"physical.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	

"$(INTDIR)\game_style.obj" : $(SOURCE) $(DEP_CPP_GAME_S) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_GAME_S=\
	"..\cmn\streams.h"\
	".\s_man.h"\
	".\stdafx.h"\
	".\xdata.h"\
	{$(INCLUDE)}"actual.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"game_style.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"physical.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	

"$(INTDIR)\game_style.obj" : $(SOURCE) $(DEP_CPP_GAME_S) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\glowpal.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_GLOWP=\
	"..\cmn\streams.h"\
	".\glowpal.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\stdafx.h"\
	".\xdata.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"utils.h"\
	

"$(INTDIR)\glowpal.obj" : $(SOURCE) $(DEP_CPP_GLOWP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_GLOWP=\
	"..\cmn\streams.h"\
	".\glowpal.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\stdafx.h"\
	".\xdata.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"utils.h"\
	

"$(INTDIR)\glowpal.obj" : $(SOURCE) $(DEP_CPP_GLOWP) "$(INTDIR)"


!ENDIF 

SOURCE=..\cmn\id.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_ID_CP=\
	"..\cmn\streams.h"\
	".\stdafx.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"utils.h"\
	

"$(INTDIR)\id.obj" : $(SOURCE) $(DEP_CPP_ID_CP) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_ID_CP=\
	"..\cmn\streams.h"\
	".\stdafx.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"utils.h"\
	

"$(INTDIR)\id.obj" : $(SOURCE) $(DEP_CPP_ID_CP) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\cmn\intel.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_INTEL=\
	"..\cmn\streams.h"\
	".\fileman.h"\
	".\firepal.h"\
	".\fogpal.h"\
	".\glowpal.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\sound.h"\
	".\stdafx.h"\
	".\ui.h"\
	".\xdata.h"\
	{$(INCLUDE)}"actual.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"game_style.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"physical.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"ui_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	{$(INCLUDE)}"xetp.h"\
	{$(INCLUDE)}"xetp_basic.h"\
	

"$(INTDIR)\intel.obj" : $(SOURCE) $(DEP_CPP_INTEL) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_INTEL=\
	"..\cmn\streams.h"\
	".\fileman.h"\
	".\firepal.h"\
	".\fogpal.h"\
	".\glowpal.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\sound.h"\
	".\stdafx.h"\
	".\ui.h"\
	".\xdata.h"\
	{$(INCLUDE)}"actual.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"game_style.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"physical.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"ui_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	{$(INCLUDE)}"xetp.h"\
	{$(INCLUDE)}"xetp_basic.h"\
	

"$(INTDIR)\intel.obj" : $(SOURCE) $(DEP_CPP_INTEL) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\keyhitdlg.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_KEYHI=\
	".\keyhitdlg.h"\
	".\stdafx.h"\
	".\xevil.h"\
	

"$(INTDIR)\keyhitdlg.obj" : $(SOURCE) $(DEP_CPP_KEYHI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_KEYHI=\
	".\keyhitdlg.h"\
	".\stdafx.h"\
	".\xevil.h"\
	

"$(INTDIR)\keyhitdlg.obj" : $(SOURCE) $(DEP_CPP_KEYHI) "$(INTDIR)"


!ENDIF 

SOURCE=.\keysets.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_KEYSE=\
	".\keyhitdlg.h"\
	".\keysets.h"\
	".\stdafx.h"\
	".\xevil.h"\
	

"$(INTDIR)\keysets.obj" : $(SOURCE) $(DEP_CPP_KEYSE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_KEYSE=\
	".\keyhitdlg.h"\
	".\keysets.h"\
	".\stdafx.h"\
	".\xevil.h"\
	

"$(INTDIR)\keysets.obj" : $(SOURCE) $(DEP_CPP_KEYSE) "$(INTDIR)"


!ENDIF 

SOURCE=..\cmn\l_agreement.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_L_AGR=\
	".\stdafx.h"\
	{$(INCLUDE)}"l_agreement.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"utils.h"\
	

"$(INTDIR)\l_agreement.obj" : $(SOURCE) $(DEP_CPP_L_AGR) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_L_AGR=\
	".\stdafx.h"\
	{$(INCLUDE)}"l_agreement.h"\
	{$(INCLUDE)}"utils.h"\
	

"$(INTDIR)\l_agreement.obj" : $(SOURCE) $(DEP_CPP_L_AGR) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\cmn\locator.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_LOCAT=\
	"..\cmn\bitmaps\locator\locator.bitmaps"\
	"..\cmn\streams.h"\
	".\fileman.h"\
	".\firepal.h"\
	".\fogpal.h"\
	".\glowpal.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\sound.h"\
	".\stdafx.h"\
	".\ui.h"\
	".\xdata.h"\
	{$(INCLUDE)}"actual.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"game_style.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"physical.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"ui_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	
NODEP_CPP_LOCAT=\
	"..\cmn\bitmaps\locator\gen_xpm\locator\tick_0_0.xpm"\
	"..\cmn\bitmaps\locator\gen_xpm\locator\tick_0_7.xpm"\
	"..\cmn\bitmaps\locator\gen_xpm\locator\tick_1_0.xpm"\
	"..\cmn\bitmaps\locator\gen_xpm\locator\tick_1_7.xpm"\
	"..\cmn\bitmaps\locator\gen_xpm\locator\tick_2_0.xpm"\
	"..\cmn\bitmaps\locator\gen_xpm\locator\tick_2_7.xpm"\
	"..\cmn\bitmaps\locator\gen_xpm\locator\tick_3_0.xpm"\
	"..\cmn\bitmaps\locator\gen_xpm\locator\tick_3_7.xpm"\
	

"$(INTDIR)\locator.obj" : $(SOURCE) $(DEP_CPP_LOCAT) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_LOCAT=\
	"..\cmn\bitmaps\locator\locator.bitmaps"\
	"..\cmn\streams.h"\
	".\fileman.h"\
	".\firepal.h"\
	".\fogpal.h"\
	".\glowpal.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\sound.h"\
	".\stdafx.h"\
	".\ui.h"\
	".\xdata.h"\
	{$(INCLUDE)}"actual.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"game_style.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"physical.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"ui_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	

"$(INTDIR)\locator.obj" : $(SOURCE) $(DEP_CPP_LOCAT) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\optionsdlg.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_OPTIO=\
	"..\cmn\streams.h"\
	".\firepal.h"\
	".\fogpal.h"\
	".\glowpal.h"\
	".\optionsdlg.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\stdafx.h"\
	".\ui.h"\
	".\xdata.h"\
	".\xevil.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"game_style.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"role.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"ui_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	

"$(INTDIR)\optionsdlg.obj" : $(SOURCE) $(DEP_CPP_OPTIO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_OPTIO=\
	"..\cmn\streams.h"\
	".\firepal.h"\
	".\fogpal.h"\
	".\glowpal.h"\
	".\optionsdlg.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\stdafx.h"\
	".\ui.h"\
	".\xdata.h"\
	".\xevil.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"game_style.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"role.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"ui_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	

"$(INTDIR)\optionsdlg.obj" : $(SOURCE) $(DEP_CPP_OPTIO) "$(INTDIR)"


!ENDIF 

SOURCE=.\palmod.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_PALMO=\
	"..\cmn\streams.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\stdafx.h"\
	".\xdata.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"utils.h"\
	

"$(INTDIR)\palmod.obj" : $(SOURCE) $(DEP_CPP_PALMO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_PALMO=\
	"..\cmn\streams.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\stdafx.h"\
	".\xdata.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"utils.h"\
	

"$(INTDIR)\palmod.obj" : $(SOURCE) $(DEP_CPP_PALMO) "$(INTDIR)"


!ENDIF 

SOURCE=..\cmn\physical.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_PHYSI=\
	"..\cmn\bitmaps\transform\transform.bitmaps"\
	"..\cmn\streams.h"\
	".\s_man.h"\
	".\stdafx.h"\
	".\xdata.h"\
	{$(INCLUDE)}"actual.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"physical.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	

"$(INTDIR)\physical.obj" : $(SOURCE) $(DEP_CPP_PHYSI) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_PHYSI=\
	"..\cmn\bitmaps\transform\transform.bitmaps"\
	"..\cmn\streams.h"\
	".\s_man.h"\
	".\stdafx.h"\
	".\xdata.h"\
	{$(INCLUDE)}"actual.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"physical.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	

"$(INTDIR)\physical.obj" : $(SOURCE) $(DEP_CPP_PHYSI) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\cmn\role.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_ROLE_=\
	"..\cmn\neth.h"\
	"..\cmn\streams.h"\
	".\fileman.h"\
	".\firepal.h"\
	".\fogpal.h"\
	".\glowpal.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\sound.h"\
	".\stdafx.h"\
	".\ui.h"\
	".\xdata.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"game_style.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"physical.h"\
	{$(INCLUDE)}"role.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"ui_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	{$(INCLUDE)}"xetp.h"\
	{$(INCLUDE)}"xetp_basic.h"\
	

"$(INTDIR)\role.obj" : $(SOURCE) $(DEP_CPP_ROLE_) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_ROLE_=\
	"..\cmn\neth.h"\
	"..\cmn\streams.h"\
	".\fileman.h"\
	".\firepal.h"\
	".\fogpal.h"\
	".\glowpal.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\sound.h"\
	".\stdafx.h"\
	".\ui.h"\
	".\xdata.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"game_style.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"physical.h"\
	{$(INCLUDE)}"role.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"ui_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	{$(INCLUDE)}"xetp.h"\
	{$(INCLUDE)}"xetp_basic.h"\
	

"$(INTDIR)\role.obj" : $(SOURCE) $(DEP_CPP_ROLE_) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\runserverdlg.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_RUNSE=\
	"..\cmn\streams.h"\
	".\fileman.h"\
	".\firepal.h"\
	".\fogpal.h"\
	".\glowpal.h"\
	".\palmod.h"\
	".\runserverdlg.h"\
	".\s_man.h"\
	".\sound.h"\
	".\stdafx.h"\
	".\ui.h"\
	".\xdata.h"\
	".\xevil.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"game_style.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"role.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"ui_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	{$(INCLUDE)}"xetp.h"\
	{$(INCLUDE)}"xetp_basic.h"\
	

"$(INTDIR)\runserverdlg.obj" : $(SOURCE) $(DEP_CPP_RUNSE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_RUNSE=\
	"..\cmn\streams.h"\
	".\fileman.h"\
	".\firepal.h"\
	".\fogpal.h"\
	".\glowpal.h"\
	".\palmod.h"\
	".\runserverdlg.h"\
	".\s_man.h"\
	".\sound.h"\
	".\stdafx.h"\
	".\ui.h"\
	".\xdata.h"\
	".\xevil.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"game_style.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"role.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"ui_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	{$(INCLUDE)}"xetp.h"\
	{$(INCLUDE)}"xetp_basic.h"\
	

"$(INTDIR)\runserverdlg.obj" : $(SOURCE) $(DEP_CPP_RUNSE) "$(INTDIR)"


!ENDIF 

SOURCE=.\s_man.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_S_MAN=\
	"..\cmn\streams.h"\
	".\s_man.h"\
	".\stdafx.h"\
	".\xdata.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"utils.h"\
	

"$(INTDIR)\s_man.obj" : $(SOURCE) $(DEP_CPP_S_MAN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_S_MAN=\
	"..\cmn\streams.h"\
	".\s_man.h"\
	".\stdafx.h"\
	".\xdata.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"utils.h"\
	

"$(INTDIR)\s_man.obj" : $(SOURCE) $(DEP_CPP_S_MAN) "$(INTDIR)"


!ENDIF 

SOURCE=.\sound.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_SOUND=\
	"..\cmn\streams.h"\
	".\fileman.h"\
	".\firepal.h"\
	".\fogpal.h"\
	".\glowpal.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\sound.h"\
	".\stdafx.h"\
	".\ui.h"\
	".\xdata.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"game_style.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"physical.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"ui_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	

"$(INTDIR)\sound.obj" : $(SOURCE) $(DEP_CPP_SOUND) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_SOUND=\
	"..\cmn\streams.h"\
	".\fileman.h"\
	".\firepal.h"\
	".\fogpal.h"\
	".\glowpal.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\sound.h"\
	".\stdafx.h"\
	".\ui.h"\
	".\xdata.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"game_style.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"physical.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"ui_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	

"$(INTDIR)\sound.obj" : $(SOURCE) $(DEP_CPP_SOUND) "$(INTDIR)"


!ENDIF 

SOURCE=..\cmn\sound_cmn.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_SOUND_=\
	"..\cmn\bitmaps\sound_cmn\sound_cmn.bitmaps"\
	"..\cmn\streams.h"\
	".\stdafx.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"utils.h"\
	

"$(INTDIR)\sound_cmn.obj" : $(SOURCE) $(DEP_CPP_SOUND_) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_SOUND_=\
	"..\cmn\bitmaps\sound_cmn\sound_cmn.bitmaps"\
	"..\cmn\streams.h"\
	".\stdafx.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	

"$(INTDIR)\sound_cmn.obj" : $(SOURCE) $(DEP_CPP_SOUND_) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\specialcontrols.cpp
DEP_CPP_SPECI=\
	".\specialcontrols.h"\
	".\stdafx.h"\
	

"$(INTDIR)\specialcontrols.obj" : $(SOURCE) $(DEP_CPP_SPECI) "$(INTDIR)"


SOURCE=.\stdafx.cpp
DEP_CPP_STDAF=\
	".\stdafx.h"\
	

"$(INTDIR)\stdafx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"


SOURCE=.\storydlg.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_STORY=\
	".\stdafx.h"\
	".\storydlg.h"\
	".\xevil.h"\
	

"$(INTDIR)\storydlg.obj" : $(SOURCE) $(DEP_CPP_STORY) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_STORY=\
	".\stdafx.h"\
	".\storydlg.h"\
	".\xevil.h"\
	

"$(INTDIR)\storydlg.obj" : $(SOURCE) $(DEP_CPP_STORY) "$(INTDIR)"


!ENDIF 

SOURCE=..\cmn\streams.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_STREA=\
	"..\cmn\neth.h"\
	"..\cmn\streams.h"\
	".\fileman.h"\
	".\firepal.h"\
	".\fogpal.h"\
	".\glowpal.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\sound.h"\
	".\stdafx.h"\
	".\ui.h"\
	".\xdata.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"game_style.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"ui_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	{$(INCLUDE)}"xetp.h"\
	{$(INCLUDE)}"xetp_basic.h"\
	

"$(INTDIR)\streams.obj" : $(SOURCE) $(DEP_CPP_STREA) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_STREA=\
	"..\cmn\neth.h"\
	"..\cmn\streams.h"\
	".\fileman.h"\
	".\firepal.h"\
	".\fogpal.h"\
	".\glowpal.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\sound.h"\
	".\stdafx.h"\
	".\ui.h"\
	".\xdata.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"game_style.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"ui_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	{$(INCLUDE)}"xetp.h"\
	{$(INCLUDE)}"xetp_basic.h"\
	

"$(INTDIR)\streams.obj" : $(SOURCE) $(DEP_CPP_STREA) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\timedlg.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_TIMED=\
	".\stdafx.h"\
	".\timedlg.h"\
	".\xevil.h"\
	

"$(INTDIR)\timedlg.obj" : $(SOURCE) $(DEP_CPP_TIMED) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_TIMED=\
	".\stdafx.h"\
	".\timedlg.h"\
	".\xevil.h"\
	

"$(INTDIR)\timedlg.obj" : $(SOURCE) $(DEP_CPP_TIMED) "$(INTDIR)"


!ENDIF 

SOURCE=.\ui.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_UI_CP=\
	"..\cmn\streams.h"\
	".\fileman.h"\
	".\firepal.h"\
	".\fogpal.h"\
	".\glowpal.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\stdafx.h"\
	".\ui.h"\
	".\uiplayer.h"\
	".\xdata.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"game_style.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"physical.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"ui_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	

"$(INTDIR)\ui.obj" : $(SOURCE) $(DEP_CPP_UI_CP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_UI_CP=\
	"..\cmn\streams.h"\
	".\fileman.h"\
	".\firepal.h"\
	".\fogpal.h"\
	".\glowpal.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\stdafx.h"\
	".\ui.h"\
	".\uiplayer.h"\
	".\xdata.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"game_style.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"physical.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"ui_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	

"$(INTDIR)\ui.obj" : $(SOURCE) $(DEP_CPP_UI_CP) "$(INTDIR)"


!ENDIF 

SOURCE=..\cmn\ui_cmn.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_UI_CM=\
	"..\cmn\streams.h"\
	".\s_man.h"\
	".\stdafx.h"\
	".\xdata.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"ui_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	

"$(INTDIR)\ui_cmn.obj" : $(SOURCE) $(DEP_CPP_UI_CM) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_UI_CM=\
	"..\cmn\streams.h"\
	".\s_man.h"\
	".\stdafx.h"\
	".\xdata.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"ui_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	

"$(INTDIR)\ui_cmn.obj" : $(SOURCE) $(DEP_CPP_UI_CM) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\uiplayer.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_UIPLA=\
	"..\cmn\streams.h"\
	".\aboutdialog.h"\
	".\connectdlg.h"\
	".\difficulty.h"\
	".\fileman.h"\
	".\firepal.h"\
	".\fogpal.h"\
	".\fsstatus.h"\
	".\glowpal.h"\
	".\keysets.h"\
	".\optionsdlg.h"\
	".\palmod.h"\
	".\runserverdlg.h"\
	".\s_man.h"\
	".\sound.h"\
	".\stdafx.h"\
	".\storydlg.h"\
	".\ui.h"\
	".\uiplayer.h"\
	".\wheel.h"\
	".\xdata.h"\
	".\xviewport.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"game_style.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"l_agreement.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"physical.h"\
	{$(INCLUDE)}"role.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"ui_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	

"$(INTDIR)\uiplayer.obj" : $(SOURCE) $(DEP_CPP_UIPLA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_UIPLA=\
	"..\cmn\streams.h"\
	".\aboutdialog.h"\
	".\connectdlg.h"\
	".\difficulty.h"\
	".\fileman.h"\
	".\firepal.h"\
	".\fogpal.h"\
	".\fsstatus.h"\
	".\glowpal.h"\
	".\keysets.h"\
	".\optionsdlg.h"\
	".\palmod.h"\
	".\runserverdlg.h"\
	".\s_man.h"\
	".\sound.h"\
	".\stdafx.h"\
	".\storydlg.h"\
	".\ui.h"\
	".\uiplayer.h"\
	".\wheel.h"\
	".\xdata.h"\
	".\xviewport.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"game_style.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"l_agreement.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"physical.h"\
	{$(INCLUDE)}"role.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"ui_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	

"$(INTDIR)\uiplayer.obj" : $(SOURCE) $(DEP_CPP_UIPLA) "$(INTDIR)"


!ENDIF 

SOURCE=.\uiserver.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_UISER=\
	"..\cmn\streams.h"\
	".\aboutdialog.h"\
	".\firepal.h"\
	".\fogpal.h"\
	".\glowpal.h"\
	".\optionsdlg.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\specialcontrols.h"\
	".\stdafx.h"\
	".\ui.h"\
	".\uiserver.h"\
	".\xdata.h"\
	".\xevilserverstatus.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"game_style.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"ui_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	

"$(INTDIR)\uiserver.obj" : $(SOURCE) $(DEP_CPP_UISER) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_UISER=\
	"..\cmn\streams.h"\
	".\aboutdialog.h"\
	".\firepal.h"\
	".\fogpal.h"\
	".\glowpal.h"\
	".\optionsdlg.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\specialcontrols.h"\
	".\stdafx.h"\
	".\ui.h"\
	".\uiserver.h"\
	".\xdata.h"\
	".\xevilserverstatus.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"game_style.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"ui_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	

"$(INTDIR)\uiserver.obj" : $(SOURCE) $(DEP_CPP_UISER) "$(INTDIR)"


!ENDIF 

SOURCE=..\cmn\utils.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_UTILS=\
	"..\cmn\streams.h"\
	".\stdafx.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"utils.h"\
	

"$(INTDIR)\utils.obj" : $(SOURCE) $(DEP_CPP_UTILS) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_UTILS=\
	"..\cmn\streams.h"\
	".\stdafx.h"\
	{$(INCLUDE)}"utils.h"\
	

"$(INTDIR)\utils.obj" : $(SOURCE) $(DEP_CPP_UTILS) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\wheel.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_WHEEL=\
	"..\cmn\streams.h"\
	".\s_man.h"\
	".\stdafx.h"\
	".\wheel.h"\
	".\xdata.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	

"$(INTDIR)\wheel.obj" : $(SOURCE) $(DEP_CPP_WHEEL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_WHEEL=\
	"..\cmn\streams.h"\
	".\s_man.h"\
	".\stdafx.h"\
	".\wheel.h"\
	".\xdata.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	

"$(INTDIR)\wheel.obj" : $(SOURCE) $(DEP_CPP_WHEEL) "$(INTDIR)"


!ENDIF 

SOURCE=..\cmn\world.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_WORLD=\
	"..\cmn\bitmaps\world\world.bitmaps"\
	"..\cmn\streams.h"\
	".\s_man.h"\
	".\stdafx.h"\
	".\xdata.h"\
	{$(INCLUDE)}"actual.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"physical.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	
NODEP_CPP_WORLD=\
	"..\cmn\bitmaps\world\gen_xpm\world\background_0.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\background_1.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\background_10.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\background_11.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\background_12.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\background_13.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\background_2.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\background_3.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\background_4.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\background_5.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\background_6.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\background_7.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\background_8.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\background_9.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\block_0.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\block_1.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\block_10.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\block_11.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\block_12.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\block_13.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\block_14.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\block_15.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\block_16.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\block_17.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\block_18.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\block_19.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\block_2.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\block_20.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\block_21.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\block_3.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\block_4.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\block_5.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\block_6.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\block_7.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\block_8.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\block_9.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\door_0.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\door_1.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\door_2.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\door_3.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\door_4.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\door_5.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\door_6.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\door_7.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\door_8.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\door_9.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\mover_0.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\mover_1.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\mover_2.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\mover_3.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\mover_4.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\mover_square_0.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\mover_square_1.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\mover_square_2.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\mover_square_3.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\mover_square_4.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\mover_square_5.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\mover_square_6.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\mover_square_7.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\mover_square_8.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\mover_square_9.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\outside_0.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\outside_1.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\outside_2.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\outside_3.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\outside_4.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\outside_5.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\poster_0.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\poster_1.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\poster_10.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\poster_11.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\poster_12.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\poster_13.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\poster_2.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\poster_3.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\poster_4.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\poster_5.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\poster_6.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\poster_7.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\poster_8.xpm"\
	"..\cmn\bitmaps\world\gen_xpm\world\poster_9.xpm"\
	

"$(INTDIR)\world.obj" : $(SOURCE) $(DEP_CPP_WORLD) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_WORLD=\
	"..\cmn\bitmaps\world\world.bitmaps"\
	"..\cmn\streams.h"\
	".\s_man.h"\
	".\stdafx.h"\
	".\xdata.h"\
	{$(INCLUDE)}"actual.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"physical.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	

"$(INTDIR)\world.obj" : $(SOURCE) $(DEP_CPP_WORLD) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\xdata.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_XDATA=\
	"..\cmn\streams.h"\
	".\firepal.h"\
	".\fogpal.h"\
	".\glowpal.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\stdafx.h"\
	".\ui.h"\
	".\xdata.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"game_style.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"ui_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	

"$(INTDIR)\xdata.obj" : $(SOURCE) $(DEP_CPP_XDATA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_XDATA=\
	"..\cmn\streams.h"\
	".\firepal.h"\
	".\fogpal.h"\
	".\glowpal.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\stdafx.h"\
	".\ui.h"\
	".\xdata.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"game_style.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"ui_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	

"$(INTDIR)\xdata.obj" : $(SOURCE) $(DEP_CPP_XDATA) "$(INTDIR)"


!ENDIF 

SOURCE=..\cmn\xetp.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_XETP_=\
	"..\cmn\streams.h"\
	".\fileman.h"\
	".\firepal.h"\
	".\fogpal.h"\
	".\glowpal.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\sound.h"\
	".\stdafx.h"\
	".\ui.h"\
	".\xdata.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"game_style.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"physical.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"ui_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	{$(INCLUDE)}"xetp.h"\
	{$(INCLUDE)}"xetp_basic.h"\
	

"$(INTDIR)\xetp.obj" : $(SOURCE) $(DEP_CPP_XETP_) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_XETP_=\
	"..\cmn\streams.h"\
	".\fileman.h"\
	".\firepal.h"\
	".\fogpal.h"\
	".\glowpal.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\sound.h"\
	".\stdafx.h"\
	".\ui.h"\
	".\xdata.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"game_style.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"physical.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"ui_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	{$(INCLUDE)}"xetp.h"\
	{$(INCLUDE)}"xetp_basic.h"\
	

"$(INTDIR)\xetp.obj" : $(SOURCE) $(DEP_CPP_XETP_) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\cmn\xetp_basic.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_XETP_B=\
	"..\cmn\streams.h"\
	".\stdafx.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"xetp_basic.h"\
	

"$(INTDIR)\xetp_basic.obj" : $(SOURCE) $(DEP_CPP_XETP_B) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_XETP_B=\
	"..\cmn\streams.h"\
	".\stdafx.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"xetp_basic.h"\
	

"$(INTDIR)\xetp_basic.obj" : $(SOURCE) $(DEP_CPP_XETP_B) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\xevil.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_XEVIL=\
	"..\cmn\streams.h"\
	".\aboutdialog.h"\
	".\fileman.h"\
	".\firepal.h"\
	".\fogpal.h"\
	".\glowpal.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\sound.h"\
	".\stdafx.h"\
	".\timedlg.h"\
	".\ui.h"\
	".\xdata.h"\
	".\xevil.h"\
	{$(INCLUDE)}"actual.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"game.h"\
	{$(INCLUDE)}"game_style.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"l_agreement.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"physical.h"\
	{$(INCLUDE)}"role.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"ui_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	

"$(INTDIR)\xevil.obj" : $(SOURCE) $(DEP_CPP_XEVIL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_XEVIL=\
	"..\cmn\streams.h"\
	".\aboutdialog.h"\
	".\fileman.h"\
	".\firepal.h"\
	".\fogpal.h"\
	".\glowpal.h"\
	".\palmod.h"\
	".\s_man.h"\
	".\sound.h"\
	".\stdafx.h"\
	".\timedlg.h"\
	".\ui.h"\
	".\xdata.h"\
	".\xevil.h"\
	{$(INCLUDE)}"actual.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"game.h"\
	{$(INCLUDE)}"game_style.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"l_agreement.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"physical.h"\
	{$(INCLUDE)}"role.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"ui_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	

"$(INTDIR)\xevil.obj" : $(SOURCE) $(DEP_CPP_XEVIL) "$(INTDIR)"


!ENDIF 

SOURCE=.\xevil.rc
DEP_RSC_XEVIL_=\
	".\res\!dog0_ru.bmp"\
	".\res\27a.bmp"\
	".\res\27b.bmp"\
	".\res\39a.bmp"\
	".\res\aaanew.bmp"\
	".\res\aaanew1.bmp"\
	".\res\Aaanew7.bmp"\
	".\res\Aliennu.bmp"\
	".\res\altar2.bmp"\
	".\res\altar3.bmp"\
	".\res\arrow.cur"\
	".\res\Autolanc.bmp"\
	".\res\babyseal.mid"\
	".\res\Back.bmp"\
	".\res\backg.BMP"\
	".\res\bangbang.wav"\
	".\res\Barrow_r.bmp"\
	".\res\Bceagle.bmp"\
	".\res\Beat_dog.bmp"\
	".\res\bitmap3.bmp"\
	".\res\Bitmap6.bmp"\
	".\res\Blok2.bmp"\
	".\res\Blood25.bmp"\
	".\res\blood39.bmp"\
	".\res\blood4.bmp"\
	".\res\Blood40.bmp"\
	".\res\Blood9.bmp"\
	".\res\Bloodg25.bmp"\
	".\res\Bloodg39.bmp"\
	".\res\Bloodg4.bmp"\
	".\res\Bloodg40.bmp"\
	".\res\Bloodg9.bmp"\
	".\res\bmp00001.bmp"\
	".\res\Bmp00002.bmp"\
	".\res\Bmp00003.bmp"\
	".\res\Bmp00004.bmp"\
	".\res\bmp00005.bmp"\
	".\res\Bmp00006.bmp"\
	".\res\Bmp00007.bmp"\
	".\res\bmp00008.bmp"\
	".\res\bmp00009.bmp"\
	".\Res\Bmp00010.bmp"\
	".\res\bmp00011.bmp"\
	".\res\Bmp00012.bmp"\
	".\res\Bmp00013.bmp"\
	".\res\Bmp00014.bmp"\
	".\res\Bmp00015.bmp"\
	".\res\Bmp00016.bmp"\
	".\res\Bmp00017.bmp"\
	".\res\Bmp00018.bmp"\
	".\Res\Bmp00019.bmp"\
	".\res\Bmp00020.bmp"\
	".\res\Bmp00021.bmp"\
	".\res\Bmp00022.bmp"\
	".\Res\Bmp00023.bmp"\
	".\res\Bmp00024.bmp"\
	".\res\bmp00025.bmp"\
	".\res\Bmp00026.bmp"\
	".\res\Bmp00027.bmp"\
	".\res\Bmp00028.bmp"\
	".\res\bmp00029.bmp"\
	".\res\Bmp00030.bmp"\
	".\res\Bmp00031.bmp"\
	".\Res\Bmp00032.bmp"\
	".\res\bmp00033.bmp"\
	".\res\Bmp00034.bmp"\
	".\Res\Bmp00035.bmp"\
	".\res\bmp00036.bmp"\
	".\res\bmp00037.bmp"\
	".\res\bmp00038.bmp"\
	".\res\bmp00039.bmp"\
	".\res\Bmp00040.bmp"\
	".\res\Bmp00041.bmp"\
	".\res\Bmp00042.bmp"\
	".\res\Bmp00043.bmp"\
	".\res\bmp00044.bmp"\
	".\res\bmp00045.bmp"\
	".\res\bmp00046.bmp"\
	".\res\bmp00047.bmp"\
	".\res\bmp00048.bmp"\
	".\res\bmp00049.bmp"\
	".\res\bmp00050.bmp"\
	".\res\bmp00051.bmp"\
	".\res\bmp00052.bmp"\
	".\res\bmp00053.bmp"\
	".\res\Bmp00054.bmp"\
	".\res\Bmp00055.bmp"\
	".\res\Bmp00056.bmp"\
	".\res\Bmp00057.bmp"\
	".\res\bmp00058.bmp"\
	".\res\bmp00059.bmp"\
	".\res\bmp00062.bmp"\
	".\res\bmp00063.bmp"\
	".\res\bmp00064.bmp"\
	".\res\bmp00065.bmp"\
	".\res\bmp00066.bmp"\
	".\res\bmp00067.bmp"\
	".\res\bmp00069.bmp"\
	".\res\Bmp00074.bmp"\
	".\res\bmp00078.bmp"\
	".\res\Bmp00081.bmp"\
	".\res\Bmp00084.bmp"\
	".\res\Bmp00085.bmp"\
	".\res\Bmp00091.bmp"\
	".\res\Bmp00092.bmp"\
	".\res\Bmp00093.bmp"\
	".\res\Bmp00094.bmp"\
	".\res\Bmp00096.bmp"\
	".\res\Bmp00099.bmp"\
	".\res\Bmp00101.bmp"\
	".\res\Bmp00103.bmp"\
	".\res\Bmp00105.bmp"\
	".\res\Bmp00107.bmp"\
	".\res\Bmp00109.bmp"\
	".\res\Bmp00111.bmp"\
	".\res\Bmp00113.bmp"\
	".\res\Bmp00127.bmp"\
	".\res\Bmp00134.bmp"\
	".\res\Bmp00138.bmp"\
	".\res\Bmp00151.bmp"\
	".\res\Bmp00154.bmp"\
	".\res\breakdwn.wav"\
	".\res\Bshield.bmp"\
	".\res\Cat.bmp"\
	".\res\Chainsaw.bmp"\
	".\res\chainsw.wav"\
	".\res\chicken2.bmp"\
	".\res\chicken4.bmp"\
	".\res\chicken5.bmp"\
	".\res\chickend.bmp"\
	".\res\chk0.bmp"\
	".\res\chk0b.bmp"\
	".\res\chk0c.bmp"\
	".\res\Choppr0.bmp"\
	".\res\Choppr0b.bmp"\
	".\res\Choppr0c.bmp"\
	".\res\Choppr0d.bmp"\
	".\res\Choppr0e.bmp"\
	".\res\Choppr0f.bmp"\
	".\res\Choppr2.bmp"\
	".\res\Choppr25.bmp"\
	".\res\Choppr2b.bmp"\
	".\res\Choppr2c.bmp"\
	".\res\Choppr2d.bmp"\
	".\res\Choppr4.bmp"\
	".\res\Choppr4b.bmp"\
	".\res\Choppr4c.bmp"\
	".\res\Choppr4d.bmp"\
	".\res\Choppr5.bmp"\
	".\res\Chopprd.bmp"\
	".\res\Chopprdb.bmp"\
	".\res\Chopprdc.bmp"\
	".\res\Chopprdd.bmp"\
	".\res\Choppru.bmp"\
	".\res\Chopprub.bmp"\
	".\res\Choppruc.bmp"\
	".\res\Chopprud.bmp"\
	".\res\chpdeath.wav"\
	".\res\cloak.bmp"\
	".\res\cloak.wav"\
	".\res\cloak1.bmp"\
	".\res\cloak3.bmp"\
	".\res\cloak4.bmp"\
	".\res\cloak5.bmp"\
	".\res\crawl-forward.bmp"\
	".\res\crawl-right-1.bmp"\
	".\res\crawl-right-2.bmp"\
	".\res\crawl-right-3.bmp"\
	".\res\crawl-right-4.bmp"\
	".\res\crawl-right-5.bmp"\
	".\res\crawl-right-6.bmp"\
	".\res\death.wav"\
	".\res\Demonsum.bmp"\
	".\res\Dg5_a.bmp"\
	".\res\Dg5_b.bmp"\
	".\res\Dg_atd.bmp"\
	".\res\Dg_atu.bmp"\
	".\res\Displayb.bmp"\
	".\res\Dog_att0.bmp"\
	".\res\dogatt.wav"\
	".\res\dogdeath.wav"\
	".\res\Door1.bmp"\
	".\res\Doorbot.bmp"\
	".\res\Doortop.bmp"\
	".\res\doppel.bmp"\
	".\res\doppel.wav"\
	".\res\doppel1.bmp"\
	".\res\Dragonde.bmp"\
	".\res\Drg_1.bmp"\
	".\res\Drg_2.bmp"\
	".\res\Drg_bdy.bmp"\
	".\res\Drg_head.bmp"\
	".\res\Drgbite.bmp"\
	".\res\Drop_sch.bmp"\
	".\res\Droplift.bmp"\
	".\res\dwhistle.bmp"\
	".\res\Egg.bmp"\
	".\res\Egg_open.bmp"\
	".\res\Empty.bmp"\
	".\res\Endblok.bmp"\
	".\res\Enf_0a.bmp"\
	".\res\Enf_0b.bmp"\
	".\res\Enf_0c.bmp"\
	".\res\Enf_0d.bmp"\
	".\res\Enf_5.bmp"\
	".\res\explosion.wav"\
	".\res\feather2.bmp"\
	".\res\feather3.bmp"\
	".\res\feather4.bmp"\
	".\res\feather5.bmp"\
	".\res\feather9.bmp"\
	".\res\Fire.bmp"\
	".\res\fire.mid"\
	".\res\Fireball.bmp"\
	".\res\fireball.wav"\
	".\res\Flame2.bmp"\
	".\res\Flame3.bmp"\
	".\res\Frog0.bmp"\
	".\res\Frog2.bmp"\
	".\res\Frog4.bmp"\
	".\res\Frog5.bmp"\
	".\res\Frog_dea.bmp"\
	".\res\frog_she.bmp"\
	".\res\frogdeth.wav"\
	".\res\Froggun.bmp"\
	".\res\froggun.wav"\
	".\res\fwbackgr.bmp"\
	".\res\Garrow_r.bmp"\
	".\res\grenade1.bmp"\
	".\res\grenade3.bmp"\
	".\res\grenade4.bmp"\
	".\res\GRENADES.BMP"\
	".\res\Handtoha.bmp"\
	".\res\Happy.bmp"\
	".\res\Hblok.bmp"\
	".\res\Heir2.bmp"\
	".\res\Hell5.bmp"\
	".\res\Hell_1.bmp"\
	".\res\Henblok.bmp"\
	".\res\Hero0.bmp"\
	".\res\Hero0_at.bmp"\
	".\res\Hero0b.bmp"\
	".\res\Hero0c.bmp"\
	".\res\Hero0d.bmp"\
	".\res\Hero0e.bmp"\
	".\res\Hero0f.bmp"\
	".\res\Hero10.bmp"\
	".\res\Hero10b.bmp"\
	".\res\heroatt.wav"\
	".\res\hhero18c.bmp"\
	".\res\hhero18e.bmp"\
	".\res\hhero18f.bmp"\
	".\res\hhero18g.bmp"\
	".\res\hhero21_.bmp"\
	".\res\hialien0.bmp"\
	".\Res\Hialien1.bmp"\
	".\res\hialien2.bmp"\
	".\Res\hialien4.bmp"\
	".\res\hialien5.bmp"\
	".\Res\Hialtar.bmp"\
	".\res\hibomb0.bmp"\
	".\res\hibomb1.bmp"\
	".\res\hibomb2.bmp"\
	".\res\hibomb3.bmp"\
	".\res\hibomb4.bmp"\
	".\res\hibomb5.bmp"\
	".\res\hibomb6.bmp"\
	".\res\hibombbl.bmp"\
	".\res\Hiddog0.bmp"\
	".\res\Hiddog0_.bmp"\
	".\res\Hiddog0b.bmp"\
	".\res\Hiddog2.bmp"\
	".\res\Hiddog21.bmp"\
	".\res\Hiddog22.bmp"\
	".\res\Hiddog2_.bmp"\
	".\res\Hiddog5.bmp"\
	".\res\Hiddog5b.bmp"\
	".\res\Hiddogde.bmp"\
	".\res\Hidog0.bmp"\
	".\res\Hidog2.bmp"\
	".\res\Hidog21.bmp"\
	".\res\Hidog21b.bmp"\
	".\res\Hidog22.bmp"\
	".\res\Hidog22b.bmp"\
	".\res\Hidog2_a.bmp"\
	".\res\Hidogdea.bmp"\
	".\res\Hidoor2.bmp"\
	".\res\hifirede.bmp"\
	".\res\Hiflag.bmp"\
	".\res\Hihero19.bmp"\
	".\res\Hihero22.bmp"\
	".\res\hihero4.bmp"\
	".\res\Hihero5.bmp"\
	".\res\Hihero_d.bmp"\
	".\res\Hihome.bmp"\
	".\res\hihugger.bmp"\
	".\res\Hilaunch.bmp"\
	".\res\Himedkit.bmp"\
	".\res\Hininja1.bmp"\
	".\res\Hininja_.bmp"\
	".\res\Hipistol.bmp"\
	".\res\Hitextbl.bmp"\
	".\res\hive.mid"\
	".\res\Hiwalk18.bmp"\
	".\res\Hiwalk2b.bmp"\
	".\res\Hiwalk4.bmp"\
	".\res\Hiwalk4b.bmp"\
	".\res\Hiwalk4c.bmp"\
	".\res\Hiwalk4d.bmp"\
	".\res\Hiwalk5.bmp"\
	".\res\Hiwalk5b.bmp"\
	".\res\Hiwalk5c.bmp"\
	".\res\Hiwalk5d.bmp"\
	".\res\Hiwalk_0.bmp"\
	".\res\hiwalk_0b.bmp"\
	".\res\hiwalk_0c.bmp"\
	".\res\hiwalk_0d.bmp"\
	".\res\Hiwalk_2.bmp"\
	".\res\Hiwalk_d.bmp"\
	".\res\Hiwlk18b.bmp"\
	".\Res\Htrack.bmp"\
	".\res\idr_xevi.ico"\
	".\res\Ind1.bmp"\
	".\res\Indt2.bmp"\
	".\res\Jump0.bmp"\
	".\res\kill_par.bmp"\
	".\res\kkkill.mid"\
	".\res\Ladder.bmp"\
	".\res\Ladder2.bmp"\
	".\res\Lance_ho.bmp"\
	".\res\Lance_ve.bmp"\
	".\res\lancer.wav"\
	".\res\Lancer1.bmp"\
	".\res\laser.wav"\
	".\res\Lasers.bmp"\
	".\res\lemdeth.wav"\
	".\res\Logob5.bmp"\
	".\res\machgun.wav"\
	".\Res\Md2door1.bmp"\
	".\res\Md2door2.bmp"\
	".\res\Md2ladde.bmp"\
	".\Res\Md2updow.bmp"\
	".\res\md4backg.bmp"\
	".\res\md4door1.bmp"\
	".\res\Md4door2.bmp"\
	".\Res\Md4horiz.bmp"\
	".\res\Md4ladde.bmp"\
	".\res\md4outsi.bmp"\
	".\res\Md4updow.bmp"\
	".\res\Md4vertm.bmp"\
	".\res\md4wall.bmp"\
	".\res\md5backg.bmp"\
	".\res\md5door1.bmp"\
	".\res\md5door2.bmp"\
	".\res\md5horiz.bmp"\
	".\res\md5ladde.bmp"\
	".\res\md5mover.bmp"\
	".\res\md5outsi.bmp"\
	".\res\md5updow.bmp"\
	".\res\md5vertm.bmp"\
	".\res\md5wall.bmp"\
	".\res\mdoutsid.bmp"\
	".\res\Mgun1.bmp"\
	".\res\missile.wav"\
	".\res\Missle25.bmp"\
	".\res\Missle39.bmp"\
	".\res\Missle40.bmp"\
	".\res\Mover.bmp"\
	".\res\Moversq.bmp"\
	".\res\Moversqu.bmp"\
	".\res\Napalm1.bmp"\
	".\res\napalm2.bmp"\
	".\res\Napalm3.bmp"\
	".\res\Napalm4.bmp"\
	".\res\Napalms.bmp"\
	".\res\Newsong.mid"\
	".\res\Nightsky.mid"\
	".\res\Nin0.bmp"\
	".\res\Nin0b.bmp"\
	".\res\Nin0c.bmp"\
	".\res\Nin0d.bmp"\
	".\res\Nin0e.bmp"\
	".\res\Nin0f.bmp"\
	".\res\Nin5.bmp"\
	".\res\nin_kick0jmp.bmp"\
	".\res\nin_kick0low.bmp"\
	".\res\Nin_lo0.bmp"\
	".\res\Nin_lo0b.bmp"\
	".\res\Nin_lo0c.bmp"\
	".\res\Nin_lo0d.bmp"\
	".\res\Nin_lo0e.bmp"\
	".\res\Nin_lo0f.bmp"\
	".\res\ninatt_down.bmp"\
	".\res\Ninj0a.bmp"\
	".\res\Ninj0b.bmp"\
	".\res\Ninj0c.bmp"\
	".\res\Ninj0d.bmp"\
	".\res\ninjaatt.wav"\
	".\res\ninjadth.wav"\
	".\res\ninjump_down1.bmp"\
	".\res\ninjump_down2.bmp"\
	".\res\ninjump_up.bmp"\
	".\res\nlift.BMP"\
	".\res\None.bmp"\
	".\res\None1.bmp"\
	".\res\Oil25.bmp"\
	".\res\Oil39.bmp"\
	".\res\Oil4.bmp"\
	".\res\oil40.bmp"\
	".\res\Oil9.bmp"\
	".\res\Outa.bmp"\
	".\res\Outb.bmp"\
	".\res\Outside.bmp"\
	".\res\Pent.bmp"\
	".\res\Pepper1.bmp"\
	".\res\Pills.bmp"\
	".\res\pistol.wav"\
	".\res\Pyramid.bmp"\
	".\res\rarrow_r.bmp"\
	".\res\Rockhi.bmp"\
	".\res\Seal0.bmp"\
	".\res\Seal0b.bmp"\
	".\res\sealcenter.bmp"\
	".\res\sealdead.bmp"\
	".\res\Shell.bmp"\
	".\res\shield.wav"\
	".\res\Sider.bmp"\
	".\res\Skull.bmp"\
	".\res\Small.ico"\
	".\res\Squanch_.bmp"\
	".\res\Star.bmp"\
	".\res\Starb.bmp"\
	".\res\Stars.bmp"\
	".\res\stop.bmp"\
	".\res\Suicideb.bmp"\
	".\res\Swapper.bmp"\
	".\res\swapper.wav"\
	".\res\Sweetdar.mid"\
	".\res\Take_can.bmp"\
	".\res\Terraexm.mid"\
	".\res\transmog.wav"\
	".\res\trapopen.bmp"\
	".\res\troll0.bmp"\
	".\res\troll0b.bmp"\
	".\res\troll0c.bmp"\
	".\res\troll0d.bmp"\
	".\res\troll1.bmp"\
	".\res\troll18.bmp"\
	".\res\troll18b.bmp"\
	".\res\troll25.bmp"\
	".\res\troll4.bmp"\
	".\res\troll5.bmp"\
	".\Res\Updn.bmp"\
	".\res\Vertmove.bmp"\
	".\Res\Vtrack.bmp"\
	".\Res\Wall.bmp"\
	".\res\Weight2.bmp"\
	".\res\Wsqaure.bmp"\
	".\res\xevil.pal"\
	".\res\XEvil.rc2"\
	".\res\Xevil1.bmp"\
	".\res\xevil1.cur"\
	".\res\xit.bmp"\
	".\res\Yarrow_r.bmp"\
	".\res\Yshield.bmp"\
	".\res\Zeepeeg.mid"\
	

"$(INTDIR)\xevil.res" : $(SOURCE) $(DEP_RSC_XEVIL_) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\xevilserverstatus.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_XEVILS=\
	".\specialcontrols.h"\
	".\stdafx.h"\
	".\xevil.h"\
	".\xevilserverstatus.h"\
	

"$(INTDIR)\xevilserverstatus.obj" : $(SOURCE) $(DEP_CPP_XEVILS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_XEVILS=\
	".\specialcontrols.h"\
	".\stdafx.h"\
	".\xevil.h"\
	".\xevilserverstatus.h"\
	

"$(INTDIR)\xevilserverstatus.obj" : $(SOURCE) $(DEP_CPP_XEVILS) "$(INTDIR)"


!ENDIF 

SOURCE=.\xviewport.cpp

!IF  "$(CFG)" == "xevil - Win32 Release"

DEP_CPP_XVIEW=\
	"..\cmn\streams.h"\
	".\s_man.h"\
	".\stdafx.h"\
	".\xdata.h"\
	".\xevil.h"\
	".\xviewport.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"sys\types.h"\
	{$(INCLUDE)}"sys\utime.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	

"$(INTDIR)\xviewport.obj" : $(SOURCE) $(DEP_CPP_XVIEW) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "xevil - Win32 Debug"

DEP_CPP_XVIEW=\
	"..\cmn\streams.h"\
	".\s_man.h"\
	".\stdafx.h"\
	".\xdata.h"\
	".\xevil.h"\
	".\xviewport.h"\
	{$(INCLUDE)}"area.h"\
	{$(INCLUDE)}"coord.h"\
	{$(INCLUDE)}"id.h"\
	{$(INCLUDE)}"intel.h"\
	{$(INCLUDE)}"locator.h"\
	{$(INCLUDE)}"sound_cmn.h"\
	{$(INCLUDE)}"utils.h"\
	{$(INCLUDE)}"world.h"\
	

"$(INTDIR)\xviewport.obj" : $(SOURCE) $(DEP_CPP_XVIEW) "$(INTDIR)"


!ENDIF 


!ENDIF 

