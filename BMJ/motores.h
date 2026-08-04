#include <Arduino.h>
#include <inttypes.h>

#define PWM_MAX 1023

#include "placa.h"
#define roda_esq_m1 MA2
#define roda_esq_m2 MA1
#define roda_dir_m1 MB2
#define roda_dir_m2 MB1

void motor(uint8_t m1, uint8_t m2, int16_t vel);

void setupMotores(void) {
    pinMode(roda_esq_m1, OUTPUT);
    pinMode(roda_esq_m2, OUTPUT);
    pinMode(roda_dir_m1, OUTPUT);
    pinMode(roda_dir_m2, OUTPUT);
}

void mover(int16_t esq, int16_t dir) {
    motor(roda_esq_m1, roda_esq_m2, - esq);
    motor(roda_dir_m1, roda_dir_m2, - dir);
}

void parar() {
#if 1
    analogWrite(roda_esq_m1, PWM_MAX);
    analogWrite(roda_esq_m2, PWM_MAX);
    analogWrite(roda_dir_m1, PWM_MAX);
    analogWrite(roda_dir_m2, PWM_MAX);
#else
    mover(0,0);
#endif
}

void motor(uint8_t m1, uint8_t m2, int16_t vel) {
    if (vel < 0) {
        analogWrite(m1, abs(vel));
        analogWrite(m2, 0);
    } else {
        analogWrite(m1, 0);
        analogWrite(m2, vel);
    }
}
