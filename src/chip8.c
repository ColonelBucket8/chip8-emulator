#include "chip8.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Built-in 4x5 pixel hexadecimal fontset (0-F), loaded at 0x050 - 0x09F */
static const uint8_t CHIP8_FONTSET[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, /* 0 */
    0x20, 0x60, 0x20, 0x20, 0x70, /* 1 */
    0xF0, 0x10, 0xF0, 0x80, 0xF0, /* 2 */
    0xF0, 0x10, 0xF0, 0x10, 0xF0, /* 3 */
    0x90, 0x90, 0xF0, 0x10, 0x10, /* 4 */
    0xF0, 0x80, 0xF0, 0x10, 0xF0, /* 5 */
    0xF0, 0x80, 0xF0, 0x90, 0xF0, /* 6 */
    0xF0, 0x10, 0x20, 0x40, 0x40, /* 7 */
    0xF0, 0x90, 0xF0, 0x90, 0xF0, /* 8 */
    0xF0, 0x90, 0xF0, 0x10, 0xF0, /* 9 */
    0xF0, 0x90, 0xF0, 0x90, 0x90, /* A */
    0xE0, 0x90, 0xE0, 0x90, 0xE0, /* B */
    0xF0, 0x80, 0x80, 0x80, 0xF0, /* C */
    0xE0, 0x90, 0x90, 0x90, 0xE0, /* D */
    0xF0, 0x80, 0xF0, 0x80, 0xF0, /* E */
    0xF0, 0x80, 0xF0, 0x80, 0x80  /* F */
};

void chip8_init(Chip8 *cpu)
{
    memset(cpu, 0, sizeof(Chip8));
    cpu->pc = CHIP8_ROM_START;
    memcpy(&cpu->memory[CHIP8_FONT_START], CHIP8_FONTSET,
           sizeof(CHIP8_FONTSET));
}

bool chip8_load_rom(Chip8 *cpu, const char *path)
{
    FILE *fp = fopen(path, "rb");
    if (!fp)
    {
        fprintf(stderr, "Error: Failed to open ROM '%s'\n", path);
        return false;
    }

    if (fseek(fp, 0, SEEK_END) != 0)
    {
        fclose(fp);
        return false;
    }
    long size = ftell(fp);
    if (size <= 0 || size > (long)(CHIP8_RAM_SIZE - CHIP8_ROM_START))
    {
        fprintf(stderr, "Error: Invalid ROM size (%ld bytes)\n", size);
        fclose(fp);
        return false;
    }
    rewind(fp);

    size_t read_bytes =
        fread(&cpu->memory[CHIP8_ROM_START], 1, (size_t)size, fp);
    fclose(fp);
    return read_bytes == (size_t)size;
}

void chip8_update_timers(Chip8 *cpu)
{
    if (cpu->delay_timer > 0)
    {
        cpu->delay_timer--;
    }
    if (cpu->sound_timer > 0)
    {
        cpu->sound_timer--;
    }
}

/* ============================================================================
 * YOUR GUIDED WORKSPACE: Fetch - Decode - Execute
 * ============================================================================
 * Every CHIP-8 instruction is 2 bytes (16 bits) stored Big-Endian in memory.
 *
 * Pre-extracted variables available inside chip8_step():
 *   opcode : Full 16-bit instruction (e.g., 0xD01F)
 *   nnn    : Lowest 12 bits  (0x0FFF) -> memory address
 *   nn     : Lowest 8 bits   (0x00FF) -> 8-bit immediate constant
 *   n      : Lowest 4 bits   (0x000F) -> 4-bit constant (sprite height)
 *   x      : Bits 8..11      (0x0F00 >> 8) -> register index Vx (0x0 - 0xF)
 *   y      : Bits 4..7       (0x00F0 >> 4) -> register index Vy (0x0 - 0xF)
 *
 * Start with MILESTONE 1 (6 opcodes: 00E0, 1NNN, 6XNN, 7XNN, ANNN, DXYN).
 * Once those 6 are done, run `make test` and `./chip8.exe roms/ibm_logo.ch8`!
 * ============================================================================
 */
