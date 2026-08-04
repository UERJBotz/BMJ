#ifndef PRINCIPAL_H
#define PRINCIPAL_H

#include <Arduino.h>
#include "sensores.h"
#include "motores.h"


// ============================================================
// ESTADOS DA VARREDURA PENDULAR
// ============================================================

enum estadoPendulo {
    GIRA_DIREITA,
    GIRA_ESQUERDA
};


// ============================================================
// CLASSIFICAÇÃO DO ALVO
// ============================================================

enum AlvoDirecao {
    SEM_ALVO,
    ALVO_ESQUERDA,
    ALVO_CENTRO,
    ALVO_DIREITA
};


// ============================================================
// SENSORES
//
// [0] = Frente Esquerda
// [1] = Frente
// [2] = Frente Direita
// ============================================================

int leitura[3];


// ============================================================
// PARÂMETROS PRINCIPAIS
// ============================================================

// Velocidade base durante tracking
const int VELOCIDADE_BASE = 650;

// Velocidade da varredura
const int VELOCIDADE_VARREDURA = 650;

// Velocidade máxima permitida no tracking
const int VELOCIDADE_MAX = 800;


// ============================================================
// PID / PD
// ============================================================

float erro_angular = 0.0;
float erro_anterior = 0.0;

float P = 0.0;
float I = 0.0;
float D = 0.0;

float PID = 0.0;


// PD
const float Kp = 350.0;
const float Ki = 0.0;
const float Kd = 120.0;


// ============================================================
// VARREDURA
// ============================================================

estadoPendulo estadoAtual = GIRA_DIREITA;

unsigned long ultimo_pendulo = 0;

// Tempo de permanência em cada direção
const unsigned long TEMPO_PENDULO = 350;


// ============================================================
// FULL ATTACK
// ============================================================

int ataque_confirmado = 0;

const int ATAQUE_THRESHOLD = 4;


// ============================================================
// LEITURA DOS SENSORES
// ============================================================

void leituraSensores() {

    leitura[0] = digitalRead(FEsq);
    leitura[1] = digitalRead(Frente);
    leitura[2] = digitalRead(FDir);
}


// ============================================================
// CLASSIFICAÇÃO DO ALVO
// ============================================================

AlvoDirecao classificarAlvo() {

    // Sensor frontal tem prioridade
    if (leitura[1]) {
        return ALVO_CENTRO;
    }

    // Os dois laterais simultaneamente
    // indicam que o alvo está próximo do centro
    if (leitura[0] && leitura[2]) {
        return ALVO_CENTRO;
    }

    if (leitura[0]) {
        return ALVO_ESQUERDA;
    }

    if (leitura[2]) {
        return ALVO_DIREITA;
    }

    return SEM_ALVO;
}


// ============================================================
// ERRO ANGULAR
// ============================================================

void calculoErroAngular() {

    const float peso[3] = {
        -1.0,
         0.0,
         1.0
    };

    float soma = 0.0;
    int ativos = 0;

    for (int i = 0; i < 3; i++) {

        if (leitura[i]) {

            soma += peso[i];
            ativos++;
        }
    }

    if (ativos > 0) {

        erro_angular = soma / ativos;

    } else {

        erro_angular = erro_anterior;
    }
}


// ============================================================
// PD
// ============================================================

void pid() {

    calculoErroAngular();

    P = erro_angular;

    // Integral desativada
    I = 0.0;

    // Derivada discreta
    D = erro_angular - erro_anterior;

    PID =
        (Kp * P) +
        (Ki * I) +
        (Kd * D);

    erro_anterior = erro_angular;
}


// ============================================================
// VARREDURA PENDULAR
// ============================================================

estadoPendulo varreduraPendular(estadoPendulo estado) {

    unsigned long agora = millis();

    if ((agora - ultimo_pendulo) < TEMPO_PENDULO) {
        return estado;
    }

    ultimo_pendulo = agora;

    switch (estado) {

        case GIRA_DIREITA:

            mover(
                VELOCIDADE_VARREDURA,
                -VELOCIDADE_VARREDURA
            );

            return GIRA_ESQUERDA;


        case GIRA_ESQUERDA:

            mover(
                -VELOCIDADE_VARREDURA,
                VELOCIDADE_VARREDURA
            );

            return GIRA_DIREITA;
    }

    return GIRA_DIREITA;
}


// ============================================================
// FULL ATTACK
// ============================================================

bool fullAttackDetectado() {

    if (leitura[0] &&
        leitura[1] &&
        leitura[2]) {

        if (ataque_confirmado < ATAQUE_THRESHOLD) {
            ataque_confirmado++;
        }

    } else {

        ataque_confirmado = 0;
    }

    return ataque_confirmado >= ATAQUE_THRESHOLD;
}


// ============================================================
// TRACKING PRINCIPAL
// ============================================================

void iSeeYou() {

    leituraSensores();

    AlvoDirecao alvo = classificarAlvo();


    // ========================================================
    // SEM ALVO
    // ========================================================

    if (alvo == SEM_ALVO) {

        // Não deixa o derivativo carregar informação
        // de uma aquisição anterior.
        erro_anterior = 0.0;

        estadoAtual =
            varreduraPendular(estadoAtual);

        return;
    }


    // ========================================================
    // FULL ATTACK
    // ========================================================

    if (fullAttackDetectado()) {

        mover(
            1023,
            1023
        );

        return;
    }


    // ========================================================
    // TRACKING PD
    // ========================================================

    pid();


    int velocidade_esq =
        VELOCIDADE_BASE + (int)PID;

    int velocidade_dir =
        VELOCIDADE_BASE - (int)PID;


    velocidade_esq =
        constrain(
            velocidade_esq,
            -VELOCIDADE_MAX,
            VELOCIDADE_MAX
        );

    velocidade_dir =
        constrain(
            velocidade_dir,
            -VELOCIDADE_MAX,
            VELOCIDADE_MAX
        );


    mover(
        velocidade_esq,
        velocidade_dir
    );
}

#endif