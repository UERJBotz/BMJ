#ifndef Estrategias_H
#define Estrategias_H

#include "Principal.h"

void paraTras() { // estratégia número 5 no controle
  mover(-1023, -1023);
}

void SeekAndDestroy_L() { // estratégia número 6 no controle
  leituraSensores();

  switch (classificarAlvo()) {

    case SEM_ALVO:
      Serial.println("Searching Enemy...");
      mover(723, -723);
      break;

    case ALVO_CENTRO:
      Serial.println("ROBOT ATTACK!");
      mover(1023, 1023);
      break;

    case ALVO_ESQUERDA:
      Serial.println("Left Detected!");
      mover(-723, 723);
      break;

    case ALVO_DIREITA:
      Serial.println("Right Detected!");
      mover(723, -723);
      break;
  }
}

void SeekAndDestroy_R() { // estratégia número 7 no controle
  leituraSensores();

  switch (classificarAlvo()) {

    case SEM_ALVO:
      Serial.println("Searching Enemy...");
      mover(-723, 723);
      break;

    case ALVO_CENTRO:
      Serial.println("ROBOT ATTACK!");
      mover(1023, 1023);
      break;

    case ALVO_ESQUERDA:
      Serial.println("Left Detected!");
      mover(-723, 723);
      break;

    case ALVO_DIREITA:
      Serial.println("Right Detected!");
      mover(723, -723);
      break;
  }
}

#endif