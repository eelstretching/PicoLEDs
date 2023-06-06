#include "Font.h"

#include "pico/types.h"


Font::Font(Canvas* canvas, const uint8_t* fontData) {
    this->canvas = canvas;
    fontWidth = fontData[0];
    fontHeight = fontData[1];
    fontSpaceWidth = fontData[2];
    fontBase = fontData[3];
    fontUpper = fontData[4];
    this->fontData = &fontData[5];
    fWBytes = (fontWidth + 7) / 8;
    fCBytes = (fWBytes * fontHeight);
}

uint Font::render(const char* text, uint bx, uint by, RGB color) {
    uint x = bx;
    uint p = 0;
    uint tw = 0;

    //
    // Loop until the string-terminating null.
    while (text[p] != 0) {
        char c = text[p++];
        if (c == 32) {
            x += fontSpaceWidth;
            tw += fontSpaceWidth;
            continue;
        }
        tw += render(c, x, by, color);
        tw += spacing;
        x += (fontWidth + spacing);
    }
    return tw;
}

uint Font::render(char c, uint bx, uint by, RGB color) {
    //
    // Figure out where the data for this caracter starts in our font.
    uint fdp = (c - fontBase) * fCBytes;

    //
    // Our origin is at the lower left (i.e., Cartesian), so we need to render from the 
    // top down with a character.
    uint y = by + fontHeight;
    for (int i = 0; i < fCBytes; i++) {
        uint x = bx;
        uint cdata = fontData[fdp + i];
        for (int j = 0; j < fontWidth; j++) {
            //
            // There's a 1 bit in the font character.
            if ((cdata & 0x80) != 0) {
                canvas->set(x,y,color);
            }
            //
            // Advance the x position, and shift for the next bit of this row of
            // the character.
            x++;
            cdata <<= 1;
        }

        //
        // Move onto the next row of the caracter.
        y--;
    }
    return fontWidth;
}

void Font::setSpacing(uint spacing) {
    this->spacing = spacing;
}
