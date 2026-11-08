#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>
#include <stdbool.h>
#include "chip8.h"

/* Create a scaled Win32 window (64*scale x 32*scale) */
bool platform_init(const char *title, int scale);

/* Pump Win32 messages and update the 16-key hexadecimal keypad state.
 * Returns false when the user closes the window or presses ESC. */
bool platform_process_input(uint8_t keys[CHIP8_NUM_KEYS]);

/* Render the 64x32 monochrome framebuffer to the window */
void platform_render(const uint8_t display[CHIP8_WIDTH * CHIP8_HEIGHT]);

/* Play a short beep when sound_timer > 0 */
void platform_beep(void);

/* Millisecond timer and sleep helpers for 60Hz frame pacing */
uint32_t platform_get_ticks(void);
void     platform_sleep(uint32_t ms);

/* Clean up window resources */
void platform_shutdown(void);

#endif /* PLATFORM_H */
