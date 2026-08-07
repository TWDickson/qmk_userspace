// Copyright 2026 Taylor Dickson
// SPDX-License-Identifier: GPL-2.0-or-later

#include "keymap.h"
#include "oled_animation.h"

#define LOWER MO(_LOWER)
#define RAISE MO(_RAISE)
#define KC_CAD LALT(LCTL(KC_DEL)) // Ctrl+Alt+Del, for the work laptop

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

/* QWERTY
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * | GESC |   1  |   2  |   3  |   4  |   5  |                    |   6  |   7  |   8  |   9  |   0  |  -   |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | Tab  |   Q  |   W  |   E  |   R  |   T  |                    |   Y  |   U  |   I  |   O  |   P  |  =   |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * |LCTRL |   A  |   S  |   D  |   F  |   G  |-------.    ,-------|   H  |   J  |   K  |   L  |   ;  |  '   |
 * |------+------+------+------+------+------|   [   |    | Play  |------+------+------+------+------+------|
 * |Sft/[ |   Z  |   X  |   C  |   V  |   B  |-------|    |-------|   N  |   M  |   ,  |   .  |   /  |Sft/] |
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *                   | LGUI | LALT |LOWER | /Space  /       \Enter \  |RAISE |BackSP| RGUI |
 *                   `----------------------------'           '------''--------------------'
 */
[_QWERTY] = LAYOUT(
  QK_GESC,          KC_1,    KC_2,    KC_3,    KC_4,    KC_5,                      KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS,
  KC_TAB,           KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,                      KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_EQL,
  KC_LCTL,          KC_A,    KC_S,    KC_D,    KC_F,    KC_G,                      KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,
  LSFT_T(KC_LBRC),  KC_Z,    KC_X,    KC_C,    KC_V,    KC_B, KC_LBRC,   KC_MPLY,  KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, RSFT_T(KC_RBRC),
                                   KC_LGUI, KC_LALT,   LOWER,  KC_SPC,   KC_ENT,   RAISE,   KC_BSPC, KC_RGUI
),

/* GAME - plain shifts and no GUI, so held keys and Windows key misfires stay out of the way
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * | GESC |   1  |   2  |   3  |   4  |   5  |                    |   6  |   7  |   8  |   9  |   0  |  -   |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | Tab  |   Q  |   W  |   E  |   R  |   T  |                    |   Y  |   U  |   I  |   O  |   P  |  =   |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * |LCTRL |   A  |   S  |   D  |   F  |   G  |-------.    ,-------|   H  |   J  |   K  |   L  |   ;  |  '   |
 * |------+------+------+------+------+------|   [   |    | Play  |------+------+------+------+------+------|
 * |LShift|   Z  |   X  |   C  |   V  |   B  |-------|    |-------|   N  |   M  |   ,  |   .  |   /  |RShift|
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *                   |  NO  | LALT |LOWER | /Space  /       \Enter \  |RAISE |BackSP| RGUI |
 *                   `----------------------------'           '------''--------------------'
 */
[_GAME] = LAYOUT(
  QK_GESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,                      KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS,
  KC_TAB,   KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,                      KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_EQL,
  KC_LCTL,  KC_A,    KC_S,    KC_D,    KC_F,    KC_G,                      KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,
  KC_LSFT,  KC_Z,    KC_X,    KC_C,    KC_V,    KC_B, KC_LBRC,   KC_MPLY,  KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,
                            KC_NO, KC_LALT,   LOWER,  KC_SPC,   KC_ENT,   RAISE,   KC_BSPC, KC_RGUI
),

/* LOWER
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * |  `   |  F1  |  F2  |  F3  |  F4  |  F5  |                    |  F6  |  F7  |  F8  |  F9  | F10  | F11  |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * |      |      |      |      |      |      |                    |      |      |  Up  |      |      | F12  |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * |      |      |      |      |      |      |-------.    ,-------|      | Left | Down | Right|      |      |
 * |------+------+------+------+------+------|       |    |       |------+------+------+------+------+------|
 * |      |      |      |      |      |      |-------|    |-------|      |      |      |      |  \   |      |
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *                   |      |      |      | /      /       \      \  |      | Del  |      |
 *                   `----------------------------'           '------''--------------------'
 */
