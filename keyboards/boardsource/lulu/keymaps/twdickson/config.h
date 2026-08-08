// Copyright 2026 Taylor Dickson
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

/* ── Split link ────────────────────────────────────────────────────────────
 * Only what each half actually renders. SPLIT_ACTIVITY_ENABLE is the load
 * bearing one: it keeps last_input_activity_elapsed() in step across the
 * link, which is what makes the RGB and OLED timeouts fire together on both
 * halves without any custom transaction.
 *
 * SPLIT_TRANSPORT_MIRROR is deliberately absent — nothing on the secondary
 * half needs to see the master's key events, and mirroring the matrix costs
 * scan rate for no benefit here.
 */
#define SPLIT_LAYER_STATE_ENABLE // secondary half draws the layer glyph
#define SPLIT_LED_STATE_ENABLE   // Caps Lock indicator
#define SPLIT_ACTIVITY_ENABLE    // shared idle clock for the timeouts
#define SPLIT_WATCHDOG_ENABLE    // reboot a half that comes up without a link

/* ── Idle behaviour ───────────────────────────────────────────────────────
 * One idle threshold drives both the lighting and the panel. The RGB
 * ramp-down that runs ahead of the cutoff lives in keymap.c; the OLED fade is
 * OLED_FADE_OUT, performed by the SSD1306 itself.
 *
 * The driver's own OLED_TIMEOUT is deliberately off. It restarts on every
 * oled_on() call, so waking the panel from the idle clock would also push its
 * next sleep out by a further OLED_TIMEOUT — the display would outlast the
 * lighting by a full minute. oled_task_user() turns the panel off explicitly
 * instead, from the same clock RGB_MATRIX_TIMEOUT uses.
 */
#define IDLE_TIMEOUT_MS 60000

#define RGB_MATRIX_TIMEOUT IDLE_TIMEOUT_MS
#define RGB_MATRIX_SLEEP // drop the lighting when the host suspends

#define OLED_TIMEOUT 0
#define OLED_FADE_OUT
#define OLED_FADE_OUT_INTERVAL 8 // 0-15, larger is slower

/* ── RGB matrix ───────────────────────────────────────────────────────────
 * These are the RGB_MATRIX_* names. The RGB_MATRIX_STARTUP_* spellings were
 * renamed in 2022 and the RGBLIGHT_*_STEP ones only ever applied to rgblight,
 * which this board does not build — both were silently doing nothing.
 */
#define RGB_MATRIX_DEFAULT_MODE RGB_MATRIX_ALPHAS_MODS
#define RGB_MATRIX_DEFAULT_HUE 240 // blue
#define RGB_MATRIX_DEFAULT_SAT 239
#define RGB_MATRIX_DEFAULT_VAL 150 // also the board's max_brightness

#define RGB_MATRIX_HUE_STEP 8
#define RGB_MATRIX_SAT_STEP 8
#define RGB_MATRIX_VAL_STEP 8

/* ── USB identity ─────────────────────────────────────────────────────────
 * On ChibiOS, SERIAL_NUMBER_USE_HARDWARE_ID defaults to TRUE, so the USB
 * serial is derived from the MCU's unique flash ID. The two halves have
 * different IDs, which means moving the cable to the other half presents a
 * different device — and macOS's "Allow accessories to connect" tracks
 * approvals per device, so it asks again.
 *
 * A fixed string makes the identity stable no matter which half is plugged in
 * or whether a controller ever gets replaced. Only the master enumerates over
 * USB, so both halves sharing a serial is not a conflict. Expect one final
 * approval prompt after first flashing this.
 */
#define SERIAL_NUMBER "TWDickson-Lulu"

/* ── Grave / Escape ───────────────────────────────────────────────────────
 * QK_GESC sends grave whenever Shift or GUI is held, and it leaves that
 * modifier applied. That is what gives "~" on Shift, but it also swallows the
 * two shortcuts that reach for Escape with a modifier already down:
 *
 *   macOS   Cmd+Opt+Esc  (Force Quit)   was arriving as Cmd+Opt+`
 *   Windows Ctrl+Shift+Esc (Task Mgr)   was arriving as Ctrl+Shift+`
 *
 * These two overrides force Escape whenever Alt or Ctrl is held, which is
 * enough to fix both without touching anything else.
 *
 * Deliberately NOT set:
 *   GRAVE_ESC_SHIFT_OVERRIDE would take away "~" on Shift+Esc.
 *   GRAVE_ESC_GUI_OVERRIDE   would take away Cmd+` window cycling on macOS.
 *
 * A bare backtick is on _LOWER, since no combination of QK_GESC produces one.
 */
#define GRAVE_ESC_ALT_OVERRIDE
#define GRAVE_ESC_CTRL_OVERRIDE

/* ── Tap-hold ─────────────────────────────────────────────────────────────
 * The pinky shifts are mod-taps. On stock settings a fast roll into one can
 * settle as a tap and emit a bracket where a capital was meant.
 *
 * PERMISSIVE_HOLD settles a mod-tap as held as soon as another key is pressed
 * *and released* inside it — which is the exact shape of a capital: shift
 * down, letter down, letter up, shift up. Rolling out of the shift before the
 * next key still taps, so typing a bracket is unaffected.
 *
 * QUICK_TAP_TERM 0 stops a tap-then-hold from auto-repeating the tap keycode,
 * so typing "[" and immediately reaching for shift gives shift, not "[[[[".
 *
 * SPECULATIVE_HOLD applies the modifier on keydown instead of waiting for the
 * decision, retracting it if the key turns out to be a tap. It defaults to
 * Shift and Ctrl mod-taps, which is exactly what this keymap has, and it is
 * what stops shift-click feeling laggy.
 *
 * CHORDAL_HOLD is deliberately NOT set. It settles same-hand chords as taps,
 * which is right for home row mods and wrong here — shifting a same-hand
 * letter (Shift+Q, Shift+A, Shift+Z) is entirely normal on a pinky shift, and
 * would come out as "[q" instead of "Q".
 */
#define TAPPING_TERM 200 // the stock default; stated because it is the first knob to turn
#define PERMISSIVE_HOLD
#define QUICK_TAP_TERM 0
#define SPECULATIVE_HOLD
