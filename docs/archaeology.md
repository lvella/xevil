# XEvil Field Notes — Code Archaeology (2026 excavation)

Findings from a full source excavation of XEvil 2.02 (1994–2000, Steve Hardt &
Michael Judge), done during the XEvil 2.5 modernization. Everything below is
verifiable in this repo at the cited locations.

## The 26-year-old request
The instruction manual's sound section (`instructions/instructions.html`):

> "UNIX XEvil has no sound. But, with the source freely available, I'm sure
> some industrious soul could write it (hint, hint)."

`x11/sound.h` shipped as *"Dummy classes, only implemented on Windows. A place
holder if we ever implement sound on X."* The placeholder waited from 2000 to
2026. XEvil 2.5 answers it — with the original audio, recovered from the
official `xevil-windows.exe` still served by xevil.com.

## The banished soundtrack
`cmn/bitmaps/sound_cmn/sound_cmn.bitmaps:67-70`:
```c
#if 0
    // Nice piece but sounds really painful on cheap sound cards.
    IDM_TERRAEXMSOUNDTRACK,
#endif
```
`TERRAEXM.MID` shipped inside every copy of the game but was excluded from the
enum, the resource table, and the random rotation — unhearable for 26 years
because 2000-era sound cards butchered it. Restored in 2.5.

## The lost soundtrack
`win32/resource.h:1009` defines `IDM_DEATHMARCHSOUNDTRACK 1631` — the only
trace of a tenth music track, "Death March," anywhere. No file, no code
reference, and the resource is absent from the shipped exe; the ID number was
even recycled for a wall bitmap (`IDB_MD4WALL 1631`). Truly lost, unless an
older 1.x/2.0-beta build surfaces.

## The Fire Demon's ten souls (a player-discovered secret, confirmed)
`cmn/physical.h:3301`: *"Has limited protection from swap attacks. Unlimited
protection from Frog attacks."* Implementation (`physical.cpp:9566`): each
Soul-Swapper hit decrements a counter and fails; FireDemon ships with
`swapResistance = 9` (`fire_demon.bitmaps:220`) — **the 10th swap-shell hit
takes his soul**. He is the only creature in the game with this mechanic, and
`frog_protect()` returns unconditional True — the Frog Gun can never touch him.

## Green Hugger vs Red Hugger — one boolean apart
The entire difference (`hugger.bitmaps:201-366`) is `useHuggeeIntel`:
- **Red** (`False`): the burst Alien gets the *hugger's* mind; you get 1 point
  of damage first — comment: *"Do one point of damage to huggee, so hugger will
  get credit for the kill"* — and *"Huggee's intel dies with the huggee's
  physical body."*
- **Green** (`True`): the new Alien gets **your** mind (`alien->set_intel(
  huggee->get_intel())`, `physical.cpp:8672`) — *"No kill is awarded for this
  death."* Your body dies; you keep playing as the Alien (wall-crawling,
  spiky, regenerating — but no hands, so no weapons).
While latched, the hugger tracks its victim with: *"Guess where hugee's face
is, upper-left + (width/2, height*.3)"*.
The Green Hugger is also the statistically rarest creature: Hive scenario only,
egg-hatched, ~20% of hatches, and the one creature the Transmogrifier refuses
to create.

## Hell's org chart
`cmn/game.cpp:369-456` — end-of-game ranks by kills: "Hell's Peg Boy" →
"Satan's Earwax Remover" → "IS Tech Support for Hell" → "Teacher at Beelzebub
Jr. High" → "Hell's Sysadmin" → "Lead Software Engineer of Hell" (250 kills) →
at exactly **666** kills, *"Replace Bill as Satan's Right Hand Man"* (hello,
year-2000 Bill Gates joke) → at **666,666,666**, *"You are the new Satan."*
The difficulty above "hard" is named `bend-over`.

## The only file XEvil ever writes
`~/.xevilrc`, whose entire contents are (`x11/l_agreement_dlg.cpp:401`):
```
XEvil is your friend.  Trust XEvil.
```

## Developers thinking out loud, shipped to production
- Pet AI (`cmn/intel.cpp:2192,2220`): *"We don't use strategyChange timer at
  all?? Is this right?"* and *"Does this do anything??"*
- Rock-carrier attack (`intel.cpp:1296`): *"we don't really know how to drop
  weights on people"* — so it just lets go.
- Dog Whistle's alternate summon produces a facehugger instead of a dog,
  annotated *"ha, ha"* (`actual.cpp:3370`).
- Dead AI brains have their class name set to `"tormented spirit"` before
  deallocation (`intel.cpp` die path).
- The famous stuck-on-a-corner dog was fixed with dice: a 1-in-20 random +1 to
  enemy reaction time, deliberately desyncing brains from physics
  (`intel.cpp:1096-1106`).

## Hidden features never documented
- **`-human_class <name>`** — play as ANY of the 18 creatures: `dragon`
  (15-segment boss, with a dedicated assembly hack at `game.cpp:1727`),
  `fire-demon`, `yeti`, `chicken`, `baby-seal`, `zombie`, `mutt`...
  Companion `-ability flying|on-fire|sticky|hopping` stacks extra abilities.
- **F1 = pause**, code comment: *"Undocumented pause key feature"*
  (`x11/ui.cpp:482`).
- `-scenario exterminate` reaches a scenario cut from the rotation with the
  comment "We don't use it right now."
- The Windows build had `-cd`: play random audio-CD tracks as the soundtrack.
- `-gen_xpm <dir>`: the Windows build could auto-generate the UNIX XPM art —
  the art pipeline ran Windows→UNIX.

## Miscellany
- Orphan sounds in `win32/res/`: `FROG.WAV` and `WOOB.WAV`, wired to nothing.
- A 2000-era real bug, fixed in 2.5: `Item::dieMessage` was declared `Boolean`
  but assigned a 3-value enum, so destroyed items have said "has been used."
  instead of "is destroyed." for 26 years (`physical.h:1396`).
- Contact email in the readme: `satan@xevil.com`. Artwork credit: Comrade.Cid.
