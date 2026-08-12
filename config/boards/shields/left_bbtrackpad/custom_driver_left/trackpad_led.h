/*
 * Copyright (c) 2025 ZitaoTech
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t indicator_tp_get_last_valid_brightness(void);
void indicator_tp_set_mouse_layer(bool active);

#ifdef __cplusplus
}
#endif
