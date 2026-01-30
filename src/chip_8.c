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
        fprintf(stderr,
            "Rom exceeds memory size. Max size: %d, ROM size: %d\n",
            MAX_FILE_SIZE,
            (int)file_size);
        fclose(file);
        return false;
    }

    size_t read = fread(emu->_memory + 512, sizeof(int8_t), file_size, file);
    if (read != file_size) {
        fprintf(stderr,
            "Failed to read full ROM: Expected: %d, Read: %d\n",
            (int)file_size,
            (int)read);
        fclose(file);
        return false;
    }

    fclose(file);
    return true;
}

bool chip_8_emulate_cycle(chip_8 *emu) {
    emu->_opcode = emu->_memory[emu->_pc] << 8 | emu->_memory[emu->_pc + 1];

    bool draw = false;

    switch (emu->_opcode & 0xF000) {
    case 0x0000:
        if (emu->_opcode == 0x00E0) {
            _chip_8_cls(emu);
            draw = true;
        } else if (emu->_opcode == 0x00EE) {
            _chip_8_ret(emu);
        } else {
            fprintf(stderr, "Unknown instruction: %x\n", emu->_opcode);
            exit(1);
        }
        break;
    case 0x1000:
        _chip_8_jp(emu);
        break;
    case 0x2000:
        _chip_8_call(emu);
        break;
    case 0x3000: {
        _chip_8_se_byte(emu);
        break;
    }
    case 0x4000: {
        _chip_8_sne_byte(emu);
        break;
    }
    case 0x5000: {
        _chip_8_se_reg(emu);
        break;
    }
    case 0x6000: {
        _chip_8_ld_byte(emu);
        break;
    }
    case 0x7000: {
        _chip_8_add_byte(emu);
        break;
    }
    case 0x8000: {
        switch (emu->_opcode & 0x000F) {
            case 0x0000: {
                _chip_8_ld_reg(emu);
                break;
            }
            case 0x0001: {
                _chip_8_or_reg(emu);
                break;
            }
            case 0x0002: {
                _chip_8_and_reg(emu);
                break;
            }
            case 0x0003: {
                _chip_8_xor_reg(emu);
                break;
            }
            case 0x0004: {
                _chip_8_add_reg(emu);
                break;
            }
            case 0x0005: {
                _chip_8_sub_reg(emu);
                break;
            }
            case 0x0006: {
                _chip_8_shr(emu);
                break;
            }
            case 0x0007: {
                _chip_8_subn_reg(emu);
                break;
            }
            case 0x000E: {
                _chip_8_shl(emu);
                break;
            }
            default: {
                fprintf(stderr, "Unknown instruction: %x\n", emu->_opcode);
                exit(1);
            }
        }
        break;
    }
    case 0x9000: {
        _chip_8_sne_reg(emu);
        break;
    }
    case 0xA000: {
        _chip_8_ld_addr(emu);
        break;
    }
    case 0xB000: {
        _chip_8_jp_rel(emu);
        break;
    }
    case 0xC000: {
        _chip_8_rnd(emu);
        break;
    }
    case 0xD000: {
        _chip_8_drw(emu);
        draw = true;
        break;
    }
    case 0xE000: {
        if ((emu->_opcode & 0x00FF) == 0x009E) {
            _chip_8_skp(emu);
        } else if ((emu->_opcode & 0x00FF) == 0x00A1) {
            _chip_8_sknp(emu);
        } else {
            fprintf(stderr, "Unknown instruction: %x\n", emu->_opcode);
            exit(1);
        }
        break;
    }
    case 0xF000: {
        switch (emu->_opcode & 0x00FF) {
            case 0x0007: {
                _chip_8_ld_dt(emu);
                break;
            }
            case 0x000A: {
                bool pressed = _chip_8_ld_k(emu);

                if (!pressed) {
                    return draw;
                }

                emu->_pc += 2;
                break;
            }
            case 0x0015: {
                _chip_8_ld_dt_reg(emu);
                break;
            }
            case 0x0018: {
                _chip_8_ld_st_reg(emu);
                break;
            }
            case 0x001E: {
                _chip_8_add_i_reg(emu);
                break;
            }
            case 0x0029: {
                _chip_8_ld_f_reg(emu);
                break;
            }
            case 0x0033: {
                _chip_8_ld_b_reg(emu);
                break;
            }
            case 0x0055: {
                _chip_8_ld_i_reg(emu);
                break;
            }
            case 0x0065: {
                _chip_8_ld_reg_i(emu);
                break;
            }
            default: {
                fprintf(stderr, "Unknown instruction: %x\n", emu->_opcode);
                exit(1);
            }
        }
        break;
    }
    default: {
        fprintf(stderr, "Unknown instruction: %x\n", emu->_opcode);
        exit(1);
    }
    }

    if (emu->_delay_timer) {
        emu->_delay_timer--;
    }

    if (emu->_sound_timer > 0) {
        emu->_sound_timer--;
    }

    return draw;
}

