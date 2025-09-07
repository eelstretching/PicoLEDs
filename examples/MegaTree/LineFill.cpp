#include "LineFill.h"

LineFill::LineFill(Canvas* canvas, RGB* colors, int n, Direction direction)
    : canvas(canvas),
      colors(colors),
      n(n),
      direction(direction),
      state(FILLING) {
    divisions = new int[n];
    int stride;
    switch (direction) {
        case UP:
        case DOWN:
            stride = canvas->getWidth() / n;
            break;
        case LEFT:
        case RIGHT:
            stride = canvas->getHeight() / n;
            break;
    }
    switch (direction) {
        case UP:
            for (int i = 0, t = canvas->getWidth() - 1; i < n; i++, stride) {
                divisions[i] = t;
            }
            divisions[n - 1] = 0;
            break;
        case DOWN:
            for (int i = 0, t = 0; i < n; i++, t += stride) {
                divisions[i] = t;
            }
            divisions[n - 1] = canvas->getWidth() - 1;
            break;
        case LEFT:
            for (int i = 0, t = 0; i < n; i++, t += stride) {
                divisions[i] = t;
            }
            divisions[n - 1] = canvas->getHeight() - 1;
            break;
        case RIGHT:
            for (int i = 0, t = canvas->getHeight() - 1; i < n;
                 i++, t -= stride) {
                divisions[i] = t;
            }
            divisions[n - 1] = 0;
            break;
    }
    currIndex = 0;
    currPos = divisions[n - 1];
    currTarget = divisions[0];
}

bool LineFill::step() {
    bool done = false;
    if (state == FILLING) {
        //
        // Erase the previous line and then draw the new one.
        switch (direction) {
            case UP:
                canvas->fillColumn(currPos - 1, canvas->getBackground());
                canvas->fillColumn(currPos, colors[currIndex]);
                break;
            case DOWN:
                canvas->fillColumn(currPos + 1, canvas->getBackground());
                canvas->fillColumn(currPos, colors[currIndex]);
                break;
            case LEFT:
                canvas->fillRow(currPos - 1, canvas->getBackground());
                canvas->fillRow(currPos, colors[currIndex]);
                break;
            case RIGHT:
                canvas->fillRow(currPos + 1, canvas->getBackground());
                canvas->fillRow(currPos, colors[currIndex]);
                break;
        }
        //
        // See if we hit the division marker and switch to flashing for a bit.
        if (currPos == currTarget) {
            if (currPos == divisions[currIndex]) {
                state = FLASHING;
                currFlash = 0;
            } else {
                //
                // Move the target and reset the current position.
                switch (direction) {
                    case UP:
                        currTarget--;
                        currPos = 0;
                        break;
                    case DOWN:
                        currTarget++;
                        currPos = canvas->getWidth() - 1;
                        break;
                    case LEFT:
                        currTarget--;
                        currPos = 0;
                        break;
                    case RIGHT:
                        currTarget++;
                        currPos = canvas->getHeight() - 1;
                        break;
                }
            }
        } else {
            //
            // Just move the drawing position.
            switch (direction) {
                case UP:
                    currPos++;
                    break;
                case DOWN:
                    currPos--;
                    break;
                case LEFT:
                    currPos++;
                    break;
                case RIGHT:
                    currPos--;
                    break;
            }
        }
        return true;
    } else if (state == FLASHING) {
        //
        // We'll flash with white when we hit a color change, so fill it in when
        // we start flashing.
        if (currFlash == 0) {
            switch (direction) {
                case UP:
                case DOWN:
                    canvas->fillColumn(currTarget, RGB::White);
                    break;
                case LEFT:
                case RIGHT:
                    canvas->fillRow(currTarget, RGB::White);
                    break;
            }
        } else if (currFlash == flashCount) {
            //
            // Move the target and reset the current position.
            switch (direction) {
                case UP:
                    canvas->fillColumn(currTarget, colors[currIndex]);
                    currTarget--;
                    currPos = 0;
                    break;
                case DOWN:
                    canvas->fillColumn(currTarget, colors[currIndex]);
                    currTarget++;
                    currPos = canvas->getWidth() - 1;
                    break;
                case LEFT:
                    canvas->fillRow(currTarget, colors[currIndex]);
                    currTarget--;
                    currPos = 0;
                    break;
                case RIGHT:
                    canvas->fillRow(currTarget, colors[currIndex]);
                    currTarget++;
                    currPos = canvas->getHeight() - 1;
                    break;
            }

            state = FILLING;
            //
            // Move onto the next index.
            currIndex++;
            if (currIndex >= n) {
                return false;
            }
        }
        currFlash++;
        return true;
    }
    return true;
}
