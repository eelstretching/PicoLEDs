#include "Fireworks2D.h"

#include <math.h>

#include "math8.h"
#include "pico/rand.h"

/// @brief Classic firework colors, for when there's no palette.
static const RGB fireworkColors[] = {
    RGB(255, 20, 10),   // red
    RGB(20, 255, 40),   // green
    RGB(40, 80, 255),   // blue
    RGB(255, 170, 30),  // gold
    RGB(190, 40, 255),  // purple
    RGB(30, 220, 255),  // cyan
    RGB(255, 60, 160),  // pink
    RGB(255, 240, 220), // white
};
static const int numFireworkColors =
    sizeof(fireworkColors) / sizeof(fireworkColors[0]);

/// @brief The flare going up burns a warm white, and drops orange embers.
static const RGB flareColor(255, 190, 110);
static const RGB emberColor(200, 90, 20);

/// @brief Willows cool from gold to a deep orange.
static const RGB willowColor(255, 160, 40);
static const RGB willowEnd(160, 30, 0);

float Fireworks2D::frand() {
    return (get_rand_32() & 0xFFFFFF) / 16777216.0f;
}

Fireworks2D::Fireworks2D(Canvas* canvas, uint launchInterval, uint maxShells,
                         uint maxSparks)
    : Animation(canvas, nullptr),
      maxShells(maxShells),
      maxSparks(maxSparks),
      launchInterval(MAX(launchInterval, 1)) {
    shells.reserve(maxShells);
    sparks.reserve(maxSparks);
    //
    // Enough gravity that a shell going to 70% of the height takes a bit over
    // a second to get there at 30 steps a second.
    gravity = canvas->getHeight() * 0.0009f;
}

void Fireworks2D::init() {
    shells.clear();
    sparks.clear();
    untilLaunch = 0;
}

RGB Fireworks2D::randomColor() {
    if (usePalette) {
        return ColorFromPalette(palette, random8(), 255, LINEARBLEND);
    }
    return fireworkColors[random8(numFireworkColors)];
}

void Fireworks2D::addSpark(float x, float y, float vx, float vy, uint16_t life,
                           const RGB& color, uint8_t drag, ShellType type) {
    if (sparks.size() >= maxSparks) {
        return;
    }
    Spark2D s;
    s.x = s.px = x;
    s.y = s.py = y;
    s.vx = vx;
    s.vy = vy;
    s.age = 0;
    s.life = life;
    s.color = color;
    s.drag = drag;
    s.type = type;
    sparks.push_back(s);
}

void Fireworks2D::launch() {
    float w = canvas->getWidth();
    float h = canvas->getHeight();
    launch((ShellType)random8(NUM_SHELL_TYPES), w * frand(0.15, 0.85),
           h * frand(0.55, 0.85));
}

void Fireworks2D::launch(ShellType type, float x, float burstY) {
    if (shells.size() >= maxShells) {
        return;
    }
    Shell s;
    s.x = s.px = x;
    s.y = s.py = 0;
    //
    // Fast enough that gravity stops it right at burstY.
    s.vy = sqrtf(2 * gravity * burstY);
    //
    // And a little sideways drift, up to a tenth of the width by the time it
    // bursts.
    float steps = s.vy / gravity;
    s.vx = frand(-1, 1) * 0.1f * canvas->getWidth() / steps;
    s.type = type;
    s.color = type == WILLOW ? willowColor : randomColor();
    s.color2 = randomColor();
    shells.push_back(s);
}

void Fireworks2D::burst(ShellType type, float x, float y, const RGB& color,
                        const RGB& color2) {
    float d = MIN(canvas->getWidth(), canvas->getHeight());
    float radius = burstSize * d / 2 * frand(0.8, 1.2);

    uint8_t drag;
    uint16_t life;
    switch (type) {
        case WILLOW:
            drag = 228;
            life = 75;
            break;
        case RING:
            drag = 232;
            life = 40;
            break;
        default:
            drag = 235;
            life = 40;
            break;
    }
    //
    // With drag, a spark goes v0 / (1 - drag) pixels in total, so this gets
    // the sparks out to about the radius we want.
    float keep = drag / 256.0f;
    float v0 = radius * (1 - keep);

    int n = type == RING ? 20 + radius * 2 : 30 + radius * 5;
    for (int i = 0; i < n; i++) {
        float vx, vy;
        if (type == RING) {
            float a = 2 * M_PI * i / n;
            vx = v0 * cosf(a);
            vy = v0 * sinf(a);
        } else {
            //
            // A point picked evenly from the surface of a sphere and squashed
            // flat, which is how a real burst looks from the ground: denser
            // at the edge than in the middle.
            float z = frand(-1, 1);
            float r = sqrtf(1 - z * z) * frand(0.9, 1.05);
            float a = frand(0, 2 * M_PI);
            vx = v0 * r * cosf(a);
            vy = v0 * r * sinf(a);
        }
        const RGB& c = type == TWO_TONE && (i & 1) ? color2 : color;
        addSpark(x, y, vx, vy, life + random8(life / 3), c, drag, type);
    }
}

