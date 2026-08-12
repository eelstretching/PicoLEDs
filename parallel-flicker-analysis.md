# Parallel-rendering flicker: code analysis

Analysis of `src/Renderer.cpp`, `src/ws2812.pio`, `src/ws2811.pio` against the reported
symptom: with 16 strips on pins 2–17 (two parallel PIO programs) panels 15 and 16 flicker,
but the same panels on pins 10–17 as a single 8-pin parallel program work perfectly.

---

## 1. The thing that rules out most software explanations

For the two experiments, **the bytes going out to pins 10–17 are bit-for-bit identical**.

Tracing the run-splitting loop in `Renderer::setup()` (`src/Renderer.cpp:121-149`) with 16
strips on pins 2–17 and `NUM_PARALLEL_PINS 8`:

- group 0 → `startIndex=0, startPin=2, size=8`
- group 1 → `startIndex=8, startPin=10, size=8`

In the bit-planer, `stripBit = 1 << (i - pip->startIndex)` (`src/Renderer.cpp:354`) is
relative to the run, so pin 10 → bit 0 … pin 17 → bit 7. And
`sm_config_set_out_pins(base=10, count=8)` maps bit 0 → GPIO 10 … bit 7 → GPIO 17.

That is exactly the mapping group 0 gets in the 8-strip test. Same buffer contents, same PIO
program, same clkdiv, same 1.25 µs bit period. Each group also gets its own state machine,
its own DMA channel, and its own buffer.

**There is no per-pin, per-group asymmetry in the data.** Whatever changes for panels 15/16
changes outside the byte stream.

Two things that could have silently broken that symmetry were checked and neither applies:

- **RP2350 `GPIOBASE` translation.** `setup()` calls
  `pio_claim_free_sm_and_add_program_for_gpio_range(..., set_gpio_base=true)`, and PINCTRL
  pin bases on RP2350 are *relative* to the PIO's GPIO base. But the SDK handles the
  translation itself — `pio_get_default_sm_config()` sets `c.pinhi = -1` and
  `pio_sm_set_config` XORs bit 4 of the base fields when `gpio_base` is 16. So passing
  absolute pin numbers to `sm_config_set_out_pins` is correct.
- **DMA starvation.** Each channel needs one byte per 1.25 µs. Even with four groups that is
  3.2 M transfers/s against a 150 MHz bus — roughly 2% of capacity. The PIO can't be
  starved here, and PIO output is independent of CPU/IRQ activity anyway.

---

## 2. Real bugs found in the code

None of these explain the 8-vs-16 symptom, but they are genuine and worth fixing.

### 2.1 The semaphore is released before the data is on the wire

`reset_delay_complete` is armed at *DMA completion* (`src/Renderer.cpp:59`), but DMA
completion means "last byte is in the TX FIFO", not "last bit is on the pin". With
`PIO_FIFO_JOIN_TX` the TX FIFO is 8 entries deep:

| path | autopull threshold | data per FIFO entry | outstanding at DMA-complete |
|---|---|---|---|
| parallel | 8 bits | 1 bit-time (upper 24 bits of the word are discarded on refill) | 8 × 1.25 µs ≈ **10 µs** |
| serial | 24 bits | 1 pixel | 8 × 30 µs = **240 µs** |

So on the parallel path the effective latch gap is 80 − 10 = 70 µs (fine, above the 50 µs
spec). On the **serial** path the semaphore is released roughly 160 µs *before* the strip has
finished receiving the frame.

### 2.2 The per-frame state-machine reset can truncate the previous frame

`src/Renderer.cpp:446-450`:

```cpp
pio_sm_set_enabled(pip->pio, pip->sm, false);
pio_sm_clear_fifos(pip->pio, pip->sm);
pio_sm_restart(pip->pio, pip->sm);
pio_sm_exec(pip->pio, pip->sm, pio_encode_jmp(pip->offset));
pio_sm_set_enabled(pip->pio, pip->sm, true);
```

Combined with 2.1, a single-strip renderer running fast enough will `clear_fifos()` away the
last ~5–8 pixels of the previous frame, mid-bit.

Note this predicts corruption in the **serial** case — the opposite of the reported symptom.
And at StripTest's 20 fps there is ~42 ms of slack per frame, so it never fires there. It
will bite OfficeSign at higher frame rates.

### 2.3 `gpio_put(pin, 0)` in the ISR does nothing

`src/Renderer.cpp:64-67`. The pads are muxed to PIO by `pio_gpio_init`, so SIO is not driving
them and `gpio_put` has no effect on the pad. If that loop was added to force the lines idle
low, it never worked.

(The lines do idle low anyway: the last instruction executed is `mov pins, null`, and the SM
then stalls at `out x, 8` waiting on an empty FIFO.)

### 2.4 Buffer overflow if strips in one run have different lengths

`pip->buffSize` comes from `strips[startIndex]` (`src/Renderer.cpp:258`), but the fill loop
runs `s->getNumPixels()` iterations for *every* strip in the run. A longer strip later in a
run writes past the end of its buffer — and straight into the next group's `calloc`'d buffer.

