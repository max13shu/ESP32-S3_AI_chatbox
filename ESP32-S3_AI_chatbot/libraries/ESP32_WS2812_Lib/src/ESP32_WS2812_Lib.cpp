// ESP32_WS2812_Lib.h
/**
 * Brief	A library for controlling ws2812 in esp32 platform.
 * Author	ZhentaoLin
 * Url      https://github.com/Zhentao-Lin/ESP32_WS2812_Lib
 * Date		2024-09-02
 */

#include "ESP32_WS2812_Lib.h"

ESP32_WS2812::ESP32_WS2812(uint16_t n /*= 8*/, uint8_t gpio /*= 2*/, uint8_t chn /*= 0*/, LED_TYPE t /*= TYPE_GRB*/)
{
	ledCounts = n;
	pin = gpio;
	rmt_chn = chn;
	rmt_mem = RMT_MEM_NUM_BLOCKS_1;
	br = 255;
	led_rgb_data = (uint32_t *)malloc(ledCounts * sizeof(uint32_t));
	setLedType(t);
}

bool ESP32_WS2812::begin()
{	
	switch(rmt_chn){
		case 0:
			rmt_mem=RMT_MEM_NUM_BLOCKS_1;
			break;
		case 1:
			rmt_mem=RMT_MEM_NUM_BLOCKS_2;
			break;
		case 2:
		#if SOC_RMT_TX_CANDIDATES_PER_GROUP > 2
			rmt_mem=RMT_MEM_NUM_BLOCKS_3;
		#endif
			break;
		case 3:
		#if SOC_RMT_TX_CANDIDATES_PER_GROUP > 2
			rmt_mem=RMT_MEM_NUM_BLOCKS_4;
		#endif
			break;
		case 4:
		#if SOC_RMT_TX_CANDIDATES_PER_GROUP > 4
			rmt_mem=RMT_MEM_NUM_BLOCKS_5;
		#endif
			break;
		case 5:
		#if SOC_RMT_TX_CANDIDATES_PER_GROUP > 4
			rmt_mem=RMT_MEM_NUM_BLOCKS_6;
		#endif
			break;
		case 6:
		#if SOC_RMT_TX_CANDIDATES_PER_GROUP > 4
			rmt_mem=RMT_MEM_NUM_BLOCKS_7;
		#endif
			break;
		case 7:
		#if SOC_RMT_TX_CANDIDATES_PER_GROUP > 4
			rmt_mem=RMT_MEM_NUM_BLOCKS_8;
		#endif
			break;	
		default:
			rmt_mem=RMT_MEM_NUM_BLOCKS_1;break;
	}
	if(rmtInit(pin, RMT_TX_MODE, rmt_mem, 10000000)==false){
		return false;
	}
	for(int i=0;i<ledCounts;i++){
		led_rgb_data[i] = 0x0;
	}
	return true;
}

void ESP32_WS2812::setLedCount(uint16_t n)
{
	ledCounts = n;
	free(led_rgb_data);
	led_rgb_data = (uint32_t *)malloc(ledCounts * sizeof(uint32_t));
	begin();
}

void ESP32_WS2812::setLedType(LED_TYPE t)
{
	rOffset = (t >> 4) & 0x03;
	gOffset = (t >> 2) & 0x03;
	bOffset = t & 0x03;
}

void ESP32_WS2812::setBrightness(uint8_t brightness)
{
	br = constrain(brightness, 0, 255);
	show();
}

esp_err_t ESP32_WS2812::setLedColorData(int index, uint8_t r, uint8_t g, uint8_t b)
{
	uint32_t color = r << 16 | g << 8 | b ;
	return setLedColorData(index, color);
}

esp_err_t ESP32_WS2812::setLedColorData(int index, uint32_t rgb)
{
	led_rgb_data[index] = rgb;
	return ESP_OK;
}

esp_err_t ESP32_WS2812::setLedColor(int index, uint32_t rgb)
{
	return setLedColor(index, rgb >> 16, rgb >> 8, rgb);
}

esp_err_t ESP32_WS2812::setLedColor(int index, uint8_t r, uint8_t g, uint8_t b)
{
	setLedColorData(index, r, g, b);
	return show();
}

esp_err_t ESP32_WS2812::setAllLedsColorData(uint32_t rgb)
{
	for (int i = 0; i < ledCounts; i++)
	{
		setLedColorData(i, rgb);
	}
	return ESP_OK;
}

