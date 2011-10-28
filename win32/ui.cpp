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

// "ui.C"
// TAG: UI
// Also has class Panel.

#include "stdafx.h"
// Include Files
#include "utils.h"

#include <iostream.h>
#include <strstrea.h>

#include "xdata.h"
#include "coord.h"
#include "world.h"
#include "locator.h"
#include "id.h"
#include "physical.h"
#include "ui.h"
#include "uiplayer.h"



Ui::Ui() {
  settingsChanges = UInone;
}



Ui::~Ui() {
}



int Ui::getWindowList(CTypedPtrList<CPtrList,CWnd *>&p_windowlist) {
  return 0;
}



int Ui::get_viewports_num() {
  return 0;
}



int Ui::get_dpy_max(){
  return 0;
}



UImask Ui::get_settings(UIsettings &s) {
  s = settings; 
  UImask tmp = settingsChanges;
  settingsChanges = UInone;
  return tmp;
}



void Ui::set_humans_num(int val)
{
  settings.humansNum=val;
}



void Ui::set_enemies_num(int val)
{
  settings.enemiesNum = val;
}



void Ui::set_enemies_refill(BOOL val)
{
  settings.enemiesRefill = val;
}



void Ui::set_sound_onoff(BOOL val)
{
  settings.sound=val;
}



void Ui::set_sound_volume(int val)
{
  settings.soundvol=val;
}



void Ui::set_track_volume(int val)
{
  settings.trackvol=val;
}



void Ui::set_world_rooms(const Rooms &r)
{
  settings.worldRooms=r;
}



void Ui::set_style(GameStyleType style)
{
  settings.style = style;
}



void Ui::set_quanta(Quanta quanta)
{
  settings.quanta = quanta;
}



void Ui::set_humans_playing(int val)
{
}

void Ui::set_cooperative(Boolean p_bool)
{
  settings.cooperative=p_bool;
}

void Ui::set_musictype(UIsettings::SOUNDTRACKTYPE val)
{
  settings.musictype=val;
}

void Ui::set_pause(BOOL val)
{
}


void Ui::set_enemies_playing(int){}

void Ui::set_level(const char *){}

void Ui::set_screen_mode(Xvars::SCREENMODE p_mode){}

Boolean Ui::other_input(){return FALSE;}

void Ui::set_input(int vNum,UIinput input){}

void Ui::set_keyset(int dpyNum,UIkeyset keyset){}
void Ui::set_keyset(int dpyNum,UIkeyset basis,
                    char right[UI_KEYS_MAX][2],
                    char left[UI_KEYS_MAX][2]){}

void Ui::set_prompt_difficulty(){}

// This needs to be fixed.
int Ui::get_difficulty(){return 0;}



void Ui::set_difficulty(int){}



int Ui::add_viewport(){return 0;}
void Ui::del_viewport(){}
void Ui::register_intel(int n, IntelP intel){}
void Ui::unregister_intel(int n){}
IntelP Ui::get_intel(int n){return NULL;}
void Ui::demo_reset(){}
void Ui::reset(){}
void Ui::reset_graphics(Xvars::SCREENMODE p_newmode){}
void Ui::set_redraw_arena(){}
Boolean Ui::process_event(int dpyNum,CMN_EVENTDATA eventdata){return FALSE;}
void Ui::set_role_type(RoleType){}



IViewportInfo* Ui::get_viewport_info() {
  return UiPlayer::get_viewport_info();
}



void Ui::check_num_lock() {
  // Ask user to turn on num lock.
  SHORT val = GetKeyState(VK_NUMLOCK);
  if (!(LOBYTE(val) & 0x1)) {
    AfxMessageBox("Turn on \"Num Lock\" if you want to use the numeric keypad.");
  }
}



void Ui::set_reduce_draw(Boolean val) {
  Xvars::set_reduce_draw(val);
}



Boolean Ui::initGraphics = TRUE;



