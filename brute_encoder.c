/*
 * brute_encoder.c — Orange Pi + WiringOP
 * Brute-force / replay encoder input on PT/CYBERSAFE RP2040 badge
 *
 * Encoder:
 *   L1 → GPIO27 (ENC_A)
 *   L2 → GPIO29 (ENC_B)
 *   B1 → GPIO28 (BUTTON)
 *   COMMON/B2 → GND
 *
 * Compile:
 *   gcc brute_encoder.c -o brute_encoder -lwiringPi
 *   (if local: gcc brute_encoder.c -o brute_encoder -L./wiringPi -I./wiringPi -lwiringPi -Wl,-rpath,./wiringPi)
 *
 * Run:
 *   sudo ./brute_encoder          # full brute
 *   sudo ./brute_encoder 9 6 8 2 13   # direct replay of known code
 *
 * Compile on Orange Pi:
 *   cd ~/progs/wiringOP
 *   sudo ./build
 *   cd ..
 *   gcc brute_encoder.c -o brute_encoder -lwiringPi
 */

#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* ---------- GPIO pin numbers (wiringPiSetupPhys → physical BCM) ----------
 * WiringPi uses BCM numbering with wiringPiSetupGpio()
 * Adjust these to match your Orange Pi ↔ target wiring
 */
#define ENC_A_PIN    17   /* attacker GPIO → target GPIO27 (L1) via open-drain */
#define ENC_B_PIN    18   /* attacker GPIO → target GPIO29 (L2) */
#define BTN_PIN      27   /* attacker GPIO → target GPIO28 (B1) */

/* Timing — adjust if badge misses pulses */
#define STEP_DELAY_MS   5    /* between encoder edge pairs */
#define SETTLE_MS       20   /* after last step before button */
#define BTN_PRESS_MS    80   /* button hold time */
#define INTER_DIGIT_MS  200  /* between digits */
#define RESET_WAIT_MS   1000 /* wait after full sequence */

/* Secret code from firmware analysis (0x10005500: 09 06 08 02 0D) */
static const int KNOWN_CODE[5] = {9, 6, 8, 2, 13};

/* Number of positions on the wheel (LEDs 0-13) */
#define POSITIONS 14

/* ------------------------------------------------------------------ */
static void encoder_init(void)
{
    wiringPiSetupGpio();   /* BCM numbering */

    /* Outputs: open-drain simulation — drive LOW to assert, float (INPUT) for HIGH */
    pinMode(ENC_A_PIN, OUTPUT);
    pinMode(ENC_B_PIN, OUTPUT);
    pinMode(BTN_PIN,   OUTPUT);

    /* Idle state: both lines HIGH (encoder not turning, button not pressed) */
    digitalWrite(ENC_A_PIN, HIGH);
    digitalWrite(ENC_B_PIN, HIGH);
    digitalWrite(BTN_PIN,   HIGH);

    pullUpDnControl(ENC_A_PIN, PUD_OFF);
    pullUpDnControl(ENC_B_PIN, PUD_OFF);
    pullUpDnControl(BTN_PIN,   PUD_OFF);
}

/*
 * One quadrature encoder step clockwise:
 *   A↓  →  B↓  →  A↑  →  B↑
 */
static void rotate_step_cw(void)
{
    digitalWrite(ENC_A_PIN, LOW);  delay(STEP_DELAY_MS);
    digitalWrite(ENC_B_PIN, LOW);  delay(STEP_DELAY_MS);
    digitalWrite(ENC_A_PIN, HIGH); delay(STEP_DELAY_MS);
    digitalWrite(ENC_B_PIN, HIGH); delay(STEP_DELAY_MS);
}

/*
 * One quadrature encoder step counter-clockwise:
 *   B↓  →  A↓  →  B↑  →  A↑
 */
static void rotate_step_ccw(void)
{
    digitalWrite(ENC_B_PIN, LOW);  delay(STEP_DELAY_MS);
    digitalWrite(ENC_A_PIN, LOW);  delay(STEP_DELAY_MS);
    digitalWrite(ENC_B_PIN, HIGH); delay(STEP_DELAY_MS);
    digitalWrite(ENC_A_PIN, HIGH); delay(STEP_DELAY_MS);
}

