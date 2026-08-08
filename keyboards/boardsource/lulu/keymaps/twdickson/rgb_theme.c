// Copyright 2026 Taylor Dickson
// SPDX-License-Identifier: GPL-2.0-or-later

#include "rgb_theme.h"
#include "keymap.h"

#ifdef RGB_MATRIX_ENABLE

#    include <string.h>

/* A theme is the resting look of the board plus the palette the indicators
 * draw from, so that "which layer am I on" and "is Caps Word armed" are told in
 * a colour that belongs with the base rather than clashing with it.
 *
 * QMK hue is 0-255 across the whole wheel, not 0-360 — hsv_to_rgb() picks its
 * sextant with h * 6 / 255. So 0 red, 43 yellow, 85 green, 128 cyan, 170 blue,
 * 213 magenta. The old RGB_MATRIX_DEFAULT_HUE 240 was commented "blue" but is
 * actually rose, which is what "Rose" below preserves.
 */
typedef struct {
    uint8_t mode;  // resting animation
    uint8_t hue;   // resting colour of the *deck* — the per-key LEDs
    uint8_t sat;   //
    uint8_t speed; // ALPHAS_MODS reads this as the alpha/mod hue split, not a rate

    uint8_t glow_hue; // underglow, held separately from the deck — see below
    uint8_t glow_sat;

    uint8_t accent[3]; // active keys on _LOWER / _RAISE / _ADJUST
    uint8_t accent_sat;
    uint8_t game_hue; // underglow while _GAME is the default layer
    uint8_t caps_hue; // the shifts while Caps Word is armed
} theme_t;

/* The deck and the underglow get separate colours because they light different
 * physical things.
 *
 * The caps on this board are solid grey — not shine-through — over a black
 * metal case, so the per-key LEDs light the plate *around* each cap rather than
 * the legend on it. Against black that wants saturation: a desaturated deck
 * reads as dirty grey.
 *
 * Every theme rests *lit*. The reactive ones are splashes, which add their
 * flare on top of the resting colour rather than scaling it, so a keystroke
 * ripples across a board that was already on. The alternative — a deck that
 * stays dark until a key is struck — reads beautifully against the black case
 * and is no use at all for finding a key, which is the other half of what the
 * deck is for.
 *
 * The underglow lights the amber underside, which is a fixed colour nothing can
 * change. Warm underglow makes it look lit on purpose; cool underglow fights it
 * and goes muddy.
 *
 * Accents lean on the keycaps rather than the palette: LOWER is blue and RAISE
 * is yellow on this board, so wherever it stays legible the lit keys match the
 * cap of the thumb key being held, and Caps Lock red matches the red Escape.
 * The caps never change, so the meaning of a colour should not either.
 */
static const theme_t PROGMEM themes[] = {
    // Lulu — the board's own colours, and the reason SOLID_MULTISPLASH is
    // compiled in. The deck rests amber to match the underside, so the whole
    // board is lit from the moment it wakes, and every keystroke sends a ring
    // out from under the cap that was struck. Multi rather than single so a
    // fast line overlaps its own ripples instead of cancelling them. Accents
    // are the blue LOWER, yellow RAISE and red Escape caps.
    {.mode = RGB_MATRIX_SOLID_MULTISPLASH, .hue = 24, .sat = 255, .speed = 96, .glow_hue = 28, .glow_sat = 255, .accent = {170, 43, 213}, .accent_sat = 255, .game_hue = 0, .caps_hue = 128},
    // Rose — what the board booted into before any of this existed.
    {.mode = RGB_MATRIX_ALPHAS_MODS, .hue = 240, .sat = 239, .speed = 128, .glow_hue = 240, .glow_sat = 200, .accent = {170, 43, 128}, .accent_sat = 255, .game_hue = 0, .caps_hue = 43},
    // Deep — the blue the old comment claimed the board already was, with a
    // single ripple that the next keystroke restarts rather than stacks: the
    // quieter of the two splashes. The deck takes the blue, so LOWER's accent
    // has to give it up and go magenta.
    {.mode = RGB_MATRIX_SOLID_SPLASH, .hue = 170, .sat = 255, .speed = 96, .glow_hue = 190, .glow_sat = 255, .accent = {213, 43, 0}, .accent_sat = 255, .game_hue = 85, .caps_hue = 128},
    // Ember — leans hardest into the underside: everything warm, on a slow
    // gradient, accents all cool so they cannot sink into it.
    {.mode = RGB_MATRIX_GRADIENT_UP_DOWN, .hue = 8, .sat = 255, .speed = 64, .glow_hue = 28, .glow_sat = 255, .accent = {170, 128, 213}, .accent_sat = 255, .game_hue = 85, .caps_hue = 191},
    // Mono — white deck, white underglow, and the amber underside does the
    // tinting in hardware. The only colour on the board is information.
    {.mode = RGB_MATRIX_ALPHAS_MODS, .hue = 0, .sat = 0, .speed = 0, .glow_hue = 0, .glow_sat = 0, .accent = {170, 43, 0}, .accent_sat = 255, .game_hue = 85, .caps_hue = 128},
};

#    define THEME_COUNT (sizeof(themes) / sizeof(themes[0]))

typedef union {
    uint32_t raw;
    struct {
        uint8_t theme;
    };
} theme_config_t;

static theme_config_t theme_config;

