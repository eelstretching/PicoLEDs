#ifndef COLORUTILS_H
#define COLORUTILS_H

#include <stdlib.h>
#include <string.h>

#include "color.h"
#include "pico/types.h"
#include "pico/mem_ops.h"

typedef uint8_t fract8;

/// @file colorutils.h
/// Utility functions for color fill, palettes, blending, and more

/// @defgroup ColorUtils Color Utility Functions
/// A variety of functions for working with color, palettes, and leds
/// @{

/// @defgroup ColorFills Color Fill Functions
/// Functions for filling LED arrays with colors and gradients
/// @{

/// Fill a range of LEDs with a solid color.
/// @param targetArray a pointer to the LED array to fill
/// @param numToFill the number of LEDs to fill in the array
/// @param color the color to fill with
void fill_solid(struct RGB* targetArray, int numToFill,
                const struct RGB& color);

/// @copydoc fill_solid()
void fill_solid(struct HSV* targetArray, int numToFill,
                const struct HSV& color);

/// Fill a range of LEDs with a rainbow of colors.
/// The colors making up the rainbow are at full saturation and full
/// value (brightness).
/// @param targetArray a pointer to the LED array to fill
/// @param numToFill the number of LEDs to fill in the array
/// @param initialhue the starting hue for the rainbow
/// @param deltahue how many hue values to advance for each LED
void fill_rainbow(struct RGB* targetArray, int numToFill, uint8_t initialhue,
                  uint8_t deltahue = 5);

/// @copydoc fill_rainbow()
void fill_rainbow(struct HSV* targetArray, int numToFill, uint8_t initialhue,
                  uint8_t deltahue = 5);

/// Fill a range of LEDs with a rainbow of colors, so that the hues
/// are continuous between the end of the strip and the beginning.
/// The colors making up the rainbow are at full saturation and full
/// value (brightness).
/// @param targetArray a pointer to the LED array to fill
/// @param numToFill the number of LEDs to fill in the array
/// @param initialhue the starting hue for the rainbow
/// @param reversed whether to progress through the rainbow hues backwards
void fill_rainbow_circular(struct RGB* targetArray, int numToFill,
                           uint8_t initialhue, bool reversed = false);

/// @copydoc fill_rainbow_circular()
void fill_rainbow_circular(struct HSV* targetArray, int numToFill,
                           uint8_t initialhue, bool reversed = false);

/// Hue direction for calculating fill gradients.
/// Since "hue" is a value around a color wheel, there are always two directions
/// to sweep from one hue to another.
typedef enum {
    FORWARD_HUES,   ///< Hue always goes clockwise around the color wheel
    BACKWARD_HUES,  ///< Hue always goes counter-clockwise around the color
                    ///< wheel
    SHORTEST_HUES,  ///< Hue goes whichever way is shortest
    LONGEST_HUES    ///< Hue goes whichever way is longest
} TGradientDirectionCode;

/// ANSI: signed short _Accum.  8 bits int, 7 bits fraction
/// @see accum88
typedef int16_t saccum87;
typedef uint16_t
    accum88;  ///< ANSI: unsigned short _Accum. 8 bits int, 8 bits fraction
typedef int16_t
    saccum78;  ///< ANSI: signed   short _Accum. 7 bits int, 8 bits fraction
typedef uint32_t
    accum1616;  ///< ANSI: signed         _Accum. 16 bits int, 16 bits fraction
typedef int32_t
    saccum1516;  ///< ANSI: signed         _Accum. 15 bits int, 16 bits fraction
typedef uint16_t
    accum124;  ///< no direct ANSI counterpart. 12 bits int, 4 bits fraction
typedef int32_t
    saccum114;  ///< no direct ANSI counterpart. 1 bit int, 14 bits fraction

/// Fill a range of LEDs with a smooth HSV gradient between two HSV colors.
/// This function can write the gradient colors either:
///
///   1. Into an array of RGBs (e.g., an leds[] array, or a RGB palette)
///   2. Into an array of HSVs (e.g. a HSV palette).
///
/// In the case of writing into a RGB array, the gradient is
/// computed in HSV space, and then HSV values are converted to RGB
/// as they're written into the RGB array.
/// @param targetArray a pointer to the color array to fill
/// @param startpos the starting position in the array
/// @param startcolor the starting color for the gradient
/// @param endpos the ending position in the array
/// @param endcolor the end color for the gradient
/// @param directionCode the direction to travel around the color wheel
template <typename T>
void fill_gradient(T* targetArray, uint16_t startpos, HSV startcolor,
                   uint16_t endpos, HSV endcolor,
                   TGradientDirectionCode directionCode = SHORTEST_HUES) {
    // if the points are in the wrong order, straighten them
    if (endpos < startpos) {
        uint16_t t = endpos;
        HSV tc = endcolor;
        endcolor = startcolor;
        endpos = startpos;
        startpos = t;
        startcolor = tc;
    }

    // If we're fading toward black (val=0) or white (sat=0),
    // then set the endhue to the starthue.
    // This lets us ramp smoothly to black or white, regardless
    // of what 'hue' was set in the endcolor (since it doesn't matter)
    if (endcolor.value == 0 || endcolor.saturation == 0) {
        endcolor.hue = startcolor.hue;
    }

    // Similarly, if we're fading in from black (val=0) or white (sat=0)
    // then set the starthue to the endhue.
    // This lets us ramp smoothly up from black or white, regardless
    // of what 'hue' was set in the startcolor (since it doesn't matter)
    if (startcolor.value == 0 || startcolor.saturation == 0) {
        startcolor.hue = endcolor.hue;
    }

    saccum87 huedistance87;
    saccum87 satdistance87;
    saccum87 valdistance87;

    satdistance87 = (endcolor.sat - startcolor.sat) << 7;
    valdistance87 = (endcolor.val - startcolor.val) << 7;

    uint8_t huedelta8 = endcolor.hue - startcolor.hue;

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
        huedistance87 = huedelta8 << 7;
    } else /* directionCode == BACKWARD_HUES */
    {
        huedistance87 = (uint8_t)(256 - huedelta8) << 7;
        huedistance87 = -huedistance87;
    }

    uint16_t pixeldistance = endpos - startpos;
    int16_t divisor = pixeldistance ? pixeldistance : 1;

    saccum87 huedelta87 = huedistance87 / divisor;
    saccum87 satdelta87 = satdistance87 / divisor;
    saccum87 valdelta87 = valdistance87 / divisor;

    huedelta87 *= 2;
    satdelta87 *= 2;
    valdelta87 *= 2;

    accum88 hue88 = startcolor.hue << 8;
    accum88 sat88 = startcolor.sat << 8;
    accum88 val88 = startcolor.val << 8;
    for (uint16_t i = startpos; i <= endpos; ++i) {
        targetArray[i] = HSV(hue88 >> 8, sat88 >> 8, val88 >> 8);
        hue88 += huedelta87;
        sat88 += satdelta87;
        val88 += valdelta87;
    }
}

/// Fill a range of LEDs with a smooth HSV gradient between two HSV colors.
/// @see fill_gradient()
/// @param targetArray a pointer to the color array to fill
/// @param numLeds the number of LEDs to fill
/// @param c1 the starting color in the gradient
/// @param c2 the end color for the gradient
/// @param directionCode the direction to travel around the color wheel
template <typename T>
void fill_gradient(T* targetArray, uint16_t numLeds, const HSV& c1,
                   const HSV& c2,
                   TGradientDirectionCode directionCode = SHORTEST_HUES) {
    uint16_t last = numLeds - 1;
    fill_gradient(targetArray, 0, c1, last, c2, directionCode);
}

/// Fill a range of LEDs with a smooth HSV gradient between three HSV colors.
/// @see fill_gradient()
/// @param targetArray a pointer to the color array to fill
/// @param numLeds the number of LEDs to fill
/// @param c1 the starting color in the gradient
/// @param c2 the middle color for the gradient
/// @param c3 the end color for the gradient
/// @param directionCode the direction to travel around the color wheel
template <typename T>
void fill_gradient(T* targetArray, uint16_t numLeds, const HSV& c1,
                   const HSV& c2, const HSV& c3,
                   TGradientDirectionCode directionCode = SHORTEST_HUES) {
    uint16_t half = (numLeds / 2);
    uint16_t last = numLeds - 1;
    fill_gradient(targetArray, 0, c1, half, c2, directionCode);
    fill_gradient(targetArray, half, c2, last, c3, directionCode);
}

/// Fill a range of LEDs with a smooth HSV gradient between four HSV colors.
/// @see fill_gradient()
/// @param targetArray a pointer to the color array to fill
/// @param numLeds the number of LEDs to fill
/// @param c1 the starting color in the gradient
/// @param c2 the first middle color for the gradient
/// @param c3 the second middle color for the gradient
/// @param c4 the end color for the gradient
/// @param directionCode the direction to travel around the color wheel
template <typename T>
void fill_gradient(T* targetArray, uint16_t numLeds, const HSV& c1,
                   const HSV& c2, const HSV& c3, const HSV& c4,
                   TGradientDirectionCode directionCode = SHORTEST_HUES) {
    uint16_t onethird = (numLeds / 3);
    uint16_t twothirds = ((numLeds * 2) / 3);
    uint16_t last = numLeds - 1;
    fill_gradient(targetArray, 0, c1, onethird, c2, directionCode);
    fill_gradient(targetArray, onethird, c2, twothirds, c3, directionCode);
    fill_gradient(targetArray, twothirds, c3, last, c4, directionCode);
}

/// Convenience synonym
#define fill_gradient_HSV fill_gradient

