// Copyright 2026 Taylor Dickson
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

/* ── Split link ────────────────────────────────────────────────────────────
 * Only what each half actually renders. SPLIT_ACTIVITY_ENABLE is the load
 * bearing one: it keeps last_input_activity_elapsed() in step across the
 * link, which is what makes RGB_MATRIX_TIMEOUT and OLED_TIMEOUT fire together
 * on both halves without any custom transaction.
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
 * The RGB ramp-down that runs ahead of RGB_MATRIX_TIMEOUT lives in keymap.c.
 * The OLED fade is OLED_FADE_OUT, which is done by the SSD1306 itself.
 */
#define RGB_MATRIX_TIMEOUT 60000
#define RGB_MATRIX_SLEEP // drop the lighting when the host suspends

#define OLED_TIMEOUT 60000
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

/* ── VIA ──────────────────────────────────────────────────────────────── */
#define DYNAMIC_KEYMAP_LAYER_COUNT 5 // QWERTY, GAME, LOWER, RAISE, ADJUST
