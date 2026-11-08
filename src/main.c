#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "chip8.h"
#include "platform.h"

#define WINDOW_SCALE     15
#define CYCLES_PER_FRAME 11  /* ~11 cycles * 60 FPS = ~660 Hz CPU clock */
#define FRAME_TIME_MS    16  /* ~60 FPS (16.6ms) */

/* Built-in 132-byte IBM Logo test ROM so you can run `./chip8.exe` even without a file! */
static const uint8_t BUILTIN_IBM_LOGO_ROM[132] = {
    0x00, 0xE0, 0xA2, 0x2A, 0x60, 0x0C, 0x61, 0x08, 0xD0, 0x1F, 0x70, 0x09,
    0xA2, 0x39, 0xD0, 0x1F, 0xA2, 0x48, 0x70, 0x08, 0xD0, 0x1F, 0x70, 0x04,
    0xA2, 0x57, 0xD0, 0x1F, 0x70, 0x08, 0xA2, 0x66, 0xD0, 0x1F, 0x70, 0x08,
    0xA2, 0x75, 0xD0, 0x1F, 0x12, 0x28, 0xFF, 0x00, 0xFF, 0x00, 0x3C, 0x00,
    0x3C, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0xFF,
    0x00, 0x38, 0x00, 0x3F, 0x00, 0x3F, 0x00, 0x38, 0x00, 0xFF, 0x00, 0xFF,
    0x80, 0x00, 0xE0, 0x00, 0xE0, 0x00, 0x80, 0x00, 0x80, 0x00, 0xE0, 0x00,
    0xE0, 0x00, 0x80, 0xF8, 0x00, 0xFC, 0x00, 0x3E, 0x00, 0x3F, 0x00, 0x3B,
    0x00, 0x39, 0x00, 0xF8, 0x00, 0xF8, 0x03, 0x00, 0x07, 0x00, 0x0F, 0x00,
    0xBF, 0x00, 0xFB, 0x00, 0xF3, 0x00, 0xE3, 0x00, 0x43, 0xE0, 0x00, 0xE0,
    0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0xE0, 0x00, 0xE0
};

int main(int argc, char *argv[]) {
    srand((unsigned int)time(NULL));

    Chip8 cpu;
    chip8_init(&cpu);

    if (argc >= 2) {
        if (!chip8_load_rom(&cpu, argv[1])) {
            return EXIT_FAILURE;
        }
        printf("Loaded ROM: %s\n", argv[1]);
    } else {
        memcpy(&cpu.memory[CHIP8_ROM_START], BUILTIN_IBM_LOGO_ROM, sizeof(BUILTIN_IBM_LOGO_ROM));
        printf("No ROM argument provided -> Loaded built-in IBM Logo test ROM!\n");
        printf("Usage: %s [path/to/rom.ch8]\n", argv[0]);
    }

    if (!platform_init("CHIP-8 Emulator (ESC to quit)", WINDOW_SCALE)) {
        fprintf(stderr, "Error: Failed to initialize Win32 window.\n");
        return EXIT_FAILURE;
    }

    /* Initial clear render */
    platform_render(cpu.display);

    while (platform_process_input(cpu.keys)) {
        uint32_t frame_start = platform_get_ticks();

        /* Execute CPU instructions for this 60Hz frame */
        for (int i = 0; i < CYCLES_PER_FRAME; i++) {
            chip8_step(&cpu);
        }

        /* Update 60Hz delay & sound timers */
        uint8_t prev_sound = cpu.sound_timer;
        chip8_update_timers(&cpu);
        if (prev_sound > 0 && cpu.sound_timer == 0) {
            platform_beep();
        }

        /* Redraw window only when display buffer changed */
        if (cpu.draw_flag) {
            platform_render(cpu.display);
            cpu.draw_flag = false;
        }

        uint32_t elapsed = platform_get_ticks() - frame_start;
        if (elapsed < FRAME_TIME_MS) {
            platform_sleep(FRAME_TIME_MS - elapsed);
        }
    }

    platform_shutdown();
    return EXIT_SUCCESS;
}
