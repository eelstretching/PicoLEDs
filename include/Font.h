#if !defined(FONT_H)
#define FONT_H
#pragma once

#include "Canvas.h"

enum RenderAngle { RENDER_0, RENDER_90, RENDER_180, RENDER_270 };


// @brief A pure abstract base class for fonts so that various font implementations can be used with the same interface.
class Font {
public:
    virtual uint render(Canvas* canvas, const char* text, int bx, int by,
                const RGB& color, bool renderMissing = true,RenderAngle angle = RENDER_0) = 0;

    virtual uint getWidth(const char* text, bool renderMissing = true) const = 0;
    virtual std::pair<uint, uint> getBoundingBox(const char* text, bool renderMissing = true) const = 0;
};

#endif // FONT_H