/// Fill a range of LEDs with a smooth RGB gradient between two RGB colors.
/// Unlike HSV, there is no "color wheel" in RGB space, and therefore there's
/// only one "direction" for the gradient to go. This means there's no
/// TGradientDirectionCode parameter for direction.
/// @param leds a pointer to the LED array to fill
/// @param startpos the starting position in the array
/// @param startcolor the starting color for the gradient
/// @param endpos the ending position in the array
/// @param endcolor the end color for the gradient
void fill_gradient_RGB(RGB* leds, uint16_t startpos, RGB startcolor,
                       uint16_t endpos, RGB endcolor);

/// Fill a range of LEDs with a smooth RGB gradient between two RGB colors.
/// @see fill_gradient_RGB()
/// @param leds a pointer to the LED array to fill
/// @param numLeds the number of LEDs to fill
/// @param c1 the starting color in the gradient
/// @param c2 the end color for the gradient
void fill_gradient_RGB(RGB* leds, uint16_t numLeds, const RGB& c1,
                       const RGB& c2);

/// Fill a range of LEDs with a smooth RGB gradient between three RGB colors.
/// @see fill_gradient_RGB()
/// @param leds a pointer to the LED array to fill
/// @param numLeds the number of LEDs to fill
/// @param c1 the starting color in the gradient
/// @param c2 the middle color for the gradient
/// @param c3 the end color for the gradient
void fill_gradient_RGB(RGB* leds, uint16_t numLeds, const RGB& c1,
                       const RGB& c2, const RGB& c3);

/// Fill a range of LEDs with a smooth RGB gradient between four RGB colors.
/// @see fill_gradient_RGB()
/// @param leds a pointer to the LED array to fill
/// @param numLeds the number of LEDs to fill
/// @param c1 the starting color in the gradient
/// @param c2 the first middle color for the gradient
/// @param c3 the second middle color for the gradient
/// @param c4 the end color for the gradient
void fill_gradient_RGB(RGB* leds, uint16_t numLeds, const RGB& c1,
                       const RGB& c2, const RGB& c3, const RGB& c4);

/// @} ColorFills

/// @defgroup ColorFades Color Fade Functions
/// Functions for fading LED arrays
/// @{

/// Reduce the brightness of an array of pixels all at once.
/// Guaranteed to never fade all the way to black.
/// @param leds a pointer to the LED array to fade
/// @param num_leds the number of LEDs to fade
/// @param fadeBy how much to fade each LED
void fadeLightBy(RGB* leds, uint16_t num_leds, uint8_t fadeBy);

/// @copydoc fadeLightBy()
void fade_video(RGB* leds, uint16_t num_leds, uint8_t fadeBy);

/// Scale the brightness of an array of pixels all at once.
/// Guaranteed to never fade all the way to black.
/// @param leds a pointer to the LED array to scale
/// @param num_leds the number of LEDs to scale
/// @param scale how much to scale each LED
void nscale8_video(RGB* leds, uint16_t num_leds, uint8_t scale);

/// Reduce the brightness of an array of pixels all at once.
/// This function will eventually fade all the way to black.
/// @param leds a pointer to the LED array to fade
/// @param num_leds the number of LEDs to fade
/// @param fadeBy how much to fade each LED
void fadeToBlackBy(RGB* leds, uint16_t num_leds, uint8_t fadeBy);

/// @copydoc fadeToBlackBy()
void fade_raw(RGB* leds, uint16_t num_leds, uint8_t fadeBy);

/// Scale the brightness of an array of pixels all at once.
/// This function will eventually fade all the way to black, even
/// if "scale" is not zero.
/// @param leds a pointer to the LED array to scale
/// @param num_leds the number of LEDs to scale
/// @param scale how much to scale each LED
void nscale8(RGB* leds, uint16_t num_leds, uint8_t scale);

/// Reduce the brightness of an array of pixels as thought it were seen through
/// a transparent filter with the specified color.
/// For example, if the colormask if RGB(200, 100, 50), then the pixels' red
/// will be faded to 200/256ths, their green to 100/256ths, and their blue to
/// 50/256ths. This particular example will give a "hot fade" look, with white
/// fading to yellow, then red, then black. You can also use colormasks like
/// RGB::Blue to zero out the red and green elements, leaving blue (largely) the
/// same.
/// @param leds a pointer to the LED array to fade
/// @param numLeds the number of LEDs to fade
/// @param colormask the color mask to fade with
void fadeUsingColor(RGB* leds, uint16_t numLeds, const RGB& colormask);

/// @} ColorFades

/// @defgroup ColorBlends Color Blending Functions
/// Functions for blending colors together
/// @{

/// Computes a new color blended some fraction of the way between two other
/// colors.
/// @param p1 the first color to blend
/// @param p2 the second color to blend
/// @param amountOfP2 the fraction of p2 to blend into p1
RGB blend(const RGB& p1, const RGB& p2, fract8 amountOfP2);

/// @copydoc blend(const RGB&, const RGB&, fract8)
/// @param directionCode the direction to travel around the color wheel
HSV blend(const HSV& p1, const HSV& p2, fract8 amountOfP2,
          TGradientDirectionCode directionCode = SHORTEST_HUES);

/// Computes a new blended array of colors, each some fraction of the way
/// between corresponding elements of two source arrays of colors. Useful for
/// blending palettes.
/// @param src1 the first array of colors to blend
/// @param src2 the second array of colors to blend
/// @param dest the destination array for the colors
/// @param count the number of LEDs to blend
/// @param amountOfsrc2 the fraction of src2 to blend into src1
RGB* blend(const RGB* src1, const RGB* src2, RGB* dest, uint16_t count,
           fract8 amountOfsrc2);

/// @copydoc blend(const RGB*, const RGB*, RGB*, uint16_t, fract8)
/// @param directionCode the direction to travel around the color wheel
HSV* blend(const HSV* src1, const HSV* src2, HSV* dest, uint16_t count,
           fract8 amountOfsrc2,
           TGradientDirectionCode directionCode = SHORTEST_HUES);

/// Destructively modifies one color, blending in a given fraction of an overlay
/// color
/// @param existing the color to modify
/// @param overlay the color to blend into existing
/// @param amountOfOverlay the fraction of overlay to blend into existing
RGB& nblend(RGB& existing, const RGB& overlay, fract8 amountOfOverlay);

/// @copydoc nblend(RGB&, const RGB&, fract8)
/// @param directionCode the direction to travel around the color wheel
HSV& nblend(HSV& existing, const HSV& overlay, fract8 amountOfOverlay,
            TGradientDirectionCode directionCode = SHORTEST_HUES);

/// Destructively blends a given fraction of a color array into an existing
/// color array
/// @param existing the color array to modify
/// @param overlay the color array to blend into existing
/// @param count the number of colors to process
/// @param amountOfOverlay the fraction of overlay to blend into existing
void nblend(RGB* existing, RGB* overlay, uint16_t count,
            fract8 amountOfOverlay);

/// @copydoc nblend(RGB*, RGB*, uint16_t, fract8)
/// @param directionCode the direction to travel around the color wheel
void nblend(HSV* existing, HSV* overlay, uint16_t count, fract8 amountOfOverlay,
            TGradientDirectionCode directionCode = SHORTEST_HUES);

/// @} ColorBlends

/// @defgroup ColorBlurs Color Blurring Functions
/// Functions for blurring colors
/// @{

/// One-dimensional blur filter.
/// Spreads light to 2 line neighbors.
///   * 0 = no spread at all
///   * 64 = moderate spreading
///   * 172 = maximum smooth, even spreading
///   * 173..255 = wider spreading, but increasing flicker
///
/// Total light is NOT entirely conserved, so many repeated
/// calls to 'blur' will also result in the light fading,
/// eventually all the way to black; this is by design so that
/// it can be used to (slowly) clear the LEDs to black.
/// @param leds a pointer to the LED array to blur
/// @param numLeds the number of LEDs to blur
/// @param blur_amount the amount of blur to apply
void blur1d(RGB* leds, uint16_t numLeds, fract8 blur_amount);

/// Two-dimensional blur filter.
/// Spreads light to 8 XY neighbors.
///   * 0 = no spread at all
///   * 64 = moderate spreading
///   * 172 = maximum smooth, even spreading
///   * 173..255 = wider spreading, but increasing flicker
///
/// Total light is NOT entirely conserved, so many repeated
/// calls to 'blur' will also result in the light fading,
/// eventually all the way to black; this is by design so that
/// it can be used to (slowly) clear the LEDs to black.
/// @param leds a pointer to the LED array to blur
/// @param width the width of the matrix
/// @param height the height of the matrix
/// @param blur_amount the amount of blur to apply
void blur2d(RGB* leds, uint8_t width, uint8_t height, fract8 blur_amount);

/// Perform a blur1d() on every row of a rectangular matrix
/// @see blur1d()
/// @param leds a pointer to the LED array to blur
/// @param width the width of the matrix
/// @param height the height of the matrix
/// @param blur_amount the amount of blur to apply
void blurRows(RGB* leds, uint8_t width, uint8_t height, fract8 blur_amount);

/// Perform a blur1d() on every column of a rectangular matrix
/// @copydetails blurRows()
void blurColumns(RGB* leds, uint8_t width, uint8_t height, fract8 blur_amount);

/// @} ColorBlurs

/// @addtogroup ColorFills
/// @{

/// Approximates a "black body radiation" spectrum for
/// a given "heat" level.  This is useful for animations of "fire".
/// Heat is specified as an arbitrary scale from 0 (cool) to 255 (hot).
/// This is NOT a chromatically correct "black body radiation"
/// spectrum, but it's surprisingly close, and it's fast and small.
RGB HeatColor(uint8_t temperature);

/// @} ColorFills
/// @} ColorUtils

