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
    /* Uppercase A-Z, five glyphs at most. The OLED is 32 px across its short
     * axis and the font advances 6, so a sixth glyph does not fit — the panel
     * clips rather than wraps. This is the same ceiling the shift gate's layer
     * names run into, and the reason "Mirror" is called Fold.
     */
    char name[6];

    uint8_t mode;  // the core effect under the deck: ALPHAS_MODS, or SOLID_COLOR
                   // as a base coat for the renderers below
    uint8_t hue;   // resting colour of the *deck* — the per-key LEDs
    uint8_t sat;   //
    uint8_t speed; // never a rate: ALPHAS_MODS reads it as the alpha/mod hue
                   // split, DECK_MIRROR and DECK_ZONES as a hue span, the
                   // reactive decks as a ripple or decay rate

    uint8_t deck; // which renderer draws the per-key LEDs

    uint8_t glow_hue; // underglow, held separately from the deck — see above
    uint8_t glow_sat;

    uint8_t knob_hue; // the two translucent encoder knobs; must stay warm
    uint8_t knob_sat;

    uint8_t accent[3]; // active keys on _LOWER / _RAISE / _ADJUST
    uint8_t accent_sat;
    uint8_t game_hue; // underglow while _GAME is the default layer
    uint8_t caps_hue; // the shifts while Caps Word is armed
} theme_t;

/* Five of the six animations the board's info.json compiles are #undef'd in
 * config.h, because they are functions of x, y or time and none of them knows
 * what a keyboard is. What replaced them is the deck renderers further down.
 *
 * ALPHAS_MODS is the one that stays, and it earns it: LED_FLAG_MODIFIER on this
 * board is exactly the outer pinky column plus the four thumbs — the eight keys
 * a half that carry the red Escape, the blue LOWER and the peach RAISE caps. It
 * is the only stock effect here whose geometry is a fact about the hardware
 * rather than about the coordinate system.
 *
 * An #undef of someone else's #define fails quiet — rename one upstream and the
 * effect simply comes back with nothing to notice it — so this is the tripwire.
 * NONE, SOLID_COLOR and ALPHAS_MODS is the whole mode table, so EFFECT_MAX is 3.
 * If it fires, check the ENABLE_RGB_MATRIX_* spellings against the current
 * quantum/rgb_matrix/animations/ before changing the number.
 */
_Static_assert(RGB_MATRIX_EFFECT_MAX == 3, "an unused stock animation is being compiled in again");

enum deck_kind {
    DECK_EFFECT, // the core effect draws it; we only add indicators on top
    DECK_SPLASH, // rings expanding out from every live keystroke
    DECK_MIRROR, // hue ramps outward from each half's inner edge, mirrored
    DECK_ZONES,  // hue by finger group, dead still
    DECK_TRAIL,  // per-key afterglow decaying back to the resting deck
    DECK_PULSE,  // deck flat; the *underglow* breathes — drawn in the glow block
};

