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
`_GAME` and persists it. `_GAME` drops the home-row-ish mod-taps on the shift
keys and blanks left GUI, so nothing gets held or eaten mid-game. While it is
active the OLED shows a "GAME" banner.

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
- Keymap directory names must match `^[0-9a-z_]*$` — the userspace schema
  rejects capitals, which is why this is `twdickson` and not `TWDickson`.
- `rules.mk` is a Makefile: `//` is not a comment. `FOO = no // why` sets `FOO`
  to the whole string.
- Lighting config uses the `RGB_MATRIX_*` names. `RGB_MATRIX_STARTUP_*` was
  renamed to `RGB_MATRIX_DEFAULT_*` in 2022, and `RGBLIGHT_*_STEP` only ever
  applied to rgblight, which this board does not build. Both spellings compile
  fine and do nothing.
