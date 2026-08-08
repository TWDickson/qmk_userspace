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

- **Both splash effects add to the resting deck; `SOLID_REACTIVE_SIMPLE` scales
  it.** `hsv.v = qadd8(hsv.v, 255 - effect)` is why a splash theme is lit at rest
  *and* ripples, and why the flare still peaks at 255 with the brightness dialled
  right down. Swapping one for a reactive effect turns the board dark between
  keystrokes, which is what it used to do and is not wanted.

- **The underglow is deliberately excluded from the animation.** `rgb_theme.c`
  overwrites every `LED_FLAG_UNDERGLOW` LED every frame with a flat colour. That
  is not a bug to optimise away: the underglow lights the amber case rather than
  the grey caps, and a fixed-colour case reads better under a steady wash than
  under a gradient crawling beneath it. `_GAME` borrows the same LEDs.
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
