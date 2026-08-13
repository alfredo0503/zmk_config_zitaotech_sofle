/*
 * Right BB board DPI-indicator helper - v6.4.9.
 *
 * The small PCB LED beside the battery/charge indicator displays the current
 * DPI/speed level only when that level is adjusted.  It is intentionally not
 * tied to the MOUSE layer.
 */
#pragma once

#include <stdint.h>

uint8_t custom_led_get_last_valid_brightness(void);
