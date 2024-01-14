#include "Animator.h"
#include "Canvas.h"
#include "Firework.h"
#include "Strip.h"
#include "pico/printf.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/types.h"

int main() {
    stdio_init_all();

    //
    // A canvas made out of strips.
    Canvas canvas(138);
    Strip strips[] = {Strip(2, 552),
                      Strip(3, 552)};  // , Strip(3, 276)}; //,Strip(4, 138)};
    int ns = 2;
    for (int i = 0; i < ns; i++) {
        strips[i].setFractionalBrightness(128);
        canvas.add(strips[i]);
    }
    int delay = 100;

    canvas.clear();

    RGB clarkGreen(30, 110, 10);
    RGB clarkGold(255, 205, 0);

    //
    // A palette that goes from black, through clarkson green, to clarkson gold.
    RGBPalette16 palette(RGB(0, 0, 0), clarkGreen, clarkGold);

    // for (int i = 0; i < 20; i++) {
    //     canvas.fill(clarkGreen);
    //     canvas.show();
    //     sleep_ms(500);
    //     canvas.fill(clarkGold);
    //     canvas.show();
    //     sleep_ms(500);
    // }

    canvas.clear();

    //
    // Show the pallette
    // for (int i = 0; i < canvas.getHeight(); i++) {
    //     for (int p = 0; p < 16; p++) {
    //         canvas.set(p, i, palette[p]);
    //     }
    // }

    // canvas.show();

    // sleep_ms(5000);

    // for (int i = 0; i < canvas.getHeight(); i++) {
    //     canvas.clearRow(i);
    //     canvas.show();
    //     sleep_ms(200);
    // }

    int nf = canvas.getHeight();
    PaletteFirework **fireworks = new PaletteFirework *[nf];
    for (int i = 0; i < nf; i++) {
        fireworks[i] = new PaletteFirework(&canvas, i, &palette);
        fireworks[i]->setMaxExplosionSteps(45*3);
        if (fireworks[i] == NULL) {
            printf("No firework for %i!\n", i);
        }
    }

    MultiAnimation fani = MultiAnimation(&canvas);
    for (int i = 0; i < nf; i++) {
        fani.add(fireworks[i]);
    }

    Animator animator(&canvas, 45);
    animator.add(&fani);
    animator.init();

    int n = 0;

    while (1) {
        animator.step();
        n++;
        if (n % 500 == 0) {
            printf(
                "%d frames run, %.2f "
                "us/frame, %.2f us/show %d missed frames at %d fps\n",
                n, animator.getAverageFrameTimeUS(),
                animator.getAverageShowTimeUS(), animator.getMissedFrames(), animator.getFPSNeeded());
        }
    }
}