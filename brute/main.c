#include <stdio.h>
#include <string.h>
#include <time.h>
#include <wiringPi.h>

#define BUTTON_DELAY 2
#define EDGE_DELAY  8
#define STEP_DELAY 50
#define INTER_DIGIT_DELAY 200
#define INTER_CODE_DELAY 1600

enum {
    BUTTON  = 11,
    LEFT    = 13,
    RIGHT   = 15
};

void set_line(int pin, int level) {
    if (level == LOW) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    } else {
        pinMode(pin, INPUT);
        pullUpDnControl(pin, PUD_OFF);
    }
}

void press_button() {
    set_line(BUTTON, LOW);
    delay(BUTTON_DELAY);
    set_line(BUTTON, HIGH);
}

void rotate_step(int dir) {
    if (dir > 0) {
        set_line(LEFT,  LOW);  delay(EDGE_DELAY);
        set_line(RIGHT, LOW);  delay(EDGE_DELAY);
        set_line(LEFT,  HIGH); delay(EDGE_DELAY);
        set_line(RIGHT, HIGH); delay(EDGE_DELAY);
    } else {
        set_line(RIGHT, LOW);  delay(EDGE_DELAY);
        set_line(LEFT,  LOW);  delay(EDGE_DELAY);
        set_line(RIGHT, HIGH); delay(EDGE_DELAY);
        set_line(LEFT,  HIGH); delay(EDGE_DELAY);
    }
    delay(STEP_DELAY);
}

void enter_digit(int d) {
    if (d > 9 || d < 0)
        return;

    while (d--) {
        rotate_step(1);
    }

    press_button();
}

void enter_code(int code) {
    if (code > 9999 || code < 0)
        return;

    static const int pow10[4] = { 1000, 100, 10, 1 };
    for (int i = 0; i < 4; i++) {
        int d = (code / pow10[i]) % 10;
        enter_digit(d);
        delay(INTER_DIGIT_DELAY);
    }
}

void brute_force() {
    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);

    static const int start_code = 9678;
    static const int max_code = 9999;
    for (int c = start_code; c <= max_code; c++) {
        enter_code(c);
        delay(INTER_CODE_DELAY);

        clock_gettime(CLOCK_MONOTONIC, &now);
        double elapsed = (now.tv_sec - start.tv_sec) +
                          (now.tv_nsec - start.tv_nsec) / 1e9;

        int tried = c - start_code + 1;
        double rate = elapsed > 0 ? tried / elapsed : 0;

        printf("Time %.2f c/s, code: %d / %d\n", rate, c, max_code);
        fflush(stdout);
    }
}

int main() {
    wiringPiSetupPhys();

    set_line(BUTTON, HIGH);
    set_line(LEFT,   HIGH);
    set_line(RIGHT,  HIGH);

    printf("[+] CyberSafe - Bruteforcer\n");

    brute_force();
    return 0;
}
