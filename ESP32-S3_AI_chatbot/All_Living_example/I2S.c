#include <driver/i2s.h>
#define KEY      17 // LED开关
#define LED      35//LED

#define I2S_WS 19
#define I2S_SD 18 
#define I2S_SCK 20
#define I2S_PORT I2S_NUM_0
#define bufferLen 64

int16_t sBuffer[bufferLen];

void i2s_install() {
	const i2s_config_t i2s_config = {
		.mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
		.sample_rate = 44100,
		.bits_per_sample = i2s_bits_per_sample_t(16),
		.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
		.communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
		.intr_alloc_flags = 0, // default interrupt priority
		.dma_buf_count = 8,
		.dma_buf_len = bufferLen,
		.use_apll = false
	};
	
	i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
}

void i2s_setpin() {
	const i2s_pin_config_t pin_config = {
		.bck_io_num = I2S_SCK,
		.ws_io_num = I2S_WS,
		.data_out_num = -1,
		.data_in_num = I2S_SD
	};
	
	i2s_set_pin(I2S_PORT, &pin_config);
}

void I2S_UP(){
	i2s_install();
	i2s_setpin();
	i2s_start(I2S_PORT);
}

void I2S_READ() {
	size_t bytesIn = 0;
	esp_err_t result = i2s_read(I2S_PORT, &sBuffer, bufferLen, &bytesIn, portMAX_DELAY);
	if (result == ESP_OK) {
		int samples_read = bytesIn / 8;
		if (samples_read > 0) {
			float mean = 0;
			for (int i = 0; i < samples_read; ++i) {
				mean += (sBuffer[i]);
			}
			mean /= samples_read;
			Serial.println(mean);
		}
	}
}


void LED_UP(){
	pinMode(KEY, INPUT_PULLUP);  // 配置GPIO为输入模式，启用内部上拉电阻
	pinMode(LED, OUTPUT); //设置LED引脚（35） 为输出模式
}


void setup() {
	Serial.begin(115200);
	LED_UP();
	I2S_UP();
	
}

// 标志位
bool status = false;

void loop() {
	if (digitalRead(KEY) == 1 && status == true) {
		// 按键松开，关闭 LED 并停止读取
		Serial.printf("关\r\n");
		digitalWrite(LED, 0);
		status = false;
	} else if (digitalRead(KEY) == 0 && status == false) {
		// 按键按下，打开 LED 并持续读取 I2S 数据
		Serial.printf("开\r\n");
		digitalWrite(LED, 1);
		status = true;
		
		// 持续读取 I2S 数据，直到按键松开
		while (digitalRead(KEY) == 0) {
			I2S_READ();
		}
	}
}
