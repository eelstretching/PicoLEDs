#ifndef PACMAN_H
#define PACMAN_H

#pragma once

#include "Sprite.h"

static RGB pacColor(0xaffff00);

static const char *pac1[]={
"13 13 2 1",
". c 255",
"a c 0",
"....aaaaa....",
"..aaaaaaaaa..",
".aaaaaaaaaaa.",
".aaaaaaaaaaa.",
"aaaaaaaaaaaaa",
"aaaaaaaaaaaaa",
"aaaaaaaaaaaaa",
"aaaaaaaaaaaaa",
"aaaaaaaaaaaaa",
".aaaaaaaaaaa.",
".aaaaaaaaaaa.",
"..aaaaaaaaa..",
"....aaaaa...."};

static const char *pac2[]={
"13 13 2 1",
". c 255",
"a c 0",
"....aaaaa....",
"..aaaaaaaaa..",
".aaaaaaaaaaa.",
".aaaaaaaaaaa.",
"aaaaaaaaaa...",
"aaaaaaa......",
"aaaa.........",
"aaaaaaa......",
"aaaaaaaaaa...",
".aaaaaaaaaaa.",
".aaaaaaaaaaa.",
"..aaaaaaaaa..",
"....aaaaa...."};

static const char *pac3[]={
"13 13 2 1",
". c 255",
"a c 0",
"....aaaaa....",
"..aaaaaaa....",
".aaaaaaa.....",
".aaaaaa......",
"aaaaaa.......",
"aaaaa........",
"aaaa.........",
"aaaaa........",
"aaaaaa.......",
".aaaaaa......",
".aaaaaaa.....",
"..aaaaaaa....",
"....aaaaa...."};

class PacMan : public Sprite {

    protected:
    
    Xpm *frames[3];
    uint8_t cmap[2];

    public:
    PacMan(Canvas *canvas, uint8_t pacColorIndex, int startX, int startY);
    ~PacMan();
    Xpm **getXpms() {return frames;};
};

#endif
