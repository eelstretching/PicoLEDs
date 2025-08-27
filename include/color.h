#ifndef COLOR_H
#define COLOR_H

#pragma once

#include <stdio.h>

#include "math8.h"
#include "pico/stdlib.h"
#include "pico/types.h"

struct RGB;
struct HSV;

static void hsv2rgb_rainbow(const HSV& hsv, RGB& rgb);

/// RGB color channel orderings, used when instantiating controllers to
/// determine what order the controller should send data out in. The default
/// ordering is RGB. Within this enum, the red channel is 0, the green channel
/// is 1, and the blue chanel is 2.
enum ColorOrder {
    ORGB = 0012,  ///< Red,   Green, Blue  (0012)
    ORBG = 0021,  ///< Red,   Blue,  Green (0021)
    OGRB = 0102,  ///< Green, Red,   Blue  (0102)
    OGBR = 0120,  ///< Green, Blue,  Red   (0120)
    OBRG = 0201,  ///< Blue,  Red,   Green (0201)
    OBGR = 0210   ///< Blue,  Green, Red   (0210)
};

struct HSV {
    union {
        struct {
            union {
                /// Color hue.
                /// This is an 8-bit value representing an angle around
                /// the color wheel. Where 0 is 0°, and 255 is 358°.
                uint8_t hue;
                uint8_t h;  ///< @copydoc hue
            };
            union {
                /// Color saturation.
                /// This is an 8-bit value representing a percentage.
                uint8_t saturation;
                uint8_t sat;  ///< @copydoc saturation
                uint8_t s;    ///< @copydoc saturation
            };
            union {
                /// Color value (brightness).
                /// This is an 8-bit value representing a percentage.
                uint8_t value;
                uint8_t val;  ///< @copydoc value
                uint8_t v;    ///< @copydoc value
            };
        };
        /// Access the hue, saturation, and value data as an array.
        /// Where:
        /// * `raw[0]` is the hue
        /// * `raw[1]` is the saturation
        /// * `raw[2]` is the value
        uint8_t raw[3];
    };

    char* toString(char* b, uint bs) {
        snprintf(b, bs, "%03d %03d %03d", raw[0], raw[1], raw[2]);
        return b;
    }

    /// Array access operator to index into the HSV object
    /// @param x the index to retrieve (0-2)
    /// @returns the HSV::raw value for the given index
    inline uint8_t& operator[](uint8_t x) { return raw[x]; }

    /// @copydoc operator[]
    inline const uint8_t& operator[](uint8_t x) const { return raw[x]; }

    /// Default constructor
    /// @warning Default values are UNITIALIZED!
    inline HSV() = default;

    /// Allow construction from hue, saturation, and value
    /// @param ih input hue
    /// @param is input saturation
    /// @param iv input value
    inline HSV(uint8_t ih, uint8_t is, uint8_t iv) : h(ih), s(is), v(iv) {}

    /// Allow copy construction
    inline HSV(const HSV& rhs) = default;

    /// Allow copy construction
    inline HSV& operator=(const HSV& rhs) = default;

    /// Assign new HSV values
    /// @param ih input hue
    /// @param is input saturation
    /// @param iv input value
    /// @returns reference to the HSV object
    inline HSV& setHSV(uint8_t ih, uint8_t is, uint8_t iv) {
        h = ih;
        s = is;
        v = iv;
        return *this;
    }
};

/// Representation of an RGB pixel (Red, Green, Blue)
struct RGB {
    union {
        struct {
            union {
                uint8_t r;    ///< Red channel value
                uint8_t red;  ///< @copydoc r
            };
            union {
                uint8_t g;      ///< Green channel value
                uint8_t green;  ///< @copydoc g
            };
            union {
                uint8_t b;     ///< Blue channel value
                uint8_t blue;  ///< @copydoc b
            };
        };
        /// Access the red, green, and blue data as an array.
        /// Where:
        /// * `raw[0]` is the red value
        /// * `raw[1]` is the green value
        /// * `raw[2]` is the blue value
        uint8_t raw[3];
    };

    char* toString(char* b, uint bs) {
        snprintf(b, bs, "%02X%02X%02X", raw[0], raw[1], raw[2]);
        return b;
    }

    /// Array access operator to index into the RGB object
    /// @param x the index to retrieve (0-2)
    /// @returns the RGB::raw value for the given index
    inline uint8_t& operator[](uint8_t x) { return raw[x]; }

    /// Array access operator to index into the RGB object
    /// @param x the index to retrieve (0-2)
    /// @returns the RGB::raw value for the given index
    inline const uint8_t& operator[](uint8_t x) const { return raw[x]; }

    /// Default constructor
    /// @warning Default values are UNITIALIZED!
    inline RGB() = default;

    /// Allow construction from red, green, and blue
    /// @param ir input red value
    /// @param ig input green value
    /// @param ib input blue value
    inline RGB(uint8_t ir, uint8_t ig, uint8_t ib) : r(ir), g(ig), b(ib) {}

    /// Allow construction from 32-bit (really 24-bit) bit 0xRRGGBB color code
    /// @param colorcode a packed 24 bit color code
    inline RGB(uint32_t colorcode)
        : r((colorcode >> 16) & 0xFF),
          g((colorcode >> 8) & 0xFF),
          b((colorcode >> 0) & 0xFF) {}

    /// Allow copy construction
    inline RGB(const RGB& rhs) = default;

    /// Allow construction from a HSV color
    inline RGB(const HSV& rhs) { hsv2rgb_rainbow(rhs, *this); }

    /// Allow assignment from one RGB struct to another
    inline RGB& operator=(const RGB& rhs) = default;

    /// Allow assignment from 32-bit (really 24-bit) 0xRRGGBB color code
    /// @param colorcode a packed 24 bit color code
    inline RGB& operator=(const uint32_t colorcode) {
        r = (colorcode >> 16) & 0xFF;
        g = (colorcode >> 8) & 0xFF;
        b = (colorcode >> 0) & 0xFF;
        return *this;
    }

