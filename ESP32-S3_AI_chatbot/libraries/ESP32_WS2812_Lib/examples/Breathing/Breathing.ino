/**
 * Brief	A library for controlling ws2812 in esp32 platform.
 * Author	ZhentaoLin
 * Url    https://github.com/Zhentao-Lin/ESP32_WS2812_Lib
 * Date		2024-09-02
 */
#include "ESP32_WS2812_Lib.h"

#define LEDS_COUNT 8
#define LEDS_PIN   2
#define CHANNEL    0

ESP32_WS2812 strip = ESP32_WS2812(LEDS_COUNT, LEDS_PIN, CHANNEL, TYPE_GRB);

void setup() {
  strip.begin();
  strip.setBrightness(0);
  strip.setAllLedsColor(0, 0, 255);
  strip.show();
}
void loop() {
  for (int i = 0; i < 50; i++) {
    strip.setBrightness(i);
    delay(20);
  }
  for (int i = 50; i >= 0; i--) {
    strip.setBrightness(i);
    delay(20);
  }
}