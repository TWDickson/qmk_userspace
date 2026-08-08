# NOTE: make does not have // comments. Putting one after a value makes it part
# of the value, so keep any explanation on its own line.

SRC += oled_animation.c
SRC += rgb_theme.c

ENCODER_MAP_ENABLE = yes
WPM_ENABLE = yes
LTO_ENABLE = yes

# Tap once and the next word is capitalised: letters get shift, digits and
# - _ pass through, and the first space or punctuation ends it. Better than
# Caps Lock for SCREAMING_SNAKE_CASE because it cannot be left switched on.
CAPS_WORD_ENABLE = yes

# One media key that counts taps, the way a headphone button does.
TAP_DANCE_ENABLE = yes

# The board's info.json turns mousekey on, but no mouse keycode is mapped
# anywhere in this keymap. Measured at 1452 bytes of flash for nothing
# (47176 -> 45724 text). Turn it back on if mouse keys ever land on a layer.
MOUSEKEY_ENABLE = no

# VIA is deliberately off. With it enabled the live keymap is the EEPROM copy,
# not this source tree, and the two only resync when VIA's build-date magic
# changes — so edits here appeared to do nothing until a rebuild on a later
# day, and VIA edits were silently wiped when that happened. It also pulled in
# a raw HID interface. keymap.c is now the only keymap.
VIA_ENABLE = no

# CONSOLE_ENABLE adds a HID interface and makes every uprintf a USB transfer.
# Turn it on only while actually debugging, and turn it back off afterwards.
CONSOLE_ENABLE = no
