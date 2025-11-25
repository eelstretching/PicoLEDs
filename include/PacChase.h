#ifndef PACCHASE_H
#define PACCHASE_H

#pragma once 

#include "Animation.h"
#include "Xpm.h"
#include "Sprite.h"
#include "PacWipe.h"

static const char *power[]={
"8 8 2 1",
"# c #ffb8ae",
". c #000000",
"..####..",
".######.",
"########",
"########",
"########",
"########",
".######.",
"..####.."};

static const char *pilled1[]={
"14 14 3 1",
". c #000000",
"# c #2121ff",
"a c #ffb8ae",
".....####.....",
"...########...",
"..##########..",
".############.",
".############.",
".###aa##aa###.",
"####aa##aa####",
"##############",
"##############",
"##aa##aa##aa##",
"#a##aa##aa##a#",
"##############",
"####.####.####",
".##...##...##."};

static const char *pilled2[]={
"14 14 3 1",
". c #000000",
"# c #2121ff",
"a c #ffb8ae",
".....####.....",
"...########...",
"..##########..",
".############.",
".############.",
".###aa##aa###.",
"####aa##aa####",
"##############",
"##############",
"##aa##aa##aa##",
"#a##aa##aa##a#",
"##############",
"##.###..###.##",
"#...##..##...#"};

static const RGB dotColor = RGB(255, 184, 174);


#define PLAIN 1
#define POWER_PILL 2
class PacChase : public Animation {

    friend class PacWipe;

    protected:
    ColorMap *colorMap;
    Sprite *pacMan;
    uint8_t pmw;
    Sprite **ghosts;
    uint8_t ghw;
    Sprite **pilled;
    Sprite *g1;
    Sprite *g2;
    Xpm *pill;
    uint8_t piw;
    uint8_t state;
    uint8_t dotColorIndex;

    
    public:
    PacChase(Canvas *canvas, ColorMap *colorMap);
    PacChase(PacWipe *wipe, ColorMap *colorMap);
    void setup();
    void drawDot(int i);
    void init();
    bool step();
};
#endif
