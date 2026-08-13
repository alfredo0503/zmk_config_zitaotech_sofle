/*
 * Copyright (c) 2023 ZitaoTech
 *
 * SPDX-License-Identifier: MIT
 *
 * v6.4.5: DPI feedback LED only.
 * - LOWER + BL_DEC/BL_INC still changes the software backlight value used by
 *   the A320 driver as the pointer-speed/DPI dial.
 * - A brightness change lights this LED at the corresponding level for a
 *   short period, so the user can gauge the selected DPI by LED brightness.
 * - Pointer motion, MOUSE-layer activation, CapsLock and USB mode do NOT
 *   control this LED.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/led.h>
#include <zephyr/logging/log.h>

#include <zmk/backlight.h>
#include "trackpad_led.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

BUILD_ASSERT(DT_HAS_CHOSEN(zmk_trackpad_led),
             "CONFIG_ZMK_TRACKPAD_LED enabled but no zmk,trackpad_led chosen node found");

static const struct device *const led_dev = DEVICE_DT_GET(DT_CHOSEN(zmk_trackpad_led));

#define CHILD_COUNT(...) +1
#define DT_NUM_CHILD(node_id) (DT_FOREACH_CHILD(node_id, CHILD_COUNT))
#define INDICATOR_LED_NUM_LEDS (DT_NUM_CHILD(DT_CHOSEN(zmk_trackpad_led)))

#define BRT_MIN 10
#define POLLING_INTERVAL_MS 10
#define DPI_FEEDBACK_HOLD_MS 5000

static struct k_work_delayable brightness_poll_work;
static struct k_work_delayable dpi_feedback_off_work;
static uint8_t last_valid_brt = 40;
static uint8_t last_backlight_brt = 0;

static void set_led_brightness(uint8_t level) {
    if (!device_is_ready(led_dev)) {
        return;
    }

    for (int i = 0; i < INDICATOR_LED_NUM_LEDS; i++) {
        int err = led_set_brightness(led_dev, i, level);
        if (err < 0) {
            LOG_ERR("Failed to set trackpad LED[%d] brightness: %d", i, err);
        }
    }
}

uint8_t indicator_tp_get_last_valid_brightness(void) { return last_valid_brt; }

static void dpi_feedback_off_handler(struct k_work *work) {
    ARG_UNUSED(work);
    set_led_brightness(0);
}

static void brightness_poll_handler(struct k_work *work) {
    ARG_UNUSED(work);
    uint8_t brt = zmk_backlight_get_brt();

    if (brt != last_backlight_brt) {
        last_backlight_brt = brt;

        if (brt > 0) {
            last_valid_brt = MAX(BRT_MIN, brt);
            set_led_brightness(last_valid_brt);
            k_work_reschedule(&dpi_feedback_off_work, K_MSEC(DPI_FEEDBACK_HOLD_MS));
        } else {
            k_work_cancel_delayable(&dpi_feedback_off_work);
            set_led_brightness(0);
        }
    }

    k_work_reschedule(&brightness_poll_work, K_MSEC(POLLING_INTERVAL_MS));
}

static int indicator_tp_init(void) {
    if (!device_is_ready(led_dev)) {
        LOG_ERR("Trackpad LED device not ready");
        return -ENODEV;
    }

    uint8_t boot_brt = zmk_backlight_get_brt();
    if (boot_brt > 0) {
        last_valid_brt = MAX(BRT_MIN, boot_brt);
    }
    last_backlight_brt = boot_brt;

    set_led_brightness(0);
    k_work_init_delayable(&brightness_poll_work, brightness_poll_handler);
    k_work_init_delayable(&dpi_feedback_off_work, dpi_feedback_off_handler);
    k_work_reschedule(&brightness_poll_work, K_NO_WAIT);

    return 0;
}

SYS_INIT(indicator_tp_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
