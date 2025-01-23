#include <WiFi.h>
#include <U8g2lib.h>
#include <Arduino_GFX_Library.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Arduino_JSON.h>
#include <esp_heap_caps.h>
#include "arduino_base64.hpp"
#include "ESP32_WS2812_Lib.h"
#include <driver/i2s.h>

// GFX 引脚设置
#define GFX_MOSI 5 
#define GFX_SCLK 4 
#define GFX_CS 15  
#define GFX_DC 7  
#define GFX_RST 6  
#define GFX_BL 16  

// WS2812灯
#define LEDS_COUNT 1 // 只需要1个LED
#define LEDS_PIN 48	 // 数据引脚
#define CHANNEL 0	 // RMT通道
#define KEY 17		 // LED开关

// I2S引脚设置
#define I2S_WS 19
#define I2S_SD 18 
#define I2S_SCK 20

#define BLOCK_SIZE 1024 // 每个DMA缓冲区的样本数

// 软实现
Arduino_DataBus *bus = new Arduino_SWSPI(GFX_DC /* DC */, GFX_CS /* CS */, GFX_SCLK /* SCK */, GFX_MOSI /* MOSI */, GFX_NOT_DEFINED /* MISO */);

// Set up the display (ST7735 in this case)
Arduino_GFX *gfx = new Arduino_ST7735(
	bus, GFX_RST /* RST */, 0 /* rotation */, false /* IPS */,
	128 /* width */, 160 /* height */,
	2 /* col offset 1 */, 1 /* row offset 1 */,
	2 /* col offset 2 */, 1 /* row offset 2 */,
	true /* BGR */);

// WIFI设置
const char *ssid = "  ";  //你的WIFI账号
const char *password = "  "; //你的WIFI密码
// const char* ssid = "HW-GenKi";
// const char* password = "3.1415926";

// 百度 AIP key
const String API_KEY = "  ";  
const String SECRET_KEY = " ";

// 豆包 API key
const char *doubao_apiKey = " ";

// 豆包 API
String inputText = "你好，豆包！";
String apiUrl = "https://ark.cn-beijing.volces.com/api/v3/chat/completions";

String answer;

// 获取豆包的回复
String getGPTAnswer(String inputText)
{
	HTTPClient http;
	http.setTimeout(20000);
	http.begin(apiUrl);
	http.addHeader("Content-Type", "application/json");
	String token_key = String("Bearer ") + doubao_apiKey;
	http.addHeader("Authorization", token_key);
	String payload = "{\"model\":\"ep-20250105145908-gq94r\",\"messages\":[{\"role\":\"system\",\"content\":\"我是你的主人,回复我消息只能用30字以内\"},{\"role\":\"user\",\"content\":\"豆包" + inputText + "\"}],\"temperature\": 0.3}";
	
	Serial.println("Sending request to Doubao API...");
	Serial.println("Payload: " + payload);
	
	int httpResponseCode = http.POST(payload);
	if (httpResponseCode == 200)
	{
		String response = http.getString();
		http.end();
		Serial.println("API Response: " + response); // 打印 API 返回的 JSON 数据
		
		// 使用 Arduino_JSON 解析 JSON 响应
		JSONVar jsonDoc = JSON.parse(response);
		if (JSON.typeof(jsonDoc) == "undefined")
		{
			Serial.println("Failed to parse JSON!");
			return "<error>";
		}
		
		// 打印解析后的 JSON 数据
		Serial.println("Parsed JSON: " + JSON.stringify(jsonDoc));
		
		// 提取回复内容
		String outputText = jsonDoc["choices"][0]["message"]["content"];
		return outputText;
	}
	else
	{
		http.end();
		Serial.printf("Error %i \n", httpResponseCode);
		return "<error>";
	}
}

// 初始化 WiFi
void WiFi_UP()
{
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	Serial.print("Connecting to WiFi ..\r\n");
	while (WiFi.status() != WL_CONNECTED)
	{
		Serial.print('.');
		delay(1000);
	}
	Serial.println("Connected to WiFi");
	Serial.println("IP Address: " + WiFi.localIP().toString());
}

