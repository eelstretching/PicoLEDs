/// @file colorutils.cpp
/// Utility functions for color fill, palettes, blending, and more

#include "colorutils.h"

#include <math.h>
#include <stdint.h>

#include "pico/printf.h"

void fill_solid(struct RGB* targetArray, int numToFill,
                const struct RGB& color) {
    for (int i = 0; i < numToFill; ++i) {
        targetArray[i] = color;
    }
}

void fill_solid(struct HSV* targetArray, int numToFill,
                const struct HSV& color) {
    for (int i = 0; i < numToFill; ++i) {
        targetArray[i] = color;
    }
}

// void fill_solid( struct RGB* targetArray, int numToFill,
// 				 const struct HSV& hsvColor)
// {
// 	fill_solid<RGB>( targetArray, numToFill, (RGB) hsvColor);
// }

void fill_rainbow(struct RGB* targetArray, int numToFill, uint8_t initialhue,
                  uint8_t deltahue) {
    HSV hsv;
    hsv.hue = initialhue;
    hsv.val = 255;
    hsv.sat = 240;

    for (int i = 0; i < numToFill; ++i) {
        targetArray[i] = hsv;
        // if(hsv.h == 0 || hsv.h==160) {
        //     printf("i: %d hsv: %d %d %d rgb: %d %d %d\n", i, hsv.hue, hsv.val,
        //            hsv.sat, targetArray[i].r, targetArray[i].g,
        //            targetArray[i].b);
        // }
        hsv.hue += deltahue;
    }
}

void fill_rainbow(struct HSV* targetArray, int numToFill, uint8_t initialhue,
                  uint8_t deltahue) {
    HSV hsv;
    hsv.hue = initialhue;
    hsv.val = 255;
    hsv.sat = 240;
    for (int i = 0; i < numToFill; ++i) {
        targetArray[i] = hsv;
        hsv.hue += deltahue;
    }
}

void fill_rainbow_circular(struct RGB* targetArray, int numToFill,
                           uint8_t initialhue, bool reversed) {
    if (numToFill == 0) return;  // avoiding div/0

    HSV hsv;
    hsv.hue = initialhue;
    hsv.val = 255;
    hsv.sat = 240;

    const uint16_t hueChange =
        65535 / (uint16_t)numToFill;  // hue change for each LED, * 256 for
                                      // precision (256 * 256 - 1)
    uint16_t hueOffset = 0;  // offset for hue value, with precision (*256)

    for (int i = 0; i < numToFill; ++i) {
        targetArray[i] = hsv;
        if (reversed)
            hueOffset -= hueChange;
        else
            hueOffset += hueChange;
        hsv.hue =
            initialhue +
            (uint8_t)(hueOffset >>
                      8);  // assign new hue with precise offset (as 8-bit)
    }
}

void fill_rainbow_circular(struct HSV* targetArray, int numToFill,
                           uint8_t initialhue, bool reversed) {
    if (numToFill == 0) return;  // avoiding div/0

    HSV hsv;
    hsv.hue = initialhue;
    hsv.val = 255;
    hsv.sat = 240;

    const uint16_t hueChange =
        65535 / (uint16_t)numToFill;  // hue change for each LED, * 256 for
                                      // precision (256 * 256 - 1)
    uint16_t hueOffset = 0;  // offset for hue value, with precision (*256)

    for (int i = 0; i < numToFill; ++i) {
        targetArray[i] = hsv;
        if (reversed)
            hueOffset -= hueChange;
        else
            hueOffset += hueChange;
        hsv.hue =
            initialhue +
            (uint8_t)(hueOffset >>
                      8);  // assign new hue with precise offset (as 8-bit)
    }
}

void fill_gradient_RGB(RGB* leds, uint16_t startpos, RGB startcolor,
                       uint16_t endpos, RGB endcolor) {
    // if the points are in the wrong order, straighten them
    if (endpos < startpos) {
        uint16_t t = endpos;
        RGB tc = endcolor;
        endcolor = startcolor;
        endpos = startpos;
        startpos = t;
        startcolor = tc;
    }

    saccum87 rdistance87;
    saccum87 gdistance87;
    saccum87 bdistance87;

    rdistance87 = (endcolor.r - startcolor.r) << 7;
    gdistance87 = (endcolor.g - startcolor.g) << 7;
    bdistance87 = (endcolor.b - startcolor.b) << 7;

    uint16_t pixeldistance = endpos - startpos;
    int16_t divisor = pixeldistance ? pixeldistance : 1;

    saccum87 rdelta87 = rdistance87 / divisor;
    saccum87 gdelta87 = gdistance87 / divisor;
    saccum87 bdelta87 = bdistance87 / divisor;

    rdelta87 *= 2;
    gdelta87 *= 2;
    bdelta87 *= 2;

    accum88 r88 = startcolor.r << 8;
    accum88 g88 = startcolor.g << 8;
    accum88 b88 = startcolor.b << 8;
    for (uint16_t i = startpos; i <= endpos; ++i) {
        leds[i] = RGB(r88 >> 8, g88 >> 8, b88 >> 8);
        r88 += rdelta87;
        g88 += gdelta87;
        b88 += bdelta87;
    }
}

