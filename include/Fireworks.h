#ifndef FIREWORKS_H
#define FIREWORKS_H

#include "Animation.h"
#include "Firework.h"

#pragma once

class Fireworks : public Animation {
   protected:
    /// @brief Our fireworks
    Firework **fw;
    /// @brief The number of fireworks.
    int nf;

   public:
    Fireworks(Canvas *canvas);
    Fireworks(Canvas *canvas, Firework **fw, int nf);
    void init();
    bool step();
    void finish();
    Firework **getFireworks() { return fw; };
};
#endif