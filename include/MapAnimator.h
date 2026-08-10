#ifndef MAP_ANIMATOR_H
#define MAP_ANIMATOR_H

#include <Animator.h>

class MapAnimator : public Animator {
   protected:
    int curr;
    std::vector<int> keys;

   public:
    MapAnimator(Canvas* canvas) : Animator(canvas) {};
    MapAnimator(Canvas* canvas, int fps) : Animator(canvas, fps) {}
    void add(int key, Animation* a);
    void set(int key);
    void animationChanged() override;
};

#endif  // MAP_ANIMATOR_H