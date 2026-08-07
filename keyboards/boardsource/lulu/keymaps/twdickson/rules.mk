# NOTE: make does not have // comments. Putting one after a value makes it part
# of the value, so keep any explanation on its own line.

SRC += oled_animation.c

VIA_ENABLE = yes
ENCODER_MAP_ENABLE = yes
WPM_ENABLE = yes
LTO_ENABLE = yes

# CONSOLE_ENABLE adds a HID interface and makes every uprintf a USB transfer.
# Turn it on only while actually debugging, and turn it back off afterwards.
CONSOLE_ENABLE = no
