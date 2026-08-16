/*
 * Synchronize actual LOWER layer state from the central half to both A320
 * drivers.  ZMK keymap.c exists only on the central side of a split keyboard,
 * so the right peripheral must not call zmk_keymap_layer_active() directly.
 */

#define DT_DRV_COMPAT zmk_behavior_lower_scroll_sync

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/event_manager.h>
#include <zmk/events/layer_state_changed.h>

#include "lower_scroll_sync.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

#define LOWER_LAYER_ID 1

static bool lower_layer_active;

bool lower_scroll_sync_is_active(void) { return lower_layer_active; }

static int lower_sync_pressed(struct zmk_behavior_binding *binding,
                              struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    ARG_UNUSED(event);
    lower_layer_active = true;
    return ZMK_BEHAVIOR_OPAQUE;
}

static int lower_sync_released(struct zmk_behavior_binding *binding,
                               struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    ARG_UNUSED(event);
    lower_layer_active = false;
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api lower_sync_driver_api = {
    .binding_pressed = lower_sync_pressed,
    .binding_released = lower_sync_released,
    .locality = BEHAVIOR_LOCALITY_GLOBAL,
};

BEHAVIOR_DT_INST_DEFINE(0, NULL, NULL, NULL, NULL, POST_KERNEL,
                        CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &lower_sync_driver_api);

static int lower_layer_sync_listener(const zmk_event_t *eh) {
    const struct zmk_layer_state_changed *ev = as_zmk_layer_state_changed(eh);
    if (!ev || ev->layer != LOWER_LAYER_ID) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    const struct zmk_behavior_binding binding = {
        .behavior_dev = DEVICE_DT_NAME(DT_NODELABEL(lssync)),
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
        LOG_WRN("Failed to sync LOWER scroll state: %d", ret);
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(lower_scroll_sync_listener, lower_layer_sync_listener);
ZMK_SUBSCRIPTION(lower_scroll_sync_listener, zmk_layer_state_changed);

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
