#ifndef CLASSICLIGHTS_H
#define CLASSICLIGHTS_H

#pragma once

#include <vector>

#include "Animation.h"

/// @brief The calm patterns of an old-fashioned string of Christmas lights.
enum class ClassicMode {
    /// @brief Every bulb on, steady.
    STEADY,
    /// @brief Steady, but now and then a bulb dims out and glows back, like
    /// the twinkle bulbs mixed into an old string.
    TWINKLE,
    /// @brief The "slo-glo" setting on a light controller: alternate bulbs
    /// glow up as their neighbours glow down, and back again.
    SLO_GLO,
    /// @brief Slow waves of brightness travel along the strands, which
    /// spiral around a tree they're wound on.
    IN_WAVES,
    /// @brief The whole tree slowly dims and brightens together.
    SLOW_FADE,
    /// @brief The lights are wired in four circuits that glow one after the
    /// other, so the brightness crawls slowly along the string.
    SLOW_CHASE,
};

/// @brief Traditional Christmas tree lights: a string of bulbs in a fixed
/// sequence of colors (one warm white, or the classic red, green, blue,
/// orange and yellow), in one of the patterns an old light controller had.
///
/// It's made to look like incandescent bulbs: each bulb is a little brighter
/// or dimmer than its neighbours, and a bulb reddens as it dims, the way a
/// filament does. Everything is slow and goes by the clock (fps 0), so it
/// looks the same at any frame rate.
///
/// Bulb n along a strand (row y of the canvas, from x = 0) gets color
/// (n + y) % colors, so the strands don't all start on the same color.
class ClassicLights : public Animation {
   protected:
    std::vector<RGB> colors;
    ClassicMode mode;

    /// @brief Light every spacing'th pixel, leaving the rest dark.
    uint spacing = 1;

    /// @brief How many twinkles start each second, per 100 bulbs.
    uint twinklesPer100 = 1;

    /// @brief How far each bulb is through a twinkle, or 0 if it's not
    /// twinkling.
    std::vector<uint8_t> twinkles;
    uint32_t owed = 0;
    uint32_t twinkleCarry = 0;
    uint32_t phaseCarry = 0;

    /// @brief How long one cycle of the pattern takes, in ms.
    uint32_t periodMs;

    /// @brief How dim the pattern lets a bulb get, out of 255.
    uint8_t floor;

    /// @brief Where the pattern is, 0-255 through a cycle, at the current
    /// step.
    uint8_t cycle();

    /// @brief How bright bulb n of row y is in this pattern, 0-255.
    uint8_t level(uint8_t c, int n, int x, int y);

   public:
    /// @param canvas the canvas to draw on
    /// @param colors the bulb colors, in the order they go along a strand
    /// @param mode the pattern
    /// @param spacing light every spacing'th pixel (1 for every pixel, 3 or
    /// so for a bulbs-with-gaps C9 look)
    ClassicLights(Canvas* canvas, const std::vector<RGB>& colors,
                  ClassicMode mode = ClassicMode::STEADY, uint spacing = 1);

    /// @brief Sets how long a cycle of the pattern takes. Each pattern has a
    /// slow default.
    void setPeriodMs(uint32_t ms) { periodMs = ms; }

    /// @brief Sets how dim the pattern lets a bulb get, out of 255.
    void setFloor(uint8_t f) { floor = f; }

    /// @brief Sets how many twinkles start each second, per 100 bulbs.
    void setTwinkles(uint per100) { twinklesPer100 = per100; }

    void init() override;
    bool step() override;

    /// @brief Dims a color the way an incandescent bulb dims: red fades
    /// slowest and blue fastest, so a dim bulb glows warmer.
    static RGB dimBulb(const RGB& c, uint8_t level);

    /// @brief Warm white, like a 2700K incandescent mini light.
    static const RGB WarmWhite;
    /// @brief The classic C7/C9 colors: red, green, blue, orange, yellow.
    static const std::vector<RGB> Multicolor;
};

/// @brief Wraps an animation so that it fades in from whatever was on the
/// canvas when it started (the end of the animation before it, or black at
/// power up), instead of cutting straight to it.
///
/// The wrapped animation draws each frame as usual, and the wrapper blends
/// that with the starting picture. That only works for animations that draw
/// every pixel fresh each step, like ClassicLights, Starlight and TwinkleFox,
/// not ones that read the canvas back and build on the last frame.
class CrossFade : public Animation {
   protected:
    Animation* animation;
    uint32_t fadeUs;
    std::vector<RGB> from;

   public:
    /// @param animation the animation to fade in; it's still yours
    /// @param fadeMs how long the fade takes
    CrossFade(Animation* animation, uint32_t fadeMs = 3000);

    void init() override;
    bool step() override;
    void finish() override { animation->finish(); }
    int getFPS() override { return animation->getFPS(); }
    void startClock(uint64_t now) override {
        Animation::startClock(now);
        animation->startClock(now);
    }
    void setFrameTime(uint64_t now) override {
        Animation::setFrameTime(now);
        animation->setFrameTime(now);
    }
    const char* getName() override { return animation->getName(); }
};

#endif
