// Copyright 2026 Taylor Dickson
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include QMK_KEYBOARD_H

// Loads the stored theme index. Call from keyboard_post_init_user().
void rgb_theme_init(void);

// Handles THEME_NEXT / THEME_PREV. Returns false if it consumed the record.
bool rgb_theme_process_record(uint16_t keycode, keyrecord_t *record);

/* Brightness with the idle ramp taken back out — defined in keymap.c next to
 * the ramp itself. theme_apply() persists, and the live value is not safe to
 * persist mid-fade; see the comment on the definition.
 */
uint8_t rgb_theme_user_val(void);

// The current theme's name, uppercase, five glyphs at most — for the _ADJUST
// config panel, which is 32 px across.
const char *rgb_theme_name(void);
