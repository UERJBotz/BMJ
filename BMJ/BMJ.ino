/*
  BMJ
  12/09/2026, para a RCX;
  Modo AUTÔNOMO usando SumoIR;
  Versão IRremote: 4.4.1

  https://github.com/UERJBotz/BMJ
*/


//#define PID_2222

#include <Arduino.h>
#include "SumoIR.h"

#include "motores.h"
#include "sensores.h"
#include "Principal.h"
#include "Estrategias.h"
//#include "LEDFX.h"
#include "placa.h"


#define boot 0
#define LED_PIN 2
int strategy = 0;

SumoIR IR;

void setup() {

  Serial.begin(115200);

  IR.begin(IR_PIN); 
  
  //pixels.begin();
  //! motor.bip(5, 250, 2500); // motor bipa (x vezes, intervalo (ms), frequencia em Hz)

  setupSensores();
  setupMotores();

  pinMode(boot, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);
  Serial.println("Sistema iniciado no modo AUTO");
}


void loop() {

    IR.update();

    if (IR.stop()) {
        digitalWrite(LED_PIN, LOW);
        parar();
        Serial.println("-> sumo stop"); 
        return;
    }
 
    else if (IR.prepare()) { // número 1 no controle
      // setar_cor_leds(255,255,0);
      // pixels.show();
      digitalWrite(LED_PIN, HIGH);
      delay(20);      
      parar();
      Serial.println("-> sumo prepare"); 
    }
    
    else if (IR.start()) {
      for(int i=0;i<6;i++){
        digitalWrite(LED_PIN,!digitalRead(LED_PIN));
        delay(50);
      }
      iniciarMMPerseguir();
      iniciarDevagarPerseguir();
      Serial.println("-> sumo start"); 
    } 
    
    else if (IR.on()) { // número 2 no controle
      // setar_cor_leds(0,255,0);
      // pixels.show();
        digitalWrite(LED_PIN, HIGH);      
        switch (strategy) {
        default: //fallthrough
        case 4:
          Perseguir(); //Somente PID, usando ultimo erro para escolher lado de giro
          digitalWrite(LED_PIN, !digitalRead(LED_PIN));
          delay(50);
          break;

        case 5:
          MadMax(); // Mad Max 
          delay(50);
          break;

        case 6:
          SeekAndDestroy_L(); // Estratégia de giro para a esquerda e atacar quando os sensores veem (sem PID)
          delay(50);
         break;

        case 7:
          SeekAndDestroy_R(); // Estratégia de giro para a direita e atacar quando os sensores veem (sem PID)
          delay(50);
        break;

        case 8:
          MMPerseguir(); // Mad Max + Perseguir
          delay(50); 
        break;

        case 9:
          DevagarPerseguir(); // Ir devagar + Perseguir
          delay(50);
        break;
      }
      Serial.println("-> sumo on"); 
    }

    else { // robô inicia caindo aqui
      // pixels.clear();
      // pixels.show();
      digitalWrite(LED_PIN, LOW);
      int cmd = IR.read();
      if (cmd >= 4 && cmd <= 9) { 
        strategy = cmd;
        digitalWrite(LED_PIN, HIGH);
        delay(100);
        digitalWrite(LED_PIN, LOW);      
      } else return;
      Serial.println("-> sumo off"); 
    }
  } 

// void strategySelection() {
//   int cmd = IR.read();
//   if (cmd >= 4 && cmd <= 7) { 
//     strategy = cmd;
//   } else return;

//   if (cmd <= 7) {
//     const int num_leds = cmd % 8;
//     for(uint8_t i = 0; i < num_leds; i++) {
//       switch ((cmd-3) % 6) { 
//         case 0: pixels.setPixelColor(i, pixels.Color(255, 50,  50  )); break; // Vermelho claro
//         case 1: pixels.setPixelColor(i, pixels.Color(0,   255, 100 )); break; // Verde com toque de azul
//         case 2: pixels.setPixelColor(i, pixels.Color(255, 0,   180 )); break; // Magenta
//         case 3: pixels.setPixelColor(i, pixels.Color(255, 140, 0   )); break; // Laranja
//         case 4: pixels.setPixelColor(i, pixels.Color(100, 200, 255 )); break; // Azul claro
//         case 5: pixels.setPixelColor(i, pixels.Color(180, 255, 0   )); break; // Verde-amarelado
//       } pixels.show();
//     }
//     delay(80);
//     for(uint8_t i = 0; i < num_leds; i++) { 
//       pixels.setPixelColor(i, pixels.Color(0, 0, 0)); // Desliga os LEDs
//       pixels.show();
//     }
//     delay(80);
//   }
// }
