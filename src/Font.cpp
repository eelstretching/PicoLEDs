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

uint Font::render(const char* text, int bx, int by, uint8_t color,
                  RenderAngle angle) {
    switch (angle) {
        case RENDER_0:
            return render0(text, bx, by, color);
        case RENDER_90:
            return render90(text, bx, by, color);
    }
    return 0;
}

uint Font::render0(const char* text, int bx, int by, uint8_t color) {
    uint x = bx;
    uint p = 0;
    uint tw = 0;
    uint lw = 0;

    //
    // Loop until the string-terminating null.
    while (text[p] != 0) {
        char c = text[p++];
        switch (c) {
            case ' ':
                //
                // Space is just a space, bro.
                x += fontSpaceWidth;
                tw += fontSpaceWidth;
                break;
            case '\n':
                //
                // New line puts us down to re-start at our current x, and we'll
                // remember which line was widest.
                x = bx;
                by = by - fontHeight - 1;
                if (tw > lw) {
                    lw = tw;
                }
                break;
            default:
                uint charWidth = render0(c, x, by, color);
                tw = tw + charWidth + spacing;
                x += (charWidth + spacing);
        }
    }
    return lw == 0 ? tw : lw;
}

uint Font::render0(char c, int bx, int by, uint8_t color) {
    //
    // Figure out where the data for this caracter starts in our font.
    uint fdp = (c - fontBase) * fCBytes;

    //
    // How wide is this particular character, in pixels?
    uint8_t charWidth = proportional ? fontData[fdp] : fontWidth;

    //
    // Our origin is at the lower left (i.e., Cartesian), so we need to
    // render from the top down with a character. Note that we want to end
    // on this row, so we need to subtract 1
    uint y = by + fontHeight - 1;

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
            // When do we need to switch to the next byte of character data
            // for more pixels? Note that this will be triggered in the
            // first iteration of the loop, which is fine, actually.
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

uint Font::render90(const char* text, int bx, int by, uint8_t color) {
    uint y = by;
    uint p = 0;
    uint tw = 0;
    uint lw = 0;

    //
    // Loop until the string-terminating null.
    while (text[p] != 0) {
        char c = text[p++];
        switch (c) {
            case ' ':
                //
                // Space is just a space, bro.
                y -= fontSpaceWidth;
                tw += fontSpaceWidth;
                break;
            case '\n':
                //
                // New line puts us down to re-start at our initial y, and we'll
                // remember which line was widest.
                y = by;
                bx = bx - fontHeight - 1;
                if (tw > lw) {
                    lw = tw;
                }
                break;
            default:
                uint charWidth = render90(c, y, bx, color);
                tw = tw + charWidth + spacing;
                y -= (charWidth + spacing);
        }
    }
    return lw == 0 ? tw : lw;
}

uint Font::render90(char c, int by, int bx, uint8_t color) {
    //
    // Figure out where the data for this caracter starts in our font.
    uint fdp = (c - fontBase) * fCBytes;

    //
    // How wide is this particular character, in pixels?
    uint8_t charWidth = proportional ? fontData[fdp] : fontWidth;

    //
    // Our origin is at the lower left (i.e., Cartesian), so we need to
    // render from the top down with a character. Note that we want to end
    // on this row, so we need to subtract 1
    uint x = bx + fontHeight - 1;

    //
    // Loop over the rows of pixels in the character.
    for (int i = 0; i < fontHeight; i++) {
        //
        // Each row starts at the same y.
        uint y = by;

        //
        // The byte of data for the character.
        uint8_t cdata;

        //
        // Loop over the columns of pixels in the character.
        for (int j = 0; j < charWidth; j++) {
            //
            // When do we need to switch to the next byte of character data
            // for more pixels? Note that this will be triggered in the
            // first iteration of the loop, which is fine, actually.
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
            y--;
            cdata <<= 1;
        }
        //
        // Skip any empty bytes of data for this possibly narrow character.
        fdp += (fontWidth - charWidth) / 8;
        x--;
    }
    return charWidth;
}

uint Font::getWidth(const char* text) {
    uint p = 0;
    uint tw = 0;

    //
    // Loop until the string-terminating null.
    while (text[p] != 0) {
        char c = text[p++];
        if (c == 32) {
            //
            // Space is just a space, bro.
            tw += fontSpaceWidth;
            continue;
        }
        uint charWidth = getWidth(c);
        tw = tw + charWidth + spacing;
    }
    return tw;
}

uint Font::getWidth(char c) {
    if (!proportional) {
        return fontWidth;
    }
    return fontData[(c - fontBase) * fCBytes];
}

void Font::setSpacing(uint spacing) { this->spacing = spacing; }
