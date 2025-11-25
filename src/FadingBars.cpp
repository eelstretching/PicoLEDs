#include "FadingBars.h"

#include "ArrayColorMap.h"
#include "FadeColorMap.h"

void FadingBar::init() {
    state = initState;
    if (state == FADING_IN) {
        currColorIndex = dimColorIndex;
    } else {
        currColorIndex = brightColorIndex;
    }
}

bool FadingBar::step() {
    switch (state) {
        case FADING_IN:
            for (int i = 0; i < barWidth; i++) {
                canvas->fillColumn(
                    position + i,
                    currColorIndex);  // Ensure bright color is set
            }
            currColorIndex--;
            if (currColorIndex <= brightColorIndex) {
                pauseCount = getFPSNeeded();
                state = PAUSING;
            }
            break;
        case FADING_OUT:
            for (int i = 0; i < barWidth; i++) {
                canvas->fillColumn(position + i, currColorIndex);
            }
            currColorIndex++;
            if (currColorIndex >= dimColorIndex) {
                pauseCount = getFPSNeeded();
                state = PAUSING;
            }
            break;
        case PAUSING:
            pauseCount--;
            if (pauseCount == 0) {
                if (currColorIndex <= brightColorIndex) {
                    currColorIndex = brightColorIndex;
                    state = FADING_OUT;
                } else {
                    currColorIndex = dimColorIndex;
                    state = FADING_IN;
                }
            }
            break;
    }
    return true;
}

FadingBars::FadingBars(Canvas* canvas,
                       ColorMap* colorMap, uint8_t barWidth, uint8_t nColors)
    : Animation(canvas, colorMap), barWidth(barWidth), nColors(nColors) {
    fadeMap = new FadeColorMap(colorMap, nColors, 32, 28);
    canvas->setColorMap(fadeMap);
    nBars = canvas->getWidth() / barWidth;
    if (canvas->getWidth() % barWidth != 0) {
        nBars++;
    }
    bars = new FadingBar*[nBars];
    for (int i = 0; i < nBars; i++) {
        uint8_t dimIndex = (i % nColors) * 32 + 31;
        uint8_t brightIndex = (i % nColors) * 32;
        bars[i] = new FadingBar(canvas, colorMap, barWidth, i * barWidth, brightIndex,
                                dimIndex, i % 2 == 0 ? FADING_IN : FADING_OUT);
    }
}

void FadingBars::init() {
    for (int i = 0; i < nBars; i++) {
        bars[i]->init();
    }
}

bool FadingBars::step() {
    for (int i = 0; i < nBars; i++) {
        bars[i]->step();
    }
    return true;
}
