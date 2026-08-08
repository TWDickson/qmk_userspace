// Copyright 2026 Taylor Dickson
// SPDX-License-Identifier: GPL-2.0-or-later

#include "keymap.h"
#include "oled_animation.h"
#include "rgb_theme.h"

#define LOWER MO(_LOWER)
#define RAISE MO(_RAISE)
#define KC_CAD LALT(LCTL(KC_DEL)) // Ctrl+Alt+Del, for the work laptop

/* Editing actions for _RAISE's left hand.
 *
 * Ctrl-based on purpose: keymap_common.c runs QK_MODS keycodes through
 * mod_config(), which is where CG_TOGG's swap happens, so every one of these
 * becomes Cmd+<key> the moment the board is switched to macOS. One definition
 * covers both machines.
 *
 * The ED_ prefix is not decoration — KC_UNDO, KC_CUT, KC_COPY, KC_PSTE and
 * KC_FIND are already real HID usages in QMK. They are also ignored by most of
 * the software anyone actually uses, which is why these are modifier combos
 * instead, and why they cannot reuse those names.
 */
#define ED_UNDO LCTL(KC_Z)
#define ED_CUT LCTL(KC_X)
#define ED_COPY LCTL(KC_C)
#define ED_PSTE LCTL(KC_V)
#define ED_REDO LCTL(LSFT(KC_Z)) // not Ctrl+Y: Ctrl+Shift+Z is the spelling that also survives CG_TOGG
#define ED_SALL LCTL(KC_A)
#define ED_SAVE LCTL(KC_S)
#define ED_FIND LCTL(KC_F)
#define ED_FNXT LCTL(KC_G)

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

/* QWERTY
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * | GESC |   1  |   2  |   3  |   4  |   5  |                    |   6  |   7  |   8  |   9  |   0  |  -   |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | Tab  |   Q  |   W  |   E  |   R  |   T  |                    |   Y  |   U  |   I  |   O  |   P  |  =   |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * |LCTRL |   A  |   S  |   D  |   F  |   G  |-------.    ,-------|   H  |   J  |   K  |   L  |   ;  |  '   |
 * |------+------+------+------+------+------| Mute  |    | Play  |------+------+------+------+------+------|
 * |Sft/[ |   Z  |   X  |   C  |   V  |   B  |-------|    |-------|   N  |   M  |   ,  |   .  |   /  |Sft/] |
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *                   | LGUI | LALT |LOWER | /Space  /       \Enter \  |RAISE |BackSP| RGUI |
 *                   `----------------------------'           '------''--------------------'
 *
 * The inner key used to be a second "[", duplicating the left shift's tap.
 * "]" only ever had the one home on the right shift's tap, so the pair now
 * behaves the same way, and the freed key pairs Mute with Media across the two
 * inner positions alongside the volume encoders.
 *
 * The right inner key is the tap-counting media key: one tap play/pause, two
 * next, three previous. See tap_dance_actions below.
 */
[_QWERTY] = LAYOUT(
  QK_GESC,          KC_1,    KC_2,    KC_3,    KC_4,    KC_5,                      KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS,
  KC_TAB,           KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,                      KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_EQL,
  KC_LCTL,          KC_A,    KC_S,    KC_D,    KC_F,    KC_G,                      KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,
  LSFT_T(KC_LBRC),  KC_Z,    KC_X,    KC_C,    KC_V,    KC_B, KC_MUTE, TD(TD_MEDIA), KC_N, KC_M,    KC_COMM, KC_DOT,  KC_SLSH, RSFT_T(KC_RBRC),
                                   KC_LGUI, KC_LALT,   LOWER,  KC_SPC,   KC_ENT,   RAISE,   KC_BSPC, KC_RGUI
),

