#ifndef Estrategias_H
#define Estrategias_H

#include "Principal.h"

#define VEL_SEEK 500

void paraTras() { // estratégia número 5 no controle
  mover(-1023, -1023);
}

int EstadoAtual;

void EstadoUpdate(){  // função que atualiza os estados
  leituraSensores();
  EstadoAtual = 1; // sem inimigo
  if(leitura[1]){ //enxergando com o sensor frontal
    EstadoAtual = 2;
  } else if (!leitura[2] && leitura[0]){ // enxergando com o esquerdo
    EstadoAtual = 3;
  } else if (leitura[2] && !leitura[0]){ // enxergando com o direito
    EstadoAtual = 4;
  } else if(   leitura[0] 
            && leitura[1]
            && leitura[2]){ //enxergando com todos
    EstadoAtual = 2;
  }
  else {
    EstadoAtual = 1; // sem inimigo
  }
}

void SeekAndDestroy_R(){  // estratégia número 6 no controle
  EstadoUpdate(); // função atualiza o estado a todo momento
  switch (EstadoAtual){
    case 1:
      Serial.println("Searching Enemy...");
      mover(VEL_SEEK, -VEL_SEEK);
      break;

    case 2:
      Serial.println("ROBOT ATTACK!");
      mover(1023, 1023);
      break;

    case 3:
      Serial.println("Left Detected!");
      mover(-VEL_SEEK, VEL_SEEK);
      break;

    case 4:
      Serial.println("Right Detected!");
      mover(VEL_SEEK, -VEL_SEEK);
      break;
  }
}

void SeekAndDestroy_L(){  // estratégia número 7 no controle
  EstadoUpdate(); // função atualiza o estado a todo momento

  switch (EstadoAtual){
    case 1:
      Serial.println("Searching Enemy...");
      mover(-VEL_SEEK, VEL_SEEK);
      break;

    case 2:
      Serial.println("ROBOT ATTACK!");
      mover(1023, 1023);
      break;

    case 3:
      Serial.println("Left Detected!");
      mover(-VEL_SEEK, VEL_SEEK);
      break;

    case 4:
      Serial.println("Right Detected!");
      mover(VEL_SEEK, -VEL_SEEK);
      break;
  }
}

#endif
    