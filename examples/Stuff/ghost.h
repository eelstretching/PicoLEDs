#ifndef GHOST_H
#define GHOST_H

#pragma once 
#include "Sprite.h"

static RGB inkyColor = RGB(0, 255, 255);
static RGB blinkyColor = RGB(255, 0, 0);
static RGB pinkyColor = RGB(255, 184, 255);
static RGB clydeColor = RGB(255, 184, 81);

//
// These ghosts have no blue part in their eyes because we'll render that according to their direction

static const char *ghost1[]={
"14 14 3 1",
". c #000000",
"a c #dedeff",
"# c #ff0000",
".....####.....",
"...########...",
"..##########..",
".#aa####aa###.",
".aaaa##aaaa##.",
".aaaa##aaaa##.",
"#aaaa##aaaa###",
"##aa####aa####",
"##############",
"##############",
"##############",
"##############",
"##.###..###.##",
"#...##..##...#"};

static const char *ghost2[]={
"14 14 3 1",
". c #000000",
"a c #dedeff",
"# c #ff0000",
".....####.....",
"...########...",
"..##########..",
".#aa####aa###.",
".aaaa##aaaa##.",
".aaaa##aaaa##.",
"#aaaa##aaaa###",
"##aa####aa####",
"##############",
"##############",
"##############",
"##############",
"####.####.####",
".##...##...##."};

class Ghost : public Sprite {
   protected:
    Xpm xpms[2] = {Xpm(ghost1), Xpm(ghost2)};

    /// @brief A color map for rendering.
    RGB *colorMap;

    RGB pupil = RGB(33, 33, 255);

    void drawPupil();

   public:
    Ghost(Canvas *canvas, RGB color, uint8_t startX, uint8_t startY);
    bool step();
};

#endif