/* Rotate to absolute position `target` from `current` (shortest path) */
static int rotate_to(int current, int target)
{
    int diff = (target - current + POSITIONS) % POSITIONS;
    if (diff == 0) return current;

    /* Go clockwise if diff <= 7, else counter-clockwise */
    if (diff <= POSITIONS / 2) {
        for (int i = 0; i < diff; i++) rotate_step_cw();
    } else {
        int ccw = POSITIONS - diff;
        for (int i = 0; i < ccw; i++) rotate_step_ccw();
    }
    delay(SETTLE_MS);
    return target;
}

static void press_button(void)
{
    digitalWrite(BTN_PIN, LOW);
    delay(BTN_PRESS_MS);
    digitalWrite(BTN_PIN, HIGH);
    delay(INTER_DIGIT_MS);
}

/* Enter one 5-digit code sequence; returns 1 if USB appears (not implemented here) */
static void enter_code(const int code[5])
{
    int pos = 0;  /* assume encoder starts at position 0 */

    for (int i = 0; i < 5; i++) {
        printf("  digit %d: rotate to %d\n", i+1, code[i]);
        pos = rotate_to(pos, code[i]);
        press_button();
    }
    delay(RESET_WAIT_MS);
}

/* ---------- USB success detection ---------------------------------- */
static int usb_detected(void)
{
    /* Check if VID=0xcafe PID=0x4000 appeared */
    FILE *f = popen("grep -rl 'cafe' /sys/bus/usb/devices/*/idVendor 2>/dev/null | head -1", "r");
    if (!f) return 0;
    char buf[256];
    int found = (fgets(buf, sizeof(buf), f) != NULL);
    pclose(f);
    return found;
}

/* ---------- main --------------------------------------------------- */
int main(int argc, char *argv[])
{
    printf("[*] PT/CYBERSAFE badge encoder bruteforcer\n");
    printf("[*] Encoder: A=GPIO%d  B=GPIO%d  BTN=GPIO%d\n",
           ENC_A_PIN, ENC_B_PIN, BTN_PIN);

    encoder_init();

    /* If code given on command line: just replay it */
    if (argc == 6) {
        int code[5];
        for (int i = 0; i < 5; i++) code[i] = atoi(argv[i+1]);
        printf("[*] Replaying code: %d %d %d %d %d\n",
               code[0], code[1], code[2], code[3], code[4]);
        enter_code(code);
        if (usb_detected())
            printf("[+] SUCCESS! USB enumerated.\n");
        else
            printf("[-] No USB after code entry.\n");
        return 0;
    }

    /* ------ Known code first (from firmware analysis) -------------- */
    printf("[*] Trying known code from firmware: 9 6 8 2 13\n");
    enter_code(KNOWN_CODE);
    if (usb_detected()) {
        printf("[+] SUCCESS with known code!\n");
        return 0;
    }

    /* ------ Full brute force: 14^5 = 537614 combos ---------------- */
    printf("[*] Starting full brute force (14^5 = 537614 combinations)...\n");
    long long attempt = 0;

    for (int a = 0; a < POSITIONS; a++)
    for (int b = 0; b < POSITIONS; b++)
    for (int c = 0; c < POSITIONS; c++)
    for (int d = 0; d < POSITIONS; d++)
    for (int e = 0; e < POSITIONS; e++) {
        int code[5] = {a, b, c, d, e};
        attempt++;

        if (attempt % 1000 == 0)
            printf("[*] Attempt %lld: %d %d %d %d %d\n", attempt, a, b, c, d, e);

        enter_code(code);

        if (usb_detected()) {
            printf("[+] SUCCESS! Code: %d %d %d %d %d (attempt %lld)\n",
                   a, b, c, d, e, attempt);
            return 0;
        }
    }

    printf("[-] Brute force complete, no code found.\n");
    return 1;
}
