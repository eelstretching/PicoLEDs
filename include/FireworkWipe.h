#ifndef FIREWORK_WIPE
#define FIREWORK_WIPE

#pragma once

#include "Animation.h"
#include "Firework.h"

/// @brief A wipe that clears the screen with fireworks.
class FireworkWipe : public Animation {
   protected:
    /// @brief Our fireworks
    Firework **fw;
    /// @brief The number of fireworks.
    int nf;

   public:
    FireworkWipe(Canvas *canvas, ColorMap *colorMap);
    FireworkWipe(Canvas *canvas, ColorMap *colorMap, Firework **fw, int nf);
    void init();
    bool step();
    void finish();
    Firework **getFireworks() { return fw; };
    int getNumFireWorks(){return nf;};
    int getFPSNeeded() {return 55;};
};
#endif