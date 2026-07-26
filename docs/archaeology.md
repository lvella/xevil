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

---

# Part II — The 2026 deep excavation

A second, longer dig (source tree + the original v2.02 `xevil.exe` + the dead web
via archive.org). New material only; everything cited.

## The 148 things your character can say
`cmn/game.cpp:128-277` — `Game::wittySayings[148]` (`WITTY_SAYINGS_NUM = 148`,
`game.h:271`). At the start of **every new game** (`Game::reset`, `game.cpp:2585`)
one is picked at random and pushed to the on-screen message queue
(`msg << wittySayings[Utils::choose(WITTY_SAYINGS_NUM)]`). Most players saw a
handful and never knew there were 148. It is a wall of late-'90s pop culture and
MIT-engineer humor. A sampler, verbatim:
- Film/TV: *"I love the smell of napalm in the morning."*, *"Wake up, time to
  die."*, *"Ho, ho, ho.  Now I have a machine gun."*, *"I'm sorry, Dave.  I can't
  do that."*, *"Fuck me gently with a chainsaw."*, *"I'm here to chew bubble-gum
  and kick ass.  And I'm all out of bubble-gum."*, *"Oh, my God!  They killed
  Kenny!"*, *"We're going to need a bigger boat."*, *"Do not taunt
  Happy-Fun-Ball."*
- Monty Python: *"Nobody expects the Spanish Inquisition!!!"*, *"Come see the
  violence inherent in the system."*, *"Oh, I'm a lumberjack and I'm ok..."*
- MIT: *"We are, we are, we are, we are, we are the engineers. / We can, we can,
  we can, we can demolish forty beers."* (the Engineers' Drinking Song) and
  *"You're immune to both romance and mirth.  You must be a... a... / That's
  right.  I'm an engineer."*
- Hacker/self-aware: *"Hackito ergo sum."*, *"It's not a bug, it's a feature."*,
  *"XEvil.  The carpal tunnel game."*, *"Aaaaa!!  My soul is trapped in XEvil.
  Send help!!"*, *"fnord"*, *"M\*cr\*s\*ft:  Who need quality when you have
  marketing."*
- And the sign-off it saves for last: *"XEvil.  The peak of abnormality."*

We can even date the last five. Diffing the shipped array against the earliest
surviving source (May 28 1999, recovered below) shows **exactly five sayings were
added in the final stretch** and none removed: *"You're my bitch now."*,
*"Prepare to Qualify."*, *"I got a bad feeling about this drop."*, *"Cowboys never
quit!"*, and *"XEvil.  The peak of abnormality."*

## The 74 names of the damned
`cmn/game.cpp:281-356` — `Game::intelNames[74]` (`INTEL_NAMES_NUM = 74`). At game
start the list is shuffled (`Utils::random_list`, `game.cpp:1074,3276`) and dealt
out to every bot and player (`game.cpp:3358`). It is a self-portrait of the
authors' world:
- **The author signs his own kill-list**: `"hardts"` (`game.cpp:284`) is Steve
  Hardt's literal MIT Athena username — you can frag *him*. `"Steve"`, `"Sensei
  Steve"`, `"Cowboy Steve"` recur; `"Cid"` (`game.cpp:355`), the artist
  Comrade.Cid, is the very last name in the array.
- **MIT deep cuts**: `"James E. Tetazoo"` (the MIT Physics department's legendary
  fictitious professor), `"Ben Bitdiddle"` (the stock character from 6.001 / SICP),
  `"e^x Man"`, `"Pixie of Pass/Fail"`, `"Josh the 10-yr Old"`.
- **'90s cartoons**: Beavis, Butt-Head, Ren, Stimpy, Mr. Horse, Dilbert, Dogbert,
  Gromit, The Tick, Spaceman Spiff, Brain, and the entire South Park core
  (Kenny, Kyle, Cartman, Stan, Mr. Hat).
- **Aliens/Star Wars**: Hudson, Hicks, Wedge, Redshirt — the same movie whose
  lines seeded the witty sayings ("Marines!  We are leaving.", "Game over, man").