/// @defgroup ColorPalettes Color Palettes
/// Functions and class definitions for color palettes.
///
/// RGB palettes map an 8-bit value (0-255) to an RGB color.
///
/// You can create any color palette you wish; a couple of starters
/// are provided: ForestColors_p, CloudColors_p, LavaColors_p, OceanColors_p,
/// RainbowColors_p, and RainbowStripeColors_p.
///
/// Palettes come in the traditional 256-entry variety, which take
/// up 768 bytes of RAM, and lightweight 16-entry varieties.  The 16-entry
/// variety automatically interpolates between its entries to produce
/// a full 256-element color map, but at a cost of only 48 bytes of RAM.
///
/// Basic operation is like this (using the 16-entry variety):
///
/// 1. Declare your palette storage:
///      @code{.cpp}
///      RGBPalette16 myPalette;
///      @endcode
///
/// 2. Fill `myPalette` with your own 16 colors, or with a preset color scheme.
///    You can specify your 16 colors a variety of ways:
///      @code{.cpp}
///      RGBPalette16 myPalette(
///          RGB::Black,
///          RGB::Black,
///          RGB::Red,
///          RGB::Yellow,
///          RGB::Green,
///          RGB::Blue,
///          RGB::Purple,
///          RGB::Black,
///
///          0x100000,
///          0x200000,
///          0x400000,
///          0x800000,
///
///          HSV( 30,255,255),
///          HSV( 50,255,255),
///          HSV( 70,255,255),
///          HSV( 90,255,255)
///      );
///      @endcode
///
///    Or you can initiaize your palette with a preset color scheme:
///      @code{.cpp}
///      myPalette = RainbowStripesColors_p;
///      @endcode
///
/// 3. Any time you want to set a pixel to a color from your palette, use
///    `ColorFromPalette()` as shown:
///
///      @code{.cpp}
///      uint8_t index = /* any value 0-255 */;
///      leds[i] = ColorFromPalette(myPalette, index);
///      @endcode
///
///    Even though your palette has only 16 explicily defined entries, you
///    can use an "index" from 0-255.  The 16 explicit palette entries will
///    be spread evenly across the 0-255 range, and the intermedate values
///    will be RGB-interpolated between adjacent explicit entries.
///
///    It's easier to use than it sounds.
///
/// @{

/// @defgroup PaletteClasses Palette Classes
/// Class definitions for color palettes.
/// @todo For documentation purposes it would be nice to reorder these
///        definitions by type and in ascending number of entries.
///
/// @{

class RGBPalette16;
class RGBPalette32;
class RGBPalette256;
class HSVPalette16;
class HSVPalette32;
class HSVPalette256;

/// Struct for digesting gradient pointer data into its components.
/// This is used when loading a gradient stored in PROGMEM or on
/// the heap into a palette. The pointer is dereferenced and interpreted as
/// this struct, so the component parts can be addressed and copied by name.
typedef union {
    struct {
        uint8_t index;  ///< index of the color entry in the gradient
        uint8_t r;      ///< RGB::red channel value of the color entry
        uint8_t g;      ///< RGB::green channel value of the color entry
        uint8_t b;      ///< RGB::blue channel value of the color entry
    };
    uint32_t dword;    ///< values as a packed 32-bit double word
    uint8_t bytes[4];  ///< values as an array
} TRGBGradientPaletteEntryUnion;

typedef uint8_t
    TDynamiRGBGradientPalette_byte;  ///< Byte of an RGB gradient entry, stored
                                     ///< in dynamic (heap) memory
typedef const TDynamiRGBGradientPalette_byte*
    TDynamiRGBGradientPalette_bytes;  ///< Pointer to bytes of an RGB gradient,
                                      ///< stored in dynamic (heap) memory
typedef TDynamiRGBGradientPalette_bytes
    TDynamiRGBGradientPalettePtr;  ///< Alias of
                                   ///< ::TDynamiRGBGradientPalette_bytes

/// @}

/// @defgroup PaletteUpscale Palette Upscaling Functions
/// Functions to upscale palettes from one type to another.
/// @{

/// Convert a 16-entry palette to a 256-entry palette
/// @param srcpal16 the source palette to upscale
/// @param destpal256 the destination palette for the upscaled data
void UpscalePalette(const struct RGBPalette16& srcpal16,
                    struct RGBPalette256& destpal256);
/// @copydoc UpscalePalette(const struct RGBPalette16&, struct RGBPalette256&)
void UpscalePalette(const struct HSVPalette16& srcpal16,
                    struct HSVPalette256& destpal256);

/// Convert a 16-entry palette to a 32-entry palette
/// @param srcpal16 the source palette to upscale
/// @param destpal32 the destination palette for the upscaled data
void UpscalePalette(const struct RGBPalette16& srcpal16,
                    struct RGBPalette32& destpal32);
/// @copydoc UpscalePalette(const struct RGBPalette16&, struct RGBPalette32&)
void UpscalePalette(const struct HSVPalette16& srcpal16,
                    struct HSVPalette32& destpal32);

/// Convert a 32-entry palette to a 256-entry palette
/// @param srcpal32 the source palette to upscale
/// @param destpal256 the destination palette for the upscaled data
void UpscalePalette(const struct RGBPalette32& srcpal32,
                    struct RGBPalette256& destpal256);
/// @copydoc UpscalePalette(const struct RGBPalette32&, struct RGBPalette256&)
void UpscalePalette(const struct HSVPalette32& srcpal32,
                    struct HSVPalette256& destpal256);

/// @} PaletteUpscale

/// @addtogroup PaletteClasses
/// @{

/// HSV color palette with 16 discrete values
class HSVPalette16 {
   public:
    HSV entries[16];  ///< the color entries that make up the palette

    /// @copydoc HSV::HSV()
    HSVPalette16(){};

    /// Create palette from 16 HSV values
    HSVPalette16(const HSV& c00, const HSV& c01, const HSV& c02, const HSV& c03,
                 const HSV& c04, const HSV& c05, const HSV& c06, const HSV& c07,
                 const HSV& c08, const HSV& c09, const HSV& c10, const HSV& c11,
                 const HSV& c12, const HSV& c13, const HSV& c14,
                 const HSV& c15) {
        entries[0] = c00;
        entries[1] = c01;
        entries[2] = c02;
        entries[3] = c03;
        entries[4] = c04;
        entries[5] = c05;
        entries[6] = c06;
        entries[7] = c07;
        entries[8] = c08;
        entries[9] = c09;
        entries[10] = c10;
        entries[11] = c11;
        entries[12] = c12;
        entries[13] = c13;
        entries[14] = c14;
        entries[15] = c15;
    };

    /// Copy constructor
    HSVPalette16(const HSVPalette16& rhs) {
        memcpy((void*)&(entries[0]), &(rhs.entries[0]), sizeof(entries));
    }

    /// @copydoc HSVPalette16(const HSVPalette16& rhs)
    HSVPalette16& operator=(const HSVPalette16& rhs) {
        memcpy((void*)&(entries[0]), &(rhs.entries[0]), sizeof(entries));
        return *this;
    }

    /// Array access operator to index into the gradient entries
    /// @param x the index to retrieve
    /// @returns reference to an entry in the palette's color array
    /// @note This does not perform any interpolation like ColorFromPalette(),
    /// it accesses the underlying entries that make up the gradient. Beware
    /// of bounds issues!
    inline HSV& operator[](uint8_t x) __attribute__((always_inline)) {
        return entries[x];
    }

    /// @copydoc operator[]
    inline const HSV& operator[](uint8_t x) const
        __attribute__((always_inline)) {
        return entries[x];
    }

    /// @copydoc operator[]
    inline HSV& operator[](int x) __attribute__((always_inline)) {
        return entries[(uint8_t)x];
    }

    /// @copydoc operator[]
    inline const HSV& operator[](int x) const __attribute__((always_inline)) {
        return entries[(uint8_t)x];
    }

    /// Get the underlying pointer to the HSV entries making up the palette
    operator HSV*() { return &(entries[0]); }

    /// Check if two palettes have the same color entries
    bool operator==(const HSVPalette16& rhs) const {
        const uint8_t* p = (const uint8_t*)(&(this->entries[0]));
        const uint8_t* q = (const uint8_t*)(&(rhs.entries[0]));
        if (p == q) return true;
        for (uint8_t i = 0; i < (sizeof(entries)); ++i) {
            if (*p != *q) return false;
            ++p;
            ++q;
        }
        return true;
    }

    /// Check if two palettes do not have the same color entries
    bool operator!=(const HSVPalette16& rhs) const { return !(*this == rhs); }

    /// Create palette filled with one color
    /// @param c1 the color to fill the palette with
    HSVPalette16(const HSV& c1) { fill_solid(&(entries[0]), 16, c1); }

    /// Create palette with a gradient from one color to another
    /// @param c1 the starting color for the gradient
    /// @param c2 the end color for the gradient
    HSVPalette16(const HSV& c1, const HSV& c2) {
        fill_gradient(&(entries[0]), 16, c1, c2);
    }

    /// Create palette with three-color gradient
    /// @param c1 the starting color for the gradient
    /// @param c2 the middle color for the gradient
    /// @param c3 the end color for the gradient
    HSVPalette16(const HSV& c1, const HSV& c2, const HSV& c3) {
        fill_gradient(&(entries[0]), 16, c1, c2, c3);
    }

    /// Create palette with four-color gradient
    /// @param c1 the starting color for the gradient
    /// @param c2 the first middle color for the gradient
    /// @param c3 the second middle color for the gradient
    /// @param c4 the end color for the gradient
    HSVPalette16(const HSV& c1, const HSV& c2, const HSV& c3, const HSV& c4) {
        fill_gradient(&(entries[0]), 16, c1, c2, c3, c4);
    }
};

