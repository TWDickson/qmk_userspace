# NOTE: make does not have // comments. Putting one after a value makes it part
# of the value, so keep any explanation on its own line.

SRC += oled_animation.c

ENCODER_MAP_ENABLE = yes
WPM_ENABLE = yes
LTO_ENABLE = yes

# VIA is deliberately off. With it enabled the live keymap is the EEPROM copy,
# not this source tree, and the two only resync when VIA's build-date magic
# changes — so edits here appeared to do nothing until a rebuild on a later
# day, and VIA edits were silently wiped when that happened. It also pulled in
# a raw HID interface. keymap.c is now the only keymap.
VIA_ENABLE = no

# CONSOLE_ENABLE adds a HID interface and makes every uprintf a USB transfer.
# Turn it on only while actually debugging, and turn it back off afterwards.
CONSOLE_ENABLE = no
