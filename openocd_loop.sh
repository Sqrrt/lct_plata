#!/bin/bash
# openocd_loop.sh — reconnect loop for flaky needle contact
# Uses debugprobe (Pico with debugprobe firmware) to connect to RP2040-Zero
# Retries until connection succeeds, then drops to interactive OpenOCD shell

IFACE="interface/cmsis-dap.cfg"
TARGET="target/rp2040.cfg"
SPEED=100   # очень медленно для иголок, можно поднять до 1000 если контакт хороший

echo "[*] OpenOCD reconnect loop (Ctrl+C to stop)"
echo "[*] Speed: ${SPEED} kHz (низкая для надёжности иголок)"
echo ""

attempt=0
while true; do
    attempt=$((attempt + 1))
    echo -n "[*] Attempt $attempt... "

    output=$(sudo openocd \
        -f "$IFACE" \
        -f "$TARGET" \
        -c "adapter speed $SPEED" \
        -c "init" \
        -c "reset halt" \
        -c "targets" \
        -c "exit" 2>&1)

    if echo "$output" | grep -q "halted\|Cortex-M0\|rp2040\|RP2040"; then
        echo "CONNECTED!"
        echo "$output"
        echo ""
        echo "[+] Dropping to interactive OpenOCD..."
        sudo openocd \
            -f "$IFACE" \
            -f "$TARGET" \
            -c "adapter speed $SPEED" \
            -c "init" \
            -c "reset halt"
        break
    elif echo "$output" | grep -q "Error\|error\|failed\|LIBUSB"; then
        echo "FAILED"
        echo "  → $(echo "$output" | grep -i 'error\|failed' | head -2)"
    else
        echo "no target"
    fi

    sleep 1
done
