#ifndef XMASTREE_H
#define XMASTREE_H

#pragma once

#include <stdint.h>

#include <utility>
#include <vector>

#include "Animation.h"
#include "Xpm.h"

/// @brief A pixmap for a roundish ornament.
static const char* ornaPixMap[] = {
    "6 6 2 1",
    ". c 255",  // Background
    "# c 0",    // A color to be specified later.
    "..##..",   //
    ".####.",   //
    "######",   //
    "######",   //
    ".####.",   //
    "..##..",   //
};

int sideOfLine(int x, int y, int offset);
bool inside(uint8_t x, uint8_t y, uint8_t llx, uint8_t lly, uint8_t urx,
            uint8_t ury);

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
    int getFPSNeeded() override { return 1; };
    bool step() override;
};

enum RibbonState {
    DRAWING,
    DONE,
};

class Ribbon : public Animation {
    friend class XmasTree;

    uint8_t x;
    uint8_t y;
    uint8_t color;
    uint8_t width;
    RibbonState state;
    /// @brief The lines making up the ribbon edges, so we can check impingment.
    std::vector<uint8_t> lineStarts;

   public:
    Ribbon(Canvas* canvas, uint8_t color, uint8_t width)
        : Animation(canvas, nullptr), color(color), width(width) {};
    void init() override;
    /// @brief Does an area that we want to draw in impinge on the ribbon's
    /// area?
    /// @param x1 X coord of lower left of area.
    /// @param y1 Y coord of lower left of area
    /// @param x1 X coord of upper right of area.
    /// @param y1 Y coord of upper right of area
    /// @return true if this area impinges on the ribbon.
    int getFPSNeeded() override { return 30; };
    bool step() override;
};

class Light : public Animation {
    friend class XmasTree;

   protected:
    uint8_t x;
    uint8_t y;
    uint8_t steadyFrames;
    uint8_t currFrame;
    uint8_t* colors;
    uint8_t nColors;
    uint8_t ci;
    uint8_t size = 2;

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
    Ribbon* ribbon;
    Xpm* ornament;
    uint8_t nOrnaments;
    Ornament** ornaments;
    uint8_t nLights;
    Light** lights;
    TreeState state;
    uint8_t showDelay;
    uint8_t el;

   public:
    XmasTree(Canvas* canvas, uint8_t nOrnaments, uint8_t nLights);
    void init() override;
    int getFPSNeeded() override { return 30; };
    bool step() override;
};

#endif