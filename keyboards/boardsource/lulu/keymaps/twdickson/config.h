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

/* The one custom split transaction in this keymap, and the exception that
 * proves the rule against them.
 *
 * SPLIT_TRANSPORT_MIRROR copies the master's *matrix* to the slave; it does not
 * run process_record there. So THEME_NEXT never reaches the slave, its stored
 * theme index never changes, and it renders whatever theme its own EEPROM
 * happens to hold — a different deck, underglow, knobs and accents from the
 * half beside it. On Zones that showed up as one green half and one gradient.
 *
 * No SPLIT_*_ENABLE covers user eeconfig, so there is nothing built-in to reach
 * for. This is one byte, sent when it changes plus a slow heartbeat, and it
 * decides how half the board looks — not the 100 ms poll with retry and
 * watchdog logic that timeout_fade.c was, and not a cosmetic indicator that is
 * already visible on the half you are looking at, which is why Caps Word still
 * does not get one.
 */
#define SPLIT_TRANSACTION_IDS_USER RPC_ID_SYNC_THEME

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
/* These only apply to a never-written or freshly reset EEPROM, since
 * rgb_theme_init() re-derives mode/hue/sat from the stored theme index at
 * every boot. They are set to theme 0's values so the two agree and a factory
 * reset comes up looking like the theme it claims to be on.
 */
#define RGB_MATRIX_DEFAULT_MODE RGB_MATRIX_SOLID_COLOR
#define RGB_MATRIX_DEFAULT_HUE 24 // amber; hue is 0-255, so 240 is rose, not blue
#define RGB_MATRIX_DEFAULT_SAT 255
#define RGB_MATRIX_DEFAULT_VAL 150 // also the board's max_brightness

#define RGB_MATRIX_HUE_STEP 8
#define RGB_MATRIX_SAT_STEP 8
#define RGB_MATRIX_VAL_STEP 8

/* ── The board's stock animations ─────────────────────────────────────────
 * boardsource/lulu's info.json compiles six, and this keymap now uses one.
 *
 * The generated info_config.h is included *before* a keymap's config.h, which
 * is what makes undefining them here work at all and why no post_config.h (and
 * so no users/ directory) is needed. Each one costs its effect function plus an
 * entry in the mode table.
 *
 * ALPHAS_MODS is the survivor, and it earns it: LED_FLAG_MODIFIER on this board
 * is exactly the outer pinky column plus the four thumbs — the eight keys a
 * half that carry the red Escape, the blue LOWER and the peach RAISE caps. Its
 * geometry is a fact about the hardware. The five below are functions of x, y
 * or time and know nothing about a keyboard, which is why what replaced them
 * lives in rgb_theme.c instead.
 *
 * Note these are undefines of someone else's defines: if a future QMK renames
 * one, this goes quiet rather than failing, and the effect comes back. The
 * check is that RGB_MATRIX_EFFECT_MAX stays at three (NONE, SOLID_COLOR,
 * ALPHAS_MODS) — or just that the flash figure does not jump.
 */
#undef ENABLE_RGB_MATRIX_GRADIENT_UP_DOWN
#undef ENABLE_RGB_MATRIX_GRADIENT_LEFT_RIGHT
#undef ENABLE_RGB_MATRIX_BREATHING
#undef ENABLE_RGB_MATRIX_BAND_SAT
#undef ENABLE_RGB_MATRIX_BAND_VAL

/* ── Reactive effects ─────────────────────────────────────────────────────
 * RGB_MATRIX_KEYPRESSES is the whole of it. It defines
 * RGB_MATRIX_KEYREACTIVE_ENABLED, which is what brings g_last_hit_tracker into
 * existence — the ring of recent key positions and ages that splash_overlay()
 * in rgb_theme.c reads to draw its ripples. It costs that buffer in RAM, which
 * is why it is not on by default.
 *
 * No ENABLE_RGB_MATRIX_*_SPLASH here, deliberately. This used to compile
 * SOLID_SPLASH and SOLID_MULTISPLASH and select them as theme modes, on the
 * belief that they add their flare to the resting deck. They do not: their
 * runner zeroes hsv.v per LED before accumulating, so both rest at black about
 * a second after the last keystroke, which is exactly the behaviour every
 * theme here exists to avoid. The reactive themes are SOLID_COLOR with the
 * ripples overlaid in the indicator hook instead — see splash_overlay(), which
 * carries the full account. Dropping the two effects also drops their code.
 */
#define RGB_MATRIX_KEYPRESSES

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
