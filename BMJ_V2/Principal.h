#ifndef Principal_H
#define Principal_H

#include "sensores.h"

enum estadoPendulo {
    DIREITA,
    MEIA_ESQUERDA,
    ESQUERDA,
    MEIA_DIREITA
};

// Classificação de onde está o alvo, usada tanto pelo PID (iSeeYou)
// quanto pelas estratégias liga-desliga (SeekAndDestroy_L/R) em
// Estrategias.h. Ter uma única fonte de verdade evita que as
// estratégias "discordem" sobre onde está o inimigo.
enum AlvoDirecao {
    SEM_ALVO,
    ALVO_ESQUERDA,
    ALVO_CENTRO,   // frontal ativo, ou os dois laterais ativos sem frontal
    ALVO_DIREITA
};


int vel_base = 400;
float erro_angular = 0;
float erro_anterior = 0;

float P = 0;
float I = 0;
float D = 0;

float PID = 0;

// Somente PD
float Kp = 140.0;
float Ki = 0.0;
float Kd = 20.0;
// Valores mantidos como estavam (não testados de novo). Se sobrar tempo
// de bancada:
// -> confirme que Kp=140 dá resposta rápida e um pouco oscilante
// -> se oscilar demais, suba Kd aos poucos (ex: 20, depois 40...)
// -> só mexa em Ki se notar erro residual persistente (raro em sumo)

unsigned long last_time = 0;

unsigned long ultimo_pendulo = 0;

estadoPendulo estadoAtual = DIREITA;

const int tempo_pendulo = 150; //ms
// Comportamento inalterado: DIREITA/MEIA_DIREITA usam o mesmo comando
// de motor (idem MEIA_ESQUERDA/ESQUERDA), então o efeito é girar
// ~300ms pra um lado e ~300ms pro outro. Já era assim, não mexi.


// quantidade de ciclos consecutivos
// detectando frontal
int ataque_confirmado = 0;

const int ATAQUE_THRESHOLD = 2;


// Única função que decide "onde está o inimigo" a partir de leitura[].
// leituraSensores() precisa ter sido chamado antes.
AlvoDirecao classificarAlvo() {
    if (leitura[1]) return ALVO_CENTRO;               // frontal manda, igual antes
    if (leitura[0] && leitura[2]) return ALVO_CENTRO;  // CORRIGIDO: os dois laterais sem
                                                        // frontal também é alvo à frente/perto,
                                                        // não "sem inimigo"
    if (leitura[0]) return ALVO_ESQUERDA;
    if (leitura[2]) return ALVO_DIREITA;
    return SEM_ALVO;
}

void calculoErroAngular() { //! retornar erro

    // geometria angular
    float peso[3] = {-2.0, 0.0, 2.0};

    float soma_pesos = 0;
    int ativos = 0;

    for (int i = 0; i < 3; i++) {

        if (leitura[i]) {

            soma_pesos += peso[i];
            ativos++;
        }
    }

    if (ativos > 0) {

        erro_angular = soma_pesos / ativos;

    } else {

        // mantém direção anterior
        // (branch hoje inalcançável: pid() só é chamado quando há
        // pelo menos 1 sensor ativo. Salvaguarda caso reaproveitem
        // esta função em outro contexto.)
        if (erro_angular >= 0)
            erro_angular = 2.5;
        else
            erro_angular = -2.5;
    }
}

// PID

void pid() { //! retornar
    calculoErroAngular(); //!

    // Anti-windup simples: só acumula I se Ki estiver realmente ativo.
    // mas evita I acumulado escondido se ligarem Ki depois.
    #if 0
    if (abs(Ki) >= EPSILON) {
        I += erro_angular;
    } else {
        I = 0.0;
    }
    #endif

    P = erro_angular;
    D = erro_angular - erro_anterior;

    PID = (Kp * P) + (Ki * I) + (Kd * D);

    erro_anterior = erro_angular;
}

// VARREDURA PENDULAR (lógica inalterada — já validada por vocês)
//! só usar no começo antes de ter visto qualquer coisa
estadoPendulo varreduraPendular(estadoPendulo estadoAtual) {
    unsigned long agora = millis();

    if ((agora-ultimo_pendulo) >= tempo_pendulo) {

        ultimo_pendulo = agora;

        switch(estadoAtual) { //! define VELOCIDADE_VARREDURA
            case DIREITA:
                mover(350,-350);
                return MEIA_ESQUERDA;

            case MEIA_ESQUERDA:
                mover(-350,350);
                return ESQUERDA;

            case ESQUERDA:
                mover(-350,350);
                return MEIA_DIREITA;

            case MEIA_DIREITA:
                mover(350,-350);
                return DIREITA;
        }
    }

    return estadoAtual;
}


// FULL ATTACK (condição original — 3 sensores confirmados por N ciclos)
bool fullAttackDetectado() {
    if (leitura[0] &&
        leitura[1] &&
        leitura[2]) {
        ataque_confirmado++;
    } else {
        ataque_confirmado = 0;
    }

    return (ataque_confirmado >= ATAQUE_THRESHOLD);
}

// TARGET TRACKER PRINCIPAL
void iSeeYou() { // estratégia número 4 no controle
    //if(evitarBorda()) return;
    leituraSensores();

    // SEM ALVO -> VARREDURA PENDULAR

    if (classificarAlvo() == SEM_ALVO) {
        estadoAtual = varreduraPendular(estadoAtual);
        return;
    }

    // OS 3 SENSORES, CONFIRMADO POR N CICLOS -> FULL ATTACK

    if (fullAttackDetectado()) {
        mover(1023, 1023);
        return;
    }

    // COM ALVO -> PID ANGULAR

    pid(); //!

    // Sinal confirmado por consistência com varreduraPendular():
    // alvo à esquerda (erro negativo) -> esq diminui, dir aumenta ->
    // gira PARA a esquerda (na direção do alvo).
    int velocidade_esq = vel_base + PID;
    int velocidade_dir = vel_base - PID;

    velocidade_esq = constrain(velocidade_esq, -800, 800);  //! define VEL_MAX
    velocidade_dir = constrain(velocidade_dir, -800, 800);  //! define VEL_MAX
    mover(velocidade_esq, velocidade_dir);
}

// bool evitarBorda() {

//     bool linha_esq = digitalRead(linhaEsq);
//     bool linha_dir = digitalRead(linhaDir);
//     if (!linha_esq && !linha_dir) return false;

//     Serial.println("!!! BORDA DETECTADA !!!");

//     // trava curta
//     parar(); delay(5);

//     // BORDA ESQUERDA
//     if (linha_esq && !linha_dir) {
//         Serial.println("BORDA ESQUERDA");

//         // micro-recuo angular
//         mover(-700, -250);
//         delay(90);

//         // gira rapidamente para dentro
//         mover(850, -850);
//         delay(140);
//     }

//     // BORDA DIREITA
//     else if (linha_dir && !linha_esq) {
//         Serial.println("BORDA DIREITA");

//         // micro-recuo angular
//         mover(-250, -700);
//         delay(90);

//         // gira rapidamente para dentro
//         mover(-850, 850);
//         delay(140);
//     }

//     // BORDA FRONTAL
//     else {
//         Serial.println("BORDA FRONTAL");

//         // recuo curto
//         mover(-850, -850);
//         delay(120);

//         // escolhe direção usando último erro PID
//         if (erro_angular >= 0) {

//             mover(-850, 850);

//         } else {

//             mover(850, -850);
//         }

//         delay(180);
//     }

//     parar();
//     return true;
// }

#endif