    /// Allow assignment from red, green, and blue
    /// @param nr new red value
    /// @param ng new green value
    /// @param nb new blue value
    inline RGB& setRGB(uint8_t nr, uint8_t ng, uint8_t nb) {
        r = nr;
        g = ng;
        b = nb;
        return *this;
    }

    /// Allow assignment from hue, saturation, and value
    /// @param hue color hue
    /// @param sat color saturation
    /// @param val color value (brightness)
    inline RGB& setHSV(uint8_t hue, uint8_t sat, uint8_t val) {
        hsv2rgb_rainbow(HSV(hue, sat, val), *this);
        return *this;
    }

    /// Allow assignment from just a hue.
    /// Saturation and value (brightness) are set automatically to max.
    /// @param hue color hue
    inline RGB& setHue(uint8_t hue) {
        hsv2rgb_rainbow(HSV(hue, 255, 255), *this);
        return *this;
    }

    /// Allow assignment from HSV color
    inline RGB& operator=(const HSV& rhs) {
        hsv2rgb_rainbow(rhs, *this);
        return *this;
    }

    /// Allow assignment from 32-bit (really 24-bit) 0xRRGGBB color code
    /// @param colorcode a packed 24 bit color code
    inline RGB& setColorCode(uint32_t colorcode) {
        r = (colorcode >> 16) & 0xFF;
        g = (colorcode >> 8) & 0xFF;
        b = (colorcode >> 0) & 0xFF;
        return *this;
    }

    /// Add one RGB to another, saturating at 0xFF for each channel
    inline RGB& operator+=(const RGB& rhs) {
        r = qadd8(r, rhs.r);
        g = qadd8(g, rhs.g);
        b = qadd8(b, rhs.b);
        return *this;
    }

    /// Add a constant to each channel, saturating at 0xFF.
    /// @note This is NOT an operator+= overload because the compiler
    /// can't usefully decide when it's being passed a 32-bit
    /// constant (e.g. RGB::Red) and an 8-bit one (RGB::Blue)
    inline RGB& addToRGB(uint8_t d) {
        r = qadd8(r, d);
        g = qadd8(g, d);
        b = qadd8(b, d);
        return *this;
    }

    /// Subtract one RGB from another, saturating at 0x00 for each channel
    inline RGB& operator-=(const RGB& rhs) {
        r = qsub8(r, rhs.r);
        g = qsub8(g, rhs.g);
        b = qsub8(b, rhs.b);
        return *this;
    }

    /// Subtract a constant from each channel, saturating at 0x00.
    /// @note This is NOT an operator+= overload because the compiler
    /// can't usefully decide when it's being passed a 32-bit
    /// constant (e.g. RGB::Red) and an 8-bit one (RGB::Blue)
    inline RGB& subtractFromRGB(uint8_t d) {
        r = qsub8(r, d);
        g = qsub8(g, d);
        b = qsub8(b, d);
        return *this;
    }

    /// Subtract a constant of '1' from each channel, saturating at 0x00
    inline RGB& operator--() {
        subtractFromRGB(1);
        return *this;
    }

    /// @copydoc operator--
    inline RGB operator--(int) {
        RGB retval(*this);
        --(*this);
        return retval;
    }

    /// Add a constant of '1' from each channel, saturating at 0xFF
    inline RGB& operator++() {
        addToRGB(1);
        return *this;
    }

    /// @copydoc operator++
    inline RGB operator++(int) {
        RGB retval(*this);
        ++(*this);
        return retval;
    }

    /// Divide each of the channels by a constant
    inline RGB& operator/=(uint8_t d) {
        r /= d;
        g /= d;
        b /= d;
        return *this;
    }

    /// Right shift each of the channels by a constant
    inline RGB& operator>>=(uint8_t d) {
        r >>= d;
        g >>= d;
        b >>= d;
        return *this;
    }

    /// Multiply each of the channels by a constant,
    /// saturating each channel at 0xFF.
    inline RGB& operator*=(uint8_t d) {
        r = qmul8(r, d);
        g = qmul8(g, d);
        b = qmul8(b, d);
        return *this;
    }

    /// Scale down a RGB to N/256ths of it's current brightness using
    /// "video" dimming rules. "Video" dimming rules means that unless the scale
    /// factor is ZERO each channel is guaranteed NOT to dim down to zero.  If
    /// it's already nonzero, it'll stay nonzero, even if that means the hue
    /// shifts a little at low brightness levels.
    /// @see nscale8x3_video
    inline RGB& nscale8_video(uint8_t scaledown) {
        nscale8x3_video(r, g, b, scaledown);
        return *this;
    }

    /// %= is a synonym for nscale8_video().  Think of it is scaling down
    /// by "a percentage"
    inline RGB& operator%=(uint8_t scaledown) {
        nscale8x3_video(r, g, b, scaledown);
        return *this;
    }

    /// fadeLightBy is a synonym for nscale8_video(), as a fade instead of a
    /// scale
    /// @param fadefactor the amount to fade, sent to nscale8_video() as (255 -
    /// fadefactor)
    inline RGB& fadeLightBy(uint8_t fadefactor) {
        nscale8x3_video(r, g, b, 255 - fadefactor);
        return *this;
    }

    /// Scale down a RGB to N/256ths of its current brightness, using
    /// "plain math" dimming rules. "Plain math" dimming rules means that the
    /// low light levels may dim all the way to 100% black.
    /// @see nscale8x3
    inline RGB& nscale8(uint8_t scaledown) {
        nscale8x3(r, g, b, scaledown);
        return *this;
    }

    /// Scale down a RGB to N/256ths of its current brightness, using
    /// "plain math" dimming rules. "Plain math" dimming rules means that the
    /// low light levels may dim all the way to 100% black.
    /// @see ::scale8
    inline RGB& nscale8(const RGB& scaledown) {
        r = ::scale8(r, scaledown.r);
        g = ::scale8(g, scaledown.g);
        b = ::scale8(b, scaledown.b);
        return *this;
    }

