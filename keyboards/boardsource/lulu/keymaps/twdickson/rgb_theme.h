// Copyright 2026 Taylor Dickson
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include QMK_KEYBOARD_H

// Applies the stored theme. Call from keyboard_post_init_user().
void rgb_theme_init(void);

// Handles THEME_NEXT. Returns false if it consumed the record.
bool rgb_theme_process_record(uint16_t keycode, keyrecord_t *record);
