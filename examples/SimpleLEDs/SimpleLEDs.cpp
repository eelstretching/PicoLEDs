#include <stdlib.h>

#include "Canvas.h"
#include "Font.h"
#include "FontTwoP.h"
#include "Strip.h"
#include "View.h"
#include "colorutils.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "pico/printf.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/types.h"

int main() {
    stdio_init_all();

    //
    // A canvas and a view made out of strips.
    Canvas canvas(138, 16);
    View view(138);
    Strip strips[] = {Strip(2, 552), Strip(3, 552), Strip(4, 552),
                      Strip(5, 552)};
    view.add(strips[0]);
    view.add(strips[1]);
    view.add(strips[2]);
    view.add(strips[3]);
    canvas.setView(&view, 0, 0);
    int delay = 30;

    printf("View is %d rows %d columns\n", view.getHeight(), view.getWidth());

    //
    // Init to clear the strips and show they're working without relying on
    // rendering.
    for (int i = 0; i < 4; i++) {
        strips[i].fill(RGB::Green);
        strips[i].show();
    }
    for (int i = 0; i < 4; i++) {
        sleep_ms(delay);
        strips[i].fill(RGB::Black, 0, 138);
        strips[i].show();

        sleep_ms(delay);
        strips[i].fill(RGB::Black, 138, 276);
        strips[i].show();

        sleep_ms(delay);
        strips[i].fill(RGB::Black, 276, 414);
        strips[i].show();

        sleep_ms(delay);
        strips[i].fill(RGB::Black, 414, 552);
        strips[i].show();
    }

    while (1) {
        canvas.clear();

        canvas.drawLine(0, 7, 40, 7, RGB::Red);

        canvas.drawLine(0, 5, 40, 5, RGB::Green);

        canvas.show();

        sleep_ms(500);

        canvas.mirrorTopToBottom();
        canvas.show();

        sleep_ms(500);

        canvas.mirrorLeftToRight();
        canvas.show();

        sleep_ms(500);

        canvas.clear();

        canvas.drawLine(97, 0, 137, 0, RGB::Blue);
        canvas.drawLine(97, 2, 137, 2, RGB::Fuchsia);
        canvas.show();
        sleep_ms(500);
        canvas.mirrorBottomToTop();
        canvas.show();
        sleep_ms(500);
        canvas.mirrorRightToLeft();
        canvas.show();
        sleep_ms(500);
    }

    // //
    // // Make a gradient that's 16 wide.
    // RGB grad1[16];
    // fill_gradient_RGB(grad1, 0, RGB::Blue, 16, RGB::Green);
    // RGB grad2[16];
    // fill_gradient_RGB(grad2, 0, RGB::Yellow, 16, RGB::Red);

    // RGB simple[] = {RGB::Red,  RGB::Orange, RGB::Yellow, RGB::Green,
    //                 RGB::Blue, RGB::Indigo, RGB::Violet, RGB::White};
    // char b[30];

    // Font robo(&canvas, RobotronFontData);

    // uint tw = robo.render("TROOP 103", 5, 0, RGB::Green);
    // canvas.show();
    // sleep_ms(10000);
    // robo.render("BTON", tw+robo.getSpacing(), 0, RGB::Red);
    // canvas.show();
    // sleep_ms(10000);

    // //
    // // Make a 4x16 block of the gradients
    // int n = 0;

    // canvas.clear();
    // canvas.copy(grad1, 16, 40, 0);
    // canvas.copy(grad2, 16, 40, 1);
    // canvas.copy(grad1, 16, 40, 2);
    // canvas.copy(grad2, 16, 40, 3);

    // canvas.show();
    // sleep_ms(200);
    // int rx = 40;
    // int ry = 0;
    // bool dir = true;
    // while (1) {
    //     printf("Bounce %d %d\n", rx, ry);
    //     for (int i = 0; i < 40; i++) {
    //         canvas.shiftRight(rx++, ry, 16, 4, 1);
    //         canvas.show();
    //         sleep_ms(delay);
    //     }

    //     if (dir) {
    //         canvas.shiftUp(rx, ry, 16, 4, 1);
    //         canvas.show();
    //         sleep_ms(delay);
    //         ry++;
    //         if (ry == canvas.getHeight() - 4) {
    //             dir = false;
    //         }
    //     } else {
    //         canvas.shiftDown(rx, ry, 16, 4, 1);
    //         canvas.show();
    //         sleep_ms(delay);
    //         ry--;
    //         if (ry == 0) {
    //             dir = true;
    //         }
    //     }

    //     for (int i = 0; i < 40; i++) {
    //         canvas.shiftLeft(rx--, ry, 16, 4, 1);
    //         canvas.show();
    //         sleep_ms(delay);
    //     }

    //     if (dir) {
    //         canvas.shiftUp(rx, ry, 16, 4, 1);
    //         canvas.show();
    //         sleep_ms(delay);
    //         ry++;
    //         if (ry == canvas.getHeight() - 4) {
    //             dir = false;
    //         }
    //     } else {
    //         canvas.shiftDown(rx, ry, 16, 4, 1);
    //         canvas.show();
    //         sleep_ms(delay);
    //         ry--;
    //         if (ry == 0) {
    //             dir = true;
    //         }
    //     }

    //     n++;
    //     if (n % 20 == 0) {
    //         StopWatch *stats = canvas.getStopWatch();
    //         printf("Average show time is %.2f us\n",
    //         stats->getAverageTime());
    //     }
    // }
}
