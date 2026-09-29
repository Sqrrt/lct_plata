/*
 * DVDD crowbar voltage glitcher for RP2040-Zero, host: Orange Pi PC (Allwinner H3)
 * Build: arm-linux-gnueabihf-gcc -O2 -o glitch glitch.c
 * Run :  sudo ./glitch <width_ns> <delay_ns> [trigger_pin_invert]
 *
 * Wiring: PA6 -> 100R -> MOSFET gate (10k pulldown to GND)
 *         MOSFET drain -> 2.2R series -> C901 "+" pad (DVDD 1.1V)
 *         MOSFET source -> target GND
 *         PA1 <- encoder button (GPIO28 side, active low)
 *
 * Timing: H3 generic timer CNTVCT = 24 MHz -> 41.67 ns per tick.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define PIO_BASE  0x01C20800UL
#define MAP_LEN   0x1000
#define TICKS_NS  41.67

static volatile uint32_t *pio;

static inline uint32_t tmr(void)
{
    uint32_t t;
    asm volatile("mrs %0, cntvct" : "=r"(t));
    return t;
}

static inline void nsleep(uint32_t ns)
{
    uint32_t t0 = tmr();
    uint32_t need = (uint32_t)(ns / TICKS_NS);
    while ((tmr() - t0) < need)
        ;
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "usage: %s <width_ns> <delay_ns>\n", argv[0]);
        return 1;
    }
    uint32_t width = (uint32_t)atoi(argv[1]);   /* pulse width   */
    uint32_t delay = (uint32_t)atoi(argv[2]);   /* delay after trigger */

    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) { perror("/dev/mem"); return 1; }
    pio = mmap(NULL, MAP_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, PIO_BASE);
    if (pio == MAP_FAILED) { perror("mmap"); return 1; }
    close(fd);

    /* PA6 = output (cfg0 bits 27:24 = 0b001) */
    pio[0x00 / 4] = (pio[0x00 / 4] & ~(0xF << 24)) | (0x1 << 24);
    /* PA1 = input (default) */

    printf("armed: width=%uns delay=%uns, waiting for button...\n", width, delay);
    for (;;) {
        while ((pio[0x10 / 4] >> 1) & 1)      /* wait for press (low) */
            ;
        nsleep(delay);                        /* align to check window */
        pio[0x10 / 4] |= (1 << 6);            /* gate on */
        nsleep(width);
        pio[0x10 / 4] &= ~(1 << 6);           /* gate off */
        printf("zap!\n");
        while (!((pio[0x10 / 4] >> 1) & 1))   /* wait for release */
            ;
    }
    return 0;
}
