#include "HolidayPalettes.h"

// From Mark Kriegsman's TwinkleFOX example for FastLED (MIT license).

const RGBPalette16 RedGreenWhitePalette(
    RGB::Red, RGB::Red, RGB::Red, RGB::Red,
    RGB::Red, RGB::Red, RGB::Red, RGB::Red,
    RGB::Red, RGB::Red, RGB::Gray, RGB::Gray,
    RGB::Green, RGB::Green, RGB::Green, RGB::Green);

static const RGB hollyGreen(0x00580c);
static const RGB hollyRed(0xB00402);
const RGBPalette16 HollyPalette(
    hollyGreen, hollyGreen, hollyGreen, hollyGreen,
    hollyGreen, hollyGreen, hollyGreen, hollyGreen,
    hollyGreen, hollyGreen, hollyGreen, hollyGreen,
    hollyGreen, hollyGreen, hollyGreen, hollyRed);

const RGBPalette16 RedWhitePalette(
    RGB::Red, RGB::Red, RGB::Red, RGB::Red,
    RGB::Gray, RGB::Gray, RGB::Gray, RGB::Gray,
    RGB::Red, RGB::Red, RGB::Red, RGB::Red,
    RGB::Gray, RGB::Gray, RGB::Gray, RGB::Gray);

const RGBPalette16 BlueWhitePalette(
    RGB::Blue, RGB::Blue, RGB::Blue, RGB::Blue,
    RGB::Blue, RGB::Blue, RGB::Blue, RGB::Blue,
    RGB::Blue, RGB::Blue, RGB::Blue, RGB::Blue,
    RGB::Blue, RGB::Gray, RGB::Gray, RGB::Gray);

static const RGB fairy(RGB::FairyLight);
static const RGB halfFairy((RGB::FairyLight & 0xFEFEFE) / 2);
static const RGB quarterFairy((RGB::FairyLight & 0xFCFCFC) / 4);
const RGBPalette16 FairyLightPalette(
    fairy, fairy, fairy, fairy,
    halfFairy, halfFairy, fairy, fairy,
    quarterFairy, quarterFairy, fairy, fairy,
    fairy, fairy, fairy, fairy);

static const RGB snow(0x304048);
static const RGB brightSnow(0xE0F0FF);
const RGBPalette16 SnowPalette(
    snow, snow, snow, snow,
    snow, snow, snow, snow,
    snow, snow, snow, snow,
    snow, snow, snow, brightSnow);

static const RGB c9Red(0xB80400);
static const RGB c9Orange(0x902C02);
static const RGB c9Green(0x046002);
static const RGB c9Blue(0x070758);
static const RGB c9White(0x606820);
const RGBPalette16 RetroC9Palette(
    c9Red, c9Orange, c9Red, c9Orange,
    c9Orange, c9Red, c9Orange, c9Red,
    c9Green, c9Green, c9Green, c9Green,
    c9Blue, c9Blue, c9Blue, c9White);

static const RGB iceBlue1(0x0C1040);
static const RGB iceBlue2(0x182080);
static const RGB iceBlue3(0x5080C0);
const RGBPalette16 IcePalette(
    iceBlue1, iceBlue1, iceBlue1, iceBlue1,
    iceBlue1, iceBlue1, iceBlue1, iceBlue1,
    iceBlue1, iceBlue1, iceBlue1, iceBlue1,
    iceBlue2, iceBlue2, iceBlue2, iceBlue3);
