/*
 * Global mouse-layer LED synchronization behavior.
 * Press = LED on, release = LED off.
 * On the central half, a layer-state listener invokes this behavior whenever
 * temporary MOUSE layer 3 changes. GLOBAL locality forwards it to peripheral.
 */

#define DT_DRV_COMPAT zmk_behavior_mouse_led_sync

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/event_manager.h>
#include <zmk/events/layer_state_changed.h>

#include "trackpad_led.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

static int mouse_led_pressed(struct zmk_behavior_binding *binding,
                             struct zmk_behavior_binding_event event) {
    indicator_tp_set_mouse_layer(true);
    return ZMK_BEHAVIOR_OPAQUE;
}

static int mouse_led_released(struct zmk_behavior_binding *binding,
                              struct zmk_behavior_binding_event event) {
    indicator_tp_set_mouse_layer(false);
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api mouse_led_driver_api = {
    .binding_pressed = mouse_led_pressed,
    .binding_released = mouse_led_released,
    .locality = BEHAVIOR_LOCALITY_GLOBAL,
};

BEHAVIOR_DT_INST_DEFINE(0, NULL, NULL, NULL, NULL, POST_KERNEL,
                        CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &mouse_led_driver_api);

#define MOUSE_LAYER_ID 3

static int mouse_layer_led_listener(const zmk_event_t *eh) {
    const struct zmk_layer_state_changed *ev = as_zmk_layer_state_changed(eh);
    if (!ev || ev->layer != MOUSE_LAYER_ID) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    const struct zmk_behavior_binding binding = {
        .behavior_dev = DEVICE_DT_NAME(DT_NODELABEL(mled)),
        .param1 = 0,
        .param2 = 0,
    };

    struct zmk_behavior_binding_event event = {
        .layer = ev->layer,
        .position = 0,
        .timestamp = ev->timestamp,
    };

    int ret = zmk_behavior_invoke_binding(&binding, event, ev->state);
    if (ret < 0) {
        LOG_WRN("Failed to sync mouse-layer LEDs: %d", ret);
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(mouse_layer_led_sync_listener, mouse_layer_led_listener);
ZMK_SUBSCRIPTION(mouse_layer_led_sync_listener, zmk_layer_state_changed);

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
