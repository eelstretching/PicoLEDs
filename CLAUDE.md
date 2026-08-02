# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this repo is

PicoLEDs is a C++ animation framework for driving WS2811/WS2812 LED strips (and multi-strip panels)
from a Raspberry Pi Pico / Pico 2 W, built on the Pico SDK. `src/` and `include/` are the reusable
library (`PicoLEDs`); `examples/` contains standalone firmware projects (one `add_executable` per
physical sign/install) that link against it. There's also a small Python `server/` (legacy weather/time
data server for older signs) and `laptop_app/` (a FastAPI app for the BLE-controlled OfficeSign example).

## Build (firmware)

This is a Pico SDK / CMake / Ninja project, normally driven through the VSCode "Raspberry Pi Pico"
extension, but buildable from the CLI. Requires `PICO_SDK_PATH` set (the extension installs the SDK
under `~/.pico-sdk/sdk/<version>` and puts its own `cmake`/`ninja`/`picotool`/toolchain on `PATH` via
`.vscode/settings.json`'s `terminal.integrated.env.*` blocks).

```sh
# Configure (from repo root; only needed once / after adding new examples or CMakeLists changes)
cmake -B build -G Ninja

# Build everything
cmake --build build

# Build a single example target (much faster while iterating)
ninja -C build OfficeSign
ninja -C build PanelTest

# Clean one target before a full rebuild (e.g. to surface warnings hidden by incremental build)
ninja -C build -t clean OfficeSign
```

`PICO_BOARD` is fixed to `pico2_w` at the top-level `CMakeLists.txt`. Most examples build straight
into that top-level build; a couple (e.g. `CampSign`) are self-contained sub-builds with their own
`project()`/`pico_sdk_import.cmake` and are commented out of `examples/CMakeLists.txt` because they
predate recent API changes and don't currently compile -- don't assume every directory under
`examples/` builds until you've checked it's `add_subdirectory`'d and actually compiles.

Flashing/running onto real hardware is normally done via the VSCode extension's "Run"/"Flash" tasks
(`picotool load ... -fx` or `openocd`), not from a plain CLI flow -- there's no bench hardware attached
in an agent environment, so firmware changes should be verified by building, not by flashing.

There is no test suite for the firmware. "Verification" means: it compiles cleanly (no warnings) for
`pico2_w`, and ideally has been checked against how the framework is already used elsewhere (see
Architecture below) rather than assumed correct from headers alone.

## Build (laptop_app)

```sh
cd laptop_app
python3 -m venv .venv && .venv/bin/pip install -r requirements.txt
.venv/bin/python -m officesign.main   # serves http://127.0.0.1:8420
```

Requires Python 3.10+ (uses `X | None` union type hints); the system `python3` on macOS may be older
-- check for a newer Homebrew Python (`/opt/homebrew/bin/python3.13`) if venv creation fails oddly.
No automated tests; sanity-check with `python -m py_compile` across `officesign/` and by importing
`officesign.web.app` directly, since a missing route/model error surfaces at import time.

## Architecture

### Rendering pipeline (`include/`, `src/`)

`Strip` → `Canvas` → `Renderer`, roughly:

- **`Strip`**: one physical LED strip (a GPIO pin, pixel count, `WS2811`/`WS2812` type, color order).
  It's fundamentally 1D -- a linear run of pixels addressed by index.
- **`Panel`** (subclass of `Strip`): still one physical strip/pin electrically, but the LEDs are
  physically wired in a zig-zag (the strip snakes down one column, up the next, etc.),
  so `Panel` is a *2D* object -- it adds `set(x, y, color)`/`get(x, y)` on top of `Strip`, translating
  panel coordinates to the right zig-zag index internally (`origin` picks which corner is (0,0)). The
  rendering pipeline underneath (PIO/DMA) doesn't know or care -- a `Panel` still renders exactly like
  any other `Strip`.
- **`Canvas`**: the logical pixel surface animations draw into, built from 1D `Strip`s. Composed of
  `Row`s, each mapping a region of a `Strip` (with a `StripDirection`) into canvas coordinates -- this
  is what lets multiple strips (or multiple rows sharing one long strip) form one rectangular
  addressable surface. Owns a `Renderer` and exposes `clear()`/`show()`/`setBrightness()`.
- **`PanelCanvas`** (subclass of `Canvas`, takes `Panel`s instead of `Strip`s/`Row`s): use this instead
  of plain `Canvas` whenever the sign is built from `Panel`s. Constructed with a per-panel
  width/height and a panel-grid size (`nx` x `ny`), then populated via `addPanel(panel, gridX, gridY)`
  for each panel's position in that grid (see `examples/PanelTest/PanelTest.cpp` for a working 3x2
  grid of 6 panels). It overrides `Canvas`'s coordinate-mapping methods (`set`/`get`/`fill`/
  `rotateRight`/`shiftLeft`/etc.) to route through the right panel's zig-zag indexing -- animations
  written against the `Canvas` interface work unmodified on a `PanelCanvas`, you just build the canvas
  differently.
