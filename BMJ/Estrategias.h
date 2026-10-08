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


// Configurações
#define VEL_BUSCA_ESTADO 600
#define VEL_BUSCA_LENTA  250
#define VEL_GIRO_ESTADO  700
#define VEL_GIRO_LENTO   400
#define VEL_FRENTE       1023

// Tempo sem detectar nada antes de passar para busca/giro lento
const unsigned long TEMPO_LENTO_MS = 5000;

// Tempo máximo no estado FRENTE antes do giro de 360°
const unsigned long TEMPO_FRENTE_360_MS = 5000;

// Duração aproximada do giro de 360°
const unsigned long TEMPO_GIRO_360_MS = 700;


// Estados
enum EstadoMaquina {
    EST_BUSCA = 0,
    EST_BUSCA_LENTA,
    EST_GIRO_ESQ,
    EST_GIRO_ESQ_LENTO,
    EST_GIRO_DIR,
    EST_GIRO_DIR_LENTO,
    EST_FRENTE,
    EST_GIRO_360
};


// Estado atual da máquina
EstadoMaquina estadoAtual = EST_BUSCA;


// Momento em que o estado atual começou
unsigned long inicioEstado = 0;


// Indica se a estratégia escolhida é a versão esquerda ou direita
bool estrategiaDireita = false;


// Funções Auxiliares
void mudarEstado(EstadoMaquina novoEstado) {
    if (estadoAtual != novoEstado) {
        estadoAtual = novoEstado;
        inicioEstado = millis();

#if BMJ_DEBUG_ESTRATEGIAS
        Serial.print("[FSM] Novo estado: ");

        switch (estadoAtual) {
            case EST_BUSCA:
                Serial.println("BUSCA");
                break;

            case EST_BUSCA_LENTA:
                Serial.println("BUSCA LENTA");
                break;

            case EST_GIRO_ESQ:
                Serial.println("GIRO ESQUERDA");
                break;

            case EST_GIRO_ESQ_LENTO:
                Serial.println("GIRO ESQUERDA LENTO");
                break;

            case EST_GIRO_DIR:
                Serial.println("GIRO DIREITA");
                break;

            case EST_GIRO_DIR_LENTO:
                Serial.println("GIRO DIREITA LENTO");
                break;

            case EST_FRENTE:
                Serial.println("FRENTE");
                break;

            case EST_GIRO_360:
                Serial.println("GIRO 360");
                break;
        }
#endif
    }
}


// Classificação dos Sensores
//
// [0] = Frente esquerda
// [1] = Frente
// [2] = Frente direita
//
// Retorno:
// 0 = NADA
// 1 = ESQUERDA
// 2 = DIREITA
// 3 = FRONTAL
// 4 = TRES SENSORES
// 5 = ESQUERDA + FRENTE
// 6 = DIREITA + FRENTE
// 7 = ESQUERDA + DIREITA

enum VisaoSensor {
    SENSOR_NADA = 0,
    SENSOR_ESQUERDA,
    SENSOR_DIREITA,
    SENSOR_FRONTAL,
    SENSOR_TRES,
    SENSOR_ESQ_FRENTE,
    SENSOR_DIR_FRENTE,
    SENSOR_ESQ_DIR
};


VisaoSensor lerVisao() {
    leituraSensores();

    const bool E = leitura[0];
    const bool F = leitura[1];
    const bool D = leitura[2];

    if (!E && !F && !D) return SENSOR_NADA;
    if ( E &&  F &&  D) return SENSOR_TRES;

    if (!E &&  F && !D) return SENSOR_FRONTAL;
    if ( E && !F && !D) return SENSOR_ESQUERDA;
    if (!E && !F &&  D) return SENSOR_DIREITA;

    if ( E &&  F && !D) return SENSOR_ESQ_FRENTE;
    if (!E &&  F &&  D) return SENSOR_DIR_FRENTE;
    if ( E && !F &&  D) return SENSOR_ESQ_DIR;

    return SENSOR_NADA;
}


