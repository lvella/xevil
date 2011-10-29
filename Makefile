# 
# XEvil(TM) Copyright (C) 1994,2000 Steve Hardt and Michael Judge
# http://www.xevil.com
# satan@xevil.com
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or (at
# your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program, the file "gpl.txt"; if not, write to the Free
# Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA, or visit http://www.gnu.org.
#

# toplevel Makefile

DEPTH = .
include $(DEPTH)/config.mk


#### IF you get an error like:
#### "makefile:21 commands commence before first target.  Stop ####
#### chances are you unzipped the XEvil source without converting from 
#### Windows to UNIX line breaks.  You can Use "unzip -a", but 
#### see http://www.xevil.com/xevil/dev/compiling.html for an important note.
ALL_OBJS = $(OBJ_DIR)/role.o $(OBJ_DIR)/game.o $(OBJ_DIR)/world.o \
	$(OBJ_DIR)/physical.o $(OBJ_DIR)/actual.o \
	$(OBJ_DIR)/main.o $(OBJ_DIR)/intel.o \
	$(OBJ_DIR)/locator.o $(OBJ_DIR)/ui.o $(OBJ_DIR)/coord.o \
	$(OBJ_DIR)/area.o $(OBJ_DIR)/utils.o $(OBJ_DIR)/xdata.o \
	$(OBJ_DIR)/draw.o $(OBJ_DIR)/game_style.o \
	$(OBJ_DIR)/streams.o \
	$(OBJ_DIR)/xetp.o $(OBJ_DIR)/xetp_basic.o $(OBJ_DIR)/id.o \
	$(OBJ_DIR)/sound_cmn.o $(OBJ_DIR)/panel.o $(OBJ_DIR)/l_agreement.o \
	$(OBJ_DIR)/ui_cmn.o $(OBJ_DIR)/l_agreement_dlg.o $(OBJ_DIR)/viewport.o


xevil: $(OBJ_DIR)/xevil
#xevil: $(OBJ_DIR)/xevil$(VERSION).$(PCKG_NAME).tar.gz


# Build the xevil executable.  
# Make sure the $(STRIP) line is commented out for a debug build.  
# For a release build, it should be uncommented.
#
# Also look at DEBUG_OPT in config.mk for debug vs. release builds.
$(OBJ_DIR)/xevil::
	@if test ! -d $(OBJ_DIR); then \
		mkdir $(OBJ_DIR); \
	fi; 
	cd $(DEPTH)/cmn; $(MAKE)
	cd $(DEPTH)/x11; $(MAKE)
	$(CC) $(LINK_FLAGS) $(LINK_OPT) $(LIBS_DIRS) -o $(OBJ_DIR)/xevil $(ALL_OBJS) $(LIBS)
#	$(STRIP) $(OBJ_DIR)/xevil

# Could also include serverping in the distribution
$(OBJ_DIR)/xevil$(VERSION).$(PCKG_NAME).tar.gz::
	cp readme.txt $(OBJ_DIR)
	cp gpl.txt $(OBJ_DIR)
	cp -r instructions $(OBJ_DIR)
	(cd $(OBJ_DIR); tar cf xevil$(VERSION).$(PCKG_NAME).tar xevil readme.txt gpl.txt instructions)
	(cd $(OBJ_DIR); gzip -f xevil$(VERSION).$(PCKG_NAME).tar)

## Make a distribution file.
dist:
	tar chf xevil`date +%m.%d.%y`.tar $(FILES)
	gzip xevil`date +%m.%d.%y`.tar

## X11 and Win32 code in a zip file, using CRLF for text files
distzip:
	rm -f xevilsrc.zip
	zip -9 -r -l xevilsrc.zip $(FILES)
	zip -9 -r xevilsrc.zip $(WIN32_BINARY_FILES)
	zip -9 -r -l xevilsrc.zip $(WIN32_TEXT_FILES)


## Make a shadow tree for the XEvil source.
workdir:
	@if test ! -d $(WORK_DIR); then \
		mkdir $(WORK_DIR); \
	else \
		echo $(WORK_DIR) already exists; \
	fi; \
	for filee in $(FILES); do \
		/bin/rm -f $(WORK_DIR)/$$filee; \
		ln -s $(SRC_DIR)/$$filee $(WORK_DIR); \
	done

## Remove executables and all junk.
clean:
	/bin/rm -f $(OBJ_DIR)/* core */core
	rmdir $(OBJ_DIR)
#	/bin/rm -f $(TARGETS) $(OBJS) core test test.o xshow.o xshow

tildaclean:
	/bin/rm -f *~ */*~ */*/*~ */*/*/*~

#.SUFFIXES: .C .o
#.C.o: $*.C
#	$(CC) $(DEBUG_OPT) $(CFLAGS) $(INCL_DIRS) -o $*.o -c $*.C 


