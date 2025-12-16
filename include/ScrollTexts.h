#ifndef SCROLLTEXTS_H
#define SCROLLTEXTS_H
#pragma once
#include <stdint.h>
#include "TextAnimation.h"
#include "Font.h"


class ScrollText : public Animation {
    protected:
    int startx;
    int starty;
    int x;
    int y;
    int width;
    uint8_t frameWait;
    const char *text;
    Font *font;
    uint8_t color;
    RenderAngle angle;

    public:
    ScrollText(Canvas* canvas, Font* font, const char* text, int startx, int starty, uint8_t color, RenderAngle angle = RENDER_90);
    void init() override;
    bool step() override;
};

class ScrollTexts : public Animation {
   protected:
    std::vector<ScrollText*> texts;

   public:
    ScrollTexts(Canvas* canvas, ColorMap *colorMap) : Animation(canvas, colorMap) {};

    void add(ScrollText* text) { texts.push_back(text); };

    void init() override;

    bool step() override;

};

#endif