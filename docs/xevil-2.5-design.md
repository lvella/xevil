# XEvil 2.5 "Resurrection" — Design

Goal: a major, faithful-but-modern upgrade of XEvil 2.02. Keep the 90s feel and
art style; add the things it always deserved. Target platform: modern Linux /
WSL2+WSLg (the win32 DirectDraw frontend stays archival).

## Wave 1 — Foundation
1. **Build modernization** — `make` just works (auto arch detect, no HOSTTYPE
   env dance), `make clean` cleans the real objdir, overridable CFLAGS, `debug`
   target, exit code 0 on quit.
2. **Sound & music for the X11 build** (the headline — answering the 2000
   manual's "UNIX XEvil has no sound... I'm sure some industrious soul could
   write it (hint, hint)"). Real `SoundManager` on miniaudio: all 26 original
   WAV effects with stereo pan + distance attenuation, the 9 original MIDI
   soundtracks streamed from pre-rendered MP3 (per-game-style tracks + random
   rotation), the long-banned `terraexm` track restored, graceful no-audio-device
   fallback, `-no_sound`/`-sound_volume`/`-music_volume`, network sound enabled.
3. **Real-bug fixes** — `Item::dieMessage` Boolean/enum bug ("is destroyed"
   message dead since 2000), font robustness (no more `exit(1)` on modern X
   servers missing `6x13`), `XKeycodeToKeysym` deprecation, signed-char map
   indexing, `ITickRenderer` virtual dtor, network-reachable `strcpy` hardening.
4. **Six new handcrafted worlds** (`worlds/*.xew`): new themed maps for `-world`.

## Wave 2 — Content
5. **Weapons pack**: Shotgun (spread), Railgun (piercing), Cryo Ray (freezing),
   Singularity Grenade (gravity well), Proximity Mine.
6. **Creatures**: Vampire (new class: melee + health-steal); surface the hidden
   roster — Dog/Mutt/Chicken join the random enemy pool at low weights.
7. **Scenarios & modes**: new scenarios (Kill the Yeti, Graveyard/zombie waves,
   Hugger Nest, The Junkyard/mutt pack) added to the random rotation; new game
   styles: **Survival** (endless escalating waves, `-survival`) and **Boss Rush**
   (`-bossrush`).
8. **AI upgrade** (difficulty-gated): distance-weighted target choice,
   line-of-sight check before shooting, target leading and hunting on
   hard/bend-over; classic dumbness preserved on trivial/normal.
9. **QoL**: pause overlay (F1, documented), persistent config (~/.xevilrc v2),
   persistent high-score table with the classic rank titles, version 2.5
   branding, `-help` documents formerly hidden flags (`-human_class dragon`!).

## Wave 3 — Big UX
10. **Integer scaling** `-scale 2..4` (extend the engine's native `stretch`
    lever + matched fonts) so the game is playable on 4K/HiDPI displays.
11. Demo/attract mode showcases new content; final docs (README, CHANGELOG).

## Ground rules for implementation
- Branch `xevil-2.5`; orchestrator commits after each verified wave.
- Every feature: implement → independent adversarial verify (build + headless
  runtime under Xvfb) → fix loop.
- Match existing code style and macro idioms (see docs in scratchpad maps).
- `ClassId`/`SoundName` enums: append/restore carefully; bump XETP version
  string to `"XETP2.5X"` (8 chars) since the wire content changes.
