#include <stdio.h>
#include <unistd.h>

#include "raylib.h"

#include "chip_8.h"

#define TARGET_FPS 250
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

uint8_t keymap[KEYMAP_SIZE] = {
    KEY_X,
    KEY_ONE,
    KEY_TWO,
    KEY_THREE,
    KEY_Q,
    KEY_W,
    KEY_E,
    KEY_A,
    KEY_S,
    KEY_D,
    KEY_Z,
    KEY_C,
    KEY_FOUR,
    KEY_R,
    KEY_F,
    KEY_V
};

int main(int argc, char **argv) {
    chip_8 emu;
    chip_8_init(&emu);

    if (argc != 2) {
        fprintf(stderr, "Invalid arguments. Usage: ./main <path-to-file>\n");
        return 1;
    }

    const char *path = argv[1];

    if (!chip_8_load(&emu, path)) {
        fprintf(stderr, "Failed to load ROM\n");
        return 1;
    }

    bool draw = false;

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "CHIP-8 Emulator");

    // Prepare the texture onto which the emulator will draw.
    Image image = GenImageColor(64, 32, BLACK);
    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);

    Color pixels[FB_SIZE];

    SetTargetFPS(TARGET_FPS);

    while (!WindowShouldClose()) {
        draw = chip_8_emulate_cycle(&emu);

        for (size_t i = 0; i < KEYMAP_SIZE; i++) {
            if (IsKeyDown(keymap[i])) {
                emu._keymap[i] = 1;
            }

            if (IsKeyUp(keymap[i])) {
                emu._keymap[i] = 0;
            }
        }

        BeginDrawing();

        // If a draw has occurred in the emulator, loop through the framebuffer
        // and update the texture.
        if (draw) {
            for (size_t i = 0; i < FB_SIZE; i++) {
                if (emu._framebuffer[i]) {
                    pixels[i] = WHITE;
                } else {
                    pixels[i] = BLACK;
                }
            }

            UpdateTexture(texture, pixels);
        }
        
        ClearBackground(BLACK);

        DrawTexturePro(texture,
            (Rectangle){0, 0, 64, 32},
            (Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()},
            (Vector2){0, 0},
            0.0f,
            WHITE);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
