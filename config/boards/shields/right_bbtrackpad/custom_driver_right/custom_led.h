#pragma once

#include <stdint.h>

/* Last non-zero software backlight value, used by A320 as the DPI/speed dial. */
uint8_t custom_led_get_last_valid_brightness(void);
