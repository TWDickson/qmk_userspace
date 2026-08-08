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

```console
$ qmk userspace-compile                          # everything in qmk.json
$ make boardsource/lulu/rp2040:twdickson         # one target
```

Flash by double-tapping reset on a half to mount `RPI-RP2` and copying the
`.uf2`. Both halves get the same file.

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
  anywhere, so QMK defaults to 128x64 — whether the two exactly cancel depends
  on that, and it has not been confirmed on hardware. Change one and the
  display flips.
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