    /// Return a RGB object that is a scaled down version of this object
    inline RGB scale8(uint8_t scaledown) const {
        RGB out = *this;
        nscale8x3(out.r, out.g, out.b, scaledown);
        return out;
    }

    /// Return a RGB object that is a scaled down version of this object
    inline RGB scale8(const RGB& scaledown) const {
        RGB out;
        out.r = ::scale8(r, scaledown.r);
        out.g = ::scale8(g, scaledown.g);
        out.b = ::scale8(b, scaledown.b);
        return out;
    }

    /// fadeToBlackBy is a synonym for nscale8(), as a fade instead of a scale
    /// @param fadefactor the amount to fade, sent to nscale8() as (255 -
    /// fadefactor)
    inline RGB& fadeToBlackBy(uint8_t fadefactor) {
        nscale8x3(r, g, b, 255 - fadefactor);
        return *this;
    }

    /// "or" operator brings each channel up to the higher of the two values
    inline RGB& operator|=(const RGB& rhs) {
        if (rhs.r > r) r = rhs.r;
        if (rhs.g > g) g = rhs.g;
        if (rhs.b > b) b = rhs.b;
        return *this;
    }

    /// @copydoc operator|=
    inline RGB& operator|=(uint8_t d) {
        if (d > r) r = d;
        if (d > g) g = d;
        if (d > b) b = d;
        return *this;
    }

    /// "and" operator brings each channel down to the lower of the two values
    inline RGB& operator&=(const RGB& rhs) {
        if (rhs.r < r) r = rhs.r;
        if (rhs.g < g) g = rhs.g;
        if (rhs.b < b) b = rhs.b;
        return *this;
    }

    /// @copydoc operator&=
    inline RGB& operator&=(uint8_t d) {
        if (d < r) r = d;
        if (d < g) g = d;
        if (d < b) b = d;
        return *this;
    }

    /// This allows testing a RGB for zero-ness
    inline explicit operator bool() const { return r || g || b; }

    /// Converts a RGB to a 32-bit color having an alpha of 255.
    inline explicit operator uint32_t() const {
        return uint32_t{0xff000000} | (uint32_t{r} << 16) | (uint32_t{g} << 8) |
               uint32_t{b};
    }

    uint32_t getColor() {
        return uint32_t{0xff000000} | (uint32_t{r} << 16) | (uint32_t{g} << 8) |
               uint32_t{b};
    }