[_LOWER] = LAYOUT(
  KC_GRV,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,                       KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,
  _______, _______, _______, _______, _______, _______,                     _______, _______, KC_UP,   _______, _______, KC_F12,
  _______, _______, _______, _______, _______, _______,                     _______, KC_LEFT, KC_DOWN, KC_RGHT, _______, _______,
  _______, _______, _______, _______, _______, _______, _______,   _______, _______, _______, _______, _______, KC_BSLS, _______,
                             _______, _______, _______, _______,   _______, _______, KC_DEL,  _______
),

/* RAISE
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * |      |      |      |      |      |      |                    |      |      |      |      |      |      |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * |      |      |      |      |      |      |                    |      |      | PgUp |      |      |      |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * |      |      |      |      |      |      |-------.    ,-------|      | Home | PgDn | End  |      |      |
 * |------+------+------+------+------+------|       |    |       |------+------+------+------+------+------|
 * |      |      |      |      |      |      |-------|    |-------|      |      |      |      |      |      |
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *                   |      |      |      | /      /       \      \  |      |      |      |
 *                   `----------------------------'           '------''--------------------'
 */
[_RAISE] = LAYOUT(
  _______, _______, _______, _______, _______, _______,                     _______, _______, _______, _______, _______, _______,
  _______, _______, _______, _______, _______, _______,                     _______, _______, KC_PGUP, _______, _______, _______,
  _______, _______, _______, _______, _______, _______,                     _______, KC_HOME, KC_PGDN, KC_END,  _______, _______,
  _______, _______, _______, _______, _______, _______, _______,   _______, _______, _______, _______, _______, _______, _______,
                             _______, _______, _______, _______,   _______, _______, _______, _______
),

/* ADJUST - reached by holding LOWER and RAISE together
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * |      |      |      |      |      |      |                    |      |      |      |      |      |      |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * |      |      |      |      |      |      |                    |      |      |      |      |      |      |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | Caps |RM_NXT|RM_HUE|RM_SAT|RM_VAL|GAME  |-------.    ,-------|      |      |      |      |      |      |
 * |------+------+------+------+------+------|RM_TOGG|    |       |------+------+------+------+------+------|
 * |      |RM_PRV|RM_HUD|RM_SAD|RM_VAD|      |-------|    |-------|      |CG_TOG|      |      |      |      |
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *                   |      |      |      | /      /       \      \  |      | CAD  |      |
 *                   `----------------------------'           '------''--------------------'
 */
[_ADJUST] = LAYOUT(
  XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                       XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
  XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                       XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
  KC_CAPS, RM_NEXT, RM_HUEU, RM_SATU, RM_VALU, GAME_TOGGLE,                    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
  XXXXXXX, RM_PREV, RM_HUED, RM_SATD, RM_VALD, XXXXXXX, RM_TOGG,     XXXXXXX, XXXXXXX, CG_TOGG, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
                             _______, _______, _______, _______,     _______, _______, KC_CAD,  _______
),

};

#ifdef ENCODER_MAP_ENABLE
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][2] = {
    [_QWERTY] = {ENCODER_CCW_CW(KC_VOLD, KC_VOLU),  ENCODER_CCW_CW(KC_VOLD, KC_VOLU)},
    [_GAME]   = {ENCODER_CCW_CW(KC_VOLD, KC_VOLU),  ENCODER_CCW_CW(KC_VOLD, KC_VOLU)},
    [_LOWER]  = {ENCODER_CCW_CW(RM_PREV, RM_NEXT),  ENCODER_CCW_CW(RM_VALD, RM_VALU)},
    [_RAISE]  = {ENCODER_CCW_CW(RM_SATD, RM_SATU),  ENCODER_CCW_CW(RM_HUED, RM_HUEU)},
    [_ADJUST] = {ENCODER_CCW_CW(_______, _______),  ENCODER_CCW_CW(_______, _______)},
};
#endif
// clang-format on