/// HSV color palette with 256 discrete values
class HSVPalette256 {
   public:
    HSV entries[256];  ///< @copydoc HSVPalette16::entries

    /// @copydoc HSVPalette16::HSVPalette16()
    HSVPalette256(){};

    /// @copydoc HSVPalette16::HSVPalette16(const HSV&, const HSV&, const HSV&,
    /// const HSV&, const HSV&, const HSV&, const HSV&, const HSV&, const HSV&,
    /// const HSV&, const HSV&, const HSV&, const HSV&, const HSV&, const HSV&,
    /// const HSV&)
    HSVPalette256(const HSV& c00, const HSV& c01, const HSV& c02,
                  const HSV& c03, const HSV& c04, const HSV& c05,
                  const HSV& c06, const HSV& c07, const HSV& c08,
                  const HSV& c09, const HSV& c10, const HSV& c11,
                  const HSV& c12, const HSV& c13, const HSV& c14,
                  const HSV& c15) {
        HSVPalette16 p16(c00, c01, c02, c03, c04, c05, c06, c07, c08, c09, c10,
                         c11, c12, c13, c14, c15);
        *this = p16;
    };

    /// Copy constructor
    HSVPalette256(const HSVPalette256& rhs) {
        memcpy((void*)&(entries[0]), &(rhs.entries[0]), sizeof(entries));
    }
    /// @copydoc HSVPalette256( const HSVPalette256&)
    HSVPalette256& operator=(const HSVPalette256& rhs) {
        memcpy((void*)&(entries[0]), &(rhs.entries[0]), sizeof(entries));
        return *this;
    }

    /// Create upscaled palette from 16-entry palette
    HSVPalette256(const HSVPalette16& rhs16) { UpscalePalette(rhs16, *this); }
    /// @copydoc  HSVPalette256( const HSVPalette16&)
    HSVPalette256& operator=(const HSVPalette16& rhs16) {
        UpscalePalette(rhs16, *this);
        return *this;
    }

    /// @copydoc HSVPalette16::operator[]
    inline HSV& operator[](uint8_t x) __attribute__((always_inline)) {
        return entries[x];
    }
    /// @copydoc operator[]
    inline const HSV& operator[](uint8_t x) const
        __attribute__((always_inline)) {
        return entries[x];
    }

    /// @copydoc operator[]
    inline HSV& operator[](int x) __attribute__((always_inline)) {
        return entries[(uint8_t)x];
    }
    /// @copydoc operator[]
    inline const HSV& operator[](int x) const __attribute__((always_inline)) {
        return entries[(uint8_t)x];
    }

    /// Get the underlying pointer to the HSV entries making up the palette
    operator HSV*() { return &(entries[0]); }

    /// @copydoc HSVPalette16::operator==
    bool operator==(const HSVPalette256& rhs) const {
        const uint8_t* p = (const uint8_t*)(&(this->entries[0]));
        const uint8_t* q = (const uint8_t*)(&(rhs.entries[0]));
        if (p == q) return true;
        for (uint16_t i = 0; i < (sizeof(entries)); ++i) {
            if (*p != *q) return false;
            ++p;
            ++q;
        }
        return true;
    }

    /// @copydoc HSVPalette16::operator!=
    bool operator!=(const HSVPalette256& rhs) const { return !(*this == rhs); }

    /// @copydoc HSVPalette16::HSVPalette16(const HSV&)
    HSVPalette256(const HSV& c1) { fill_solid(&(entries[0]), 256, c1); }
    /// @copydoc HSVPalette16::HSVPalette16(const HSV&, const HSV&)
    HSVPalette256(const HSV& c1, const HSV& c2) {
        fill_gradient(&(entries[0]), 256, c1, c2);
    }
    /// @copydoc HSVPalette16::HSVPalette16(const HSV&, const HSV&, const HSV&)
    HSVPalette256(const HSV& c1, const HSV& c2, const HSV& c3) {
        fill_gradient(&(entries[0]), 256, c1, c2, c3);
    }
    /// @copydoc HSVPalette16::HSVPalette16(const HSV&, const HSV&, const HSV&,
    /// const HSV&)
    HSVPalette256(const HSV& c1, const HSV& c2, const HSV& c3, const HSV& c4) {
        fill_gradient(&(entries[0]), 256, c1, c2, c3, c4);
    }
};

/// RGB color palette with 16 discrete values
class RGBPalette16 {
   public:
    RGB entries[16];  ///< @copydoc HSVPalette16::entries

    /// @copydoc RGB::RGB()
    RGBPalette16(){};

    /// Create palette from 16 RGB values
    RGBPalette16(const RGB& c00, const RGB& c01, const RGB& c02, const RGB& c03,
                 const RGB& c04, const RGB& c05, const RGB& c06, const RGB& c07,
                 const RGB& c08, const RGB& c09, const RGB& c10, const RGB& c11,
                 const RGB& c12, const RGB& c13, const RGB& c14,
                 const RGB& c15) {
        entries[0] = c00;
        entries[1] = c01;
        entries[2] = c02;
        entries[3] = c03;
        entries[4] = c04;
        entries[5] = c05;
        entries[6] = c06;
        entries[7] = c07;
        entries[8] = c08;
        entries[9] = c09;
        entries[10] = c10;
        entries[11] = c11;
        entries[12] = c12;
        entries[13] = c13;
        entries[14] = c14;
        entries[15] = c15;
    };

    /// Copy constructor
    RGBPalette16(const RGBPalette16& rhs) {
        memcpy((void*)&(entries[0]), &(rhs.entries[0]), sizeof(entries));
    }
    /// Create palette from array of RGB colors
    RGBPalette16(const RGB rhs[16]) {
        memcpy((void*)&(entries[0]), &(rhs[0]), sizeof(entries));
    }
    RGBPalette16(const int rhs[16]) {
        for(int i = 0; i < 16; i++) {
            entries[0] = rhs[i];
        }
    }

    /// @copydoc RGBPalette16(const RGBPalette16&)
    RGBPalette16& operator=(const RGBPalette16& rhs) {
        memcpy((void*)&(entries[0]), &(rhs.entries[0]), sizeof(entries));
        return *this;
    }
    /// Create palette from array of RGB colors
    RGBPalette16& operator=(const RGB rhs[16]) {
        memcpy((void*)&(entries[0]), &(rhs[0]), sizeof(entries));
        return *this;
    }

    /// Create palette from HSV palette
    RGBPalette16(const HSVPalette16& rhs) {
        for (uint8_t i = 0; i < 16; ++i) {
            entries[i] = rhs.entries[i];  // implicit HSV-to-RGB conversion
        }
    }
    /// Create palette from array of HSV colors
    RGBPalette16(const HSV rhs[16]) {
        for (uint8_t i = 0; i < 16; ++i) {
            entries[i] = rhs[i];  // implicit HSV-to-RGB conversion
        }
    }
    /// @copydoc RGBPalette16(const HSVPalette16&)
    RGBPalette16& operator=(const HSVPalette16& rhs) {
        for (uint8_t i = 0; i < 16; ++i) {
            entries[i] = rhs.entries[i];  // implicit HSV-to-RGB conversion
        }
        return *this;
    }
    /// Create palette from array of HSV colors
    RGBPalette16& operator=(const HSV rhs[16]) {
        for (uint8_t i = 0; i < 16; ++i) {
            entries[i] = rhs[i];  // implicit HSV-to-RGB conversion
        }
        return *this;
    }

    /// @copydoc HSVPalette16::operator==
    bool operator==(const RGBPalette16& rhs) const {
        const uint8_t* p = (const uint8_t*)(&(this->entries[0]));
        const uint8_t* q = (const uint8_t*)(&(rhs.entries[0]));
        if (p == q) return true;
        for (uint8_t i = 0; i < (sizeof(entries)); ++i) {
            if (*p != *q) return false;
            ++p;
            ++q;
        }
        return true;
    }
    /// @copydoc HSVPalette16::operator!=
    bool operator!=(const RGBPalette16& rhs) const { return !(*this == rhs); }
    /// @copydoc HSVPalette16::operator[]
    inline RGB& operator[](uint8_t x) __attribute__((always_inline)) {
        return entries[x];
    }
    /// @copydoc HSVPalette16::operator[]
    inline const RGB& operator[](uint8_t x) const
        __attribute__((always_inline)) {
        return entries[x];
    }

    /// @copydoc HSVPalette16::operator[]
    inline RGB& operator[](int x) __attribute__((always_inline)) {
        return entries[(uint8_t)x];
    }
    /// @copydoc HSVPalette16::operator[]
    inline const RGB& operator[](int x) const __attribute__((always_inline)) {
        return entries[(uint8_t)x];
    }

    /// Get the underlying pointer to the HSV entries making up the palette
    operator RGB*() { return &(entries[0]); }

    /// @copydoc HSVPalette16::HSVPalette16(const HSV&)
    RGBPalette16(const HSV& c1) { fill_solid(&(entries[0]), 16, c1); }
    /// @copydoc HSVPalette16::HSVPalette16(const HSV&, const HSV&)
    RGBPalette16(const HSV& c1, const HSV& c2) {
        fill_gradient(&(entries[0]), 16, c1, c2);
    }
    /// @copydoc HSVPalette16::HSVPalette16(const HSV&, const HSV&, const HSV&)
    RGBPalette16(const HSV& c1, const HSV& c2, const HSV& c3) {
        fill_gradient(&(entries[0]), 16, c1, c2, c3);
    }
    /// @copydoc HSVPalette16::HSVPalette16(const HSV&, const HSV&, const HSV&,
    /// const HSV&)
    RGBPalette16(const HSV& c1, const HSV& c2, const HSV& c3, const HSV& c4) {
        fill_gradient(&(entries[0]), 16, c1, c2, c3, c4);
    }

