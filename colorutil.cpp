#include "colorutil.h"

void fill_gradient_RGB( RGB* leds,
                   uint16_t startpos, RGB startcolor,
                   uint16_t endpos,   RGB endcolor )
{
    // if the points are in the wrong order, straighten them
    if( endpos < startpos ) {
        uint16_t t = endpos;
        RGB tc = endcolor;
        endcolor = startcolor;
        endpos = startpos;
        startpos = t;
        startcolor = tc;
    }

    int16_t rdistance87;
    int16_t gdistance87;
    int16_t bdistance87;

    rdistance87 = (endcolor.r - startcolor.r) << 7;
    gdistance87 = (endcolor.g - startcolor.g) << 7;
    bdistance87 = (endcolor.b - startcolor.b) << 7;

    uint16_t pixeldistance = endpos - startpos;
    int16_t divisor = pixeldistance ? pixeldistance : 1;

    int16_t rdelta87 = rdistance87 / divisor;
    int16_t gdelta87 = gdistance87 / divisor;
    int16_t bdelta87 = bdistance87 / divisor;

    rdelta87 *= 2;
    gdelta87 *= 2;
    bdelta87 *= 2;

    uint16_t r88 = startcolor.r << 8;
    uint16_t g88 = startcolor.g << 8;
    uint16_t b88 = startcolor.b << 8;
    for( uint16_t i = startpos; i <= endpos; ++i) {
        leds[i] = RGB( r88 >> 8, g88 >> 8, b88 >> 8);
        r88 += rdelta87;
        g88 += gdelta87;
        b88 += bdelta87;
    }
}