/* GAME - overrides only; everything else falls through to _QWERTY.
 *
 * layer_switch_get_layer() walks layer_state | default_layer_state from the top
 * down and falls back to layer 0 when every active layer is transparent, so a
 * transparent key here resolves against _QWERTY even though _GAME is the active
 * *default* layer and layer 0's bit is not set. That means this layer only has
 * to state what it changes, and edits to _QWERTY carry over automatically.
 *
 * The three changes: plain shifts instead of mod-taps, so a held shift never
 * decides it was a tap and emits a bracket mid-fight; and no left GUI, so the
 * desktop never gets yanked away.
 *
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * |      |      |      |      |      |      |                    |      |      |      |      |      |      |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * |      |      |      |      |      |      |                    |      |      |      |      |      |      |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * |      |      |      |      |      |      |-------.    ,-------|      |      |      |      |      |      |
 * |------+------+------+------+------+------|       |    |       |------+------+------+------+------+------|
 * |LShift|      |      |      |      |      |-------|    |-------|      |      |      |      |      |RShift|
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *                   |  NO  |      |      | /       /       \      \  |      |      |      |
 *                   `----------------------------'           '------''--------------------'
 */
[_GAME] = LAYOUT(
  _______, _______, _______, _______, _______, _______,                     _______, _______, _______, _______, _______, _______,
  _______, _______, _______, _______, _______, _______,                     _______, _______, _______, _______, _______, _______,
  _______, _______, _______, _______, _______, _______,                     _______, _______, _______, _______, _______, _______,
  KC_LSFT, _______, _______, _______, _______, _______, _______,   _______, _______, _______, _______, _______, _______, KC_RSFT,
                             KC_NO,   _______, _______, _______,   _______, _______, _______, _______
),

/* LOWER
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * |  `   |  F1  |  F2  |  F3  |  F4  |  F5  |                    |  F6  |  F7  |  F8  |  F9  | F10  | F11  |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * |      |      |      |      |      |      |                    |      |      |  Up  |      |      | F12  |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * |CapsWd|      |      |      |      |      |-------.    ,-------|      | Left | Down | Right|      |      |
 * |------+------+------+------+------+------|       |    |       |------+------+------+------+------+------|
 * |      |      |      |      |      |      |-------|    |-------|      |      |      |      |  \   |      |
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *                   |      |      |      | /      /       \      \  |      | Del  |      |
 *                   `----------------------------'           '------''--------------------'
 *
 * KC_GRV here is NOT redundant with QK_GESC below it. QK_GESC emits grave only
 * while Shift or GUI is held, and it leaves that modifier applied — so Shift
 * gives "~", and on macOS GUI+grave cycles windows. This is the only bare
 * backtick on the board. Do not "clean it up".
 *
 * CW_TOGG sits on the same physical key that carries Caps Lock on _ADJUST, one
 * layer down, because it is the one you actually reach for mid-word.
 */
[_LOWER] = LAYOUT(
  KC_GRV,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,                       KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,
  _______, _______, _______, _______, _______, _______,                     _______, _______, KC_UP,   _______, _______, KC_F12,
  CW_TOGG, _______, _______, _______, _______, _______,                     _______, KC_LEFT, KC_DOWN, KC_RGHT, _______, _______,
  _______, _______, _______, _______, _______, _______, _______,   _______, _______, _______, _______, _______, KC_BSLS, _______,
                             _______, _______, _______, _______,   _______, _______, KC_DEL,  _______
),

/* RAISE - navigation, and the things you do to what you just navigated
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * | F13  | F14  | F15  | F16  | F17  | F18  |                    | F19  | F20  | F21  | F22  | F23  | F24  |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * |      |      |      |      |      |      |                    |      |      | PgUp |      |PrtScn|      |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * |      |SelAll| Save |      | Find | FndNx|-------.    ,-------|      | Home | PgDn | End  |      |      |
 * |------+------+------+------+------+------| Prev  |    | Next  |------+------+------+------+------+------|
 * |      | Undo | Cut  | Copy | Paste| Redo |-------|    |-------|      |      |      |      |      |      |
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *                   |      |      |      | /      /       \      \  |      | Ins  |      |
 *                   `----------------------------'           '------''--------------------'
 *
 * The right hand was already the navigation cluster; the left hand is what you
 * do to what it lands on. Undo/Cut/Copy/Paste sit on the physical Z X C V keys
 * they already live on, and Select All / Save / Find / Find-Next keep their
 * mnemonic letters, so nothing new has to be memorised — the only change is
 * that a right thumb replaces the left pinky reaching for Ctrl.
 *
 * Media pairs across the inner keys the same way the base layer does: Mute and
 * Play there, previous and next track here.
 *
 * F13-F24 continue _LOWER's F1-F12. Nothing binds them by default on either
 * OS, which is exactly what makes them useful — they are free targets for IDE
 * and window-manager shortcuts that cannot collide with anything.
 *
 * Ins mirrors _LOWER's Del on the same thumb.
 */