    /// @copydoc HSVPalette16::HSVPalette16(const HSV&)
    RGBPalette16(const RGB& c1) { fill_solid(&(entries[0]), 16, c1); }
    /// @copydoc HSVPalette16::HSVPalette16(const HSV&, const HSV&)
    RGBPalette16(const RGB& c1, const RGB& c2) {
        fill_gradient_RGB(&(entries[0]), 16, c1, c2);
    }
    /// @copydoc HSVPalette16::HSVPalette16(const HSV&, const HSV&, const HSV&)
    RGBPalette16(const RGB& c1, const RGB& c2, const RGB& c3) {
        fill_gradient_RGB(&(entries[0]), 16, c1, c2, c3);
    }
    /// @copydoc HSVPalette16::HSVPalette16(const HSV&, const HSV&, const HSV&,
    /// const HSV&)
    RGBPalette16(const RGB& c1, const RGB& c2, const RGB& c3, const RGB& c4) {
        fill_gradient_RGB(&(entries[0]), 16, c1, c2, c3, c4);
    }

    /// Creates a palette from a gradient palette in dynamic (heap) memory.
    /// @copydetails
    /// RGBPalette16::RGBPalette16(TProgmemRGBGradientPalette_bytes)
    RGBPalette16& loadDynamicGradientPalette(
        TDynamiRGBGradientPalette_bytes gpal) {
        TRGBGradientPaletteEntryUnion* ent =
            (TRGBGradientPaletteEntryUnion*)(gpal);
        TRGBGradientPaletteEntryUnion u;

        // Count entries
        uint16_t count = 0;
        do {
            u = *(ent + count);
            ++count;
        } while (u.index != 255);

        int8_t lastSlotUsed = -1;

        u = *ent;
        RGB rgbstart(u.r, u.g, u.b);

        int indexstart = 0;
        uint8_t istart8 = 0;
        uint8_t iend8 = 0;
        while (indexstart < 255) {
            ++ent;
            u = *ent;
            int indexend = u.index;
            RGB rgbend(u.r, u.g, u.b);
            istart8 = indexstart / 16;
            iend8 = indexend / 16;
            if (count < 16) {
                if ((istart8 <= lastSlotUsed) && (lastSlotUsed < 15)) {
                    istart8 = lastSlotUsed + 1;
                    if (iend8 < istart8) {
                        iend8 = istart8;
                    }
                }
                lastSlotUsed = iend8;
            }
            fill_gradient_RGB(&(entries[0]), istart8, rgbstart, iend8, rgbend);
            indexstart = indexend;
            rgbstart = rgbend;
        }
        return *this;
    }
};

/// HSV color palette with 32 discrete values
class HSVPalette32 {
   public:
    HSV entries[32];  ///< @copydoc HSVPalette16::entries

    /// @copydoc HSVPalette16::HSVPalette16()
    HSVPalette32(){};

    /// @copydoc HSVPalette16::HSVPalette16(const HSV&, const HSV&, const HSV&,
    /// const HSV&, const HSV&, const HSV&, const HSV&, const HSV&, const HSV&,
    /// const HSV&, const HSV&, const HSV&, const HSV&, const HSV&, const HSV&,
    /// const HSV&)
    HSVPalette32(const HSV& c00, const HSV& c01, const HSV& c02, const HSV& c03,
                 const HSV& c04, const HSV& c05, const HSV& c06, const HSV& c07,
                 const HSV& c08, const HSV& c09, const HSV& c10, const HSV& c11,
                 const HSV& c12, const HSV& c13, const HSV& c14,
                 const HSV& c15) {
        for (uint8_t i = 0; i < 2; ++i) {
            entries[0 + i] = c00;
            entries[2 + i] = c01;
            entries[4 + i] = c02;
            entries[6 + i] = c03;
            entries[8 + i] = c04;
            entries[10 + i] = c05;
            entries[12 + i] = c06;
            entries[14 + i] = c07;
            entries[16 + i] = c08;
            entries[18 + i] = c09;
            entries[20 + i] = c10;
            entries[22 + i] = c11;
            entries[24 + i] = c12;
            entries[26 + i] = c13;
            entries[28 + i] = c14;
            entries[30 + i] = c15;
        }
    };

    /// Copy constructor
    HSVPalette32(const HSVPalette32& rhs) {
        memcpy((void*)&(entries[0]), &(rhs.entries[0]), sizeof(entries));
    }
    /// @copydoc HSVPalette32( const HSVPalette32&)
    HSVPalette32& operator=(const HSVPalette32& rhs) {
        memcpy((void*)&(entries[0]), &(rhs.entries[0]), sizeof(entries));
        return *this;
    }

    /// @copydoc HSVPalette16::HSVPalette16(const TProgmemHSVPalette16&)
    inline HSV& operator[](uint8_t x) __attribute__((always_inline)) {
        return entries[x];
    }
    /// @copydoc HSVPalette16::HSVPalette16(const TProgmemHSVPalette16&)
    inline const HSV& operator[](uint8_t x) const
        __attribute__((always_inline)) {
        return entries[x];
    }

    /// @copydoc HSVPalette16::HSVPalette16(const TProgmemHSVPalette16&)
    inline HSV& operator[](int x) __attribute__((always_inline)) {
        return entries[(uint8_t)x];
    }
    /// @copydoc HSVPalette16::HSVPalette16(const TProgmemHSVPalette16&)
    inline const HSV& operator[](int x) const __attribute__((always_inline)) {
        return entries[(uint8_t)x];
    }

    /// Get the underlying pointer to the HSV entries making up the palette
    operator HSV*() { return &(entries[0]); }

    /// @copydoc HSVPalette16::operator==
    bool operator==(const HSVPalette32& rhs) const {
        const uint8_t* p = (const uint8_t*)(&(this->entries[0]));
        const uint8_t* q = (const uint8_t*)(&(rhs.entries[0]));
        if (p == q) return true;
        for (uint8_t i = 0; i < (sizeof(entries)); ++i) {
            if (*p != *q) return false;
            ++p;
            ++q;
        }
        return true;
    }
    /// @copydoc HSVPalette16::operator!=
    bool operator!=(const HSVPalette32& rhs) const { return !(*this == rhs); }

    /// @copydoc HSVPalette16::HSVPalette16(const HSV&)
    HSVPalette32(const HSV& c1) { fill_solid(&(entries[0]), 32, c1); }
    /// @copydoc HSVPalette16::HSVPalette16(const HSV&, const HSV&)
    HSVPalette32(const HSV& c1, const HSV& c2) {
        fill_gradient(&(entries[0]), 32, c1, c2);
    }
    /// @copydoc HSVPalette16::HSVPalette16(const HSV&, const HSV&, const HSV&)
    HSVPalette32(const HSV& c1, const HSV& c2, const HSV& c3) {
        fill_gradient(&(entries[0]), 32, c1, c2, c3);
    }
    /// @copydoc HSVPalette16::HSVPalette16(const HSV&, const HSV&, const HSV&,
    /// const HSV&)
    HSVPalette32(const HSV& c1, const HSV& c2, const HSV& c3, const HSV& c4) {
        fill_gradient(&(entries[0]), 32, c1, c2, c3, c4);
    }
};

/// RGB color palette with 32 discrete values
class RGBPalette32 {
   public:
    RGB entries[32];  ///< @copydoc HSVPalette16::entries

    /// @copydoc RGB::RGB()
    RGBPalette32(){};

    /// @copydoc RGBPalette16::RGBPalette16(const RGB&,
    /// const RGB&, const RGB&, const RGB&,
    /// const RGB&, const RGB&, const RGB&, const RGB&,
    /// const RGB&, const RGB&, const RGB&, const RGB&,
    /// const RGB&, const RGB&, const RGB&, const RGB&)
    RGBPalette32(const RGB& c00, const RGB& c01, const RGB& c02, const RGB& c03,
                 const RGB& c04, const RGB& c05, const RGB& c06, const RGB& c07,
                 const RGB& c08, const RGB& c09, const RGB& c10, const RGB& c11,
                 const RGB& c12, const RGB& c13, const RGB& c14,
                 const RGB& c15) {
        for (uint8_t i = 0; i < 2; ++i) {
            entries[0 + i] = c00;
            entries[2 + i] = c01;
            entries[4 + i] = c02;
            entries[6 + i] = c03;
            entries[8 + i] = c04;
            entries[10 + i] = c05;
            entries[12 + i] = c06;
            entries[14 + i] = c07;
            entries[16 + i] = c08;
            entries[18 + i] = c09;
            entries[20 + i] = c10;
            entries[22 + i] = c11;
            entries[24 + i] = c12;
            entries[26 + i] = c13;
            entries[28 + i] = c14;
            entries[30 + i] = c15;
        }
    };

    /// @copydoc RGBPalette16::RGBPalette16(const RGBPalette16&)
    RGBPalette32(const RGBPalette32& rhs) {
        memcpy((void*)&(entries[0]), &(rhs.entries[0]), sizeof(entries));
    }
    /// Create palette from array of RGB colors
    RGBPalette32(const RGB rhs[32]) {
        memcpy((void*)&(entries[0]), &(rhs[0]), sizeof(entries));
    }

