/* Global mouse-layer LED synchronization behavior, peripheral implementation. */

#define DT_DRV_COMPAT zmk_behavior_mouse_led_sync

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>

#include "custom_led.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

static int mouse_led_pressed(struct zmk_behavior_binding *binding,
                             struct zmk_behavior_binding_event event) {
    custom_led_set_mouse_layer(true);
    return ZMK_BEHAVIOR_OPAQUE;
}

static int mouse_led_released(struct zmk_behavior_binding *binding,
                              struct zmk_behavior_binding_event event) {
    custom_led_set_mouse_layer(false);
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api mouse_led_driver_api = {
    .binding_pressed = mouse_led_pressed,
    .binding_released = mouse_led_released,
    .locality = BEHAVIOR_LOCALITY_GLOBAL,
};

BEHAVIOR_DT_INST_DEFINE(0, NULL, NULL, NULL, NULL, POST_KERNEL,
                        CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &mouse_led_driver_api);

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
