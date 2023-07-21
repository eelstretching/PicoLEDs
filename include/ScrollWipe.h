#ifndef SCROLLWIPE_H
#define SCROLLWIPE_H

#include "Animation.h"
#include "Direction.h"
#include "pico/stdlib.h"
#include "pico/types.h"

class ScrollWipe : public Animation {
    /// @brief How much we've scrolled so far in the animation.
    uint scrolled;
    /// @brief The amount that we need to scroll, dependent on direction.
    uint limit;
    /// @brief The direction that we'll scroll.
    Direction dir;
    /// @brief Whether we should have some extra frames at the end.
    uint extraFrames = 0;

   public:
    /// @brief Create a scroll wipe on the canvas for the given direction.
    /// @param canvas
    /// @param dir
    ScrollWipe(Canvas *canvas, Direction dir);
    /// @brief Initializes the scroll to start from 0 again and scroll all the
    /// way in the given direction.
    void init();
    /// @brief Take a step in the scrolling direction.
    /// @return true if there's more scrolling to come, false otherwise.
    bool step();
    void setExtraFrames(uint extraFrames);
};

#endif