void fill_gradient_RGB(RGB* leds, uint16_t numLeds, const RGB& c1,
                       const RGB& c2) {
    uint16_t last = numLeds - 1;
    fill_gradient_RGB(leds, 0, c1, last, c2);
}

void fill_gradient_RGB(RGB* leds, uint16_t numLeds, const RGB& c1,
                       const RGB& c2, const RGB& c3) {
    uint16_t half = (numLeds / 2);
    uint16_t last = numLeds - 1;
    fill_gradient_RGB(leds, 0, c1, half, c2);
    fill_gradient_RGB(leds, half, c2, last, c3);
}

void fill_gradient_RGB(RGB* leds, uint16_t numLeds, const RGB& c1,
                       const RGB& c2, const RGB& c3, const RGB& c4) {
    uint16_t onethird = (numLeds / 3);
    uint16_t twothirds = ((numLeds * 2) / 3);
    uint16_t last = numLeds - 1;
    fill_gradient_RGB(leds, 0, c1, onethird, c2);
    fill_gradient_RGB(leds, onethird, c2, twothirds, c3);
    fill_gradient_RGB(leds, twothirds, c3, last, c4);
}

void nscale8_video(RGB* leds, uint16_t num_leds, uint8_t scale) {
    for (uint16_t i = 0; i < num_leds; ++i) {
        leds[i].nscale8_video(scale);
    }
}

void fade_video(RGB* leds, uint16_t num_leds, uint8_t fadeBy) {
    nscale8_video(leds, num_leds, 255 - fadeBy);
}

void fadeLightBy(RGB* leds, uint16_t num_leds, uint8_t fadeBy) {
    nscale8_video(leds, num_leds, 255 - fadeBy);
}

void fadeToBlackBy(RGB* leds, uint16_t num_leds, uint8_t fadeBy) {
    nscale8(leds, num_leds, 255 - fadeBy);
}

void fade_raw(RGB* leds, uint16_t num_leds, uint8_t fadeBy) {
    nscale8(leds, num_leds, 255 - fadeBy);
}

/// Unused alias of nscale8(RGB*, uint16_t, uint8_t)
/// @todo Remove this or add a declaration? This is not listed in the
/// colorutils.h header.
void nscale8_raw(RGB* leds, uint16_t num_leds, uint8_t scale) {
    nscale8(leds, num_leds, scale);
}

void nscale8(RGB* leds, uint16_t num_leds, uint8_t scale) {
    for (uint16_t i = 0; i < num_leds; ++i) {
        leds[i].nscale8(scale);
    }
}

void fadeUsingColor(RGB* leds, uint16_t numLeds, const RGB& colormask) {
    uint8_t fr, fg, fb;
    fr = colormask.r;
    fg = colormask.g;
    fb = colormask.b;

    for (uint16_t i = 0; i < numLeds; ++i) {
        leds[i].r = scale8_LEAVING_R1_DIRTY(leds[i].r, fr);
        leds[i].g = scale8_LEAVING_R1_DIRTY(leds[i].g, fg);
        leds[i].b = scale8(leds[i].b, fb);
    }
}

// Map from one full-range 8-bit value into a narrower
/// range of 8-bit values, possibly a range of hues.
///
/// E.g. map `myValue` into a hue in the range blue..purple..pink..red
///   @code
///   hue = map8( myValue, HUE_BLUE, HUE_RED);
///   @endcode
///
/// Combines nicely with the waveform functions (like sin8(), etc)
/// to produce continuous hue gradients back and forth:
///   @code
///   hue = map8( sin8( myValue), HUE_BLUE, HUE_RED);
///   @endcode
///
/// Mathematically simiar to lerp8by8(), but arguments are more
/// like Arduino's "map"; this function is similar to
///   @code
///   map( in, 0, 255, rangeStart, rangeEnd)
///   @endcode
///
/// but faster and specifically designed for 8-bit values.
static inline uint8_t map8(uint8_t in, uint8_t rangeStart, uint8_t rangeEnd) {
    uint8_t rangeWidth = rangeEnd - rangeStart;
    uint8_t out = scale8(in, rangeWidth);
    out += rangeStart;
    return out;
}

