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

uint8_t m_color[5][3] = { { 255, 0, 0 }, { 0, 255, 0 }, { 0, 0, 255 }, { 255, 255, 255 }, { 0, 0, 0 } };
int delayval = 100;
uint32_t *allLedColor;

void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.setBrightness(10);
}
void loop() {
  for (int j = 0; j < 5; j++) {
    for (int i = 0; i < strip.getLedCount(); i++) {
      strip.setLedColorData(i, m_color[j][0], m_color[j][1], m_color[j][2]);
      strip.show();
      delay(delayval);
    }
    delay(500);

    /*********************************************/
    Serial.printf("strip.getLedColorData() Test:\r\n");
    for (int i = 0; i < strip.getLedCount(); i++) {
      Serial.printf("%x\t", strip.getLedColorData(i));
      if (i % 8 == 7) {
        Serial.printf("\r\n");
      }
    }
    /*********************************************/

    /*********************************************/
    Serial.printf("strip.getLedAllColorDate() Test:\r\n");
    allLedColor = strip.getLedAllColorDate();
    for (int i = 0; i < strip.getLedCount(); i++) {
      Serial.printf("%x\t", allLedColor[i]);
      if (i % 8 == 7) {
        Serial.printf("\r\n");
      }
    }
    Serial.printf("\r\n");
    /*********************************************/
  }
}