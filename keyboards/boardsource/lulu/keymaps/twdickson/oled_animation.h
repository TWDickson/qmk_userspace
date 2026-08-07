// Copyright 2026 Taylor Dickson
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include QMK_KEYBOARD_H

// Master half draws the scrolling starfield with the ship position driven by
// WPM; the secondary half draws the current layer glyph.
void oled_render_animation(void);