void chip8_step(Chip8 *cpu)
{
    /* 1. FETCH: Read 2 big-endian bytes at PC and advance PC by 2 */
    // 0xA2 -> 0x00A2 -> 0xA200
    // 0x2A -> 0x002A
    // 0xA200 | 0x002A = 0xA22A
    uint16_t opcode =
        ((uint16_t)cpu->memory[cpu->pc] << 8) | cpu->memory[cpu->pc + 1];
    cpu->pc += 2;

    /* 2. DECODE fields */
    uint16_t nnn = OPCODE_NNN(opcode);
    uint8_t nn = OPCODE_NN(opcode);
    uint8_t n = OPCODE_N(opcode);
    uint8_t x = OPCODE_X(opcode);
    uint8_t y = OPCODE_Y(opcode);

    /* Suppress unused-variable warnings while TODOs are still unimplemented */
    (void)nnn;
    (void)nn;
    (void)n;
    (void)x;
    (void)y;

    /* 3. EXECUTE: Switch on the top 4 bits (highest nibble) */
    //   opcode:          0x D  0  1  F
    // & mask:            0x F  0  0  0
    // ---------------------------------
    //   result:          0x D  0  0  0  --> matches `case 0xD000:`
    switch (opcode & 0xF000)
    {
    case 0x0000:
        switch (opcode)
        {
        case 0x00E0:
            /* TODO [Milestone 1]: 00E0 (CLS)
             * Clear the entire 64x32 display buffer to 0 and set cpu->draw_flag
             * = true. */
            memset(cpu->display, 0, CHIP8_WIDTH * CHIP8_HEIGHT);
            cpu->draw_flag = true;
            break;

        case 0x00EE:
            /* TODO [Milestone 2]: 00EE (RET)
             * Return from subroutine: decrement cpu->sp, then set cpu->pc =
             * cpu->stack[cpu->sp]. */
            break;

        default:
            break;
        }
        break;

    case 0x1000:
        /* TODO [Milestone 1]: 1NNN (JP addr)
         * Jump directly to address nnn: set cpu->pc = nnn. */
        cpu->pc = nnn;
        break;

    case 0x2000:
        /* TODO [Milestone 2]: 2NNN (CALL addr)
         * Call subroutine at nnn:
         * 1. Store current cpu->pc into cpu->stack[cpu->sp]
         * 2. Increment cpu->sp
         * 3. Set cpu->pc = nnn */
        break;

    case 0x3000:
        /* TODO [Milestone 2]: 3XNN (SE Vx, byte)
         * Skip next instruction (cpu->pc += 2) if cpu->V[x] == nn. */
        break;

    case 0x4000:
        /* TODO [Milestone 2]: 4XNN (SNE Vx, byte)
         * Skip next instruction (cpu->pc += 2) if cpu->V[x] != nn. */
        break;

    case 0x5000:
        /* TODO [Milestone 2]: 5XY0 (SE Vx, Vy)
         * Skip next instruction (cpu->pc += 2) if cpu->V[x] == cpu->V[y]. */
        break;

    case 0x6000:
        /* TODO [Milestone 1]: 6XNN (LD Vx, byte)
         * Load immediate byte nn into register cpu->V[x]. */
        cpu->V[x] = nn;
        break;

    case 0x7000:
        /* TODO [Milestone 1]: 7XNN (ADD Vx, byte)
         * Add nn to cpu->V[x] (carry flag VF is NOT modified). */
        cpu->V[x] += nn;
        break;

    case 0x8000:
        /* TODO [Milestone 3]: 8XY0 - 8XYE (ALU Register Operations)
         * Switch on lowest nibble `n`:
         *   0x0: V[x] = V[y]
         *   0x1: V[x] |= V[y]
         *   0x2: V[x] &= V[y]
         *   0x3: V[x] ^= V[y]
         *   0x4: Add V[y] to V[x], set V[0xF] = 1 if sum > 255 else 0
         *   0x5: Set V[0xF] = (V[x] >= V[y]), then V[x] -= V[y]
         *   0x6: Save LSB (V[x] & 1) to flag, V[x] >>= 1, V[0xF] = flag
         *   0x7: Set V[0xF] = (V[y] >= V[x]), then V[x] = V[y] - V[x]
         *   0xE: Save MSB ((V[x] >> 7) & 1) to flag, V[x] <<= 1, V[0xF] = flag
         */
        break;

    case 0x9000:
        /* TODO [Milestone 2]: 9XY0 (SNE Vx, Vy)
         * Skip next instruction (cpu->pc += 2) if cpu->V[x] != cpu->V[y]. */
        break;

    case 0xA000:
        /* TODO [Milestone 1]: ANNN (LD I, addr)
         * Set index register cpu->I = nnn. */
        cpu->I = nnn;
        break;

    case 0xB000:
        /* TODO [Milestone 2]: BNNN (JP V0, addr)
         * Jump to address nnn + cpu->V[0]. */
        break;

    case 0xC000:
        /* TODO [Milestone 2]: CXNN (RND Vx, byte)
         * Set cpu->V[x] = (uint8_t)(rand() & 0xFF) & nn. */
        break;

    case 0xD000:
        /* TODO [Milestone 1]: DXYN (DRW Vx, Vy, nibble)
         * Draw an N-byte sprite from memory[I .. I + n - 1] at (V[x], V[y]):
         * 1. Set uint8_t start_x = cpu->V[x] % CHIP8_WIDTH;
         *    Set uint8_t start_y = cpu->V[y] % CHIP8_HEIGHT;
         * 2. Set cpu->V[0xF] = 0;
         * 3. Loop row from 0 to n - 1:
         *      uint8_t sprite_byte = cpu->memory[cpu->I + row];
         *      Loop col from 0 to 7:
         *        If bit (sprite_byte & (0x80 >> col)) is non-zero:
         *          Compute px = start_x + col, py = start_y + row.
         *          If px < CHIP8_WIDTH && py < CHIP8_HEIGHT:
         *            size_t idx = py * CHIP8_WIDTH + px;
         *            If cpu->display[idx] == 1, set cpu->V[0xF] = 1
         * (collision!) cpu->display[idx] ^= 1;
         * 4. Set cpu->draw_flag = true; */
        uint8_t start_x = cpu->V[x] % CHIP8_WIDTH;
        uint8_t start_y = cpu->V[y] % CHIP8_HEIGHT;
        cpu->V[0xF] = 0;

        for (int row = 0; row < n; row++)
        {
            uint8_t sprite_byte = cpu->memory[cpu->I + row];

            for (int col = 0; col < 8; col++)
            {
                // col=0 checks 10000000, col=1 checks 01000000, ..., col=7
                // checks 00000001
                if ((sprite_byte & (0x80 >> col)) != 0)
                {
                    int px = start_x + col;
                    int py = start_y + row;

                    if (px < CHIP8_WIDTH && py < CHIP8_HEIGHT)
                    {
                        size_t idx = py * CHIP8_WIDTH + px;
                        if (cpu->display[idx] == 1)
                        {
                            cpu->V[0xF] = 1;
                        }
                        cpu->display[idx] ^= 1;
                    }
                }
            }
        }

        cpu->draw_flag = true;
        break;

    case 0xE000:
        /* TODO [Milestone 4]: Keypad Skip Instructions
         *   nn == 0x9E (EX9E): Skip next instruction (pc += 2) if
         * cpu->keys[cpu->V[x]] != 0 nn == 0xA1 (EXA1): Skip next instruction
         * (pc += 2) if cpu->keys[cpu->V[x]] == 0 */
        break;

    case 0xF000:
        /* TODO [Milestone 4]: Timers, Keypad Wait, Font, BCD, Memory
         * Switch on `nn`:
         *   0x07: V[x] = cpu->delay_timer
         *   0x0A: Wait for key press: scan keys[0..15]; if key i is pressed,
         * V[x] = i; if NO key pressed, repeat this instruction by setting
         * cpu->pc -= 2; 0x15: cpu->delay_timer = V[x] 0x18: cpu->sound_timer =
         * V[x] 0x1E: cpu->I += V[x] 0x29: cpu->I = CHIP8_FONT_START + (V[x] &
         * 0x0F) * 5 0x33: Store BCD of V[x] at memory[I..I+2]: memory[I]   =
         * V[x] / 100; memory[I+1] = (V[x] / 10) % 10; memory[I+2] = V[x] % 10;
         *   0x55: Copy V[0..x] into memory[I .. I+x]
         *   0x65: Load V[0..x] from memory[I .. I+x] */
        break;

    default:
        fprintf(stderr, "Unknown opcode: 0x%04X\n", opcode);
        break;
    }
}
