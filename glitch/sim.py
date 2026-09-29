#!/usr/bin/env python3
"""Voltage-glitch feasibility sim for RP2040-Zero DVDD (1.1V) crowbar glitch.

Model: C901/DVDD rail discharged through N-MOSFET + series resistor.
Rail: V0=1.1V, bulk C=2.2uF + 12x100nF decoupling ~= 3.4uF, ESR 50mOhm.
VREG (on-chip 1.1V regulator) fights back at ~150mA (ignored => conservative dip).

Checks:
  dip depth < 0.45V      -> stays above POR/BOD (~0.6-0.7V, no chip reset)
  dip depth > 0.15V      -> deep enough to fault logic (DVDD min op ~1.05V)
  Ipk limited by Rser    -> regulator only sees brief current-limited short
  recovery << 1ms        -> firmware state survives, no watchdog hit
"""
import math

V0   = 1.10          # nominal DVDD
C    = 3.4e-6        # total rail capacitance (F)
ESR  = 0.050         # cap ESR
RON  = 0.050         # MOSFET Rds(on)
IREG = 0.150         # VREG max current (A)

print(f"rail: V0={V0}V C={C*1e6:.1f}uF ESR={ESR*1e3:.0f}mOhm RON={RON*1e3:.0f}mOhm")
print(f"{'Rser':>5} {'W':>7} {'dip(V)':>8} {'Vmin(V)':>8} {'Ipk(A)':>7} {'trec(us)':>9}  verdict")
ok = []
for R in (1.0, 2.2, 4.7, 10.0):
    for W in (50e-9, 100e-9, 200e-9, 500e-9, 1e-6, 2e-6):
        Rt  = R + RON + ESR
        dip = V0 * (1 - math.exp(-W / (Rt * C)))
        vmin = V0 - dip
        ipk = V0 / Rt
        trec = dip * C / IREG
        v = "GOOD " if 0.15 < dip < 0.45 and ipk < 2.0 else "     "
        if dip < 0.15:  v = "shallow"
        if dip > 0.45:  v = "POR-risk"
        if ipk > 2.0:   v += " Ipk!"
        if v.startswith("GOOD"):
            ok.append((R, W, dip, vmin, ipk, trec))
        print(f"{R:>5.1f} {W*1e9:>6.0f}ns {dip:>8.3f} {vmin:>8.3f} {ipk:>7.2f} {trec*1e6:>9.1f}  {v}")

print("\nrecommended start points (sweep around these):")
for R, W, dip, vmin, ipk, trec in ok[:6]:
    print(f"  Rser={R:>4.1f} ohm, W={W*1e9:>4.0f}ns -> dip={dip:.2f}V, Ipk={ipk:.2f}A, recover={trec*1e6:.0f}us")
