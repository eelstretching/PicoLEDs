#ifndef HOLIDAYPALETTES_H
#define HOLIDAYPALETTES_H

#pragma once

#include "colorutils.h"

/// @file HolidayPalettes.h
/// Christmas palettes from Mark Kriegsman's TwinkleFOX example for FastLED
/// (MIT license). Several use gray rather than white to keep the brightness
/// even across the palette.

/// @brief Mostly red, with green accents and white trim.
extern const RGBPalette16 RedGreenWhitePalette;

/// @brief Mostly dark green, with the occasional red berry.
extern const RGBPalette16 HollyPalette;

/// @brief Red and white stripes, like a candy cane.
extern const RGBPalette16 RedWhitePalette;

/// @brief Mostly blue, with white accents.
extern const RGBPalette16 BlueWhitePalette;

/// @brief Warm "fairy light" white at a few different brightnesses.
extern const RGBPalette16 FairyLightPalette;

/// @brief Soft snowflakes with the occasional bright one.
extern const RGBPalette16 SnowPalette;

/// @brief Old-school C9 bulbs: red, orange, green, blue and white.
extern const RGBPalette16 RetroC9Palette;

/// @brief Cold, icy pale blues.
extern const RGBPalette16 IcePalette;

#endif
