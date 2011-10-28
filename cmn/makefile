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

# cmn directory Makefile

DEPTH = ..
include $(DEPTH)/config.mk

OBJS =	\
	$(OBJ_DIR)/actual.o \
	$(OBJ_DIR)/area.o \
	$(OBJ_DIR)/role.o \
	$(OBJ_DIR)/utils.o \
	$(OBJ_DIR)/streams.o $(OBJ_DIR)/xetp_basic.o $(OBJ_DIR)/xetp.o \
	$(OBJ_DIR)/locator.o \
	$(OBJ_DIR)/world.o \
	$(OBJ_DIR)/ui_cmn.o \
	$(OBJ_DIR)/game.o $(OBJ_DIR)/physical.o \
	$(OBJ_DIR)/intel.o \
	$(OBJ_DIR)/coord.o $(OBJ_DIR)/game_style.o \
	$(OBJ_DIR)/id.o $(OBJ_DIR)/sound_cmn.o $(OBJ_DIR)/l_agreement.o	


ALL_INCL_DIRS = -I$(DEPTH)/cmn -I$(DEPTH)/x11 -I$(DEPTH)/x11/bitmaps $(INCL_DIRS)
ALL_CFLAGS = -DX11=1 $(CFLAGS)

xevil: $(OBJS)
	@echo "Directory cmn finished."


#### XEvil modules
$(OBJ_DIR)/utils.o: utils.cpp utils.h
	$(CC) $(DEBUG_OPT) $(ALL_CFLAGS) $(ALL_INCL_DIRS) -o $(OBJ_DIR)/utils.o -c utils.cpp 

$(OBJ_DIR)/coord.o: coord.cpp utils.h coord.h streams.h
	$(CC) $(DEBUG_OPT) $(ALL_CFLAGS) $(ALL_INCL_DIRS) -o $(OBJ_DIR)/coord.o -c coord.cpp 

$(OBJ_DIR)/area.o: area.cpp utils.h coord.h area.h streams.h
	$(CC) $(DEBUG_OPT) $(ALL_CFLAGS) $(ALL_INCL_DIRS) -o $(OBJ_DIR)/area.o -c area.cpp 

$(OBJ_DIR)/world.o: world.cpp utils.h coord.h area.h world.h locator.h \
	bitmaps/world/world.bitmaps ../x11/xdata.h actual.h streams.h
	$(CC) $(DEBUG_OPT) $(ALL_CFLAGS) $(ALL_INCL_DIRS) -o $(OBJ_DIR)/world.o -c world.cpp 

$(OBJ_DIR)/intel.o: intel.cpp utils.h coord.h area.h world.h id.h intel.h \
    actual.h streams.h \
	locator.h physical.h ../x11/xdata.h xetp.h
	$(CC) $(DEBUG_OPT) $(ALL_CFLAGS) $(ALL_INCL_DIRS) -o $(OBJ_DIR)/intel.o -c intel.cpp

$(OBJ_DIR)/locator.o: locator.cpp utils.h coord.h area.h world.h id.h intel.h \
physical.h locator.h actual.h ../x11/xdata.h ../x11/sound.h streams.h
	$(CC) $(DEBUG_OPT) $(ALL_CFLAGS) $(ALL_INCL_DIRS) -o $(OBJ_DIR)/locator.o -c locator.cpp

$(OBJ_DIR)/physical.o: physical.cpp utils.h coord.h area.h world.h id.h intel.h \
	locator.h physical.h ../x11/xdata.h streams.h
	$(CC) $(DEBUG_OPT) $(ALL_CFLAGS) $(ALL_INCL_DIRS) -o $(OBJ_DIR)/physical.o -c physical.cpp 

