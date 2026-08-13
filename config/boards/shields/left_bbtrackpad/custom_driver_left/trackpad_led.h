/*
 * Copyright (c) 2025 ZitaoTech
 * SPDX-License-Identifier: MIT
 *
 * v6.4.6 LED roles:
 * - Board/backlight LED: DPI indicator via ZMK backlight (BL_DEC/BL_INC).
 * - Physical BB touchpad illumination: MOUSE-layer indicator only.
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Last non-zero ZMK backlight value; A320 uses this as the DPI/speed dial. */
uint8_t indicator_tp_get_last_valid_brightness(void);

/* Turn the physical BB touchpad illumination on only while MOUSE layer is active. */
void indicator_tp_set_mouse_layer(bool active);

#ifdef __cplusplus
}
#endif
