# Changelog

All notable changes in the **XEvil 2.5** revival, relative to the last official
release, **XEvil 2.02** (Steve Hardt & Michael Judge, 2000). Grouped by area.

## [2.5] — 2026

### Sound & music (new — X11/UNIX had none before)
- Real `SoundManager` on the vendored **miniaudio** engine, with lazy
  initialization and a graceful silent fallback when no audio device is present
  (so headless / no-soundcard machines still run).
- All **26 original sound effects** play, with stereo panning and
  distance-based attenuation. The WAVs were recovered from the official
  `xevil-windows.exe` and are documented in `sounds/README.md`.
- All **nine original MIDI soundtracks** stream, chosen per game style.
- **terraexm** restored to the rotation — it shipped in every copy since 2000
  but was `#if 0`'d out because it *"sounds really painful on cheap sound
  cards."*
- **WOOB.WAV**, orphaned in the original Win32 resources and never played, is
  now the collapse sound of the new Singularity weapon.
- New flags: `-no_sound`, `-sound_volume <0-100>`, `-music_volume <0-100>`.
  Network clients request the server's sound relay.

### Weapons
- **Shotgun** — fires a three-shell spread.
- **Railgun** — a single fast, piercing Rail shot.
- **Cryo Ray** — fires an IceBolt that freezes/stuns what it hits.
- **Singularity** — lobbed like a grenade; anchors, drags every nearby moving
  thing into its collapse (a GravityWell), then explodes. Voiced by WOOB.
- **Proximity Mine** — blinks, arms, and detonates when something approaches.

### Creatures
- **Vampire** — a Walking/Fighter/Healing palette-swap of the Hero with a new
  `melee_hit_hook` that steals health on landed hits only.
- The random enemy pool grew from **8 to 12 classes**: Dogs, Mutts, and
  Chickens now spawn in ordinary levels at treat-tier weights (8/5/4).

### Scenarios (added to the random rotation and the `-scenario` override list)
- **Melt the Yetis** (`-scenario yeti`) — fire weapons vs. heat-sensitive yetis.
- **The Graveyard Shift** (`-scenario graveyard`) — chainsaws vs. 25 zombies.
- **The Nest** (`-scenario hugger-nest`) — a dense egg/hugger hive.
- **Junkyard Dogs** (`-scenario junkyard`) — a mutt pack.

### Game styles
- **Survival** (`-survival`) — endless escalating waves; a Fire Demon every
  fifth wave.
- **Boss Rush** (`-bossrush`) — a Fire Demon / Dragon / Yeti boss cycle.

### AI
- Difficulty-gated smartness: on **hard** and **bend-over**, enemies use
  distance-weighted target selection, check line of sight before firing, lead
  moving targets, and hunt the human players. **Trivial** and **normal**
  behavior is unchanged.

### Worlds
- Six handcrafted worlds in `worlds/`: **citadel, catacombs, skyline, arena,
  vertigo, depths**. Load any with `-world <file>`; the `.xew` format is plain
  text (see `world1.xew`).

### Attract-mode demo
- Three new demo scenes join the eight originals (`Game::demo_setup` now picks
  from eleven): **Vampire hunt** (Heroes vs. Vampires), **New arms expo**
  (Ninjas fighting over the 2.5 arsenal), and **Singularity storm** (Heroes
  dueling with WOOB singularities amid collapsing gravity wells).

### Quality of life
- **F1** pauses the game and shows a visible **PAUSED** overlay.
- **`-scale <1-4>`** (X11): integer display scaling for modern / HiDPI screens.
- Persistent **`~/.xevilrc`** config (v2 format; still readable by pre-2.5
  builds) remembers your settings between sessions.
- **`~/.xevil_scores`** — a top-10 high-score table stamped with Hell-rank
  titles.
- New **Sound**, **Survival**, and **Boss Rush** entries in the menu bar.
- Version strings bumped to **2.5** (network protocol `XETP2.5X`); `-help` now
  documents flags that were previously undocumented.

### Build
- Plain **`make`** now auto-detects the architecture (`uname -m`) and builds
  without hand-set environment variables; the binary lands at
  `x11/REDHAT_LINUX/xevil` on x86-64.
- New **`make debug`** target (`-O0 -g` into `x11/DEBUG/`) and a **`make
  clean`** that removes every real objdir.
- Builds and runs on **Windows 11 / WSL2** via WSLg (display and audio work out
  of the box).
- Normal quit now `exit(0)`s cleanly.

### Bug fixes (carried over from the 2.5 modernization)
- `Item::dieMessage` changed from `Boolean` to the `MESSAGE` enum, so destroyed
  items say *"is destroyed."* again.
- Font fallback chain so modern X servers without the `6x13` font no longer
  `exit(1)`.
- `XkbKeycodeToKeysym` keycode handling; signed-`char` map-indexing fix.
- Added a virtual destructor to `ITickRenderer`; fixed a delete-`void*` closure
  undefined-behavior bug.
- Bounded several network-reachable `strcpy`s.

### Known cosmetic note
- `config.mk`'s `VERSION` (used only for the packaged tarball name) still reads
  `2.1` from the 2.02 tree; it does not affect the in-game version, which is
  2.5 everywhere it is shown.