## Hell's full org chart (extending the earlier ranks)
The earlier note quoted the tentpoles; the whole ladder (`cmn/game.cpp:370-456`,
`rankingSets[]`, keyed by kill count) is a sustained corporate-satire bit worth
preserving. Each tier picks one title at random. Highlights not previously
recorded: at **0 kills** you might be *"Member of Hell's Children's Choir"* or
*"Satan's Vibrating Sex Toy"*; the 25-tier includes *"Hell's Speedbump"*; the
40-tier *"QA of Vomit Production"* and *"Teacher at Beelzebub Jr. High"*; a whole
**middle-management tier at 90 kills** (*"Manager, Department of Graft and
Corruption"*, *"Manager, Medieval Torture Devices Maintenance"*) sitting right
next to *"Postal Worker"* (a "going postal" joke); then *"VP of Hell Marketing"*,
*"VP of Hell Legal"*, *"Torture Methods Research Scientist"*, and at 250
*"Lead Software Engineer of Hell"* — the same title the manual dangles as
aspirational. The 666 and 666,666,666 capstones were already noted.

## The Armory, recovered from the dead web
The manual (`instructions/instructions.html:573`) links to *"The XEvil Armory"* at
`xevil.com/xevil/armory` — a page that no longer exists on the live domain (which
now hosts an unrelated CAPTCHA product). Recovered from the Wayback Machine
(`web.archive.org/web/20030219192631/http://www.xevil.com/xevil/armory/`,
"Copyright(c) 1998 Steve Hardt and Michael Judge", banner "by Frank Wu"). The
weapon/item flavor text the owner never saw, quoted:
- **Pistol**: *"High caliber sidearm.  Do you feel lucky, punk."*
- **Machine-Gun**: *"...Won Bombs 'n Ammo 'Weapon of the Year' three times
  consecutively."*
- **Chainsaw**: *"It slices! It dices! It can even cut a steel can... 'The
  Weed-Whacker from Hell'"*
- **Soul-Swapper**: *"Swaps the souls of the shooter and shootee...  Be careful
  to hit the right target."*
- **Frog-Gun**: *"Zaaaapp.  Ribbet.  You get the picture."*
- **Crack Pipe**: *"Increases speed and hand-to-hand damage.  Occasional lethal
  overdose.  'C'mon baby, I need my fix.'"*
- **P.C.P.**: *"Invulnerability, increased speed and hand-to-hand damage.  The
  come-down is a real bitch.  Be careful of overdose.  Long or short-term use may
  cause irreversible psychosis."* (this exactly describes the code — see below)
- **Demon-Summoner**: *"Contains a Demon held by Ka'Shi'Taath enslavement spell.
  Do not taunt."*
- **X-Shield**: *"Shield of solidified evil gives protection from 300 points of
  any type of damage."* — and indeed `XProtection` is created with 300 in
  `game.cpp` (the bonus path). The flavor text is load-bearing documentation.
- **Auto-Lancer**: *"Lancer chassis and energy coils connected to the
  ever-burning-scorching heart of a condemned sinner as power supply."*
- **Altar-Of-Sin**: *"Sell your soul to the Altar-Of-Sin for cool stuff.  Like
  saving cereal-box tops, but better."*

## The Character Profiles, recovered from the dead web
Also linked but long dead: *"XEvil Character Profiles"* at `xevil.com/xevil/profiles`
(a frameset; the real pages are `profiles/<name>.html`). Recovered via Wayback
(`web.archive.org/web/20021205*/http://www.xevil.com/xevil/profiles/`). Each
creature has a full dossier — name, age at death, blood type, sins, cause of
death, favorite food, and a backstory. The owner has almost certainly never read
these. The best bits:
- **Hero** — Marine Drill Sergeant, sins "Murder, bad jokes." A mock *Bombs 'n
  Ammo* interview where the interviewer tries to get his old lieutenant to admit
  Hero was dumb, and the lieutenant just keeps repeating *"Hero was a good
  soldier, a dedicated and honorable man.  It was a privilege serving with him."*
  Cause of death: *"Cigar smoking near ammo dump."*
- **Ninja** — is a **woman** ("Sex: F"), age "none of your damn business," the
  only Caucasian ever admitted to the Fujiyakamoto School of Ninjitsu; her entire
  clan was poisoned at a ceremonial meal and she is the sole survivor seeking the
  murderer. A real tragic backstory hiding behind a deathmatch sprite.
- **ChopperBoy** — 17, "M.I.T. student, sophomore," sins include "masturbation,"
  "Kicks ass at 'Quake'," "Has a serious 'thing' for Ninja," built the helicopter
  backpack to "impress the chicks," and was **killed by his own creation,
  Enforcer.**
- **Enforcer** — the payoff: it is *"ChopperBoy's entry for the M.I.T. 6.270 Lego
  Robot Competition.  Disqualified for use of non-regulation... military-grade
  quantum driver cores,"* whereupon it decided all humanity was the enemy, *"Killed
  85.3% of the contest attendants, including ChopperBoy,"* and went on a 23-hour
  rampage "through Cambridge and downtown Boston before destruction by
  Massachusetts National Guard forces." (6.270 is a real MIT LEGO-robot class.)
- **Walker** — a human brain (age 52) in a 1.3-year-old polycarbon chassis, built
  by "Chief Scientist Stephen Healy, Ph.D." at "Cruncheed Combat Technologies";
  project hit "97.32% of projected capacity... with only minor schedule and budget
  overruns." Blood type: "EM Radiation (753-872nm)."
- **Yeti** — "Good friends with Elvis and the Loch Ness Monster," does "occasional
  modeling work for fur coat manufacturers," quote "Mrrrrghhhh!" translated as
  *"Goddamn it.  I said no pictures."*, and died of **heat stroke** on an iceberg
  drifting into warm water.
- **Dog** — reveals the Mutt's origin: *"Dog is good friends with Mutt, a slightly
  slower, but tougher animal than Dog."* Cause of death: "Hit by car."
- **Dragon** — *"Segmented, capable of splitting into multiple independent
  sub-dragons"* (the profile documents the 15-segment composite the code
  implements); "Lord of fear, pain, and torment in the Earthly realm of Asia."

