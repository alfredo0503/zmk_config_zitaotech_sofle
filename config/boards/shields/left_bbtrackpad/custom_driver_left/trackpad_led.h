/*
 * Copyright (c) 2025 ZitaoTech
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Last non-zero software backlight value, used by A320 as the DPI/speed dial. */
uint8_t indicator_tp_get_last_valid_brightness(void);

#ifdef __cplusplus
}
#endif
