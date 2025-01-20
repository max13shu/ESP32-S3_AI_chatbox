#include <U8g2lib.h>
#include <Arduino_GFX_Library.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <SPI.h>
#include <WiFi.h>

// Define the pins for your display
#define GFX_MOSI 5  // Data pin
#define GFX_SCLK 4  // Clock pin
#define GFX_CS   15 // Chip select pin
#define GFX_DC   7  // Data/Command pin
#define GFX_RST  6  // Reset pin
#define GFX_BL   16 // Backlight pin

// 设置WiFi
const char* ssid = "wsyzlmz2";
const char* password = "qwerasdf12345";

// 豆包 API key
const char* doubao_apiKey = "77a5806a-7bb4-4c96-a500-9c7b8a731ac7";

// 豆包 API
String inputText = "你好，豆包！";
String apiUrl = "https://ark.cn-beijing.volces.com/api/v3/chat/completions";

String answer;

// 获取豆包的回复
String getGPTAnswer(String inputText) {
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
	if (httpResponseCode == 200) {
		String response = http.getString();
		http.end();
		Serial.println("API Response: " + response); // 打印 API 返回的 JSON 数据
		
		// 使用 Arduino_JSON 解析 JSON 响应
		JSONVar jsonDoc = JSON.parse(response);
		if (JSON.typeof(jsonDoc) == "undefined") {
			Serial.println("Failed to parse JSON!");
			return "<error>";
		}
		
		// 打印解析后的 JSON 数据
		Serial.println("Parsed JSON: " + JSON.stringify(jsonDoc));
		
		// 提取回复内容
		String outputText = jsonDoc["choices"][0]["message"]["content"];
		return outputText;
	} else {
		http.end();
		Serial.printf("Error %i \n", httpResponseCode);
		return "<error>";
	}
}

// 软实现
Arduino_DataBus *bus = new Arduino_SWSPI(GFX_DC /* DC */, GFX_CS /* CS */, GFX_SCLK /* SCK */, GFX_MOSI /* MOSI */, GFX_NOT_DEFINED /* MISO */);

// Set up the display (ST7735 in this case)
Arduino_GFX *gfx = new Arduino_ST7735(
	bus, GFX_RST /* RST */, 0 /* rotation */, false /* IPS */,
	128 /* width */, 160 /* height */,
	2 /* col offset 1 */, 1 /* row offset 1 */,
	2 /* col offset 2 */, 1 /* row offset 2 */,
	true /* BGR */);

// 初始化 WiFi
void WiFi_UP() {
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	Serial.print("Connecting to WiFi ..\r\n");
	while (WiFi.status() != WL_CONNECTED) {
		Serial.print('.');
		delay(1000);
	}
	Serial.println("Connected to WiFi");
	Serial.println("IP Address: " + WiFi.localIP().toString());
}

// 初始化显示屏
void GFX_UP() {
	Serial.println("Initializing display...");
	if (!gfx->begin()) {
		Serial.println("gfx->begin() failed!");
		return;
	}
	gfx->fillScreen(RGB565_BLACK);
	gfx->setUTF8Print(true);    // 开启 setUTF8Print
	gfx->setFont(u8g2_font_unifont_t_chinese4);  // 设置 u8g2_font_unifont_t_chinese4
	Serial.println("Display initialized successfully.");
}

void setup(void) {
	// 设置频率 115200
	Serial.begin(115200);
	WiFi_UP();
	GFX_UP();
	
	// 获取豆包的回复
	answer = getGPTAnswer(inputText);
	gfx->setCursor(0, 16);  // x,y
	gfx->setTextColor(YELLOW);  
	gfx->println("豆包: " + answer);
}

void loop() {
	if (Serial.available()) {
		inputText = Serial.readStringUntil('\r');  // \r 表示结束符为回车符
		gfx->setCursor(0, 16);  // x,y
		gfx->fillScreen(BLACK);  // 底色
		gfx->setTextColor(CYAN);
		gfx->println("我:" + inputText);
		
		// 获取豆包的回复
		answer = getGPTAnswer(inputText);
		gfx->setTextColor(YELLOW);
		gfx->println("豆包: " + answer);
	}
}