    RGBPalette32(const int rhs[32]) {
        for(int i = 0; i < 32; i++) {
            entries[i] = rhs[i];
        }
    }
    /// @copydoc RGBPalette32(const RGBPalette32&)
    RGBPalette32& operator=(const RGBPalette32& rhs) {
        memcpy((void*)&(entries[0]), &(rhs.entries[0]), sizeof(entries));
        return *this;
    }
    /// Create palette from array of RGB colors
    RGBPalette32& operator=(const RGB rhs[32]) {
        memcpy((void*)&(entries[0]), &(rhs[0]), sizeof(entries));
        return *this;
    }

    /// @copydoc RGBPalette16::RGBPalette16(const HSVPalette16&)
    RGBPalette32(const HSVPalette32& rhs) {
        for (uint8_t i = 0; i < 32; ++i) {
            entries[i] = rhs.entries[i];  // implicit HSV-to-RGB conversion
        }
    }
    /// Create palette from array of HSV colors
    RGBPalette32(const HSV rhs[32]) {
        for (uint8_t i = 0; i < 32; ++i) {
            entries[i] = rhs[i];  // implicit HSV-to-RGB conversion
        }
    }
    /// @copydoc RGBPalette32(const HSVPalette32&)
    RGBPalette32& operator=(const HSVPalette32& rhs) {
        for (uint8_t i = 0; i < 32; ++i) {
            entries[i] = rhs.entries[i];  // implicit HSV-to-RGB conversion
        }
        return *this;
    }
    /// Create palette from array of HSV colors
    RGBPalette32& operator=(const HSV rhs[32]) {
        for (uint8_t i = 0; i < 32; ++i) {
            entries[i] = rhs[i];  // implicit HSV-to-RGB conversion
        }
        return *this;
    }

    /// @copydoc RGBPalette16::operator==
    bool operator==(const RGBPalette32& rhs) const {
        const uint8_t* p = (const uint8_t*)(&(this->entries[0]));
        const uint8_t* q = (const uint8_t*)(&(rhs.entries[0]));
        if (p == q) return true;
        for (uint8_t i = 0; i < (sizeof(entries)); ++i) {
            if (*p != *q) return false;
            ++p;
            ++q;
        }
        return true;
    }
    /// @copydoc RGBPalette16::operator!=
    bool operator!=(const RGBPalette32& rhs) const { return !(*this == rhs); }

    /// @copydoc RGBPalette16::operator[]
    inline RGB& operator[](uint8_t x) __attribute__((always_inline)) {
        return entries[x];
    }
    /// @copydoc RGBPalette16::operator[]
    inline const RGB& operator[](uint8_t x) const
        __attribute__((always_inline)) {
        return entries[x];
    }

    /// @copydoc RGBPalette16::operator[]
    inline RGB& operator[](int x) __attribute__((always_inline)) {
        return entries[(uint8_t)x];
    }
    /// @copydoc RGBPalette16::operator[]
    inline const RGB& operator[](int x) const __attribute__((always_inline)) {
        return entries[(uint8_t)x];
    }

    /// Get the underlying pointer to the RGB entries making up the palette
    operator RGB*() { return &(entries[0]); }

    /// @copydoc RGBPalette16::RGBPalette16(const HSV&)
    RGBPalette32(const HSV& c1) { fill_solid(&(entries[0]), 32, c1); }
    /// @copydoc RGBPalette16::RGBPalette16(const HSV&, const HSV&)
    RGBPalette32(const HSV& c1, const HSV& c2) {
        fill_gradient(&(entries[0]), 32, c1, c2);
    }
    /// @copydoc RGBPalette16::RGBPalette16(const HSV&, const HSV&, const HSV&)
    RGBPalette32(const HSV& c1, const HSV& c2, const HSV& c3) {
        fill_gradient(&(entries[0]), 32, c1, c2, c3);
    }
    /// @copydoc RGBPalette16::RGBPalette16(const HSV&, const HSV&, const HSV&,
    /// const HSV&)
    RGBPalette32(const HSV& c1, const HSV& c2, const HSV& c3, const HSV& c4) {
        fill_gradient(&(entries[0]), 32, c1, c2, c3, c4);
    }

    /// @copydoc RGBPalette16::RGBPalette16(const RGB&)
    RGBPalette32(const RGB& c1) { fill_solid(&(entries[0]), 32, c1); }
    /// @copydoc RGBPalette16::RGBPalette16(const RGB&, const RGB&)
    RGBPalette32(const RGB& c1, const RGB& c2) {
        fill_gradient_RGB(&(entries[0]), 32, c1, c2);
    }
    /// @copydoc RGBPalette16::RGBPalette16(const RGB&, const RGB&, const RGB&)
    RGBPalette32(const RGB& c1, const RGB& c2, const RGB& c3) {
        fill_gradient_RGB(&(entries[0]), 32, c1, c2, c3);
    }
    /// @copydoc RGBPalette16::RGBPalette16(const RGB&, const RGB&, const RGB&,
    /// const RGB&)
    RGBPalette32(const RGB& c1, const RGB& c2, const RGB& c3, const RGB& c4) {
        fill_gradient_RGB(&(entries[0]), 32, c1, c2, c3, c4);
    }

    /// Create upscaled palette from 16-entry palette
    RGBPalette32(const RGBPalette16& rhs16) { UpscalePalette(rhs16, *this); }
    /// @copydoc RGBPalette32(const RGBPalette16&)
    RGBPalette32& operator=(const RGBPalette16& rhs16) {
        UpscalePalette(rhs16, *this);
        return *this;
    }

    /// @copydoc
    /// RGBPalette16::loadDynamicGradientPalette(TDynamiRGBGradientPalette_bytes)
    RGBPalette32& loadDynamicGradientPalette(
        TDynamiRGBGradientPalette_bytes gpal) {
        TRGBGradientPaletteEntryUnion* ent =
            (TRGBGradientPaletteEntryUnion*)(gpal);
        TRGBGradientPaletteEntryUnion u;

        // Count entries
        uint16_t count = 0;
        do {
            u = *(ent + count);
            ++count;
        } while (u.index != 255);

        int8_t lastSlotUsed = -1;

        u = *ent;
        RGB rgbstart(u.r, u.g, u.b);

        int indexstart = 0;
        uint8_t istart8 = 0;
        uint8_t iend8 = 0;
        while (indexstart < 255) {
            ++ent;
            u = *ent;
            int indexend = u.index;
            RGB rgbend(u.r, u.g, u.b);
            istart8 = indexstart / 8;
            iend8 = indexend / 8;
            if (count < 16) {
                if ((istart8 <= lastSlotUsed) && (lastSlotUsed < 31)) {
                    istart8 = lastSlotUsed + 1;
                    if (iend8 < istart8) {
                        iend8 = istart8;
                    }
                }
                lastSlotUsed = iend8;
            }
            fill_gradient_RGB(&(entries[0]), istart8, rgbstart, iend8, rgbend);
            indexstart = indexend;
            rgbstart = rgbend;
        }
        return *this;
    }
};

/// RGB color palette with 256 discrete values
class RGBPalette256 {
   public:
    RGB entries[256];  ///< @copydoc HSVPalette16::entries

    /// @copydoc RGB::RGB()
    RGBPalette256(){};

    /// @copydoc RGBPalette16::RGBPalette16(const RGB&,
    /// const RGB&, const RGB&, const RGB&,
    /// const RGB&, const RGB&, const RGB&, const RGB&,
    /// const RGB&, const RGB&, const RGB&, const RGB&,
    /// const RGB&, const RGB&, const RGB&, const RGB&)
    RGBPalette256(const RGB& c00, const RGB& c01, const RGB& c02,
                  const RGB& c03, const RGB& c04, const RGB& c05,
                  const RGB& c06, const RGB& c07, const RGB& c08,
                  const RGB& c09, const RGB& c10, const RGB& c11,
                  const RGB& c12, const RGB& c13, const RGB& c14,
                  const RGB& c15) {
        RGBPalette16 p16(c00, c01, c02, c03, c04, c05, c06, c07, c08, c09, c10,
                         c11, c12, c13, c14, c15);
        *this = p16;
    };

    /// @copydoc RGBPalette16::RGBPalette16(const RGBPalette16&)
    RGBPalette256(const RGBPalette256& rhs) {
        memcpy((void*)&(entries[0]), &(rhs.entries[0]), sizeof(entries));
    }
    /// Create palette from array of RGB colors
    RGBPalette256(const RGB rhs[256]) {
        memcpy((void*)&(entries[0]), &(rhs[0]), sizeof(entries));
    }
    /// @copydoc RGBPalette256(const RGBPalette256&)
    RGBPalette256& operator=(const RGBPalette256& rhs) {
        memcpy((void*)&(entries[0]), &(rhs.entries[0]), sizeof(entries));
        return *this;
    }
    /// Create palette from array of RGB colors
    RGBPalette256& operator=(const RGB rhs[256]) {
        memcpy((void*)&(entries[0]), &(rhs[0]), sizeof(entries));
        return *this;
    }

    /// @copydoc RGBPalette16::RGBPalette16(const HSVPalette16&)
    RGBPalette256(const HSVPalette256& rhs) {
        for (int i = 0; i < 256; ++i) {
            entries[i] = rhs.entries[i];  // implicit HSV-to-RGB conversion
        }
    }
    /// Create palette from array of HSV colors
    RGBPalette256(const HSV rhs[256]) {
        for (int i = 0; i < 256; ++i) {
            entries[i] = rhs[i];  // implicit HSV-to-RGB conversion
        }
    }
    /// @copydoc RGBPalette256(const RGBPalette256&)
    RGBPalette256& operator=(const HSVPalette256& rhs) {
        for (int i = 0; i < 256; ++i) {
            entries[i] = rhs.entries[i];  // implicit HSV-to-RGB conversion
        }
        return *this;
    }
    /// Create palette from array of HSV colors
    RGBPalette256& operator=(const HSV rhs[256]) {
        for (int i = 0; i < 256; ++i) {
            entries[i] = rhs[i];  // implicit HSV-to-RGB conversion
        }
        return *this;
    }

