#ifndef Estrategias_H
#define Estrategias_H


#define VEL_SEEK 700

extern int leitura[3];
void leituraSensores();

int EstadoAtual;

void EstadoUpdate() {
  leituraSensores();
  EstadoAtual = 1; // sem inimigo
  if (leitura[0] && leitura[1] && leitura[2]) {
    EstadoAtual = 2; // todos os sensores
  }
  else if (leitura[1]) {
    EstadoAtual = 2; // sensor frontal
  }
  else if (!leitura[2] && leitura[0]) {
    EstadoAtual = 3; // sensor esquerdo
  }
  else if (leitura[2] && !leitura[0]) {
    EstadoAtual = 4; // sensor direito
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
    