void Fireworks2D::splat(float x, float y, const RGB& color) {
    int ix = floorf(x);
    int iy = floorf(y);
    uint8_t fx = (x - ix) * 255;
    uint8_t fy = (y - iy) * 255;
    uint8_t w[4] = {
        scale8(255 - fx, 255 - fy),
        scale8(fx, 255 - fy),
        scale8(255 - fx, fy),
        scale8(fx, fy),
    };
    int w8 = canvas->getWidth();
    int h8 = canvas->getHeight();
    for (int i = 0; i < 4; i++) {
        int px = ix + (i & 1);
        int py = iy + (i >> 1);
        if (w[i] == 0 || px < 0 || py < 0 || px >= w8 || py >= h8) {
            continue;
        }
        RGB c = canvas->get(px, py);
        c += color.scale8(w[i]);
        canvas->set(px, py, c);
    }
}

void Fireworks2D::streak(float x0, float y0, float x1, float y1,
                         const RGB& color) {
    float dist = MAX(fabsf(x1 - x0), fabsf(y1 - y0));
    int n = MAX((int)ceilf(dist), 1);
    for (int i = 1; i <= n; i++) {
        float t = (float)i / n;
        splat(x0 + (x1 - x0) * t, y0 + (y1 - y0) * t, color);
    }
}

RGB Fireworks2D::sparkColor(const Spark2D& s) {
    uint8_t t = s.age * 255 / s.life;
    if (s.type == WILLOW) {
        return s.color.lerp8(willowEnd, t).scale8(255 - t);
    }
    //
    // White hot for a moment, then into the spark's color.
    if (s.age < 4) {
        return RGB(RGB::White).lerp8(s.color, s.age * 64);
    }
    if (s.type == CRACKLE && t > 128) {
        return random8() < 40 ? RGB(RGB::White) : s.color.scale8(255 - t).scale8(80);
    }
    //
    // Fade out as 1 - t^2, so the sparks stay bright for most of their life
    // and then die off quickly.
    return s.color.scale8(255 - scale8(t, t));
}

void Fireworks2D::stepShells() {
    for (size_t i = 0; i < shells.size();) {
        Shell& s = shells[i];
        s.px = s.x;
        s.py = s.y;
        s.x += s.vx;
        s.y += s.vy;
        s.vy -= gravity;
        if (s.vy <= 0) {
            burst(s.type, s.x, s.y, s.color, s.color2);
            shells[i] = shells.back();
            shells.pop_back();
            continue;
        }
        streak(s.px, s.py, s.x, s.y, flareColor);
        //
        // An ember falling off the back.
        addSpark(s.x, s.y, frand(-0.05, 0.05), s.vy * 0.1f,
                 6 + random8(8), emberColor, 220, PEONY);
        i++;
    }
}

void Fireworks2D::stepSparks() {
    int w = canvas->getWidth();
    for (size_t i = 0; i < sparks.size();) {
        Spark2D& s = sparks[i];
        s.px = s.x;
        s.py = s.y;
        s.x += s.vx;
        s.y += s.vy;
        float keep = s.drag / 256.0f;
        s.vx *= keep;
        s.vy = s.vy * keep - gravity * 0.5f;
        s.age++;
        if (s.age >= s.life || s.y < -1 || s.x < -1 || s.x > w) {
            sparks[i] = sparks.back();
            sparks.pop_back();
            continue;
        }
        streak(s.px, s.py, s.x, s.y, sparkColor(s));
        i++;
    }
}

void Fireworks2D::fadeCanvas() {
    if (trailFade == 0) {
        return;
    }
    int w = canvas->getWidth();
    int h = canvas->getHeight();
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            RGB c = canvas->get(x, y);
            if (c) {
                canvas->set(x, y, c.nscale8(trailFade));
            }
        }
    }
}

bool Fireworks2D::step() {
    fadeCanvas();
    if (untilLaunch == 0) {
        launch();
        uint interval = finale ? MAX(launchInterval / 6, 2) : launchInterval;
        untilLaunch = interval * frand(0.5, 1.5) + 1;
    }
    untilLaunch--;
    stepSparks();
    stepShells();
    return true;
}
