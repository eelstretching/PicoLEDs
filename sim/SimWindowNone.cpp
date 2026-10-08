#include "SimWindow.h"

namespace SimWindow {

bool available() { return false; }

int run(const char* title) { return 1; }

uint64_t waitToShow() { return 0; }

void programFinished() {}

}  // namespace SimWindow
