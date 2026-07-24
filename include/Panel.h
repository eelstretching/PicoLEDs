#ifndef PANEL_H
#define PANEL_H

#include <cstdint>

#include "Strip.h"
#include "color.h"

enum PanelOrigin {
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight
};

class Panel : public Strip {
   public:

    Panel(uint pin, uint width, uint height, PanelOrigin origin = TopLeft)
        : Strip(pin, width * height, StripType::WS2812),
          width(width),
          height(height),
          origin(origin) {}

    void set(uint x, uint y, const RGB& color) {
        if (x >= width || y >= height) return;
        uint index;
        switch (origin) {
            case TopLeft:
                index = x * height + ((x % 2 == 0) ? (height - 1 - y) : y);
                break;
            case BottomRight:
                index = (width - 1 - x) * height + ((x % 2 == 0) ? y : (height - 1 - y));
                break;
            case TopRight:
                index = (width - 1 - x) * height + ((x % 2 == 0) ? (height - 1 - y) : y);
                break;
            case BottomLeft:
                index = x * height + ((x % 2 == 0) ? (height - 1 - y) : y);
                break;
            default:
                return;
        }

        Strip::putPixel(color, index);
    }

    int getWidth() const { return width; }
    int getHeight() const { return height; }

   private:
    uint width;
    uint height;
    PanelOrigin origin;
};

#endif  // PANEL_H
