#ifndef FIREWORK_WIPE
#define FIREWORK_WIPE

#pragma once

#include "Animation.h"
#include "Firework.h"

/// @brief A wipe that clears the screen with fireworks.
class FireworkWipe : public Animation {
   protected:
    Firework **fw;

   public:
    FireworkWipe(Canvas *canvas);
    void init();
    bool step();
    void finish();
};
#endif