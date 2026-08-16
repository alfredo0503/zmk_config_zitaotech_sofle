/*
 * Right split-peripheral receiver for actual LOWER layer state.
 * keymap.c is not linked on ZMK split peripherals, so the central half sends
 * LOWER ON/OFF through this GLOBAL behavior.
 */

#define DT_DRV_COMPAT zmk_behavior_lower_scroll_sync

#include <zephyr/device.h>
#include <zephyr/kernel.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>

#include "lower_scroll_sync.h"

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

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

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
