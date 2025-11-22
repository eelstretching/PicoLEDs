#include "FadeColorMap.h"

FadeColorMap::FadeColorMap(ColorMap* baseMap, uint8_t nColors, uint8_t nSteps,
                           uint8_t fadeFactor)
    : nSteps(nSteps) {
    this->size = nColors * nSteps;
    this->p = nColors * nSteps;
    entries = new RGB[size];
    background = baseMap->getBackground();
    for (int i = 0; i < nColors; i++) {
        RGB c = baseMap->getColor(i);
        for (int j = 0; j < nSteps; j++) {
            entries[i * nSteps + j] = j == 0 ? c : c.fadeToBlackBy(fadeFactor);
        }
    }
}

FadeColorMap::~FadeColorMap() {
    delete[] entries;
}
