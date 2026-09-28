#ifndef Sensores_H
#define Sensores_H
#include <Arduino.h>

//BMJ
// ultima atualização 16/05/2026
#define FEsq 33
#define Frente 35
#define FDir 32
// #define linhaEsq XX
// #define linhaDir XX


uint8_t sensores[] = {FEsq, Frente, FDir};

void setupSensores() {
  pinMode(FEsq, INPUT);
  pinMode(Frente, INPUT);
  pinMode(FDir, INPUT);
}

#endif
