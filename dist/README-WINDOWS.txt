===============================================================================
  XEvil 2.5  --  native Windows edition (64-bit)
===============================================================================

WHAT THIS IS
------------
XEvil is a fast, side-view, splatter-happy fighting game from 1994-2000 by
Steve Hardt and Michael Judge.  You are dropped into a scrolling underworld of
clanking machines and hungry creatures; climb the ladders, ride the elevators,
grab whatever weapon you can find, and kill everything.  When you finally die,
XEvil ranks you somewhere in Hell's hierarchy -- from "Hell's Peg Boy" up to
"You are the new Satan."

This is XEvil 2.5, a 2026 revival of the last official release (2.02).  It adds
sound and music, five new weapons, a new creature, new scenarios and game
styles, smarter enemies, HiDPI display scaling, and more.

xevil.exe is a SINGLE self-contained program.  There are no DLLs to install and
no asset folders to keep next to it -- every sprite, sound effect and music
track is baked into the one file.  You can copy xevil.exe anywhere and it will
just run.


HOW TO PLAY
-----------
Double-click  xevil.exe.

A window opens on the license screen -- click "Accept" (once) to agree to the
GNU GPL.  Then use the menu bar at the top:

  * "Game style"  -- pick Levels, Kill Kill Kill, Survival, Boss Rush, etc.
  * "New Game"    -- start.  You will be asked to pick a difficulty:
                     press 0 (trivial), 1 (normal), 2 (hard) or 3 (bend-over).

(No sound card?  No problem -- XEvil detects that and plays silently.)


CONTROLS (quickstart)
---------------------
Movement is on the NUMERIC KEYPAD (turn NumLock OFF).  The arrow keys and
W/A/S/D also move you.

    Keypad 8 / 2 / 4 / 6 .... up / down / left / right
    Keypad 7 / 9 / 1 / 3 .... diagonals
    Keypad 5 ................ stand still
    Insert .................. fire / use your weapon
    Home .................... switch to the next weapon
    Page Up ................. drop the current weapon
    Delete .................. use your item (e.g. a med-kit)
    End ..................... switch to the next item
    Space ................... chat (type a message, Enter to send)

    F1 ..................... PAUSE (shows a "PAUSED" overlay; any key resumes)
    F11 ................... toggle FULLSCREEN
    Esc ................... close an overlay / the Help screen

Everything is remappable from "Set Controls" in the menu bar; "Show Controls"
displays the current layout.  Two players can share one keyboard.

Handy command-line flags (open a Command Prompt in this folder):
    xevil.exe -kill                 free-for-all deathmatch
    xevil.exe -survival             endless escalating waves
    xevil.exe -human_class dragon   play as a Dragon (try yeti, ninja, ...)
    xevil.exe -fullscreen           start in fullscreen (F11 toggles)
    xevil.exe -scale 2              enlarge everything 2x for big / 4K screens
    xevil.exe -help                 full list of flags


WHERE YOUR SETTINGS AND HIGH SCORES ARE SAVED
---------------------------------------------
XEvil stores two small text files in your Windows profile, under:

    %APPDATA%\XEvil\

which is normally:

    C:\Users\<you>\AppData\Roaming\XEvil\

  * .xevilrc        -- your saved settings (sound, controls, window mode, ...)
  * .xevil_scores   -- the top-10 high-score table, stamped with Hell ranks

The folder is created automatically the first time it is needed.  Delete these
files to reset your settings or clear the high scores.


LICENSE AND SOURCE CODE
-----------------------
XEvil is free software under the GNU General Public License, version 2 or later.
It comes with ABSOLUTELY NO WARRANTY.  See the file "gpl.txt" in the XEvil
source tree for the full license text; the in-game license screen (the one you
Accept at startup, or run "xevil.exe -info") also reproduces it.

This binary was built entirely from freely available source code.  The complete
corresponding source -- the cross-platform game engine, the SDL2 front end, and
the mingw-w64 build files that produce this exact xevil.exe -- is distributed
with the XEvil 2.5 project.  The static SDL2 library linked into the exe is the
official SDL2 mingw development release (also under the zlib license).

XEvil(TM) is a trademark of Steve Hardt and Michael Judge.  http://www.xevil.com
===============================================================================
