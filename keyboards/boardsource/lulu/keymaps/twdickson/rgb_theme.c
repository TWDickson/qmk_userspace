// Copyright 2026 Taylor Dickson
// SPDX-License-Identifier: GPL-2.0-or-later

#include "rgb_theme.h"
#include "keymap.h"

#ifdef RGB_MATRIX_ENABLE

#    include <string.h>

// splash_overlay() runs upstream's own ripple maths, which is qadd8/qsub8,
// sqrt16 and scale16by8. quantum.h does not pull lib8tion in; rgb_matrix.c
// includes it by this same path.
#    include <lib/lib8tion/lib8tion.h>

/* A theme is the resting look of the board plus the palette the indicators
 * draw from, so that "which layer am I on" and "is Caps Word armed" are told in
 * a colour that belongs with the base rather than clashing with it.
 *
 * QMK hue is 0-255 across the whole wheel, not 0-360 — hsv_to_rgb() picks its
 * sextant with h * 6 / 255. So 0 red, 21 peach, 43 yellow, 85 green, 128 cyan,
 * 170 blue, 213 magenta, 240 rose. The old RGB_MATRIX_DEFAULT_HUE 240 was
 * commented "blue" but is actually rose, which is what "Rose" below preserves.
 *
 * ── What the light is actually landing on ────────────────────────────────
 *
 * The bottom case is BLACK and the top plate is CHARCOAL. Neither is amber —
 * an earlier version of this file said the underglow "lights the amber case"
 * and built a warm-only rule on top of that. It is wrong. There is nothing
 * amber on this board except the two encoder knobs.
 *
 * That matters three ways, and every theme below is built out of them:
 *
 *   The deck (LED_FLAG_KEYLIGHT, 5 columns per half) lights the charcoal plate
 *   between opaque greige keycaps. It is a low-contrast wash — atmosphere and
 *   a rough "where is the board", not illumination. Desaturated colours read
 *   as lit grey and disappear, so the deck wants saturation.
 *
 *   The underglow (LED_FLAG_UNDERGLOW, 6 per half) does not wash anything. It
 *   escapes as a thin line between plate and case and reads as a crisp neon
 *   outline against a black desk — by a distance the most visible thing on the
 *   board. Being an outline against black rather than a wash over amber, it
 *   takes ANY hue cleanly. Cool underglow does not go muddy; it goes neon.
 *
 *   The two encoder knobs are translucent amber, and they are the only
 *   shine-through parts on the board — every keycap is opaque. Their LEDs are
 *   the sole per-key light you see as light rather than as plate glow, so they
 *   are held at a fixed warm colour below instead of being left to the
 *   animation. The amber body multiplies whatever it is given: warm hues blaze,
 *   cool hues go dead brown. knob_hue must stay warm in every theme, and a low
 *   knob_sat is brighter than a high one because the material supplies the
 *   colour.
 *
 * ── Resting lit ──────────────────────────────────────────────────────────
 *
 * Every theme rests lit. The reactive ones get there via splash_overlay()
 * below rather than via a core reactive effect, because no stock reactive
 * effect rests lit — see the comment on that function.
 *
 * ── Accents ──────────────────────────────────────────────────────────────
 *
 * Accents lean on the keycaps where they can: LOWER's cap is blue and RAISE's
 * is peach, so wherever it stays legible the lit keys match the cap of the
 * thumb key being held, and Caps Lock red matches the red Escape. The caps
 * never change, so the meaning of a colour should not either.
 *
 * "Where it stays legible" is the binding half of that rule, and it is what
 * decides the warm-decked themes. An accent within ~30 hue of the deck it is
 * drawn on top of is invisible: Lulu used to accent RAISE at 43 on a deck of
 * 24, which is a yellow layer indicator on an amber board. On a warm deck the
 * accents all go cool and the cap match is given up.
 */
