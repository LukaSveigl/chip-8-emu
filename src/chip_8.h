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

void chip_8_init(chip_8 *emu);

bool chip_8_load(chip_8 *emu, const char *path);

void chip_8_emulate_cycle(chip_8 *emu);

#endif // CHIP_8_H