$(OBJ_DIR)/actual.o: actual.cpp utils.h coord.h area.h world.h id.h intel.h \
	locator.h physical.h actual.h ../x11/xdata.h streams.h \
	bitmaps/altar_of_sin/altar_of_sin.bitmaps bitmaps/bomb/bomb.bitmaps \
	bitmaps/chainsaw/chainsaw.bitmaps bitmaps/enforcer/enforcer.bitmaps \
	bitmaps/f_thrower/f_thrower.bitmaps bitmaps/fireball/fireball.bitmaps \
	bitmaps/frog/frog.bitmaps bitmaps/frog_gun/frog_gun.bitmaps \
	bitmaps/frog_shell/frog_shell.bitmaps bitmaps/hero/hero.bitmaps \
	bitmaps/lance/lance.bitmaps bitmaps/lancer/lancer.bitmaps \
	bitmaps/laser/laser.bitmaps \
	bitmaps/m_gun/m_gun.bitmaps bitmaps/med_kit/med_kit.bitmaps \
	bitmaps/ninja/ninja.bitmaps bitmaps/pistol/pistol.bitmaps \
	bitmaps/rock/rock.bitmaps bitmaps/shell/shell.bitmaps \
	bitmaps/swap_shell/swap_shell.bitmaps bitmaps/swapper/swapper.bitmaps \
	bitmaps/alien/alien.bitmaps \
	bitmaps/weight/weight.bitmaps \
	bitmaps/launcher/launcher.bitmaps \
	bitmaps/missile/missile.bitmaps bitmaps/grenades/grenades.bitmaps \
	bitmaps/grenade/grenade.bitmaps \
	bitmaps/chopper_boy/chopper_boy.bitmaps \
	bitmaps/n_protection/n_protection.bitmaps \
	bitmaps/n_shield/n_shield.bitmaps \
	bitmaps/t_protection/t_protection.bitmaps \
	bitmaps/t_shield/t_shield.bitmaps \
	bitmaps/transmogifier/transmogifier.bitmaps bitmaps/xit/xit.bitmaps \
	bitmaps/flag/flag.bitmaps bitmaps/doppel/doppel.bitmaps \
	bitmaps/home/home.bitmaps bitmaps/trapdoor/trapdoor.bitmaps \
	bitmaps/fire_demon/fire_demon.bitmaps \
	bitmaps/stars/stars.bitmaps bitmaps/star/star.bitmaps \
	bitmaps/walker/walker.bitmaps bitmaps/hugger/hugger.bitmaps \
	bitmaps/dog/dog.bitmaps bitmaps/dog_whistle/dog_whistle.bitmaps \
	bitmaps/dragon/dragon.bitmaps \
	bitmaps/blood/blood.bitmaps bitmaps/green_blood/green_blood.bitmaps \
	bitmaps/caffine/caffine.bitmaps \
	bitmaps/oil_droplet/oil_droplet.bitmaps bitmaps/p_c_p/p_c_p.bitmaps \
    bitmaps/seal/seal.bitmaps
	$(CC) $(DEBUG_OPT) $(ALL_CFLAGS) $(ALL_INCL_DIRS) -o $(OBJ_DIR)/actual.o -c actual.cpp 

$(OBJ_DIR)/game.o: game.cpp utils.h coord.h area.h world.h id.h intel.h \
	physical.h actual.h locator.h game_style.h role.h ../x11/ui.h game.h \
	../x11/sound.h ../x11/xdata.h ../x11/viewport.h
	$(CC) $(DEBUG_OPT) $(ALL_CFLAGS) $(ALL_INCL_DIRS) -o $(OBJ_DIR)/game.o -c game.cpp 

$(OBJ_DIR)/game_style.o: game_style.cpp game_style.h utils.h coord.h world.h locator.h physical.h actual.h 
	$(CC) $(DEBUG_OPT) $(ALL_CFLAGS) $(ALL_INCL_DIRS) -o $(OBJ_DIR)/game_style.o -c game_style.cpp

$(OBJ_DIR)/role.o: role.cpp role.h streams.h utils.h xetp.h locator.h world.h intel.h coord.h physical.h neth.h game_style.h
	$(CC) $(DEBUG_OPT) $(ALL_CFLAGS) $(ALL_INCL_DIRS) -o $(OBJ_DIR)/role.o -c role.cpp

$(OBJ_DIR)/streams.o: streams.cpp utils.h streams.h neth.h
	$(CC) $(DEBUG_OPT) $(ALL_CFLAGS) $(ALL_INCL_DIRS) -o $(OBJ_DIR)/streams.o -c streams.cpp

$(OBJ_DIR)/xetp.o: xetp.cpp utils.h xetp.h xetp_basic.h streams.h intel.h physical.h world.h locator.h
	$(CC) $(DEBUG_OPT) $(ALL_CFLAGS) $(ALL_INCL_DIRS) -o $(OBJ_DIR)/xetp.o -c xetp.cpp

$(OBJ_DIR)/xetp_basic.o: xetp_basic.cpp utils.h xetp_basic.h streams.h 
	$(CC) $(DEBUG_OPT) $(ALL_CFLAGS) $(ALL_INCL_DIRS) -o $(OBJ_DIR)/xetp_basic.o -c xetp_basic.cpp

$(OBJ_DIR)/id.o: id.cpp id.h utils.h streams.h
	$(CC) $(DEBUG_OPT) $(ALL_CFLAGS) $(ALL_INCL_DIRS) -o $(OBJ_DIR)/id.o -c id.cpp

$(OBJ_DIR)/sound_cmn.o: sound_cmn.cpp sound_cmn.h utils.h streams.h
	$(CC) $(DEBUG_OPT) $(ALL_CFLAGS) $(ALL_INCL_DIRS) -o $(OBJ_DIR)/sound_cmn.o -c sound_cmn.cpp

$(OBJ_DIR)/ui_cmn.o: ui_cmn.cpp ui_cmn.h utils.h
	$(CC) $(DEBUG_OPT) $(ALL_CFLAGS) $(ALL_INCL_DIRS) -o $(OBJ_DIR)/ui_cmn.o -c ui_cmn.cpp

$(OBJ_DIR)/l_agreement.o: l_agreement.cpp l_agreement.h utils.h ../x11/xdata.h
	$(CC) $(DEBUG_OPT) $(ALL_CFLAGS) $(ALL_INCL_DIRS) -o $(OBJ_DIR)/l_agreement.o -c l_agreement.cpp
