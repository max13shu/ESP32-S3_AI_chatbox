// ESP32_WS2812_Lib.h
/**
 * Brief	A library for controlling ws2812 in esp32 platform.
 * Author	ZhentaoLin
 * Url      https://github.com/Zhentao-Lin/ESP32_WS2812_Lib
 * Date		2024-09-02
 */

#ifndef __ESP32_WS2812_Lib_H
#define __ESP32_WS2812_Lib_H

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include "WProgram.h"
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "Arduino.h"

#include "esp32-hal.h"

enum LED_TYPE
{                     //R  G  B
    TYPE_RGB = 0x06,  //00 01 10
    TYPE_RBG = 0x09,  //00 10 01
    TYPE_GRB = 0x12,  //01 00 10
    TYPE_GBR = 0x21,  //10 00 01
    TYPE_BRG = 0x18,  //01 10 00
    TYPE_BGR = 0x24   //10 01 00
};

class ESP32_WS2812
{
protected:
    
    uint16_t ledCounts;
    uint8_t pin;
    uint8_t br;
    uint8_t rmt_chn;
    
    uint8_t rOffset;
    uint8_t gOffset;
    uint8_t bOffset;
    
    float realTick;
    rmt_reserve_memsize_t rmt_mem;
    uint32_t *led_rgb_data;

public:
    ESP32_WS2812(uint16_t n = 8, uint8_t gpio = 2, uint8_t chn = 0, LED_TYPE t = TYPE_GRB);

    bool begin();
    void setLedCount(uint16_t n);
    void setLedType(LED_TYPE t);
    void setBrightness(uint8_t brightness);

    esp_err_t setLedColorData(int index, uint32_t rgb);
    esp_err_t setLedColorData(int index, uint8_t r, uint8_t g, uint8_t b);

    esp_err_t setLedColor(int index, uint32_t rgb);
    esp_err_t setLedColor(int index, uint8_t r, uint8_t g, uint8_t b);

    esp_err_t setAllLedsColorData(uint32_t rgb);
    esp_err_t setAllLedsColorData(uint8_t r, uint8_t g, uint8_t b);

    esp_err_t setAllLedsColor(uint32_t rgb);
    esp_err_t setAllLedsColor(uint8_t r, uint8_t g, uint8_t b);

    esp_err_t show();
    
    uint16_t getLedCount(void);
    uint8_t getBrightness(void);
    
    uint32_t getLedColorData(int index);
    uint32_t* getLedAllColorDate(void);

    
    uint32_t Wheel(byte pos);
    uint32_t hsv2rgb(uint32_t h, uint32_t s, uint32_t v);
    
};

#endif

