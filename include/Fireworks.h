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
    /// @brief Whether we made the fireworks (and so delete them), or borrowed
    /// them from whoever passed them in.
    bool ownsFireworks;

   public:
    Fireworks(Canvas *canvas, ColorMap *colorMap);
    /// @brief Uses fireworks that someone else made. They still belong to
    /// the caller, who has to keep them around until this is deleted.
    Fireworks(Canvas *canvas, ColorMap *colorMap, Firework **fw, int nf);
    ~Fireworks();
    Fireworks(const Fireworks&) = delete;
    Fireworks& operator=(const Fireworks&) = delete;
    void init();
    bool step();
    void finish();
    Firework **getFireworks() { return fw; };
    int getNumFireworks() {
        return nf;
    };
};
#endif