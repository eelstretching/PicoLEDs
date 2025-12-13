#ifndef SCROLLTEXT_H
#define SCROLLTEXT_H
#pragma once
#include "TextAnimation.h"
#include "Font.h"

class ScrollText : public TextAnimation {
   protected:
    int scrollX;
    int speed;
    uint lastUpdateTime;
    bool allOffCanvas;

   public:
    ScrollText(Canvas* canvas, ColorMap *colorMap, Font* font);

    void init() override;

    bool step() override;

    int getFPSNeeded() override { return 20; };
};

#endif