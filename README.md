# XEvil

XEvil is a fast-action, side-view, multi-player fighting game for X11 (with an
archival Win32 port). You battle waves of machines and creatures through
scrolling worlds using an arsenal of over-the-top weapons. It began life in
1994-2000 as XEvil 2.02; this tree is the **XEvil 2.5** upgrade.

Copyright (C) 1994,2000 Steve Hardt and Michael Judge. Free software under the
GNU General Public License (see `gpl.txt`).

## Build

```
make
```

The Makefile auto-detects your architecture and links the binary at
`x11/REDHAT_LINUX/xevil` (on x86_64 Linux).

## Run

```
./x11/REDHAT_LINUX/xevil
```

Click **New Game** to start. Useful flags: `-kill -machines 6` (instant
deathmatch), `-training` (calm, no enemies), `-difficulty normal`,
`-scenarios`. Run `./x11/REDHAT_LINUX/xevil -help` for the full list.

## Handcrafted worlds

Load any of the bundled worlds in `worlds/` with `-world`:

```
./x11/REDHAT_LINUX/xevil -world worlds/citadel.xew
```

| World | Theme |
|-------|-------|
| `citadel.xew`   | Fortress: thick walls, a tall central keep, battlements, a dungeon. |
| `catacombs.xew` | Dense claustrophobic maze of small chambers and many ladders. |
| `skyline.xew`   | Vertical city: towers joined by elevators and one-way sky bridges. |
| `arena.xew`     | Open colosseum with a perimeter gallery; built for `-kill`. |
| `vertigo.xew`   | A narrow, very tall climb of platforms, ladders and movers. |
| `depths.xew`    | Wide underground strata joined by shafts and floor elevators. |

You can write your own too: a world file is plain text (see the comments in
`world1.xew` for the character set) and is loaded the same way.

## More documentation

- `instructions/` — player instructions (`instructions.html`), controls, keys.
- `docs/` — project design notes (`xevil-2.5-design.md`) and history.
