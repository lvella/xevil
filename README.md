# XEvil 2.5

XEvil is a fast, side-view, splatter-happy fighting game. You wake up as one
more expendable body dropped into a scrolling underworld of clanking machines
and ravenous creatures, armed with your fists and whatever over-the-top weapon
you can scavenge — chainsaws, flamethrowers, soul-swappers, gravity
singularities. Kill everything, climb the ladders, ride the elevators, and try
to last one more level than the thing chasing you. When you finally die, XEvil
totals your kills and hands you a rank in Hell's hierarchy — anything from
**"Hell's Peg Boy"** up to **"You are the new Satan."** It has been played by
strangers over a network since 1994; this is the **XEvil 2.5** revival of the
last official release (2.02, Steve Hardt & Michael Judge, 2000).

---

## What's new in 2.5

- **Sound, after 26 years of silence.** The UNIX manual always said: *"UNIX
  XEvil has no sound. But, with the source freely available, I'm sure some
  industrious soul could write it (hint, hint)."* 2.5 takes the hint — a real
  audio engine (miniaudio) plays all 26 original sound effects with stereo pan
  and distance falloff, and streams all nine original MIDI soundtracks. That
  includes **terraexm**, a track that shipped inside every copy since 2000 but
  was disabled because it *"sounds really painful on cheap sound cards"* —
  restored to the rotation at last.
- **Five new weapons:** the **Shotgun** (3-shell spread), the **Railgun**
  (piercing rail shot), the **Cryo Ray** (freezes what it hits), the
  **Proximity Mine** (safe on the ground — *place it* with your item key to
  arm; it lies dormant while you back clear, then detonates when anything
  strays near),
  and the **Singularity** — a lobbed **WOOB** that anchors, drags everything
  nearby into its collapse, and explodes. Its collapse is voiced by WOOB.WAV,
  an audio file that was orphaned in the original Windows resources and never
  once played until now.
- **A new creature, the Vampire:** a pale, blood-drinking cousin of the Hero
  that heals itself on every landed melee hit. The random enemy roster also
  grew from **8 to 12 classes** — Dogs, Mutts, and Chickens now turn up in
  ordinary levels at treat-tier odds.
- **Four new scenarios** in the rotation: **Melt the Yetis** (fire weapons vs.
  heat-hating yetis), **The Graveyard Shift** (chainsaws vs. a horde of
  zombies), **The Nest** (a dense egg-and-hugger hive), and **Junkyard Dogs**
  (a mutt pack).
- **Two new game styles:** **Survival** (endless escalating waves, a Fire Demon
  every fifth) and **Boss Rush** (a Fire Demon / Dragon / Yeti gauntlet).
- **Smarter enemies on the hard settings:** on *hard* and *bend-over* the AI
  weighs distance when picking targets, checks line of sight before firing,
  leads moving targets, and actively hunts the humans. Holding onto their loot
  instead of fumbling it, they now **deploy the items they carry** — planting
  mines and bombs when you close in, and casting a **doppelganger** slave of
  themselves (a machine ability restored from code left dormant since 2000).
  *Trivial* and *normal* enemies still fumble their items quickly and otherwise
  play as they always did.
- **Six handcrafted worlds** in `worlds/` and an integer **display scale**
  (`-scale`) for modern/HiDPI screens.
- **Quality of life:** a visible **PAUSED** overlay on **F1**, a persistent
  `~/.xevilrc` config, and a `~/.xevil_scores` top-10 high-score table with
  Hell-rank titles.

---

## Quickstart

You need a C++ compiler, X11, and libXpm. On Debian/Ubuntu:

```
sudo apt install build-essential libx11-dev libxpm-dev
```

Then build and run:

```
make
./x11/REDHAT_LINUX/xevil
```

`make` auto-detects your architecture and links the binary at
`x11/REDHAT_LINUX/xevil` on x86-64 Linux. Click **New Game** in the menu bar to
start; on UNIX you'll be asked to pick a difficulty first. `make debug` builds
an unoptimized `-g` binary into `x11/DEBUG/`; `make clean` removes all build
output.

---

## Controls

Movement is on the **numeric keypad** (NumLock off). The keys to its right
handle your weapon and your item; everything is remappable from **Set
Controls** in the menu bar.

| Keys | Action |
|------|--------|
| **KP 8 / 2 / 4 / 6** | up / down / left / right |
| **KP 7 / 9 / 1 / 3** | diagonals (up-left, up-right, down-left, down-right) |
| **KP 5** | stop / stand still |
| **Insert** | fire / use weapon |
| **Home** | cycle to next weapon |
| **Page Up** | drop weapon |
| **Delete** | use item |
| **End** | cycle to next item |
| **Page Down** | drop item |
| **Space** | chat (type a message to other players) |
| **F1** | pause (shows a PAUSED overlay) |

Two players can share one keyboard: the second player gets the same layout
mirrored onto the left-hand keys — see **Show Controls**.

---

## Game styles

Pick one from the **Game style** menu, or pass the matching flag.

| Style | Flag | One-liner |
|-------|------|-----------|
| **Levels** | `-levels` | Clear each level of machines to advance. |
| **Scenarios** | `-scenarios` | A rotating campaign of themed set-pieces. |
| **Normal** | *(default)* | Levels, with a random scenario every fifth. |
| **Kill, Kill, Kill** | `-kill` | Free-for-all deathmatch / screensaver. |
| **Duel** | `-duel` | Human vs. human, limited lives. |
| **Extended Duel** | `-extended` | Human vs. human, unlimited lives. |
| **Training** | `-training` | No enemies — learn the controls in peace. |
| **Survival** | `-survival` | Endless waves that escalate; a Fire Demon every fifth. |
| **Boss Rush** | `-bossrush` | A Fire Demon / Dragon / Yeti boss gauntlet. |