void _chip_8_cls(chip_8 *emu) {
    memset(emu->_framebuffer, 0, FB_SIZE);
    emu->_pc += 2;
}

void _chip_8_ret(chip_8 *emu) {
    emu->_sp--;
    emu->_pc = emu->_stack[emu->_sp];
    emu->_pc += 2;    
}

void _chip_8_jp(chip_8 *emu) { emu->_pc = emu->_opcode & 0x0FFF; }

void _chip_8_call(chip_8 *emu) {
    emu->_stack[emu->_sp] = emu->_pc;
    emu->_sp++;
    emu->_pc = emu->_opcode & 0x0FFF;
}

void _chip_8_se_byte(chip_8 *emu) {
    uint16_t x = (emu->_opcode & 0x0F00) >> 8;
    uint16_t kk = (emu->_opcode & 0x00FF);
    if (emu->_V[x] == kk) {
	emu->_pc += 4;
    } else {
	emu->_pc += 2;
    }
}

void _chip_8_sne_byte(chip_8 *emu) {
    uint16_t x = (emu->_opcode & 0x0F00) >> 8;
    uint16_t kk = (emu->_opcode & 0x00FF);
    if (emu->_V[x] != kk) {
	emu->_pc += 4;
    } else {
	emu->_pc += 2;
    }    
}

void _chip_8_se_reg(chip_8 *emu) {
    uint16_t x = (emu->_opcode & 0x0F00) >> 8;
    uint16_t y = (emu->_opcode & 0x00F0) >> 4;
    if (emu->_V[x] == emu->_V[y]) {
	emu->_pc += 4;
    } else {
	emu->_pc += 2;
    }
}

void _chip_8_ld_byte(chip_8 *emu) {
    uint16_t x = (emu->_opcode & 0x0F00) >> 8;
    uint16_t kk = (emu->_opcode & 0x00FF);
    emu->_V[x] = kk;
    emu->_pc += 2;
}

void _chip_8_add_byte(chip_8 *emu) {
    uint16_t x = (emu->_opcode & 0x0F00) >> 8;
    uint16_t kk = (emu->_opcode & 0x00FF);
    emu->_V[x] = emu->_V[x] + kk;
    emu->_pc += 2;
}

void _chip_8_ld_reg(chip_8 *emu) {
    uint16_t x = (emu->_opcode & 0x0F00) >> 8;
    uint16_t y = (emu->_opcode & 0x00F0) >> 4;        
    emu->_V[x] = emu->_V[y];
    emu->_pc += 2;
}

void _chip_8_or_reg(chip_8 *emu) {
    uint16_t x = (emu->_opcode & 0x0F00) >> 8;
    uint16_t y = (emu->_opcode & 0x00F0) >> 4;            
    emu->_V[x] = emu->_V[x] | emu->_V[y];
    emu->_pc += 2;
}

