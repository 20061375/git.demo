#include <Arduino.h>
#include <U8g2lib.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0, 21, 22, U8X8_PIN_NONE);// put function declarations here:


void setup() {
  oled.begin();
  oled.clearBuffer();
  oled.enableUTF8Print();
  oled.setFont(u8g2_font_10x20_me);
 
  
}

void loop() {
  oled.clearBuffer();
    oled.setCursor(0, 20);
 oled.print("Hello World!");
 oled.sendBuffer();// put your main code here, to run repeatedly:
}

// put function definitions here:

