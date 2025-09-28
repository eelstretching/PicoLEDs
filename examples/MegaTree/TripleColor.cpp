#include "TripleColor.h"

TripleColor::TripleColor(Canvas* canvas) : canvas(canvas) {}

TripleColor::~TripleColor() {}

bool TripleColor::step() {
    canvas->clear();
    canvas->fillRow(currRow % canvas->getHeight(), 1);
    canvas->fillRow((currRow + 1) % canvas->getHeight(), 2);
    canvas->fillRow((currRow + 2) % canvas->getHeight(), 3);
    currRow = (currRow + 1) % canvas->getHeight();
    return true;
 }
