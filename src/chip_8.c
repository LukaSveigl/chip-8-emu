#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "chip_8.h"

uint8_t chip_8_fontset[FONTSET_SIZE] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void chip_8_init(chip_8 *emu) {
    emu->_I = 0;
    emu->_sp = 0;
    emu->_opcode = 0;

    emu->_pc = 0x200;

    memset(emu->_memory, 0, MEMORY_SIZE);
    memset(emu->_V, 0, REGISTERS);
    memset(emu->_stack, 0, STACK_SIZE);
    memset(emu->_framebuffer, 0, FB_SIZE);
    memset(emu->_keymap, 0, KEYMAP_SIZE);

    emu->_sound_timer = 0;
    emu->_delay_timer = 0;

    for (size_t i = 0; i < FONTSET_SIZE; i++) {
	emu->_memory[i] = chip_8_fontset[i];
    }
}

bool chip_8_load(chip_8 *emu, const char *path) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
	fprintf(stderr, "Failed to open ROM: %s\n", path);
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    if (file_size > MAX_FILE_SIZE) {
	fprintf(
	    stderr,
	    "Rom exceeds memory size. Max size: %d, ROM size: %d\n",
	    MAX_FILE_SIZE,
	    (int)file_size
	);
	fclose(file);
	return false;
    }

    size_t read = fread(emu->_memory + 512, sizeof(int8_t), file_size, file);
    if (read != file_size) {
	fprintf(
	    stderr,
	    "Failed to read full ROM: Expected: %d, Read: %d\n",
	    (int)file_size,
	    (int)read
	);
	fclose(file);
	return false;
    }

    fclose(file);
    return true;
}

void chip_8_emulate_cycle(chip_8 *emu) {

}
