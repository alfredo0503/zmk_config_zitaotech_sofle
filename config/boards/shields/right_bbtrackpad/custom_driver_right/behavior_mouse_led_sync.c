/*
 * Right-half implementation of the global MOUSE-layer LED behavior - v6.4.9.
 *
 * The global behavior still needs to exist on the peripheral so the split
 * behavior can be invoked safely.  However, the user's right-side P0.04 LED
 * is the PCB DPI indicator beside the battery/charge LED, NOT a touchpad
 * MOUSE-layer indicator.  Therefore MOUSE-layer ON/OFF is intentionally a
 * no-op on the right half.
 *
 * Only the LEFT BB touchpad illumination follows MOUSE layer 3.
 */

#define DT_DRV_COMPAT zmk_behavior_mouse_led_sync

#include <zephyr/device.h>
#include <zephyr/kernel.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

static int mouse_led_pressed(struct zmk_behavior_binding *binding,
                             struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    ARG_UNUSED(event);
    return ZMK_BEHAVIOR_OPAQUE;
}

static int mouse_led_released(struct zmk_behavior_binding *binding,
                              struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    ARG_UNUSED(event);
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
