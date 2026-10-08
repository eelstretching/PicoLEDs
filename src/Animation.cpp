#include "Animation.h"

Animation::Animation() { canvas = NULL; colorMap = NULL; name[0] = '\0'; }

Animation::Animation(Canvas* canvas, ColorMap *colorMap) : canvas(canvas), colorMap(colorMap) {name[0] = '\0';}

Animation::Animation(Canvas* canvas, ColorMap* colorMap, uint8_t fps) : canvas(canvas), colorMap(colorMap), fps(fps) {name[0] = '\0';}
