#ifndef FIREWORKS2D_H
#define FIREWORKS2D_H

#pragma once

#include <vector>

#include "Animation.h"
#include "colorutils.h"

/// @brief The kinds of shell that a 2D fireworks show can launch.
enum ShellType {
    /// @brief A ball of sparks that all start out the same color.
    PEONY,
    /// @brief A peony with two colors, half the sparks each.
    TWO_TONE,
    /// @brief A single ring of sparks.
    RING,
    /// @brief Long-lived gold sparks that droop down into trails.
    WILLOW,
    /// @brief A peony whose sparks flicker white as they burn out.
    CRACKLE,
    NUM_SHELL_TYPES,
};

/// @brief One burning bit of a firework, either the flare going up or a spark
/// after a burst. Positions and velocities are in pixels and pixels per step.
struct Spark2D {
    float x, y;
    float vx, vy;
    /// @brief Where the spark was on the last step, so we can draw a streak
    /// between there and here.
    float px, py;
    /// @brief How many steps the spark has burned for, and how many it'll
    /// burn for in total.
    uint16_t age, life;
    RGB color;
    /// @brief How much velocity the spark keeps each step, out of 256.
    uint8_t drag;
    /// @brief The ShellType this spark came from, which decides how it cools.
    uint8_t type;
};

/// @brief A shell on its way up.
struct Shell {
    float x, y;
    float vx, vy;
    float px, py;
    ShellType type;
    RGB color;
    RGB color2;
};

/// @brief Fireworks on a whole 2D canvas, for something like a grid of pixels
/// out on the lawn.
///
/// Shells launch from the bottom of the canvas and rise, slowing down, until
/// they burst at the top of their arc into a ball of sparks. The sparks fly
/// out, slowed by drag and pulled down by gravity, and cool from white hot
/// through their color down to black. Everything moves in floating point and
/// is drawn with sub-pixel precision: a spark between pixels lights up its
/// four neighbours in proportion to how close it is to each, and a fast spark
/// is drawn as a streak from where it was to where it is. Draws are added to
/// what's already on the canvas, so overlapping bursts brighten each other.
///
/// By default the canvas fades a bit each step rather than being cleared, which
/// gives everything a trail. If you're drawing something else on the canvas
/// too, turn that off with setTrailFade(0) and clear the canvas yourself.
///
/// The burst sizes and the height that shells reach are worked out from the
/// size of the canvas, so the same show works on a 32x32 panel or a lawn
/// full of pixels.
class Fireworks2D : public Animation {
   protected:
    std::vector<Shell> shells;
    std::vector<Spark2D> sparks;

    uint maxShells;
    uint maxSparks;

    /// @brief Colors for the bursts. If there's no palette, we pick from a
    /// list of classic firework colors.
    RGBPalette16 palette;
    bool usePalette = false;

    /// @brief Gravity, in pixels per step per step.
    float gravity;

    /// @brief Bursts are this fraction of the canvas's smaller dimension
    /// across, roughly.
    float burstSize = 0.45;

    /// @brief How much of each pixel's brightness is kept each step, out of
    /// 256. 0 means don't fade (or clear) the canvas at all.
    uint8_t trailFade = 180;

    /// @brief Average steps between launches, and when the next one is due.
    uint launchInterval;
    uint untilLaunch = 0;

    bool finale = false;

    /// @brief Gets a random float between 0 and 1.
    static float frand();

    /// @brief Gets a random float between lo and hi.
    static float frand(float lo, float hi) { return lo + frand() * (hi - lo); }

    RGB randomColor();

    void addSpark(float x, float y, float vx, float vy, uint16_t life,
                  const RGB& color, uint8_t drag, ShellType type);

    /// @brief Adds some light to a point on the canvas, spread over the four
    /// pixels around it.
    void splat(float x, float y, const RGB& color);

    /// @brief Draws a streak of light from (x0, y0) to (x1, y1), splatting
    /// points along the way so a fast spark doesn't leave gaps.
    void streak(float x0, float y0, float x1, float y1, const RGB& color);

    /// @brief The color of a spark at its current age.
    RGB sparkColor(const Spark2D& s);

    void stepShells();
    void stepSparks();
    void fadeCanvas();

   public:
    /// @brief Makes a fireworks show for a canvas.
    /// @param canvas the canvas to draw on.
    /// @param launchInterval the average number of steps between launches.
    /// @param maxShells the most shells that can be in the air at once.
    /// @param maxSparks the most sparks that can be burning at once. Each one
    /// takes 36 bytes, and they're all allocated up front.
    Fireworks2D(Canvas* canvas, uint launchInterval = 30, uint maxShells = 6,
                uint maxSparks = 1000);

    void init() override;

    bool step() override;

    /// @brief Launches a shell of a random type from a random spot along the
    /// bottom of the canvas.
    void launch();

    /// @brief Launches a shell of the given type from position x along the
    /// bottom of the canvas, to burst at height y.
    void launch(ShellType type, float x, float burstY);

    /// @brief Bursts a shell of the given type right now at (x, y).
    void burst(ShellType type, float x, float y, const RGB& color,
               const RGB& color2);

    /// @brief Picks burst colors from a palette instead of the built-in list.
    void setPalette(const RGBPalette16& palette) {
        this->palette = palette;
        usePalette = true;
    }

    /// @brief Sets how much of each pixel's brightness is kept from step to
    /// step, out of 256. Bigger numbers give longer trails. 0 turns fading off
    /// altogether, leaving the canvas alone except for the fireworks, which
    /// is what you want if you're clearing it yourself.
    void setTrailFade(uint8_t trailFade) { this->trailFade = trailFade; }

    /// @brief Sets the average number of steps between launches.
    void setLaunchInterval(uint launchInterval) {
        this->launchInterval = MAX(launchInterval, 1);
    }

    /// @brief Sets the size of bursts as a fraction of the canvas's smaller
    /// dimension. The default is 0.45.
    void setBurstSize(float burstSize) { this->burstSize = burstSize; }

    /// @brief Turns on the finale: shells go up as fast as we can manage.
    void setFinale(bool finale) { this->finale = finale; }

    /// @brief How many sparks are burning right now. Handy for working out
    /// how big maxSparks needs to be.
    uint getSparkCount() { return sparks.size(); }
};

#endif
