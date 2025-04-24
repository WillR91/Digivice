#include "platform/pc/pc_display.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_log.h> // Include SDL logging

int main(int argc, char* argv[]) {
    // Use SDL_Log which might be more reliable under debugger/redirection
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG); // Show all SDL log levels
    SDL_Log("--- Starting main() ---");

    PCDisplay display;
    SDL_Log("--- PCDisplay object created ---");

    SDL_Log("--- Calling display.init() ---");
    if (!display.init("Digivice Sim", 466, 466)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "!!! display.init() returned false!");
        // SDL_GetError should still work
        const char* sdlError = SDL_GetError();
        if (sdlError && sdlError[0] != '\0') {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Last SDL Error: %s", sdlError);
        }
        SDL_Quit(); // Quit SDL if init failed here
        return 1;
    }
    SDL_Log("--- display.init() succeeded ---");

    // *** ADD A DELAY HERE TO SEE IF WINDOW IS VISIBLE ***
    SDL_Log("--- Delaying for 5 seconds before main loop ---");
    SDL_Delay(5000); // Delay for 5000 milliseconds (5 seconds)

    bool quit = false;
    SDL_Event e;

    SDL_Log("--- Entering main loop ---");
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                SDL_Log("--- SDL_QUIT event received ---");
                quit = true;
            }
        }

        display.clear(0x0000); // Black
        display.present();

        // Shorter delay inside loop now that we have the initial delay
        SDL_Delay(16); // Delay roughly 16ms (aiming for ~60fps loop)
    }

    SDL_Log("--- Exited main loop ---");
    display.close();
    SDL_Log("--- display.close() called ---");
    SDL_Log("--- Simulator finished ---");
    return 0;
}