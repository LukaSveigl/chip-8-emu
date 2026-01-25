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

#endif // CHIP_8_H