// FRENTE
bool visaoFrontal(VisaoSensor visao) {
    return (
        visao == SENSOR_FRONTAL ||
        visao == SENSOR_TRES ||
        visao == SENSOR_ESQ_FRENTE ||
        visao == SENSOR_DIR_FRENTE ||
        visao == SENSOR_ESQ_DIR
    );
}


// Inicialização da máquina

void iniciarMaquinaEstados(bool direita) {
    estrategiaDireita = direita;

    estadoAtual = EST_BUSCA;
    inicioEstado = millis();

    BMJ_LOG("[FSM] Inicializada");
}


// BUSCA
void executarBusca(VisaoSensor visao) {
    // Nada vendo
    if (visao == SENSOR_NADA) {
        if (millis() - inicioEstado >= TEMPO_LENTO_MS) {
            mudarEstado(EST_BUSCA_LENTA);
        } else {
            // Busca normal
            if (estrategiaDireita) {
                mover(VEL_BUSCA_ESTADO, -VEL_BUSCA_ESTADO);
            } else {
                mover(-VEL_BUSCA_ESTADO, VEL_BUSCA_ESTADO);
            }
        }

        return;
    }

    // Sensor esquerdo
    if (visao == SENSOR_ESQUERDA) {
        mudarEstado(EST_GIRO_ESQ);
        return;
    }

    // Sensor direito
    if (visao == SENSOR_DIREITA) {
        mudarEstado(EST_GIRO_DIR);
        return;
    }

    // Qualquer detecção frontal
    if (visaoFrontal(visao)) {
        mudarEstado(EST_FRENTE);
        return;
    }
}


// BUSCA LENTA
void executarBuscaLenta(VisaoSensor visao) {
    if (visao == SENSOR_NADA) {
        if (estrategiaDireita) {
            mover(VEL_BUSCA_LENTA, -VEL_BUSCA_LENTA);
        } else {
            mover(-VEL_BUSCA_LENTA, VEL_BUSCA_LENTA);
        }
        return;
    }

    if (visao == SENSOR_ESQUERDA) {
        mudarEstado(EST_GIRO_ESQ);
        return;
    }

    if (visao == SENSOR_DIREITA) {
        mudarEstado(EST_GIRO_DIR);
        return;
    }


    if (visaoFrontal(visao)) {
        mudarEstado(EST_FRENTE);
        return;
    }
}


// GIRO ESQUERDA
void executarGiroEsquerda(VisaoSensor visao) {
    // Nada vendo
    if (visao == SENSOR_NADA) {
        if (millis() - inicioEstado >= TEMPO_LENTO_MS) {
            mudarEstado(EST_GIRO_ESQ_LENTO);
        } else {
            mover(-VEL_GIRO_ESTADO, VEL_GIRO_ESTADO);
        }

        return;
    }

    // Qualquer detecção frontal
    if (visaoFrontal(visao)) {
        mudarEstado(EST_FRENTE);
        return;
    }
}


// GIRO ESQUERDA LENTO
void executarGiroEsquerdaLento(VisaoSensor visao) {
    if (visaoFrontal(visao)) {
        mudarEstado(EST_FRENTE);
        return;
    }

    // Continua girando lentamente
    mover(-VEL_GIRO_LENTO, VEL_GIRO_LENTO);
}


// GIRO DIREITA
void executarGiroDireita(VisaoSensor visao) {
    // Nada vendo
    if (visao == SENSOR_NADA) {
        if (millis() - inicioEstado >= TEMPO_LENTO_MS) {
            mudarEstado(EST_GIRO_DIR_LENTO);
        } else {
            mover(VEL_GIRO_ESTADO, -VEL_GIRO_ESTADO);
        }
        return;
    }

    // Qualquer detecção frontal
    if (visaoFrontal(visao)) {
        mudarEstado(EST_FRENTE);
        return;
    }
}