static inline uint8_t blend8(uint8_t a, uint8_t b, uint8_t amountOfB) {
    // The BLEND_FIXED formula is
    //
    //   result = (  A*(amountOfA) + B*(amountOfB)              )/ 256
    //
    // …where amountOfA = 255-amountOfB.
    //
    // This formula will never return 255, which is why the BLEND_FIXED +
    // SCALE8_FIXED version is
    //
    //   result = (  A*(amountOfA) + A + B*(amountOfB) + B      ) / 256
    //
    // We can rearrange this formula for some great optimisations.
    //
    //   result = (  A*(amountOfA) + A + B*(amountOfB) + B      ) / 256
    //          = (  A*(255-amountOfB) + A + B*(amountOfB) + B  ) / 256
    //          = (  A*(256-amountOfB) + B*(amountOfB) + B      ) / 256
    //          = (  A*256 + B + B*(amountOfB) - A*(amountOfB)  ) / 256  // this
    //          is the version used in SCALE8_FIXED AVR below = (  A*256 + B +
    //          (B-A)*(amountOfB)              ) / 256  // this is the version
    //          used in SCALE8_FIXED C below

    uint16_t partial;
    uint8_t result;

    partial = (a << 8) | b;  // A*256 + B

    // on many platforms this compiles to a single multiply of (B-A) * amountOfB
    partial += (b * amountOfB);
    partial -= (a * amountOfB);

    result = partial >> 8;

    return result;
}

RGB& nblend(RGB& existing, const RGB& overlay, fract8 amountOfOverlay) {
    if (amountOfOverlay == 0) {
        return existing;
    }

    if (amountOfOverlay == 255) {
        existing = overlay;
        return existing;
    }

    existing.red = blend8(existing.red, overlay.red, amountOfOverlay);
    existing.green = blend8(existing.green, overlay.green, amountOfOverlay);
    existing.blue = blend8(existing.blue, overlay.blue, amountOfOverlay);
    return existing;
}

void nblend(RGB* existing, RGB* overlay, uint16_t count,
            fract8 amountOfOverlay) {
    for (uint16_t i = count; i; --i) {
        nblend(*existing, *overlay, amountOfOverlay);
        ++existing;
        ++overlay;
    }
}

RGB blend(const RGB& p1, const RGB& p2, fract8 amountOfP2) {
    RGB nu(p1);
    nblend(nu, p2, amountOfP2);
    return nu;
}

RGB* blend(const RGB* src1, const RGB* src2, RGB* dest, uint16_t count,
           fract8 amountOfsrc2) {
    for (uint16_t i = 0; i < count; ++i) {
        dest[i] = blend(src1[i], src2[i], amountOfsrc2);
    }
    return dest;
}

HSV& nblend(HSV& existing, const HSV& overlay, fract8 amountOfOverlay,
            TGradientDirectionCode directionCode) {
    if (amountOfOverlay == 0) {
        return existing;
    }

    if (amountOfOverlay == 255) {
        existing = overlay;
        return existing;
    }

    fract8 amountOfKeep = 255 - amountOfOverlay;

    uint8_t huedelta8 = overlay.hue - existing.hue;

    if (directionCode == SHORTEST_HUES) {
        directionCode = FORWARD_HUES;
        if (huedelta8 > 127) {
            directionCode = BACKWARD_HUES;
        }
    }

    if (directionCode == LONGEST_HUES) {
        directionCode = FORWARD_HUES;
        if (huedelta8 < 128) {
            directionCode = BACKWARD_HUES;
        }
    }

    if (directionCode == FORWARD_HUES) {
        existing.hue = existing.hue + scale8(huedelta8, amountOfOverlay);
    } else /* directionCode == BACKWARD_HUES */
    {
        huedelta8 = -huedelta8;
        existing.hue = existing.hue - scale8(huedelta8, amountOfOverlay);
    }

    existing.sat = scale8_LEAVING_R1_DIRTY(existing.sat, amountOfKeep) +
                   scale8_LEAVING_R1_DIRTY(overlay.sat, amountOfOverlay);
    existing.val = scale8_LEAVING_R1_DIRTY(existing.val, amountOfKeep) +
                   scale8_LEAVING_R1_DIRTY(overlay.val, amountOfOverlay);

    return existing;
}

