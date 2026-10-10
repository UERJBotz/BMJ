#ifndef Principal_H
#define Principal_H

#include <Arduino.h>
#include "motores.h"
#include "sensores.h"

// Leitura dos Sensores
// [0] = Frente Esquerda
// [1] = Frente
// [2] = Frente Direita

int leitura[3];

int vel_base = 400;
float erro_angular = 0;
float erro_anterior = 0;

float P = 0;
float I = 0;
float D = 0;

float PID = 0;

// Somente PD 
float Kp = 200.0; // ponto inicial experimental, nao um valor otimo medido
float Ki = 0.0;
float Kd = 20.0;
// Escala preservada: -2 corresponde ao sensor a -30 graus e +2 a +30.
// D e normalizado para uma amostra de 5 ms; nao usa graus/segundo.
const unsigned long PERIODO_PID_MS = 5;
const int VEL_MAX_PID = 900;
const int VEL_LATERAL = 250;
const int VEL_CENTRAL = 650;
const int VEL_BUSCA = 400;
const unsigned long TEMPO_CONFIRMA_ATAQUE_MS = 15;
int ultima_direcao = 1; // padrao: direita; atualizada somente com visao lateral inequivoca
bool derivada_valida = false;
bool controle_iniciado = false;
bool confirmando_ataque = false;
bool devagar_encerrado = false;
unsigned long inicio_ataque = 0;
unsigned long ultimo_controle = 0;

unsigned long last_time = 0;

unsigned long ultimo_pendulo = 0;
 
const int tempo_pendulo = 250; //ms

const int tempo_devagar = 600; //ms
unsigned long inicio_devagar = 0; //início devagar 
const int VEL_DEVAGAR = 200;

const int tempo_chegada = 600; //ms
unsigned long inicio_MM = 0; //início Mad Max

// quantidade de ciclos consecutivos
// detectando frontal
int ataque_confirmado = 0;

// Mantido como contador diagnostico saturado, sem depender da frequencia do loop.
const int ATAQUE_THRESHOLD = 3;

// Chamar no START e ao trocar de estrategia; nao chamar a cada loop.
void reiniciarPerseguicao() {
    erro_angular = erro_anterior = P = I = D = PID = 0;
    ultima_direcao = 1;
    derivada_valida = controle_iniciado = confirmando_ataque = false;
    ataque_confirmado = 0;
    devagar_encerrado = false;
    last_time = ultimo_controle = millis();
}

void MadMax() { 
  mover(1023, 1023);
}

void leituraSensores() {
    leitura[0] = digitalRead(FEsq);
    leitura[1] = digitalRead(Frente);
    leitura[2] = digitalRead(FDir);
}

void calculoErroAngular() {

    // geometria angular
    const float peso[3] = {-1.5f, 0.0f, 2.0f};

    float soma_pesos = 0;
    int ativos = 0;

    for (int i = 0; i < 3; i++) {
        if (leitura[i]) {
            soma_pesos += peso[i];
            ativos++;
        }
    }

    if (leitura[0] && leitura[2] && !leitura[1]) {
        // 101 e ambiguo: nao assumir alvo centralizado nem atualizar a memoria.
        erro_angular = static_cast<float>(ultima_direcao);
    } else if (ativos > 0) {
        erro_angular = soma_pesos / ativos;
        if (erro_angular > 0) ultima_direcao = 1;
        else if (erro_angular < 0) ultima_direcao = -1;
    }
}

// PID

void pid() {
    calculoErroAngular();
    const unsigned long agora = millis();
    const unsigned long dt = agora - last_time;
    P = erro_angular;
    // Ki=0: nao acumular uma integral inutil indefinidamente.
    if (Ki != 0.0f && derivada_valida && dt > 0) {
        I = constrain(I + erro_angular * (dt / 5.0f), -100.0f, 100.0f);
    } else if (Ki == 0.0f) I = 0;
    D = (derivada_valida && dt > 0 && dt <= 100)
        ? (erro_angular - erro_anterior) * (5.0f / dt) : 0.0f;
    PID = (Kp * P) + (Ki * I) + (Kd * D);
    erro_anterior = erro_angular;
    last_time = agora;
    derivada_valida = true;
}


