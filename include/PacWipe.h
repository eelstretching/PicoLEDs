#ifndef PACWIPE_H
#define PACWIPE_H

#pragma once

#include "Animation.h"
#include "Ghost.h"
#include "PacMan.h"

class PacWipe : public Animation {
    friend class PacChase;

   protected:
    Xpm* ghostFrames[2];

    Sprite** sprites;
    Sprite* curr;

   public:
    PacWipe(Canvas* canvas, ColorMap* colorMap);
    void init();
    bool step();
};

#endif