That would look exactly like "the other panel group flickers". Not the current case (all
strips are `STRIP_LEN`), but a landmine for any mixed-size sign.

### 2.5 `add_alarm_in_us` called from inside a DMA ISR

`src/Renderer.cpp:59`. Heavyweight for an ISR, and if it ever fails to schedule, that group's
semaphore is never released and `render()` blocks forever. A deadline check
(`busy_wait_until(dma_start + xfer_us + RESET_TIME_US)`) would be simpler and can't wedge.

### 2.6 `pioPrograms[]` has internal linkage in a header

`include/Renderer.h:60` declares `static PIOProgram* pioPrograms[NUM_DMA_CHANNELS] = {0};`,
so every translation unit that includes the header gets its own private copy. It works only
because the ISR and all its users live in `Renderer.cpp`. Should be `extern` in the header
plus one definition in the .cpp.

### 2.7 Minor

`examples/StripTest/StripTest.cpp:96` — printf has four format specifiers and three
arguments.

---

## 3. What is most likely actually happening

Since the bitstream is provably identical, only two things genuinely differ between the
8-strip and 16-strip configurations, and both are physical:

**Twice the pins switching simultaneously.** `mov pins, !null` drives every pin in a group
high on the same clock edge, and `mov pins, null` drives them all low, 800k times per second.
Going from 8 to 16 pins doubles the peak di/dt through the Pico's supply and ground. And
because the two SMs' fractional clock dividers (div = 18.75 at 150 MHz) run in arbitrary,
drifting relative phase, the aggregate current spike *beats* — which presents as flicker
rather than as steady corruption.

**Twice the LEDs drawing current.** With only 8 strips registered, the other 8 panels receive
no data and stay dark. 4096 LEDs even at brightness 8 is several amps, and the panels at the
far end of the power distribution sag first.

Both mechanisms predict "the last panels in the chain", and neither is visible from the code.
Both are also distinct from the shared-ground issue already ruled out.

---

## 4. Experiments that discriminate, cheapest first

**A. Separate current from signalling.** Register all 16 strips — so 16 pins toggle, 2 SMs,
identical DMA and PIO load — but `fill(black)` on 14 of them and light only panels 15/16.

- Flicker gone → supply sag / LED current.
- Flicker stays → signalling.

**B. Separate SM count from pin count. No code changes needed.** Keep 16 strips and build
once with `NUM_PARALLEL_PINS 8` (2 SMs) and once with `NUM_PARALLEL_PINS 4` (4 SMs). Same
pins, same current, different number of independently-phased state machines.

- Flicker tracks SM count → cross-SM interaction.
- Identical → SM count is irrelevant; it's pin count or current.

**C.** Drop brightness to 1–2 with all 16 panels lit. Vanishes → power.

**D.** Change `GPIO_DRIVE_STRENGTH_4MA` → `GPIO_DRIVE_STRENGTH_2MA` at
`src/Renderer.cpp:294`. Into level-shifter inputs 2 mA is plenty, and it halves the switching
transient.

Also worth adding to `addPIOProgram`: print `pio_get_index(pip->pio)` and `pip->sm` per
group, to confirm both groups land on the same PIO. (The SDK searches PIO instances
downward — `while (pio_num--)` in `pio_claim_free_sm_and_add_program_for_gpio_range` — so on
RP2350 they will likely both be on PIO2.)

---

## 5. The code change worth making regardless

Put the whole contiguous run on **one state machine** with 32-bit bit-planes, the way
pico-examples' `ws2812_parallel` does it:

- PIO: `out x, 32` instead of `out x, 8`
- `sm_config_set_out_shift(&c, true, true, 32)`
- `sm_config_set_out_pins(base, count)` with count up to 32
- `NUM_PARALLEL_PINS 32`
- buffer becomes `uint32_t[numPixels * 24]`, DMA transfer size `DMA_SIZE_32`
- `stripBit = 1u << (i - startIndex)` (already correct, just needs to be 32-bit)

This removes the multi-SM variable entirely for anything up to 32 strips: one state machine,
one DMA channel, every edge on every pin from the same clock, no phase drift between groups,
and a quarter as many DMA transfers. Total memory is unchanged — one 24 KB buffer instead of
four 6 KB ones.

If the flicker survives that, it is electrical with no remaining software confound.

### Cheaper alternative if 8-pin groups stay

Set the parallel autopull threshold to 32 and DMA as `DMA_SIZE_32`. The existing byte buffer
works verbatim under shift-right (byte 0 of a little-endian word shifts out first), and you
get 4× fewer DMA transfers plus a 40 µs FIFO cushion instead of 10 µs.

### Also

Bump `RESET_TIME_US` from 80 to ~300 (`include/Renderer.h:22`). WS2812B-V5 and several
current clones want >280 µs of latch time, and at these frame rates it costs nothing.
