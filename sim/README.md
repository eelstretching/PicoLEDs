# PicoLEDs desktop simulator

Run PicoLEDs programs on your Mac (or Linux box) and watch them in a window,
or record them to an animated GIF, before you put them on a Pico. The
programs are compiled as they are, from the same library sources the Pico
build uses; only the Pico SDK underneath is swapped out.

## On a Mac

You need the Xcode command line tools, CMake, and SDL2 for the window:

```sh
xcode-select --install        # if you haven't already
brew install cmake sdl2
```

Then, from the top of your PicoLEDs checkout:

```sh
cmake -S sim -B build-sim
cmake --build build-sim -j
./build-sim/FireDemo
```

The first `cmake` line should say `SDL2 found, programs will open a window`.
If it says there's no SDL2, `brew install sdl2` and delete `build-sim` before
running it again.

In the window:

| Key | What it does |
| --- | --- |
| **→** | Stop and single step. The first press stops the animation; each press after that shows the next frame. |
| **space** | Pause, or go back to running after pausing or stepping. |
| **v** | Switch views: flat, tree, radial, front (see below). **1** to **4** pick one directly. |
| **q** or **Esc** | Quit. |

The program's clock stops while it's paused or stepping, so the animation
picks up where it left off rather than trying to catch up. While stepping, the
window title shows the frame number.

To record a GIF instead:

```sh
./build-sim/Fireworks --gif fireworks.gif --seconds 15
```

Recording doesn't run in real time. The program gets a pretend clock that
only moves when it sleeps or sends a frame, so a GIF comes out the same every
time and a minute of animation takes a few seconds to record.

## Options

| Option | What it does |
| --- | --- |
| `--gif FILE` | Record to an animated GIF instead of opening a window. |
| `--seconds N` | How much of the animation to record (default 10). |
| `--scale N` | Screen pixels per LED (default: as big as fits). |
| `--view V` | Start in a view: `flat` (the default), `tree`, `radial` or `front`. GIFs are recorded in this view. |
| `--arc N` | How far around the MegaTree its strands go, in degrees, for the radial and front views (default 180, the half facing the street). |
| `--seed N` | Seed for `get_rand_32()`, so random animations repeat (default 1). |
| `--wrap N` | For programs that use strips without a `Canvas`, fold each strip into rows of N pixels (default 100 for strips over 150). |
| `--no-gamma` | Show raw color values. See below. |

## Your own programs

Every `.cpp` file in `sim/programs/` becomes a program named after the file.
`Playground.cpp` is there to copy. Write it exactly as you would for the Pico,
with a `main()` that loops forever; re-run `cmake --build build-sim -j` and
run `./build-sim/YourFile`.

The examples in `examples/` that make sense without hardware are built too:
FireDemo, Fireworks, MegaTree, MicroTree, CanvasTest, AniTest, PatternTest,
Fader, Fonts, PanelTest, SimpleLEDs, and StripTest (and Fireworks2D once that's
on the branch).

Another project can simulate its programs by pulling this directory in. For
MegaTree, next to this checkout, something like:

```cmake
cmake_minimum_required(VERSION 3.13)
project(MegaTreeSim LANGUAGES C CXX)
add_subdirectory(../PicoLEDs/sim picoleds-sim)
file(GLOB ARCHES arches/src/*.cpp)
picoleds_sim_program(LeapingArches LeapingArches.cpp ${ARCHES})
target_include_directories(LeapingArches PRIVATE arches/include)
```

## What you're looking at

There are four views. The last two treat each row of the canvas as a MegaTree
strand, with x = 0 at the bottom of the tree, and spread the strands evenly
over `--arc` degrees (180 by default, the half facing the street), with row 0
at the left end as you look from the street.

* **flat** shows the canvas the way the animation sees it: x to the right, y
  up, and (0, 0) at the bottom left.
* **tree** turns it the way a MegaTree hangs: x goes up the strands from the
  ground, and y goes across, around the tree. (0, 0) is still at the bottom
  left.
* **radial** is a MegaTree from above, with the street at the bottom of the
  window. The bottom of each strand is at the outside and the top of the tree
  is in the middle.
* **front** is a MegaTree from the end of the driveway: a cone seen side on,
  each strand running from its spot on the base up to the top, with the ones
  toward the sides foreshortened. If `--arc` is over 180, the strands on the
  back are drawn dimmer, behind the ones in front.

Also:

* Without a `Canvas` (a bare `Renderer` and `Strip`s), each strip is drawn
  as rows, first strip at the top.
* LEDs that are off are drawn faint gray, so you can see where they are.
* Colors are shown before the brightness scaling, since a screen pixel at
  32/255 looks much dimmer than an LED at 32/255. They're also gamma
  corrected: an LED's light goes up in a straight line with the value you
  send, but a screen's doesn't, so without the correction dim colors look far
  darker on the screen than on the tree. `--no-gamma` turns that off.
* Timing is the real thing. `sleep_ms()` sleeps, and each frame takes as long
  to "send" as it would on the strips (30 us per pixel of the longest strip,
  plus the reset time), so the Animator's frame rate and dithering refreshes
  behave as they do on the Pico.

## How it works

* `stubs/` has stand-ins for the Pico SDK headers PicoLEDs uses
  (`pico/stdlib.h`, `hardware/pio.h`, and friends). GPIO calls do nothing,
  `time_us_64()` and `sleep_us()` come from the simulator, and
  `get_rand_32()` is seeded.
* `SimRenderer.cpp` replaces `src/Renderer.cpp`. Instead of PIO and DMA,
  `render()` copies the frame out and hands it to the simulator.
  `Canvas::show()` tells the simulator which canvas it's showing (the only
  change to the library, behind `#ifdef PICOLEDS_SIM`).
* `Sim.cpp` has the real `main()`. The program's own `main()` is renamed to
  `picoleds_user_main()` by the build and run on its own thread, since macOS
  wants the window on the main thread.
* `GifWriter.cpp` writes GIFs without needing any libraries, and
  `SimWindowSDL.cpp` is the window.

Things that need real hardware, like WiFi and the RTC, aren't simulated,
which is why CampSign isn't built. The status LED calls just do nothing.
