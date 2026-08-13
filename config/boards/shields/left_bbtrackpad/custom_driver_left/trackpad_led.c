/*
 * Copyright (c) 2023 ZitaoTech
 * SPDX-License-Identifier: MIT
 *
 * v6.4.12: left BB touchpad LED has two independent indications.
 *
 * - MOUSE layer 3 active: steady touchpad illumination.
 * - Caps Lock active: restore the seller/original breathing LED animation.
 * - Caps Lock takes visual priority while it is active; when Caps Lock turns
 *   off, the LED immediately returns to the current MOUSE-layer state.
 * - ZMK backlight brightness remains a DPI/speed value only and is not mirrored
 *   onto this touchpad LED.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/led.h>
#include <zephyr/logging/log.h>

#include <zmk/backlight.h>
#include <zmk/hid_indicators.h>
#include "trackpad_led.h"

#define HID_INDICATORS_CAPS_LOCK (1 << 1)

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

BUILD_ASSERT(DT_HAS_CHOSEN(zmk_trackpad_led),
             "CONFIG_ZMK_TRACKPAD_LED enabled but no zmk,trackpad_led chosen node found");

static const struct device *const led_dev = DEVICE_DT_GET(DT_CHOSEN(zmk_trackpad_led));

#define CHILD_COUNT(...) +1
#define DT_NUM_CHILD(node_id) (DT_FOREACH_CHILD(node_id, CHILD_COUNT))
#define TOUCHPAD_LED_NUM_LEDS (DT_NUM_CHILD(DT_CHOSEN(zmk_trackpad_led)))

#define BRT_MIN 10
#define BRT_MAX 100
#define BRT_LOW 20
#define BRT_STEP 5
#define CAPS_ANIMATION_INTERVAL_MS 20
#define POLLING_INTERVAL_MS 10

/* Fixed MOUSE-layer indicator brightness, independent from DPI level. */
#define TOUCHPAD_MOUSE_LED_BRT 60

static struct k_work_delayable state_poll_work;
static struct k_work_delayable caps_animation_work;

static uint8_t last_valid_brt = 40;
static uint8_t last_backlight_brt = 0;
static bool mouse_layer_led_on = false;
static bool capslock_on = false;
static bool caps_animation_increasing = true;
static uint8_t caps_brightness = BRT_MIN;

static void set_touchpad_led(uint8_t level) {
    if (!device_is_ready(led_dev)) {
        return;
    }

    for (int i = 0; i < TOUCHPAD_LED_NUM_LEDS; i++) {
        int err = led_set_brightness(led_dev, i, level);
        if (err < 0) {
            LOG_ERR("Failed to set BB touchpad LED[%d] brightness: %d", i, err);
        }
    }
}

static void apply_mouse_layer_led(void) {
    set_touchpad_led(mouse_layer_led_on ? TOUCHPAD_MOUSE_LED_BRT : 0);
}

void indicator_tp_set_mouse_layer(bool active) {
    mouse_layer_led_on = active;

    /* Caps Lock owns the visual indication while it is active. */
    if (!capslock_on) {
        apply_mouse_layer_led();
    }
}

uint8_t indicator_tp_get_last_valid_brightness(void) { return last_valid_brt; }

/* Original seller Caps Lock breathing animation. */
static void caps_animation_handler(struct k_work *work) {
    ARG_UNUSED(work);

    if (!capslock_on) {
        return;
    }

    if (caps_animation_increasing) {
        caps_brightness += BRT_STEP;
        if (caps_brightness >= BRT_MAX) {
            caps_brightness = BRT_MAX;
            caps_animation_increasing = false;
        }
    } else {
        caps_brightness -= BRT_STEP;
        if (caps_brightness <= BRT_LOW) {
            caps_brightness = BRT_LOW;
            caps_animation_increasing = true;
        }
    }

    set_touchpad_led(caps_brightness);
    k_work_reschedule(&caps_animation_work, K_MSEC(CAPS_ANIMATION_INTERVAL_MS));
}

/*
 * Poll two independent states:
 * 1) normal ZMK backlight value, used only as the A320 DPI/speed dial;
 * 2) host Caps Lock indicator, used for the original left touchpad LED animation.
 */
static void state_poll_handler(struct k_work *work) {
    ARG_UNUSED(work);

    uint8_t brt = zmk_backlight_get_brt();
    if (brt != last_backlight_brt) {
        last_backlight_brt = brt;
        if (brt > 0) {
            last_valid_brt = MAX(BRT_MIN, brt);
        }
    }

    bool current_capslock =
        (zmk_hid_indicators_get_current_profile() & HID_INDICATORS_CAPS_LOCK) != 0;

    if (current_capslock != capslock_on) {
        capslock_on = current_capslock;

        if (capslock_on) {
            caps_brightness = BRT_MIN;
            caps_animation_increasing = true;
            k_work_reschedule(&caps_animation_work, K_NO_WAIT);
            LOG_DBG("Caps Lock on: left BB touchpad breathing LED enabled");
        } else {
            k_work_cancel_delayable(&caps_animation_work);
            apply_mouse_layer_led();
            LOG_DBG("Caps Lock off: restored MOUSE-layer LED state");
        }
    }

    k_work_reschedule(&state_poll_work, K_MSEC(POLLING_INTERVAL_MS));
}

static int indicator_tp_init(void) {
    if (!device_is_ready(led_dev)) {
        LOG_ERR("BB touchpad LED device not ready");
        return -ENODEV;
    }

    uint8_t boot_brt = zmk_backlight_get_brt();
    if (boot_brt > 0) {
        last_valid_brt = MAX(BRT_MIN, boot_brt);
    }
    last_backlight_brt = boot_brt;

    mouse_layer_led_on = false;
    capslock_on = false;
    caps_animation_increasing = true;
    caps_brightness = BRT_MIN;
    set_touchpad_led(0);

    k_work_init_delayable(&state_poll_work, state_poll_handler);
    k_work_init_delayable(&caps_animation_work, caps_animation_handler);
    k_work_reschedule(&state_poll_work, K_NO_WAIT);

    return 0;
}

SYS_INIT(indicator_tp_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
