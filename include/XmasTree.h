#ifndef XMASTREE_H
#define XMASTREE_H

#pragma once

#include <stdint.h>

#include "Animation.h"
#include "Xpm.h"

/// @brief A pixmap for a roundish ornament.
static const char* ornaPixMap[] = {
    "6 6 2 1",
    ". c 255",  // Background
    "# c 0",    // A color to be specified later.
    "..##..",  ".####.", "######", "######", ".####.", "..##..",
};

class Ornament : public Animation {
    friend class XmasTree;

    uint8_t x;
    uint8_t y;
    Xpm* orn;
    uint8_t cmap[2];

   public:
    Ornament(Canvas* canvas, Xpm* orn);
    void setXY(uint8_t x, uint8_t y);
    void setColor(uint8_t color);
    void init() override;
    int getFPSNeeded() override { return 30; };
    bool step() override;
};

enum RibbonState {
    DRAWING,
    DONE,
};

class Ribbon : public Animation {
    friend class XmasTree;

    uint8_t startx;
    uint8_t starty;
    uint8_t x;
    uint8_t y;
    uint8_t color;
    uint8_t width;
    RibbonState state;

   public:
    Ribbon(Canvas* canvas, uint8_t startx, uint8_t starty, uint8_t color,
           uint8_t width)
        : Animation(canvas, nullptr),
          startx(startx),
          starty(starty),
          color(color),
          width(width) {};
    void init() override;
    int getFPSNeeded() override { return 30; };
    bool step() override;
};

class Light : public Animation {
    uint8_t x;
    uint8_t y;
    uint8_t steadyFrames;
    uint8_t currFrame;
    uint8_t* colors;
    uint8_t nColors;
    uint8_t ci;

   public:
    Light(Canvas* canvas, uint8_t* colors, uint8_t nColors,
          uint8_t steadyFrames)
        : Animation(canvas, nullptr),
          x(x),
          y(y),
          colors(colors),
          nColors(nColors),
          steadyFrames(steadyFrames) {}
    void setXY(uint8_t x, uint8_t y);

    void init() override;
    int getFPSNeeded() override { return 30; };
    bool step() override;
};

enum XmasState {
    DRAWRIBBON,
    DRAWORNAMENTS,
};

enum TreeState {
    RIBBON,
    ORNAMENTS,
    LIGHTS, 
    BLINKING,
};

class XmasTree : public Animation {
    Ribbon *ribbon;
    Xpm* ornament;
    uint8_t nOrnaments;
    Ornament** ornaments;
    uint8_t nLights;
    Light** lights;
    TreeState state;
    uint8_t showDelay;
    uint8_t el;

   public:
    XmasTree(Canvas* canvas, ColorMap* colorMap, uint8_t nOrnaments,
             uint8_t nLights);
    void init() override;
    int getFPSNeeded() override { return 30; };
    bool step() override;
};

#endif