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
            p = i+1;
            el++;
        }
    }

    //
    // Colors.
    char *cc = new char[nc];
    RGB *colors = new RGB[nc];
    char b[10];
    for (int i = 0; i < nc; i++) {
        const char *row = xpm[i + 1];
        cc[i] = row[0];
        colors[i] = RGB(hextoi(row[5], row[6]), hextoi(row[7], row[8]),
                        hextoi(row[9], row[10]));
    }

    pixels = new RGB[w * h];
    p = 0;
    //
    // Yeah, this is a triple-barrelled loop, but it only runs once per pixmap!
    for (int i = nc + 1; i < nc + 1 + h; i++) {
        for (int j = 0; j < w; j++) {
            char p = xpm[i][j];
            for (int k = 0; k < nc; k++) {
                if (cc[k] == p) {
                    pixels[p++] = RGB(colors[k]);
                    break;
                }
            }
        }

        delete[] cc;
        delete[] colors;
    }
}

void Xpm::render(Canvas *canvas, uint x, uint y) {
    int cy = y + h - 1;
    int p = 0;
    char b[10];
    for (int i = 0; i < h; i++) {
        int cx = x;
        for (int j = 0; j < w; j++) {
            printf("cx: %d cy: %d pix: %s\n", cx, cy, pixels[p].toString(b, 10));
            canvas->set(cx++, cy, pixels[p++]);
        }
        cy--;
    }
}
