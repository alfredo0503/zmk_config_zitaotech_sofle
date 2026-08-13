/*
 * Right BB trackpad LED/DPI helper - v6.4.6.
 *
 * The ZMK backlight value remains the DPI/speed dial.  The separate physical
 * touchpad illumination is used only as a MOUSE-layer indicator.
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

uint8_t custom_led_get_last_valid_brightness(void);
void custom_led_set_mouse_layer(bool active);
