#include "FadingBars.h"

void FadingBar::init() {
    state = initState;
    if(state == FADING_IN) {
        currColorIndex = dimColorIndex;
    } else {
        currColorIndex = brightColorIndex; 
    }
}

bool FadingBar::step() {
    switch(state) {
        case FADING_IN:
            for(int i = 0; i < barWidth; i++) {
                uint8_t x = position + i;
                canvas->fillColumn(x, currColorIndex); // Ensure bright color is set
                currColorIndex++;
                if(currColorIndex > brightColorIndex) {
                    pauseCount = getFPSNeeded();
                    state = PAUSING;
                }
            }
            break;
        case FADING_OUT:
            for(int i = 0; i < barWidth; i++) {
                uint8_t x = position + i;
                canvas->fillColumn(x, currColorIndex);
                currColorIndex--;
                if(currColorIndex < dimColorIndex) {
                    pauseCount = getFPSNeeded();
                    state = PAUSING;
                }
            }
            break;
        case PAUSING:
            pauseCount--;
            if(pauseCount == 0) {   
                if(currColorIndex > brightColorIndex) {
                    currColorIndex = brightColorIndex - 1;
                    state = FADING_OUT;
                } else {
                    currColorIndex = dimColorIndex + 1;
                    state = FADING_IN;
                }
            }
            break;
    }
    return true;
}

FadingBars::FadingBars(Canvas *canvas, uint8_t barWidth, uint8_t nColors,
                       ColorMap *colorMap)
    : Animation(canvas), barWidth(barWidth), nColors(nColors){
    fadeMap = new ColorMap(nColors * 16);
    for(int i = 0; i < nColors; i++) {
        RGB c = colorMap->getColor(i);
        for(int j = 0; j < 16; j++) {
            RGB dimmed = c.fadeToBlackBy(20);
            fadeMap->setColor(i * 16 + j, dimmed);
        }
    }   
    nBars = canvas->getWidth() / barWidth;
    if(canvas->getWidth() % barWidth != 0) {
        nBars++;
    }
    bars = new FadingBar*[nBars];
    for(int i = 0; i < nBars; i++) {
        uint8_t dimIndex = (i % nColors) * 16 + 15;
        uint8_t brightIndex = (i % nColors) * 16;
        bars[i] = new FadingBar(canvas, barWidth, i * barWidth, brightIndex, dimIndex, i % 2 == 0 ? FADING_IN : FADING_OUT);
    }

}

void FadingBars::init() {
    for(int i = 0; i < nBars; i++) {
        bars[i]->init();
    }
}

bool FadingBars::step() { 
    for(int i = 0; i < nBars; i++) {
        bars[i]->step();
    }
    return true;
 }