static const theme_t PROGMEM themes[] = {
    // Lulu — the board's own colours. The deck rests amber to match the
    // knobs, so the whole board is lit from the moment it wakes, and every
    // keystroke sends a ring out from under the cap that was struck. The
    // accents are all cool because the deck is warm: an accent within ~30 hue
    // of the deck it is drawn on is invisible, which is what a yellow RAISE at
    // 43 on this 24 deck used to be.
    {.name = "LULU", .mode = RGB_MATRIX_SOLID_COLOR, .hue = 24, .sat = 255, .speed = 96, .deck = DECK_SPLASH, .glow_hue = 12, .glow_sat = 255, .knob_hue = 28, .knob_sat = 180, .accent = {170, 128, 213}, .accent_sat = 255, .game_hue = 85, .caps_hue = 213},

    // Rose — what the board booted into before any of this existed, with the
    // ALPHAS_MODS split finally pointed somewhere. speed is the hue offset
    // applied to LED_FLAG_MODIFIER, so 240 + 20 wraps to 4 and the outer
    // column and thumbs go red under the red Escape while the alphas stay
    // rose. It used to be 128, which put them 112 hue away: a green fringe on
    // a rose board, almost certainly a leftover from reading speed as a rate.
    {.name = "ROSE", .mode = RGB_MATRIX_ALPHAS_MODS, .hue = 240, .sat = 239, .speed = 20, .deck = DECK_EFFECT, .glow_hue = 240, .glow_sat = 255, .knob_hue = 24, .knob_sat = 180, .accent = {170, 43, 128}, .accent_sat = 255, .game_hue = 85, .caps_hue = 43},

    // Mono — the only colour on the board is information. Not actually
    // monochrome: warm white on the alphas, cool white on the mods, 48
    // saturation apart, which is enough to see the outer column and the thumbs
    // as a block and not enough to read as a colour. It used to be sat 0 with
    // speed 0, and ALPHAS_MODS with no saturation and no offset is a very long
    // way of writing SOLID_COLOR. If the split turns out to be invisible on the
    // charcoal plate, .sat is the knob — try 70-90.
    {.name = "MONO", .mode = RGB_MATRIX_ALPHAS_MODS, .hue = 30, .sat = 48, .speed = 128, .deck = DECK_EFFECT, .glow_hue = 0, .glow_sat = 0, .knob_hue = 30, .knob_sat = 0, .accent = {170, 21, 0}, .accent_sat = 255, .game_hue = 85, .caps_hue = 128},

    // Mirror — hue ramps outward from each half's inner edge, so the colour
    // follows the reach of the fingers and both hands are identical. This is
    // what GRADIENT_LEFT_RIGHT was reaching for and structurally cannot do:
    // its x axis runs monotonically 0-224 across the pair, nothing folds it,
    // and the left hand always ends a different colour from the right. Blue
    // under the index fingers easing to rose out at the pinkies.
    {.name = "FOLD", .mode = RGB_MATRIX_SOLID_COLOR, .hue = 170, .sat = 255, .speed = 64, .deck = DECK_MIRROR, .glow_hue = 202, .glow_sat = 255, .knob_hue = 28, .knob_sat = 180, .accent = {21, 85, 0}, .accent_sat = 255, .game_hue = 85, .caps_hue = 43},

    // Zones — colour by finger rather than by geometry: the pinky and its
    // outer reach, ring and middle, index and its stretch, then the thumbs.
    // Completely still, and the only look here that encodes how the board is
    // used rather than where the LEDs happen to sit. Nothing stock comes
    // close, because every built-in effect is a function of x, y or time and
    // none of them can see the matrix column.
    {.name = "ZONES", .mode = RGB_MATRIX_SOLID_COLOR, .hue = 85, .sat = 255, .speed = 60, .deck = DECK_ZONES, .glow_hue = 107, .glow_sat = 255, .knob_hue = 26, .knob_sat = 180, .accent = {213, 21, 0}, .accent_sat = 255, .game_hue = 85, .caps_hue = 213},

    // Trail — each key flares on the strike and decays back to the resting
    // deck over about a second. A trail of where the hands have been rather
    // than a ring expanding away from it, and much quieter under fast typing
    // than Lulu is. TYPING_HEATMAP is the nearest stock equivalent and is
    // wrong three ways: it hard-codes a blue-to-red ramp that ignores the
    // theme hue, it wants a framebuffer in RAM, and it rests at black.
    {.name = "TRAIL", .mode = RGB_MATRIX_SOLID_COLOR, .hue = 205, .sat = 255, .speed = 96, .deck = DECK_TRAIL, .glow_hue = 205, .glow_sat = 255, .knob_hue = 24, .knob_sat = 200, .accent = {43, 85, 128}, .accent_sat = 255, .game_hue = 85, .caps_hue = 43},

    // Pulse — the deck held perfectly flat while the outline breathes. Only
    // possible because the underglow is already overwritten every frame below,
    // so the two groups can be driven independently; no stock effect can move
    // one and not the other, and BREATHING takes the whole board including the
    // deck to black at the trough. The calmest theme here, and the one that
    // makes the most of the underglow being a neon line rather than a wash.
    {.name = "PULSE", .mode = RGB_MATRIX_SOLID_COLOR, .hue = 128, .sat = 255, .speed = 70, .deck = DECK_PULSE, .glow_hue = 128, .glow_sat = 255, .knob_hue = 24, .knob_sat = 200, .accent = {213, 21, 0}, .accent_sat = 255, .game_hue = 85, .caps_hue = 213},
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
/* For the _ADJUST config panel. Copied into a static rather than returning a
 * pointer into the caller's stack copy of the theme, since the OLED renderer
 * holds it across draw_text().
 */
const char *rgb_theme_name(void) {
    static char name[sizeof(((theme_t *)0)->name)];
    theme_t     t;
    theme_load(&t);
    memcpy(name, t.name, sizeof(name));
    return name;
}

static void zones_init(void); // defined with the deck renderers, further down

void rgb_theme_init(void) {
    theme_config.raw = eeconfig_read_user();
    zones_init();
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

/* ── Underglow brightness is normalised across hues ───────────────────────
 *
 * The outline is six LEDs a half shining out through a slot, and whether it
 * reads as a *line* or as six dots depends on whether each one's diffusion
 * reaches its neighbours. That is a question about apparent brightness, and
 * apparent brightness is not `val` — it is `val` weighted by how much of the
 * colour the eye can actually see.
 *
 * The eye is roughly 10x more sensitive to green than to blue, so at identical
 * hsv the outlines were spread over a 4:1 range:
 *
 *     Fold   hue 202 violet   luma  35      six dots
 *     Trail  hue 205 violet   luma  37
 *     Rose   hue 240 rose     luma  36
 *     Lulu   hue  12 amber    luma  62      a line
 *     Zones  hue 107 green    luma 113      a line, and too hot
 *     Pulse  hue 128 cyan     luma 117
 *     Mono   white            luma 150
 *
 * So the fix is not a brighter number on one theme. Every glow hue is solved
 * for the value that lands it on the *same* perceived brightness — the amber
 * one, which is the level that already looked right, and which is also the
 * ceiling: the violets only just reach it at full value, so nothing higher is
 * available to normalise to.
 *
 * Luminance is linear in v for a fixed hue and saturation, so this is one
 * divide: probe the hue at full value, then scale.
 *
 * This is deliberately not a per-theme knob. A knob would have to be set
 * correctly for every new theme and would be wrong by default; the hue already
 * carries the information needed to derive it. `glow_sat` remains the tuning
 * knob that matters — a paler outline is a brighter one, because white has the
 * most luminance of anything the LED can make.
 */
#    define GLOW_REF_LUMA 106 // amber (hue 12, sat 255) at full value

static uint8_t glow_value(uint8_t hue, uint8_t sat, uint8_t val) {
    const RGB probe = hsv_to_rgb((HSV){.h = hue, .s = sat, .v = 255});
    // Rec. 709 weights, x256: R 0.2126, G 0.7152, B 0.0722. Peaks at 254 for
    // white, and bottoms out at 17 for pure blue — never zero, so no guard.
    const uint8_t luma = (uint8_t)(((uint16_t)54 * probe.r + (uint16_t)183 * probe.g + (uint16_t)18 * probe.b) >> 8);

    const uint16_t want = ((uint16_t)val * GLOW_REF_LUMA) / (luma ? luma : 1);
    return want > 255 ? 255 : (uint8_t)want;
}

static void light(uint8_t row, uint8_t col, RGB rgb, uint8_t led_min, uint8_t led_max) {
    const uint8_t led = g_led_config.matrix_co[row][col];
    if (led == NO_LED || led < led_min || led >= led_max) {
        return;
    }
    rgb_matrix_set_color(led, rgb.r, rgb.g, rgb.b);
}

/* ── deck renderers ───────────────────────────────────────────────────────
 *
 * Each of these paints the per-key LEDs itself, on top of a SOLID_COLOR base
 * coat laid down by the core effect. They exist because the stock animations
 * are all functions of x, y or time, and none of the looks below is.
 *
 * Every one reads the *live* hue/sat/speed rather than the theme's, so a theme
 * seeds them and RM_HUEU/RM_SATD/RM_SPDU keep working from there. Reading the
 * table instead would repaint the deck back to the theme's own colour on every
 * frame and make the knobs do nothing — which is exactly the bug the reactive
 * decks shipped with.
 *
 * The underglow is skipped throughout: it is drawn separately below, and a deck
 * effect crawling along the outline would fight the steady line that is the
 * best thing about it.
 */

/* Hue ramps outward from each half's inner edge, mirrored about the gap, so
 * both hands are identical and the colour follows the reach of the fingers.
 *
 * GRADIENT_LEFT_RIGHT cannot do this. Its x runs monotonically 0-224 across the
 * pair — left half 0-103, right half 120-224 — and nothing in it folds that
 * axis, so the two hands always end on different hues. Folding is the entire
 * difference, and it is one line.
 *
 * speed is the hue span across the reach, not a rate: at 64 the pinkies land 64
 * hue from the index fingers.
 */
static void mirror_deck(uint8_t val, uint8_t led_min, uint8_t led_max) {
    const uint8_t hue  = rgb_matrix_get_hue();
    const uint8_t sat  = rgb_matrix_get_sat();
    const uint8_t span = rgb_matrix_get_speed();

    for (uint8_t i = led_min; i < led_max; i++) {
        if (g_led_config.flags[i] & LED_FLAG_UNDERGLOW) {
            continue;
        }
        const uint8_t x = g_led_config.point[i].x;
        // Distance from this half's inner edge. The gap is empty, so anything
        // at or below the left half's inner key (x 103) belongs to the left.
        const uint8_t d   = x <= 110 ? 103 - x : x - 120;
        const RGB     rgb = hsv_to_rgb((HSV){.h = hue + (uint8_t)(((uint16_t)span * d) / 104), .s = sat, .v = val});
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
}

/* Colour by finger. The LED map has no idea which finger reaches a key — it
 * only knows where the diode is — so the grouping comes off the matrix column,
 * walked once at init into a table rather than reverse-searched every frame.
 *
 * speed is the hue step between zones.
 */
enum { ZONE_PINKY, ZONE_MID, ZONE_INDEX, ZONE_THUMB };

static uint8_t led_zone[RGB_MATRIX_LED_COUNT];

static void zones_init(void) {
    memset(led_zone, ZONE_THUMB, sizeof(led_zone));
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            const uint8_t led = g_led_config.matrix_co[row][col];
            if (led == NO_LED) {
                continue;
            }
            // Rows 4 and 9 are the thumb clusters, one per half. Everything
            // else is a finger column: 0-1 pinky and its outer reach, 2-3 ring
            // and middle, 4-5 index and its stretch.
            led_zone[led] = (row == 4 || row == 9) ? ZONE_THUMB : col <= 1 ? ZONE_PINKY : col <= 3 ? ZONE_MID : ZONE_INDEX;
        }
    }
}

static void zones_deck(uint8_t val, uint8_t led_min, uint8_t led_max) {
    const uint8_t hue  = rgb_matrix_get_hue();
    const uint8_t sat  = rgb_matrix_get_sat();
    const uint8_t step = rgb_matrix_get_speed() / 4;

    for (uint8_t i = led_min; i < led_max; i++) {
        if (g_led_config.flags[i] & LED_FLAG_UNDERGLOW) {
            continue;
        }
        const RGB rgb = hsv_to_rgb((HSV){.h = hue + led_zone[i] * step, .s = sat, .v = val});
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
}

#    ifdef RGB_MATRIX_KEYREACTIVE_ENABLED

/* Per-key afterglow: the struck key flares and decays back to the resting deck.
 *
 * This walks the *hits* rather than the LEDs — there are at most
 * LED_HITS_TO_REMEMBER of them and each one names the LED it landed on, so this
 * is a handful of iterations a frame against the splash's 58. Forward order
 * matters: the newest hit is last in the buffer, so a key struck twice ends on
 * the fresher flare.
 */
static void trail_overlay(uint8_t val, uint8_t led_min, uint8_t led_max) {
    const uint8_t hue   = rgb_matrix_get_hue();
    const uint8_t sat   = rgb_matrix_get_sat();
    const uint8_t speed = rgb_matrix_get_speed();

    for (uint8_t j = 0; j < g_last_hit_tracker.count; j++) {
        const uint8_t i = g_last_hit_tracker.index[j];
        if (i < led_min || i >= led_max || (g_led_config.flags[i] & LED_FLAG_UNDERGLOW)) {
            continue;
        }
        const uint16_t age = scale16by8(g_last_hit_tracker.tick[j], qadd8(speed, 1));
        if (age >= 255) {
            continue; // faded out; SOLID_COLOR's resting pixel already stands
        }
        const RGB rgb = hsv_to_rgb((HSV){.h = hue, .s = sat, .v = qadd8(val, 255 - (uint8_t)age)});
        rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
    }
}

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
static void splash_overlay(uint8_t val, uint8_t led_min, uint8_t led_max) {
    const uint8_t hue   = rgb_matrix_get_hue();
    const uint8_t sat   = rgb_matrix_get_sat();
    const uint8_t speed = rgb_matrix_get_speed();
    const uint8_t count = g_last_hit_tracker.count;

    for (uint8_t i = led_min; i < led_max; i++) {
        if (g_led_config.flags[i] & LED_FLAG_UNDERGLOW) {
            continue;
        }

        uint8_t ripple = 0;
        for (uint8_t j = 0; j < count; j++) {
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

    switch (t.deck) {
        case DECK_MIRROR:
            mirror_deck(val, led_min, led_max);
            break;
        case DECK_ZONES:
            zones_deck(val, led_min, led_max);
            break;
#    ifdef RGB_MATRIX_KEYREACTIVE_ENABLED
        case DECK_SPLASH:
            splash_overlay(val, led_min, led_max);
            break;
        case DECK_TRAIL:
            trail_overlay(val, led_min, led_max);
            break;
#    endif
        default:
            // DECK_EFFECT leaves the deck to the core animation; DECK_PULSE
            // wants it flat, which SOLID_COLOR has already drawn.
            break;
    }

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

    /* DECK_PULSE breathes here rather than on the deck, which is the whole
     * theme: no stock effect can move one LED group and hold the other, since
     * they all walk every LED the flags let them touch. BREATHING would take
     * the deck to black at the trough along with the outline.
     *
     * The maths is upstream's own, out of breathing_anim.h — sin8 folded about
     * its midpoint so the ramp is symmetric, then doubled to reach full swing.
     */
    const uint8_t glow_hue = game ? t.game_hue : t.glow_hue;
    const uint8_t glow_sat = game ? t.accent_sat : t.glow_sat;

    // Normalised first, so the breathing scales a value that already reads the
    // same as every other theme's rather than one that starts four times too
    // dim or twice too hot.
    uint8_t glow_val = glow_value(glow_hue, glow_sat, val);
    if (t.deck == DECK_PULSE && !game) {
        const uint8_t phase = scale16by8(g_rgb_timer, qadd8(rgb_matrix_get_speed() / 8, 1));
        glow_val            = scale8(glow_val, abs8(sin8(phase) - 128) * 2);
    }

    const RGB glow = hsv_to_rgb((HSV){.h = glow_hue, .s = glow_sat, .v = glow_val});
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
