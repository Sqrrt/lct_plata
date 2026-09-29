# DVDD voltage glitch on RP2040-Zero (Orange Pi PC host)

## What is cap9?

Per the official Waveshare RP2040-Zero schematic, the 1.1V core rail (VREG_OUT
→ DVDD pins 23/45/50) is decoupled by C701, C901, C1802, C301, C502, C802,
C1001, C1201, C1302, C1401, C1502, C1901 (+ ferrite L101). The 3.3V rail bulk
cap is C1101. So **C901 ("cap9") = DVDD decoupling** — the classic glitch point.

Verify with a multimeter (board powered):
- cap reads **~1.1V** → DVDD, correct glitch point
- cap reads **~3.3V** → 3V3 decoupling (C1101), do NOT glitch (it feeds VREG_IN
  and the QSPI flash; brownouts there hit flash reads, different attack)
- other end of the cap = GND

## Scheme

```
 Orange Pi PC (Allwinner H3)              RP2040-Zero (target, powered by its USB)
 ───────────────────────────              ────────────────────────────────────────
 PA6 (phys pin 7) ──[100Ω]──┬─ gate  Q1 N-MOSFET logic-level
                            │        (AO3400 / IRLML6344, Vgs(th)<1.5V)
 PA GND (pin 6)  ───────────┴─[10k pulldown]── GND
 PA1 (phys pin 11) ←────── encoder button (GPIO28 terminal)   [trigger]
 GND ─────────────────────── target GND                       [mandatory]

 Q1 drain ──[2.2Ω series R]── C901 "+" terminal (DVDD 1.1V rail)
 Q1 source ── GND
```

- Scope: CH1 on C901 (+) (the wire itself, right at the pad), CH2 on Q1 gate,
  GND clips on target GND. Keep the drain/gate loop **< 5 cm**, twisted with GND.
- Series R is non-negotiable: it limits peak current AND shapes the dip; even if
  the MOSFET dies shorted, 2.2Ω keeps the fault current bounded (chip browns out,
  pull the wire, still alive).

## Desolder

**Nothing.** C901 stays. Only if the dip is too soft (heavy rail decoupling):
desolder 1-2 small decoupling caps next to the DVDD pins, or lift ferrite L101.
Do that only after trying the pulse sweep — and never touch GND/VREG_OUT bulk cap.

## Glitch concept

- Glitch point in firmware: unlock check `cmp r2,r3; bne #fail` @ 0x10000C8C.
  A DVDD brownout during the compare/corrupts the fetched instruction or flags
  → wrong code falls through into the success path (0x10000C9A).
- Button press (GPIO28) = synchronous trigger: Pi waits `delay_ns`, fires pulse
  `width_ns`. Sweep delay 0..200µs in ~200ns steps, width 100ns..5µs.
- Success detector: `lsusb | grep cafe:4000` (USB MSC appears when the unlock
  flag 0x20002ED2 is set) or the UART boot log.
- Build/run on the Pi: see `glitch.c`. `sudo ./glitch <width_ns> <delay_ns>`.
- VREG current-limits itself (≈150 mA max); recovery after dip = C·ΔV/I ≈ µs —
  firmware never notices.

## Safety (what the sim verifies)

`sim.py` models the RC discharge: V0=1.1V, C≈3.4µF (bulk + 12 decoupling caps),
ESR 50mΩ, Ron 50mΩ, VREG 150mA. Safe window = dip 0.15..0.45V:

- Rser=1Ω, W=1µs  → dip 0.26V, Ipk 1.0A, recover 6µs
- Rser=2.2Ω, W=2µs → dip 0.25V, Ipk 0.48A, recover 6µs

Assumptions to double-check in the datasheet: POR/BOD thresholds (~0.6-0.7V,
section 5.2.4). Keep dip above those → no chip reset. Crowbar can only pull the
rail DOWN (no overvoltage possible); the only overshoot source is wire
inductance on turn-off — keep the loop short. Duty cycle: one pulse per button
press, µs wide → junction heating negligible.

Honest caveat: no simulation can *guarantee* survival; this keeps every
parameter inside the datasheet envelope. The real killers are ESD while
soldering (grounded iron) and slip-bridges to neighboring pads (check with
continuity before power).
