#pragma once

#include <stdbool.h>

/* True while ZMK LOWER layer 1 is active on the central keymap.
 * The central half broadcasts state changes through a GLOBAL behavior so the
 * right split peripheral can use the same value without linking keymap.c.
 */
bool lower_scroll_sync_is_active(void);
