#!/usr/bin/env bash
sudo pkill -9 openocd 2>/dev/null
sudo ./src/openocd -s ./tcl -c "set USE_CORE 0" -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 100" -c "gdb memory_map disable" -c "gdb port 3333" -c "tcl port disabled" -c "telnet port 4444" >"/tmp/openocd-$UID.log" 2>&1 &
sleep 3
python3 /home/sneaky/dev/ctf/hardware/pt/attack/swd/attack.py
sudo pkill -9 openocd