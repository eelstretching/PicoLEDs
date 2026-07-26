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
    const RGB& color;
    RenderAngle angle;
    bool clear;
    bool frameDelay;

    public:
    ScrollText(Canvas* canvas, Font* font, const char* text, int startx, int starty, const RGB& color, RenderAngle angle = RENDER_90);
    void setClear(bool clear) { this->clear = clear; }
    void setFrameDelay(bool frameDelay) { this->frameDelay = frameDelay; }
    void init() override;
    bool step() override;
};

class ScrollTexts : public Animation {
   protected:
    std::vector<ScrollText*> texts;

   public:
    ScrollTexts(Canvas* canvas) : Animation(canvas, nullptr) {};

    void add(ScrollText* text) { texts.push_back(text); };

    void init() override;

    bool step() override;

};

#endif