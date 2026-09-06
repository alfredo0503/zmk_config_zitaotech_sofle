/*
 * Synchronize keyboard-backlight activity across the dual-BB split.
 *
 * The built-in RGB underglow behavior remains the GLOBAL ON/OFF source.
 * This behavior only tells both halves "a key was pressed; re-check RGB and
 * refresh the physical keyboard-backlight idle timer".
 */

#define DT_DRV_COMPAT zmk_behavior_keyboard_backlight_activity_sync

#include <zephyr/device.h>
#include <zephyr/kernel.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>

#include "../../../arm/zitaotech_sofle/keyboard_backlight_control.h"
#include <zephyr/logging/log.h>
#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

static int kbact_pressed(struct zmk_behavior_binding *binding,
                         struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    ARG_UNUSED(event);
    keyboard_backlight_sync_activity();
    return ZMK_BEHAVIOR_OPAQUE;
}

static int kbact_released(struct zmk_behavior_binding *binding,
                          struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    ARG_UNUSED(event);
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api kbact_driver_api = {
    .binding_pressed = kbact_pressed,
    .binding_released = kbact_released,
    .locality = BEHAVIOR_LOCALITY_GLOBAL,
};

BEHAVIOR_DT_INST_DEFINE(0, NULL, NULL, NULL, NULL, POST_KERNEL,
                        CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &kbact_driver_api);

static int kbact_position_listener(const zmk_event_t *eh) {
    const struct zmk_position_state_changed *ev = as_zmk_position_state_changed(eh);

    if (!ev || !ev->state) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    const struct zmk_behavior_binding binding = {
        .behavior_dev = DEVICE_DT_NAME(DT_NODELABEL(kbact)),
        .param1 = 0,
        .param2 = 0,
    };

    struct zmk_behavior_binding_event event = {
        .layer = 0,
        .position = ev->position,
        .timestamp = ev->timestamp,
    };

    int ret = zmk_behavior_invoke_binding(&binding, event, true);
    if (ret < 0) {
        LOG_WRN("Failed to sync keyboard backlight activity: %d", ret);
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(keyboard_backlight_activity_sync_listener, kbact_position_listener);
ZMK_SUBSCRIPTION(keyboard_backlight_activity_sync_listener, zmk_position_state_changed);

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
