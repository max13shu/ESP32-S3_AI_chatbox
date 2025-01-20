#include <U8g2lib.h>
#include <Arduino_GFX_Library.h>
#include <SPI.h>

// Define the pins for your display
#define GFX_MOSI 15  // Data pin
#define GFX_SCLK 14  // Clock pin
#define GFX_CS   5   // Chip select pin
#define GFX_DC   27  // Data/Command pin
#define GFX_RST  33  // Reset pin
#define GFX_BL   22  // Backlight pin

//软实现
Arduino_DataBus *bus = new Arduino_SWSPI(GFX_DC /* DC */, GFX_CS /* CS */, GFX_SCLK /* SCK */, GFX_MOSI /* MOSI */, GFX_NOT_DEFINED /* MISO */);

// Set up the display (ST7735 in this case)
Arduino_GFX *gfx = new Arduino_ST7735(
  bus, GFX_RST /* RST */, 0 /* rotation */, false /* IPS */,
  128 /* width */, 160 /* height */,
  2 /* col offset 1 */, 1 /* row offset 1 */,
  2 /* col offset 2 */, 1 /* row offset 2 */,
  false /* BGR */);

void setup(void)
{
  Serial.begin(115200);
  Serial.println("Arduino_GFX U8g2 Font UTF8 Chinese example");

  // Init Display
  if (!gfx->begin())
  {
    Serial.println("gfx->begin() failed!");
  }
  
  // Turn on backlight
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);  // Turn on the backlight

  gfx->fillScreen(RGB565_BLACK);
  gfx->setUTF8Print(true); // enable UTF8 support for the Arduino print() function

  gfx->setFont(u8g2_font_unifont_t_chinese4);  // Set Chinese font
  gfx->setTextColor(RGB565_YELLOW);
  gfx->setCursor(0, 16);
  gfx->println("Arduino 是一個開源嵌入式硬體平台，用來供用戶製作可互動式的嵌入式專案。此外 Arduino 作為一個開源硬體和開源軟件的公司，同時兼有專案和用戶社群。該公司負責設計和製造Arduino電路板及相關附件。這些產品按照GNU寬通用公共許可證（LGPL）或GNU通用公共許可證（GPL）[1]許可的開源硬體和軟件分發的，Arduino 允許任何人製造 Arduino 板和軟件分發。 Arduino 板可以以預裝的形式商業銷售，也可以作為DIY套件購買。");
  gfx->println("Arduino 专案始于2003年，作为意大利伊夫雷亚地区伊夫雷亚互动设计研究所的学生专案，目的是为新手和专业人员提供一种低成本且简单的方法，以建立使用感测器与环境相互作用的装置执行器。适用于初学者爱好者的此类装置的常见范例包括感测器、简单机械人、恒温器和运动检测器。");

  Serial.print("OK");
}

void loop()
{
  // Nothing needed in loop
}