layer_state_t layer_state_set_user(layer_state_t state) {
    return update_tri_layer_state(state, _LOWER, _RAISE, _ADJUST);
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case GAME_TOGGLE:
            if (record->event.pressed) {
                const bool in_game = get_highest_layer(default_layer_state) == _GAME;
                set_single_persistent_default_layer(in_game ? _QWERTY : _GAME);
                layer_clear();
            }
            return false;
    }
    return true;
}

#ifdef RGB_MATRIX_ENABLE

// Index of the LED under Caps Lock on the master half.
#    define CAPS_LOCK_LED 23

/* RGB_MATRIX_TIMEOUT cuts the lighting dead the moment it expires. Ramping the
 * value down over the last RGB_FADE_MS turns that into a dim-out instead.
 *
 * The ramp is a pure function of last_input_activity_elapsed(), which
 * SPLIT_ACTIVITY_ENABLE keeps in step across the link, and the resulting
 * config is mirrored to the other half by the core's own RGB_MATRIX_SPLIT
 * sync. So there is no transaction to register and no state to reconcile —
 * the two halves arrive at the same brightness because they are computing the
 * same function of the same clock.
 */
#    define RGB_FADE_MS 5000
#    if RGB_FADE_MS >= RGB_MATRIX_TIMEOUT
#        error "RGB_FADE_MS must be shorter than RGB_MATRIX_TIMEOUT"
#    endif
#    define RGB_FADE_START (RGB_MATRIX_TIMEOUT - RGB_FADE_MS)

static void rgb_fade_task(void) {
    // The slave takes rgb_matrix_config straight from the master, so letting
    // both sides drive it would just be two writers racing for the same value.
    if (!is_keyboard_master()) {
        return;
    }

    static uint8_t user_val = RGB_MATRIX_DEFAULT_VAL;
    static bool    fading   = false;

    const uint32_t idle = last_input_activity_elapsed();

    if (idle < RGB_FADE_START) {
        if (fading) {
            fading = false;
            rgb_matrix_sethsv_noeeprom(rgb_matrix_get_hue(), rgb_matrix_get_sat(), user_val);
        } else {
            // Not fading, so the live value is whatever the user dialled in.
            user_val = rgb_matrix_get_val();
        }
        return;
    }

    if (!fading) {
        fading   = true;
        user_val = rgb_matrix_get_val();
    }

    const uint32_t into = idle - RGB_FADE_START;
    const uint8_t  want = into >= RGB_FADE_MS ? 0 : (uint8_t)((user_val * (RGB_FADE_MS - into)) / RGB_FADE_MS);

    if (want != rgb_matrix_get_val()) {
        rgb_matrix_sethsv_noeeprom(rgb_matrix_get_hue(), rgb_matrix_get_sat(), want);
    }
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    if (host_keyboard_led_state().caps_lock && CAPS_LOCK_LED >= led_min && CAPS_LOCK_LED < led_max) {
        // Scaled by the live value so the indicator dims out with the rest.
        rgb_matrix_set_color(CAPS_LOCK_LED, rgb_matrix_get_val(), 0, 0);
    }
    return false;
}
#endif // RGB_MATRIX_ENABLE

void housekeeping_task_user(void) {
#ifdef RGB_MATRIX_ENABLE
    rgb_fade_task();
#endif
}

#ifdef OLED_ENABLE
bool oled_task_user(void) {
    /* OLED_TIMEOUT only restarts when something calls oled_on(). Tying that to
     * real input rather than to the panel redrawing itself is what lets it
     * sleep at all, and SPLIT_ACTIVITY_ENABLE means both halves agree on when
     * that is. The fade itself is OLED_FADE_OUT, done in the display
     * controller, so no per-step brightness writes go over the link.
     */
    if (last_input_activity_elapsed() < OLED_TIMEOUT) {
        oled_on();
    }
    oled_render_animation();
    return false;
}
#endif