## The pharmacology of XEvil (the exact drug math)
Three drugs, three `Modifier` subclasses in `cmn/actual.cpp`, each a stack of
attribute multipliers. Timing calibrates against `PH_INVISIBLE_TIME 1500 //
One minute` (`physical.h:65`), i.e. ~25 turns/sec.
- **Crack** (`Crack::apply`, `actual.cpp:1627`; `CRACK_TIME 500` ≈ 20s): speed
  ×2.0, jump ×1.5, melee damage ×2, sets `HIGH`. Overdose chance **1-in-10** per
  use (`crack_pipe.bitmaps:70`, `odChance` checked at `physical.cpp:2732`, message
  *"dies from crack overdose."*).
- **Caffine** (`CaffineM::apply`, `actual.cpp:1702`; `CAFFINE_M_TIME 300` ≈ 12s):
  speed ×1.4, jump ×1.3, no damage boost. Overdose chance **0** — the only
  completely safe drug (`caffine.bitmaps` `odChance = 0`). The manual's *"little
  boost to get you going in the morning."*
- **PCP** (`PCPM::apply`, `actual.cpp:1774`; `PCP_M_TIME 350` ≈ 14s): the
  berserker. Incoming `CORPOREAL_ATTACK` is negated — *"Turn damage into
  superficial damage"* (impact/melee can't hurt you); `HEAT_ATTACK` set to 0 (fire
  immunity); melee damage ×2. **But**: overdose chance **1-in-6** (deadliest to
  even touch), and `PCPM::preDie` (`actual.cpp:1803`) *"Whack off half of health"*
  when it wears off — the comedown the Armory warns is "a real bitch." Everything
  the flavor text promised, in code.

## The Altar of Sin has two faces
Two entirely different mechanics on one object (`cmn/actual.cpp`):
- **Touch it as a human** (`AltarOfSin::collide`, `actual.cpp:1094`): it grants one
  random **blessing** from `Utils::choose(9)` (`actual.cpp:1117-1307`) and then
  consumes itself (`kill_self`). The nine: extra life, Double Speed, Double Jump,
  Double Health, Healing (regen), HellFire (become on-fire), Fireballs (a built-in
  fireball weapon), Flying, or Sticky (wall-crawl). Several are skipped if
  redundant, so you always get something useful. Buried in case 0 is a removed
  feature: a commented-out *"You Gain 5 Kills"* path with the note *"Everybody
  hates the 'You Gain 5 Kills' thing.  So, I removed it."* (`actual.cpp:1123-1133`).