void _chip_8_and_reg(chip_8 *emu) {
    uint16_t x = (emu->_opcode & 0x0F00) >> 8;
    uint16_t y = (emu->_opcode & 0x00F0) >> 4;            
    emu->_V[x] = emu->_V[x] & emu->_V[y];
    emu->_pc += 2;
}

void _chip_8_xor_reg(chip_8 *emu) {
    uint16_t x = (emu->_opcode & 0x0F00) >> 8;
    uint16_t y = (emu->_opcode & 0x00F0) >> 4;            
    emu->_V[x] = emu->_V[x] ^ emu->_V[y];
    emu->_pc += 2;    
}

void _chip_8_add_reg(chip_8 *emu) {
    uint16_t x = (emu->_opcode & 0x0F00) >> 8;
    uint16_t y = (emu->_opcode & 0x00F0) >> 4;            
    if (emu->_V[x] + emu->_V[y] > 255) {
	emu->_V[0xF] = 1;
    } else {
	emu->_V[0xF] = 0;
    }
    emu->_V[x] = emu->_V[x] + emu->_V[y];
    emu->_pc += 2;
}

void _chip_8_sub_reg(chip_8 *emu) {
    uint16_t x = (emu->_opcode & 0x0F00) >> 8;
    uint16_t y = (emu->_opcode & 0x00F0) >> 4;            
    if (emu->_V[x] < emu->_V[y]) {
	emu->_V[0xF] = 1;
    } else {
	emu->_V[0xF] = 0;
    }
    emu->_V[x] = emu->_V[x] - emu->_V[y];
    emu->_pc += 2;    
}

void _chip_8_shr(chip_8 *emu) {
    uint16_t x = (emu->_opcode & 0x0F00) >> 8;         
    emu->_V[0xF] = emu->_V[x] & 0x1;
    emu->_V[x] = emu->_V[x] >> 1;
    emu->_pc += 2;    
}

void _chip_8_subn_reg(chip_8 *emu) {
    uint16_t x = (emu->_opcode & 0x0F00) >> 8;
    uint16_t y = (emu->_opcode & 0x00F0) >> 4;            
    if (emu->_V[x] > emu->_V[y]) {
	emu->_V[0xF] = 0;
    } else {
	emu->_V[0xF] = 1;
    }
    emu->_V[x] = emu->_V[y] - emu->_V[x];
    emu->_pc += 2;    
}

void _chip_8_shl(chip_8 *emu) {
    uint16_t x = (emu->_opcode & 0x0F00) >> 8;     
    emu->_V[0xF] = emu->_V[x] >> 7;
    emu->_V[x] = emu->_V[x] << 1;
    emu->_pc += 2;
}

void _chip_8_sne_reg(chip_8 *emu) {
    uint16_t x = (emu->_opcode & 0x0F00) >> 8;
    uint16_t y = (emu->_opcode & 0x00F0) >> 4;
    if (emu->_V[x] != emu->_V[y]) {
	emu->_pc += 4;
    } else {
	emu->_pc += 2;
    }    
}

void _chip_8_ld_addr(chip_8 *emu) {
    emu->_I = emu->_opcode & 0x0FFF;
    emu->_pc += 2;    
}

void _chip_8_jp_rel(chip_8 *emu) {
    emu->_pc = emu->_V[0x0] + (emu->_opcode & 0x0FFF);
}

void _chip_8_rnd(chip_8 *emu) {
    uint16_t x = (emu->_opcode & 0x0F00) >> 8;
    uint16_t kk = emu->_opcode & 0x00FF;
    emu->_V[x] = (rand() % (255 + 1)) & kk;
    emu->_pc += 2;    
}

