#ifndef COLORUTIL_H
#define COLORUTIL_H

#include "colorutil.h"
#include "color.h"

#include "math8.h"
#include "pico/stdlib.h"
#include "pico/types.h"


void fill_gradient_RGB( RGB* data,
                   uint16_t startpos, RGB startcolor,
                   uint16_t endpos,   RGB endcolor );
#endif
