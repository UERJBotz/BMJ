#ifndef ledFX_H
#define ledFX_H
#include <Adafruit_NeoPixel.h>

#define LED_PIN 2 //pino do anel de leds
#define NUMPIXELS 8 // quantidade de leds do anel
Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800); // necessario


const auto azul  = pixels.Color(0,   0,   150);
const auto verde = pixels.Color(150, 0,   0);

void setar_cor_leds(uint8_t r, uint8_t g, uint8_t b) {
  for (uint8_t i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
}

void mostra_sensores_no_led(int leitura[]) {
  setar_cor_leds(3, 5, 3); // Define a cor padrão para todos os LEDs

  if (leitura[0]) pixels.setPixelColor(5, azul);
  if (leitura[1]) pixels.setPixelColor(4, azul);
  if (leitura[2]) pixels.setPixelColor(3, azul);

  pixels.show();
}
void mostra_estrategia_no_led(int num_estrategia) {
  setar_cor_leds(3, 5, 3);
  for (uint8_t i = 0; i < (num_estrategia - 3); i++) pixels.setPixelColor(i, verde);
  pixels.show();
}

#endif //ledFX_H