#pragma once

#include "idisplay.h" // Include the interface we defined (Compiler looks in src/hal)
#include <SDL2/SDL.h>     // The main SDL2 header

// This class *implements* the IDisplay interface specifically for the PC
// using the SDL2 library.
class PCDisplay : public IDisplay {
public:
    PCDisplay();                  // Constructor
    ~PCDisplay() override;        // Destructor (overrides the base class one)

    // --- Implementations of the IDisplay interface ---
    bool init(const char* title, int width, int height) override;
    void clear(uint16_t color) override;
    void present() override;
    void close() override;
    // We will add drawPixels(...) later

    // Helper function for our main loop to check if SDL initialized okay
    bool isInitialized() const;
    // Helper function for our main loop to handle events (like closing the window)
    SDL_Window* getWindow() const; // We don't strictly need this yet, but good to have


private: // Internal details the outside world doesn't need to know
    SDL_Window* window_ = nullptr;     // Pointer to the SDL window
    SDL_Renderer* renderer_ = nullptr; // Pointer to the SDL renderer (for drawing)
    bool initialized_ = false;         // Flag to track if init() was successful

    // Internal helper function to convert our 16-bit color to SDL's format
    SDL_Color convert_rgb565_to_sdl_color(uint16_t color565);
};