void nblend(HSV* existing, HSV* overlay, uint16_t count, fract8 amountOfOverlay,
            TGradientDirectionCode directionCode) {
    if (existing == overlay) return;
    for (uint16_t i = count; i; --i) {
        nblend(*existing, *overlay, amountOfOverlay, directionCode);
        ++existing;
        ++overlay;
    }
}

HSV blend(const HSV& p1, const HSV& p2, fract8 amountOfP2,
          TGradientDirectionCode directionCode) {
    HSV nu(p1);
    nblend(nu, p2, amountOfP2, directionCode);
    return nu;
}

HSV* blend(const HSV* src1, const HSV* src2, HSV* dest, uint16_t count,
           fract8 amountOfsrc2, TGradientDirectionCode directionCode) {
    for (uint16_t i = 0; i < count; ++i) {
        dest[i] = blend(src1[i], src2[i], amountOfsrc2, directionCode);
    }
    return dest;
}

/// Forward declaration of the function "XY" which must be provided by
/// the application for use in two-dimensional filter functions.
uint16_t XY(uint8_t, uint8_t);  // __attribute__ ((weak));

// blur1d: one-dimensional blur filter. Spreads light to 2 line neighbors.
// blur2d: two-dimensional blur filter. Spreads light to 8 XY neighbors.
//
//           0 = no spread at all
//          64 = moderate spreading
//         172 = maximum smooth, even spreading
//
//         173..255 = wider spreading, but increasing flicker
//
//         Total light is NOT entirely conserved, so many repeated
//         calls to 'blur' will also result in the light fading,
//         eventually all the way to black; this is by design so that
//         it can be used to (slowly) clear the LEDs to black.
void blur1d(RGB* leds, uint16_t numLeds, fract8 blur_amount) {
    uint8_t keep = 255 - blur_amount;
    uint8_t seep = blur_amount >> 1;
    RGB carryover = RGB::Black;
    for (uint16_t i = 0; i < numLeds; ++i) {
        RGB cur = leds[i];
        RGB part = cur;
        part.nscale8(seep);
        cur.nscale8(keep);
        cur += carryover;
        if (i) leds[i - 1] += part;
        leds[i] = cur;
        carryover = part;
    }
}

void blur2d(RGB* leds, uint8_t width, uint8_t height, fract8 blur_amount) {
    blurRows(leds, width, height, blur_amount);
    blurColumns(leds, width, height, blur_amount);
}

void blurRows(RGB* leds, uint8_t width, uint8_t height, fract8 blur_amount) {
    /*    for( uint8_t row = 0; row < height; row++) {
            RGB* rowbase = leds + (row * width);
            blur1d( rowbase, width, blur_amount);
        }
    */
    // blur rows same as columns, for irregular matrix
    uint8_t keep = 255 - blur_amount;
    uint8_t seep = blur_amount >> 1;
    for (uint8_t row = 0; row < height; row++) {
        RGB carryover = RGB::Black;
        for (uint8_t i = 0; i < width; i++) {
            RGB cur = leds[XY(i, row)];
            RGB part = cur;
            part.nscale8(seep);
            cur.nscale8(keep);
            cur += carryover;
            if (i) leds[XY(i - 1, row)] += part;
            leds[XY(i, row)] = cur;
            carryover = part;
        }
    }
}

// blurColumns: perform a blur1d on each column of a rectangular matrix
void blurColumns(RGB* leds, uint8_t width, uint8_t height, fract8 blur_amount) {
    // blur columns
    uint8_t keep = 255 - blur_amount;
    uint8_t seep = blur_amount >> 1;
    for (uint8_t col = 0; col < width; ++col) {
        RGB carryover = RGB::Black;
        for (uint8_t i = 0; i < height; ++i) {
            RGB cur = leds[XY(col, i)];
            RGB part = cur;
            part.nscale8(seep);
            cur.nscale8(keep);
            cur += carryover;
            if (i) leds[XY(col, i - 1)] += part;
            leds[XY(col, i)] = cur;
            carryover = part;
        }
    }
}

// RGB HeatColor( uint8_t temperature)
//
// Approximates a 'black body radiation' spectrum for
// a given 'heat' level.  This is useful for animations of 'fire'.
// Heat is specified as an arbitrary scale from 0 (cool) to 255 (hot).
// This is NOT a chromatically correct 'black body radiation'
// spectrum, but it's surprisingly close, and it's fast and small.
//
// On AVR/Arduino, this typically takes around 70 bytes of program memory,
// versus 768 bytes for a full 256-entry RGB lookup table.

