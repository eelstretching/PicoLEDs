#include "SimWindow.h"

namespace SimWindow {

bool available() { return false; }

int run(const char* title) { return 1; }

void waitWhilePaused() {}

void programFinished() {}

}  // namespace SimWindow
