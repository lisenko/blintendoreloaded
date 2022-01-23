#include "raylib.h"
#include "6502.h"

#include <stdlib.h>         // Required for: malloc() and free()
#include <stdio.h>
#include <stdint.h>

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 512;
    const int screenHeight = 480;

    struct cpu cpu;

    cpu.cycles = 0;
    cpu.pc = 0;
    cpu.sp = 0;
    cpu.a = 0;
    cpu.x = 0;
    cpu.y = 0;
    cpu.sr = 0;
    cpu.wram[1] = 69;

    uint8_t rom[] = { 0xa9, 0x80, 0x85, 0x01, 0x65, 0x01, 0x65, 0x01, 0xe9, 0x01 };

    char *status;

    InitWindow(screenWidth, screenHeight, "blintendo");

    SetTargetFPS(60);

    // NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)

    // Generate a checked texture by code
    int width = 960;
    int height = 480;

    // Dynamic memory allocation to store pixels data (Color type)
    Color *pixels = (Color *)malloc(width*height*sizeof(Color));

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if (((x/32+y/32)/1)%2 == 0) pixels[y*width + x] = ORANGE;
            else pixels[y*width + x] = GOLD;
        }
    }

    // Load pixels data into an image structure and create texture
    Image checkedIm = {
        .data = pixels,             // We can assign pixels directly to data
        .width = width,
        .height = height,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        .mipmaps = 1
    };

    Texture2D checked = LoadTextureFromImage(checkedIm);
    UnloadImage(checkedIm);         // Unload CPU (RAM) image data (pixels)
    //---------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if(IsKeyPressed(KEY_S)) {
            status = step(&cpu, rom[cpu.pc], rom);
        }

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);
        
        DrawText(status, 5, 460, 20, BLACK);

        // DrawTexture(checked, screenWidth/2 - checked.width/2, screenHeight/2 - checked.height/2, Fade(WHITE, 0.5f));

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    free(status);

    UnloadTexture(checked);     // Texture unloading

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}