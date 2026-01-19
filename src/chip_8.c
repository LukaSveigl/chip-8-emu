#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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
    emu->_opcode = emu->_memory[emu->_pc] | emu->_memory[emu->_pc + 1];

    switch (emu->_opcode & 0xF000) {
        case 0x0000:
	    if (emu->_opcode == 0x00E0) {
		// 0x00E0 - Clear the display.
		memset(emu->_framebuffer, 0, FB_SIZE);
		emu->_pc += 2;
	    } else if (emu->_opcode == 0x00EE) {
		// Return from the subroutine.
		emu->_pc = emu->_stack[emu->_sp];
		emu->_sp--;
	    }
	    break;
        case 0x1000:
	    // Jumps to address at NNN.
	    emu->_pc  = emu->_opcode & 0x0FFF;
	    break;
        case 0x2000:
	    // Calls subroutine at NNN.
	    emu->_sp++;
	    emu->_stack[emu->_sp] = emu->_pc;
	    emu->_pc = emu->_opcode & 0x0FFF;
	    break;
        case 0x3000: {
	    // If Vx == kk, skips next instruction.
	    uint16_t x = (emu->_opcode & 0x0F00) >> 8;
	    uint16_t kk = (emu->_opcode & 0x00FF);
	    if (emu->_V[x] == kk) {
		emu->_pc += 4;
	    } else {
		emu->_pc += 2;
	    }
	    break;
        }
        case 0x4000: {
	    // If Vx != kk, skips next instruction.
	    uint16_t x = (emu->_opcode & 0x0F00) >> 8;
	    uint16_t kk = (emu->_opcode & 0x00FF);
	    if (emu->_V[x] != kk) {
		emu->_pc += 4;
	    } else {
		emu->_pc += 2;
	    }
	    break;
        }
        case 0x5000: {
	    // If Vx == Vy, skips the next instruction.
	    uint16_t x = (emu->_opcode & 0x0F00) >> 8;
	    uint16_t y = (emu->_opcode & 0x00F0) >> 4;
	    if (emu->_V[x] == emu->_V[y]) {
		emu->_pc += 4;
	    } else {
		emu->_pc += 2;
	    }
	    break;
        }
        case 0x6000: {
	    // Set Vx = kk.
	    uint16_t x = (emu->_opcode & 0x0F00) >> 8;
	    uint16_t kk = (emu->_opcode & 0x00FF);
	    emu->_V[x] = kk;
	    emu->_pc += 2;
	    break;
	}
        case 0x7000: {
	    // Add kk to Vx.
	    uint16_t x = (emu->_opcode & 0x0F00) >> 8;
	    uint16_t kk = (emu->_opcode & 0x00FF);
	    emu->_V[x] = emu->_V[x] + kk;
	    emu->_pc += 2;
	    break;
	}
        case 0x8000: {
	    uint16_t x = (emu->_opcode & 0x0F00) >> 8;
	    uint16_t y = (emu->_opcode & 0x00F0) >> 4;
	    switch (emu->_opcode & 0x000F) {
	        case 0x0000: {
		    // Set Vx = Vy.
		    emu->_V[x] = emu->_V[y];
		    emu->_pc += 2;
		    break;
		}
	        case 0x0001: {
		    // Set Vx = Vx OR Vy.
		    emu->_V[x] = emu->_V[x] | emu->_V[y];
		    emu->_pc += 2;
		    break;
		}
	        case 0x0002: {
		    // Set Vx = Vx AND Vy.
		    emu->_V[x] = emu->_V[x] & emu->_V[y];
		    emu->_pc += 2;
		    break;
		}
    	        case 0x0003: {
		    // Set Vx = Vx XOR Vy.
		    emu->_V[x] = emu->_V[x] ^ emu->_V[y];
		    emu->_pc += 2;
		    break;
		}
	        case 0x0004: {
		    // Set Vx = Vx + Vy. If result > 8 bits (255),
		    // set the carry bit.
		    if (emu->_V[x] + emu->_V[y] > 255) {
			emu->_V[0xF] = 1;
		    } else {
			emu->_V[0xF] = 0;
		    }
		    emu->_V[x] = emu->_V[x] + emu->_V[y];
		    emu->_pc += 2;
		    break;
		}
	        case 0x0005: {
		    // Set Vx = Vx - Vy. If Vx < Vy, set the borrow
		    // bit.
		    if (emu->_V[x] < emu->_V[y]) {
			emu->_V[0xF] = 1;
		    } else {
			emu->_V[0xF] = 0;
		    }
		    emu->_V[x] = emu->_V[x] - emu->_V[y];
		    emu->_pc += 2;
		    break;
		}
	        case 0x0006: {
		    // Set Vx = Vx SHR 1. If least significant bit
		    // of Vx is 1, set Vf to 1. Vx is divided
		    // by 2.
		    emu->_V[0xF] = emu->_V[x] & 0x1;
		    emu->_V[x] = emu->_V[x] >> 1;
		    emu->_pc += 2;
		    break;
		}
	        case 0x0007: {
		    // Set Vx = Vy - Vx. If Vx > Vy, set the borrow
		    // bit.
		    if (emu->_V[x] > emu->_V[y]) {
			emu->_V[0xF] = 1;
		    } else {
			emu->_V[0xF] = 0;
		    }
		    emu->_V[x] = emu->_V[y] - emu->_V[x];
		    emu->_pc += 2;
		    break;
		}
	        case 0x000E: {
		    // Set Vx = Vx SHL 1. If most significant bit
		    // of Vx is 1, set Vf to 1. Vx is multiplied
		    // by 2.
		    emu->_V[0xF] = emu->_V[x] >> 7;
		    emu->_V[x] = emu->_V[x] << 1;
		    emu->_pc += 2;
		    break;
		}
	        default:
		    // TODO: Add error reporting.
		    break;
	    }
	    break;
	}
        case 0x9000: {
	    // If Vx == Vy, skip next instruction.
	    uint16_t x = (emu->_opcode & 0x0F00) >> 8;
	    uint16_t y = (emu->_opcode & 0x00F0) >> 4;
	    if (emu->_V[x] == emu->_V[y]) {
		emu->_pc += 4;
	    } else {
		emu->_pc += 2;
	    }
	    break;
	}
        case 0xA000: {
	    // Set I = NNN.
	    emu->_I = emu->_opcode & 0x0FFF;
	    break;
	}
        case 0xB000: {
	    // Jump to location NNN + V0.
	    emu->_pc = emu->_V[0x0] + (emu->_opcode & 0x0FFF);
	    break;
	}
        case 0xC000: {
	    // Set Vx = random byte AND kk.
	    uint16_t x = (emu->_opcode & 0x0F00) >> 8;
	    uint16_t kk = emu->_opcode & 0x00FF;
	    emu->_V[x] = (rand() % (255 + 1)) & kk;
	    emu->_pc += 2;
	    break;
	}
        case 0xD000:
	    break;
        case 0xE000:
	    break;
        case 0xF000:
	    break;
        default:
	    break;
    };

    if (emu->_delay_timer) {
	emu->_delay_timer--;
    }
  
    if (emu->_sound_timer > 0) {
	emu->_sound_timer--;
    }
}
