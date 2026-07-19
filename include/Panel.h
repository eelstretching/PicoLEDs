#ifndef PANEL_H
#define PANEL_H

#include <cstdint>

#include "Strip.h"
#include "color.h"

class Panel : public Strip {
   public:
    Panel(uint pin, uint width, uint height)
        : Strip(pin, width * height, StripType::WS2812),
          width(width),
          height(height) {}

    void set(uint x, uint y, RGB& color) {
        if (x >= width || y >= height) return;

        int index = x * height + ((x % 2 == 0) ? (height - 1 - y) : y);

        putPixel(color, index);
    }

    int getWidth() const { return width; }
    int getHeight() const { return height; }

   private:
    uint width;
    uint height;
};

#endif  // PANEL_H