- **`Renderer`**: turns canvas pixel data into PIO+DMA output. Claims a PIO state machine per contiguous
  run of GPIO pins via `pio_claim_free_sm_and_add_program_for_gpio_range` (dynamic "any free SM" claim
  -- see the cyw43/BLE caveat below). For parallel multi-strip output it uses a bit-planing technique
  (see `parallel.txt`): pixel data for N strips is transposed so each output word carries one bit from
  every strip, packed MSB-first per color channel, and shifted out via the `ws2812_parallel`/
  `ws2811_parallel` PIO programs. Single-strip runs use the plain serial `ws2812`/`ws2811` programs
  instead. Both variants live side by side in `src/ws2812.pio`/`src/ws2811.pio` and are code-generated
  into `src/generated/` at configure time via `pico_generate_pio_header`.
- **`Animation`** (base class) / **`Animator`** (drives a list of `Animation`s in sequence, pacing frames
  to a target FPS and calling `canvas->show()` each frame) / **`RandomAnimator`** (same, but shuffles
  animation order each pass through the list -- the usual way to build a "random attract mode" playlist,
  see `RandomAnimator.h`). `MultiAnimation` runs several animations concurrently instead of in sequence.
  Concrete animations (`Fireworks`, `*Wipe`, `Bouncer`, `Marquees`, `RainbowWipe`, etc.) live in
  `include/`/`src/` and take a `Canvas*` + usually a `ColorMap*` in their constructor.
- **`ColorMap`** (`ArrayColorMap`, `FadeColorMap`, ...): indexable palettes animations pull colors from.

Check an *actively-built* example (see below) for current constructor signatures before writing new
code against these classes -- several classes' APIs have changed recently (see Fonts note) and stale
examples that are no longer in the build (like `CampSign`) do not reflect the current API.

### Fonts -- two parallel systems, don't mix them up

- **`SimpleFont`** (implements the `Font` interface): the original/simpler format. Font data is a
  flat byte array like `FontTwoPData` (`include/FontTwoP.h`). Construct with just the data
  (`SimpleFont f(FontTwoPData)`), then render with `f.render(&canvas, text, x, y, color)` -- `Canvas*`
  is an explicit argument to `render()`, not baked into the font object. `examples/Fonts/Fonts.cpp` is
  the current, actively-built reference for this API.
- **`BDFFont`**: newer, generated from real BDF font files (`fonts/*.bdf`) via `fonts/bdf_to_header.py`
  → headers like `include/9x15B_font.h`, sharing metadata layout from `include/font_common.h`/
  `bdf_font_common.h`. Assumes Cartesian coordinates (0,0 at lower-left, y up) and positions text by
  baseline origin (see the comment at the top of `include/BDFFont.h` for the ascent/descent math if you
  need top-left-corner placement instead).

Both implement the abstract `Font` interface (`include/Font.h`), so `Animation`s that take a `Font*`
work with either.

### Dual-core BLE example (`examples/OfficeSign`)

Reference pattern for any future example that needs Bluetooth: LED animation runs on core 0 (the usual
`Canvas`/`Animator` loop); a BTStack BLE GATT server runs entirely on core 1
(`BluetoothServer::run()`, launched via `multicore_launch_core1`). The two cores talk through a
mutex-protected, latest-message-wins `Mailbox`, with the multicore FIFO used only as a "new message"
doorbell (the payload itself is never pushed through the FIFO). Messages are a fixed-size, hand-rolled
little-endian binary protocol (`SignMessage`/`WireFormat`) chosen specifically to avoid struct-padding
mismatches between the firmware's C++ and the laptop app's Python `struct` module -- mirror that
byte-for-byte if you touch the protocol, in both `SignMessage.h` and `laptop_app/officesign/protocol/`.

Two hardware gotchas worth knowing before extending this example:

- **PIO contention at startup**: the cyw43 (WiFi/BT) driver hard-claims a specific PIO1 state machine,
  while `Renderer::setup()` dynamically claims "any free" state machine. If core 0 calls
  `canvas.setup()` before core 1 finishes `cyw43_arch_init()`, the renderer can steal the SM cyw43
  needs. `BtReadySignal` (a dedicated hardware doorbell, separate from the `Mailbox`'s message doorbell)
  makes core 0 wait for core 1 to finish claiming its PIO resources first -- keep that ordering if you
  restructure `main.cpp`.
- **RP2350 has no `hardware_rtc`** (that target/header only exists for RP2040 boards). Time on the
  Pico 2 W goes through `pico/aon_timer.h` instead (`aon_timer_set_time`/`aon_timer_get_time_calendar`,
  using `struct timespec`/`struct tm`, not `datetime_t`). Older examples (`CampSign`'s `TimeAnimation`)
  reference `hardware/rtc.h`/`rtc_get_datetime`/`datetime_t` and will not compile as-is on `pico2_w`.

`laptop_app/officesign/` is the BLE central side: bleak (asyncio) for BLE, FastAPI/uvicorn for the
control web UI (chosen over Flask specifically so route handlers can `await` BLE calls on the same
event loop bleak requires, instead of marshalling across threads), NWS's free `api.weather.gov` API for
weather (resolve the forecast gridpoint dynamically via `/points/{lat},{lon}` rather than hardcoding
one -- gridpoints do go stale), and macOS EventKit (`pyobjc`) for calendar auto-fill.

## Fonts/color data generation

`fonts/bdf_to_header.py` converts a `.bdf` font file into a generated header under `include/`.
`server/bdf2font.py` is an older, separate conversion script tied to the legacy `server/` data format --
don't confuse the two.
