#include <stdio.h>

#include "raylib.h"

#include "chip_8.h"

int main(void) {
    chip_8 emu;
    chip_8_init(&emu);

    chip_8_load(&emu, "prg/test_opcode.ch8");

    InitWindow(800, 600, "Hello, World");

    while (!WindowShouldClose()) {
	BeginDrawing();
	ClearBackground(RAYWHITE);
	EndDrawing();
    }
    
    CloseWindow();
    
    printf("Hello, World!\n");
    return 0;
}
