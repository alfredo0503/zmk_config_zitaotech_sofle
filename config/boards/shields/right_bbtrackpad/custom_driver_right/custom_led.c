/*
 * Right BB board DPI indicator LED - v6.4.9.
 *
 * Hardware role confirmed on the user's keyboard:
 * - P0.04 drives the small PCB LED beside the battery/charge indicator.
 * - It is NOT a MOUSE-layer/touchpad illumination LED.
 *
 * Behavior:
 * - ZMK backlight brightness is the existing A320 pointer-speed/DPI dial.
 * - When LOWER + DPI-/DPI+ changes that value, show the new level on this
 *   board LED for a short time, then turn it fully off.
 * - MOUSE-layer changes and pointer movement never turn this LED on.
 * - The A320 driver reads the last non-zero value from
 *   custom_led_get_last_valid_brightness() for pointer sensitivity.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/led.h>
#include <zephyr/logging/log.h>

#include <zmk/backlight.h>
#include "custom_led.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

BUILD_ASSERT(DT_HAS_CHOSEN(zmk_custom_led),
             "Custom LED enabled but no zmk,custom_led chosen node found");

static const struct device *const led_dev = DEVICE_DT_GET(DT_CHOSEN(zmk_custom_led));

#define CHILD_COUNT(...) +1
#define DT_NUM_CHILD(node_id) (DT_FOREACH_CHILD(node_id, CHILD_COUNT))
#define LED_NUM (DT_NUM_CHILD(DT_CHOSEN(zmk_custom_led)))

#define BRT_MIN 10
#define POLLING_INTERVAL_MS 10
#define DPI_LED_TIMEOUT_MS 3000

static struct k_work_delayable poll_work;
static struct k_work_delayable auto_off_work;

static uint8_t last_backlight_brt = 0;
static uint8_t last_valid_brt = 40;

static void apply_board_led(uint8_t brightness) {
    if (!device_is_ready(led_dev)) {
        return;
    }

    for (int i = 0; i < LED_NUM; i++) {
        int err = led_set_brightness(led_dev, i, brightness);
        if (err < 0) {
            LOG_ERR("Failed to set right DPI indicator LED[%d]: %d", i, err);
        }
    }
}

static void auto_off_handler(struct k_work *work) {
    ARG_UNUSED(work);
    apply_board_led(0);
}

static void poll_handler(struct k_work *work) {
    ARG_UNUSED(work);

    uint8_t brt = zmk_backlight_get_brt();

    if (brt != last_backlight_brt) {
        last_backlight_brt = brt;

        if (brt > 0) {
            last_valid_brt = MAX(BRT_MIN, brt);

            /* DPI changed: show the current level briefly, then go dark. */
            apply_board_led(last_valid_brt);
            k_work_reschedule(&auto_off_work, K_MSEC(DPI_LED_TIMEOUT_MS));
        } else {
            /* A zero backlight value is not a new DPI level; preserve the
             * previous non-zero sensitivity and keep the indicator dark. */
            k_work_cancel_delayable(&auto_off_work);
            apply_board_led(0);
        }
    }

    k_work_reschedule(&poll_work, K_MSEC(POLLING_INTERVAL_MS));
}

uint8_t custom_led_get_last_valid_brightness(void) { return last_valid_brt; }

static int init_dpi_indicator(void) {
    if (!device_is_ready(led_dev)) {
        LOG_ERR("Right DPI indicator LED device not ready");
        return -ENODEV;
    }

    /* Remember the boot DPI value, but do not light the indicator merely
     * because the keyboard powered on. */
    uint8_t boot_brt = zmk_backlight_get_brt();
    if (boot_brt > 0) {
        last_valid_brt = MAX(BRT_MIN, boot_brt);
    }
    last_backlight_brt = boot_brt;

    apply_board_led(0);

    k_work_init_delayable(&auto_off_work, auto_off_handler);
    k_work_init_delayable(&poll_work, poll_handler);
    k_work_reschedule(&poll_work, K_NO_WAIT);

    LOG_INF("Right board DPI indicator initialized (mouse-layer independent)");
    return 0;
}

SYS_INIT(init_dpi_indicator, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
