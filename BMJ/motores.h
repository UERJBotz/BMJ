#ifndef BMJ_MOTORES_H
#define BMJ_MOTORES_H
#include <Arduino.h>
#include <inttypes.h>

#define PWM_MAX 1023

#include "placa.h"
#define roda_esq_m1 MB2
#define roda_esq_m2 MB1
#define roda_dir_m1 MA2
#define roda_dir_m2 MA1

void motor(uint8_t m1, uint8_t m2, int16_t vel);
void mover(int16_t esq, int16_t dir);

void setupMotores(void) {
    pinMode(roda_esq_m1, OUTPUT);
    pinMode(roda_esq_m2, OUTPUT);
    pinMode(roda_dir_m1, OUTPUT);
    pinMode(roda_dir_m2, OUTPUT);

    mover(0,0);
}

int16_t prev_esq = 0;
int16_t prev_dir = 0;
void mover(int16_t esq, int16_t dir) { //! checar depois se isso é necessário
    esq = constrain(esq, -PWM_MAX, PWM_MAX);
    if (prev_esq != esq) motor(roda_esq_m1, roda_esq_m2, esq);
    prev_esq = esq;

    dir = constrain(dir, -PWM_MAX, PWM_MAX);
    if (prev_dir != dir) motor(roda_dir_m1, roda_dir_m2, dir);
    prev_dir = dir;
}

void parar() {
#if 0 // parece que habilitar isso faz girar pra sempre [no 3]!
    digitalWrite(roda_esq_m1, HIGH);
    digitalWrite(roda_esq_m2, HIGH);
    digitalWrite(roda_dir_m1, HIGH);
    digitalWrite(roda_dir_m2, HIGH);
#else
    mover(0,0);
#endif
}

void motor(uint8_t m1, uint8_t m2, int16_t vel) {
    vel = constrain(vel, -PWM_MAX, PWM_MAX);
    if (vel < 0) {
        analogWrite(m2, 0);
        analogWrite(m1, -vel);
    } else {
        analogWrite(m1, 0);
        analogWrite(m2, vel);
    }
}

#endif
