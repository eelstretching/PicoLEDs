# Parallel-rendering flicker: diagnosis and fix plan

Investigation of the long-standing flicker on parallel-rendered panels, 2026-08-11/12.

**Conclusion: this is not a firmware bug.** It is simultaneous-switching ground bounce in the
SN74HCT245N level shifters. The bits leaving the Pico are provably correct.

---

## 1. How we know it isn't the firmware

### The data path is symmetric

Tracing `Renderer::setup()` for 16 strips on pins 2–17 with `NUM_PARALLEL_PINS 8`:

- group 0 → `startIndex=0, startPin=2, size=8`
- group 1 → `startIndex=8, startPin=10, size=8`

`stripBit = 1 << (i - pip->startIndex)` is relative to the run, so pin 10 → bit 0 … pin 17 → bit 7,
matching `sm_config_set_out_pins(base=10, count=8)`. That is byte-for-byte the same stream group 0
gets when those eight panels are driven as a single group. **The bytes on the wire are identical in
both configurations**, so nothing in the buffer layout can explain why one works and the other
doesn't.

Two things that could have broken that symmetry were checked and don't apply: the SDK handles
RP2350 `GPIOBASE` translation itself (via the `pinhi` field in `pio_sm_set_config`), so passing
absolute pin numbers is correct; and DMA starvation isn't credible at ~2% of bus capacity.

### The observation that settled it

Slow-motion capture of the flash frame showed **one strip desyncing mid-chain while another strip
in the same 8-pin group rendered perfectly**. Both are driven by the same state machine, the same
`mov pins` instruction, the same clock edge, in the same instant. There is no mechanism by which the
emitted bit stream can be wrong for one and right for the other.

So the stream is correct and one panel is misreading a bit off its own data line. A single misread
bit desynchronises everything downstream in that chain, which is why the corruption starts partway
along and runs to the end, and why the corrupted region is dominated by red (`colorMap[0]`) as the
chain re-locks.

### Why it looks data-dependent

The PIO program does this per bit:

```
out x, 8
mov pins, !null   ; all 8 outputs rise together
mov pins, x       ; a DATA-DEPENDENT subset falls
mov pins, null    ; all 8 outputs fall together
```

The middle transition switches however many outputs carry a zero at that instant. When seven fall
and one holds high, ground bounce is near worst-case and the output holding high is the one at
risk. Which outputs those are is a function of the pixel data across all eight strips — so the
worst-case combination recurs at a specific point in the rotate cycle, on whichever channel has the
least margin.

That is the phase-locking, and it is exactly what marginal signalling looks like. It is not evidence
of a logic bug.

### Why the hardware is marginal

`SN74HCT245N` is DIP-20 with **one ground pin (10) and one Vcc pin (20) serving all eight outputs**,
on perfboard with no ground plane, no local decoupling, and no series resistors. Eight outputs
charging cable capacitance on the same edge, through a single pin with 5–10 nH of lead inductance,
is a few hundred mA/ns — a few hundred millivolts of bounce on the chip's internal ground reference.

The datasheet rates continuous current through Vcc or GND at ±70 mA. Eight simultaneous
capacitive-charging currents transiently exceed that.

This also explains every earlier observation: fine on one panel, fine on four, degrading as panels
are added, and worst on whichever channels have the least margin.

---

## 2. Free software experiment — do this before any soldering

Set `NUM_PARALLEL_PINS` to **4** in `include/Renderer.h`.

That splits the eight pins across two independently-clocked state machines, so the 245's eight
outputs no longer switch on a single edge — the switching spreads out in time and peak ground
current roughly halves.

**If ground bounce is the mechanism, 4 should be better than 8.** No rewiring, one `#define`.

Also revert `src/Renderer.cpp:294` from `GPIO_DRIVE_STRENGTH_2MA` back to `4MA` — see dead ends
below.

---

## 3. Weekend hardware work

### Parts to order

| Part | Spec | Qty needed | Order |
|---|---|---|---|
| Decoupling cap | 100 nF ceramic X7R, 50 V, through-hole 2.54 mm pitch | 1 per 245 | 10 |
| Bulk cap | 10 µF, 16 V+, electrolytic or ceramic | 1 per 245 | 5 |
| Series resistors | 330 Ω, 1/4 W, through-hole | 8 per 245 | 25 |

With 16 panels at 8 per shifter that's two 245s, so 2 × 100 nF, 2 × 10 µF, 16 × 330 Ω. Ordering
spares is worth it.

### Pinout reference (SN74HCT245N, DIP-20)

| Pin | Function |
|---|---|
| 1 | DIR |
| 2–9 | A1–A8 |
| 10 | GND |
| 11–18 | B8–B1 (note the reversed order) |
| 19 | /OE |
| 20 | Vcc |

With `DIR` high the part runs A→B, so **A (2–9) are inputs and B (11–18) are outputs**. Check which
way yours is wired before fitting the resistors — they go on the *output* side.

### Step 1: decoupling (do this first, on its own)

For each 245:

1. Solder a 100 nF ceramic **directly across pins 10 and 20**, on the underside of the board, legs
   trimmed as short as they will physically go. Pin to pin. Do **not** route it through perfboard
   traces or take it to a distant ground rail — the whole point is minimising the loop area of cap,
   pins, and return path, and a routed cap does nothing.
2. Add a 10 µF bulk cap within about a centimetre of the chip, across the same supply.

Then retest before doing anything else. This is the highest-value change and it may be sufficient
on its own. Knowing whether it was is worth more than fixing everything at once.

### Step 2: series resistors