esp_err_t ESP32_WS2812::setAllLedsColorData(uint8_t r, uint8_t g, uint8_t b)
{
	for (int i = 0; i < ledCounts; i++)
	{
		setLedColorData(i, r, g, b);
	}
	return ESP_OK;
}

esp_err_t ESP32_WS2812::setAllLedsColor(uint32_t rgb)
{
	setAllLedsColorData(rgb);
	show();
	return ESP_OK;
}

esp_err_t ESP32_WS2812::setAllLedsColor(uint8_t r, uint8_t g, uint8_t b)
{
	setAllLedsColorData(r, g, b);
	show();
	return ESP_OK;
}

esp_err_t ESP32_WS2812::show()
{
	esp_err_t rmt_state;
	uint32_t color;
	uint8_t p[3];
	rmt_data_t *led_send_data = (rmt_data_t *)malloc(ledCounts * 24 * sizeof(rmt_data_t));
	for(int i = 0; i < ledCounts; i++){
		for (int bit = 0; bit < 24; bit++) {
			color = led_rgb_data[i];
			p[rOffset] = (uint8_t)((color >> 16 & 0xff) * br / 255);
			p[gOffset] = (uint8_t)((color >> 8 & 0xff) * br / 255);
			p[bOffset] = (uint8_t)((color & 0xff) * br / 255);
			color = p[0] << 16 | p[1] << 8 | p[2];
			if (color & (1 << (23-bit))) {
				led_send_data[i*24+bit].level0 = 1;
				led_send_data[i*24+bit].duration0 = 8;
				led_send_data[i*24+bit].level1 = 0;
				led_send_data[i*24+bit].duration1 = 4;
			} else {
				led_send_data[i*24+bit].level0 = 1;
				led_send_data[i*24+bit].duration0 = 4;
				led_send_data[i*24+bit].level1 = 0;
				led_send_data[i*24+bit].duration1 = 8;
			}
		}
	}
	rmt_state = rmtWrite(pin, led_send_data, ledCounts*24, RMT_WAIT_FOR_EVER);
	free(led_send_data);
	return rmt_state;
}

uint16_t ESP32_WS2812::getLedCount(void)
{
	return ledCounts;
}

uint8_t ESP32_WS2812::getBrightness(void)
{
	return br;
}

uint32_t ESP32_WS2812::getLedColorData(int index)
{
	return led_rgb_data[index];
}

uint32_t* ESP32_WS2812::getLedAllColorDate(void)
{
	return led_rgb_data;
}
	
uint32_t ESP32_WS2812::Wheel(byte pos)
{
	uint32_t WheelPos = pos % 0xff;
	if (WheelPos < 85) {
		return ((255 - WheelPos * 3) << 16) | ((WheelPos * 3) << 8);
	}
	if (WheelPos < 170) {
		WheelPos -= 85;
		return (((255 - WheelPos * 3) << 8) | (WheelPos * 3));
	}
	WheelPos -= 170;
	return ((WheelPos * 3) << 16 | (255 - WheelPos * 3));
}

uint32_t ESP32_WS2812::hsv2rgb(uint32_t h, uint32_t s, uint32_t v)
{
	uint8_t r, g, b;
	h %= 360; // h -> [0,360]
	uint32_t rgb_max = v * 2.55f;
	uint32_t rgb_min = rgb_max * (100 - s) / 100.0f;

	uint32_t i = h / 60;
	uint32_t diff = h % 60;

	// RGB adjustment amount by hue
	uint32_t rgb_adj = (rgb_max - rgb_min) * diff / 60;

	switch (i) {
	case 0:
		r = rgb_max;
		g = rgb_min + rgb_adj;
		b = rgb_min;
		break;
	case 1:
		r = rgb_max - rgb_adj;
		g = rgb_max;
		b = rgb_min;
		break;
	case 2:
		r = rgb_min;
		g = rgb_max;
		b = rgb_min + rgb_adj;
		break;
	case 3:
		r = rgb_min;
		g = rgb_max - rgb_adj;
		b = rgb_max;
		break;
	case 4:
		r = rgb_min + rgb_adj;
		g = rgb_min;
		b = rgb_max;
		break;
	default:
		r = rgb_max;
		g = rgb_min;
		b = rgb_max - rgb_adj;
		break;
	}
	return (uint32_t)(r << 16 | g << 8 | b);
}
