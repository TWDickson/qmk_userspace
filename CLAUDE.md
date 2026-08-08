# qmk_userspace

QMK keymap for a Boardsource Lulu (RP2040 split). This is a QMK **External
Userspace**: it holds only the keymap and builds against an unmodified upstream
`qmk_firmware`. Read `README.md` for the layout and design rationale.

## Two repositories

| repo | role |
| --- | --- |
| this one | the keymap — the only thing that gets edited |
| the QMK checkout | branch `modernize-2026`, **tracks upstream unmodified** |

`qmk config user.qmk_home` is the authoritative location of the QMK checkout;
`user.overlay_dir` points back here. The `.code-workspace` file assumes the two
sit as siblings under `~` and `~/Projects`, so adjust it if that changes.

The firmware checkout used to be a fork with the keymap committed inside it,
which is what made it impossible to update. Keep it clean — if a change seems to
need editing files under `qmk_firmware`, it almost certainly belongs in this
repo instead. The older branches (`master`, `Lulu`, `LuluWIP`) are archives of
that fork; don't build from them.

## Build

```sh
qmk userspace-compile                          # everything in qmk.json
make boardsource/lulu/rp2040:twdickson         # one target
```

Flash with `QK_BOOT` on `_ADJUST` (far top-right), which mounts `RPI-RP2`, then
copy the `.uf2` over. Both halves get the same file, and only the plugged-in
half reboots, so it takes two passes.

The reset button is **not** an alternative: `RP2040_BOOTLOADER_DOUBLE_TAP_RESET`
is not set for `boardsource/lulu`, so pressing it just restarts the firmware.
The fallback with no working keymap is bootmagic — hold the top outer key while
plugging the cable in.

## Load-bearing things — do not "clean these up"

- **`KC_GRV` on `_LOWER`.** `QK_GESC` only emits grave while Shift or GUI is
  held *and leaves that modifier applied*, so it yields `~`, never a bare
  backtick. This is the only backtick on the board. Taylor is a programmer and
  uses it constantly.
- **`GRAVE_ESC_ALT_OVERRIDE` / `GRAVE_ESC_CTRL_OVERRIDE`.** Without them,
  Cmd+Opt+Esc (Force Quit) and Ctrl+Shift+Esc (Task Manager) arrive as grave.
  The Shift and GUI overrides must stay off — they would cost `~` and Cmd+`.
- **No `CHORDAL_HOLD`.** It settles same-hand chords as taps. Correct for home
  row mods, wrong for the pinky shifts here: `Shift+Q`/`A`/`Z` would emit `[q`.
- **The OLED rotation and the manual byte/bit flip in `oled_animation.c` are
  both required.** The board's `oled_init_kb` drives the secondary panel at
  180°, and the flip is its inverse. The panel geometry is never declared
  anywhere, so QMK falls through to its default of **128x32**
  (`drivers/oled/oled_driver.h`, the `#else // defined(OLED_DISPLAY_128X64)`
  branch). That makes `sizeof(layer0_img)` == 512 == exactly one full
  framebuffer, so reversing it byte-wise and bit-wise *is* an exact 180° — the
  two do cancel. Change one and the display flips.

- **The panels are mounted portrait, and nothing in software says so.** Both
  get `OLED_ROTATION_0` from the master's side, but physically the long 128px
  axis runs away from the user: framebuffer +x is *up* the screen and +y runs
  *across* it. The evidence is upstream's own `layer0_img`, which only decodes
  to legible glyphs when the 128x32 buffer is rotated 90°.

  The starfield does not care — the ship just flies up the long axis, which is
  why this went unnoticed. **Text does.** Anything glyph-shaped has to be
  authored in screen coordinates and transposed on blit; see
  `overlay_game_banner()`, which is confirmed right way up on hardware. Drawing
  a font the ordinary way lands it on its side.

- **`layerN_img` points at N+1, so none of them can be used directly.** The
  board's four layer bitmaps are not digits — each is a dial with 1 and 2 at the
  top, 4 and 3 at the bottom, and a pointer swinging to one. Mapping `_LOWER`
  (enum 2) to `layer2_img` put a 3 on the panel. There is no assignment that
  works: four dial positions, five layers. The panel draws a shift gate instead,
  and the bitmaps are no longer referenced at all — which is where 2 KB of the
  flash for the font and the reactive effects came from.

- **The gate's two-above/three-below split is semantic.** `GAME` and `BASE` sit
  above the line because they are *default* layers you toggle and stay in;
  `LOWER`, `RAISE` and `ADJ` below it are momentary. Reordering it into enum
  order, or into one flat list, throws that away. The detent marks flanking the
  engaged default layer only fit because `GAME` and `BASE` are four glyphs —
  a five-glyph name uses 29 of the 32 columns.