// 定义 strip 对象
ESP32_WS2812 strip = ESP32_WS2812(LEDS_COUNT, LEDS_PIN, CHANNEL, TYPE_GRB);

void LED_UP()
{
	strip.begin();				// 初始化灯带
	strip.setBrightness(50);		// 设置亮度（0-255）
	pinMode(KEY, INPUT_PULLUP); // 配置GPIO为输入模式，启用内部上拉电阻
}

// 初始化显示屏
void GFX_UP()
{
	Serial.println("Initializing display...");
	if (!gfx->begin())
	{
		Serial.println("gfx->begin() failed!");
		return;
	}
	gfx->fillScreen(RGB565_BLACK);
	gfx->setUTF8Print(true);					// 开启 setUTF8Print
	gfx->setFont(u8g2_font_unifont_t_chinese4); // 设置 u8g2_font_unifont_t_chinese4
	Serial.println("Display initialized successfully.");
}

String getAccessToken()
{
	WiFiClientSecure client;
	HTTPClient http;
	
	String url = "https://aip.baidubce.com/oauth/2.0/token";
	String params = "grant_type=client_credentials&client_id=" + API_KEY + "&client_secret=" + SECRET_KEY;
	
	client.setInsecure(); // 忽略SSL证书验证
	
	if (http.begin(client, url))
	{
		http.addHeader("Content-Type", "application/x-www-form-urlencoded");
		int httpCode = http.POST(params);
		
		if (httpCode == HTTP_CODE_OK)
		{
			String payload = http.getString();
			JSONVar doc = JSON.parse(payload);
			return doc["access_token"];
		}
		else
		{
			Serial.printf("[HTTPS] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
		}
		http.end();
	}
	else
	{
		Serial.printf("[HTTPS] Unable to connect\n");
	}
	return "";
}

// 初始化I2S
void I2S_UP()
{
	const i2s_config_t i2s_config = {
		.mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
		.sample_rate = 16000,
		.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
		.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
		.communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
		.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // default interrupt priority
		.dma_buf_count = 8,
		.dma_buf_len = BLOCK_SIZE,
		.use_apll = false
	};
	const i2s_pin_config_t pin_config = {
		.bck_io_num = I2S_SCK,	// BCKL
		.ws_io_num = I2S_WS,	// LRCL
		.data_out_num = -1, // 未使用（仅用于扬声器）
		.data_in_num = I2S_SD	// DOUT
	};
	
	i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL); // 安装I2S驱动
	i2s_set_pin(I2S_NUM_0, &pin_config);				 // 设置I2S引脚
}


// array size is 129600
#define SPEECH_MAX_LEN 129600
unsigned char speech[SPEECH_MAX_LEN];

// flag = 0, 1, 2, 3
uint32_t speech_count = 0;

// 读取I2S数据
int readI2SData()
{
	int16_t buffer[BLOCK_SIZE];
	size_t bytes_read = 0;
	float mean = 0;
	
	// 从I2S读取数据
	i2s_read(I2S_NUM_0, buffer, sizeof(buffer), &bytes_read, portMAX_DELAY);
	
	// 判断是否超出speech数组的长度
	if (speech_count + bytes_read > SPEECH_MAX_LEN)
	{
		Serial.println("Speech buffer overflow!");
		return -1;
	}
	
	// 将buffer拷贝数据到speech数组
	memcpy(speech + speech_count, buffer, bytes_read);
	// 更新speech_count
	speech_count += bytes_read ;
	return 0;
}

// 按键检测
bool checkButtonPress()
{
	static int lastKeyState = HIGH;			   // 上一次按键状态
	static unsigned long lastDebounceTime = 0; // 消抖计时器
	const unsigned long debounceDelay = 50;	   // 消抖延时（毫秒）
	
	int reading = digitalRead(KEY); // 读取当前按键状态
	
	// 消抖逻辑
	if (reading != lastKeyState)
	{
		lastDebounceTime = millis(); // 重置消抖计时器
	}
	
	if ((millis() - lastDebounceTime) > debounceDelay)
	{
		if (reading == LOW)
		{ // 按键按下（低电平）
			return true;
		}
	}
	
	lastKeyState = reading;
	return false;
}

