#ifndef FADINGBARS_H
#define FADINGBARS_H
#pragma once
#include "Animation.h"
#include "pico/stdlib.h"
#include "pico/types.h"

enum FadeState {
    FADING_IN,
    FADING_OUT,
    PAUSING,
};

class FadingBar : public Animation {
    uint8_t barWidth;
    uint8_t brightColorIndex;
    uint8_t dimColorIndex;
    uint8_t position;
    uint8_t currColorIndex;
    uint8_t pauseCount;
    FadeState initState;
    FadeState state;

   public:
    FadingBar(Canvas* canvas, ColorMap *colorMap, uint8_t barWidth, uint8_t position,
              uint8_t brightColorIndex, uint8_t dimColorIndex, FadeState state)
        : Animation(canvas, colorMap),
          barWidth(barWidth),
          brightColorIndex(brightColorIndex),
          dimColorIndex(dimColorIndex),
          position(position), initState(state), state(state) {};
    int getFPSNeeded() override { return 10; };
    virtual void init() override;
    virtual bool step() override;

};

class FadingBars : public Animation {
    uint8_t barWidth;
    uint8_t nColors;
    uint8_t nBars;
    ColorMap* fadeMap;
    FadingBar** bars;

   public:
    FadingBars(Canvas* canvas, ColorMap *colorMap, uint8_t barWidth, uint8_t nColors);
    int getFPSNeeded() override { return 10; };
    virtual void init() override;
    virtual bool step() override;
};
#endif