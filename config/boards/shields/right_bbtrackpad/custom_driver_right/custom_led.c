/*
 * Right BB trackpad LED / DPI helper.
 *
 * v4: LED follows the globally synchronized MOUSE-layer state only.
 *     ZMK backlight brightness remains the pointer-speed/DPI dial.
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

static struct k_work_delayable brightness_poll_work;
static uint8_t last_valid_brt = 40;
static uint8_t last_backlight_brt = 0;
static bool mouse_layer_led_on = false;

static void apply_led(uint8_t level) {
    if (!device_is_ready(led_dev)) {
        return;
    }

    for (int i = 0; i < LED_NUM; i++) {
        int err = led_set_brightness(led_dev, i, level);
        if (err < 0) {
            LOG_ERR("Failed to set right trackpad LED[%d] brightness: %d", i, err);
        }
    }
}

void custom_led_set_mouse_layer(bool active) {
    mouse_layer_led_on = active;
    apply_led(active ? MAX(BRT_MIN, last_valid_brt) : 0);
}

uint8_t custom_led_get_last_valid_brightness(void) { return last_valid_brt; }

static void brightness_poll_handler(struct k_work *work) {
    uint8_t brt = zmk_backlight_get_brt();

    if (brt != last_backlight_brt) {
        last_backlight_brt = brt;
        if (brt > 0) {
            last_valid_brt = MAX(BRT_MIN, brt);
            if (mouse_layer_led_on) {
                apply_led(last_valid_brt);
            }
        }
    }

    k_work_reschedule(&brightness_poll_work, K_MSEC(POLLING_INTERVAL_MS));
}

static int init_led_follow(void) {
    if (!device_is_ready(led_dev)) {
        LOG_ERR("Right trackpad LED device not ready");
        return -ENODEV;
    }

    uint8_t boot_brt = zmk_backlight_get_brt();
    if (boot_brt > 0) {
        last_valid_brt = MAX(BRT_MIN, boot_brt);
    }
    last_backlight_brt = boot_brt;
    mouse_layer_led_on = false;
    apply_led(0);

    k_work_init_delayable(&brightness_poll_work, brightness_poll_handler);
    k_work_reschedule(&brightness_poll_work, K_NO_WAIT);

    return 0;
}

SYS_INIT(init_led_follow, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
