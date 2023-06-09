#ifndef COLORPALETTES_H
#define COLORPALETTES_H

#include "colorutils.h"

/// @file colorpalettes.h
/// Declarations for the predefined color palettes supplied by FastLED.

// Have Doxygen ignore these declarations
/// @cond

extern const RGBPalette16 CloudColors(
    {RGB::Blue, RGB::DarkBlue, RGB::DarkBlue, RGB::DarkBlue,

     RGB::DarkBlue, RGB::DarkBlue, RGB::DarkBlue, RGB::DarkBlue,

     RGB::Blue, RGB::DarkBlue, RGB::SkyBlue, RGB::SkyBlue,

     RGB::LightBlue, RGB::White, RGB::LightBlue, RGB::SkyBlue});

extern const RGBPalette16 LavaColors(
    {RGB::Black, RGB::Maroon, RGB::Black, RGB::Maroon,

     RGB::DarkRed, RGB::Maroon, RGB::DarkRed,

     RGB::DarkRed, RGB::DarkRed, RGB::Red, RGB::Orange,

     RGB::White, RGB::Orange, RGB::Red, RGB::DarkRed});

extern const RGBPalette16 OceanColors(
    {RGB::MidnightBlue, RGB::DarkBlue, RGB::MidnightBlue, RGB::Navy,

     RGB::DarkBlue, RGB::MediumBlue, RGB::SeaGreen, RGB::Teal,

     RGB::CadetBlue, RGB::Blue, RGB::DarkCyan, RGB::CornflowerBlue,

     RGB::Aquamarine, RGB::SeaGreen, RGB::Aqua, RGB::LightSkyBlue});

extern const RGBPalette16 ForestColors(
    {RGB::DarkGreen, RGB::DarkGreen, RGB::DarkOliveGreen, RGB::DarkGreen,

     RGB::Green, RGB::ForestGreen, RGB::OliveDrab, RGB::Green,

     RGB::SeaGreen, RGB::MediumAquamarine, RGB::LimeGreen, RGB::YellowGreen,

     RGB::LightGreen, RGB::LawnGreen, RGB::MediumAquamarine, RGB::ForestGreen});

extern const RGBPalette16 RainbowColors({0xFF0000, 0xD52A00, 0xAB5500, 0xAB7F00,
                                         0xABAB00, 0x56D500, 0x00FF00, 0x00D52A,
                                         0x00AB55, 0x0056AA, 0x0000FF, 0x2A00D5,
                                         0x5500AB, 0x7F0081, 0xAB0055,
                                         0xD5002B});

/// Alias of RainbowStripeColors
extern const RGBPalette16 RainbowStripeColors(
    {0xFF0000, 0x000000, 0xAB5500, 0x000000, 0xABAB00, 0x000000, 0x00FF00,
     0x000000, 0x00AB55, 0x000000, 0x0000FF, 0x000000, 0x5500AB, 0x000000,
     0xAB0055, 0x000000});
;

extern const RGBPalette16 PartyColors({0x5500AB, 0x84007C, 0xB5004B, 0xE5001B,
                                       0xE81700, 0xB84700, 0xAB7700, 0xABAB00,
                                       0xAB5500, 0xDD2200, 0xF2000E, 0xC2003E,
                                       0x8F0071, 0x5F00A1, 0x2F00D0, 0x0007F9});

extern const RGBPalette16 HeatColors({0x000000, 0x330000, 0x660000, 0x990000,
                                      0xCC0000, 0xFF0000, 0xFF3300, 0xFF6600,
                                      0xFF9900, 0xFFCC00, 0xFFFF00, 0xFFFF33,
                                      0xFFFF66, 0xFFFF99, 0xFFFFCC, 0xFFFFFF});

#endif
