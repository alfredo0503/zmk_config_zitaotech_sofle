#pragma once

#include <stdbool.h>
#include <stdint.h>

/* Last non-zero brightness, also used as the A320 pointer-speed dial. */
uint8_t custom_led_get_last_valid_brightness(void);

/* Called by the global MOUSE-layer synchronization behavior. */
void custom_led_set_mouse_layer(bool active);

/* Local fallback/diagnostic: right A320 motion should light the right pad LED
 * for roughly the same lifetime as the temporary MOUSE layer.
 */
void custom_led_note_pointer_activity(void);
