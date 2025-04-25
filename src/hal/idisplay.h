#pragma once // Standard guard to prevent including this file multiple times

#include <cstdint> // Needed for types like uint16_t

// Interface class defining what any display must be able to do.
class IDisplay {
public:
    // Virtual destructor (important for base classes)
    virtual ~IDisplay() = default;

    // --- Pure virtual functions (= 0) must be implemented by derived classes ---

    // Initialize the display
    virtual bool init(const char* title, int width, int height) = 0;

    // Clear the display to a single color (using RGB565 format)
    virtual void clear(uint16_t color) = 0;

    // Draw a block of pixel data (RGB565 format) at specific coordinates
    // THIS IS THE NEW FUNCTION DECLARATION WE ADDED
    virtual void drawPixels(int x, int y, int w, int h, const uint16_t* data) = 0;

    // Present the drawn buffer to the screen (make it visible)
    virtual void present() = 0;

    // Clean up display resources
    virtual void close() = 0;
};