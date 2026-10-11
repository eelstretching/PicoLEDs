#ifndef SPRITE_H
#define SPRITE_H

#pragma once

#include <vector>

#include "Animation.h"
#include "Direction.h"
#include "Canvas.h"
#include "Xpm.h"

class Sprite : public Animation {
   protected:
    std::vector<Xpm *> frames;
    int startX;
    int startY;
    int pos;

    int x;
    int y;

    int deltaX;
    int deltaY;

    //
    // A local color map we might want for rendering variants of the same pixmap.
    // Allocated with new[] by subclasses that want one, and freed by us. The
    // frames, on the other hand, belong to whoever added them.
    RGB *myColors = nullptr;

   public:
    Sprite(Canvas *canvas, int startX, int startY, Direction dir) : Animation(canvas, nullptr), startX(startX), startY(startY), deltaX(0), deltaY(0) {
        switch(dir) {
            case Direction::UP:
                deltaY = 1;
                break;
            case Direction::DOWN:
                deltaY = -1;
                break;
            case Direction::LEFT:
                deltaX = -1;
                break;
            case Direction::RIGHT:
                deltaX = 1;
                break;
        }
    };

    Sprite(Canvas *canvas, int startX, int startY, int deltaX, int deltaY)
        : Animation(canvas, nullptr), startX(startX), startY(startY), deltaX(deltaX), deltaY(deltaY) {};
    void add(Xpm *frame);
    /// @brief Copies another sprite's frames (which are shared, not copied)
    /// and start position. The copy doesn't get the other's myColors.
    Sprite(Sprite &o);
    Sprite& operator=(const Sprite&) = delete;
    ~Sprite() { delete[] myColors; }
    std::vector<Xpm *> &getFrames() { return frames; };
    
    void setStartPosition(int startX, int startY) {
        this->startX = startX;
        this->startY = startY;
    }

    void setDirection(int deltaX, int deltaY) {
        this->deltaX = deltaX;
        this->deltaY = deltaY;
    }

    int getDeltaX() { return deltaX; };
    int getDeltaY() { return deltaY; };

    void setDirection(Direction dir) {
        deltaX = 0;
        deltaY = 0;
        switch(dir) {
            case Direction::UP:
                deltaY = 1;
                break;
            case Direction::DOWN:
                deltaY = -1;
                break;
            case Direction::LEFT:
                deltaX = -1;
                break;
            case Direction::RIGHT:
                deltaX = 1;
                break;
        }
    }

    uint getWidth();
    uint getHeight();

    int getX() {return x;};
    int getY() {return y;};

    void init();
    bool step();
};

#endif