[_RAISE] = LAYOUT(
  KC_F13,  KC_F14,  KC_F15,  KC_F16,  KC_F17,  KC_F18,                      KC_F19,  KC_F20,  KC_F21,  KC_F22,  KC_F23,  KC_F24,
  _______, _______, _______, _______, _______, _______,                     _______, _______, KC_PGUP, _______, KC_PSCR, _______,
  _______, ED_SALL, ED_SAVE, _______, ED_FIND, ED_FNXT,                     _______, KC_HOME, KC_PGDN, KC_END,  _______, _______,
  _______, ED_UNDO, ED_CUT,  ED_COPY, ED_PSTE, ED_REDO, KC_MPRV,   KC_MNXT, _______, _______, _______, _______, _______, _______,
                             _______, _______, _______, _______,   _______, _______, KC_INS,  _______
),

/* ADJUST - reached by holding LOWER and RAISE together
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * |      |      |      |      |      |      |                    |      |      |      |      |      | BOOT |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * |      |      |      |      |      |      |                    |      |      |      |      |      |      |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | Caps |RM_NXT|RM_HUE|RM_SAT|RM_VAL|GAME  |-------.    ,-------|      |      |      |      |      |      |
 * |------+------+------+------+------+------|RM_TOGG|    |       |------+------+------+------+------+------|
 * |      |RM_PRV|RM_HUD|RM_SAD|RM_VAD|THEME |-------|    |-------|      |CG_TOG|      |      |      |      |
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *                   |      |      |      | /      /       \      \  |      | CAD  |      |
 *                   `----------------------------'           '------''--------------------'
 *
 * QK_BOOT reboots whichever half is holding the USB cable into its RP2040
 * bootloader, so it mounts as RPI-RP2 and the .uf2 can just be copied over —
 * the same thing double-tapping the physical reset button does. It only ever
 * affects the plugged-in half, so flashing both still means moving the cable.
 * The far top-right corner of a layer that needs two thumbs held is about the
 * hardest place on the board to hit by accident, which is the point.
 */
[_ADJUST] = LAYOUT(
  XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                       XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, QK_BOOT,
  XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                       XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
  KC_CAPS, RM_NEXT, RM_HUEU, RM_SATU, RM_VALU, GAME_TOGGLE,                    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
  XXXXXXX, RM_PREV, RM_HUED, RM_SATD, RM_VALD, THEME_NEXT, RM_TOGG,   XXXXXXX, XXXXXXX, CG_TOGG, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
                             _______, _______, _______, _______,     _______, _______, KC_CAD,  _______
),

};

#ifdef ENCODER_MAP_ENABLE
/* _GAME has no row here for the same reason it has almost no keys:
 * layer_switch_get_encoder_layer() walks layer_state | default_layer_state from
 * the top down and falls back to layer 0 when every candidate is transparent,
 * exactly as the keymap lookup does. Its volume entry was a verbatim copy of
 * _QWERTY's and resolved there anyway.
 */
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][2] = {
    [_QWERTY] = {ENCODER_CCW_CW(KC_VOLD, KC_VOLU),  ENCODER_CCW_CW(KC_VOLD, KC_VOLU)},
    [_GAME]   = {ENCODER_CCW_CW(_______, _______),  ENCODER_CCW_CW(_______, _______)},
    [_LOWER]  = {ENCODER_CCW_CW(RM_PREV, RM_NEXT),  ENCODER_CCW_CW(RM_VALD, RM_VALU)},
    [_RAISE]  = {ENCODER_CCW_CW(RM_SATD, RM_SATU),  ENCODER_CCW_CW(RM_HUED, RM_HUEU)},
    [_ADJUST] = {ENCODER_CCW_CW(_______, _______),  ENCODER_CCW_CW(_______, _______)},
};
#endif
// clang-format on

