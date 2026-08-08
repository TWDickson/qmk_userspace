// Copyright 2026 Taylor Dickson
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include QMK_KEYBOARD_H

enum layers {
    _QWERTY,
    _GAME,
    _LOWER,
    _RAISE,
    _ADJUST,
};

enum custom_keycodes {
    GAME_TOGGLE = QK_USER,
    THEME_NEXT,
    THEME_PREV,
};

// Tap-dance indices. TD(TD_MEDIA) is the headphone-button media key.
enum tap_dances {
    TD_MEDIA,
};
