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

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
