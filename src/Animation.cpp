#include "Animation.h"

Animation::Animation() { canvas = NULL; colorMap = NULL; }

Animation::Animation(Canvas* canvas, ColorMap *colorMap) : canvas(canvas), colorMap(colorMap) {}

Animation::Animation(Canvas* canvas, ColorMap* colorMap, uint8_t fps) : canvas(canvas), colorMap(colorMap), fps(fps) {}
