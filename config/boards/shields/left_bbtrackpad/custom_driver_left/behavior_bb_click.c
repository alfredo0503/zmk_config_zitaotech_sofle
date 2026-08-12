/*
 * BB trackpad physical-click behavior.
 *
 * The tiny A320 pad activates the temporary MOUSE layer before a physical
 * click is scanned.  Layer priority therefore cannot reliably implement
 * LOWER + click.  Resolve the click type from the actual LOWER layer state
 * at press time instead:
 *   LOWER active  -> right click
 *   otherwise     -> left click
 */

#define DT_DRV_COMPAT zmk_behavior_bb_click

#include <zephyr/device.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/keymap.h>

#define LOWER_LAYER_ID 1
#define BB_CLICK_POSITION_SLOTS 128
#define CODE_UNSET 0
#define CODE_TO_SLOT(code) ((uint16_t)(code) + 1U)
#define SLOT_TO_CODE(slot) ((uint16_t)(slot) - 1U)

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

static uint16_t button_for_position[BB_CLICK_POSITION_SLOTS];

static uint16_t choose_button(void) {
    return zmk_keymap_layer_active(LOWER_LAYER_ID) ? INPUT_BTN_1 : INPUT_BTN_0;
}

static int bb_click_pressed(struct zmk_behavior_binding *binding,
                            struct zmk_behavior_binding_event event) {
    const uint16_t code = choose_button();

    if (event.position < BB_CLICK_POSITION_SLOTS) {
        button_for_position[event.position] = CODE_TO_SLOT(code);
    }

    input_report_key(zmk_behavior_get_binding(binding->behavior_dev), code, 1, true, K_FOREVER);
    return 0;
}

static int bb_click_released(struct zmk_behavior_binding *binding,
                             struct zmk_behavior_binding_event event) {
    uint16_t code = choose_button();

    if (event.position < BB_CLICK_POSITION_SLOTS &&
        button_for_position[event.position] != CODE_UNSET) {
        code = SLOT_TO_CODE(button_for_position[event.position]);
        button_for_position[event.position] = CODE_UNSET;
    }

    input_report_key(zmk_behavior_get_binding(binding->behavior_dev), code, 0, true, K_FOREVER);
    return 0;
}

static const struct behavior_driver_api bb_click_driver_api = {
    .binding_pressed = bb_click_pressed,
    .binding_released = bb_click_released,
};

#define BB_CLICK_INST(n)                                                                           \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                                \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &bb_click_driver_api);

DT_INST_FOREACH_STATUS_OKAY(BB_CLICK_INST)

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