// FULL ATTACK

bool fullAttackDetectado() {
    if (!(leitura[0] && leitura[1] && leitura[2])) {
        confirmando_ataque = false;
        ataque_confirmado = 0;
        return false;
    }
    if (!confirmando_ataque) {
        inicio_ataque = millis();
        confirmando_ataque = true;
    }
    if (ataque_confirmado < ATAQUE_THRESHOLD) ++ataque_confirmado;
    return (millis() - inicio_ataque >= TEMPO_CONFIRMA_ATAQUE_MS);
}

// TARGET TRACKER PRINCIPAL
void Perseguir() { // estrategia numero 4 no controle
    const unsigned long agora = millis();
    // Sem delay: o loop continua livre para receber STOP pelo IR.
    if (controle_iniciado && agora - ultimo_controle < PERIODO_PID_MS) return;
    // Nao aproveitar confirmacoes/derivadas anteriores a uma pausa longa.
    if (controle_iniciado && agora - ultimo_controle > 100) {
        confirmando_ataque = false;
        ataque_confirmado = 0;
        derivada_valida = false;
    }
    ultimo_controle = agora;
    controle_iniciado = true;
    leituraSensores();
    const bool ataque = fullAttackDetectado(); // tambem zera ao perder alvo

    if (!leitura[0] && !leitura[1] && !leitura[2]) {
        derivada_valida = false; // sem impulso D de uma amostra antiga na retomada
        I = 0;
        mover(ultima_direcao * VEL_BUSCA, -ultima_direcao * VEL_BUSCA);
        return;
    }

    // Atualiza erro inclusive durante ataque maximo.
    pid();
    if (ataque) {
        mover(1023, 1023);
        return;
    }

    // Reduz translacao quando o alvo esta fora do centro: curva mais fechada.
    // Aumenta avanco quando somente o frontal detecta ou os tres detectam.
    int base = vel_base;
    if (!leitura[1]) base = VEL_LATERAL;
    else if (leitura[0] == leitura[2]) base = VEL_CENTRAL;

    int velocidade_esq = base + PID;
    int velocidade_dir = base - PID;
    velocidade_esq = constrain(velocidade_esq, -VEL_MAX_PID, VEL_MAX_PID);
    velocidade_dir = constrain(velocidade_dir, -VEL_MAX_PID, VEL_MAX_PID);
    mover(velocidade_esq, velocidade_dir);
}

void iniciarMMPerseguir() {
    reiniciarPerseguicao();
    inicio_MM = millis();
}

void iniciarDevagarPerseguir() {
    reiniciarPerseguicao();
    inicio_devagar = millis();
}

void MMPerseguir() {
    if (millis() - inicio_MM < tempo_chegada) {
        MadMax(); 
    } else {
        Perseguir(); 
    }
}

void DevagarPerseguir() {

    leituraSensores();

    // Período inicial curto: anda para frente devagar
    if (!devagar_encerrado && millis() - inicio_devagar < tempo_devagar) {
        // Detectou o inimigo
        if (leitura[0] || leitura[1] || leitura[2]) {
            devagar_encerrado = true;
            Perseguir();
        }
        else {
            // Ainda não detectou: continua avançando
            mover(VEL_DEVAGAR, VEL_DEVAGAR);
        }

    }
    else {

        // Tempo inicial acabou: passa para perseguição
        Perseguir();
    }

}
// bool evitarBorda() {

//     bool linha_esq = digitalRead(linhaEsq);
//     bool linha_dir = digitalRead(linhaDir);

//     if (!linha_esq && !linha_dir) {
//         return false;
//     }

//     Serial.println("!!! BORDA DETECTADA !!!");

//     // trava curta
//     parar();
//     delay(5);

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