- **Attack it** (`AltarOfSin::corporeal_attack`, `actual.cpp:1015`): a coin-flip
  either morphs you into a **Frog or a Baby-Seal**, or *"BLASPHMER!  ... loses
  health for daring to attack the Altar of Sin"* (drains all health). Either way
  the arena flashes *"Don't FUCK with the Altar of Sin."* [sic — the code comment
  at `1016` opens with *"Don't fuck with the Altar of Sin."*]

## The facehugger's ninety turns
`Hugger::collide`/`act` (`physical.cpp:8666-8775`). On latching to a valid
biological victim it **stuns itself** for `HUGGER_TIME` (90 turns ≈ 3.6s) and
stuns the **victim** for `HUGGER_TIME + 1` and marks them alien-immune for
`HUGGER_TIME + 2` — so during the whole gestation the victim is completely frozen
and nothing else can touch them. The hugger rides their face the entire time
(*"Guess where hugee's face is, upper-left + (width/2,height\*.3)"*). When its own
stun expires it spawns the Alien and kills both bodies. This is the ~3.6-second
window in which a hugged player can only watch. Red vs Green (whose mind the new
Alien gets) was covered in Part I.

## Coolness: the weapon-switch brain you never noticed
Every weapon and item has a `coolness` integer, and picking up gear silently
auto-selects your "coolest" one (`User::coolest_weapon`/`coolest_item`,
`physical.cpp:6324-6376`; only non-negative coolness is eligible). The values
(from each `.bitmaps`) explain years of muscle memory:
- Weapons: Auto-Lancer 11000 > Demon-Summoner 10100 > Dog-Whistle 10000 > Launcher
  8000 > Lancer 7000 > Stars 6000 > Pistol 5000 > Flame-Thrower 900 > Chainsaw 800
  > Frog-Gun 700.
- **Deliberately un-auto-equippable** (negative coolness): **Napalm Grenades −5,
  Grenades −10, Soul-Swapper −20.** The game will *never* auto-switch you to these
  — so you can't accidentally lob a grenade or swap souls the instant you grab one;
  you must select them by hand. Ammo weapons carry a separate `coolnessNoAmmo`
  (Dog-Whistle −30, Demon-Summoner −25) so they drop to the bottom once empty.
- Items: N-Shield 9000 > Med-Kit 8000 > T-Shield 7000 > Caffine 6500 > PCP 6300 >
  Crack 6000 > Doppel 900 > Cloak 800 > Transmogifier 90 > Bomb 80 — which is why
  grabbing a Bomb never bumps you off your Med-Kit.

## The Cloak and the blind machines
`Cloak` grants near-total invisibility for `PH_INVISIBLE_TIME` (1500 turns ≈ one
minute, per the comment). The part that matters: the enemy AI is **hard-coded to
ignore invisible things entirely** — both invisible creatures and invisible items
are skipped in the target scan (`intel.cpp:1831-1832`, `1857-1858`,
`!is_invisible()`). A cloaked player is not merely hard to see; to a bot they do
not exist. The Armory's rule "You may not fire when cloaked" is the balancing cost.

## Corpses decompose; rocks need momentum
Two physics details a careful player feels but never reads:
- **Corpses** persist `CORPSE_TIME` (140 turns ≈ 5.6s) then *"corpse has
  decomposed."*; but you can gib one early by dealing enough extra damage —
  `get_health() < -corpseHealth` → *"corpse has been destroyed."*
  (`physical.cpp:4439-4451`). Bodies fall (`DROP_SPEED 2`) and never collide.
- **Thrown/falling weights and rocks** only hurt if moving fast enough: `Heavy`
  requires downward velocity `dy >= HEAVY_DAMAGE_VEL` (3) *and* the target below or
  inside it (`physical.cpp:2308-2322`). A rock at rest is harmless; kill credit
  goes to the last "pusher." Only certain bodies can even throw — `throwSpeed` is
  8 for Hero but **0 for ChopperBoy** ("ChopperBoy can't throw",
  `chopper_boy.bitmaps:232`), and only when holding no weapon.

