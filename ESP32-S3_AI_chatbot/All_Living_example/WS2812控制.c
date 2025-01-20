#include "ESP32_WS2812_Lib.h"

#define LEDS_COUNT 1  // LED数量
#define LEDS_PIN   48  // 数据引脚
#define CHANNEL    0  // RMT通道
#define KEY 17  // 按键引脚

// 定义 strip 对象
ESP32_WS2812 strip = ESP32_WS2812(LEDS_COUNT, LEDS_PIN, CHANNEL, TYPE_GRB);

void LED_UP() {
  strip.begin();        // 初始化灯带
  strip.setBrightness(0);  // 设置亮度（0-255）
  pinMode(KEY, INPUT_PULLUP);  // 配置GPIO为输入模式，启用内部上拉电阻
  // 设置LED为白色
  strip.setLedColorData(0, 255, 255, 255);  // RGB值均为255表示白色
  strip.show();  // 更新LED显示
}

void setup() {
  Serial.begin(115200);
  LED_UP();
}

int lastKeyState = HIGH;  // 存储上一次按键状态
int currentKeyState;      // 存储当前按键状态
unsigned long lastDebounceTime = 0;  // 消抖计时器
unsigned long debounceDelay = 50;    // 消抖延时（毫秒）

void loop() {
  // 读取当前按键状态
  int reading = digitalRead(KEY);

  // 消抖逻辑
  if (reading != lastKeyState) {
    lastDebounceTime = millis();  // 重置消抖计时器
  }

  // 如果状态稳定超过消抖时间
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // 如果当前状态与之前存储的状态不同
    if (reading != currentKeyState) {
      currentKeyState = reading;

      // 检测按键按下（低电平）
      if (currentKeyState == LOW) {
        Serial.println("开");
        // 取消注释以下代码以控制LED
        strip.setBrightness(255);  // 设置亮度（0-255）
        strip.show();  // 更新LED显示
      } else {
        Serial.println("关");
        // 取消注释以下代码以控制LED
        strip.setBrightness(0);  // 设置亮度（0-255）
        strip.show();  // 更新LED显示
      }
    }
  }

  // 更新上一次按键状态
  lastKeyState = reading;
}