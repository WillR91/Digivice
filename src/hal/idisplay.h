#pragma once // Standard guard to prevent including this file multiple times

#include <cstdint> // Needed for types like uint16_t

// This is an "Interface" class. It defines *what* a display must do,
// but not *how*. Other classes will inherit from this.
class IDisplay {
public:
    // Virtual destructor: Important for classes designed to be inherited from.
    virtual ~IDisplay() = default;

    // Pure virtual functions (indicated by = 0).
    // Any class inheriting from IDisplay *must* provide implementations for these.
    virtual bool init(const char* title, int width, int height) = 0;
    virtual void clear(uint16_t color) = 0; // We'll use RGB565 for color representation
    virtual void present() = 0;             // Makes the drawn stuff visible
    virtual void close() = 0;               // Cleans up resources

    // We will add drawPixels(...) later
};