One 330 Ω in series with each output pin, fitted **at the chip** rather than out at the panel end.

The value is sized for limiting simultaneous switching current, not for impedance matching — 330 Ω
holds each output to roughly 10 mA peak instead of 33 mA. The cost is edge rate, and it's
affordable: into roughly 60 pF of cable plus LED input capacitance, 330 Ω gives about a 44 ns rise
time against a 375 ns pulse.

Fit them on all eight outputs, not just the misbehaving channel. They help as much on the aggressor
side as the victim side.

### Step 3: ground returns

Add more ground wires from the board to the panels, distributed through the data bundle rather than
one shared return for everything. If you're using ribbon, alternating signal and ground is the easy
way to get it.

### Step 4: if still marginal

Split the load across more packages — four channels per 245 halves the per-package switching
current.

### Testing between steps

Run the same StripTest configuration each time and watch for the flash at the same rotation offset.
Printing `frameWatch.count` every frame at a low FPS makes it countable. Changing one thing at a
time is what makes the results mean anything — this investigation lost a cycle to several
variables moving at once.

---

## 4. If you build a PCB

Most of the above carries over, and a ground plane fixes the biggest contributor for free.

**Changes:**

- A solid ground pour on its own layer — not hatched, not a star arrangement. This eliminates the
  high-inductance return paths that cause most of the trouble on perfboard.
- SOIC rather than DIP: roughly a third of the lead inductance for the same part.
- **Do not** move to a faster logic family. ACT/AHCT look like an upgrade but faster edges mean more
  di/dt and worse ground bounce. HCT's slower edges are a feature here.

**Still applies:** decoupling (100 nF right at the Vcc pin with its own via straight into the
plane), series resistors (resistor arrays save space over eight discretes), and grounds distributed
through the cable — the cable run is the part a PCB doesn't fix.

**New for a board:**

- Keep the LED supply current off the signal ground. Several amps of LED return sharing copper with
  logic ground creates common-impedance coupling that no amount of decoupling will fix. Route it
  separately and tie to signal ground at one point near the supply.
- Design the mitigations in as depopulatable — footprints for the series resistors with 0 Ω links
  fitted if they turn out to be unnecessary. A footprint costs nothing at design time; adding one
  after the boards arrive costs a respin.
- Bring a data line and a ground out to a scope header.

---

## 5. Firmware bugs still outstanding

Found during the investigation. None of these cause the flicker, but they are real.

### 5.1 Semaphore released before the data is on the wire

`reset_delay_complete` is armed at DMA completion (`src/Renderer.cpp:65`), but that means "last byte
is in the TX FIFO", not "last bit is on the pin". With `PIO_FIFO_JOIN_TX` the FIFO is 8 entries deep:

| path | autopull threshold | data per entry | outstanding at DMA-complete |
|---|---|---|---|
| parallel | 32 bits | 4 bit-times | ~40 µs |
| serial | 24 bits | 1 pixel | ~240 µs |

At `RESET_TIME_US` of 300 both are currently safe, but the accounting is wrong in principle and the
serial path has no margin if that value is ever reduced.

### 5.2 Buffer overflow if strips in one run have different lengths

`pip->buffSize` comes from `strips[startIndex]` (`src/Renderer.cpp:252`) but the fill loop runs
`s->getNumPixels()` iterations for *every* strip in the run (`src/Renderer.cpp:355`). A longer strip
later in a run writes past the end of its buffer and into the next group's allocation.

Harmless today because all strips are the same length. It will bite the moment a sign is built from
mixed-size panels, and the symptom will look exactly like "the other panel group flickers".

### 5.3 `add_alarm_in_us` from inside a DMA ISR

`src/Renderer.cpp:65`. Heavyweight for an ISR, and if it ever fails to schedule, that group's
semaphore is never released and `render()` blocks forever. A deadline check
(`busy_wait_until(dma_start + xfer_us + RESET_TIME_US)`) would be simpler and can't wedge.

### 5.4 Minor

`examples/StripTest/StripTest.cpp` — the periodic printf has four format specifiers and three
arguments.

### Already fixed this session

- `pioPrograms[]` and `stripBlack` had internal linkage in headers, giving every translation unit
  its own copy. Now `extern` with single definitions.
- The per-frame PIO restart block in `render()` was removed. It was cargo-culted in from another
  project and was actively harmful: `pio_sm_restart` clears the output shift *counter* but
  explicitly not the OSR contents, so the state machine would shift out `threshold/8` stale
  bit-planes at the start of every frame.
- Parallel autopull threshold raised from 8 to 32 with word-sized DMA: a quarter as many DMA
  transfers and a 4× deeper FIFO cushion, with the buffer layout unchanged.
- The dead `gpio_put(pin, 0)` loop in the ISR was removed — the pads are muxed to PIO, so SIO wasn't
  driving them and it never did anything.

---

## 6. Dead ends — don't revisit

**Pico GPIO drive strength.** The Pico only drives the 245's CMOS inputs over a short trace. That
load is trivial and 2 mA versus 8 mA barely changes it. All the current that matters is on the 245's
output side. (The 2 mA setting currently in the tree was a bad suggestion and should go back to
4 mA.)

**Cross-state-machine interference.** The theory that two parallel groups' independently-phased
clock dividers were beating against each other. Ruled out by the frame capture: the victim and a
healthy strip were in the *same* group, on the same state machine.

**Bit-plane ordering and buffer layout.** Verified correct, including under the autopull threshold
change — shifting right, the four `out x, 8` operations consume buffer bytes 0, 1, 2, 3 in that
order, identical to the previous byte-at-a-time DMA.
