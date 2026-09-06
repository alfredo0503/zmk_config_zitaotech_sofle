/*
 * Split-synchronized keyboard backlight control for ZitaoTech Sofle.
 *
 * - Toggle is converted on the central half to an absolute ON/OFF command.
 * - ON/OFF is then executed globally, so both halves change together.
 * - While backlight mode is enabled, each key press seen by the central half
 *   broadcasts an ACTIVITY command so both halves share the same 30 s timer.
 */

#define DT_DRV_COMPAT zmk_behavior_keyboard_backlight_sync

#include <errno.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/rgb_underglow.h>

#include "keyboard_backlight_control.h"

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>
#endif

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

#define KBL_TOGGLE   0
#define KBL_ON       1
#define KBL_OFF      2
#define KBL_ACTIVITY 3

static int kbl_convert_central_state(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    ARG_UNUSED(event);

    if (binding->param1 == KBL_TOGGLE) {
        binding->param1 = keyboard_backlight_is_enabled() ? KBL_OFF : KBL_ON;
    }

    return 0;
}

static int kbl_pressed(struct zmk_behavior_binding *binding,
                       struct zmk_behavior_binding_event event) {
    ARG_UNUSED(event);

    switch (binding->param1) {
    case KBL_TOGGLE:
        /* Fallback for an invocation that was not centrally converted. */
        keyboard_backlight_set_enabled(!keyboard_backlight_is_enabled());
        break;
    case KBL_ON:
#if IS_ENABLED(CONFIG_ZMK_RGB_UNDERGLOW)
        zmk_rgb_underglow_on();
#endif
        keyboard_backlight_set_enabled(true);
        break;
    case KBL_OFF:
#if IS_ENABLED(CONFIG_ZMK_RGB_UNDERGLOW)
        zmk_rgb_underglow_off();
#endif
        keyboard_backlight_set_enabled(false);
        break;
    case KBL_ACTIVITY:
        keyboard_backlight_activity();
        break;
    default:
        return -ENOTSUP;
    }

    return ZMK_BEHAVIOR_OPAQUE;
}

static int kbl_released(struct zmk_behavior_binding *binding,
                        struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    ARG_UNUSED(event);
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api kbl_driver_api = {
    .binding_convert_central_state_dependent_params = kbl_convert_central_state,
    .binding_pressed = kbl_pressed,
    .binding_released = kbl_released,
    .locality = BEHAVIOR_LOCALITY_GLOBAL,
};

BEHAVIOR_DT_INST_DEFINE(0, NULL, NULL, NULL, NULL, POST_KERNEL,
                        CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &kbl_driver_api);

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
static int kbl_activity_listener(const zmk_event_t *eh) {
    const struct zmk_position_state_changed *ev = as_zmk_position_state_changed(eh);

    if (!ev || !ev->state || !keyboard_backlight_is_enabled()) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    const struct zmk_behavior_binding binding = {
        .behavior_dev = DEVICE_DT_NAME(DT_NODELABEL(kbl)),
        .param1 = KBL_ACTIVITY,
        .param2 = 0,
    };

    struct zmk_behavior_binding_event event = {
        .layer = 0,
        .position = ev->position,
        .timestamp = ev->timestamp,
    };

    zmk_behavior_invoke_binding(&binding, event, true);
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(keyboard_backlight_activity_listener, kbl_activity_listener);
ZMK_SUBSCRIPTION(keyboard_backlight_activity_listener, zmk_position_state_changed);
#endif

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
