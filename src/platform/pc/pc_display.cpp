#include <SDL2/SDL.h>
#include "pc_display.h"
#include <stdexcept> // For runtime_error (optional error handling)
#include <iostream>  // For printing error messages using std::cerr (can be removed if using SDL_Log exclusively)
#include <SDL2/SDL_log.h> // Using SDL_Log for consistency

// Constructor
PCDisplay::PCDisplay() {}

// Destructor - ensure resources are cleaned up
PCDisplay::~PCDisplay() {
    close();
}

// Initialize SDL, create window and renderer
bool PCDisplay::init(const char* title, int width, int height) {
    if (initialized_) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "PCDisplay already initialized.");
        return true; // Already done
    }

    // Initialize SDL video subsystem
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL could not initialize! SDL_Error: %s", SDL_GetError());
        return false;
    }

    // Create the window
    window_ = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    if (window_ == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window could not be created! SDL_Error: %s", SDL_GetError());
        SDL_Quit(); // Clean up SDL initialization
        return false;
    }

    // Create the renderer (hardware accelerated, vsync enabled)
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer_ == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Renderer could not be created! SDL Error: %s", SDL_GetError());
        SDL_DestroyWindow(window_);
        window_ = nullptr;
        SDL_Quit();
        return false;
    }

    // --- Optional: Print renderer info safely ---
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PCDisplay initialized successfully.");
    SDL_RendererInfo info; // Declare info variable
    if (SDL_GetRendererInfo(renderer_, &info) == 0) { // Check return value
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Created renderer: %s", info.name ? info.name : "Unknown"); // Safely print name
    } else {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Could not get renderer info.");
    }
    // --- End of optional block ---

    initialized_ = true;
    return true;
}

// Helper function to convert RGB565 (16-bit) to SDL_Color (RGBA8888)
SDL_Color PCDisplay::convert_rgb565_to_sdl_color(uint16_t color565) {
    // Extract R5, G6, B5 components
    uint8_t r5 = (color565 >> 11) & 0x1F;
    uint8_t g6 = (color565 >> 5)  & 0x3F;
    uint8_t b5 = color565        & 0x1F;

    // Scale to R8, G8, B8
    uint8_t r8 = (r5 * 255) / 31;
    uint8_t g8 = (g6 * 255) / 63;
    uint8_t b8 = (b5 * 255) / 31;

    SDL_Color sdlColor = {r8, g8, b8, 255}; // Full Alpha (opaque)
    return sdlColor;
}

// Clear the screen with a specific RGB565 color
void PCDisplay::clear(uint16_t color565) {
    if (!initialized_) return;

    SDL_Color sdlColor = convert_rgb565_to_sdl_color(color565);
    SDL_SetRenderDrawColor(renderer_, sdlColor.r, sdlColor.g, sdlColor.b, sdlColor.a);
    SDL_RenderClear(renderer_);
}

// --- UPDATED FUNCTION IMPLEMENTATION with LOGGING ---
// Draw raw RGB565 pixel data onto the screen, using Magenta (0xF81F) as transparent key
void PCDisplay::drawPixels(int x, int y, int w, int h, const uint16_t* data) {
    if (!initialized_ || !data || w <= 0 || h <= 0) {
        return; // Cannot draw if not initialized, no data, or invalid size
    }

    // Define the bit masks for RGB565 format.
    Uint32 rmask = 0xF800; // Red mask for 5 bits
    Uint32 gmask = 0x07E0; // Green mask for 6 bits
    Uint32 bmask = 0x001F; // Blue mask for 5 bits
    Uint32 amask = 0x0000; // No alpha in RGB565

    // Create an SDL_Surface directly mapping our existing pixel data in RAM.
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormatFrom(
        (void*)data, w, h, 16, w * 2, SDL_PIXELFORMAT_RGB565); // Use format enum instead of masks

    if (!surface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "drawPixels: Failed to create surface from pixel data! SDL Error: %s", SDL_GetError());
        return;
    }

    // === Set Color Key and Add Logging ===
    Uint32 key = 0xF81F; // Magenta in RGB565
    if (SDL_SetColorKey(surface, SDL_TRUE, key) < 0) {
        // Log error if setting color key fails
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "drawPixels: Error setting color key 0x%X: %s", key, SDL_GetError());
    } else {
        // Log success if setting color key works
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "drawPixels: Color key 0x%X set successfully.", key);
    }
    // ====================================

    // Create an SDL_Texture from the surface.
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);

    // Free the surface wrapper (doesn't free your original sprite_data).
    SDL_FreeSurface(surface);
    surface = nullptr;

    if (!texture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "drawPixels: Failed to create texture from surface! SDL Error: %s", SDL_GetError());
        return;
    }

    // Define the destination rectangle on the screen.
    SDL_Rect dstRect = { x, y, w, h };

    // Copy the texture to the renderer (respecting the color key).
    SDL_RenderCopy(renderer_, texture, NULL, &dstRect);

    // Clean up the texture resource.
    SDL_DestroyTexture(texture);
    texture = nullptr;
}
// --- END OF UPDATED FUNCTION IMPLEMENTATION ---


// Show the contents of the renderer on the window
void PCDisplay::present() {
    if (!initialized_) return;
    SDL_RenderPresent(renderer_);
}

// Clean up SDL resources
void PCDisplay::close() {
    if (!initialized_) return;

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Closing PCDisplay."); // Use SDL_Log
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

// Get the window pointer (optional)
SDL_Window* PCDisplay::getWindow() const {
    return window_;
}