RGB HeatColor(uint8_t temperature) {
    RGB heatcolor;

    // Scale 'heat' down from 0-255 to 0-191,
    // which can then be easily divided into three
    // equal 'thirds' of 64 units each.
    uint8_t t192 = scale8_video(temperature, 191);

    // calculate a value that ramps up from
    // zero to 255 in each 'third' of the scale.
    uint8_t heatramp = t192 & 0x3F;  // 0..63
    heatramp <<= 2;                  // scale up to 0..252

    // now figure out which third of the spectrum we're in:
    if (t192 & 0x80) {
        // we're in the hottest third
        heatcolor.r = 255;       // full red
        heatcolor.g = 255;       // full green
        heatcolor.b = heatramp;  // ramp up blue

    } else if (t192 & 0x40) {
        // we're in the middle third
        heatcolor.r = 255;       // full red
        heatcolor.g = heatramp;  // ramp up green
        heatcolor.b = 0;         // no blue

    } else {
        // we're in the coolest third
        heatcolor.r = heatramp;  // ramp up red
        heatcolor.g = 0;         // no green
        heatcolor.b = 0;         // no blue
    }

    return heatcolor;
}

/// Helper function to divide a number by 16, aka four logical shift right
/// (LSR)'s. On avr-gcc, "u8 >> 4" generates a loop, which is big, and slow.
/// merely forcing it to be four /=2's causes avr-gcc to emit
/// a SWAP instruction followed by an AND 0x0F, which is faster, and smaller.
inline uint8_t lsrX4(uint8_t dividend) __attribute__((always_inline));
inline uint8_t lsrX4(uint8_t dividend) {
    dividend >>= 4;
    return dividend;
}

RGB ColorFromPalette(const RGBPalette16& pal, uint8_t index, uint8_t brightness,
                     TBlendType blendType) {
    if (blendType == LINEARBLEND_NOWRAP) {
        index = map8(index, 0, 239);  // Blend range is affected by lo4 blend of
                                      // values, remap to avoid wrapping
    }

    //      hi4 = index >> 4;
    uint8_t hi4 = lsrX4(index);
    uint8_t lo4 = index & 0x0F;

    // const RGB* entry = &(pal[0]) + hi4;
    // since hi4 is always 0..15, hi4 * sizeof(RGB) can be a single-byte value,
    // instead of the two byte 'int' that avr-gcc defaults to.
    // So, we multiply hi4 X sizeof(RGB), giving hi4XsizeofRGB;
    uint8_t hi4XsizeofRGB = hi4 * sizeof(RGB);
    // We then add that to a base array pointer.
    const RGB* entry = (RGB*)((uint8_t*)(&(pal[0])) + hi4XsizeofRGB);

    uint8_t blend = lo4 && (blendType != NOBLEND);

    uint8_t red1 = entry->red;
    uint8_t green1 = entry->green;
    uint8_t blue1 = entry->blue;

    if (blend) {
        if (hi4 == 15) {
            entry = &(pal[0]);
        } else {
            ++entry;
        }

        uint8_t f2 = lo4 << 4;
        uint8_t f1 = 255 - f2;

        //    rgb1.nscale8(f1);
        uint8_t red2 = entry->red;
        red1 = scale8_LEAVING_R1_DIRTY(red1, f1);
        red2 = scale8_LEAVING_R1_DIRTY(red2, f2);
        red1 += red2;

        uint8_t green2 = entry->green;
        green1 = scale8_LEAVING_R1_DIRTY(green1, f1);
        green2 = scale8_LEAVING_R1_DIRTY(green2, f2);
        green1 += green2;

        uint8_t blue2 = entry->blue;
        blue1 = scale8_LEAVING_R1_DIRTY(blue1, f1);
        blue2 = scale8_LEAVING_R1_DIRTY(blue2, f2);
        blue1 += blue2;
    }

    if (brightness != 255) {
        if (brightness) {
            ++brightness;  // adjust for rounding
            // Now, since brightness is nonzero, we don't need the full
            // scale8_video logic; we can just to scale8 and then add one
            // (unless scale8 fixed) to all nonzero inputs.
            if (red1) {
                red1 = scale8_LEAVING_R1_DIRTY(red1, brightness);
            }
            if (green1) {
                green1 = scale8_LEAVING_R1_DIRTY(green1, brightness);
            }
            if (blue1) {
                blue1 = scale8_LEAVING_R1_DIRTY(blue1, brightness);
            }
        } else {
            red1 = 0;
            green1 = 0;
            blue1 = 0;
        }
    }

    return RGB(red1, green1, blue1);
}

