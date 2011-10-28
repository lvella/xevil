REM Windows batch file to package up all the source code.

REM We always delete the old zip.
del xevilsrc.zip

IF "%1" == "-no_resources" goto noResources
IF "%1" == "-cmn_only" goto cmnOnly


REM Bitmaps, sounds, tools
zip -9 -r xevilsrc.zip win32\res\*
zip -9 -r xevilsrc.zip x11\bitmaps\*
zip -9 -r xevilsrc.zip x11\gen_xpm\*
zip -9 xevilsrc.zip win32\sounds\*


REM Everything else
:noResources
zip -9 xevilsrc.zip * -x *.zip -x *.exe
zip -9 -r xevilsrc.zip instructions\*
zip -9 -r xevilsrc.zip cmn\bitmaps\*
zip -9 xevilsrc.zip win32\*.cpp
zip -9 xevilsrc.zip win32\*.h
zip -9 xevilsrc.zip win32\*.mak
zip -9 xevilsrc.zip win32\*.mdp
zip -9 xevilsrc.zip win32\*.dsp
zip -9 xevilsrc.zip win32\*.lib
zip -9 xevilsrc.zip win32\*.rc
zip -9 xevilsrc.zip win32\*.pal
zip -9 xevilsrc.zip win32\*.cpp
zip -9 xevilsrc.zip win32\*.dsw
zip -9 xevilsrc.zip x11\*.cpp
zip -9 xevilsrc.zip x11\*.h
zip -9 xevilsrc.zip x11\*.c
zip -9 xevilsrc.zip x11\makefile
zip -9 xevilsrc.zip x11\app-defaults\*


REM Only the common files
:cmnOnly
zip -9 xevilsrc.zip cmn\*.cpp
zip -9 xevilsrc.zip cmn\*.h
zip -9 xevilsrc.zip cmn\makefile
