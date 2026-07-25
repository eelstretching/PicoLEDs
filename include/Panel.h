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

    bool set(uint x, uint y, const RGB& color);

    const RGB& get(uint x, uint y);

    void fill(const RGB& color);

    void rotateRight();
    void rotateLeft();

    int getWidth() const { return width; }
    int getHeight() const { return height; }

   private:
    uint width;
    uint height;
    PanelOrigin origin;
};

#endif  // PANEL_H