RGB ColorFromPalette(const RGBPalette32& pal, uint8_t index, uint8_t brightness,
                     TBlendType blendType) {
    if (blendType == LINEARBLEND_NOWRAP) {
        index = map8(index, 0, 247);  // Blend range is affected by lo3 blend of
                                      // values, remap to avoid wrapping
    }

    uint8_t hi5 = index;
    hi5 >>= 3;
    uint8_t lo3 = index & 0x07;

    // const RGB* entry = &(pal[0]) + hi5;
    // since hi5 is always 0..31, hi4 * sizeof(RGB) can be a single-byte value,
    // instead of the two byte 'int' that avr-gcc defaults to.
    // So, we multiply hi5 X sizeof(RGB), giving hi5XsizeofRGB;
    uint8_t hi5XsizeofRGB = hi5 * sizeof(RGB);
    // We then add that to a base array pointer.
    const RGB* entry = (RGB*)((uint8_t*)(&(pal[0])) + hi5XsizeofRGB);

    uint8_t red1 = entry->red;
    uint8_t green1 = entry->green;
    uint8_t blue1 = entry->blue;

    uint8_t blend = lo3 && (blendType != NOBLEND);

    if (blend) {
        if (hi5 == 31) {
            entry = &(pal[0]);
        } else {
            ++entry;
        }

        uint8_t f2 = lo3 << 5;
        uint8_t f1 = 255 - f2;

        uint8_t red2 = entry->red;
        red1 = scale8_LEAVING_R1_DIRTY(red1, f1);
        red2 = scale8_LEAVING_R1_DIRTY(red2, f2);
        red1 += red2;

        uint8_t green2 = entry->green;
        green1 = scale8_LEAVING_R1_DIRTY(green1, f1);
        green2 = scale8_LEAVING_R1_DIRTY(green2, f2);
        green1 += green2;

        uint8_t blue2 = entry->blue;
        blue1 = scale8_LEAVING_R1_DIRTY(blue1, f1);
        blue2 = scale8_LEAVING_R1_DIRTY(blue2, f2);
        blue1 += blue2;
    }

    if (brightness != 255) {
        if (brightness) {
            ++brightness;  // adjust for rounding
            // Now, since brightness is nonzero, we don't need the full
            // scale8_video logic; we can just to scale8 and then add one
            // (unless scale8 fixed) to all nonzero inputs.
            if (red1) {
                red1 = scale8_LEAVING_R1_DIRTY(red1, brightness);
            }
            if (green1) {
                green1 = scale8_LEAVING_R1_DIRTY(green1, brightness);
            }
            if (blue1) {
                blue1 = scale8_LEAVING_R1_DIRTY(blue1, brightness);
            }
        } else {
            red1 = 0;
            green1 = 0;
            blue1 = 0;
        }
    }

    return RGB(red1, green1, blue1);
}

RGB ColorFromPalette(const RGBPalette256& pal, uint8_t index,
                     uint8_t brightness, TBlendType) {
    const RGB* entry = &(pal[0]) + index;

    uint8_t red = entry->red;
    uint8_t green = entry->green;
    uint8_t blue = entry->blue;

    if (brightness != 255) {
        ++brightness;  // adjust for rounding
        red = scale8_video(red, brightness);
        green = scale8_video(green, brightness);
        blue = scale8_video(blue, brightness);
    }

    return RGB(red, green, blue);
}

