#ifndef Sensores_H
#define Sensores_H

#define FEsq 33
#define Frente 35
#define FDir 32

// ============================================================
// SENSORES
//
// [0] = Frente Esquerda
// [1] = Frente
// [2] = Frente Direita
// ============================================================

union leitura {
    bool arr[3];
    struct {
        bool esq, centro, dir;
    };
}; //! usar

int leitura[3]; //! mudar tipo

uint8_t sensores[] = {FEsq, Frente, FDir};

void setupSensores() {
    pinMode(FEsq, INPUT);
    pinMode(Frente, INPUT);
    pinMode(FDir, INPUT);
}

void leituraSensores() { //! retornar
    leitura[0] = digitalRead(FEsq);
    leitura[1] = digitalRead(Frente);
    leitura[2] = digitalRead(FDir);
}

#endif