## 666, everywhere
Beyond the two Hell ranks, the number of the beast is woven through the engine as
sentinel and filler: `intel.cpp:272` sets a dead brain's lives to `-666` with the
comment *"This should never be used."*; the world serializer writes `666` as
literal padding, twice (`world.cpp:1047,1054`, *"// padding"*). Even the
network wire format is a little satanic.

## The Story that was never written
There is a "Story" menu item (`ID_STORY`, `resource.h:1124`) wired to
`UiPlayer::doStory()` (`uiplayer.cpp:2252`), which opens `CStoryDlg`. But
`win32/storydlg.cpp` is an **empty ClassWizard stub** — constructor, DoDataExchange,
and an empty message map, and nothing else (`storydlg.cpp:39-61`). There is no
dialog template for `IDD_STORY` in the resource script either. The premise lives
only on the website ("You have sinned in life.  Now, you die and go to Hell").
The in-game Story screen was scaffolded and never filled — a blank page you can
still open.

## The shareware that almost was
`win32/aboutdialog.cpp:127` carries a commented-out line:
`// m_versionvalue += "  (Shareware)";`. It is not a joke — Frank Wu's surviving
mirror page states it plainly:
> "Since XEvil and the XEvil source code are shareware, please send a $20 check or
> money order made out to 'Steve Hardt' at XEvil, P.O. Box 391530, Mountain View,
> CA 94039-1530"
(`web.archive.org/web/20021201162153/http://www.frankwu.com/xevil.html`). So XEvil
shipped as $20 shareware before/alongside the GPL, and by ~2000 Hardt had left
Cambridge for **Mountain View, CA** (Silicon Valley). The "(Shareware)" tag in the
About box was disabled when the game went GPL, but the code remembers.
Bonus artifact: the sound effects were edited in **GoldWave** — the shipped
`xevil.exe` still contains the WAV chunk *"File created by GoldWave.  GoldWave
copyright (C) Chris Craig."*

## Who wrote what (attribution by dialect)
The two authors have unmistakable, countable fingerprints. Hardt writes function
contracts (`EFFECTS:` / `MODIFIES:` / `REQUIRES:`) and camelCase; Judge writes MFC
Hungarian (`m_`, `p`, `CDialog`, `DDX_`, `BEGIN_MESSAGE_MAP`). Counting across the
trees:
- **`cmn/` (the engine)**: 791 contract lines vs 29 MFC tokens → **Steve Hardt**,
  essentially wholesale. This is the ~40kloc cross-platform game.
- **`win32/` (the Windows UI)**: 1329 MFC tokens vs 124 contract lines →
  **Michael Judge**, with Hardt's fingerprints concentrated in the cross-platform
  glue/sound files (`s_man`, `xdata`, `draw`, `wheel`, `fsstatus`, `ui`) — matching
  the credits' "Windows Front End: Michael Judge, Steve Hardt."
- **`x11/` (the UNIX UI)**: contract-heavy → **Hardt** (the "UNIX Front End").
This exactly corroborates the credits screen buried in the license text
(`cmn/l_agreement.cpp:182-227`): Design/Architecture/Cross-Platform/UNIX = Hardt,
Windows Front End = Judge + Hardt, Sound = Judge, Artwork = Comrade.Cid + Hardt +
Judge + frankwu.com + five others, Music = Ann Greyson / Michael Cummings /
D.J. Litany. That credits block ships as one giant C string literal with a
gloriously exasperated comment about keeping the columns aligned across X11's
fixed-width and Win32's variable-width fonts: *"Ohhhh!!  This is horrible.  Putting
an #ifdef here..."* (`l_agreement.cpp:176-181`). (The "lawyer text" that the
agreement dialog word-wraps at runtime, via the hand-rolled `Line`/`Page` engine,
is simply the verbatim GPLv2 — not encoded or obfuscated, just a long literal
`get_text()` string, `l_agreement.cpp:155-514`.)

