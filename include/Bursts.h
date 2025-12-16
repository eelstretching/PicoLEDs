#ifndef BURSTS_H
#define BURSTS_H

#include "Animation.h"
#include "pico/stdlib.h"
#include "pico/types.h"

enum BurstState { BURSTING, WAITING };

class Burst : public Animation {
    uint8_t p;
    uint8_t tailLen;
    uint8_t speed;
    BurstState state;

   public:
    Burst(Canvas* canvas, uint8_t tailLen) : Animation(canvas, nullptr, 50), tailLen(tailLen) {};
    virtual void init() override;
    virtual bool step() override;
};

class Bursts : public Animation {
    uint8_t maxBursts;
    uint8_t nBursts;
    uint8_t tailLen;
    Burst **bursts;
    public:
    Bursts(Canvas *canvas, RGB backGround, uint8_t maxBursts, RGB burstColor, uint8_t tailLen);
    ~Bursts();
    virtual void init() override;
    virtual bool step() override;
};
#endif