HSV ColorFromPalette(const HSVPalette16& pal, uint8_t index, uint8_t brightness,
                     TBlendType blendType) {
    if (blendType == LINEARBLEND_NOWRAP) {
        index = map8(index, 0, 239);  // Blend range is affected by lo4 blend of
                                      // values, remap to avoid wrapping
    }

    //      hi4 = index >> 4;
    uint8_t hi4 = lsrX4(index);
    uint8_t lo4 = index & 0x0F;

    //  RGB rgb1 = pal[ hi4];
    const HSV* entry = &(pal[0]) + hi4;

    uint8_t hue1 = entry->hue;
    uint8_t sat1 = entry->sat;
    uint8_t val1 = entry->val;

    uint8_t blend = lo4 && (blendType != NOBLEND);

    if (blend) {
        if (hi4 == 15) {
            entry = &(pal[0]);
        } else {
            ++entry;
        }

        uint8_t f2 = lo4 << 4;
        uint8_t f1 = 255 - f2;

        uint8_t hue2 = entry->hue;
        uint8_t sat2 = entry->sat;
        uint8_t val2 = entry->val;

        // Now some special casing for blending to or from
        // either black or white.  Black and white don't have
        // proper 'hue' of their own, so when ramping from
        // something else to/from black/white, we set the 'hue'
        // of the black/white color to be the same as the hue
        // of the other color, so that you get the expected
        // brightness or saturation ramp, with hue staying
        // constant:

        // If we are starting from white (sat=0)
        // or black (val=0), adopt the target hue.
        if (sat1 == 0 || val1 == 0) {
            hue1 = hue2;
        }

        // If we are ending at white (sat=0)
        // or black (val=0), adopt the starting hue.
        if (sat2 == 0 || val2 == 0) {
            hue2 = hue1;
        }

        sat1 = scale8_LEAVING_R1_DIRTY(sat1, f1);
        val1 = scale8_LEAVING_R1_DIRTY(val1, f1);

        sat2 = scale8_LEAVING_R1_DIRTY(sat2, f2);
        val2 = scale8_LEAVING_R1_DIRTY(val2, f2);

        //    cleanup_R1();

        // These sums can't overflow, so no qadd8 needed.
        sat1 += sat2;
        val1 += val2;

        uint8_t deltaHue = (uint8_t)(hue2 - hue1);
        if (deltaHue & 0x80) {
            // go backwards
            hue1 -= scale8(256 - deltaHue, f2);
        } else {
            // go forwards
            hue1 += scale8(deltaHue, f2);
        }
    }

    if (brightness != 255) {
        val1 = scale8_video(val1, brightness);
    }

    return HSV(hue1, sat1, val1);
}

HSV ColorFromPalette(const HSVPalette32& pal, uint8_t index, uint8_t brightness,
                     TBlendType blendType) {
    if (blendType == LINEARBLEND_NOWRAP) {
        index = map8(index, 0, 247);  // Blend range is affected by lo3 blend of
                                      // values, remap to avoid wrapping
    }

    uint8_t hi5 = index;
    hi5 >>= 3;
    uint8_t lo3 = index & 0x07;

    uint8_t hi5XsizeofHSV = hi5 * sizeof(HSV);
    const HSV* entry = (HSV*)((uint8_t*)(&(pal[0])) + hi5XsizeofHSV);

    uint8_t hue1 = entry->hue;
    uint8_t sat1 = entry->sat;
    uint8_t val1 = entry->val;

    uint8_t blend = lo3 && (blendType != NOBLEND);

    if (blend) {
        if (hi5 == 31) {
            entry = &(pal[0]);
        } else {
            ++entry;
        }

        uint8_t f2 = lo3 << 5;
        uint8_t f1 = 255 - f2;

        uint8_t hue2 = entry->hue;
        uint8_t sat2 = entry->sat;
        uint8_t val2 = entry->val;

        // Now some special casing for blending to or from
        // either black or white.  Black and white don't have
        // proper 'hue' of their own, so when ramping from
        // something else to/from black/white, we set the 'hue'
        // of the black/white color to be the same as the hue
        // of the other color, so that you get the expected
        // brightness or saturation ramp, with hue staying
        // constant:

        // If we are starting from white (sat=0)
        // or black (val=0), adopt the target hue.
        if (sat1 == 0 || val1 == 0) {
            hue1 = hue2;
        }

        // If we are ending at white (sat=0)
        // or black (val=0), adopt the starting hue.
        if (sat2 == 0 || val2 == 0) {
            hue2 = hue1;
        }

        sat1 = scale8_LEAVING_R1_DIRTY(sat1, f1);
        val1 = scale8_LEAVING_R1_DIRTY(val1, f1);

        sat2 = scale8_LEAVING_R1_DIRTY(sat2, f2);
        val2 = scale8_LEAVING_R1_DIRTY(val2, f2);

        //    cleanup_R1();

        // These sums can't overflow, so no qadd8 needed.
        sat1 += sat2;
        val1 += val2;

        uint8_t deltaHue = (uint8_t)(hue2 - hue1);
        if (deltaHue & 0x80) {
            // go backwards
            hue1 -= scale8(256 - deltaHue, f2);
        } else {
            // go forwards
            hue1 += scale8(deltaHue, f2);
        }
    }

    if (brightness != 255) {
        val1 = scale8_video(val1, brightness);
    }

    return HSV(hue1, sat1, val1);
}

HSV ColorFromPalette(const HSVPalette256& pal, uint8_t index,
                     uint8_t brightness, TBlendType) {
    HSV hsv = *(&(pal[0]) + index);

    if (brightness != 255) {
        hsv.value = scale8_video(hsv.value, brightness);
    }

    return hsv;
}

