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

# x11 directory Makefile

DEPTH = ..
include $(DEPTH)/config.mk

OBJS = $(OBJ_DIR)/xdata.o $(OBJ_DIR)/panel.o $(OBJ_DIR)/l_agreement_dlg.o \
       $(OBJ_DIR)/viewport.o \
       $(OBJ_DIR)/ui.o \
       $(OBJ_DIR)/draw.o $(OBJ_DIR)/main.o
ALL_INCL_DIRS = -I$(DEPTH)/cmn -I$(DEPTH)/x11 -I$(DEPTH)/x11/bitmaps $(INCL_DIRS)
ALL_CFLAGS = -DX11=1 $(CFLAGS) 

xevil: $(OBJS) $(OBJ_DIR)/serverping 
	@echo "Directory x11 finished."


#### XEvil modules
$(OBJ_DIR)/ui.o: ui.cpp ../cmn/utils.h ../cmn/coord.h ../cmn/area.h \
	../cmn/world.h ../cmn/id.h ../cmn/intel.h ../cmn/physical.h \
	../cmn/locator.h ui.h ../cmn/bitmaps/ui/ui.bitmaps ../cmn/game_style.h \
	xdata.h sound.h viewport.h
	$(CC) $(DEBUG_OPT) $(ALL_CFLAGS) $(ALL_INCL_DIRS) -o $(OBJ_DIR)/ui.o -c ui.cpp 

$(OBJ_DIR)/xdata.o: xdata.cpp ../cmn/utils.h xdata.h ../cmn/world.h ../cmn/locator.h
	$(CC) $(DEBUG_OPT) $(ALL_CFLAGS) $(ALL_INCL_DIRS) -o $(OBJ_DIR)/xdata.o -c xdata.cpp

$(OBJ_DIR)/panel.o: panel.cpp ../cmn/utils.h xdata.h
	$(CC) $(DEBUG_OPT) $(ALL_CFLAGS) $(ALL_INCL_DIRS) -o $(OBJ_DIR)/panel.o -c panel.cpp

$(OBJ_DIR)/viewport.o: viewport.cpp ../cmn/utils.h ../cmn/locator.h ../cmn/physical.h ../cmn/coord.h ../cmn/game_style.h viewport.h panel.h
	$(CC) $(DEBUG_OPT) $(ALL_CFLAGS) $(ALL_INCL_DIRS) -o $(OBJ_DIR)/viewport.o -c viewport.cpp

$(OBJ_DIR)/l_agreement_dlg.o: l_agreement_dlg.cpp ../cmn/utils.h xdata.h \
panel.h ../cmn/l_agreement.h
	$(CC) $(DEBUG_OPT) $(ALL_CFLAGS) $(ALL_INCL_DIRS) -o $(OBJ_DIR)/l_agreement_dlg.o -c l_agreement_dlg.cpp

$(OBJ_DIR)/draw.o: draw.cpp ../cmn/utils.h xdata.h ../cmn/coord.h ../cmn/actual.h ../cmn/locator.h ../cmn/world.h ../cmn/physical.h
	$(CC) $(DEBUG_OPT) $(ALL_CFLAGS) $(ALL_INCL_DIRS) -o $(OBJ_DIR)/draw.o -c draw.cpp

# NOTE: Does not need actual.h
$(OBJ_DIR)/main.o: main.cpp ../cmn/utils.h ../cmn/coord.h ../cmn/area.h \
	../cmn/world.h ../cmn/id.h ../cmn/intel.h \
	../cmn/physical.h ../cmn/locator.h ui.h ../cmn/game.h ../cmn/neth.h xdata.h
	$(CC) $(DEBUG_OPT) $(ALL_CFLAGS) $(ALL_INCL_DIRS) -o $(OBJ_DIR)/main.o -c main.cpp 



### Serverping utility program.
### serverping allows you to remotely check the status of an XEvil server.
$(OBJ_DIR)/serverping.o: serverping.cpp ../cmn/utils.h ../cmn/streams.h \
    ../cmn/xetp.h ../cmn/neth.h
	$(CC) $(DEBUG_OPT) $(ALL_CFLAGS) $(ALL_INCL_DIRS) -o $(OBJ_DIR)/serverping.o -c serverping.cpp

$(OBJ_DIR)/serverping:: $(OBJ_DIR)/serverping.o
	$(CC) $(LINK_FLAGS) $(LINK_OPT) $(LIBS_DIRS) -o $(OBJ_DIR)/serverping $(OBJ_DIR)/serverping.o $(OBJ_DIR)/utils.o $(OBJ_DIR)/xetp_basic.o $(OBJ_DIR)/streams.o $(OBJ_DIR)/id.o $(LIBS)
#	$(STRIP) $(OBJ_DIR)/serverping