void _chip_8_drw(chip_8 *emu) {
    uint16_t x = (emu->_opcode & 0x0F00) >> 8;
    uint16_t y = (emu->_opcode & 0x00F0) >> 4;
    uint16_t n = (emu->_opcode & 0x000F);

    uint16_t v_x = emu->_V[x];
    uint16_t v_y = emu->_V[y];

    emu->_V[0xF] = 0;
    for (size_t row = 0; row < n; row++) {
	uint16_t pixel = emu->_memory[emu->_I + row];
	for (size_t col = 0; col < 8; col++) {
	    if ((pixel & (0x80 >> col)) != 0) {
		size_t index = (v_x + col + ((v_y + row) * 64));
		if (emu->_framebuffer[index] == 1) {
		    emu->_V[0xF] = 1;
		}
		emu->_framebuffer[index] ^= 1;
	    }
	}
    }
    emu->_pc += 2;    
}

void _chip_8_skp(chip_8 *emu) {
    uint16_t key_index = emu->_V[(emu->_opcode & 0x0F00) >> 8];
    if (emu->_keymap[key_index] != 0) {
	emu->_pc += 4;
    } else {
	emu->_pc += 2;
    }    
}

void _chip_8_sknp(chip_8 *emu) {
    uint16_t key_index = emu->_V[(emu->_opcode & 0x0F00) >> 8];
    if (emu->_keymap[key_index] == 0) {
	emu->_pc += 4;
    } else {
	emu->_pc += 2;
    }    
}

void _chip_8_ld_dt(chip_8 *emu) {
    uint16_t x = (emu->_opcode & 0x0F00) >> 8;
    emu->_V[x] = emu->_delay_timer;
    emu->_pc += 2;
}

bool _chip_8_ld_k(chip_8 *emu) {
    bool pressed = false;
    uint16_t x = (emu->_opcode & 0x0F00) >> 8;
    for (size_t i = 0; i < KEYMAP_SIZE; i++) {
	if (emu->_keymap[i] != 0) {
	    emu->_V[x] = i;
	    pressed = true;
	}
    }
    return pressed;
}

void _chip_8_ld_dt_reg(chip_8 *emu) {
    uint16_t x = (emu->_opcode & 0x0F00) >> 8;
    emu->_delay_timer = emu->_V[x];
    emu->_pc += 2;
}

void _chip_8_ld_st_reg(chip_8 *emu) {
    uint16_t x = (emu->_opcode & 0x0F00) >> 8;
    emu->_sound_timer = emu->_V[x];
    emu->_pc += 2;
}

void _chip_8_add_i_reg(chip_8 *emu) {
    uint16_t x = (emu->_opcode & 0x0F00) >> 8;
    if (emu->_I + emu->_V[x] > 0xFFF) {
	emu->_V[0xF] = 1;
    } else {
	emu->_V[0xF] = 0;
    }
    emu->_I += emu->_V[x];
    emu->_pc += 2;    
}

void _chip_8_ld_f_reg(chip_8 *emu) {
    uint16_t x = (emu->_opcode & 0x0F00) >> 8;
    emu->_I = emu->_V[x] * 0x5;
    emu->_pc += 2;    
}

void _chip_8_ld_b_reg(chip_8 *emu) {
    uint16_t x = (emu->_opcode & 0x0F00) >> 8;
    emu->_memory[emu->_I] = emu->_V[x] / 100;
    emu->_memory[emu->_I + 1] = (emu->_V[x] / 10) % 10;
    emu->_memory[emu->_I + 2] = emu->_V[x] % 10;
    emu->_pc += 2;
}

void _chip_8_ld_i_reg(chip_8 *emu) {
    uint16_t x = (emu->_opcode & 0x0F00) >> 8;
    for (size_t i = 0; i <= x; ++i) {
	emu->_memory[emu->_I + i] = emu->_V[i];
    }

    emu->_I += x + 1;
    emu->_pc += 2;    
}

void _chip_8_ld_reg_i(chip_8 *emu) {
    uint16_t x = (emu->_opcode & 0x0F00) >> 8;
    for (size_t i = 0; i <= x; ++i) {
	emu->_V[i] = emu->_memory[emu->_I + i];
    }

    emu->_I += x + 1;
    emu->_pc += 2;
}