static void theme_load(theme_t *out) {
    uint8_t i = theme_config.theme;
    if (i >= THEME_COUNT) {
        i = 0; // a stale or never-written EEPROM byte should not index off the end
    }
    memcpy_P(out, &themes[i], sizeof(*out));
}

/* Brightness is deliberately not part of the theme. It is a room-lighting
 * preference rather than a palette choice, so the EEPROM value survives a theme
 * change and a reboot, and only hue/sat/mode get rewritten.
 *
 * All three writes are _noeeprom: the theme index is the one thing persisted,
 * and re-deriving the rest from it every boot is what keeps this off the
 * EEPROM's write budget.
 */
static void theme_apply(void) {
    theme_t t;
    theme_load(&t);
    rgb_matrix_mode_noeeprom(t.mode);
    rgb_matrix_set_speed_noeeprom(t.speed);
    rgb_matrix_sethsv_noeeprom(t.hue, t.sat, rgb_matrix_get_val());
}

void rgb_theme_init(void) {
    theme_config.raw = eeconfig_read_user();
    theme_apply();
}

void eeconfig_init_user(void) {
    theme_config.raw   = 0;
    theme_config.theme = 0;
    eeconfig_update_user(theme_config.raw);
}

bool rgb_theme_process_record(uint16_t keycode, keyrecord_t *record) {
    if (keycode == THEME_NEXT && record->event.pressed) {
        theme_config.theme = (theme_config.theme + 1) % THEME_COUNT;
        eeconfig_update_user(theme_config.raw);
        theme_apply();
        return false;
    }
    return true;
}

static void light(uint8_t row, uint8_t col, RGB rgb, uint8_t led_min, uint8_t led_max) {
    const uint8_t led = g_led_config.matrix_co[row][col];
    if (led == NO_LED || led < led_min || led >= led_max) {
        return;
    }
    rgb_matrix_set_color(led, rgb.r, rgb.g, rgb.b);
}

/* Everything here is scaled by the *live* value, which is what makes it
 * compose with the idle ramp in keymap.c for free: as rgb_fade_task() walks the
 * value down to zero the indicators dim out with the rest of the board instead
 * of sitting there at full brightness on a sleeping keyboard.
 */
bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    theme_t t;
    theme_load(&t);

    const uint8_t val   = rgb_matrix_get_val();
    const uint8_t layer = get_highest_layer(layer_state | default_layer_state);

    /* The underglow is held at the theme's own colour rather than left to the
     * animation. It is lighting the case, not the caps, and a case that is a
     * fixed colour looks better lit by a steady wash than by a gradient
     * crawling underneath it. That does mean an animation only plays across the
     * deck — which is the half of the board anyone is actually looking at.
     *
     * _GAME borrows the same LEDs for its tint. Doing it here rather than as a
     * full-board wash keeps the deck animating for as long as the game lasts.
     */
    const bool game = layer == _GAME;
    const RGB  glow = hsv_to_rgb((HSV){.h = game ? t.game_hue : t.glow_hue, .s = game ? t.accent_sat : t.glow_sat, .v = val});
    for (uint8_t i = led_min; i < led_max; i++) {
        if (g_led_config.flags[i] & LED_FLAG_UNDERGLOW) {
            rgb_matrix_set_color(i, glow.r, glow.g, glow.b);
        }
    }

    if (layer >= _LOWER) {
        /* Light only the keys the layer actually defines. KC_NO is 0 and
         * KC_TRANSPARENT is 1, so anything above the latter is a real keycode —
         * which on these deliberately sparse layers is a small minority of the
         * board, and exactly the thing worth pointing at.
         */
        const RGB rgb = hsv_to_rgb((HSV){.h = t.accent[layer - _LOWER], .s = t.accent_sat, .v = val});
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            for (uint8_t col = 0; col < MATRIX_COLS; col++) {
                if (keymap_key_to_keycode(layer, (keypos_t){.row = row, .col = col}) > KC_TRANSPARENT) {
                    light(row, col, rgb, led_min, led_max);
                }
            }
        }
    }

    // Caps Lock is a red warning on the key that toggles it, and stays red in
    // every theme — a stuck-on state should not be styled.
    if (host_keyboard_led_state().caps_lock) {
        light(2, 0, (RGB){.r = val, .g = 0, .b = 0}, led_min, led_max);
    }

#    ifdef CAPS_WORD_ENABLE
    /* Caps Word lights the shifts instead, since that is what it stands in for,
     * and it ends on its own so it does not need Caps Lock's warning colour.
     *
     * Expect exactly *one* shift to light: the one on the half holding the USB
     * cable. Caps Word state lives in the master only — there is no
     * SPLIT_CAPS_WORD_ENABLE and the slave never runs the state machine — and a
     * half only ever flushes its own LEDs, so the master's write to the other
     * side's shift goes nowhere. Both are set here so that whichever half is
     * master, its own shift is the one that lights.
     *
     * Do not "fix" this with a custom split transaction. That is the same trade
     * the old timeout_fade.c made and it is not worth 450 lines for an
     * indicator that is already visible on the half you are looking at.
     */
    if (is_caps_word_on()) {
        const RGB rgb = hsv_to_rgb((HSV){.h = t.caps_hue, .s = t.accent_sat, .v = val});
        light(3, 0, rgb, led_min, led_max); // left shift
        light(8, 0, rgb, led_min, led_max); // right shift
    }
#    endif

    return false;
}

#endif // RGB_MATRIX_ENABLE