typedef struct {
    uint8_t mode;  // resting animation
    uint8_t hue;   // resting colour of the *deck* — the per-key LEDs
    uint8_t sat;   //
    uint8_t speed; // per-mode: ALPHAS_MODS reads it as the alpha/mod hue split,
                   // the gradients as hue span, a splash theme as ripple rate

    uint8_t splash; // SPLASH_OFF, or which hits splash_overlay() ripples from

    uint8_t glow_hue; // underglow, held separately from the deck — see above
    uint8_t glow_sat;

    uint8_t knob_hue; // the two translucent encoder knobs; must stay warm
    uint8_t knob_sat;

    uint8_t accent[3]; // active keys on _LOWER / _RAISE / _ADJUST
    uint8_t accent_sat;
    uint8_t game_hue; // underglow while _GAME is the default layer
    uint8_t caps_hue; // the shifts while Caps Word is armed
} theme_t;

enum splash_kind {
    SPLASH_OFF, // not a reactive theme
    SPLASH_ONE, // newest hit only — the next keystroke cancels the last ripple
    SPLASH_ALL, // every live hit — fast typing overlaps its own ripples
};

static const theme_t PROGMEM themes[] = {
    // Lulu — the board's own colours. The deck rests amber to match the
    // knobs, so the whole board is lit from the moment it wakes, and every
    // keystroke sends a ring out from under the cap that was struck. ALL
    // rather than ONE so a fast line overlaps its ripples instead of
    // cancelling them. The accents are all cool because the deck is warm.
    {.mode = RGB_MATRIX_SOLID_COLOR, .hue = 24, .sat = 255, .speed = 96, .splash = SPLASH_ALL, .glow_hue = 12, .glow_sat = 255, .knob_hue = 28, .knob_sat = 180, .accent = {170, 128, 213}, .accent_sat = 255, .game_hue = 85, .caps_hue = 213},

    // Rose — what the board booted into before any of this existed, with the
    // ALPHAS_MODS split finally pointed somewhere. speed is the hue offset
    // applied to LED_FLAG_MODIFIER, which on this board is exactly the outer
    // pinky column plus all four thumbs — the eight keys per half that carry
    // the coloured caps. 240 + 20 wraps to 4, so those go red under the red
    // Escape while the alphas stay rose. It used to be 128, which put them 112
    // hue away: a green fringe on a rose board, almost certainly a leftover
    // from reading `speed` as a rate.
    {.mode = RGB_MATRIX_ALPHAS_MODS, .hue = 240, .sat = 239, .speed = 20, .splash = SPLASH_OFF, .glow_hue = 240, .glow_sat = 255, .knob_hue = 24, .knob_sat = 180, .accent = {170, 43, 128}, .accent_sat = 255, .game_hue = 85, .caps_hue = 43},

    // Deep — the blue the old comment claimed the board already was, with a
    // single ripple that the next keystroke restarts rather than stacks: the
    // quieter of the two reactive themes. The deck takes the blue, so LOWER's
    // accent gives it up and goes magenta. The knobs are the only warm thing
    // on it, which is the point — two amber lamps on a cold board.
    {.mode = RGB_MATRIX_SOLID_COLOR, .hue = 170, .sat = 255, .speed = 96, .splash = SPLASH_ONE, .glow_hue = 150, .glow_sat = 255, .knob_hue = 24, .knob_sat = 200, .accent = {213, 43, 0}, .accent_sat = 255, .game_hue = 85, .caps_hue = 213},

    // Ember — the warmest theme, on a gradient running away from the wrists.
    // GRADIENT_UP_DOWN spans `scale8(64, speed) * 4` hue from the top row to
    // the thumbs, so speed is a span and not a rate: 28 gives 4 -> 32, deep
    // red at the number row easing to amber under the thumbs. It was 64,
    // which is a span of 64 and put the thumb row at hue 72 — a chartreuse
    // bottom edge on a theme whose whole idea is that everything is warm.
    {.mode = RGB_MATRIX_GRADIENT_UP_DOWN, .hue = 4, .sat = 255, .speed = 28, .splash = SPLASH_OFF, .glow_hue = 0, .glow_sat = 255, .knob_hue = 32, .knob_sat = 170, .accent = {170, 128, 213}, .accent_sat = 255, .game_hue = 85, .caps_hue = 191},

    // Mono — the only colour on the board is information. Not actually
    // monochrome: it is warm white on the alphas and cool white on the mods,
    // 48 saturation apart, which is enough to see the outer column and the
    // thumbs as a separate block and not enough to read as a colour. It used
    // to be sat 0 with speed 0, and ALPHAS_MODS with no saturation and no
    // offset is a very long way of writing SOLID_COLOR.
    {.mode = RGB_MATRIX_ALPHAS_MODS, .hue = 30, .sat = 48, .speed = 128, .splash = SPLASH_OFF, .glow_hue = 0, .glow_sat = 0, .knob_hue = 30, .knob_sat = 0, .accent = {170, 21, 0}, .accent_sat = 255, .game_hue = 85, .caps_hue = 128},

    // Split — the only theme that uses the board being a split. The x axis of
    // the LED map runs 0-224 across BOTH halves, so GRADIENT_LEFT_RIGHT lays
    // one gradient over the pair: blue on the left hand, violet-rose on the
    // right, with the seam falling in the gap where there is no board. The
    // underglow takes the midpoint so the two outlines meet in the middle.
    {.mode = RGB_MATRIX_GRADIENT_LEFT_RIGHT, .hue = 160, .sat = 255, .speed = 32, .splash = SPLASH_OFF, .glow_hue = 188, .glow_sat = 255, .knob_hue = 28, .knob_sat = 180, .accent = {128, 43, 85}, .accent_sat = 255, .game_hue = 85, .caps_hue = 43},

    // Sweep — movement without ever going dark. BAND_SAT bands *saturation*
    // and leaves value alone, so the deck sits pale at full brightness and a
    // saturated teal band travels across it, left hand to right. Every other
    // moving effect this board compiles (BREATHING, BAND_VAL) bands value
    // instead and takes the deck to black at the trough, which is the one
    // thing every theme here is built to avoid.
    {.mode = RGB_MATRIX_BAND_SAT, .hue = 140, .sat = 255, .speed = 40, .splash = SPLASH_OFF, .glow_hue = 140, .glow_sat = 255, .knob_hue = 24, .knob_sat = 200, .accent = {213, 21, 0}, .accent_sat = 255, .game_hue = 85, .caps_hue = 213},
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

/* A theme *seeds* the lighting; it does not own it.
 *
 * These writes are the EEPROM ones, not the _noeeprom ones, and rgb_theme_init()
 * deliberately does not call this. That is what makes RM_HUEU, RM_SATD and
 * RM_SPDU worth having: picking a theme lays down its hue, saturation, speed and
 * mode, the knobs adjust from there, and QMK persists both the same way — so the
 * board comes back up looking like the version of the theme you actually dialled
 * in rather than snapping back to the table.
 *
 * This inverts what the file used to do, which was to re-apply the table at
 * every boot with the _noeeprom setters. That kept the theme index as the single
 * persisted byte, and it also meant every adjustment was silently discarded on
 * the next power cycle — a knob that appeared to work and did not. The write
 * budget is unaffected either way: the writes happen on THEME_NEXT and on the
 * adjustment keycodes, which is exactly where they happened before.
 *
 * Brightness is still not part of a theme — it is a room-lighting preference,
 * not a palette choice — so it is carried through rather than set, and
 * RM_VALU/RM_VALD own it alone. rgb_theme_user_val() rather than
 * rgb_matrix_get_val() because the idle ramp drives the live value and a theme
 * change during the fade would otherwise persist a half-dimmed board.
 */
static void theme_apply(void) {
    theme_t t;
    theme_load(&t);
    rgb_matrix_mode(t.mode);
    rgb_matrix_set_speed(t.speed);
    rgb_matrix_sethsv(t.hue, t.sat, rgb_theme_user_val());
}

/* Only the index is read here. The look itself comes back from rgb_matrix's own
 * persisted config, which is the whole point — see theme_apply().
 *
 * The index still matters at boot because it selects the indicator palette:
 * accents, underglow, knobs and Caps Word are not adjustable from the board, so
 * the table remains their only source.
 *
 * Reflashing with a reordered or shortened themes[] can therefore leave the deck
 * showing the old theme's colours under the new theme's accents. One press of
 * THEME_NEXT resyncs it, and there is no cheap way to detect it that is worth
 * more than that.
 */
void rgb_theme_init(void) {
    theme_config.raw = eeconfig_read_user();
}

void eeconfig_init_user(void) {
    theme_config.raw   = 0;
    theme_config.theme = 0;
    eeconfig_update_user(theme_config.raw);
}

/* THEME_NEXT / THEME_PREV are the *only* way to change how the board looks, and
 * that is deliberate. The stock RM_NEXT, RM_HUEU/RM_HUED and RM_SATU/RM_SATD
 * keycodes used to be mapped across _ADJUST and both _LOWER/_RAISE encoders,
 * and every one of them fought this table: they write mode/hue/sat straight
 * into the EEPROM copy of rgb_matrix_config, which theme_apply() then
 * overwrites from the theme index at the next boot. So a hue nudged with the
 * encoder spent a write on the EEPROM this file goes out of its way to
 * protect, left the board looking like no theme in particular, and was silently
 * discarded on the next power cycle.
 *
 * The line that survives is simply which state theme_apply() does not touch:
 * RM_VALU/RM_VALD (brightness, a room-lighting preference and explicitly not
 * part of a palette) and RM_TOGG (the enable flag) both persist correctly and
 * are still mapped. Everything else is a theme's business.
 */
bool rgb_theme_process_record(uint16_t keycode, keyrecord_t *record) {
    if (!record->event.pressed) {
        return true;
    }
    switch (keycode) {
        case THEME_NEXT:
            theme_config.theme = (theme_config.theme + 1) % THEME_COUNT;
            break;
        case THEME_PREV:
            theme_config.theme = (theme_config.theme + THEME_COUNT - 1) % THEME_COUNT;
            break;
        default:
            return true;
    }
    eeconfig_update_user(theme_config.raw);
    theme_apply();
    return false;
}

static void light(uint8_t row, uint8_t col, RGB rgb, uint8_t led_min, uint8_t led_max) {
    const uint8_t led = g_led_config.matrix_co[row][col];
    if (led == NO_LED || led < led_min || led >= led_max) {
        return;
    }
    rgb_matrix_set_color(led, rgb.r, rgb.g, rgb.b);
}

#    ifdef RGB_MATRIX_KEYREACTIVE_ENABLED

/* The reactive themes are SOLID_COLOR underneath with the ripples painted on
 * top here, rather than SOLID_SPLASH / SOLID_MULTISPLASH.
 *
 * That is not a preference. No stock reactive effect rests lit, including the
 * two this used to use. effect_runner_reactive_splash() opens each LED with
 *
 *     hsv_t hsv = rgb_matrix_config.hsv;
 *     hsv.v     = 0;                         <-- here
 *
 * and only then accumulates the hits, so the qadd8() inside SOLID_SPLASH_math
 * adds ripples to *each other*, never to the resting brightness. Once the last
 * front has passed every LED — about 1.3 s after the last keystroke at speed
 * 96 — the whole deck is black. effect_runner_reactive(), which drives
 * SOLID_REACTIVE_SIMPLE, reaches zero the same way via scale8(255 - offset, v)
 * with offset clamped at 255. The distinction this file used to draw between
 * the two ("splashes add, reactive scales") does not exist: both rest dark.
 *
 * So the effect is inverted instead. Core's SOLID_COLOR lays the resting deck
 * every frame, and this only touches the LEDs that have a live ripple over
 * them, adding to that resting value the way the comments always claimed:
 *
 *     v = qadd8(resting_val, ripple)   here
 *     v = scale8(ripple, hsv.v)        upstream
 *
 * which is also why the flare still peaks at full with the brightness dialled
 * right down for a dark room. The ripple maths is upstream's, unchanged, and
 * g_last_hit_tracker is core's — SPLIT_TRANSPORT_MIRROR is what fills it on
 * the half that is not running process_record.
 *
 * The underglow is skipped because it is overwritten below anyway, and a
 * ripple crawling along the outline would fight the steady line that is the
 * best thing about it.
 */
static void splash_overlay(uint8_t splash, uint8_t val, uint8_t led_min, uint8_t led_max) {
    /* Live config, not the theme table. A theme *seeds* hue/sat/speed; from then
     * on RM_HUEU, RM_SATD and RM_SPDU own them, and a ripple drawn in the
     * table's colour would ignore every one of those and repaint the deck back
     * to the theme's own hue on the next keystroke. The table is still the
     * authority for the indicator palette below — accents, glow, knobs — because
     * nothing on the board adjusts those live.
     */
    const uint8_t hue   = rgb_matrix_get_hue();
    const uint8_t sat   = rgb_matrix_get_sat();
    const uint8_t speed = rgb_matrix_get_speed();
    const uint8_t count = g_last_hit_tracker.count;
    const uint8_t first = splash == SPLASH_ONE ? qsub8(count, 1) : 0;

    for (uint8_t i = led_min; i < led_max; i++) {
        if (g_led_config.flags[i] & LED_FLAG_UNDERGLOW) {
            continue;
        }

        uint8_t ripple = 0;
        for (uint8_t j = first; j < count; j++) {
            const int16_t  dx     = g_led_config.point[i].x - g_last_hit_tracker.x[j];
            const int16_t  dy     = g_led_config.point[i].y - g_last_hit_tracker.y[j];
            const uint8_t  dist   = sqrt16(dx * dx + dy * dy);
            const uint16_t tick   = scale16by8(g_last_hit_tracker.tick[j], qadd8(speed, 1));
            const uint16_t effect = tick - dist;
            ripple                = qadd8(ripple, effect > 255 ? 0 : 255 - effect);
        }

        if (ripple == 0) {
            continue; // no live front here; leave SOLID_COLOR's resting pixel alone
        }
        const RGB rgb = hsv_to_rgb((HSV){.h = hue, .s = sat, .v = qadd8(val, ripple)});
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
}

#    endif // RGB_MATRIX_KEYREACTIVE_ENABLED

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

#    ifdef RGB_MATRIX_KEYREACTIVE_ENABLED
    if (t.splash != SPLASH_OFF) {
        splash_overlay(t.splash, val, led_min, led_max);
    }
#    endif

    /* The underglow is held at the theme's own colour rather than left to the
     * animation. It is the board's outline, not a wash across the caps, and an
     * outline reads better as one steady line than as a gradient crawling
     * around it. That does mean an animation only plays across the deck —
     * which is the half of the board anyone is actually looking at.
     *
     * _GAME borrows the same LEDs for its tint, and gets green in every theme
     * for the same reason Caps Lock gets red below: it is a mode you toggle
     * and stay in, and a state that can be left switched on should announce
     * itself in one colour rather than in whatever the palette fancies.
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

    /* The encoder knobs, last so nothing overrides them.
     *
     * They are translucent amber over an LED, and every keycap on the board is
     * opaque, so these two are the only places the light is seen as light. A
     * theme is therefore never entirely dark or entirely unreadable: whatever
     * the deck is doing, the volume controls are lit, which is the one control
     * you reach for without looking at the board.
     *
     * Drawn after the layer indicator on purpose, even though [4,5] carries
     * RM_TOGG on _ADJUST and would otherwise be lit as part of that layer. An
     * accent hue through an amber knob is a dim brown; a knob that is the same
     * warm colour on every layer is worth more than one more lit key on a
     * layer that already lights ten.
     */
    const RGB knob = hsv_to_rgb((HSV){.h = t.knob_hue, .s = t.knob_sat, .v = val});
    light(4, 5, knob, led_min, led_max); // left  — mute / volume
    light(9, 5, knob, led_min, led_max); // right — media / volume

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
