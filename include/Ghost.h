#ifndef GHOST_H
#define GHOST_H

#pragma once 

#include "Sprite.h"

/// @brief Ghost colors
static RGB inkyColor = RGB(0, 255, 255);
static RGB blinkyColor = RGB(255, 0, 0);
static RGB pinkyColor = RGB(255, 184, 255);
static RGB clydeColor = RGB(255, 184, 81);
static RGB pupilColor = RGB(33, 33, 255);


//
// These ghosts have no blue part in their eyes because we'll render that according to their direction
static const char *ghost1[]={
"14 14 3 1",
". c B",
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
". c B",
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

    //
    // Map for our ghost's colors.
    RGB ghostColor;
    RGB pupilColor;
    Xpm **frames;
    void drawPupils();

   public:
    Ghost(Canvas *canvas, Xpm **frames, const RGB& ghostColor, const RGB& pupilColor, int startX, int startY, Direction dir);
    ~Ghost();
    void init() override;
    bool step();
};

#endif