void UpscalePalette(const struct RGBPalette16& srcpal16,
                    struct RGBPalette256& destpal256) {
    for (int i = 0; i < 256; ++i) {
        destpal256[(uint8_t)(i)] = ColorFromPalette(srcpal16, i);
    }
}

void UpscalePalette(const struct HSVPalette16& srcpal16,
                    struct HSVPalette256& destpal256) {
    for (int i = 0; i < 256; ++i) {
        destpal256[(uint8_t)(i)] = ColorFromPalette(srcpal16, i);
    }
}

void UpscalePalette(const struct RGBPalette16& srcpal16,
                    struct RGBPalette32& destpal32) {
    for (uint8_t i = 0; i < 16; ++i) {
        uint8_t j = i * 2;
        destpal32[j + 0] = srcpal16[i];
        destpal32[j + 1] = srcpal16[i];
    }
}

void UpscalePalette(const struct HSVPalette16& srcpal16,
                    struct HSVPalette32& destpal32) {
    for (uint8_t i = 0; i < 16; ++i) {
        uint8_t j = i * 2;
        destpal32[j + 0] = srcpal16[i];
        destpal32[j + 1] = srcpal16[i];
    }
}

void UpscalePalette(const struct RGBPalette32& srcpal32,
                    struct RGBPalette256& destpal256) {
    for (int i = 0; i < 256; ++i) {
        destpal256[(uint8_t)(i)] = ColorFromPalette(srcpal32, i);
    }
}

void UpscalePalette(const struct HSVPalette32& srcpal32,
                    struct HSVPalette256& destpal256) {
    for (int i = 0; i < 256; ++i) {
        destpal256[(uint8_t)(i)] = ColorFromPalette(srcpal32, i);
    }
}

void nblendPaletteTowardPalette(RGBPalette16& current, RGBPalette16& target,
                                uint8_t maxChanges) {
    uint8_t* p1;
    uint8_t* p2;
    uint8_t changes = 0;

    p1 = (uint8_t*)current.entries;
    p2 = (uint8_t*)target.entries;

    const uint8_t totalChannels = sizeof(RGBPalette16);
    for (uint8_t i = 0; i < totalChannels; ++i) {
        // if the values are equal, no changes are needed
        if (p1[i] == p2[i]) {
            continue;
        }

        // if the current value is less than the target, increase it by one
        if (p1[i] < p2[i]) {
            ++p1[i];
            ++changes;
        }

        // if the current value is greater than the target,
        // increase it by one (or two if it's still greater).
        if (p1[i] > p2[i]) {
            --p1[i];
            ++changes;
            if (p1[i] > p2[i]) {
                --p1[i];
            }
        }

        // if we've hit the maximum number of changes, exit
        if (changes >= maxChanges) {
            break;
        }
    }
}

uint8_t applyGamma_video(uint8_t brightness, float gamma) {
    float orig;
    float adj;
    orig = (float)(brightness) / (255.0);
    adj = pow(orig, gamma) * (255.0);
    uint8_t result = (uint8_t)(adj);
    if ((brightness > 0) && (result == 0)) {
        result = 1;  // never gamma-adjust a positive number down to zero
    }
    return result;
}

RGB applyGamma_video(const RGB& orig, float gamma) {
    RGB adj;
    adj.r = applyGamma_video(orig.r, gamma);
    adj.g = applyGamma_video(orig.g, gamma);
    adj.b = applyGamma_video(orig.b, gamma);
    return adj;
}

RGB applyGamma_video(const RGB& orig, float gammaR, float gammaG,
                     float gammaB) {
    RGB adj;
    adj.r = applyGamma_video(orig.r, gammaR);
    adj.g = applyGamma_video(orig.g, gammaG);
    adj.b = applyGamma_video(orig.b, gammaB);
    return adj;
}

RGB& napplyGamma_video(RGB& rgb, float gamma) {
    rgb = applyGamma_video(rgb, gamma);
    return rgb;
}

RGB& napplyGamma_video(RGB& rgb, float gammaR, float gammaG, float gammaB) {
    rgb = applyGamma_video(rgb, gammaR, gammaG, gammaB);
    return rgb;
}

void napplyGamma_video(RGB* rgbarray, uint16_t count, float gamma) {
    for (uint16_t i = 0; i < count; ++i) {
        rgbarray[i] = applyGamma_video(rgbarray[i], gamma);
    }
}

void napplyGamma_video(RGB* rgbarray, uint16_t count, float gammaR,
                       float gammaG, float gammaB) {
    for (uint16_t i = 0; i < count; ++i) {
        rgbarray[i] = applyGamma_video(rgbarray[i], gammaR, gammaG, gammaB);
    }
}
