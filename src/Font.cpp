#include "Font.h"

#include "pico/types.h"

Font::Font(Canvas* canvas, const uint8_t* fontData) {
    this->canvas = canvas;
    this->proportional = fontData[0] == 1;
    fontWidth = fontData[1];
    fontHeight = fontData[2];
    fontSpaceWidth = fontData[3];
    fontBase = fontData[4];
    fontUpper = fontData[5];
    this->fontData = &fontData[6];
    //
    // How many bytes per row of pixels for the character. We may slop over into
    // an extra byte if the width is not divisible by 8!
    fWBytes = fontWidth / 8 + (fontWidth % 8 != 0 ? 1 : 0);
    //
    // How many bytes for each character.
    fCBytes = (fWBytes * fontHeight);
    if (proportional) {
        //
        // Proportional fonts have an extra byte for the width of the character.
        fCBytes++;
    }
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
            //
            // Space is just a space, bro.
            x += fontSpaceWidth;
            tw += fontSpaceWidth;
            continue;
        }
        uint charWidth = render(c, x, by, color);
        tw = tw + charWidth + spacing;
        x += (charWidth + spacing);
    }
    return tw;
}

uint Font::render(char c, uint bx, uint by, RGB color) {
    //
    // Figure out where the data for this caracter starts in our font.
    uint fdp = (c - fontBase) * fCBytes;

    //
    // How wide is this particular character, in pixels?
    uint8_t charWidth = proportional ? fontData[fdp] : fontWidth;

    //
    // Our origin is at the lower left (i.e., Cartesian), so we need to render
    // from the top down with a character.
    uint y = by + fontHeight;

    //
    // Loop over the rows of pixels in the character.
    for (int i = 0; i < fontHeight; i++) {
        //
        // Each row starts at the same x.
        uint x = bx;

        //
        // The byte of data for the character.
        uint8_t cdata;

        //
        // Loop over the columns of pixels in the character.
        for (int j = 0; j < charWidth; j++) {
            //
            // When do we need to switch to the next byte of character data for
            // more pixels? Note that this will be triggered in the first
            // iteration of the loop, which is fine, actually.
            if (j % 8 == 0) {
                cdata = fontData[++fdp];
            }
            //
            // There's a 1 bit in the font character.
            if ((cdata & 0x80) != 0) {
                canvas->set(x, y, color);
            }
            //
            // Advance the x position, and shift for the next bit of this
            // row of the character.
            x++;
            cdata <<= 1;
        }
        //
        // Skip any empty bytes of data for this possibly narrow character.
        fdp += (fontWidth - charWidth) / 8;
        y--;
    }
    return charWidth;
}

void Font::setSpacing(uint spacing) { this->spacing = spacing; }
