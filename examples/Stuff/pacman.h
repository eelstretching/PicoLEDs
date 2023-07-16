#include "Sprite.h"

static const char *pac1[]={
"13 13 2 1",
". c #000000",
"a c #ffff00",
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
". c #000000",
"# c #ffff00",
"....#####....",
"..#########..",
".###########.",
".###########.",
"##########...",
"#######......",
"####.........",
"#######......",
"##########...",
".###########.",
".###########.",
"..#########..",
"....#####...."};

static const char *pac3[]={
"13 13 2 1",
". c #000000",
"# c #ffff00",
"....#####....",
"..#######....",
".#######.....",
".######......",
"######.......",
"#####........",
"####.........",
"#####........",
"######.......",
".######......",
".#######.....",
"..#######....",
"....#####...."};

class PacMan : public Sprite {

    protected:
    
    Xpm xpms[3] = {Xpm(pac1), Xpm(pac2), Xpm(pac3)};

    public:
    PacMan(Canvas *canvas, int startX, int startY);
    Xpm *getXpms() {return xpms;};
};
