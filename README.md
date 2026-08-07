# qmk_userspace

QMK keymaps for [Boardsource Lulu](https://boardsource.xyz/products/lulu) (RP2040 split).

This is an [External Userspace](https://docs.qmk.fm/newbs_external_userspace) repo:
it holds only my keymaps, and builds against an unmodified upstream `qmk_firmware`.
Nothing here forks QMK, so pulling a new QMK release is just `git pull` in the
firmware checkout.

## Build

```console
$ qmk config user.qmk_home=/path/to/qmk_firmware
$ qmk config user.overlay_dir=/path/to/qmk_userspace
$ qmk userspace-compile
```

Or a single target: `make boardsource/lulu/rp2040:twdickson`.

Pushing to GitHub builds every target in `qmk.json` via Actions and attaches the
`.uf2` to a release.

## Flashing

Double-tap reset on a half to mount `RPI-RP2`, then copy the `.uf2` over. Flash
both halves with the same file.

## Layout

Five layers: `_QWERTY`, `_GAME`, `_LOWER`, `_RAISE`, `_ADJUST`. `_ADJUST` is a
tri-layer, reached by holding both `LOWER` and `RAISE`.

`GAME_TOGGLE` (on `_ADJUST`) swaps the default layer between `_QWERTY` and
`_GAME` and persists it. While it is active the OLED shows a "GAME" banner.

`_GAME` states only its three overrides — plain shifts instead of mod-taps, and
no left GUI — and leaves everything else transparent.
`layer_switch_get_layer()` falls back to layer 0 unconditionally when every
active layer is transparent, and `_QWERTY` is layer 0, so the rest resolves
against it even though `_GAME` is the active *default* layer. Edits to
`_QWERTY` therefore carry into `_GAME` automatically.

`KC_GRV` on `_LOWER` is **not** redundant with `QK_GESC` beneath it. `QK_GESC`
only emits grave while Shift or GUI is held and leaves that modifier applied,
so Shift gives `~` and, on macOS, GUI+grave cycles windows. It is the only bare
backtick on the board.

## Tap-hold

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

## Idle behaviour

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

## clangd

```console
$ cd /path/to/qmk_firmware && qmk generate-compilation-database -kb boardsource/lulu/rp2040 -km twdickson
```

## Notes to self

- `CONSOLE_ENABLE` adds a HID interface and turns every `uprintf` into a USB
  transfer. Keep it `no` unless actively debugging.
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
