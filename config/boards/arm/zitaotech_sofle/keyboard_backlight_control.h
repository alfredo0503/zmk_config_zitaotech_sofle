#pragma once

/*
 * Called by the BB-trackpad GLOBAL split behavior on both halves.
 * The implementation waits briefly for the built-in RGB_TOG command to
 * settle, then mirrors that global RGB on/off state to the physical keyboard
 * backlight and refreshes the 30 s idle timer.
 */
void keyboard_backlight_sync_activity(void);
