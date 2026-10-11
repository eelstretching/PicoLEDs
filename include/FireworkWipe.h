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
    /// @brief Whether we made the fireworks (and so delete them), or borrowed
    /// them from whoever passed them in.
    bool ownsFireworks;

   public:
    FireworkWipe(Canvas *canvas, ColorMap *colorMap);
    /// @brief Uses fireworks that someone else made. They still belong to
    /// the caller, who has to keep them around until this is deleted.
    FireworkWipe(Canvas *canvas, ColorMap *colorMap, Firework **fw, int nf);
    ~FireworkWipe();
    FireworkWipe(const FireworkWipe&) = delete;
    FireworkWipe& operator=(const FireworkWipe&) = delete;
    void init();
    bool step();
    void finish();
    Firework **getFireworks() { return fw; };
    int getNumFireWorks(){return nf;};
};
#endif