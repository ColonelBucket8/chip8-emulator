#include "chip8.h"
#include <stdio.h>
#include <string.h>

static int g_base_passed = 0;
static int g_base_failed = 0;
static int g_todo_passed = 0;
static int g_todo_pending = 0;

static const uint8_t IBM_LOGO_BYTES[132] = {
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

static void ensure_ibm_rom_exists(void) {
    FILE *fp = fopen("roms/ibm_logo.ch8", "rb");
    if (fp) {
        fclose(fp);
        return;
    }
    fp = fopen("roms/ibm_logo.ch8", "wb");
    if (fp) {
        fwrite(IBM_LOGO_BYTES, 1, sizeof(IBM_LOGO_BYTES), fp);
        fclose(fp);
    }
}

static void put_opcode(Chip8 *cpu, uint16_t addr, uint16_t opcode) {
    cpu->memory[addr]     = (uint8_t)(opcode >> 8);
    cpu->memory[addr + 1] = (uint8_t)(opcode & 0xFF);
}

static void check_base(const char *name, bool cond) {
    if (cond) {
        printf("  [PASS] %s\n", name);
        g_base_passed++;
    } else {
        printf("  [FAIL] %s\n", name);
        g_base_failed++;
    }
}

static void check_milestone(const char *opcode_name, bool cond, const char *hint) {
    if (cond) {
        printf("  [PASS] %-28s\n", opcode_name);
        g_todo_passed++;
    } else {
        printf("  [TODO] %-28s -> Hint: %s\n", opcode_name, hint);
        g_todo_pending++;
    }
}

static void test_foundation(void) {
    printf("=== Foundation Checks (Pre-built) ===\n");
    ensure_ibm_rom_exists();

    Chip8 cpu;
    memset(&cpu, 0xAA, sizeof(cpu));
    chip8_init(&cpu);

    check_base("chip8_init sets PC to 0x200", cpu.pc == CHIP8_ROM_START);
    check_base("chip8_init loads fontset at 0x050", cpu.memory[CHIP8_FONT_START] == 0xF0 && cpu.memory[CHIP8_FONT_START + 4] == 0xF0);
    check_base("chip8_init clears registers and timers", cpu.I == 0 && cpu.sp == 0 && cpu.delay_timer == 0 && cpu.sound_timer == 0);

    cpu.delay_timer = 2;
    cpu.sound_timer = 1;
    chip8_update_timers(&cpu);
    check_base("chip8_update_timers decrements active timers", cpu.delay_timer == 1 && cpu.sound_timer == 0);

    check_base("chip8_load_rom rejects missing file", !chip8_load_rom(&cpu, "nonexistent_rom.ch8"));
    check_base("chip8_load_rom loads roms/ibm_logo.ch8", chip8_load_rom(&cpu, "roms/ibm_logo.ch8") && cpu.memory[0x200] == 0x00 && cpu.memory[0x201] == 0xE0);

    /* Create temporary oversized ROM (4000 bytes > 3584 max) and verify rejection */
    {
        FILE *big = fopen("roms/oversize_tmp.bin", "wb");
        if (big) {
            uint8_t buf[4000];
            memset(buf, 0, sizeof(buf));
            fwrite(buf, 1, sizeof(buf), big);
            fclose(big);
            check_base("chip8_load_rom rejects oversized ROM (>3584 bytes)", !chip8_load_rom(&cpu, "roms/oversize_tmp.bin"));
            remove("roms/oversize_tmp.bin");
        }
    }
}

static void test_milestone_1_ibm_logo(void) {
    printf("\n=== Milestone 1: IBM Logo Opcodes (6 Opcodes) ===\n");
    Chip8 cpu;

    /* 00E0: CLS */
    chip8_init(&cpu);
    cpu.display[10] = 1;
    cpu.draw_flag = false;
    put_opcode(&cpu, 0x200, 0x00E0);
    chip8_step(&cpu);
    check_milestone("00E0 (CLS - Clear Screen)",
        cpu.pc == 0x202 && cpu.display[10] == 0 && cpu.draw_flag,
        "memset(cpu->display, 0, sizeof(cpu->display)); cpu->draw_flag = true;");

    /* 1NNN: JP addr */
    chip8_init(&cpu);
    put_opcode(&cpu, 0x200, 0x1345);
    chip8_step(&cpu);
    check_milestone("1NNN (JP - Jump to NNN)",
        cpu.pc == 0x0345,
        "cpu->pc = nnn;");

    /* 6XNN: LD Vx, byte */
    chip8_init(&cpu);
    put_opcode(&cpu, 0x200, 0x63AB);
    chip8_step(&cpu);
    check_milestone("6XNN (LD Vx, NN)",
        cpu.pc == 0x202 && cpu.V[3] == 0xAB,
        "cpu->V[x] = nn;");

    /* 7XNN: ADD Vx, byte */
    chip8_init(&cpu);
    cpu.V[4] = 0x10;
    put_opcode(&cpu, 0x200, 0x7425);
    chip8_step(&cpu);
    check_milestone("7XNN (ADD Vx, NN)",
        cpu.pc == 0x202 && cpu.V[4] == 0x35,
        "cpu->V[x] += nn;");

    /* ANNN: LD I, addr */
    chip8_init(&cpu);
    put_opcode(&cpu, 0x200, 0xA2F0);
    chip8_step(&cpu);
    check_milestone("ANNN (LD I, NNN)",
        cpu.pc == 0x202 && cpu.I == 0x02F0,
        "cpu->I = nnn;");

    /* DXYN: DRW Vx, Vy, nibble */
    chip8_init(&cpu);
    cpu.I = 0x300;
    cpu.memory[0x300] = 0x80; /* Top-left pixel bit (bit 7) set */
    cpu.V[0] = 2;
    cpu.V[1] = 3;
    put_opcode(&cpu, 0x200, 0xD011);
    chip8_step(&cpu);
    bool first_draw = (cpu.display[3 * CHIP8_WIDTH + 2] == 1) && (cpu.V[0xF] == 0) && cpu.draw_flag;
    /* Draw again at same spot to test XOR collision */
    put_opcode(&cpu, 0x202, 0xD011);
    chip8_step(&cpu);
    bool second_draw = (cpu.display[3 * CHIP8_WIDTH + 2] == 0) && (cpu.V[0xF] == 1);
    check_milestone("DXYN (DRW Vx, Vy, N)",
        first_draw && second_draw,
        "XOR sprite bits from memory[I..I+N-1] onto display; set VF=1 if any pixel flips 1->0");
}

static void test_milestone_2_flow_control(void) {
    printf("\n=== Milestone 2: Subroutines & Branches ===\n");
    Chip8 cpu;

    /* 2NNN + 00EE: CALL & RET */
    chip8_init(&cpu);
    put_opcode(&cpu, 0x200, 0x2300);
    put_opcode(&cpu, 0x300, 0x00EE);
    chip8_step(&cpu);
    bool called = (cpu.pc == 0x300 && cpu.sp == 1 && cpu.stack[0] == 0x202);
    chip8_step(&cpu);
    bool returned = (cpu.pc == 0x202 && cpu.sp == 0);
    check_milestone("2NNN & 00EE (CALL & RET)",
        called && returned,
        "2NNN pushes PC to stack[sp++] and jumps to NNN; 00EE pops PC = stack[--sp]");

    /* 3XNN: SE Vx, NN */
    chip8_init(&cpu);
    cpu.V[2] = 0x42;
    put_opcode(&cpu, 0x200, 0x3242);
    chip8_step(&cpu);
    check_milestone("3XNN (SE Vx, NN)",
        cpu.pc == 0x204,
        "if (cpu->V[x] == nn) cpu->pc += 2;");

    /* 4XNN: SNE Vx, NN */
    chip8_init(&cpu);
    cpu.V[2] = 0x42;
    put_opcode(&cpu, 0x200, 0x4299);
    chip8_step(&cpu);
    check_milestone("4XNN (SNE Vx, NN)",
        cpu.pc == 0x204,
        "if (cpu->V[x] != nn) cpu->pc += 2;");
}

static void test_milestone_3_alu(void) {
    printf("\n=== Milestone 3: ALU Register Ops (8XY0 - 8XYE) ===\n");
    Chip8 cpu;

    /* 8XY4: ADD Vx, Vy with carry */
    chip8_init(&cpu);
    cpu.V[0] = 200;
    cpu.V[1] = 100;
    put_opcode(&cpu, 0x200, 0x8014);
    chip8_step(&cpu);
    check_milestone("8XY4 (ADD Vx, Vy w/ carry)",
        cpu.V[0] == 44 && cpu.V[0xF] == 1,
        "uint16_t sum = V[x] + V[y]; V[x] = (uint8_t)sum; V[0xF] = (sum > 255);");

    /* 8XY5: SUB Vx, Vy with borrow */
    chip8_init(&cpu);
    cpu.V[0] = 10;
    cpu.V[1] = 4;
    put_opcode(&cpu, 0x200, 0x8015);
    chip8_step(&cpu);
    check_milestone("8XY5 (SUB Vx, Vy w/ NOT borrow)",
        cpu.V[0] == 6 && cpu.V[0xF] == 1,
        "uint8_t not_borrow = (V[x] >= V[y]); V[x] -= V[y]; V[0xF] = not_borrow;");
}

static void test_milestone_4_timers_mem(void) {
    printf("\n=== Milestone 4: Keypad, Timers & Memory (EXxx / FXxx) ===\n");
    Chip8 cpu;

    /* FX33: BCD */
    chip8_init(&cpu);
    cpu.I = 0x300;
    cpu.V[2] = 234;
    put_opcode(&cpu, 0x200, 0xF233);
    chip8_step(&cpu);
    check_milestone("FX33 (LD B, Vx - BCD)",
        cpu.memory[0x300] == 2 && cpu.memory[0x301] == 3 && cpu.memory[0x302] == 4,
        "memory[I] = V[x]/100; memory[I+1] = (V[x]/10)%10; memory[I+2] = V[x]%10;");
}

int main(void) {
    test_foundation();
    test_milestone_1_ibm_logo();
    test_milestone_2_flow_control();
    test_milestone_3_alu();
    test_milestone_4_timers_mem();

    printf("\n--------------------------------------------------\n");
    printf("Foundation: %d passed, %d failed\n", g_base_passed, g_base_failed);
    printf("Your Opcodes: %d implemented, %d remaining TODOs\n", g_todo_passed, g_todo_pending);
    printf("--------------------------------------------------\n");

    return (g_base_failed == 0) ? 0 : 1;
}