    /// @copydoc RGBPalette32::RGBPalette32(const RGBPalette16&)
    RGBPalette256(const RGBPalette16& rhs16) { UpscalePalette(rhs16, *this); }
    /// @copydoc RGBPalette256(const RGBPalette16&)
    RGBPalette256& operator=(const RGBPalette16& rhs16) {
        UpscalePalette(rhs16, *this);
        return *this;
    }

    /// @copydoc RGBPalette16::operator==
    bool operator==(const RGBPalette256& rhs) const {
        const uint8_t* p = (const uint8_t*)(&(this->entries[0]));
        const uint8_t* q = (const uint8_t*)(&(rhs.entries[0]));
        if (p == q) return true;
        for (uint16_t i = 0; i < (sizeof(entries)); ++i) {
            if (*p != *q) return false;
            ++p;
            ++q;
        }
        return true;
    }
    /// @copydoc RGBPalette16::operator!=
    bool operator!=(const RGBPalette256& rhs) const { return !(*this == rhs); }

    /// @copydoc RGBPalette16::operator[]
    inline RGB& operator[](uint8_t x) __attribute__((always_inline)) {
        return entries[x];
    }
    /// @copydoc RGBPalette16::operator[]
    inline const RGB& operator[](uint8_t x) const
        __attribute__((always_inline)) {
        return entries[x];
    }

    /// @copydoc RGBPalette16::operator[]
    inline RGB& operator[](int x) __attribute__((always_inline)) {
        return entries[(uint8_t)x];
    }
    /// @copydoc RGBPalette16::operator[]
    inline const RGB& operator[](int x) const __attribute__((always_inline)) {
        return entries[(uint8_t)x];
    }

    /// Get the underlying pointer to the HSV entries making up the palette
    operator RGB*() { return &(entries[0]); }

    /// @copydoc RGBPalette16::RGBPalette16(const HSV&)
    RGBPalette256(const HSV& c1) { fill_solid(&(entries[0]), 256, c1); }
    /// @copydoc RGBPalette16::RGBPalette16(const HSV&, const HSV&)
    RGBPalette256(const HSV& c1, const HSV& c2) {
        fill_gradient(&(entries[0]), 256, c1, c2);
    }
    /// @copydoc RGBPalette16::RGBPalette16(const HSV&, const HSV&, const HSV&)
    RGBPalette256(const HSV& c1, const HSV& c2, const HSV& c3) {
        fill_gradient(&(entries[0]), 256, c1, c2, c3);
    }
    /// @copydoc RGBPalette16::RGBPalette16(const HSV&, const HSV&, const HSV&,
    /// const HSV&)
    RGBPalette256(const HSV& c1, const HSV& c2, const HSV& c3, const HSV& c4) {
        fill_gradient(&(entries[0]), 256, c1, c2, c3, c4);
    }

    /// @copydoc RGBPalette16::RGBPalette16(const RGB&)
    RGBPalette256(const RGB& c1) { fill_solid(&(entries[0]), 256, c1); }
    /// @copydoc RGBPalette16::RGBPalette16(const RGB&, const RGB&)
    RGBPalette256(const RGB& c1, const RGB& c2) {
        fill_gradient_RGB(&(entries[0]), 256, c1, c2);
    }
    /// @copydoc RGBPalette16::RGBPalette16(const RGB&, const RGB&, const RGB&)
    RGBPalette256(const RGB& c1, const RGB& c2, const RGB& c3) {
        fill_gradient_RGB(&(entries[0]), 256, c1, c2, c3);
    }
    /// @copydoc RGBPalette16::RGBPalette16(const RGB&, const RGB&, const RGB&,
    /// const RGB&)
    RGBPalette256(const RGB& c1, const RGB& c2, const RGB& c3, const RGB& c4) {
        fill_gradient_RGB(&(entries[0]), 256, c1, c2, c3, c4);
    }

    /// @copydoc
    /// RGBPalette16::loadDynamicGradientPalette(TDynamiRGBGradientPalette_bytes)
    RGBPalette256& loadDynamicGradientPalette(
        TDynamiRGBGradientPalette_bytes gpal) {
        TRGBGradientPaletteEntryUnion* ent =
            (TRGBGradientPaletteEntryUnion*)(gpal);
        TRGBGradientPaletteEntryUnion u;
        u = *ent;
        RGB rgbstart(u.r, u.g, u.b);

        int indexstart = 0;
        while (indexstart < 255) {
            ++ent;
            u = *ent;
            int indexend = u.index;
            RGB rgbend(u.r, u.g, u.b);
            fill_gradient_RGB(&(entries[0]), indexstart, rgbstart, indexend,
                              rgbend);
            indexstart = indexend;
            rgbstart = rgbend;
        }
        return *this;
    }
};

/// @} PaletteClasses

/// @defgroup PaletteColors Palette Color Functions
/// Functions to retrieve smooth color data from palettes
/// @{

/// Color interpolation options for palette
typedef enum {
    NOBLEND = 0,      ///< No interpolation between palette entries
    LINEARBLEND = 1,  ///< Linear interpolation between palette entries, with
                      ///< wrap-around from end to the beginning again
    LINEARBLEND_NOWRAP =
        2  ///< Linear interpolation between palette entries, but no wrap-around
} TBlendType;

/// Get a color from a palette.
/// These are the main functions for getting and using palette colors.
/// Regardless of the number of entries in the base palette, this function will
/// interpolate between entries to turn the discrete colors into a smooth
/// gradient.
/// @param pal the palette to retrieve the color from
/// @param index the position in the palette to retrieve the color for (0-255)
/// @param brightness optional brightness value to scale the resulting color
/// @param blendType whether to take the palette entries directly (NOBLEND)
/// or blend linearly between palette entries (LINEARBLEND)
RGB ColorFromPalette(const RGBPalette16& pal, uint8_t index,
                     uint8_t brightness = 255,
                     TBlendType blendType = LINEARBLEND);

/// @copydoc ColorFromPalette(const RGBPalette16&, uint8_t, uint8_t, TBlendType)
RGB ColorFromPalette(const RGBPalette256& pal, uint8_t index,
                     uint8_t brightness = 255, TBlendType blendType = NOBLEND);

/// @copydoc ColorFromPalette(const RGBPalette16&, uint8_t, uint8_t, TBlendType)
HSV ColorFromPalette(const HSVPalette16& pal, uint8_t index,
                     uint8_t brightness = 255,
                     TBlendType blendType = LINEARBLEND);

/// @copydoc ColorFromPalette(const RGBPalette16&, uint8_t, uint8_t, TBlendType)
HSV ColorFromPalette(const HSVPalette256& pal, uint8_t index,
                     uint8_t brightness = 255, TBlendType blendType = NOBLEND);

/// @copydoc ColorFromPalette(const RGBPalette16&, uint8_t, uint8_t, TBlendType)
RGB ColorFromPalette(const RGBPalette32& pal, uint8_t index,
                     uint8_t brightness = 255,
                     TBlendType blendType = LINEARBLEND);

/// @copydoc ColorFromPalette(const RGBPalette16&, uint8_t, uint8_t, TBlendType)
HSV ColorFromPalette(const HSVPalette32& pal, uint8_t index,
                     uint8_t brightness = 255,
                     TBlendType blendType = LINEARBLEND);

/// Fill a range of LEDs with a sequence of entries from a palette
/// @tparam PALETTE the type of the palette used (auto-deduced)
/// @param L pointer to the LED array to fill
/// @param N number of LEDs to fill in the array
/// @param startIndex the starting color index in the palette
/// @param incIndex how much to increment the palette color index per LED
/// @param pal the color palette to pull colors from
/// @param brightness brightness value used to scale the resulting color
/// @param blendType whether to take the palette entries directly (NOBLEND)
/// or blend linearly between palette entries (LINEARBLEND)
template <typename PALETTE>
void fill_palette(RGB* L, uint16_t N, uint8_t startIndex, uint8_t incIndex,
                  const PALETTE& pal, uint8_t brightness = 255,
                  TBlendType blendType = LINEARBLEND) {
    uint8_t colorIndex = startIndex;
    for (uint16_t i = 0; i < N; ++i) {
        L[i] = ColorFromPalette(pal, colorIndex, brightness, blendType);
        colorIndex += incIndex;
    }
}

/// Fill a range of LEDs with a sequence of entries from a palette, so that
/// the entire palette smoothly covers the range of LEDs.
/// @tparam PALETTE the type of the palette used (auto-deduced)
/// @param L pointer to the LED array to fill
/// @param N number of LEDs to fill in the array
/// @param startIndex the starting color index in the palette
/// @param pal the color palette to pull colors from
/// @param brightness brightness value used to scale the resulting color
/// @param blendType whether to take the palette entries directly (NOBLEND)
/// or blend linearly between palette entries (LINEARBLEND)
/// @param reversed whether to progress through the palette backwards
template <typename PALETTE>
void fill_palette_circular(RGB* L, uint16_t N, uint8_t startIndex,
                           const PALETTE& pal, uint8_t brightness = 255,
                           TBlendType blendType = LINEARBLEND,
                           bool reversed = false) {
    if (N == 0) return;  // avoiding div/0

    const uint16_t colorChange =
        65535 / N;  // color change for each LED, * 256 for precision
    uint16_t colorIndex =
        ((uint16_t)startIndex)
        << 8;  // offset for color index, with precision (*256)

    for (uint16_t i = 0; i < N; ++i) {
        L[i] = ColorFromPalette(pal, (colorIndex >> 8), brightness, blendType);
        if (reversed)
            colorIndex -= colorChange;
        else
            colorIndex += colorChange;
    }
}

