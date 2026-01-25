#ifndef CHIP_8_H
#define CHIP_8_H

#include <stdbool.h>
#include <stdint.h>

#define MEMORY_SIZE   4096
#define REGISTERS     16
#define STACK_SIZE    64
#define FB_SIZE       64 * 32
#define KEYMAP_SIZE   16
#define FONTSET_SIZE  80
#define MAX_FILE_SIZE MEMORY_SIZE - 512

/**
 * The CHIP-8 hardware structure.
 *
 * This structure contains all the necessary elements to emulate
 * the architecture of the systems on which CHIP-8 can run.
 */
typedef struct chip_8 {
    uint8_t _memory[MEMORY_SIZE];
    uint8_t _V[REGISTERS];

    uint16_t _I;
    uint16_t _pc;
    uint16_t _opcode;
    
    uint16_t _stack[STACK_SIZE];
    uint16_t _sp;
    
    uint8_t _sound_timer;
    uint8_t _delay_timer;

    uint8_t _framebuffer[FB_SIZE];
    uint8_t _keymap[KEYMAP_SIZE];
} chip_8;

/**
 * Initializes the CHIP-8 structure by setting all of the memory fields to
 * zero, initializing the fontset and setting the program counter to 0x200.
 *
 * @param emu The emulator structure.
 */
void chip_8_init(chip_8 *emu);

/**
 * Loads the ROM at the given path into the memory of the emulator.
 *
 * @param emu  The emulator structure.
 * @param path The path to the ROM.
 * @return True if the ROM is loaded successfully, False otherwise.
 */
bool chip_8_load(chip_8 *emu, const char *path);

/**
 * Emulates one cycle of the program, processing a single opcode.
 *
 * @param emu The emulator structure.
 */
bool chip_8_emulate_cycle(chip_8 *emu);


// Instructions.

/**
 * 0x00E0 - Clear the display.
 *
 * @param emu The emulator structure.
 */
void _chip_8_cls(chip_8 *emu);

/**
 * 0x00EE - Returns from the subroutine.
 *
 * @param emu The emulator structure.
 */
void _chip_8_ret(chip_8 *emu);

/**
 * 0x1nnn - Jump to the location nnn.
 *
 * @param emu The emulator structure.
 */
void _chip_8_jp(chip_8 *emu);

/**
 * 0x2nnn - Call subroutine at nnn.
 *
 * @param emu The emulator structure.
 */
void _chip_8_call(chip_8 *emu);

/**
 * 0x3xkk - Skip next instruction if Vx == kk.
 *
 * @param emu The emulator structure.
 */
void _chip_8_se_byte(chip_8 *emu);

/**
 * 0x4xkk - Skip next instruction if Vx != kk.
 *
 * @param emu The emulator structure.
 */
void _chip_8_sne_byte(chip_8 *emu);

/**
 * 0x5xy0 - Skip next instruction if Vx = Vy.
 *
 * @param emu The emulator structure.
 */
void _chip_8_se_reg(chip_8 *emu);

/**
 * 0x6xkk - Set Vx = kk.
 *
 * @param emu The emulator structure.
 */
void _chip_8_ld_byte(chip_8 *emu);

/**
 * 0x7xkk - Set Vx = Vx + kk.
 *
 * @param emu The emulator structure.
 */
void _chip_8_add_byte(chip_8 *emu);

/**
 * 0x8xy0 - Set Vx = Vy.
 *
 * @param emu The emulator structure.
 */
void _chip_8_ld_reg(chip_8 *emu);

/**
 * 0x8xy1 - Set Vx = Vx OR Vy.
 *
 * @param emu The emulator structure.
 */
void _chip_8_or_reg(chip_8 *emu);

/**
 * 0x8xy2 - Set Vx = Vx AND Vy.
 *
 * @param emu The emulator structure.
 */
void _chip_8_and_reg(chip_8 *emu);

/**
 * 0x8xy3 - Set Vx = Vx XOR Vy.
 *
 * @param emu The emulator structure.
 */
void _chip_8_xor_reg(chip_8 *emu);

