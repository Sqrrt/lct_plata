# SWD attack via Picoprobe — scheme & usage

Target: challenge board (Waveshare **RP2040-Zero** module, RP2040 QFN-56).
The Zero does **not** break out SWD on its castellated pads (per Waveshare FAQ),
so SWCLK/SWDIO are soldered directly to the RP2040 QFN package pads.

Firmware context (XIP `0x10000000`, Thumb-only):

```
10000C82   loop over 4 code bytes:  r2=[sp+0x30+r4] vs r3=[r8+r4]
10000C8C   bne   #0x10000cd4        ; wrong byte -> FAIL path
10000C9A   movs  r2, #0             ; SUCCESS path (fall-through when all 4 match)
10000CD4   movs  r3, #0xd0          ; FAIL path: LED "wrong code" blink x3, reset state
```

Success path also stores `1` to `0x20002ed2` (the LED/USB unlock gate byte).

## 1. Picoprobe firmware (donor Pico)

Any RP2040 board with BOOTSEL works as the probe.

```bash
git clone https://github.com/raspberrypi/debugprobe
cd debugprobe
git submodule update --init --recursive
mkdir build && cd build
cmake -DDEBUG_ON_PICO=ON -DPICO_SDK_PATH=$PICO_SDK_PATH ..
make -j"$(nproc)"
# -> build/debugprobe_on_pico.uf2   (or grab a release .uf2 from GitHub releases)
```

Upload: hold **BOOTSEL** on the donor Pico, plug USB, drag `debugprobe_on_pico.uf2`
onto the `RPI-RP2` drive. After reboot it enumerates as
`2e8a:000c Raspberry Pi Debug Probe (CMSIS-DAP)`.

## 2. Wiring (jump wires)

```
 PROBE (donor Pico, debugprobe firmware)        TARGET (challenge board, RP2040 QFN-56)
 ──────────────────────────────────────────     ────────────────────────────────────────
 GND   Pico pin 3   ────────────────────────►   GND (any ground pad)
 GP2   Pico pin 4   (SWCLK) ───────────────►   SWCLK — QFN pad 25
 GP3   Pico pin 5   (SWDIO) ───────────────►   SWDIO — QFN pad 26
 GP4   Pico pin 6   (UART TX) ─── optional ►   GPIO0  — target UART0 RX (Zero pad GP0)
 GP5   Pico pin 7   (UART RX) ─── optional ►   GPIO1  — target UART0 TX (Zero pad GP1)
 GP6   Pico pin 9   (/RESET)  ─── optional ►   RUN    — QFN pad 24
 3V3   Pico pin 36  ─── only to power a bare target (do NOT use if board is USB-powered)
```

QFN-56 pad numbering: find the pin-1 dot (corner marker), count **counter-clockwise**,
14 pads per side. East side top→bottom = pads 15-28, so:
`TESTEN`=19, `RUN`=24, `SWCLK`=25, `SWDIO`=26 (11th/12th pad from top on the east edge).
Cross-check against `materials/images/qfn.png` and the die photos in
`attack/testen/decap/`. Use short wires; enameled wire on the QFN pads is fine.

`adapter speed 4000` is safe for short jumpers; drop to 1000 for long flimsy wires.

## 3. Udev (if openocd can't see the probe)

NixOS: `services.udev.extraRules`:

```
ATTRS{idVendor}=="2e8a", MODE="0660", GROUP="users"
```

## 4. Smoke test

```bash
openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 4000"
```

Expected: probe detected, target Halted via `halt` over telnet (port 4444).

## 5. Attack

```bash
./script.sh
```

The script loops until SWD attaches, arms a hardware breakpoint at `0x10000CD4`
(fail path) and waits. **Interact with the board (encoder + button, i.e. submit a
code).** The moment the firmware takes the "wrong code" branch, the script rewrites
`PC = 0x10000C9A` (success path), resumes, and the unlock flag `0x20002ed2` gets set.

Variables at the top of `script.sh`:

- `BP_FAIL=0x10000cd4` — breakpoint (fail-path entry).
  Alternative: `0x10000c8c` (the `bne` itself) forces success on the first digit
  compare, no wrong-code attempt needed.
- `PC_WIN=0x10000c9a` — success path to jump to.
- `GDB`/`OPENOCD`/`GDB_PORT`/`ATTACK_TIMEOUT` — env-overridable.
