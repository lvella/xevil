#
# XEvil(TM) Copyright (C) 1994,2000 Steve Hardt and Michael Judge
# http://www.xevil.com
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or (at
# your option) any later version.  See gpl.txt.
#

# sdl directory Makefile  --  native SDL2 frontend for XEvil 2.5.
#
# Self-contained (does NOT include ../config.mk) and x11-independent: all
# objects land under sdl/BUILD, and the cmn/*.cpp objects go to their OWN
# objdir (sdl/BUILD/cmn) so the SDL and X11 builds never share a single .o.
#
# The default goal builds the game: sdl/BUILD/xevil-sdl.  It compiles cmn/*.cpp
# against the SDL frontend headers (-I../sdl BEFORE -I../x11, so xdata.h / ui.h
# / sound.h resolve to the SDL versions) with -DX11=1 -DSDL=1, and links the
# SDL frontend + the reused x11 miniaudio sound engine.
#
# `make test_foundation` still builds the A1 proof-of-pixels test app.

DEPTH       = ..
OBJ_DIR     = BUILD
CMN_OBJ_DIR = BUILD/cmn

CXX     ?= g++

SDL_CFLAGS := $(shell pkg-config --cflags sdl2)
SDL_LIBS   := $(shell pkg-config --libs sdl2)

# -I$(DEPTH)/sdl MUST come first so the swappable frontend headers (xdata.h,
# ui.h, sound.h, panel.h, viewport.h, font.h) resolve to the SDL versions; the
# compiled XPM art still resolves via -I$(DEPTH)/x11 ("gen_xpm/<name>/...xpm").
ALL_INCL_DIRS = -I$(DEPTH)/sdl -I$(DEPTH)/cmn -I$(DEPTH)/x11 \
                -I$(DEPTH)/x11/bitmaps $(SDL_CFLAGS)

# Match the known-good Linux X11 build's portability defines, plus -DSDL=1.
# (RANDOM_NEEDS_PROTOTYPES is intentionally omitted: it declares
#  'void srandom(int)', which conflicts with glibc's 'unsigned int' prototype.)
COMMON_DEFS = -DX11=1 -DSDL=1 -DUSE_RANDOM -DXEVIL_KEYSET=UIlinux \
              -DUSE_UINT_NET_LENGTH
OPT         = -O2 -g

# cmn/*.cpp: match the X11 build's warning posture (quiet); -Wno-deprecated
# silences <strstream>, -Wno-write-strings the compiled-in XPM char* arrays.
CMN_CFLAGS  = $(COMMON_DEFS) $(OPT) -Wno-deprecated -Wno-write-strings
# sdl/*.cpp: our own code, compiled with -Wall.  -Wno-delete-non-virtual-dtor:
# main deletes an exactly-typed Game* on quit (no slicing), same as x11/main.cpp.
SDL_CFLAGS_ALL = $(COMMON_DEFS) $(OPT) -Wall -Wno-deprecated -Wno-write-strings \
                 -Wno-delete-non-virtual-dtor

LINK_LIBS   = $(SDL_LIBS) -lm -lpthread -ldl


# ---- object lists ----
CMN_MODULES = actual area role utils streams xetp_basic xetp locator world \
              ui_cmn game physical intel coord game_style id sound_cmn \
              l_agreement
CMN_OBJS    = $(patsubst %,$(CMN_OBJ_DIR)/%.o,$(CMN_MODULES))

SDL_MODULES = xpm font xdata draw panel viewport ui main
SDL_OBJS    = $(patsubst %,$(OBJ_DIR)/%.o,$(SDL_MODULES))

SOUND_OBJS  = $(OBJ_DIR)/sound.o $(OBJ_DIR)/miniaudio.o

# Frontend headers -- touching any forces a rebuild of everything that could
# include it (cmn objects included, since they consume xdata.h/ui.h/sound.h).
SDL_HEADERS = xdata.h ui.h sound.h panel.h viewport.h font.h xpm.h


#### Default goal: the native SDL game.
all: $(OBJ_DIR)/xevil-sdl
	@echo "Directory sdl finished: $(OBJ_DIR)/xevil-sdl"

.DEFAULT_GOAL := all


$(OBJ_DIR) $(CMN_OBJ_DIR):
	@mkdir -p $@


$(OBJ_DIR)/xevil-sdl: $(CMN_OBJS) $(SDL_OBJS) $(SOUND_OBJS)
	$(CXX) $(OPT) -o $@ $(CMN_OBJS) $(SDL_OBJS) $(SOUND_OBJS) $(LINK_LIBS)


#### cmn/*.cpp -> BUILD/cmn/*.o  (SDL frontend headers, separate objdir)
$(CMN_OBJ_DIR)/%.o: $(DEPTH)/cmn/%.cpp $(SDL_HEADERS) | $(CMN_OBJ_DIR)
	$(CXX) $(CMN_CFLAGS) $(ALL_INCL_DIRS) -o $@ -c $<


#### sdl/*.cpp -> BUILD/*.o
$(OBJ_DIR)/%.o: %.cpp $(SDL_HEADERS) | $(OBJ_DIR)
	$(CXX) $(SDL_CFLAGS_ALL) $(ALL_INCL_DIRS) -o $@ -c $<


#### Reused x11 sound engine (no X includes; verified by the port audit).
$(OBJ_DIR)/sound.o: $(DEPTH)/x11/sound.cpp $(DEPTH)/x11/sound.h \
                    $(DEPTH)/x11/vendor/miniaudio.h | $(OBJ_DIR)
	$(CXX) $(CMN_CFLAGS) $(ALL_INCL_DIRS) -I$(DEPTH)/x11/vendor \
		-o $@ -c $(DEPTH)/x11/sound.cpp

$(OBJ_DIR)/miniaudio.o: $(DEPTH)/x11/vendor/miniaudio.c \
                        $(DEPTH)/x11/vendor/miniaudio.h | $(OBJ_DIR)
	$(CXX) -x c $(OPT) -I$(DEPTH)/x11/vendor -o $@ \
		-c $(DEPTH)/x11/vendor/miniaudio.c


#### A1 foundation proof-of-pixels test app (still available).
FOUNDATION_OBJS = $(OBJ_DIR)/xpm.o $(OBJ_DIR)/font.o $(OBJ_DIR)/sdlvars.o \
                  $(OBJ_DIR)/test_foundation.o

test_foundation: $(OBJ_DIR)/test_foundation
	@echo "Directory sdl finished: $(OBJ_DIR)/test_foundation"

$(OBJ_DIR)/test_foundation: $(FOUNDATION_OBJS)
	$(CXX) $(SDL_CFLAGS_ALL) -o $@ $(FOUNDATION_OBJS) $(SDL_LIBS) -lm


#### Regenerate the compiled-in bitmap fonts from the system X11 PCFs.
fonts:
	cd fonts && python3 gen_font.py


clean:
	/bin/rm -rf $(OBJ_DIR)

.PHONY: all test_foundation fonts clean