    uint32_t getColor(ColorOrder order) {
        switch (order) {
            case ORGB:
                return ((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) |
                       (uint32_t)(b);
            case ORBG:
                return ((uint32_t)(r) << 16) | ((uint32_t)(b) << 8) |
                       (uint32_t)(g);
            case OGRB:
                return ((uint32_t)(g) << 16) | ((uint32_t)(r) << 8) |
                       (uint32_t)(b);
            case OGBR:
                return ((uint32_t)(g) << 16) | ((uint32_t)(b) << 8) |
                       (uint32_t)(r);
            case OBRG:
                return ((uint32_t)(b) << 16) | ((uint32_t)(r) << 8) |
                       (uint32_t)(g);
            case OBGR:
                return ((uint32_t)(b) << 16) | ((uint32_t)(g) << 8) |
                       (uint32_t)(r);
            default:
                return ((uint32_t)(g) << 16) | ((uint32_t)(r) << 8) |
                       (uint32_t)(b);
        }
    }

    /// Invert each channel
    inline RGB operator-() const {
        RGB retval;
        retval.r = 255 - r;
        retval.g = 255 - g;
        retval.b = 255 - b;
        return retval;
    }

    /// Get the "luma" of a RGB object. In other words, roughly how much
    /// light the RGB pixel is putting out (from 0 to 255).
    inline uint8_t getLuma() const {
        // Y' = 0.2126 R' + 0.7152 G' + 0.0722 B'
        //      54            183       18 (!)

        uint8_t luma = scale8_LEAVING_R1_DIRTY(r, 54) +
                       scale8_LEAVING_R1_DIRTY(g, 183) +
                       scale8_LEAVING_R1_DIRTY(b, 18);
        return luma;
    }

    /// Get the average of the R, G, and B values
    inline uint8_t getAverageLight() const {
#if FASTLED_SCALE8_FIXED == 1
        const uint8_t eightyfive = 85;
#else
        const uint8_t eightyfive = 86;
#endif
        uint8_t avg = scale8_LEAVING_R1_DIRTY(r, eightyfive) +
                      scale8_LEAVING_R1_DIRTY(g, eightyfive) +
                      scale8_LEAVING_R1_DIRTY(b, eightyfive);
        return avg;
    }

    /// Maximize the brightness of this RGB object.
    /// This makes the individual color channels as bright as possible
    /// while keeping the same value differences between channels.
    /// @note This does not keep the same ratios between channels,
    /// just the same difference in absolute values.
    inline void maximizeBrightness(uint8_t limit = 255) {
        uint8_t max = red;
        if (green > max) max = green;
        if (blue > max) max = blue;

        // stop div/0 when color is black
        if (max > 0) {
            uint16_t factor = ((uint16_t)(limit)*256) / max;
            red = (red * factor) / 256;
            green = (green * factor) / 256;
            blue = (blue * factor) / 256;
        }
    }

    /// Return a new RGB object after performing a linear interpolation between
    /// this object and the passed in object
    inline RGB lerp8(const RGB& other, fract8 frac) const {
        RGB ret;

        ret.r = lerp8by8(r, other.r, frac);
        ret.g = lerp8by8(g, other.g, frac);
        ret.b = lerp8by8(b, other.b, frac);

        return ret;
    }

    /// Returns 0 or 1, depending on the lowest bit of the sum of the color
    /// components.
    inline uint8_t getParity() {
        uint8_t sum = r + g + b;
        return (sum & 0x01);
    }

    /// Adjusts the color in the smallest way possible
    /// so that the parity of the coloris now the desired value.
    /// This allows you to "hide" one bit of information in the color.
    ///
    /// Ideally, we find one color channel which already
    /// has data in it, and modify just that channel by one.
    /// We don't want to light up a channel that's black
    /// if we can avoid it, and if the pixel is 'grayscale',
    /// (meaning that R==G==B), we modify all three channels
    /// at once, to preserve the neutral hue.
    ///
    /// There's no such thing as a free lunch; in many cases
    /// this "hidden bit" may actually be visible, but this
    /// code makes reasonable efforts to hide it as much
    /// as is reasonably possible.
    ///
    /// Also, an effort is made to make it such that
    /// repeatedly setting the parity to different values
    /// will not cause the color to "drift". Toggling
    /// the parity twice should generally result in the
    /// original color again.
    ///
    inline void setParity(uint8_t parity) {
        uint8_t curparity = getParity();

        if (parity == curparity) return;

        if (parity) {
            // going 'up'
            if ((b > 0) && (b < 255)) {
                if (r == g && g == b) {
                    ++r;
                    ++g;
                }
                ++b;
            } else if ((r > 0) && (r < 255)) {
                ++r;
            } else if ((g > 0) && (g < 255)) {
                ++g;
            } else {
                if (r == g && g == b) {
                    r ^= 0x01;
                    g ^= 0x01;
                }
                b ^= 0x01;
            }
        } else {
            // going 'down'
            if (b > 1) {
                if (r == g && g == b) {
                    --r;
                    --g;
                }
                --b;
            } else if (g > 1) {
                --g;
            } else if (r > 1) {
                --r;
            } else {
                if (r == g && g == b) {
                    r ^= 0x01;
                    g ^= 0x01;
                }
                b ^= 0x01;
            }
        }
    }

    /// Predefined RGB colors
    typedef enum {
        AliceBlue = 0xF0F8FF,             ///< @htmlcolorblock{F0F8FF}
        Amethyst = 0x9966CC,              ///< @htmlcolorblock{9966CC}
        AntiqueWhite = 0xFAEBD7,          ///< @htmlcolorblock{FAEBD7}
        Aqua = 0x00FFFF,                  ///< @htmlcolorblock{00FFFF}
        Aquamarine = 0x7FFFD4,            ///< @htmlcolorblock{7FFFD4}
        Azure = 0xF0FFFF,                 ///< @htmlcolorblock{F0FFFF}
        Beige = 0xF5F5DC,                 ///< @htmlcolorblock{F5F5DC}
        Bisque = 0xFFE4C4,                ///< @htmlcolorblock{FFE4C4}
        Black = 0x000000,                 ///< @htmlcolorblock{000000}
        BlanchedAlmond = 0xFFEBCD,        ///< @htmlcolorblock{FFEBCD}
        Blue = 0x0000FF,                  ///< @htmlcolorblock{0000FF}
        BlueViolet = 0x8A2BE2,            ///< @htmlcolorblock{8A2BE2}
        Brown = 0xA52A2A,                 ///< @htmlcolorblock{A52A2A}
        BurlyWood = 0xDEB887,             ///< @htmlcolorblock{DEB887}
        CadetBlue = 0x5F9EA0,             ///< @htmlcolorblock{5F9EA0}
        Chartreuse = 0x7FFF00,            ///< @htmlcolorblock{7FFF00}
        Chocolate = 0xD2691E,             ///< @htmlcolorblock{D2691E}
        Coral = 0xFF7F50,                 ///< @htmlcolorblock{FF7F50}
        CornflowerBlue = 0x6495ED,        ///< @htmlcolorblock{6495ED}
        Cornsilk = 0xFFF8DC,              ///< @htmlcolorblock{FFF8DC}
        Crimson = 0xDC143C,               ///< @htmlcolorblock{DC143C}
        Cyan = 0x00FFFF,                  ///< @htmlcolorblock{00FFFF}
        DarkBlue = 0x00008B,              ///< @htmlcolorblock{00008B}
        DarkCyan = 0x008B8B,              ///< @htmlcolorblock{008B8B}
        DarkGoldenrod = 0xB8860B,         ///< @htmlcolorblock{B8860B}
        DarkGray = 0xA9A9A9,              ///< @htmlcolorblock{A9A9A9}
        DarkGrey = 0xA9A9A9,              ///< @htmlcolorblock{A9A9A9}
        DarkGreen = 0x006400,             ///< @htmlcolorblock{006400}
        DarkKhaki = 0xBDB76B,             ///< @htmlcolorblock{BDB76B}
        DarkMagenta = 0x8B008B,           ///< @htmlcolorblock{8B008B}
        DarkOliveGreen = 0x556B2F,        ///< @htmlcolorblock{556B2F}
        DarkOrange = 0xFF8C00,            ///< @htmlcolorblock{FF8C00}
        DarkOrchid = 0x9932CC,            ///< @htmlcolorblock{9932CC}
        DarkRed = 0x8B0000,               ///< @htmlcolorblock{8B0000}
        DarkSalmon = 0xE9967A,            ///< @htmlcolorblock{E9967A}
        DarkSeaGreen = 0x8FBC8F,          ///< @htmlcolorblock{8FBC8F}
        DarkSlateBlue = 0x483D8B,         ///< @htmlcolorblock{483D8B}
        DarkSlateGray = 0x2F4F4F,         ///< @htmlcolorblock{2F4F4F}
        DarkSlateGrey = 0x2F4F4F,         ///< @htmlcolorblock{2F4F4F}
        DarkTurquoise = 0x00CED1,         ///< @htmlcolorblock{00CED1}
        DarkViolet = 0x9400D3,            ///< @htmlcolorblock{9400D3}
        DeepPink = 0xFF1493,              ///< @htmlcolorblock{FF1493}
        DeepSkyBlue = 0x00BFFF,           ///< @htmlcolorblock{00BFFF}
        DimGray = 0x696969,               ///< @htmlcolorblock{696969}
        DimGrey = 0x696969,               ///< @htmlcolorblock{696969}
        DodgerBlue = 0x1E90FF,            ///< @htmlcolorblock{1E90FF}
        FireBrick = 0xB22222,             ///< @htmlcolorblock{B22222}
        FloralWhite = 0xFFFAF0,           ///< @htmlcolorblock{FFFAF0}
        ForestGreen = 0x228B22,           ///< @htmlcolorblock{228B22}
        Fuchsia = 0xFF00FF,               ///< @htmlcolorblock{FF00FF}
        Gainsboro = 0xDCDCDC,             ///< @htmlcolorblock{DCDCDC}
        GhostWhite = 0xF8F8FF,            ///< @htmlcolorblock{F8F8FF}
        Gold = 0xFFD700,                  ///< @htmlcolorblock{FFD700}
        Goldenrod = 0xDAA520,             ///< @htmlcolorblock{DAA520}
        Gray = 0x808080,                  ///< @htmlcolorblock{808080}
        Grey = 0x808080,                  ///< @htmlcolorblock{808080}
        Green = 0x00FF00,                 ///< @htmlcolorblock{008000}
        GreenYellow = 0xADFF2F,           ///< @htmlcolorblock{ADFF2F}
        Honeydew = 0xF0FFF0,              ///< @htmlcolorblock{F0FFF0}
        HotPink = 0xFF69B4,               ///< @htmlcolorblock{FF69B4}
        IndianRed = 0xCD5C5C,             ///< @htmlcolorblock{CD5C5C}
        Indigo = 0x4B0082,                ///< @htmlcolorblock{4B0082}
        Ivory = 0xFFFFF0,                 ///< @htmlcolorblock{FFFFF0}
        Khaki = 0xF0E68C,                 ///< @htmlcolorblock{F0E68C}
        Lavender = 0xE6E6FA,              ///< @htmlcolorblock{E6E6FA}
        LavenderBlush = 0xFFF0F5,         ///< @htmlcolorblock{FFF0F5}
        LawnGreen = 0x7CFC00,             ///< @htmlcolorblock{7CFC00}
        LemonChiffon = 0xFFFACD,          ///< @htmlcolorblock{FFFACD}
        LightBlue = 0xADD8E6,             ///< @htmlcolorblock{ADD8E6}
        LightCoral = 0xF08080,            ///< @htmlcolorblock{F08080}
        LightCyan = 0xE0FFFF,             ///< @htmlcolorblock{E0FFFF}
        LightGoldenrodYellow = 0xFAFAD2,  ///< @htmlcolorblock{FAFAD2}
        LightGreen = 0x90EE90,            ///< @htmlcolorblock{90EE90}
        LightGrey = 0xD3D3D3,             ///< @htmlcolorblock{D3D3D3}
        LightPink = 0xFFB6C1,             ///< @htmlcolorblock{FFB6C1}
        LightSalmon = 0xFFA07A,           ///< @htmlcolorblock{FFA07A}
        LightSeaGreen = 0x20B2AA,         ///< @htmlcolorblock{20B2AA}
        LightSkyBlue = 0x87CEFA,          ///< @htmlcolorblock{87CEFA}
        LightSlateGray = 0x778899,        ///< @htmlcolorblock{778899}
        LightSlateGrey = 0x778899,        ///< @htmlcolorblock{778899}
        LightSteelBlue = 0xB0C4DE,        ///< @htmlcolorblock{B0C4DE}
        LightYellow = 0xFFFFE0,           ///< @htmlcolorblock{FFFFE0}
        Lime = 0x00FF00,                  ///< @htmlcolorblock{00FF00}
        LimeGreen = 0x32CD32,             ///< @htmlcolorblock{32CD32}
        Linen = 0xFAF0E6,                 ///< @htmlcolorblock{FAF0E6}
        Magenta = 0xFF00FF,               ///< @htmlcolorblock{FF00FF}
        Maroon = 0x800000,                ///< @htmlcolorblock{800000}
        MediumAquamarine = 0x66CDAA,      ///< @htmlcolorblock{66CDAA}
        MediumBlue = 0x0000CD,            ///< @htmlcolorblock{0000CD}
        MediumOrchid = 0xBA55D3,          ///< @htmlcolorblock{BA55D3}
        MediumPurple = 0x9370DB,          ///< @htmlcolorblock{9370DB}
        MediumSeaGreen = 0x3CB371,        ///< @htmlcolorblock{3CB371}
        MediumSlateBlue = 0x7B68EE,       ///< @htmlcolorblock{7B68EE}
        MediumSpringGreen = 0x00FA9A,     ///< @htmlcolorblock{00FA9A}
        MediumTurquoise = 0x48D1CC,       ///< @htmlcolorblock{48D1CC}
        MediumVioletRed = 0xC71585,       ///< @htmlcolorblock{C71585}
        MidnightBlue = 0x191970,          ///< @htmlcolorblock{191970}
        MintCream = 0xF5FFFA,             ///< @htmlcolorblock{F5FFFA}
        MistyRose = 0xFFE4E1,             ///< @htmlcolorblock{FFE4E1}
        Moccasin = 0xFFE4B5,              ///< @htmlcolorblock{FFE4B5}
        NavajoWhite = 0xFFDEAD,           ///< @htmlcolorblock{FFDEAD}
        Navy = 0x000080,                  ///< @htmlcolorblock{000080}
        OldLace = 0xFDF5E6,               ///< @htmlcolorblock{FDF5E6}
        Olive = 0x808000,                 ///< @htmlcolorblock{808000}
        OliveDrab = 0x6B8E23,             ///< @htmlcolorblock{6B8E23}
        Orange = 0xFFA500,                ///< @htmlcolorblock{FFA500}
        OrangeRed = 0xFF4500,             ///< @htmlcolorblock{FF4500}
        Orchid = 0xDA70D6,                ///< @htmlcolorblock{DA70D6}
        PaleGoldenrod = 0xEEE8AA,         ///< @htmlcolorblock{EEE8AA}
        PaleGreen = 0x98FB98,             ///< @htmlcolorblock{98FB98}
        PaleTurquoise = 0xAFEEEE,         ///< @htmlcolorblock{AFEEEE}
        PaleVioletRed = 0xDB7093,         ///< @htmlcolorblock{DB7093}
        PapayaWhip = 0xFFEFD5,            ///< @htmlcolorblock{FFEFD5}
        PeachPuff = 0xFFDAB9,             ///< @htmlcolorblock{FFDAB9}
        Peru = 0xCD853F,                  ///< @htmlcolorblock{CD853F}
        Pink = 0xFFC0CB,                  ///< @htmlcolorblock{FFC0CB}
        Plaid = 0xCC5533,                 ///< @htmlcolorblock{CC5533}
        Plum = 0xDDA0DD,                  ///< @htmlcolorblock{DDA0DD}
        PowderBlue = 0xB0E0E6,            ///< @htmlcolorblock{B0E0E6}
        Purple = 0x800080,                ///< @htmlcolorblock{800080}
        Red = 0xFF0000,                   ///< @htmlcolorblock{FF0000}
        RosyBrown = 0xBC8F8F,             ///< @htmlcolorblock{BC8F8F}
        RoyalBlue = 0x4169E1,             ///< @htmlcolorblock{4169E1}
        SaddleBrown = 0x8B4513,           ///< @htmlcolorblock{8B4513}
        Salmon = 0xFA8072,                ///< @htmlcolorblock{FA8072}
        SandyBrown = 0xF4A460,            ///< @htmlcolorblock{F4A460}
        SeaGreen = 0x2E8B57,              ///< @htmlcolorblock{2E8B57}
        Seashell = 0xFFF5EE,              ///< @htmlcolorblock{FFF5EE}
        Sienna = 0xA0522D,                ///< @htmlcolorblock{A0522D}
        Silver = 0xC0C0C0,                ///< @htmlcolorblock{C0C0C0}
        SkyBlue = 0x87CEEB,               ///< @htmlcolorblock{87CEEB}
        SlateBlue = 0x6A5ACD,             ///< @htmlcolorblock{6A5ACD}
        SlateGray = 0x708090,             ///< @htmlcolorblock{708090}
        SlateGrey = 0x708090,             ///< @htmlcolorblock{708090}
        Snow = 0xFFFAFA,                  ///< @htmlcolorblock{FFFAFA}
        SpringGreen = 0x00FF7F,           ///< @htmlcolorblock{00FF7F}
        SteelBlue = 0x4682B4,             ///< @htmlcolorblock{4682B4}
        Tan = 0xD2B48C,                   ///< @htmlcolorblock{D2B48C}
        Teal = 0x008080,                  ///< @htmlcolorblock{008080}
        Thistle = 0xD8BFD8,               ///< @htmlcolorblock{D8BFD8}
        Tomato = 0xFF6347,                ///< @htmlcolorblock{FF6347}
        Turquoise = 0x40E0D0,             ///< @htmlcolorblock{40E0D0}
        Violet = 0xEE82EE,                ///< @htmlcolorblock{EE82EE}
        Wheat = 0xF5DEB3,                 ///< @htmlcolorblock{F5DEB3}
        White = 0xFFFFFF,                 ///< @htmlcolorblock{FFFFFF}
        WhiteSmoke = 0xF5F5F5,            ///< @htmlcolorblock{F5F5F5}
        Yellow = 0xFFFF00,                ///< @htmlcolorblock{FFFF00}
        YellowGreen = 0x9ACD32,           ///< @htmlcolorblock{9ACD32}

        // LED RGB color that roughly approximates
        // the color of incandescent fairy lights,
        // assuming that you're using FastLED
        // color correction on your LEDs (recommended).
        FairyLight = 0xFFE42D,  ///< @htmlcolorblock{FFE42D}

        // If you are using no color correction, use this
        FairyLightNCC = 0xFF9D2A  ///< @htmlcolorblock{FFE42D}

    } HTMLColorCode;
};

/// Apply dimming compensation to values
#define APPLY_DIMMING(X) (X)

/// Divide the color wheel into eight sections, 32 elements each
/// @todo Unused. Remove?
#define HSV_SECTION_6 (0x20)

/// Divide the color wheel into four sections, 64 elements each
/// @todo I believe this is mis-named, and should be HSV_SECTION_4
#define HSV_SECTION_3 (0x40)

static inline void hsv2rgb_raw(const struct HSV& hsv, struct RGB& rgb) {
    // Convert hue, saturation and brightness ( HSV/HSB ) to RGB
    // "Dimming" is used on saturation and brightness to make
    // the output more visually linear.

    // Apply dimming curves
    uint8_t value = APPLY_DIMMING(hsv.val);
    uint8_t saturation = hsv.sat;

    // The brightness floor is minimum number that all of
    // R, G, and B will be set to.
    uint8_t invsat = APPLY_DIMMING(255 - saturation);
    uint8_t brightness_floor = (value * invsat) / 256;

    // The color amplitude is the maximum amount of R, G, and B
    // that will be added on top of the brightness_floor to
    // create the specific hue desired.
    uint8_t color_amplitude = value - brightness_floor;

    // Figure out which section of the hue wheel we're in,
    // and how far offset we are withing that section
    uint8_t section = hsv.hue / HSV_SECTION_3;  // 0..2
    uint8_t offset = hsv.hue % HSV_SECTION_3;   // 0..63

    uint8_t rampup = offset;                          // 0..63
    uint8_t rampdown = (HSV_SECTION_3 - 1) - offset;  // 63..0

    // We now scale rampup and rampdown to a 0-255 range -- at least
    // in theory, but here's where architecture-specific decsions
    // come in to play:
    // To scale them up to 0-255, we'd want to multiply by 4.
    // But in the very next step, we multiply the ramps by other
    // values and then divide the resulting product by 256.
    // So which is faster?
    //   ((ramp * 4) * othervalue) / 256
    // or
    //   ((ramp    ) * othervalue) /  64
    // It depends on your processor architecture.
    // On 8-bit AVR, the "/ 256" is just a one-cycle register move,
    // but the "/ 64" might be a multicycle shift process. So on AVR
    // it's faster do multiply the ramp values by four, and then
    // divide by 256.
    // On ARM, the "/ 256" and "/ 64" are one cycle each, so it's
    // faster to NOT multiply the ramp values by four, and just to
    // divide the resulting product by 64 (instead of 256).
    // Moral of the story: trust your profiler, not your insticts.

    // Since there's an AVR assembly version elsewhere, we'll
    // assume what we're on an architecture where any number of
    // bit shifts has roughly the same cost, and we'll remove the
    // redundant math at the source level:

    //  // scale up to 255 range
    //  //rampup *= 4; // 0..252
    //  //rampdown *= 4; // 0..252

    // compute color-amplitude-scaled-down versions of rampup and rampdown
    uint8_t rampup_amp_adj = (rampup * color_amplitude) / (256 / 4);
    uint8_t rampdown_amp_adj = (rampdown * color_amplitude) / (256 / 4);

    // add brightness_floor offset to everything
    uint8_t rampup_adj_with_floor = rampup_amp_adj + brightness_floor;
    uint8_t rampdown_adj_with_floor = rampdown_amp_adj + brightness_floor;

    if (section) {
        if (section == 1) {
            // section 1: 0x40..0x7F
            rgb.r = brightness_floor;
            rgb.g = rampdown_adj_with_floor;
            rgb.b = rampup_adj_with_floor;
        } else {
            // section 2; 0x80..0xBF
            rgb.r = rampup_adj_with_floor;
            rgb.g = brightness_floor;
            rgb.b = rampdown_adj_with_floor;
        }
    } else {
        // section 0: 0x00..0x3F
        rgb.r = rampdown_adj_with_floor;
        rgb.g = rampup_adj_with_floor;
        rgb.b = brightness_floor;
    }
}

#define K255 255
#define K171 171
#define K170 170
#define K85 85

static inline void hsv2rgb_rainbow(const HSV& hsv, RGB& rgb) {
    // Yellow has a higher inherent brightness than
    // any other color; 'pure' yellow is perceived to
    // be 93% as bright as white.  In order to make
    // yellow appear the correct relative brightness,
    // it has to be rendered brighter than all other
    // colors.
    // Level Y1 is a moderate boost, the default.
    // Level Y2 is a strong boost.
    const uint8_t Y1 = 1;
    const uint8_t Y2 = 0;

    // G2: Whether to divide all greens by two.
    // Depends GREATLY on your particular LEDs
    const uint8_t G2 = 0;

    // Gscale: what to scale green down by.
    // Depends GREATLY on your particular LEDs
    const uint8_t Gscale = 0;

    uint8_t hue = hsv.hue;
    uint8_t sat = hsv.sat;
    uint8_t val = hsv.val;

    uint8_t offset = hue & 0x1F;  // 0..31

    // offset8 = offset * 8
    uint8_t offset8 = offset;
    // On ARM and other non-AVR platforms, we just shift 3.
    offset8 <<= 3;

    uint8_t third = scale8(offset8, (256 / 3));  // max = 85

    uint8_t r, g, b;

    if (!(hue & 0x80)) {
        // 0XX
        if (!(hue & 0x40)) {
            // 00X
            // section 0-1
            if (!(hue & 0x20)) {
                // 000
                // case 0: // R -> O
                r = K255 - third;
                g = third;
                b = 0;
            } else {
                // 001
                // case 1: // O -> Y
                if (Y1) {
                    r = K171;
                    g = K85 + third;
                    b = 0;
                }
                if (Y2) {
                    r = K170 + third;
                    // uint8_t twothirds = (third << 1);
                    uint8_t twothirds =
                        scale8(offset8, ((256 * 2) / 3));  // max=170
                    g = K85 + twothirds;
                    b = 0;
                }
            }
        } else {
            // 01X
            //  section 2-3
            if (!(hue & 0x20)) {
                // 010
                // case 2: // Y -> G
                if (Y1) {
                    // uint8_t twothirds = (third << 1);
                    uint8_t twothirds =
                        scale8(offset8, ((256 * 2) / 3));  // max=170
                    r = K171 - twothirds;
                    g = K170 + third;
                    b = 0;
                }
                if (Y2) {
                    r = K255 - offset8;
                    g = K255;
                    b = 0;
                }
            } else {
                // 011
                // case 3: // G -> A
                r = 0;
                g = K255 - third;
                b = third;
            }
        }
    } else {
        // section 4-7
        // 1XX
        if (!(hue & 0x40)) {
            // 10X
            if (!(hue & 0x20)) {
                // 100
                // case 4: // A -> B
                r = 0;
                // uint8_t twothirds = (third << 1);
                uint8_t twothirds =
                    scale8(offset8, ((256 * 2) / 3));  // max=170
                g = K171 - twothirds;                  // K170?
                b = K85 + twothirds;

            } else {
                // 101
                // case 5: // B -> P
                r = third;
                g = 0;
                b = K255 - third;
            }
        } else {
            if (!(hue & 0x20)) {
                // 110
                // case 6: // P -- K
                r = K85 + third;
                g = 0;
                b = K171 - third;

            } else {
                // 111
                // case 7: // K -> R
                r = K170 + third;
                g = 0;
                b = K85 - third;
            }
        }
    }

    // This is one of the good places to scale the green down,
    // although the client can scale green down as well.
    if (G2) g = g >> 1;
    if (Gscale) g = scale8_video(g, Gscale);

    // Scale down colors if we're desaturated at all
    // and add the brightness_floor to r, g, and b.
    if (sat != 255) {
        if (sat == 0) {
            r = 255;
            b = 255;
            g = 255;
        } else {
            uint8_t desat = 255 - sat;
            desat = scale8_video(desat, desat);

            uint8_t satscale = 255 - desat;
            // satscale = sat; // uncomment to revert to pre-2021 saturation
            // behavior

            // nscale8x3_video( r, g, b, sat);
            if (r) r = scale8(r, satscale) + 1;
            if (g) g = scale8(g, satscale) + 1;
            if (b) b = scale8(b, satscale) + 1;
            uint8_t brightness_floor = desat;
            r += brightness_floor;
            g += brightness_floor;
            b += brightness_floor;
        }
    }

    // Now scale everything down if we're at value < 255.
    if (val != 255) {
        val = scale8_video(val, val);
        if (val == 0) {
            r = 0;
            g = 0;
            b = 0;
        } else {
            // nscale8x3_video( r, g, b, val);
            if (r) r = scale8(r, val) + 1;
            if (g) g = scale8(g, val) + 1;
            if (b) b = scale8(b, val) + 1;
        }
    }

    // Here we have the old AVR "missing std X+n" problem again
    // It turns out that fixing it winds up costing more than
    // not fixing it.
    // To paraphrase Dr Bronner, profile! profile! profile!
    // asm volatile(  ""  :  :  : "r26", "r27" );
    // asm volatile (" movw r30, r26 \n" : : : "r30", "r31");
    rgb.r = r;
    rgb.g = g;
    rgb.b = b;
}

/// @brief AdaFruit's HSV to RGB conversion function
/// @param hue 
/// @param sat 
/// @param val 
/// @param rgb 
inline void adaHSVToRGB(uint16_t hue, uint8_t sat, uint8_t val, RGB &rgb) {

  // Remap 0-65535 to 0-1529. Pure red is CENTERED on the 64K rollover;
  // 0 is not the start of pure red, but the midpoint...a few values above
  // zero and a few below 65536 all yield pure red (similarly, 32768 is the
  // midpoint, not start, of pure cyan). The 8-bit RGB hexcone (256 values
  // each for red, green, blue) really only allows for 1530 distinct hues
  // (not 1536, more on that below), but the full unsigned 16-bit type was
  // chosen for hue so that one's code can easily handle a contiguous color
  // wheel by allowing hue to roll over in either direction.
  hue = (hue * 1530L + 32768) / 65536;
  // Because red is centered on the rollover point (the +32768 above,
  // essentially a fixed-point +0.5), the above actually yields 0 to 1530,
  // where 0 and 1530 would yield the same thing. Rather than apply a
  // costly modulo operator, 1530 is handled as a special case below.

  // So you'd think that the color "hexcone" (the thing that ramps from
  // pure red, to pure yellow, to pure green and so forth back to red,
  // yielding six slices), and with each color component having 256
  // possible values (0-255), might have 1536 possible items (6*256),
  // but in reality there's 1530. This is because the last element in
  // each 256-element slice is equal to the first element of the next
  // slice, and keeping those in there this would create small
  // discontinuities in the color wheel. So the last element of each
  // slice is dropped...we regard only elements 0-254, with item 255
  // being picked up as element 0 of the next slice. Like this:
  // Red to not-quite-pure-yellow is:        255,   0, 0 to 255, 254,   0
  // Pure yellow to not-quite-pure-green is: 255, 255, 0 to   1, 255,   0
  // Pure green to not-quite-pure-cyan is:     0, 255, 0 to   0, 255, 254
  // and so forth. Hence, 1530 distinct hues (0 to 1529), and hence why
  // the constants below are not the multiples of 256 you might expect.

  // Convert hue to R,G,B (nested ifs faster than divide+mod+switch):
  if (hue < 510) { // Red to Green-1
    rgb.b = 0;
    if (hue < 255) { //   Red to Yellow-1
      rgb.r = 255;
      rgb.g = hue;       //     g = 0 to 254
    } else {         //   Yellow to Green-1
      rgb.r = 510 - hue; //     r = 255 to 1
      rgb.g = 255;
    }
  } else if (hue < 1020) { // Green to Blue-1
    rgb.r = 0;
    if (hue < 765) { //   Green to Cyan-1
      rgb.g = 255;
      rgb.b = hue - 510;  //     b = 0 to 254
    } else {          //   Cyan to Blue-1
      rgb.g = 1020 - hue; //     g = 255 to 1
      rgb.b = 255;
    }
  } else if (hue < 1530) { // Blue to Red-1
    rgb.g = 0;
    if (hue < 1275) { //   Blue to Magenta-1
      rgb.r = hue - 1020; //     r = 0 to 254
      rgb.b = 255;
    } else { //   Magenta to Red-1
      rgb.r = 255;
      rgb.b = 1530 - hue; //     b = 255 to 1
    }
  } else { // Last 0.5 Red (quicker than % operator)
    rgb.r = 255;
    rgb.g = rgb.b = 0;
  }

  // Apply saturation and value to R,G,B, pack into 32-bit result:
  uint32_t v1 = 1 + val;  // 1 to 256; allows >>8 instead of /255
  uint16_t s1 = 1 + sat;  // 1 to 256; same reason
  uint8_t s2 = 255 - sat; // 255 to 0
  rgb.r = (((rgb.r * s1) >> 8) + s2) * v1;
  rgb.g = (((rgb.g * s1) >> 8) + s2) * v1;
  rgb.b = (((rgb.b * s1) >> 8) + s2) * v1;
}


#endif