- **`SPLIT_TRANSPORT_MIRROR` is now required, and the old comment saying
  otherwise was right at the time.** Reactive lighting needs it: only the master
  runs `process_record`, so without the mirror the slave's `last_hit_buffer`
  never fills and that half of the board stays dark while the other flares.
  Removing it does not break the build, it just half-breaks the lighting.

- **No stock reactive effect rests lit, and the reactive themes are not reactive
  effects.** This file used to claim the opposite — that both splashes add their
  flare to the resting deck and only `SOLID_REACTIVE_SIMPLE` scales it. Untrue:
  `effect_runner_reactive_splash()` sets `hsv.v = 0` per LED before accumulating,
  so `qadd8` adds ripples to *each other*, and the deck is black ~1.3 s after the
  last keystroke. Both families rest dark. The reactive themes are therefore
  `SOLID_COLOR` with `splash_overlay()` in the indicator hook painting the rings
  on top, which is what actually delivers a lit deck *and* a ripple. Do not
  "simplify" it back to `RGB_MATRIX_SOLID_MULTISPLASH`.

- **The case is black and the plate is charcoal. Nothing on this board is amber
  except the two encoder knobs.** An earlier version of this file said the
  underglow "lights the amber case" and derived a warm-underglow-only rule from
  it. The underglow does not wash a surface at all — it escapes as a thin line
  between plate and case and reads as a neon outline against black, so it takes
  any hue cleanly. Three themes depend on that.

- **The underglow is still deliberately excluded from the animation.**
  `rgb_theme.c` overwrites every `LED_FLAG_UNDERGLOW` LED every frame with a flat
  colour. That is not a bug to optimise away — it is the board's outline, and an
  outline reads better as one steady line than as a gradient crawling around it.
  `_GAME` borrows the same LEDs.

- **The encoder knobs are the only shine-through parts on the board.** Every
  keycap is opaque, so LEDs `[4,5]` and `[9,5]` are the sole per-key light seen
  as light rather than as plate glow. They are held at a fixed warm colour after
  the layer indicator so they are lit on every layer in every theme. The amber
  body multiplies its input: warm hues blaze, cool hues go dead brown, and a low
  `knob_sat` is *brighter* than a high one because the material supplies the
  colour. Do not give a theme a cool `knob_hue`.

- **`speed` is never a rate.** `ALPHAS_MODS` reads it as the alpha/mod hue
  offset, `DECK_MIRROR` as a hue span, `DECK_ZONES` as a hue step; only
  `DECK_PULSE` treats it as a rate. Two themes were previously tuned as though it
  were one and were visibly wrong for it (a green fringe on Rose, a chartreuse
  thumb row on the since-deleted Ember). Check the renderer before touching it.

- **Five of the board's six stock animations are `#undef`'d in the keymap's
  `config.h`, and there is a `_Static_assert` guarding it.** That works without a
  `post_config.h` (and so without a `users/` directory) because the generated
  `info_config.h` is included *before* a keymap's `config.h`. An `#undef` of
  someone else's `#define` fails quiet, so `RGB_MATRIX_EFFECT_MAX == 3` is the
  tripwire — NONE, SOLID_COLOR, ALPHAS_MODS. If it fires, check the
  `ENABLE_RGB_MATRIX_*` spellings against the current QMK before touching the
  number.

- **`ALPHAS_MODS` is kept deliberately and is the only stock effect left.**
  `LED_FLAG_MODIFIER` on this board is exactly the outer pinky column plus the
  four thumbs — the eight keys a half carrying the red Escape, the blue LOWER and
  the peach RAISE caps. Its geometry is a fact about the hardware. Everything
  else was a function of x, y or time; the deck renderers in `rgb_theme.c`
  replaced them.

- **A theme seeds the lighting; it does not own it.** `theme_apply()` uses the
  EEPROM setters and `rgb_theme_init()` deliberately does *not* call it, so
  `RM_HUEU`/`RM_SATD`/`RM_SPDU` adjust from the theme's starting point and
  survive a reboot. Re-applying the table at boot is what made those knobs
  appear to work and then silently discard everything. The deck renderers read
  the *live* hue/sat/speed for the same reason — reading the table repaints over
  every adjustment.

- **Brightness must not be read live in `theme_apply()`.**
  `last_matrix_activity_trigger()` runs *after* `matrix_task()`, so when
  `process_record` dispatches `THEME_NEXT` the idle clock still reads what it did
  before the keypress. A theme change mid-fade would persist a half-dimmed board
  as a preference. Use `rgb_theme_user_val()`.