/// Maps an array of palette color indexes into an array of LED colors.
///
/// This function provides an easy way to create lightweight color patterns that
/// can be deployed using any palette.
///
/// @param dataArray the source array, containing color indexes for the palette
/// @param dataCount the number of data elements in the array
/// @param targetColorArray the LED array to store the resulting colors into.
/// Must be at least as long as `dataCount`.
/// @param pal the color palette to pull colors from
/// @param brightness optional brightness value used to scale the resulting
/// color
/// @param opacity optional opacity value for the new color. If this is 255
/// (default), the new colors will be written to the array directly. Otherwise
/// the existing LED data will be scaled down using `RGB::nscale8_video()` and
/// then new colors will be added on top. A higher value means that the new
/// colors will be more visible.
/// @param blendType whether to take the palette entries directly (NOBLEND)
/// or blend linearly between palette entries (LINEARBLEND)
template <typename PALETTE>
void map_data_into_colors_through_palette(
    uint8_t* dataArray, uint16_t dataCount, RGB* targetColorArray,
    const PALETTE& pal, uint8_t brightness = 255, uint8_t opacity = 255,
    TBlendType blendType = LINEARBLEND) {
    for (uint16_t i = 0; i < dataCount; ++i) {
        uint8_t d = dataArray[i];
        RGB rgb = ColorFromPalette(pal, d, brightness, blendType);
        if (opacity == 255) {
            targetColorArray[i] = rgb;
        } else {
            targetColorArray[i].nscale8(256 - opacity);
            rgb.nscale8_video(opacity);
            targetColorArray[i] += rgb;
        }
    }
}

/// Alter one palette by making it slightly more like a "target palette".
/// Used for palette cross-fades.
///
/// It does this by comparing each of the R, G, and B channels
/// of each entry in the current palette to the corresponding
/// entry in the target palette and making small adjustments:
///   * If the RGB::red channel is too low, it will be increased.
///   * If the RGB::red channel is too high, it will be slightly reduced.
///
/// ...and so on and so forth for the RGB::green and RGB::blue channels.
///
/// Additionally, there are two significant visual improvements
/// to this algorithm implemented here. First is this:
///   * When increasing a channel, it is stepped up by ONE.
///   * When decreasing a channel, it is stepped down by TWO.
///
/// Due to the way the eye perceives light, and the way colors
/// are represented in RGB, this produces a more uniform apparent
/// brightness when cross-fading between most palette colors.
///
/// The second visual tweak is limiting the number of changes
/// that will be made to the palette at once. If all the palette
/// entries are changed at once, it can give a muddled appearance.
/// However, if only a *few* palette entries are changed at once,
/// you get a visually smoother transition: in the middle of the
/// cross-fade your current palette will actually contain some
/// colors from the old palette, a few blended colors, and some
/// colors from the new palette.
///
/// @param currentPalette the palette to modify
/// @param targetPalette the palette to move towards
/// @param maxChanges the maximum number of possible palette changes
/// to make to the color channels per call. The limit is 48 (16 color
/// entries times 3 channels each). The default is 24, meaning that
/// only half of the palette entries can be changed per call.
/// @warning The palette passed as `currentPalette` will be modified! Be sure
/// to make a copy beforehand if needed.
/// @todo Shouldn't the `targetPalette` be `const`?
void nblendPaletteTowardPalette(RGBPalette16& currentPalette,
                                RGBPalette16& targetPalette,
                                uint8_t maxChanges = 24);

/// @} PaletteColors

/// Defines a static RGB palette very compactly using a series
/// of connected color gradients.
///
/// For example, if you want the first 3/4ths of the palette to be a slow
/// gradient ramping from black to red, and then the remaining 1/4 of the
/// palette to be a quicker ramp to white, you specify just three points: the
/// starting black point (at index 0), the red midpoint (at index 192),
/// and the final white point (at index 255).  It looks like this:
///   @code
///   index:  0                                    192          255
///           |----------r-r-r-rrrrrrrrRrRrRrRrRRRR-|-RRWRWWRWWW-|
///   color: (0,0,0)                           (255,0,0)    (255,255,255)
///   @endcode
///
/// Here's how you'd define that gradient palette using this macro:
///   @code{.cpp}
///   DEFINE_GRADIENT_PALETTE( black_to_red_to_white_p ) {
///        0,    0,   0,   0,  /* at index 0,   black(0,0,0) */
///       192, 255,   0,   0,  /* at index 192, red(255,0,0) */
///       255, 255, 255, 255   /* at index 255, white(255,255,255) */
///   };
///   @endcode
///
/// This format is designed for compact storage.  The example palette here
/// takes up just 12 bytes of PROGMEM (flash) storage, and zero bytes
/// of SRAM when not currently in use.
///
/// To use one of these gradient palettes, simply assign it into a
/// RGBPalette16 or a RGBPalette256, like this:
///   @code{.cpp}
///   RGBPalette16 pal = black_to_red_to_white_p;
///   @endcode
///
/// When the assignment is made, the gradients are expanded out into
/// either 16 or 256 palette entries, depending on the kind of palette
/// object they're assigned to.
///
/// @warning The last "index" position **MUST** be 255! Failure to end
/// with index 255 will result in program hangs or crashes.
/// @par
/// @warning At this point, these gradient palette definitions **MUST**
/// be stored in PROGMEM on AVR-based Arduinos. If you use the
/// `DEFINE_GRADIENT_PALETTE` macro, this is taken of automatically.
///
#define DEFINE_GRADIENT_PALETTE(X) \
    FL_ALIGN_PROGMEM               \
    extern const TProgmemRGBGradientPalette_byte X[] FL_PROGMEM =

/// Forward-declaration macro for DEFINE_GRADIENT_PALETTE(X)
#define DECLARE_GRADIENT_PALETTE(X) \
    FL_ALIGN_PROGMEM                \
    extern const TProgmemRGBGradientPalette_byte X[] FL_PROGMEM

/// @} ColorPalettes

/// @defgroup GammaFuncs Gamma Adjustment Functions
/// Functions for applying gamma adjustments to LED data.
///
/// Gamma correction tries to compensate for the non-linear
/// manner in which humans perceive light and color. Gamma
/// correction is applied using the following expression:
///   @code{.cpp}
///   output = pow(input / 255.0, gamma) * 255.0;
///   @endcode
///
/// Larger gamma values result in darker images that have more contrast.
/// Lower gamma values result in lighter images with less contrast.
///
/// These functions apply either:
///  * a single gamma adjustment to a single scalar value
///  * a single gamma adjustment to each channel of a RGB color, or
///  * different gamma adjustments for each channel of a RGB color
///
/// Note that the gamma is specified as a traditional floating point value,
/// e.g., "2.5", and as such these functions should not be called in
/// your innermost pixel loops, or in animations that are extremely
/// low on program storage space.  Nevertheless, if you need these
/// functions, here they are.
///
/// Furthermore, bear in mind that RGB LEDs have only eight bits
/// per channel of color resolution, and that very small, subtle shadings
/// may not be visible.
///
/// @see @ref Dimming
/// @see https://en.wikipedia.org/wiki/Gamma_correction
/// @{

/// Applies a gamma adjustment to a color channel
/// @param brightness the value of the color data
/// @param gamma the gamma value to apply
/// @returns the color data, adjusted for gamma
uint8_t applyGamma_video(uint8_t brightness, float gamma);

/// Applies a gamma adjustment to a color
/// @param orig the color to apply an adjustment to
/// @param gamma the gamma value to apply
/// @returns copy of the RGB object with gamma adjustment applied
RGB applyGamma_video(const RGB& orig, float gamma);

/// Applies a gamma adjustment to a color
/// @param orig the color to apply an adjustment to
/// @param gammaR the gamma value to apply to the RGB::red channel
/// @param gammaG the gamma value to apply to the RGB::green channel
/// @param gammaB the gamma value to apply to the RGB::blue channel
/// @returns copy of the RGB object with gamma adjustment applied
RGB applyGamma_video(const RGB& orig, float gammaR, float gammaG, float gammaB);

/// Destructively applies a gamma adjustment to a color
/// @param rgb the color to apply an adjustment to (modified in place)
/// @param gamma the gamma value to apply
RGB& napplyGamma_video(RGB& rgb, float gamma);

/// Destructively applies a gamma adjustment to a color
/// @param rgb the color to apply an adjustment to (modified in place)
/// @param gammaR the gamma value to apply to the RGB::red channel
/// @param gammaG the gamma value to apply to the RGB::green channel
/// @param gammaB the gamma value to apply to the RGB::blue channel
RGB& napplyGamma_video(RGB& rgb, float gammaR, float gammaG, float gammaB);

/// Destructively applies a gamma adjustment to a color array
/// @param rgbarray pointer to an LED array to apply an adjustment to (modified
/// in place)
/// @param count the number of LEDs to modify
/// @param gamma the gamma value to apply
void napplyGamma_video(RGB* rgbarray, uint16_t count, float gamma);

/// Destructively applies a gamma adjustment to a color array
/// @param rgbarray pointer to an LED array to apply an adjustment to (modified
/// in place)
/// @param count the number of LEDs to modify
/// @param gammaR the gamma value to apply to the RGB::red channel
/// @param gammaG the gamma value to apply to the RGB::green channel
/// @param gammaB the gamma value to apply to the RGB::blue channel
void napplyGamma_video(RGB* rgbarray, uint16_t count, float gammaR,
                       float gammaG, float gammaB);

/// @} GammaFuncs

#endif