/* The media key counts taps the way a headphone button does: one for
 * play/pause, two for the next track, three or more for the previous one.
 *
 * The cost is inherent and worth stating: nothing can fire until the tap window
 * closes, so play/pause lands MEDIA_TAP_TERM after the tap rather than
 * instantly. There is no way around it — a single tap cannot be dispatched
 * before it is known not to be the first of two — and it is the same delay a
 * pair of earbuds has. _RAISE's inner keys stay mapped to previous and next
 * track as the instant, no-guessing path.
 */
#define MEDIA_TAP_TERM 300 // longer than TAPPING_TERM; 200ms is tight for three taps

static void media_tap(tap_dance_state_t *state, void *user_data) {
    switch (state->count) {
        case 1:
            tap_code(KC_MPLY);
            break;
        case 2:
            tap_code(KC_MNXT);
            break;
        default:
            tap_code(KC_MPRV);
            break;
    }
}

tap_dance_action_t tap_dance_actions[] = {
    [TD_MEDIA] = ACTION_TAP_DANCE_FN(media_tap),
};

uint16_t get_tapping_term(uint16_t keycode, keyrecord_t *record) {
    // Only the media dance wants a longer window. Everything else — the pinky
    // shifts especially — keeps the term config.h sets.
    return keycode == TD(TD_MEDIA) ? MEDIA_TAP_TERM : TAPPING_TERM;
}

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
#ifdef RGB_MATRIX_ENABLE
    return rgb_theme_process_record(keycode, record);
#else
    return true;
#endif
}

void keyboard_post_init_user(void) {
#ifdef RGB_MATRIX_ENABLE
    rgb_theme_init();
#endif
}

#ifdef RGB_MATRIX_ENABLE

/* RGB_MATRIX_TIMEOUT cuts the lighting dead the moment it expires. Ramping the
 * value down over the last RGB_FADE_MS turns that into a dim-out instead.
 *
 * The ramp is a pure function of last_input_activity_elapsed(), which
 * SPLIT_ACTIVITY_ENABLE keeps in step across the link, and the resulting
 * config is mirrored to the other half by the core's own RGB_MATRIX_SPLIT
 * sync. So there is no transaction to register and no state to reconcile —
 * the two halves arrive at the same brightness because they are computing the
 * same function of the same clock.
 *
 * Half the timeout, so the board is at full brightness for 30 seconds and then
 * spends the next 30 sliding to black. It used to be 5 seconds of ramp on the
 * end of 55 at full, which was fine when the deck was dark between keystrokes
 * anyway — every theme rests *lit* now, so that would have meant a keyboard
 * glowing at full for the whole minute after the last thing typed on it.
 */
#    define RGB_FADE_MS 30000
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

#endif // RGB_MATRIX_ENABLE

void housekeeping_task_user(void) {
#ifdef RGB_MATRIX_ENABLE
    rgb_fade_task();
#endif
}

#ifdef OLED_ENABLE
bool oled_task_user(void) {
    /* Driven off the same idle clock as the lighting, so the panel and the LEDs
     * sleep together. SPLIT_ACTIVITY_ENABLE means both halves agree on what
     * that clock reads, and the fade out is OLED_FADE_OUT — done inside the
     * display controller, so no per-step brightness writes cross the link.
     *
     * Both calls are cheap to repeat: each only emits I2C on a state change.
     */
    if (last_input_activity_elapsed() >= IDLE_TIMEOUT_MS) {
        oled_off();
        return false; // nothing worth drawing into a panel that is asleep
    }

    oled_on();
    oled_render_animation();
    return false;
}
#endif
