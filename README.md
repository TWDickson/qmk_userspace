# qmk_userspace

QMK keymap for a [Boardsource Lulu](https://boardsource.xyz/products/lulu) (RP2040 split).

This is an [External Userspace](https://docs.qmk.fm/newbs_external_userspace) repo:
it holds only the keymap, and builds against an unmodified upstream
`qmk_firmware`. Nothing here forks QMK, so pulling a new QMK release is just
`git pull` in the firmware checkout.

## Build

```sh
qmk config user.qmk_home=/path/to/qmk_firmware
qmk config user.overlay_dir=/path/to/qmk_userspace
qmk userspace-compile
```

Or a single target: `make boardsource/lulu/rp2040:twdickson`.

Pushing to GitHub lints, builds every target in `qmk.json`, and attaches the
`.uf2` to a release.

## Flashing

`QK_BOOT` on `_ADJUST` — far top-right, so it takes both thumbs and a corner key
to reach by accident — reboots the half holding the USB cable into its RP2040
bootloader, mounting `RPI-RP2`. Copy the `.uf2` over.

It only ever affects the plugged-in half, so flashing both means moving the
cable and doing it again. Both halves take the same file.

**The reset button on the PCB is not a substitute.** Double-tapping it enters
the bootloader on RP2040 boards that set `RP2040_BOOTLOADER_DOUBLE_TAP_RESET`,
and `boardsource/lulu` does not — check `keyboards/boardsource/lulu/rp2040/`
before believing otherwise. Pressing it just restarts the firmware. The board's
own readme says otherwise because it also covers the AVR build, where the Pro
Micro's Caterina bootloader does work that way.

So the fallback, if a flash ever leaves a half that will not boot far enough to
honour a keycode, is bootmagic: **hold the top outer key while plugging the
cable in** (`"bootmagic": true` is set in the board's `info.json`). That path
needs no working keymap, which is exactly when you need it.

## Layers

| layer | reached by | holds |
| --- | --- | --- |
| `_QWERTY` | default | the alphas, number row, pinky mod-tap shifts, media |
| `_GAME` | `GAME_TOGGLE`, persisted | three overrides; everything else falls through |
| `_LOWER` | left thumb | F1-F12, arrows, backtick, Caps Word, Del |
| `_RAISE` | right thumb | F13-F24, navigation, editing actions, media, Ins |
| `_ADJUST` | both thumbs (tri-layer) | lighting, themes, Caps Lock, `QK_BOOT`, Ctrl+Alt+Del |

### `_GAME` states only what it changes

Plain shifts instead of mod-taps, so a held shift never decides it was a tap and
emits a bracket mid-fight, and no left GUI, so the desktop never gets yanked
away. Everything else is transparent.

That works because `layer_switch_get_layer()` falls back to layer 0
unconditionally when every active layer is transparent, and `_QWERTY` is layer
0 — so the rest resolves against it even though `_GAME` is the active *default*
layer and layer 0's bit is not set. Edits to `_QWERTY` carry into `_GAME` for
free.

The same fallback applies to `encoder_map`, which is why `_GAME`'s row there is
all-transparent rather than a copy of `_QWERTY`'s. It cannot be *deleted* — a
missing designated initializer would zero-fill to `KC_NO` and block the
fallthrough rather than allow it.

While `_GAME` is active the master OLED shows a "GAME" banner and the underglow
takes the theme's game tint.

### `_RAISE` — navigation, and what you do to what you navigated

The right hand is the navigation cluster (`Home`/`End`/`PgUp`/`PgDn`). The left
hand is what you do to what it lands on:

| key | does | key | does |
| --- | --- | --- | --- |
| `Z` | Undo | `A` | Select All |
| `X` | Cut | `S` | Save |
| `C` | Copy | `F` | Find |
| `V` | Paste | `G` | Find Next |
| `B` | Redo | | |

Nothing new has to be memorised — every one of those sits on the letter its
shortcut already uses. The only change is that a right thumb replaces the left
pinky reaching for Ctrl, which turns a same-hand stretch into a two-hand roll.

**These are written Ctrl-based on purpose.** `keymap_common.c` runs `QK_MODS`
keycodes through `mod_config()`, which is exactly where `CG_TOGG`'s swap
happens, so every one of them becomes Cmd+*key* the moment the board is
switched to macOS. One definition covers both machines.

They are prefixed `ED_` rather than named `KC_UNDO` and friends because those
are already real HID usages in QMK — and are ignored by most software anyone
actually uses, which is why these are modifier combos instead.

Redo is `Ctrl+Shift+Z`, not `Ctrl+Y`. It is the spelling that survives the
`CG_TOGG` swap into a working macOS shortcut; `Ctrl+Y` would become Cmd+Y,
which is not redo anywhere.

The rest: media pairs across the inner keys the way the base layer does (Mute
and Play there, previous and next track here), `Ins` mirrors `_LOWER`'s `Del` on
the same thumb, and F13-F24 continue `_LOWER`'s F1-F12. Nothing binds F13-F24
by default on either OS, which is what makes them useful — free targets for IDE
and window-manager shortcuts that cannot collide with anything.

### The media key counts taps

The right inner key is a tap dance, the way a headphone button works:

| taps | does |
| --- | --- |
| 1 | Play / Pause |
| 2 | Next track |
| 3+ | Previous track |

Mute stays on the left inner key, and volume stays on the encoders.

The cost is inherent: nothing can fire until the tap window closes, so
play/pause lands 300 ms after the tap rather than instantly. A single tap cannot
be dispatched before it is known not to be the first of two — a pair of earbuds
has exactly the same delay. `_RAISE`'s inner keys stay mapped to previous and
next track as the instant, no-guessing path.

300 ms rather than the board's 200 ms `TAPPING_TERM`, because 200 is tight for
three taps. `get_tapping_term()` returns the longer window for this one keycode
only — the pinky shifts keep the term `config.h` sets, which the whole tap-hold
section below depends on.

### Grave, tilde and escape

`QK_GESC` sends grave whenever Shift **or GUI** is held, and leaves that
modifier applied. So:

| want | press |
| --- | --- |
| `Esc` | top-left, unmodified |
| `~` | Shift + top-left |
| `` ` `` | `LOWER` + top-left |

`KC_GRV` on `_LOWER` is therefore **not** redundant — no combination of
`QK_GESC` yields a bare backtick, because the modifier that triggers grave is
still in the report. Do not remove it.

The same behaviour silently ate two shortcuts that reach for Escape with a
modifier already down: Cmd+Opt+Esc (Force Quit, macOS) arrived as Cmd+Opt+`` ` ``,
and Ctrl+Shift+Esc (Task Manager, Windows) as Ctrl+Shift+`` ` ``.
`GRAVE_ESC_ALT_OVERRIDE` and `GRAVE_ESC_CTRL_OVERRIDE` force Escape whenever
Alt or Ctrl is held, fixing both. The Shift and GUI overrides are deliberately
left off, since those would cost `~` and macOS's Cmd+`` ` `` window cycling.

### Caps Word

`CW_TOGG` on `_LOWER`, on the same physical key that carries Caps Lock on
`_ADJUST` — one layer down, because unlike Caps Lock it is something you reach
for mid-word.

Tap it and the next word is capitalised: letters get shift, digits and `-` `_`
pass through, Backspace still works inside the word, and the first space or
punctuation ends it. Better than Caps Lock for `SCREAMING_SNAKE_CASE` mostly
because it cannot be left switched on.

### Tap-hold

The pinky shifts are mod-taps, so the tap-hold settings are what keep them from
emitting a bracket mid-word:

- `PERMISSIVE_HOLD` — a nested press-and-release inside the hold settles as
  hold, which is the shape of a capital. Rolling out of the shift first still
  taps, so brackets are unaffected.
- `QUICK_TAP_TERM 0` — no auto-repeat of the tap keycode, so `[` then shift
  gives shift rather than `[[[[`.
- `SPECULATIVE_HOLD` — applies the modifier on keydown and retracts it on a
  tap. Defaults to Shift/Ctrl mod-taps, which is exactly what this keymap has.

`CHORDAL_HOLD` is deliberately **not** set: it settles same-hand chords as taps,
which is correct for home row mods but wrong for a pinky shift, where
`Shift+Q` / `Shift+A` / `Shift+Z` are ordinary and would come out as `[q`.

## Lighting

`rgb_theme.c` holds a small table of themes. A theme is the resting look of the
board — animation, hue, saturation — plus the palette its indicators draw from,
so that layer and Caps Word feedback lands in a colour that belongs with the
base instead of clashing with it. `THEME_NEXT` on `_ADJUST` cycles them.

| theme | mode | deck | underglow |
| --- | --- | --- | --- |
| Lulu *(default)* | `SOLID_COLOR` + ripples | lit amber, overlapping rings out from each keystroke | hot amber |
| Rose | `ALPHAS_MODS` | rose alphas, red mods — what this board booted into before any of this | rose |
| Deep | `SOLID_COLOR` + ripples | lit blue, one ring at a time | cyan-blue |
| Ember | `GRADIENT_UP_DOWN` | deep red at the number row easing to amber under the thumbs | deep red |
| Mono | `ALPHAS_MODS` | warm white alphas, cool white mods | white |
| Split | `GRADIENT_LEFT_RIGHT` | one gradient across both halves — blue left hand, violet-rose right | violet |
| Sweep | `BAND_SAT` | pale deck at full brightness, saturated teal band travelling across it | teal |

### What the light actually lands on

Three surfaces, and every theme is built out of the difference between them.

The **deck** (`LED_FLAG_KEYLIGHT`, five columns a half) lights a **charcoal
plate** between **opaque greige keycaps**. It is a low-contrast wash —
atmosphere and a rough "where is the board", not illumination. Desaturated
colours read as lit grey and vanish, so the deck wants saturation.

The **underglow** (`LED_FLAG_UNDERGLOW`, six a half) washes nothing. It escapes
as a thin line between plate and **black** bottom case and reads as a crisp neon
outline against the desk — by a distance the most visible thing on the board.

> An earlier version of this file said the underglow lights an "amber
> underside… a fixed colour nothing in firmware can change", and built a
> warm-underglow-only rule on it. **There is nothing amber on this board except
> the encoder knobs.** Being an outline against black rather than a wash over
> amber, the underglow takes any hue cleanly — cool underglow does not go muddy,
> it goes neon. "Deep", "Split" and "Sweep" are all built on that.

The **encoder knobs** are translucent amber and are the *only* shine-through
parts on the board — every keycap is opaque, so these two LEDs are the sole
per-key light seen as light rather than as plate glow. They are held at a fixed
warm colour instead of being left to the animation, so whatever the deck is
doing the volume controls are lit: the one control you reach for without looking.
The amber body multiplies whatever it is given — warm hues blaze, cool hues go
dead brown — so `knob_hue` stays warm in every theme, and a *low* `knob_sat` is
brighter than a high one because the material supplies the colour.

### Every theme rests lit — and no stock reactive effect does

This is the load-bearing one, and the firmware used to have it backwards.

`SOLID_SPLASH` and `SOLID_MULTISPLASH` do **not** add their flare to the resting
deck. Their runner opens each LED with

```c
hsv_t hsv = rgb_matrix_config.hsv;
hsv.v     = 0;                      // effect_runner_reactive_splash()
```

and only then accumulates hits, so the `qadd8(hsv.v, 255 - effect)` inside
`SOLID_SPLASH_math` adds ripples to *each other*, never to the resting
brightness. About 1.3 s after the last keystroke — once every front has passed —
the deck is black. `effect_runner_reactive()`, which drives
`SOLID_REACTIVE_SIMPLE`, reaches zero the same way via `scale8(255 - offset, v)`
with `offset` clamped at 255. **The distinction this repo used to draw between
the two — "splashes add, reactive scales" — does not exist. Both rest dark.**

So the reactive themes are `SOLID_COLOR` with the ripples overlaid in the
indicator hook instead. Core lays the resting deck every frame;
`splash_overlay()` touches only the LEDs with a live front over them and adds to
that resting value the way the old comments always claimed:

```c
v = qadd8(resting_val, ripple);   // splash_overlay()
v = scale8(ripple, hsv.v);        // upstream
```

which is also why the flare still peaks at full with the brightness dialled right
down for a dark room. The ripple maths is upstream's, unchanged. `SPLASH_ONE`
ripples from the newest hit only, so each keystroke cancels the last ring;
`SPLASH_ALL` ripples from every live hit and lets a fast line overlap its own.
All is the default theme's, one is Deep's.

Inverting it this way costs **12 bytes of flash net**, because dropping
`ENABLE_RGB_MATRIX_SOLID_SPLASH` and `ENABLE_RGB_MATRIX_SOLID_MULTISPLASH` also
drops their code, and pays for two extra themes besides.

### Reactive lighting still needs two things the board does not enable

`RGB_MATRIX_KEYPRESSES` is what defines `RGB_MATRIX_KEYREACTIVE_ENABLED`, which
is what brings `g_last_hit_tracker` into existence — the ring of recent key
positions and ages `splash_overlay()` reads. It costs that buffer in RAM.

`SPLIT_TRANSPORT_MIRROR` is the other half. Only the master runs
`process_record`, so without the mirror the slave's `last_hit_buffer` stays
empty and that whole side of the board sits inert while the other one flares.
This config used to leave it off on the grounds that nothing on the secondary
half needed the master's key events — **that reasoning no longer holds.** It
costs a matrix's worth of link traffic per scan and +100 bytes of flash.

**Accents lean on the keycaps, not the palette.** This board has a blue `LOWER`
cap, a peach `RAISE` cap and a red `Escape`, so wherever it stays legible the
lit keys match the cap of the thumb key being held, and Caps Lock red matches
the Escape.

*Where it stays legible* is the binding half of that rule. An accent within ~30
hue of the deck it is drawn on is invisible — "Lulu" used to accent `_RAISE` at
43 on a deck of 24, which is a yellow layer indicator on an amber board. The
warm-decked themes therefore give the cap match up and take all-cool accents,
and "Deep" gives `_LOWER` up to magenta because its deck has taken the blue.

`speed` is not a rate in any theme here — every mode in the table reads it as
something else, which is the first thing to check when a theme looks wrong:

| mode | what `speed` means |
| --- | --- |
| `ALPHAS_MODS` | hue offset applied to `LED_FLAG_MODIFIER` LEDs |
| `GRADIENT_UP_DOWN` | hue **span** top row → thumbs, `scale8(64, speed) * 4` |
| `GRADIENT_LEFT_RIGHT` | hue **span** across both halves, `scale8(64, speed) * 224 >> 5` |
| `BAND_SAT` | sweep rate (the one genuine rate) |
| `SOLID_COLOR` + ripples | ripple rate, read by `splash_overlay()`; core ignores it |

Two of those were being read as rates and were wrong for it. "Rose" ran
`ALPHAS_MODS` at 128, putting its mods 112 hue from its alphas — a green fringe
on a rose board; it is 20 now, which wraps 240 to 4 and puts red under the red
`Escape`. "Ember" ran `GRADIENT_UP_DOWN` at 64, a span of 64 that landed the
thumb row on hue 72 — a chartreuse bottom edge on the theme whose entire idea is
that everything is warm; it is 28 now, 4 → 32.

On this board `LED_FLAG_MODIFIER` is exactly the outer pinky column plus all
four thumbs: the eight keys a half that carry the coloured caps. That is what
makes `ALPHAS_MODS` worth two of the seven themes.

The board's own `info.json` compiles six animations. Four of them —
`GRADIENT_LEFT_RIGHT`, `BREATHING`, `BAND_SAT`, `BAND_VAL` — went unused for a
long time, i.e. paid for in flash and not spent. "Split" and "Sweep" are built
on two of them for nothing. `BREATHING` and `BAND_VAL` stay unused on purpose:
both band *value* and take the deck to black at the trough, which is the one
thing every theme here exists to avoid. `BAND_SAT` bands saturation and leaves
value alone, which is why "Sweep" can move without ever going dark.

QMK hue is 0-255 across the whole wheel, not 0-360: `hsv_to_rgb()` picks its
sextant with `h * 6 / 255`, so 0 is red, 85 green, 170 blue, 213 magenta.
**240 is rose**, which is what the board has actually been lighting up as.

**Only the theme index is persisted.** Everything else is re-derived from it at
boot with the `_noeeprom` setters, which keeps the EEPROM write budget for the
one byte that matters. Brightness is deliberately *not* part of a theme — it is
a room-lighting preference, not a palette choice — so `RM_VALU`/`RM_VALD` still
own it and it survives a theme change.

Indicators, all scaled by the live value so they dim out with the idle ramp
rather than sitting at full brightness on a sleeping board:

- **`_LOWER` / `_RAISE` / `_ADJUST`** light only the keys that layer actually
  defines. `KC_NO` is 0 and `KC_TRANSPARENT` is 1, so anything above the latter
  is real — on these deliberately sparse layers that is a small minority of the
  board, and exactly what is worth pointing at.
- **`_GAME`** borrows the underglow for its tint. A full-board wash would read
  more clearly but would also mean losing the animation for as long as the game
  lasts. Green in every theme, for the same reason Caps Lock is always red: it
  is a mode you toggle and stay in, and a state that can be left switched on
  should announce itself in one colour rather than in whatever the palette
  fancies. It used to be red in two themes and green in the other three, which
  meant red stood for both "stuck on" and "game".
- **The encoder knobs** are held at the theme's warm `knob_hue` — drawn *after*
  the layer indicator, even though `[4,5]` carries `RM_TOGG` on `_ADJUST` and
  would otherwise be lit by it. An accent hue through an amber knob is a dim
  brown, and a knob that is the same warm colour on every layer is worth more
  than one more lit key on a layer that already lights ten.
- **Caps Lock** is red on the key that toggles it, in every theme. A stuck-on
  state should not be styled.
- **Caps Word** lights a shift — one shift, see below.

### Caps Word only lights one shift

Expect exactly one: the one on the half holding the USB cable. Caps Word state
lives in the master only — there is no `SPLIT_CAPS_WORD_ENABLE` and the slave
never runs the state machine — and a half only ever flushes its own LEDs, so the
master's write to the other side's shift goes nowhere. Both are set so that
whichever half is master, its own shift lights.

Do not "fix" that with a custom split transaction. It is the same trade the old
`timeout_fade.c` made, and it is not worth it for an indicator that is already
visible on the half you are looking at.

## Displays

The panels are 128x32 — nothing declares a size, so QMK falls through to that
default — and they are mounted **portrait**, with the long axis running away
from the user. Nothing in software says so: the master gets `OLED_ROTATION_0`.
In practice that means framebuffer `+x` is *up* the screen and `+y` runs
*across* it. Upstream's own `layer0_img` only decodes to legible glyphs under
that same rotation, which is how it was pinned down.

The starfield is indifferent — the ship flies up the long axis and looks
deliberate. Text is not: drawn the ordinary way it comes out on its side, which
is exactly how the `GAME` banner first shipped. It is now authored as a 5x7 font
in *screen* coordinates and transposed on blit, mapping `screen (sx, sy)` to
`buffer (x = 127 - sy, y = sx)` so that one glyph column becomes one bit spread
across eight bytes. Confirmed right way up on hardware. **Any future text on
these panels has to do the same.**

### The secondary panel is a shift gate

The panel is 32 px across and 128 px down — a column — and there are five layers
to put in it, so they are stacked like the gate of an automatic shifter, with a
lit block that slides between the positions:

```text
        GAME           the two default layers, above the gate: where the
      ▌ BASE ▐         board is parked and what it returns to on release
      ────  ────       the gate, with the notch the lever passes through
       [LOWER]         the three momentary layers, below it: where the
        RAISE          board only is while a thumb holds it there
         ADJ
```

That split is the point, not decoration. The line is the difference between a
layer you toggle and a layer you hold, so the block being above or below it says
which kind you are in before you have read a word. The two detent marks flank
whichever default layer is engaged, which is what keeps `_GAME` legible while a
momentary layer sits on top of it — they fit because only the four-glyph names,
`GAME` and `BASE`, can ever *be* a default layer; the five-glyph ones use 29 of
the 32 columns and leave no margin.

The slide eases out, so it leaves fast and settles slow, the way a lever falling
into a detent does. It runs off `timer_elapsed` rather than a frame count
because the frame rate is not ours to choose — `OLED_UPDATE_INTERVAL` defaults
to 50 ms on split boards, and anything that changed it would otherwise change
the animation with it. Nothing is redrawn once the block stops.

It replaces the board's `layerN_img` bitmaps, which are **not digits**: each is
a dial with `1` and `2` printed at the top, `4` and `3` at the bottom, and a
pointer that swings to one — so `layerN_img` points at **N+1**, and mapping
`_LOWER` (enum 2) to `layer2_img` displayed a 3. No assignment fixes that: the
dial has four positions and this keymap has five layers, which is also why
`_ADJUST` had nowhere to go at all.

Dropping the four bitmaps reclaimed 2 KB of flash, which is most of what the
font, the text renderer and the reactive effects cost.

It reads `layer_state | default_layer_state`, not just `layer_state`. `_GAME` is
only ever a *default* layer, so it never sets a bit in `layer_state` — reading
that alone left the `_GAME` label unreachable and the panel showing the base one
for an entire game.

The byte-and-bit reversal is still there and still load-bearing:
`oled_write_raw()` bypasses the driver's rotation handling, so a full 512-byte
frame has to be reversed by hand to cancel the board's `OLED_ROTATION_180`.

## Idle behaviour

| idle | |
| --- | --- |
| 0–30 s | full brightness |
| 30–60 s | linear ramp to black (`RGB_FADE_MS`, in `keymap.c`) |
| 60 s | `RGB_MATRIX_TIMEOUT` expires, `oled_task_user` calls `oled_off()` |
| host suspends | `RGB_MATRIX_SLEEP` drops the lighting at once, no wait |

The ramp is half the timeout rather than the 5 seconds it used to be, because
every theme now rests **lit** — see above. When the deck went dark between
keystrokes on its own, a late ramp cost nothing; with the deck lit it would have
meant a keyboard glowing at full for a whole minute after the last thing typed
on it. One threshold, `IDLE_TIMEOUT_MS`, still drives both the lighting and the
panel.

Both halves dim and sleep together **without any custom split transaction**:

- `SPLIT_ACTIVITY_ENABLE` keeps `last_input_activity_elapsed()` in step across
  the link.
- `RGB_MATRIX_TIMEOUT` / `OLED_TIMEOUT` are both driven off that same clock, so
  they fire simultaneously on each side.
- The RGB ramp-down (`rgb_fade_task` in `keymap.c`) is a pure function of the
  idle clock. The master writes `rgb_matrix_config` and the core's
  `RGB_MATRIX_SPLIT` sync mirrors it to the secondary half for free.
- The OLED fade is `OLED_FADE_OUT`, performed by the SSD1306 itself — no
  per-step brightness writes cross the link.

The previous version of this keymap hand-rolled all of the above as a ~450 line
`timeout_fade.c` with its own `transaction_rpc_send` running every 100 ms, a
retry counter and a watchdog. All of it is now core QMK config.

## USB identity

`SERIAL_NUMBER` is pinned to a fixed string. Without it QMK derives the USB
serial from the MCU's unique flash ID, so the two halves present as different
devices and macOS re-asks "Allow accessory to connect" whenever the cable moves
to the other half. See `config.h` for the full reasoning.

## Working on this

Generate a compilation database for clangd:

```sh
cd /path/to/qmk_firmware
qmk generate-compilation-database -kb boardsource/lulu/rp2040 -km twdickson
```

Pushing runs `qmk lint` and `clang-format --dry-run --Werror` over `keyboards/`
before the build. The lint step reads its targets out of `qmk.json`, so a new
board is covered without editing the workflow.

**Verify a new define actually took effect** rather than assuming it did. Delete
it, rebuild, and compare `arm-none-eabi-size` on
`.build/boardsource_lulu_rp2040_twdickson.elf`; identical size means inert. The
`.uf2` is block-padded, so compare the ELF, not the UF2. The same trick works on
a weak hook that is silently not overriding anything — rename it and see whether
the binary changes.

## Notes to self

- `CONSOLE_ENABLE` adds a HID interface and turns every `uprintf` into a USB
  transfer. Keep it `no` unless actively debugging.
- `MOUSEKEY_ENABLE = no` overrides the board's `info.json`, which turns mousekey
  on for everyone. No mouse keycode is mapped anywhere here, and it measured at
  1452 bytes of flash for nothing (47176 -> 45724 text). Turn it back on the day
  mouse keys land on a layer.
- VIA is off on purpose. With it on, the live keymap is the EEPROM copy rather
  than this source tree, and they only resync when VIA's `QMK_BUILDDATE` magic
  changes — so source edits appeared to do nothing until a rebuild on a later
  calendar day, which then silently wiped any VIA edits. `keymap.c` is now the
  only keymap. Note `VIA_ENABLE` also implies `TRI_LAYER_ENABLE`, but
  `update_tri_layer_state()` lives in `action_layer.c` and is always built, so
  the tri-layer is unaffected.
- Keymap directory names must match `^[0-9a-z_]*$` — the userspace schema
  rejects capitals, which is why this is `twdickson` and not `TWDickson`.
- `rules.mk` is a Makefile: `//` is not a comment. `FOO = no // why` sets `FOO`
  to the whole string.
- Lighting config uses the `RGB_MATRIX_*` names. `RGB_MATRIX_STARTUP_*` was
  renamed to `RGB_MATRIX_DEFAULT_*` in 2022, and `RGBLIGHT_*_STEP` only ever
  applied to rgblight, which this board does not build. Both spellings compile
  fine and do nothing.