// GIRO DIREITA LENTO
void executarGiroDireitaLento(VisaoSensor visao) {
    if (visaoFrontal(visao)) {
        mudarEstado(EST_FRENTE);
        return;
    }

    // Continua girando lentamente
    mover(VEL_GIRO_LENTO, -VEL_GIRO_LENTO);
}


// FRENTE
void executarFrente(VisaoSensor visao) {
    // Qualquer combinação contendo 2 ou 3 sensores
    // mantém o ataque frontal.

    if (
        visao == SENSOR_TRES ||
        visao == SENSOR_ESQ_FRENTE ||
        visao == SENSOR_DIR_FRENTE ||
        visao == SENSOR_ESQ_DIR
    ) {
        if (millis() - inicioEstado >= TEMPO_FRENTE_360_MS) {
            mudarEstado(EST_GIRO_360);
            return;
        }

        mover(VEL_FRENTE, VEL_FRENTE);
        return;
    }


    // Somente sensor frontal
    if (visao == SENSOR_FRONTAL) {
        if (millis() - inicioEstado >= TEMPO_FRENTE_360_MS) {
            mudarEstado(EST_GIRO_360);
            return;
        }

        mover(VEL_FRENTE, VEL_FRENTE);
        return;
    }

    // Somente sensor esquerdo
    if (visao == SENSOR_ESQUERDA) {
        mudarEstado(EST_GIRO_ESQ);
        return;
    }

    // Somente sensor direito
    if (visao == SENSOR_DIREITA) {
        mudarEstado(EST_GIRO_DIR);
        return;
    }


    // Nada vendo
    if (visao == SENSOR_NADA) {
        mudarEstado(EST_BUSCA);
        return;
    }
}


// GIRO 360º
void executarGiro360(VisaoSensor visao) {
    // Qualquer detecção frontal ou múltipla
    // faz o robô voltar imediatamente para FRENTE.

    if (
        visao == SENSOR_TRES ||
        visao == SENSOR_ESQ_FRENTE ||
        visao == SENSOR_DIR_FRENTE ||
        visao == SENSOR_FRONTAL
    ) {
        mudarEstado(EST_FRENTE);
        return;
    }

    // Somente esquerdo
    if (visao == SENSOR_ESQUERDA) {
        mudarEstado(EST_GIRO_ESQ);
        return;
    }

    // Somente direito
    if (visao == SENSOR_DIREITA) {
        mudarEstado(EST_GIRO_DIR);
        return;
    }


    // Nada vendo
    if (visao == SENSOR_NADA) {
        // Continua o giro durante o tempo necessário
        // para completar aproximadamente 360 graus.

        if (millis() - inicioEstado >= TEMPO_GIRO_360_MS) {

            mudarEstado(EST_BUSCA);
            return;
        }

        // Faz o giro no sentido oposto à estratégia.
        if (estrategiaDireita) {
            mover(VEL_GIRO_ESTADO, 0);
        } else {
            mover(0, VEL_GIRO_ESTADO);
        }

        return;
    }
}


// Executar máquina

void executarMaquinaEstados() {
    VisaoSensor visao = lerVisao();

    switch (estadoAtual) {
        case EST_BUSCA:
            executarBusca(visao);
            break;

        case EST_BUSCA_LENTA:
            executarBuscaLenta(visao);
            break;

        case EST_GIRO_ESQ:
            executarGiroEsquerda(visao);
            break;

        case EST_GIRO_ESQ_LENTO:
            executarGiroEsquerdaLento(visao);
            break;

        case EST_GIRO_DIR:
            executarGiroDireita(visao);
            break;

        case EST_GIRO_DIR_LENTO:
            executarGiroDireitaLento(visao);
            break;

        case EST_FRENTE:
            executarFrente(visao);
            break;

        case EST_GIRO_360:
            executarGiro360(visao);
            break;
    }
}


// ESTRATÉGIA 7 — BUSCA PARA DIREITA
void SeekAndDestroy_R() {
    executarMaquinaEstados();
}


// ESTRATÉGIA 6 — BUSCA PARA ESQUERDA
void SeekAndDestroy_L() {
    executarMaquinaEstados();
}

#endif