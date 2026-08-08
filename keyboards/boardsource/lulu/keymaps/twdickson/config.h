// Copyright 2026 Taylor Dickson
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

/* ── Split link ────────────────────────────────────────────────────────────
 * Only what each half actually renders. SPLIT_ACTIVITY_ENABLE is the load
 * bearing one: it keeps last_input_activity_elapsed() in step across the
 * link, which is what makes the RGB and OLED timeouts fire together on both
 * halves without any custom transaction.
 *
 * SPLIT_TRANSPORT_MIRROR used to be absent on the grounds that nothing on the
 * secondary half needed the master's key events. Reactive lighting does: only
 * the master runs process_record, so without the mirror the slave's
 * last_hit_buffer stays empty and that whole half of the board sits inert while
 * the other one flares. It costs a matrix's worth of link traffic per scan.
 */
#define SPLIT_LAYER_STATE_ENABLE // secondary half draws the layer name
#define SPLIT_LED_STATE_ENABLE   // Caps Lock indicator
#define SPLIT_ACTIVITY_ENABLE    // shared idle clock for the timeouts
#define SPLIT_WATCHDOG_ENABLE    // reboot a half that comes up without a link
#define SPLIT_TRANSPORT_MIRROR   // so reactive effects fire on both halves

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

/* ── Reactive effects ─────────────────────────────────────────────────────
 * The board's info.json enables six animations, all of which run regardless of
 * what anyone is doing. These two respond to typing instead, which is the only
 * kind of movement on a keyboard that means anything.
 *
 * RGB_MATRIX_KEYPRESSES is what defines RGB_MATRIX_KEYREACTIVE_ENABLED, and
 * every reactive effect is compiled out without it. It also costs a per-key
 * hit buffer in RAM, which is why it is not on by default.
 *
 * Both splashes ADD to the resting deck rather than replacing it —
 *
 *     hsv.v = qadd8(hsv.v, 255 - effect)      solid_splash_anim.h
 *
 * — so the board stays lit at the theme's own colour and a keystroke sends a
 * ring out from under the cap that was struck, peaking at 255 no matter how low
 * the resting brightness is set. That is what gives a lit deck and a ripple at
 * the same time, and it is why the flare keeps its punch when the value is
 * dialled down for a dark room.
 *
 * The difference between them: SOLID_SPLASH tracks only the newest hit, so a
 * ripple is cancelled by the next keystroke; SOLID_MULTISPLASH runs one per
 * entry in the hit buffer, so fast typing overlaps them. Multi is the default
 * theme's, single is "Deep"'s.
 *
 * SOLID_REACTIVE_SIMPLE, which used to be here, is the opposite trade: it
 * *scales* the deck by the strike instead of adding to it, leaving the board
 * dark until a key is hit. It looked good against the black case but it meant
 * the deck was off whenever the hands were.
 */
#define RGB_MATRIX_KEYPRESSES
#define ENABLE_RGB_MATRIX_SOLID_SPLASH
#define ENABLE_RGB_MATRIX_SOLID_MULTISPLASH

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
