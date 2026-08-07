#include <stdlib.h>

#include "ArrayColorMap.h"
#include "BluetoothServer.h"
#include "BtReadySignal.h"
#include "Direction.h"
#include "FireworkWipe.h"
#include "Mailbox.h"
#include "ModeController.h"
#include "PacWipe.h"
#include "Panel.h"
#include "PanelCanvas.h"
#include "RainbowWipe.h"
#include "RandomAnimator.h"
#include "ScrollWipe.h"
#include "BDFFont.h"
#include "6x10_font.h"
#include "pico/aon_timer.h"
#include "pico/flash.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"

// TODO: placeholder hardware configuration, copied from PanelTest's pattern.
// Update these once the physical OfficeSign's panel wiring/grid layout is
// finalized. The sign is built from Panels (2D zig-zag-wired sub-strips),
// one per pin, arranged in a PANELS_X x PANELS_Y grid.
#define PANEL_WIDTH 32
#define PANEL_HEIGHT 8
#define PANELS_X 3
#define PANELS_Y 3
#define NUM_PANELS (PANELS_X * PANELS_Y)
#define START_PIN 2

static Mailbox gMailbox;

static void core1Entry() {
    BluetoothServer server(&gMailbox);
    server.run();  // never returns
}

int main() {
    stdio_init_all();
    sleep_ms(1000);
    // Seed the always-on timer with whatever gettimeofday() returns (likely
    // the epoch on a cold boot) so reads are well-defined before the laptop
    // app sends its first SET_TIME message.
    aon_timer_start_with_timeofday();

    // Core 1 brings up BTStack, which persists its link keys/device DB to
    // flash (see NVM_NUM_LINK_KEYS etc. in btstack_config.h) via
    // flash_safe_execute(). That call needs the *other* core (us) to already
    // be registered as a lockout victim, or it asserts deep in pico_flash
    // the first time BTStack touches flash -- register before core 1 ever
    // gets a chance to.
    flash_safe_execute_core_init();

    // Core 1 will bring up cyw43/BTStack, which needs to claim a specific
    // PIO1 state machine. We must not let the LED renderer's "claim any free
    // state machine" logic race that, so core 0 waits here until core 1
    // signals it's done.
    BtReadySignal::claim();
    multicore_launch_core1(core1Entry);
    BtReadySignal::waitForReady();

    // Build the panel grid for the sign. Each Panel is one pin's worth of
    // zig-zag-wired LEDs forming a PANEL_WIDTH x PANEL_HEIGHT block; the
    // PanelCanvas arranges them into one PANELS_X x PANELS_Y addressable
    // surface. See examples/PanelTest/PanelTest.cpp for the reference usage.
    Panel* panels[NUM_PANELS];
    for (int i = 0; i < NUM_PANELS; i++) {
        panels[i] = new Panel(START_PIN + i, PANEL_WIDTH, PANEL_HEIGHT);
    }

    PanelCanvas canvas(PANEL_WIDTH, PANEL_HEIGHT, PANELS_X, PANELS_Y);
    for (int y = 0; y < PANELS_Y; y++) {
        for (int x = 0; x < PANELS_X; x++) {
            canvas.addPanel(panels[y * PANELS_X + x], x, y);
        }
    }
    canvas.setup();
    canvas.setBrightness(8);
    canvas.clear();
    canvas.show();

    BDFFont font(font_6x10);

    font.render(&canvas, "Start up!", 0, 3, RGB::Green);
    canvas.show();
    sleep_ms(2000);

    // Vibe Mode: a reasonable starting playlist built from existing
    // animation building blocks, following the same pattern CampSign uses
    // for its transition wipes. Easy to extend/edit once it's running.
    ArrayColorMap vibeColors({RGB::Red, RGB::Orange, RGB::Yellow, RGB::Green, RGB::Blue,
                               RGB::Indigo, RGB::Violet, RGB::Gold});

    ScrollWipe upWipe(&canvas, &vibeColors, UP);
    ScrollWipe downWipe(&canvas, &vibeColors, DOWN);
    ScrollWipe leftWipe(&canvas, &vibeColors, LEFT);
    ScrollWipe rightWipe(&canvas, &vibeColors, RIGHT);
    upWipe.setExtraFrames(20);
    downWipe.setExtraFrames(20);
    leftWipe.setExtraFrames(20);
    rightWipe.setExtraFrames(20);

    FireworkWipe fireworkWipe(&canvas, &vibeColors);
    PacWipe pacWipe(&canvas, &vibeColors);
    RainbowWipe rainbowWipe(&canvas, &vibeColors);

    RandomAnimator vibeAnimator(&canvas, 30);
    vibeAnimator.add(&upWipe);
    vibeAnimator.add(&downWipe);
    vibeAnimator.add(&leftWipe);
    vibeAnimator.add(&rightWipe);
    vibeAnimator.add(&fireworkWipe);
    vibeAnimator.add(&pacWipe);
    vibeAnimator.add(&rainbowWipe);

    // ModeController boots into SignMode::Dark itself (see ModeController.h)
    // and clears the canvas as part of construction, so the sign starts dark
    // until the laptop app sends a mode-switching message.
    ModeController controller(&canvas, &font, &vibeAnimator, &gMailbox);

    while (true) {
        SignMessage msg;
        if (gMailbox.take(msg)) {
            controller.handleMessage(msg);
        }
        controller.step();
    }
}
