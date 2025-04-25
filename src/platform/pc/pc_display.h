#include <SDL2/SDL.h>
#pragma once

#include "idisplay.h" // Include the interface we are implementing#include <SDL2/SDL.h>     // Include the main SDL2 header

// PC-specific implementation of the IDisplay interface using SDL2.
class PCDisplay : public IDisplay {
public:
    PCDisplay();                  // Constructor
    ~PCDisplay() override;        // Destructor (marked override)

    // --- Implementations of the IDisplay virtual functions ---
    bool init(const char* title, int width, int height) override;
    void clear(uint16_t color) override;
    // ADDED declaration for drawPixels implementation
    void drawPixels(int x, int y, int w, int h, const uint16_t* data) override;
    void present() override;
    void close() override;

    // Helper function to check if initialization was successful
    bool isInitialized() const;
    // Helper function (optional, might be useful later)
    SDL_Window* getWindow() const;


private: // Internal implementation details
    SDL_Window* window_ = nullptr;     // Pointer to the SDL window object
    SDL_Renderer* renderer_ = nullptr; // Pointer to the SDL renderer object
    bool initialized_ = false;         // Tracks if init() was successful

    // Internal helper function to convert colors
    SDL_Color convert_rgb565_to_sdl_color(uint16_t color565);
};