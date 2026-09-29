#!/usr/bin/env python3
import re
from pwn import *

FAIL, GATE, WIN, USB = 0x10000CD4, 0x100008AA, 0x10000C9A, 0x100008AA
PC = re.compile(rb"(?i)\b(?:pc|r15)\b[^\r\n]*?(0x[0-9a-fA-F]+)")

io = None
while io is None:
    try:
        io = remote("127.0.0.1", 4444)
    except Exception:
        sleep(1)

io.sendline(b"halt")
sleep(1)
io.sendline(f"bp {FAIL:#x} 2 hw".encode())
io.sendline(f"bp {GATE:#x} 2 hw".encode())
io.sendline(b"resume")
log.info("enter code")

while True:
    io.recvuntil(b"halted due to breakpoint", timeout=600)

    blob = io.recvrepeat(1.0)
    m = PC.search(blob)
    if not m:
        io.sendline(b"reg pc")
        blob += io.recvrepeat(1.0)
        m = PC.search(blob)
    if not m:
        log.warning(f"no pc in {blob!r}")
        io.sendline(b"resume")
        continue

    addr = int(m.group(1), 16) & ~1

    if addr == FAIL:
        io.sendline(f"reg pc {WIN:#x}".encode())
        io.sendline(b"resume")
        log.success("(˶ᵔ ᵕ ᵔ˶)")
    elif addr == GATE:
        io.sendline(f"reg pc {USB:#x}".encode())
        io.sendline(b"resume")
        log.success("^_^")
    else:
        io.sendline(b"resume")
        log.warning(f"halt @ {addr:#x}, resumed")
