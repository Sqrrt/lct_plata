#!/usr/bin/env bash
while true; do
  echo "[-] Starting openocd..."
  sudo ./src/openocd -s ./tcl -f interface/cmsis-dap.cfg -f target/rp2040.cfg \
    -c "adapter speed 100" -c "set USE_CORE 0" \
    -c "gdb_port 3333" -c "tcl_port disabled" -c "telnet_port 4444" \
    >"/tmp/openocd-$UID.log" 2>&1 &
  UP=0
  for _ in $(seq 1 40); do
    (exec 3<>/dev/tcp/127.0.0.1/3333) 2>/dev/null && { exec 3>&- 3<&-; UP=1; break; }
    kill -0 %1 2>/dev/null || break
    sleep 0.25
  done
  if [ "$UP" = 0 ]; then
    echo "[$(date +%T)] [-] openocd is not up. Log tail:"
    tail -5 "/tmp/openocd-$UID.log"
    kill %1 2>/dev/null; wait 2>/dev/null
    sleep 1
    continue
  fi
  echo "[+] openocd up, bp @ 0x10000cd4"
  echo "[+] Enter a code on the board..."
  if arm-none-eabi-gdb -batch \
      -ex "target extended-remote :3333" \
      -ex "monitor halt" \
      -ex "hbreak *0x10000cd4" \
      -ex "continue" \
      -ex "set \$pc=0x10000c9a" \
      -ex "detach" 2>&1 | tee "/tmp/gdb-$UID.log" | grep -q "0x10000cd4"; then
    echo "[$(date +%T)] [+] Mischief managed!"
    break
  fi
  echo "[-] Attack failed, retrying..."
  kill %1 2>/dev/null; wait 2>/dev/null
done
