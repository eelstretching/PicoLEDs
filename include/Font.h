#ifndef FONT_H
#define FONT_H
#pragma once

#include "Canvas.h"
#include "color.h"
#include "pico/types.h"

/* Binary constant generator macro By Tom Torfs - donated to the public domain
 */
/* All macro's evaluate to compile-time constants */
#ifndef HEX__
#define HEX__(n) 0x##n##LU
#endif
#ifndef B8__
#define B8__(x)                                                         \
    ((x & 0x0000000FLU) ? 1 : 0) + ((x & 0x000000F0LU) ? 2 : 0) +       \
        ((x & 0x00000F00LU) ? 4 : 0) + ((x & 0x0000F000LU) ? 8 : 0) +   \
        ((x & 0x000F0000LU) ? 16 : 0) + ((x & 0x00F00000LU) ? 32 : 0) + \
        ((x & 0x0F000000LU) ? 64 : 0) + ((x & 0xF0000000LU) ? 128 : 0)
#define B8(d1) ((uint8_t)B8__(HEX__(d1)))
#define B16(d1, d2) ((uint8_t)B8__(HEX__(d1))), ((uint8_t)B8__(HEX__(d2)))
#define B24(d1, d2, d3) ((uint8_t)B8__(HEX__(d1))),((uint8_t)B8__(HEX__(d2))),((uint8_t)B8__(HEX__(d3))))
#endif

class Font {
   public:
    /// @brief Create a class that can render the given font data onto the
    /// provided canvas.
    /// @param canvas The canvas onto which we'll render text
    /// @param fontData The data for the font we'll render.
    Font(Canvas *canvas, const uint8_t *fontData);
    
    /// @brief Renders the provided text onto the underlying matrix, at the
    /// given pixel position. We're assuming that the canvas has 0,0 in the
    /// bottom-left corner (i.e., Cartesian coordinates)!
    /// @param text The carachters to render. They must appear in the font!
    /// @param bx The base x coordinate for rendering
    /// @param by The base y coordinate for rendering
    /// @param color The color to render the text in
    /// @return The width of the rendered text, in pixels.
    uint render(const char *text, uint bx, uint by, RGB color);

    /// @brief Renders a single character onto our canvas at the given pixel
    /// position.
    /// @param c The character to render
    /// @param bx The base x coordinate for rendering
    /// @param by The base y coordinate for rendering
    /// @param color The color to render the text in.
    /// @return the width of the rendered character, in pixels.
    uint render(char c, uint bx, uint by, RGB color);

    /// @brief Gets the width of the given string rendered in this font.
    /// @param text 
    void getWidth(const char *text);

    uint getSpacing() {return spacing;};

    /// @brief Sets the spacing between characters, if we wish to override the
    /// definition in the font.
    /// @param spacing The spacing (in pixels) to use between characters.
    void setSpacing(uint spacing);
    /// @brief Gets the width of the font
    /// @return the width of the font
    uint8_t getFontWidth() { return (fontWidth); };
    /// @brief Gets the height of the font
    /// @return the height of the font
    uint8_t getFontHeight() { return (fontHeight); };

   private:
    Canvas *canvas;
    bool proportional;
    uint8_t fontWidth, fontHeight, fontSpaceWidth, fontBase, fontUpper, fWBytes,
        fCBytes;
    uint8_t spacing = 1;
    const uint8_t *fontData;
    uint8_t dbp;
};

#endif