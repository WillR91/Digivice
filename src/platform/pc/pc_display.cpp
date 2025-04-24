#include "pc_display.h"
#include <stdexcept> // For runtime_error (optional error handling)
#include <iostream>  // For printing error messages

// Constructor: Doesn't need to do anything right now
PCDisplay::PCDisplay() {}

// Destructor: Make sure we clean up SDL resources when the object is destroyed
PCDisplay::~PCDisplay() {
    close();
}

// Initialize SDL, create window and renderer
bool PCDisplay::init(const char* title, int width, int height) {
    if (initialized_) {
        std::cerr << "Warning: PCDisplay already initialized." << std::endl;
        return true; // Already done
    }

    // Initialize SDL video subsystem
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Create the window
    window_ = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    if (window_ == nullptr) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit(); // Clean up SDL initialization
        return false;
    }

    // Create the renderer (hardware accelerated, vsync)
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer_ == nullptr) {
        std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window_);
        window_ = nullptr;
        SDL_Quit();
        return false;
    }

    std::cout << "PCDisplay initialized successfully." << std::endl;
    initialized_ = true;
    return true;
}

// Helper function to convert our color format
SDL_Color PCDisplay::convert_rgb565_to_sdl_color(uint16_t color565) {
    // Extract R, G, B components from RGB565 format
    uint8_t r5 = (color565 >> 11) & 0x1F; // 5 bits for Red
    uint8_t g6 = (color565 >> 5) & 0x3F;  // 6 bits for Green
    uint8_t b5 = color565 & 0x1F;         // 5 bits for Blue

    // Scale components to 8-bit (0-255)
    uint8_t r8 = (r5 * 255) / 31;
    uint8_t g8 = (g6 * 255) / 63;
    uint8_t b8 = (b5 * 255) / 31;

    // Create SDL_Color (RGBA8888 format)
    SDL_Color sdlColor = {r8, g8, b8, 255}; // 255 is fully opaque for Alpha
    return sdlColor;
}


// Clear the screen with a specific color
void PCDisplay::clear(uint16_t color565) {
    if (!initialized_) return;

    SDL_Color sdlColor = convert_rgb565_to_sdl_color(color565);

    // Set the drawing color for the renderer
    SDL_SetRenderDrawColor(renderer_, sdlColor.r, sdlColor.g, sdlColor.b, sdlColor.a);
    // Clear the entire rendering target with the drawing color
    SDL_RenderClear(renderer_);
}

// Show the contents of the renderer on the window
void PCDisplay::present() {
    if (!initialized_) return;
    SDL_RenderPresent(renderer_);
}

// Clean up SDL resources
void PCDisplay::close() {
    if (!initialized_) return;

    std::cout << "Closing PCDisplay." << std::endl;
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
    SDL_Quit(); // Quit SDL subsystems
    initialized_ = false;
}

// Check if initialization was successful
bool PCDisplay::isInitialized() const {
    return initialized_;
}

 // Get the window pointer (might need later for event handling)
SDL_Window* PCDisplay::getWindow() const {
    return window_;
}