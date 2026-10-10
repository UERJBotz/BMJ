#ifndef Estrategias_H
#define Estrategias_H

#include <Arduino.h>
#include "motores.h"

#ifndef BMJ_DEBUG_ESTRATEGIAS
#define BMJ_DEBUG_ESTRATEGIAS 0
#endif

#if BMJ_DEBUG_ESTRATEGIAS
#define BMJ_LOG(msg) Serial.println(msg)
#else
#define BMJ_LOG(msg) ((void)0)
#endif

extern int leitura[3];
void leituraSensores();

#define VEL_BUSCA_ESTADO 600
#define VEL_BUSCA_LENTA  250
#define VEL_GIRO_ESTADO  700
#define VEL_GIRO_LENTO   400
#define VEL_FRENTE       1023

enum simbolo {
    NADA = 0,
    FRENTE,
    ESQ,
    DIR,
    FRENTE_ESQ,
    FRENTE_DIR,
};
enum estado {
    G_ESQ,
    G_FRENTE_ESQ,
    RETO,
    G_FRENTE_DIR,
    G_DIR,
};

// Define os próximos estados com base no estado atual e símbolo lido
estado prox_estado(estado e, simbolo s) {
    switch (e) {
        case G_DIR:
            switch (s) {
                case FRENTE:     return RETO;
                case ESQ:        return G_ESQ;
                case FRENTE_ESQ: return G_FRENTE_ESQ;
                case FRENTE_DIR: return G_FRENTE_DIR;
                case DIR:        return G_DIR;
                case NADA:       return G_DIR;
            } break;
        case RETO:
            switch (s) {
                case FRENTE:     return RETO;
                case ESQ:        return G_ESQ;
                case FRENTE_ESQ: return G_FRENTE_ESQ;
                case FRENTE_DIR: return G_FRENTE_DIR;
                case DIR:        return G_DIR;
                case NADA:       return G_DIR;
            } break;
        case G_ESQ:
            switch (s) {
                case FRENTE:     return RETO;
                case ESQ:        return G_ESQ;
                case FRENTE_ESQ: return G_FRENTE_ESQ;
                case FRENTE_DIR: return G_FRENTE_DIR;
                case DIR:        return G_DIR;
                case NADA:       return G_ESQ;
            } break;
        case G_FRENTE_ESQ:
            switch(s) {
                case FRENTE:     return RETO;
                case ESQ:        return G_ESQ;
                case FRENTE_ESQ: return G_FRENTE_ESQ;
                case FRENTE_DIR: return G_FRENTE_DIR;
                case DIR:        return G_DIR;
                case NADA:       return G_ESQ;
            } break;
        case G_FRENTE_DIR:
            switch(s) {
                case FRENTE:     return RETO;
                case ESQ:        return G_ESQ;
                case FRENTE_ESQ: return G_FRENTE_ESQ;
                case FRENTE_DIR: return G_FRENTE_DIR;
                case DIR:        return G_DIR;
                case NADA:       return G_DIR;
            } break;
    }

    return e; // valor padrão de segurança
}

void acao_atual(estado e) {
    switch (e) {
        case RETO: {
            Serial.println("EMPURRANDO");
            mover(1023,1023);
        } break;
        case G_ESQ: {
            Serial.println("GIRANDO PRA ESQUERDA");
            mover(-600,600);
        } break;
        case G_FRENTE_ESQ: {
            Serial.println("GIRANDO LEVE PARA ESQUERDA");
            mover(800, 1023);
        } break;
        case G_FRENTE_DIR: {
            Serial.println("GIRANDO LEVE PARA DIREITA");
            mover(1023, 800);
        } break;
        case G_DIR: {
            Serial.println("GIRANDO PRA DIREITA");
            mover(600,-600);
        } break;
    }
}


#endif