#include "ScrollText.h"

ScrollText::ScrollText(Canvas* canvas, ColorMap* colorMap, Font* font) : TextAnimation(canvas, colorMap, font) {}

void ScrollText::init() {
    canvas->setColorMap(colorMap);
    canvas->clear();
    for(auto el : elements) {
        el->width = font->getWidth(el->text);
        el->waiting = false;
    }
    allOffCanvas = false;
}

bool ScrollText::step() {
    canvas->clear();
    //
    // See if all the text elements are off the canvas.
    allOffCanvas = true;
    for(auto el : elements) {
        if(!el->waiting) {
            allOffCanvas = false;
            break;
        }
    }
    //
    // If they are, the reset them all to their starting positions.
    if(allOffCanvas) {
        for(auto el : elements) {
            el->x = el->startx;
            el->y = el->starty;
            el->waiting = false;
        }
    }
    for(auto el : elements) {
        if(el->waiting) {
            continue;
        }
        font->render(el->text, el->x, el->y, el->color, angle);
        switch(angle) {
            case RENDER_0:
                el->x++;
                if(el->x >= canvas->getWidth()) {
                    el->waiting = true;
                    el->x = -((int)el->width);
                }
                break;
            case RENDER_90:
                el->y++;
                if(el->y >= canvas->getHeight()+el->width) {
                    el->waiting = true;
                    el->y = 0;
                }
                break;
            default:
                break;
        }

    }
    return true;
 }
