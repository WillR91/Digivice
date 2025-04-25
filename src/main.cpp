// Full correct code for src/main.cpp

#include "platform/pc/pc_display.h" // Include PC implementation
#include "my_sprite.h"              // <<< Ensure this line uses the correct include
#include <SDL2/SDL.h>
#include <SDL2/SDL_log.h>           // Include SDL logging

int main(int argc, char* argv[]) {
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG); // Show all SDL log levels
    SDL_Log("--- Starting main() ---");

    PCDisplay display; // Create display object
    SDL_Log("--- PCDisplay object created ---");

    // Initialize display (using target resolution for window size)
    SDL_Log("--- Calling display.init() ---");
    if (!display.init("Digivice Sim", 466, 466)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "!!! display.init() returned false!");
        const char* sdlError = SDL_GetError();
        if (sdlError && sdlError[0] != '\0') {
             SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Last SDL Error: %s", sdlError);
        }
        SDL_Quit();
        return 1;
    }
    SDL_Log("--- display.init() succeeded ---");

    // Optional short delay after init if needed
    // SDL_Delay(100);

    bool quit = false;
    SDL_Event e;

    SDL_Log("--- Entering main loop ---");
    while (!quit) {
        // --- Handle Input ---
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) { // User clicks the window's 'X' button
                SDL_Log("--- SDL_QUIT event received ---");
                quit = true;
            }
            // Add keyboard/mouse event handling here later
        }

        // --- Drawing ---
        // Clear screen (e.g., to black)
        display.clear(0x0000);

        // Inside the while(!quit) loop, after display.clear():

        // Draw the static sprite! Use the names/sizes from the header
        display.drawPixels(100, 100, SPRITE_WIDTH, SPRITE_HEIGHT, sprite_data);
         // Draw other things here later (UI, backgrounds...)


        // --- Update Screen ---
        // Show everything that was drawn in this frame
        display.present();

        // Small delay to prevent 100% CPU usage and cap framerate
        SDL_Delay(16); // ~60 FPS target
    }

    // --- Cleanup ---
    SDL_Log("--- Exited main loop ---");
    display.close(); // Clean up display resources
    SDL_Log("--- display.close() called ---");
    SDL_Log("--- Simulator finished ---");
    return 0;
}