/**
 * 0x8xy4 - Set Vx = Vx + Vy, set VF = carry.
 *
 * @param emu The emulator structure.
 */
void _chip_8_add_reg(chip_8 *emu);

/**
 * 0x8xy5 - Set Vx = Vx - Vy, set VF = NOT borrow.
 *
 * @param emu The emulator structure.
 */
void _chip_8_sub_reg(chip_8 *emu);

/**
 * 0x8xy6 - Set Vx = Vx SHR 1. If least-significant bit of Vx is 1, then set VF to 1, otherwise 0.
 *
 * @param emu The emulator structure.
 */
void _chip_8_shr(chip_8 *emu);

/**
 * 0x8xy7 - Set Vx = Vy - Vx, set VF = NOT borrow.
 *
 * @param emu The emulator structure.
 */
void _chip_8_subn_reg(chip_8 *emu);

/**
 * 0x8xyE - Set Vx = Vx SHL 1. If the most-significant bit of Vx is 1, then set VF to 1, otherwise 0.
 *
 * @param emu The emulator structure.
 */
void _chip_8_shl(chip_8 *emu);

/**
 * 0x9xy0 - Skip next instruction if Vx != Vy.
 *
 * @param emu The emulator structure.
 */
void _chip_8_sne_reg(chip_8 *emu);

/**
 * 0xAnnn - Set I = nnn.
 *
 * @param emu The emulator structure.
 */
void _chip_8_ld_addr(chip_8 *emu);

/**
 * 0xBnnn - Jump to location nnn + V0.
 *
 * @param emu The emulator structure.
 */
void _chip_8_jp_rel(chip_8 *emu);

/**
 * 0xCxkk - Set Vx = random byte AND kk.
 *
 * @param emu The emulator structure.
 */
void _chip_8_rnd(chip_8 *emu);

/**
 * 0xDxyn - Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
 *
 * @param emu The emulator structure.
 */
void _chip_8_drw(chip_8 *emu);

/**
 * 0xEx9E - Skip next instruction if key with the value of Vx is pressed.
 *
 * @param emu The emulator structure.
 */
void _chip_8_skp(chip_8 *emu);

/**
 * 0xExA1 - Skip next instruction if key with the value of Vx is not pressed.
 *
 * @param emu The emulator structure.
 */
void _chip_8_sknp(chip_8 *emu);

/**
 * 0xFx07 - Set Vx = delay timer.
 *
 * @param emu The emulator structure.
 */
void _chip_8_ld_dt(chip_8 *emu);

/**
 * 0xFx0A - Wait for a key press, store the vlaue of the key in Vx.
 *
 * @param emu The emulator structure.
 */
bool _chip_8_ld_k(chip_8 *emu);

/**
 * 0xFx15 - Set delay timer = Vx.
 *
 * @param emu The emulator structure.
 */
void _chip_8_ld_dt_reg(chip_8 *emu);

/**
 * 0xFx18 - Set sound timer = Vx.
 *
 * @param emu The emulator structure.
 */
void _chip_8_ld_st_reg(chip_8 *emu);

/**
 * 0xFx1E - Set I = I + Vx.
 *
 * @param emu The emulator structure.
 */
void _chip_8_add_i_reg(chip_8 *emu);

/**
 * 0xFx29 - Set I = location of sprite for digit Vx.
 *
 * @param emu The emulator structure.
 */
void _chip_8_ld_f_reg(chip_8 *emu);

/**
 * 0xFx33 - Store BCD representation of Vx in memory locations I, I + 1, I + 2.
 *
 * @param emu The emulator structure.
 */
void _chip_8_ld_b_reg(chip_8 *emu);

/**
 * 0xFx55 - Store V0 to Vx in memory starting at address I.
 *
 * @param emu The emulator structure.
 */
void _chip_8_ld_i_reg(chip_8 *emu);

/**
 * 0xFx65 - Fills V0 to Vx with values from memory starting at address x.
 *
 * @param emu The emulator structure.
 */
void _chip_8_ld_reg_i(chip_8 *emu);

#endif // CHIP_8_H
