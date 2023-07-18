#ifndef SCOUTLAW_H
#define SCOUTLAW_H

#pragma once

#include "Animation.h"
#include "Font.h"
#include "pico/types.h"

class ScoutLaw : public Animation {
   protected:
    uint scrollDuration = 1000 * 1000;
    uint8_t state;
    uint64_t start;
    int curr;
    int cw;
    int mid;

    bool single;

    /// @brief Where we'll render the current element.
    int scrollX = 0;

    Font *font;
    uint n = 12;
    const char *sis = "A SCOUT IS ";
    const char *law[12] = {"TRUSTWORTY", "LOYAL", "HELPFUL",  "FRIENDLY",
                           "COURTEOUS",  "KIND",  "OBEDIENT", "CHEERFUL",
                           "THRIFTY",    "BRAVE", "CLEAN",    "REVERENT"};

   public:
    ScoutLaw(Canvas *canvas, Font *font);
    void init();
    bool step();
};

#endif