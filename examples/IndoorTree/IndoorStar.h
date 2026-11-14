//
// The seed pixel star on top of the indoor tree: a piece of strand pressed
// into the three rings of star/IndoorStar.scad, on its own output.
//
// Along the strand, from the end nearest the Pico:
//
//   STAR_LEAD_IN hidden pixels before the star (0 if the strand starts at it)
//   outer ring:  29 pixels, 3 inch edges
//   STAR_HOP hidden pixels in the pocket on the back
//   middle ring: 19 pixels, 2 inch edges
//   STAR_HOP hidden pixels
//   inner ring:   9 pixels, 1 inch edges
//
// Each ring starts one pixel past its bottom valley and goes round to one
// pixel before it, leaving the valley itself empty where the strand goes
// through to the back. The hops between rings need a little over three
// pixel gaps of strand, so three pixels stay hidden in the pocket for each
// one and the slack folds up in there with them. If yours come out
// differently, count them and change STAR_HOP (or STAR_LEAD_IN).
//
// It glows the way a traditional tree top star does, whatever the tree is
// doing: a warm white center fading to deep gold at the points, slow swells
// of brightness moving out from the center, and now and then a slow white
// glint.
#ifndef INDOORSTAR_H
#define INDOORSTAR_H

#include <math.h>

#include <vector>

#include "Strip.h"
#include "colorutils.h"
#include "math8.h"

#ifndef STAR_LEAD_IN
#define STAR_LEAD_IN 0
#endif
#ifndef STAR_HOP
#define STAR_HOP 3
#endif
//
// 1 if the strand goes round each ring clockwise as you look at the front of
// the star (from the bottom valley toward the lower left point), 0 if it goes
// the other way.
#ifndef STAR_CLOCKWISE
#define STAR_CLOCKWISE 1
#endif

class IndoorStar {
   public:
    static constexpr int kRings = 3;
    /// @brief Edge length of each ring in pixel gaps, outside in.
    static constexpr int kEdges[kRings] = {3, 2, 1};
    /// @brief Valley radius over tip radius, as in IndoorStar.scad.
    static constexpr float kValley = 0.382f;

    struct Pixel {
        uint16_t index;  // where it is on the strand
        uint8_t ring;    // 0 outer, 2 inner
        bool tip;        // sits on a point of the star
        float x, y;      // where it is, in mm from the center, y up
    };

   protected:
    Strip* strip;
    std::vector<Pixel> pixels;
    std::vector<uint8_t> glints;
    uint32_t glintCarry = 0;
    uint32_t phaseCarry = 0;
    uint64_t lastUs = 0;

    static uint32_t due(uint32_t ratePerSecond, uint32_t us, uint32_t& carry) {
        uint64_t total = (uint64_t)ratePerSecond * us + carry;
        carry = (uint32_t)(total % 1000000);
        return (uint32_t)(total / 1000000);
    }

    static uint8_t easeInOut(uint8_t i) {
        uint8_t j = i < 128 ? i : 255 - i;
        uint8_t e = scale8(j, j) * 2;
        return i < 128 ? e : 255 - e;
    }

    /// @brief A glint rises over the first third and falls slowly.
    static uint8_t glintLevel(uint8_t phase) {
        if (phase < 86) {
            return easeInOut(phase * 3);
        }
        uint8_t fall = (phase - 86) + ((phase - 86) >> 1);
        return easeInOut(255 - fall);
    }

   public:
    /// @brief The number of pixels the star takes off the strand, hidden ones
    /// included.
    static int length() {
        int n = STAR_LEAD_IN;
        for (int r = 0; r < kRings; r++) {
            n += kEdges[r] * 10 - 1 + (r < kRings - 1 ? STAR_HOP : 0);
        }
        return n;
    }

    IndoorStar(Strip* strip) : strip(strip) {
        const float pitch = 25.4f;
        const float edgeOverR =
            sqrtf(1 + kValley * kValley - 2 * kValley * cosf(M_PI / 5));
        int index = STAR_LEAD_IN;
        for (int r = 0; r < kRings; r++) {
            int e = kEdges[r];
            float R = e * pitch / edgeOverR;
            //
            // The star's ten corners, from the bottom valley round the way
            // the strand goes.
            float cx[11], cy[11];
            for (int c = 0; c <= 10; c++) {
                float a = (-90.0f + (STAR_CLOCKWISE ? -36.0f : 36.0f) * c) *
                          (float)M_PI / 180.0f;
                float rr = (c % 2 == 0) ? kValley * R : R;
                cx[c] = rr * cosf(a);
                cy[c] = rr * sinf(a);
            }
            for (int p = 1; p < e * 10; p++) {
                int c = p / e;
                float f = (float)(p % e) / e;
                Pixel px;
                px.index = index++;
                px.ring = r;
                px.tip = (p % e == 0) && (c % 2 == 1);
                px.x = cx[c] + (cx[c + 1] - cx[c]) * f;
                px.y = cy[c] + (cy[c + 1] - cy[c]) * f;
                pixels.push_back(px);
            }
            if (r < kRings - 1) {
                index += STAR_HOP;
            }
        }
        glints.assign(pixels.size(), 0);
    }

    const std::vector<Pixel>& getPixels() { return pixels; }

    /// @brief Draws the star as it is at the given time, in microseconds.
    void draw(uint64_t us) {
        uint32_t frameUs = lastUs == 0 || us < lastUs ? 0 : (uint32_t)(us - lastUs);
        lastUs = us;

        //
        // About one glint every two seconds.
        uint32_t starts = due(1, frameUs / 2, glintCarry);
        while (starts-- > 0) {
            uint i = random16(glints.size());
            if (glints[i] == 0) {
                glints[i] = 1;
            }
        }
        //
        // A glint takes about three seconds to come and go.
        uint32_t glintStep = due(85, frameUs, phaseCarry);

        static const RGB ringColors[kRings] = {
            RGB(235, 110, 12),   // deep gold at the points
            RGB(255, 150, 38),   // gold
            RGB(255, 195, 115),  // warm white center
        };
        static const RGB glint(255, 245, 225);

        //
        // A swell moves out from the center every seven seconds.
        uint8_t swell = (uint8_t)((us % 7000000) * 256 / 7000000);

        for (size_t i = 0; i < pixels.size(); i++) {
            const Pixel& px = pixels[i];
            uint8_t wave = sin8(swell + px.ring * 40);
            //
            // 72% to 100% as the swells pass, a little extra on the points.
            uint8_t level = 184 + scale8(easeInOut(wave), 71);
            RGB c = ringColors[px.ring].scale8(level);
            if (px.tip) {
                c = blend(c, glint, 40);
            }
            uint8_t& g = glints[i];
            if (g != 0) {
                c = blend(c, glint, glintLevel(g));
                g = g + glintStep > 255 ? 0 : g + glintStep;
            }
            strip->putPixel(c, px.index);
        }
    }
};

#endif
