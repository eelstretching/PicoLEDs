#include "Xpm.h"

#include "color.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

/// @brief Single hex char to integer
/// @param c the character, which we're going to assume is 0-9 or a-f.
/// @return the integer representation of the character.
uint8_t hextoi(char c) {
    uint8_t r;
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else {
        return c - 'a' + 10;
    }
}
uint8_t hextoi(char c1, char c2) { return hextoi(c1) << 4 | hextoi(c2); }

uint8_t remap(uint8_t *remap, uint8_t o) {
    if(remap == nullptr || o == 255) {
        return o;
    }
    return remap[o];
}

Xpm::Xpm(const char *xpm[]) {
    char holder[3];

    //
    // width, height, and number of colors.
    const char *vals = xpm[0];
    int l = strlen(vals);
    int el = 0;
    int p = 0;
    for (int i = 0; i < l; i++) {
        if (vals[i] == ' ' || vals[i] == 0) {
            memcpy(holder, &vals[p], i - p);
            holder[i - p] = 0;
            int v = atoi(holder);
            switch (el) {
                case 0:
                    w = v;
                    break;
                case 1:
                    h = v;
                    break;
                case 2:
                    nc = v;
                    break;
            }
            p = i + 1;
            el++;
        }
    }

    //
    // Colors.
    char *cc = new char[nc];
    colorIndexes = new uint8_t[nc];
    for (int i = 0; i < nc; i++) {
        const char *row = xpm[i + 1];
        cc[i] = row[0];
        colorIndexes[i] = atoi(&row[3]);
    }

    //
    // The pixel values are just an index into the colors table. We're limited
    // to 256 24-bit colors by this hack, but it makes the Xpms one-third the
    // size, and lets us re-color pixmaps on the fly.
    pixels = new uint8_t[w * h];
    p = 0;
    //
    // Yeah, this is a triple-barrelled loop, but it only runs once per pixmap!
    for (int i = nc + 1; i < nc + 1 + h; i++) {
        const char *row = xpm[i];
        for (int j = 0; j < w; j++) {
            char pc = row[j];
            for (int k = 0; k < nc; k++) {
                if (cc[k] == pc) {
                    pixels[p++] = colorIndexes[k];
                    break;
                }
            }
        }
    }
    delete[] cc;
}

Xpm::Xpm(const Xpm& other) : nc(other.nc), h(other.h), w(other.w) {
    pixels = new uint8_t[w*h];
    memcpy(pixels, other.pixels, w*h*sizeof(uint8_t));
    colorIndexes = new uint8_t[nc];
    memcpy(colorIndexes, other.colorIndexes, nc*sizeof(uint8_t));    
}

void Xpm::replaceColor(uint8_t oldColor, uint8_t newColor) {
    for (int i = 0; i < w * h; i++) {
        if (pixels[i] == oldColor) {
            pixels[i] = newColor;
        }
    }
}

bool Xpm::render(Canvas *canvas, uint x, uint y) {
    return render(canvas, nullptr, x, y);
}

bool Xpm::render(Canvas *canvas, uint8_t *map, uint x, uint y) {
    bool atLeastOnePixel = false;
    int cy = y + h - 1;
    int p = 0;
    char b[10];
    for (int i = 0; i < h; i++) {
        int cx = x;
        for (int j = 0; j < w; j++) {
            if (canvas->set(cx++, cy, remap(map, pixels[p]))) {
                atLeastOnePixel = true;
            }
            p++;
        }
        cy--;
    }
    return atLeastOnePixel;
}

void Xpm::dump() {
    int p = 0;
    printf("w: %d h: %d nc: %d\n", w, h, nc);
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            RGB x = pixels[p++];
            printf("#%02X%02X%02X ", x.r, x.g, x.b);
        }
        printf("\n");
    }
}
