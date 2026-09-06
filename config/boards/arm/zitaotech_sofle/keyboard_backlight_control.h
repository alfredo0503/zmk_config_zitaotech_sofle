#pragma once

#include <stdbool.h>

bool keyboard_backlight_is_enabled(void);
void keyboard_backlight_set_enabled(bool enabled);
void keyboard_backlight_activity(void);