- **Theme names are capped at five glyphs.** The `_ADJUST` config panel is 32 px
  across and the font advances 6, so a sixth glyph clips. Same ceiling as the
  shift gate's layer names, and the reason `DECK_MIRROR`'s theme is called Fold.
  The font is **A-Z only** — `draw_text()` indexes it with `c - 'A'`, so a digit
  or symbol reads off the end of the table rather than printing. That is why the
  panel's four knobs are bars.
- **`_RAISE`'s `ED_*` editing keys are Ctrl-based on purpose.** They are not
  missing a macOS variant. `keymap_common.c` runs `QK_MODS` keycodes through
  `mod_config()`, which is where `CG_TOGG`'s swap happens, so `LCTL(KC_C)`
  arrives as Cmd+C the moment the board is switched over. Writing a `LGUI(...)`
  branch would double-swap. Redo is `Ctrl+Shift+Z` and not `Ctrl+Y` for the same
  reason — Cmd+Y is not redo anywhere.

  They cannot be called `KC_UNDO`/`KC_CUT`/`KC_COPY`/`KC_PSTE`/`KC_FIND` either;
  those are real HID usages QMK already defines, and are ignored by most
  software, which is why these are modifier combos in the first place.

- **Only the theme *index* is persisted; everything else is `_noeeprom`.**
  `rgb_theme.c` re-derives mode/hue/sat from that one byte at every boot, which
  is what keeps a per-boot theme apply off the EEPROM's write budget. It also
  deliberately leaves brightness alone — that is a room-lighting preference, not
  part of a palette, so `RM_VAL*` still owns it.

- **Caps Word lights one shift, not two, and that is correct.** The state lives
  in the master only (no `SPLIT_CAPS_WORD_ENABLE` exists, and the slave never
  runs the state machine), and a half only flushes its own LEDs. Both shifts are
  set so whichever half holds the cable lights its own. Adding a split
  transaction for this is the `timeout_fade.c` mistake again.

- **`_GAME` is overrides-only and relies on layer-0 fallback.**
  `layer_switch_get_layer()` falls back to layer 0 unconditionally when every
  active layer is transparent, and `_QWERTY` is layer 0 — that is why 55
  transparent keys resolve correctly even though `_GAME` is the active
  *default* layer.

## Config that silently does nothing

This keymap previously carried several defines that had no effect. Check the
spelling against the QMK source before trusting any lighting or tap-hold knob:

- `RGB_MATRIX_STARTUP_*` was renamed `RGB_MATRIX_DEFAULT_*` in 2022.
- `RGBLIGHT_*_STEP` only applies to rgblight; this board builds rgb_matrix, so
  the `RGB_MATRIX_*_STEP` names are the real ones.
- `RGB_*` keycodes now act on underglow. Use `RM_*` for the matrix.
- `rules.mk` is a Makefile — `//` is not a comment. `FOO = no // why` sets
  `FOO` to the whole string including the text.
- QMK hue is 0-255 across the whole wheel, not 0-360. `hsv_to_rgb` picks a
  sextant with `h * 6 / 255`, so **240 is rose, not blue** — blue is ~170. The
  `RGB_MATRIX_DEFAULT_HUE 240 // blue` comment was wrong about what the board
  actually lights up as.

**Verify a new define actually took effect** rather than assuming: delete it,
rebuild, and compare `arm-none-eabi-size` on
`.build/boardsource_lulu_rp2040_twdickson.elf`. Identical size means inert.
(The `.uf2` is block-padded, so compare the ELF, not the UF2.)

## Idle behaviour is core QMK, not custom code

`SPLIT_ACTIVITY_ENABLE` keeps `last_input_activity_elapsed()` in step across the
link; `RGB_MATRIX_TIMEOUT` and `oled_task_user` both read that one clock;
`OLED_FADE_OUT` is done by the SSD1306 itself; and the core's `RGB_MATRIX_SPLIT`
sync mirrors `rgb_matrix_config` to the secondary half.

This replaced a ~450-line `timeout_fade.c` with its own `transaction_rpc_send`
every 100 ms plus retry and watchdog logic. **Do not reintroduce a custom split
transaction for display state** — if the halves disagree about something, the
answer is almost certainly an existing `SPLIT_*_ENABLE`.

## Conventions

- Keymap directory names must match `^[0-9a-z_]*$`. The userspace schema
  rejects capitals, which is why it is `twdickson`.
- `CONSOLE_ENABLE` stays `no`. It adds a HID interface and makes every
  `uprintf` a USB transfer; it was previously left on in a daily-driver build
  with unconditional `printf` on every key press *and* release.
- VIA stays off. With it on the live keymap is the EEPROM copy rather than this
  source, and they only resync when VIA's `QMK_BUILDDATE` magic changes.
- Commits here cannot be signed non-interactively — 1Password's git signer
  needs a desktop unlock. Use `-c commit.gpgsign=false` and say so.
