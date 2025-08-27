#include "TripleColor.h"

TripleColor::TripleColor(Canvas* canvas, RGB* colors) : canvas(canvas), colors(colors) {}

TripleColor::~TripleColor() {}

bool TripleColor::step() {
    canvas->clear();
    canvas->fillRow(currRow % canvas->getHeight(), colors[0]);
    canvas->fillRow((currRow + 1) % canvas->getHeight(), colors[1]);
    canvas->fillRow((currRow + 2) % canvas->getHeight(), colors[2]);
    currRow = (currRow + 1) % canvas->getHeight();
    return true;
 }
