// Full code for pc_display.cpp - Test 1: NO Byte Swapping, YES Color Keying

#include "pc_display.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_log.h>
#include <iostream>
#include <vector> // Keep include just in case, though not used here


// Constructor
PCDisplay::PCDisplay() {}

// Destructor - ensure resources are cleaned up
PCDisplay::~PCDisplay() {
    close();
}

// Initialize SDL, create window and renderer
bool PCDisplay::init(const char* title, int width, int height) {
    // (Keep the init function exactly as it was)
    if (initialized_) { /*...*/ return true; }
    if (SDL_Init(SDL_INIT_VIDEO) < 0) { /*...*/ return false; }
    window_ = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    if (window_ == nullptr) { /*...*/ return false; }
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer_ == nullptr) { /*...*/ return false; }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PCDisplay initialized successfully.");
    SDL_RendererInfo info;
    if (SDL_GetRendererInfo(renderer_, &info) == 0) { SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Created renderer: %s", info.name ? info.name : "Unknown"); } else { SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Could not get renderer info."); }
    initialized_ = true;
    return true;
}

// Helper function to convert RGB565 (16-bit) to SDL_Color (RGBA8888)
SDL_Color PCDisplay::convert_rgb565_to_sdl_color(uint16_t color565) {
   // (Keep this function exactly as it was)
    uint8_t r5 = (color565 >> 11) & 0x1F;
    uint8_t g6 = (color565 >> 5)  & 0x3F;
    uint8_t b5 = color565        & 0x1F;
    uint8_t r8 = (r5 * 255) / 31;
    uint8_t g8 = (g6 * 255) / 63;
    uint8_t b8 = (b5 * 255) / 31;
    SDL_Color sdlColor = {r8, g8, b8, 255};
    return sdlColor;
}

// Clear the screen with a specific RGB565 color
void PCDisplay::clear(uint16_t color565) {
    // (Keep this function exactly as it was)
    if (!initialized_) return;
    SDL_Color sdlColor = convert_rgb565_to_sdl_color(color565);
    SDL_SetRenderDrawColor(renderer_, sdlColor.r, sdlColor.g, sdlColor.b, sdlColor.a);
    SDL_RenderClear(renderer_);
}

// --- Draw pixels - VERSION for Test 1: NO Byte Swapping, YES Color Keying ---
void PCDisplay::drawPixels(int x, int y, int w, int h, const uint16_t* data) {
    if (!initialized_ || !data || w <= 0 || h <= 0) {
        return;
    }

    // --- NO Byte Swapping Logic in this version ---

    // Create surface directly from original data using the explicit RGB565 format enum.
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormatFrom(
        (void*)data,              // <<< Use ORIGINAL data pointer
        w,                        // Width
        h,                        // Height
        16,                       // Depth (bits per pixel)
        w * 2,                    // Pitch (bytes per row)
        SDL_PIXELFORMAT_RGB565 ); // <<< Use standard RGB565 format enum

    if (!surface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "drawPixels: Failed to create surface! SDL Error: %s", SDL_GetError());
        return;
    }

    // *** SET THE COLOR KEY (Keep this for now) ***
    Uint32 magenta_key_rgb565 = 0xF81F;
    if (SDL_SetColorKey(surface, SDL_TRUE, magenta_key_rgb565) != 0) { // SDL_TRUE enables color key
         SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "drawPixels: Failed to set color key! SDL Error: %s", SDL_GetError());
    }
    // *** END COLOR KEYING ***

    // Create texture from surface
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
    SDL_FreeSurface(surface); // Free surface wrapper
    surface = nullptr;

    if (!texture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "drawPixels: Failed to create texture! SDL Error: %s", SDL_GetError());
        return;
    }

    SDL_Rect dstRect = { x, y, w, h };
    SDL_RenderCopy(renderer_, texture, NULL, &dstRect); // Render, skipping magenta pixels if color key worked
    SDL_DestroyTexture(texture);
    texture = nullptr;
}
// --- End of drawPixels function ---


// Show the contents of the renderer on the window
void PCDisplay::present() {
   // (Keep this function exactly as it was)
    if (!initialized_) return;
    SDL_RenderPresent(renderer_);
}

// Clean up SDL resources
void PCDisplay::close() {
   // (Keep this function exactly as it was)
    if (!initialized_) return;
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Closing PCDisplay.");
    if (renderer_) { SDL_DestroyRenderer(renderer_); renderer_ = nullptr; }
    if (window_) { SDL_DestroyWindow(window_); window_ = nullptr; }
    SDL_Quit();
    initialized_ = false;
}

// Check if initialization was successful
bool PCDisplay::isInitialized() const {
    return initialized_;
}

// Get the window pointer (optional)
SDL_Window* PCDisplay::getWindow() const {
    return window_;
}