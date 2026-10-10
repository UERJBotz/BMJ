/* BMJ
 * IRremote=4.4.1
 * 1=PREPARE, 2=START (somente depois de 1), 3=STOP, 4..8=estratégias
 * https://github.com/UERJBotz/BMJ
 */
#include <Arduino.h>
#include "SumoIR.h"
#include "motores.h"
#include "sensores.h"
#include "LEDFX.h"
#include "Principal.h"
#include "Estrategias.h"
#include "placa.h"

#define boot 0
#define LED_PIN 2
#ifndef BMJ_DIAGNOSTICO_IR
#define BMJ_DIAGNOSTICO_IR 1
#endif

enum estado estado_atual;

SumoIR IR;
int strategy = 4;
int modo_anterior = SumoIR::SUMO_STOP;
unsigned long inicio_sinalizacao = 0;
unsigned long duracao_sinalizacao = 0;
unsigned long ultimo_status = 0;
unsigned long quadros_ir = 0;

unsigned long inicio_led_estrategia = 0;
int pulsos_led_estrategia = 0;
bool led_estrategia_ativo = false;


void sinalizar(unsigned long duracao) {
    inicio_sinalizacao = millis();
    duracao_sinalizacao = duracao;
}

void atualizarLed() {
    const bool base = IR.on() || IR.prepare();

    // Indicacao da estrategia: 120 ms aceso e 300 ms apagado
    if (led_estrategia_ativo) {
        unsigned long dt = millis() - inicio_led_estrategia;
        unsigned long duracao = (unsigned long)pulsos_led_estrategia * 420UL;

        if (dt < duracao) {
            unsigned long fase = dt % 420UL;
            bool aceso = fase < 120UL;
            digitalWrite(LED_PIN, aceso ? HIGH : LOW);
            return;
        }

        led_estrategia_ativo = false;
    }

    // Sinalizacao normal de STOP, PREPARE e START
    const unsigned long dt = millis() - inicio_sinalizacao;
    
    if (duracao_sinalizacao && dt < duracao_sinalizacao) {
        digitalWrite(
            LED_PIN,
            ((dt / 100) % 2 == 0) ? !base : base
        );
    } else {
        duracao_sinalizacao = 0;
        digitalWrite(LED_PIN, base ? HIGH : LOW);
    }
}


void indicarEstrategia(int numero) {
    if (numero < 4 || numero > 8) return;

    pulsos_led_estrategia = numero - 3;
    inicio_led_estrategia = millis();
    led_estrategia_ativo = true;

    Serial.print("[ESTRATEGIA] Selecionada: ");
    Serial.println(numero);
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
    pinMode(boot, INPUT_PULLUP);
    Serial.println("[BOOT] BMJ IR-recovery 2");

    setupSensores();
    setupMotores(); // somente GPIO; nenhum canal PWM e criado aqui
    parar();        // freio por GPIO enquanto PWM ainda nao foi configurado
    Serial.println("[BOOT] GPIO dos motores pronto; PWM adiado ate START");

    IR.setLed(-1, true, 250); // BMJ e o unico responsavel pelo LED GPIO2
    IR.debug(false);         // log detalhado controlado abaixo, fora da biblioteca
    IR.begin(IR_PIN);        // configura explicitamente GPIO15 durante setup
    modo_anterior = IR.mode();
    Serial.println("[BOOT] IR pronto: GPIO15; 1 prepara, 2 inicia, 3 para");
    sinalizar(400);          // duas piscadas sem impedir recepcao
}

void loop() {
    // Ler exatamente uma vez. Um segundo update apagaria o comando do ciclo.
    const int cmd = IR.update();
    const int modo = IR.mode();
    const bool mudou = modo != modo_anterior;

    if (IR.off()) {
      parar();
      //mostra_estrategia_no_led(strategy);
    }
    if (IR.prepare()) {
      estado_atual = G_ESQ;
      leituraSensores();
      //mostra_sensores_no_led(leitura);
      parar();
    }

    if (IR.available()) {
        ++quadros_ir; // inclui protocolos nao mapeados; available nao significa cmd valido
#if BMJ_DIAGNOSTICO_IR
        Serial.println(IR.str());
#endif
    }

    // STOP e tratado tambem quando o estado ja era STOP.
    if (cmd == 3 || (mudou && modo == SumoIR::SUMO_STOP)) {
        parar();
        reiniciarPerseguicao();
        duracao_sinalizacao = 0;
        Serial.println("[ESTADO] STOP");
    }

    if (mudou && modo == SumoIR::SUMO_PREPARE) {
        parar();
        Serial.println("[ESTADO] PREPARE: aguardando 2");
    }

    
    if (!IR.on() && IR.available() && cmd >= 4 && cmd <= 8) {
        strategy = cmd;

        reiniciarPerseguicao();
        indicarEstrategia(strategy);

        Serial.println("[SELECAO] Estrategia aceita");
    }


    if (mudou && modo == SumoIR::SUMO_START) {
        Serial.println("[START] configurando PWM de 10 bits");
        Serial.println("[START] rotina PWM retornou; executando estrategia");
        iniciarMMPerseguir();
        iniciarDevagarPerseguir();
        duracao_sinalizacao = 0;
    }

    if (IR.on()) {
        if(strategy == 6) leituraSensores();

        enum simbolo simb;

        // Máquina de Estados 
        if      (leitura[0] && leitura[1] && leitura[2]) simb = FRENTE;
        else if (leitura[0] && leitura[1])               simb = FRENTE_ESQ;
        else if (leitura[1] && leitura[2])               simb = FRENTE_DIR;
        else if (leitura[1])                             simb = FRENTE;
        else if (leitura[0])                             simb = ESQ;
        else if (leitura[2])                             simb = DIR;
        else                                             simb = NADA;

        switch (strategy) {
            default:
            case 4: Perseguir(); break;
            case 5:
                MadMax();
                delay(5);
                break;
            case 6:
                estado_atual = prox_estado(estado_atual, simb);
                acao_atual(estado_atual);
                delay(5);
                break;
            case 7: MMPerseguir(); break;
            case 8: DevagarPerseguir(); break;
        }
    }
#if BMJ_DIAGNOSTICO_IR
    else if (millis() - ultimo_status >= 2000) {
        ultimo_status = millis();
        char status[100];
        snprintf(status, sizeof(status), "[VIVO] modo=%d quadros=%lu entradaIR=%d",
                 modo, quadros_ir, digitalRead(IR_PIN));
        Serial.println(status);
    }
#endif
    modo_anterior = modo;
    atualizarLed();
    // Pausa cooperativa curta. Nao e uma espera de 50 ms por decisao do PD.
    delay(1);
}
