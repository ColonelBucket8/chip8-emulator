#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <stdbool.h>

#define CHIP8_RAM_SIZE  4096
#define CHIP8_ROM_START 0x200
#define CHIP8_FONT_START 0x050
#define CHIP8_WIDTH     64
#define CHIP8_HEIGHT    32
#define CHIP8_NUM_REGS  16
#define CHIP8_STACK_LEN 16
#define CHIP8_NUM_KEYS  16

/* Bit-extraction helpers for a 16-bit CHIP-8 opcode */
#define OPCODE_NNN(op) ((uint16_t)((op) & 0x0FFFu))
#define OPCODE_NN(op)  ((uint8_t)((op) & 0x00FFu))
#define OPCODE_N(op)   ((uint8_t)((op) & 0x000Fu))
#define OPCODE_X(op)   ((uint8_t)(((op) >> 8) & 0x0Fu))
#define OPCODE_Y(op)   ((uint8_t)(((op) >> 4) & 0x0Fu))

typedef struct Chip8 {
    uint8_t  memory[CHIP8_RAM_SIZE];              /* 4KB RAM (0x000 - 0xFFF) */
    uint8_t  V[CHIP8_NUM_REGS];                   /* V0 - VF general registers */
    uint16_t I;                                   /* 16-bit index register */
    uint16_t pc;                                  /* Program Counter (starts at 0x200) */
    uint16_t stack[CHIP8_STACK_LEN];              /* 16-level call stack */
    uint8_t  sp;                                  /* Stack pointer */
    uint8_t  delay_timer;                         /* Decrements at 60Hz when > 0 */
    uint8_t  sound_timer;                         /* Decrements at 60Hz & beeps when > 0 */
    uint8_t  display[CHIP8_WIDTH * CHIP8_HEIGHT]; /* 64x32 monochrome pixels (0 or 1) */
    uint8_t  keys[CHIP8_NUM_KEYS];                /* Hex keypad state (0 = up, 1 = down) */
    bool     draw_flag;                           /* Set true when display changes */
} Chip8;

/* Initialize CPU state, clear RAM/display/registers, load built-in fontset, set PC = 0x200 */
void chip8_init(Chip8 *cpu);

/* Load a binary ROM file into memory starting at 0x200. Returns false on error or oversize. */
bool chip8_load_rom(Chip8 *cpu, const char *path);

/* Execute one Fetch-Decode-Execute cycle (YOUR GUIDED WORKSPACE in src/chip8.c) */
void chip8_step(Chip8 *cpu);

/* Decrement delay_timer and sound_timer by 1 if > 0 (called at 60Hz) */
void chip8_update_timers(Chip8 *cpu);

#endif /* CHIP8_H */