## The machine-name graveyard (config.mk)
`config.mk` is a stratigraphic record of every UNIX box XEvil was ever built on,
each architecture target tagged with the actual host's nickname in a comment or an
embedded home-directory path. A catalog of the fossils:
- Named boxes (comments): `#gargamel` (`alpha-gargamel-old`), `#russia`
  (`hp9000s700`), `#chainsaw` (`iris.old`), `#For lancer running Linux` (`i386`),
  `#for devastator and truth` (`i386-linux-old2`), `#jordan` (`rs6000jordan`,
  with *"static doesn't work on jordan"*), `#iron` (`rs6000`), `#scar`
  (`sun4-scar`), `#was acland, now worms` and `#now, mocha` (`solaris`),
  `#openwound` (`openwound-sun4`), `beepbeep-sun4`, `vision-sun4`, `jsc-sun4`.
  Two of the machine names — `chainsaw` and `lancer` — are also weapons in the
  game.
- Personal-account fossils in include/lib paths: **`/u/mjudge/xevil/xpm/...`**
  (Michael Judge's account, appearing in the alpha/hp/iris/aix/solaris targets —
  he maintained the XPM library builds) and the packaging pointer
  **`SRC_DIR = /mit/hardts/src/X/xevil1.4.9`** (Steve Hardt's MIT directory).
- **MIT Athena traces**: `athena-sun4` / `athena-sun5` targets, and
  `INCL_DIRS="-I/mit/gnu/lib"` — the game was still being compiled on Athena's
  Suns. (The modern `x86_64`, `arm`, `powerpc`, `darwin`, and `debug` targets and
  the auto-detecting `default:` rule are 2.5-era additions, not fossils.)

## The version labyrinth, reconstructed
Three different "versions" disagree in this tree, and the reason is historical
layering:
- `config.mk:42` says `VERSION = 2.1` (this is the value `Game::get_version_string`
  returns on UNIX, `game.cpp:1113-1114`) — yet the game everyone calls "2.02."
- `config.mk:45` still points `SRC_DIR` at `xevil1.4.9`.
- The shipped `xevil.exe` About box reads `2.02`.
Piecing together the strings and the recovered archive downloads, the real
timeline is:
1994‑07‑26 first release on MIT Athena (X11, monochrome, a learn-C++ project) →
UNIX 1.x line, reaching **1.4.9** (the `SRC_DIR` fossil), **1.5.1e**, and **1.5.5**
(the version shipped on the "100 Great Linux Games" CD) → Windows port with Judge,
whose earliest surviving binary is dated **Jan 16 1999** and whose May 1999 source
already carries `VERSION = 2.01` internally → public **GPL release Jan 18 2000** as
2.0 → 2.0r2, 2.01, 2.02, 2.02r2, with **2.02 dated July 26 2000 — the game's sixth
birthday, to the day.** The `config.mk` `VERSION` field was an internal/packaging
number that drifted independently of the public label the whole time; the `2.1`
you see is that drift, frozen. (The netcode protocol string tells the same story
from another angle — the original 2.02 exe advertises `XETP1.00`; the protocol was
held stable across the entire 2.x line. The 2.5 tree deliberately breaks it with
`XETP2.5X`, `xetp_basic.cpp:36`, so 2.5 clients and 2.02 servers can't handshake.)

## The built-in chaos monkey (`-buggy`)
`UDPOutStream::buggy_tests` (`streams.cpp:539-594`), enabled by the undocumented
`-buggy` flag, is a deliberate netcode fuzzer: for every outgoing packet it will,
by dice, **drop it (50%)**, **send 2–6 duplicate copies (~1/30)**, replace it with
a random-length garbage packet, truncate it, append 1–20 junk bytes, or twiddle a
few bytes at random — each logging what it did. It's proof the XETP transport was
engineered to survive hostile networks (loss, duplication, reordering, corruption),
and a lovely window into how a one-person engine got its reliability tested in
1999. Relatedly, `x11/serverping.cpp` is a whole standalone `ping`-for-XEvil-servers
utility (sends `SERVER_PING`, times the `SERVER_PONG`); `SERVER_PING` is notably
the *only* packet type the server will accept from an unknown address
(`role.cpp:2973-2975`).

## The Death March: a definitive burial
Part I flagged `IDM_DEATHMARCHSOUNDTRACK 1631` as the only trace of a tenth,
missing music track. This dig settles it. I checked every reachable build:
- v2.02 shipped `xevil.exe`: MIDI resources 1630, 1632–1639 — **nine**, no 1631.
- The **Jan 16 1999** Windows binary (recovered from
  `xevil.com/download/xevil2.0.windows/xevil2.0.windows.exe`): same nine, no 1631.
- The **May 28 1999** experimental Windows binary
  (`download/exp/xevil052899.windows.zip`): same nine, no 1631.
- The **May 28 1999 source package** (`download/developer/xevilsrc052899.zip`, the
  oldest source that survives): nine `.MID` files in `win32/res/` and no
  `deathmarch.mid`; and its `resource.h` already gives ID 1631 to **two** wall
  bitmaps (`IDB_MD5WALL`, `IDB_MD4WALL`) *in addition to* the death-march menu ID.
So by the earliest artifact that exists anywhere, the Death March had already been
cut and its ID recycled. There is no file to recover — it was gone before the tape
starts. The nine survivors, mapped (`resource.h:1008-1026` ↔ `win32/res/`):
BABYSEAL, FIRE, HIVE, KKKILL, ZEEPEEG, NIGHTSKY, SWEETDAR, TERRAEXM (the "banished"
one), NEWSONG. Sources checked are all on `web.archive.org/*/xevil.com/download/*`.

## XDeathlord, the ancestor
XEvil was not Hardt's first X11 game. `xevil.com/download/xdeathlord/` archived
**XDeathlord 1.2** (Linux binary + source, GPL'd the same Jan 18 2000). Per the
retrospectives it is *"the same style as XEvil, but in a vehicular combat mode
instead of a personal combat mode"* — the vehicular-combat prototype whose engine
ideas grew into XEvil's creatures. XEvil is the sequel to a game almost nobody
remembers.

## What became of xevil.com
For the owner's closure: the game was still officially downloadable as late as 2018
(`web.archive.org/web/20180613141954/http://www.xevil.com/download.html` lists 2.02
for "Windows 7, XP, 2000, NT, 98, 95", Unix/X, and Mac OS X). Some time after, the
`xevil.com` domain was taken over by an unrelated commercial "XEvil" CAPTCHA-solving
product (the "3.0.4.123" you'll find on download sites is *that*, not the game — no
Java rewrite of the game ever shipped). The original artwork survives on the mirror
of **Frank Wu** — a four-time Hugo Award-winning science-fiction artist — who drew
the Character Profile portraits, the Ninja, the machine guns and background tiles
(`frankwu.com/xevil.html`, credited in-game as "frankwu.com"). Comrade.Cid, the
lead pixel artist, signs the credits and appears as the final AI name "Cid";
identity otherwise undocumented.

## The two orphan sounds, measured
Part I noted `FROG.WAV` and `WOOB.WAV` sat in `win32/res/` wired to nothing.
Measured (numpy, `win32/res/`): **FROG.WAV** is 0.37s, 11025 Hz, **8-bit** mono —
a short, gritty, three-pulse mid-band croak ("rib-bit-bit"), the lo-fi bit depth
giving it a deliberately cheap texture. **WOOB.WAV** is 0.25s, 11025 Hz, **16-bit**
mono — a punchy, cleaner two-part downward "woob." The happy ending: in the 2.5
revival, WOOB finally has a job — it voices the new **Singularity** weapon
(`actual.cpp:3257-3258`, with the comment *"The long-orphaned WOOB finally gets a
job."*).

## The artist's working files (win32/res)
Rendering the oddly-named BMPs (PIL) shows they're not orphans but working sprite
frames and tiles, and they preserve the artists' file-management hacks: `27A.BMP`,
`27b.bmp`, `39A.BMP` are Hero animation frames (kneeling, punching, standing —
green fatigues, blond, magenta `#FF00FF` transparency key). `AAANEW7.BMP` (a
machinery/turret tile on hazard stripes) and `!DOG0_RU.BMP` (a cheerful white
cartoon dog, tongue out) use leading `AAA`/`!` so the work-in-progress files sort
to the top of the folder — the 1990s equivalent of pinning a tab. Nothing hidden,
but a nice human trace of how the art actually got made.