size_t speechLength = 129600; // 原始数据长度
String accessToken;
void setup()
{
	Serial.begin(115200);
	WiFi_UP();
	GFX_UP();
	LED_UP();
	I2S_UP();
	
	if (!psramFound())
	{
		Serial.println("PSRAM is not available or not enabled.");
		return;
	}
	
	// 获取百度语音识别的访问令牌
	accessToken = getAccessToken();
	if (accessToken == "")
	{
		Serial.println("Failed to get access token.");
		return;
	}
	Serial.println("Access Token: " + accessToken);
	
}

uint8_t record_flag = 0;

void send_record()
{
	// 發送數據
	
	WiFiClientSecure client;
	HTTPClient http;
	String url = "https://vop.baidu.com/server_api";
	size_t outputSize = base64::encodeLength(speech_count); // Base64 编码后长度
	char *output = (char *)ps_malloc(outputSize + 1);		// 分配 PSRAM 内存
	if (output == NULL)
	{
		Serial.println("Failed to allocate PSRAM!");
		return;
	}
	base64::encode(speech, speech_count, output);
	output[outputSize] = '\0'; // 手动添加字符串结束符
	
	JSONVar doc;
	doc["format"] = "pcm";
	doc["rate"] = 16000;
	doc["channel"] = 1;
	doc["cuid"] = "ntPmjqtUSVKeT8Pp09H4g3QonmElHEj8";
	doc["speech"] = output;
	doc["len"] = speech_count;
	doc["token"] = accessToken;
	
	String payload = JSON.stringify(doc);
	if (payload.length() == 0)
	{
		Serial.println("Failed to serialize JSON!");
		free(output);
		return;
	}
	
	client.setInsecure(); // 忽略SSL证书验证
	if (http.begin(client, url))
	{
		http.addHeader("Content-Type", "application/json");
		http.addHeader("Accept", "application/json");
		int httpCode = http.POST(payload);
		
		if (httpCode == HTTP_CODE_OK)
		{
			String response = http.getString();
			Serial.println("Response: " + response);
			
			// 解析 JSON
			JSONVar jsonDoc = JSON.parse(response);
			if (JSON.typeof(jsonDoc) == "undefined")
			{
				Serial.println("Failed to parse JSON!");
				free(output);
				return;
			}
			
			// 检查是否有错误
			if (jsonDoc.hasOwnProperty("err_no") && int(jsonDoc["err_no"]) != 0)
			{
				Serial.println("Error: " + String(jsonDoc["err_msg"]));
				free(output);
				return;
			}
			
			// 提取语音识别结果
			if (jsonDoc.hasOwnProperty("result"))
			{
				String result = jsonDoc["result"][0]; // 获取第一个识别结果
				Serial.println("Speech Recognition Result: " + result);
				gfx->setCursor(0, 16);	// x,y
				gfx->fillScreen(BLACK); // 底色
				gfx->setTextColor(CYAN);
				inputText = result;
				gfx->println("我:" + result);
				// 获取豆包的回复
				answer = getGPTAnswer(inputText);
				gfx->setTextColor(YELLOW);
				gfx->println("豆包: " + answer);
			}
			else
			{
				Serial.println("No result found in response!");
			}
		}
		else
		{
			Serial.printf("[HTTPS] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
		}
		http.end();
	}
	else
	{
		Serial.printf("[HTTPS] Unable to connect\n");
	}
	
	// 釋放數據
	free(output); // 释放 PSRAM 内存
}

void loop()
{
	if (checkButtonPress())
	{ // 检测按键按下
	  strip.setLedColorData(0, 255, 0, 0); // 红
		strip.show();			  // 更新LED显示
		if (record_flag == 0)
		{
			record_flag = 1;
			speech_count = 0;
			// 開始記錄數據
	
		}
		
		int rst = readI2SData(); // 读取I2S数据
	}
	else
	{
    strip.setLedColorData(0, 0, 255, 0); // 绿
		strip.show();			// 更新LED显示
		if (record_flag == 1)
		{
			record_flag = 0;
			// 數據發送
			send_record();
			// 清理數據個數
			Serial.printf("count: %d\r\n", speech_count);
			
			speech_count = 0;
		}
	}
}