---

## Handcrafted worlds

Load any bundled world in `worlds/` with `-world`:

```
./x11/REDHAT_LINUX/xevil -world worlds/citadel.xew
```

| World | Feel |
|-------|------|
| `citadel.xew`   | Fortress: thick walls, a tall central keep, battlements, a dungeon. |
| `catacombs.xew` | Claustrophobic maze of small chambers and many ladders. |
| `skyline.xew`   | Vertical city: towers joined by elevators and one-way sky bridges. |
| `arena.xew`     | Open colosseum with a perimeter gallery; built for `-kill`. |
| `vertigo.xew`   | A narrow, very tall climb of platforms, ladders and movers. |
| `depths.xew`    | Wide underground strata joined by shafts and floor elevators. |

A world file is plain text — see the comments in `world1.xew` for the character
set — so you can write your own and load it the same way.

---

## Fun flags

Everything below is real; run `./x11/REDHAT_LINUX/xevil -help` for the full
list.

| Flag | Does |
|------|------|
| `-human_class dragon` | Play as a **Dragon** instead of a Hero. (Try `yeti`, `ninja`, `fire-demon`, `chopper-boy`… too.) |
| `-scenarios -scenario <name>` | Play one scenario every level: `yeti` (Melt the Yetis), `graveyard`, `hugger-nest`, `junkyard`, `hive`, `dragon`, `the-pound`, `japan-town`, `chicken-little`, … |
| `-one_each` | Spawn exactly one of every weapon and item — an instant armory. |
| `-world worlds/vertigo.xew` | Play any handcrafted (or your own) world. |
| `-scale 3` | Enlarge the whole game 3× (1–4) for big / HiDPI screens. |
| `-kill -machines 20` | Twenty bots, no humans — leave it running as a screensaver. |
| `-no_sound` / `-sound_volume N` / `-music_volume N` | Silence or balance the new audio (N is 0–100). |

---

## Windows

There are two ways to play on Windows. **Option A is the easy one** — a single
native `xevil.exe` you double-click. Option B (WSL2) is for people who already
live in a Linux shell.

### Option A — native `xevil.exe` (recommended)

XEvil 2.5 ships as a **single, self-contained `xevil.exe`** for 64-bit Windows.
There are no DLLs to install and no asset folders to keep beside it: every
sprite, sound effect and music track is baked into the one file. Download it
(or build it — below), then **double-click `xevil.exe`**. Click **Accept** on
the license screen, pick a **Game style** and press **New Game**.

- Settings and high scores are saved under `%APPDATA%\XEvil\` (that's
  `C:\Users\<you>\AppData\Roaming\XEvil\`) as `.xevilrc` and `.xevil_scores`.
- **F1** pauses (with a *PAUSED* overlay), **F11** toggles fullscreen, **Esc**
  closes an overlay. Movement is the numeric keypad (or the arrow keys / WASD);
  see [Controls](#controls) above and **Set Controls** in the menu.
- No sound card? The audio engine detects that and plays silently — the game
  still runs.
- Handy flags from a Command Prompt: `xevil.exe -survival`,
  `xevil.exe -human_class dragon`, `xevil.exe -fullscreen`, `xevil.exe -scale 2`,
  `xevil.exe -help`.

**Building `xevil.exe` yourself** (cross-compiled from these same sources with
mingw-w64, on Linux or WSL2):

```
sudo apt install g++-mingw-w64-x86-64 imagemagick python3
make windows          # -> sdl/BUILD-WIN/xevil.exe
make dist-windows     # -> dist/xevil.exe + dist/XEvil-2.5-win64.zip
```

`make windows` produces the single static `xevil.exe` (SDL2, libgcc, libstdc++
and all audio linked in). `make dist-windows` also drops the bare exe at
`dist/xevil.exe` and wraps it with a short `README-WINDOWS.txt` into
`dist/XEvil-2.5-win64.zip`. The committed static SDL2 lives in
`sdl/vendor/SDL2-mingw`, so the cross-compile is reproducible.

### Option B — WSL2 + WSLg

The X11 build also runs on **Windows 11 under WSL2** with no extra setup —
**WSLg** provides both the X display and audio out of the box. In a WSL2
Ubuntu shell:

```
sudo apt install build-essential libx11-dev libxpm-dev
make
./x11/REDHAT_LINUX/xevil
```

The window opens on your Windows desktop and sound plays through your Windows
audio device. On a HiDPI laptop the classic sprites are tiny — add `-scale 2`
or `-scale 3`:

```
./x11/REDHAT_LINUX/xevil -scale 3
```

---

## More documentation

- `instructions/instructions.html` — the original player's manual (controls,
  creatures, weapons, network play).
- `docs/archaeology.md` — field notes from a 2026 excavation of the source:
  the mechanics, secrets, and lost pieces uncovered while building 2.5.
- `docs/xevil-2.5-design.md` — the design notes behind this release.
- `CHANGELOG.md` — everything that changed from 2.02 to 2.5.
- `sounds/README.md` — where the recovered audio came from.

---

## Credits & license

Original XEvil by **Steve Hardt** (design, architecture, cross-platform and
UNIX code) and **Michael Judge** (Windows front end, sound), with artwork by
**Comrade.Cid** and others. XEvil 2.5 modernization, 2026. (Full credits are in
the in-game license screen — run `xevil -info`.)

XEvil is free software under the **GNU General Public License** (version 2 or
later); see `gpl.txt`. It comes with absolutely no warranty. XEvil(TM) is a
trademark of its authors — see the notice in `cmn/game.cpp` before changing the
copyright or in-game text.
