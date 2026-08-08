# qmk_userspace

QMK keymap for a [Boardsource Lulu](https://boardsource.xyz/products/lulu) (RP2040 split).

An [External Userspace](https://docs.qmk.fm/newbs_external_userspace) repo — it
holds only the keymap and builds against an unmodified upstream `qmk_firmware`,
so pulling a new QMK release is just `git pull` in the firmware checkout.

**[Design notes](NOTES.md)** — the reasoning, the corrections and the traps.

**[Theme preview](docs/theme-preview.html)** — every theme drawn on the board's
real LED map, with live hue/speed sliders. Self-contained: open the file, no
network or account needed. Also hosted at
[claude.ai/code/artifact/bf5a7015…](https://claude.ai/code/artifact/bf5a7015-7957-434d-be30-7cb2169511d4)
([redirect](docs/theme-preview-hosted.html)), though that copy is a private
artifact and only opens for Taylor's account.

---

## Build and flash

```sh
qmk userspace-compile                          # everything in qmk.json
make boardsource/lulu/rp2040:twdickson         # one target
```

To flash: **`QK_BOOT`** on `_ADJUST` (far top-right) reboots the half holding
the cable into its bootloader, which mounts as `RPI-RP2`. Copy the `.uf2` over.

Only the plugged-in half reboots, so **both halves means two passes** — flash
one, move the cable, flash the other. Same file for both.

> The PCB reset button is **not** a substitute — it just restarts the firmware.
> If a half ever won't boot far enough to honour a keycode, hold the **top outer
> key while plugging the cable in** (bootmagic). That path needs no working
> keymap.

---

## Layers

| layer | reached by | holds |
| --- | --- | --- |
| `_QWERTY` | default | alphas, number row, pinky shifts, media |
| `_GAME` | `GAME_TOGGLE`, persisted | plain shifts, no left GUI |
| `_LOWER` | left thumb | F1–F12, arrows, backtick, Caps Word, Del |
| `_RAISE` | right thumb | F13–F24, navigation, editing, Ins |
| `_ADJUST` | **both** thumbs | lighting, themes, mode toggles, `QK_BOOT` |

### `_LOWER` — left thumb

Backtick sits here because `QK_GESC` can't produce one: it only emits grave
while Shift or GUI is held, and leaves that modifier applied. So Escape is
unmodified, `~` is Shift+Escape, and a bare `` ` `` is `LOWER`+Escape.

F1–F12 across the top, arrows on the right hand (`IJKL` positions), `Del` on the
right thumb, and `CW_TOGG` on the `A` key — one layer below where Caps Lock
lives, because it's the one you reach for mid-word.

### `_RAISE` — right thumb

Right hand is navigation: `Home` `End` `PgUp` `PgDn`. Left hand is what you do
to what it lands on, each on the letter its shortcut already uses:

| | | | |
| --- | --- | --- | --- |
| `Z` Undo | `X` Cut | `C` Copy | `V` Paste |
| `B` Redo | `A` Select All | `S` Save | `F` Find |
| `G` Find Next | | | |

A right thumb replaces the left pinky reaching for Ctrl, so a same-hand stretch
becomes a two-hand roll. **These follow the Mac/Windows toggle automatically** —
they're written Ctrl-based and `CG_TOGG` swaps them to Cmd.

F13–F24 continue `_LOWER`'s F-keys. Nothing binds them on either OS, which is
what makes them useful for IDE and window-manager shortcuts.

`Ins` on the right thumb, mirroring `_LOWER`'s `Del`.

### `_ADJUST` — both thumbs

```
 Caps  THM+  HUE+  SAT+  VAL+  SPD+          ·    GAME   ·     ·     ·   BOOT
  ·    THM-  HUE-  SAT-  VAL-  SPD-          ·    CG_T   ·     ·     ·     ·
                              RM_TOGG                              Ctrl+Alt+Del
```

Four lighting columns in the same order as the four bars on the OLED panel.
`RM_TOGG` (the left inner key) kills the lighting entirely. `GAME_TOGGLE` and
`CG_TOGG` sit together on the right — the two mode switches, and the two things
the bottom of the panel reports.

### Encoders

| layer | left knob | right knob |
| --- | --- | --- |
| base | volume | volume |
| `_LOWER` | **theme** | brightness |
| `_RAISE` | saturation | hue |
| `_ADJUST` | speed | — |

Press the left knob for Mute. Press the right knob for media: **one tap**
play/pause, **two** next track, **three** previous — like a headphone button.
(There's a 300 ms delay before play/pause fires, because a single tap can't be
dispatched until it's known not to be the first of two. `_RAISE`'s inner keys
are prev/next track with no delay.)

---

## Themes

`THEME_NEXT` / `THEME_PREV` on `_ADJUST`, or hold `LOWER` and turn the left
knob. Seven of them:

| theme | deck | underglow |
| --- | --- | --- |
| **Lulu** | amber, with a ring expanding out from every keystroke | hot amber |
| **Rose** | rose alphas, red mods — the board's original look | rose |
| **Mono** | warm white alphas, cool white mods. Colour means information | white |
| **Fold** | hue ramps outward from each half's inner edge, both hands identical | violet |
| **Zones** | colour by finger — pinky, ring+middle, index, thumbs. Dead still | teal |
| **Trail** | each key flares on the strike and fades back over about a second | violet |
| **Pulse** | deck held flat, the outline breathes | cyan |

**A theme is a starting point, not a fixed look.** Picking one lays down its
hue, saturation, speed and brightness; the knobs and keys adjust from there and
**your adjustments persist** — the board comes back up looking like the version
you dialled in, not the table's.

`speed` is never a rate. It means:

| theme | `speed` controls |
| --- | --- |
| Rose, Mono | the hue gap between alphas and mods |
| Fold | the hue span from the inner edge out to the pinkies |
| Zones | the hue step between finger groups |
| Lulu | ripple rate |
| Trail | how fast a key fades back |
| Pulse | breathing rate |

### What lights up when

- **`_LOWER` / `_RAISE` / `_ADJUST`** light only the keys that layer actually
  defines, in that theme's accent colour — so a held thumb shows you what's
  available.
- **`_GAME`** turns the underglow green, in every theme.
- **Caps Lock** is red on the key that toggles it, in every theme.
- **Caps Word** lights a shift — one shift, on the half holding the cable.
- **The encoder knobs** stay a fixed warm colour on every layer and every theme.
  They're the only translucent caps on the board, so they're always findable.

---

## The displays

**Master** (whichever half has the cable) — a starfield, with a ship that pulls
ahead as you type faster. Shows a `GAME` banner while `_GAME` is on.

Hold **both thumbs** and it becomes a config readout:

```
   THEME
 ▉ LULU  ▉
 ──────────
   HUE  ▓▓▓░░░░░
   SAT  ▓▓▓▓▓▓▓▓
   VAL  ▓▓▓▓▓░░░
   SPD  ▓▓▓░░░░░
 ──────────
    MAC          <- which machine the keymap thinks it's on
    BASE         <- BASE or GAME
```

`MAC`/`WIN` is the single most useful line: `CG_TOGG`'s setting is otherwise
invisible until you press a key and get the wrong modifier.

**Secondary** — a shift gate. The two layers you *toggle and stay in* sit above
the line, the three you *hold* below it, and a lit block slides between them:

```
      GAME
    ▌ BASE ▐        <- detents mark the engaged default layer
    ────  ────      <- the gate
     [LOWER]
      RAISE
       ADJ
```

---

## Idle

| after | |
| --- | --- |
| 30 s | lighting starts fading |
| 60 s | lighting off, displays off |
| host sleeps | everything off immediately |

Both halves fade and sleep together. Brightness is **not** part of a theme — it
survives a theme change and a reboot, because it's a room-lighting preference
rather than a palette choice.

---

## Conventions

- `CONSOLE_ENABLE` stays `no` — it adds a HID interface and makes every
  `uprintf` a USB transfer.
- **VIA stays off.** With it on the live keymap is the EEPROM copy rather than
  this source, and the two only resync when VIA's build-date magic changes.
- Keymap directory names must match `^[0-9a-z_]*$`, which is why it's
  `twdickson` and not `TWDickson`.
- Commits here can't be signed non-interactively — 1Password's git signer needs
  a desktop unlock. Use `-c commit.gpgsign=false`.

Pushing to GitHub lints, builds every target in `qmk.json`, and attaches the
`.uf2` to a release.
