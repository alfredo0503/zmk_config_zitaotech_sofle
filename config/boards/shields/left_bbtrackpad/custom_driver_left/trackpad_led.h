/*
 * Copyright (c) 2025 ZitaoTech
 * SPDX-License-Identifier: MIT
 *
 * v6.4.12 LED roles:
 * - Right PCB/backlight LED remains the DPI indicator.
 * - Left physical BB touchpad LED is steady on MOUSE layer 3.
 * - Left physical BB touchpad LED restores the original breathing indication
 *   while Caps Lock is active.
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Last non-zero ZMK backlight value; A320 uses this as the DPI/speed dial. */
uint8_t indicator_tp_get_last_valid_brightness(void);

/* Set MOUSE-layer state for the physical left BB touchpad illumination. */
void indicator_tp_set_mouse_layer(bool active);

#ifdef __cplusplus
}
#endif
