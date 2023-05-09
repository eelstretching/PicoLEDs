#include "Canvas.h"

#include "View.h"

Canvas::Canvas(uint width, uint height) : width(width), height(height) {
    numPixels = width * height;
    data = new RGB[numPixels];
}

void Canvas::set(uint x, uint y, RGB c) {
    if (x >= width || y >= height) {
        //
        // Off the canvas.
        return;
    }

    data[getPos(x,y)] = c;
}

void Canvas::fill(uint row, RGB c) {
    if(row >= height) {
        return;
    }
    uint pos = row * width;
    for(int i = 0; i < width; i++) {
        data[pos++] = c;
    }
}

uint Canvas::getPos(uint x, uint y) {
    return y * width + x;
}

void Canvas::clear() {
    for(int i = 0; i < numPixels; i++) {
        data[i] = RGB::Black;
    }
}

void Canvas::setView(View *v, uint x, uint y) {
    view = v;
    view->setCanvas(this);
    viewX = x;
    viewY = y;
}

void Canvas::show() {
    view->render();
}
