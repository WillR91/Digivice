// Full code for src/main.cpp - Centered 2 Frame Animation

#include "platform/pc/pc_display.h"
// --- Include the generated headers for your frames ---
// !!! Double-check these filenames match your files in assets/ !!!
#include "TaiTest_0.h" // Frame 1 Header
#include "TaiTest2_0.h" // Frame 2 Header
// ---
#include <SDL2/SDL.h>
#include <SDL2/SDL_log.h>
#include <vector> // Keep include just in case

// --- Window Dimensions ---
const int WINDOW_WIDTH = 466;
const int WINDOW_HEIGHT = 466;

// --- Animation Setup ---

// Pointers to the pixel data arrays defined in the generated header files
// !!! Uses variable names based on your header file !!!
const uint16_t* animation_frames[] = {
    TaiTest_0_data, // Pointer to Frame 1 data array
    TaiTest2_0_data // Pointer to Frame 2 data array
};
const int animation_frame_count = sizeof(animation_frames) / sizeof(animation_frames[0]);

// Define animation speed (milliseconds between frame changes)
const Uint32 animation_delay = 500; // 0.5 seconds per frame

// --- Sprite Dimensions ---
// Use the dimensions defined in the first frame's header
// !!! Uses define names based on your header file !!!
#ifndef TAITEST_0_WIDTH
    #error "TAITEST_0_WIDTH not defined - check TaiTest_0.h"
#endif
#ifndef TAITEST_0_HEIGHT
    #error "TAITEST_0_HEIGHT not defined - check TaiTest_0.h"
#endif
const int SPRITE_WIDTH = TAITEST_0_WIDTH;   // Should be 192
const int SPRITE_HEIGHT = TAITEST_0_HEIGHT; // Should be 192
// --- (Assuming frame 2 has same dimensions based on headers) ---


int main(int argc, char* argv[]) {
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
    SDL_Log("--- Starting Centered Animation Test ---"); // Updated log

    PCDisplay display;
    SDL_Log("--- PCDisplay object created ---");

    SDL_Log("--- Calling display.init() ---");
    if (!display.init("Digivice Sim - Centered!", WINDOW_WIDTH, WINDOW_HEIGHT)) { // Use constants
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "!!! display.init() failed!");
        // ... (error handling) ...
        SDL_Quit();
        return 1;
    }
    SDL_Log("--- display.init() succeeded ---");

    bool quit = false;
    SDL_Event e;

    // --- Animation State Variables ---
    int current_frame_index = 0;
    Uint32 last_animation_update_time = 0;

    SDL_Log("--- Entering main loop ---");
    while (!quit) {
        // --- Handle Input ---
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                SDL_Log("--- SDL_QUIT event received ---");
                quit = true;
            }
        }

        // --- Animation Timing Logic ---
        Uint32 current_time = SDL_GetTicks();
        if (current_time > last_animation_update_time + animation_delay) {
            current_frame_index = (current_frame_index + 1) % animation_frame_count;
            last_animation_update_time = current_time;
        }

        // --- Calculate Centered Position ---
        // Integer division is fine here
        int draw_x = (WINDOW_WIDTH / 2) - (SPRITE_WIDTH / 2);
        int draw_y = (WINDOW_HEIGHT / 2) - (SPRITE_HEIGHT / 2);

        // --- Drawing ---
        display.clear(0x0000); // Clear screen to black

        // Draw the *current* animation frame AT THE CALCULATED CENTERED POSITION
        display.drawPixels(draw_x, draw_y,             // <<< Use calculated X, Y
                           SPRITE_WIDTH,
                           SPRITE_HEIGHT,
                           animation_frames[current_frame_index]);

        // --- Update Screen ---
        display.present();

        // Loop delay
        SDL_Delay(16);
    }

    // --- Cleanup ---
    SDL_Log("--- Exited main loop ---");
    display.close();
    SDL_Log("--- display.close() called ---");
    SDL_Log("--- Simulator finished ---");
    return 0;
}