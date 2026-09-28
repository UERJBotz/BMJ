#ifndef BMJ_MOTORES_H
#define BMJ_MOTORES_H
#include <Arduino.h>
#include <inttypes.h>

#define PWM_MAX 1023

#include "placa.h"
#define roda_esq_m1 MB1
#define roda_esq_m2 MB2
#define roda_dir_m1 MA1
#define roda_dir_m2 MA2

void motor(uint8_t m1, uint8_t m2, int16_t vel);

bool pwm_motores_configurado = false;

void configurarPWMMotores();

void setupMotores(void) {
    pinMode(roda_esq_m1, OUTPUT);
    pinMode(roda_esq_m2, OUTPUT);
    pinMode(roda_dir_m1, OUTPUT);
    pinMode(roda_dir_m2, OUTPUT);
    // Nao chamar analogWrite/analogWriteResolution antes de testar o IR.
    digitalWrite(roda_esq_m1, LOW);
    digitalWrite(roda_esq_m2, LOW);
    digitalWrite(roda_dir_m1, LOW);
    digitalWrite(roda_dir_m2, LOW);
}

void configurarPWMMotores() {
    if (pwm_motores_configurado) return;
    const uint8_t pinos[] = {roda_esq_m1, roda_esq_m2, roda_dir_m1, roda_dir_m2};
    for (uint8_t pino : pinos) {
        analogWrite(pino, 0);
        analogWriteResolution(pino, 10);
    }
    pwm_motores_configurado = true;
}

void mover(int16_t esq, int16_t dir) {
    esq = constrain(esq, -PWM_MAX, PWM_MAX);
    dir = constrain(dir, -PWM_MAX, PWM_MAX);
    motor(roda_esq_m1, roda_esq_m2, - esq);
    motor(roda_dir_m1, roda_dir_m2, - dir);
}

void parar() {
    if (!pwm_motores_configurado) {
        // Mesma intencao de freio HIGH/HIGH do original, sem inicializar LEDC.
        digitalWrite(roda_esq_m1, HIGH);
        digitalWrite(roda_esq_m2, HIGH);
        digitalWrite(roda_dir_m1, HIGH);
        digitalWrite(roda_dir_m2, HIGH);
        return;
    }
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
    configurarPWMMotores();
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
