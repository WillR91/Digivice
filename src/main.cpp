// Full code for src/main.cpp - 2 Frame Animation with TaiTest names

#include "platform/pc/pc_display.h"
// --- Include the headers for your frames ---
// !!! These MUST match the filenames in your assets folder !!!
#include "TaiTest_0.h" // Frame 1 Header (adjust if filename differs)
#include "TaiTest2_0.h" // Frame 2 Header (adjust if filename differs)
// ---
#include <SDL2/SDL.h>
#include <SDL2/SDL_log.h>
#include <vector> // Keep include just in case

// --- Animation Setup ---

// Array of pointers to the pixel data arrays defined in the generated header files
// !!! Uses variable names from your header files !!!
const uint16_t* animation_frames[] = {
    TaiTest_0_data, // Pointer to Frame 1 data array
    TaiTest2_0_data // Pointer to Frame 2 data array
};
// Calculate how many frames are in the array automatically
const int animation_frame_count = sizeof(animation_frames) / sizeof(animation_frames[0]);

// Define animation speed (milliseconds between frame changes)
const Uint32 animation_delay = 500; // 0.5 seconds per frame

// --- Sprite Dimensions ---
// Use the dimensions defined in the first frame's header
// !!! Uses define names from your first header file !!!
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
    SDL_Log("--- Starting Animation Test ---");

    PCDisplay display;
    SDL_Log("--- PCDisplay object created ---");

    SDL_Log("--- Calling display.init() ---");
    // Using sprite dimensions for window size for now
    if (!display.init("Digivice Sim - Animated!", SPRITE_WIDTH * 2, SPRITE_HEIGHT * 2)) { // Made window bigger to fit sprite
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "!!! display.init() failed!");
        const char* sdlError = SDL_GetError();
        if (sdlError && sdlError[0] != '\0') {
             SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Last SDL Error: %s", sdlError);
        }
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

        // --- Drawing ---
        display.clear(0x0000); // Clear screen to black

        // Draw the *current* animation frame using the index we calculated
        // Drawing at (10, 10) for visibility
        display.drawPixels(10, 10,                 // Position (X, Y)
                           SPRITE_WIDTH,           // Use width from defines
                           SPRITE_HEIGHT,          // Use height from defines
                           animation_frames[current_frame_index]); // Use current frame data pointer

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