# Guided CHIP-8 Emulator in C

Welcome to your **CHIP-8 Emulator** starter project!
All the Windows graphics (`StretchDIBits`), 60Hz frame timing, and 16-key input handling are pre-built so you can focus 100% on writing the CPU emulator core in [`src/chip8.c`](src/chip8.c).

---

## Quick Start

```powershell
# 1. Run the self-check test suite to see your progress:
make test

# 2. Build and launch the emulator window (loads built-in IBM Logo ROM by default):
make
./chip8.exe

# Or pass any .ch8 ROM file:
./chip8.exe roms/ibm_logo.ch8
```

---

## Architecture Overview

| Component | File | Status |
|---|---|---|
| CPU State & Constants | `include/chip8.h` | Complete |
| Fetch-Decode-Execute Core | `src/chip8.c` | **Your Guided Workspace (`TODO`s)** |
| Win32 GDI Display & Input | `src/platform_win32.c` | Complete |
| 60Hz Main Loop & Built-in ROM | `src/main.c` | Complete |
| Self-Check Unit Tests | `tests/test_opcodes.c` | Complete (`make test`) |

---

## How CHIP-8 Works

1. **Memory (`4096` bytes)**:
   - `0x000`–`0x1FF`: Reserved for interpreter (fontset lives at `0x050`–`0x09F`).
   - `0x200`–`0xFFF`: Program ROM and work RAM (`pc` starts at `0x200`).
2. **Registers**:
   - `V[0]` .. `V[0xE]`: 15 general-purpose 8-bit registers.
   - `V[0xF]` (`VF`): Flag register (used for arithmetic carry, borrow, and sprite collision).
   - `I`: 16-bit index register used for memory addresses.
   - `pc`: 16-bit Program Counter.
   - `stack[16]` & `sp`: 16-level call stack for `2NNN` (`CALL`) and `00EE` (`RET`).
3. **Display (`64 x 32` monochrome)**:
   - `display[64 * 32]` stores `0` (off) or `1` (on) for each pixel.
   - Sprites are 8 pixels wide and `1..15` rows tall, drawn using XOR (`^= 1`). If any pixel flips from `1` to `0`, `V[0xF]` is set to `1`.
4. **Keypad (`16` keys, `0x0`–`0xF`)**:
   ```text
   CHIP-8 Keypad        Your Keyboard
   1  2  3  C           1  2  3  4
   4  5  6  D    ==>    Q  W  E  R
   7  8  9  E           A  S  D  F
   A  0  B  F           Z  X  C  V
   ```

---

## Your 4 Guided Milestones in `src/chip8.c`

Open [`src/chip8.c`](src/chip8.c) and look inside `chip8_step(Chip8 *cpu)`.
Every instruction already has its operands extracted for you:
- `nnn`: 12-bit address (`opcode & 0x0FFF`)
- `nn`: 8-bit constant (`opcode & 0x00FF`)
- `n`: 4-bit constant (`opcode & 0x000F`)
- `x`: 4-bit register index `Vx` (`(opcode >> 8) & 0x0F`)
- `y`: 4-bit register index `Vy` (`(opcode >> 4) & 0x0F`)

### Milestone 1: Draw the IBM Logo (Only 6 Opcodes!)
Implement these 6 opcodes first:
1. **`00E0` (`CLS`)**: Clear `cpu->display` with `memset` and set `cpu->draw_flag = true`.
2. **`1NNN` (`JP addr`)**: Set `cpu->pc = nnn`.
3. **`6XNN` (`LD Vx, byte`)**: Set `cpu->V[x] = nn`.
4. **`7XNN` (`ADD Vx, byte`)**: Add `nn` to `cpu->V[x]`.
5. **`ANNN` (`LD I, addr`)**: Set `cpu->I = nnn`.
6. **`DXYN` (`DRW Vx, Vy, nibble`)**: Draw an `n`-row sprite from `cpu->memory[cpu->I]` at `(V[x], V[y])`.

Run `make test` to verify all 6 pass, then run `./chip8.exe` to see the IBM logo on your screen!

### Milestone 2: Flow Control (`00EE`, `2NNN`, `3XNN`, `4XNN`, `5XY0`, `9XY0`, `BNNN`, `CXNN`)
Subroutines (`CALL`/`RET`), conditional skips (`SE`/`SNE`), and random numbers.

### Milestone 3: ALU Operations (`8XY0` – `8XYE`)
Bitwise `OR`, `AND`, `XOR`, addition with carry (`VF`), subtraction with borrow (`VF`), and bit shifts.

### Milestone 4: Timers, Keypad & Memory (`EX9E`, `EXA1`, `FX07` – `FX65`)
Keyboard input checks, delay/sound timers, BCD conversion (`FX33`), and register load/store (`FX55`/`FX65`). Once complete, your emulator can play Pong, Tetris, Breakout, and Space Invaders!
