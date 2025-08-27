#include "TreeLight.h"

TreeLight::TreeLight(Canvas* canvas, uint x, uint y, uint size, RGB color,
                     uint row) : canvas(canvas), x(x), y(y), size(size), color(color) {}

TreeLight::~TreeLight() {}

bool TreeLight::step() {
    if(fadingIn) {
        brightness += 5;
        if(brightness >= 255) {
            brightness = 255;
            fadingIn = false;
        }
        canvas->fillRect(x, y, x + size, y + size, color.scale8(brightness));
    }
}