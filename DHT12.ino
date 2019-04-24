#include <M5Stack.h>
#include "DHT12.h"
#include <Wire.h>     //The DHT12 uses I2C comunication.
DHT12 dht12;          //Preset scale CELSIUS and ID 0x5c.

TFT_eSprite img = TFT_eSprite(&M5.Lcd);

void setup() {
  M5.begin();
  Wire.begin();
  img.setColorDepth(8);
  img.createSprite(M5.Lcd.width(), M5.Lcd.height());  
  img.fillSprite(TFT_BLUE);
  img.pushSprite(0, 0);
  
  M5.Lcd.setBrightness(60);
}

inline
int16_t rl2x(double deg, double r, int x, int y) {
  return (int16_t)( cos(deg/180.0*PI) * r + x );
}
inline
int16_t rl2y(double deg, double r, int x, int y) {
  return (int16_t)( sin(deg/180.0*PI) * r + y );
}

void loop() {
  img.fillScreen(TFT_BLUE);

  // 温度計のメモリと針
  {
    int16_t i;
    int16_t cx,cy, r; // 温度計針とメモリの中央の座標とメモリの大きさr
    cx = M5.Lcd.width()/2;
    cy = M5.Lcd.height()/2;
    r = cy*7/8;
    int16_t max_angle=140;  // メモリの角度範囲　140度

    // 温度計の目盛り 15個　1度単位
    for(i = 0; i <= max_angle; i+=10) {
      img.drawLine(
        rl2x(i-max_angle-(180-max_angle)/2, r*4/5, cx,cy), rl2y(i-max_angle-(180-max_angle)/2, r*4/5, cx,cy),
        rl2x(i-max_angle-(180-max_angle)/2, r, cx,cy), rl2y(i-max_angle-(180-max_angle)/2, r, cx,cy),
        TFT_WHITE);
    }


    // 温度計の針を描画
    double angle = dht12.readTemperature()-16.0;  // 16度左端。23度が真上。30度が右端
    angle*=10.0;

      // とがった三角形の針
      img.fillTriangle(
        rl2x(angle+90-max_angle-(180-max_angle)/2, r/20, cx,cy), rl2y(angle+90-max_angle-(180-max_angle)/2, r/20, cx,cy),

        rl2x(angle-max_angle-(180-max_angle)/2, r, cx,cy), rl2y(angle-max_angle-(180-max_angle)/2, r, cx,cy),
        rl2x(angle-90-max_angle-(180-max_angle)/2, r/20, cx,cy), rl2y(angle-90-max_angle-(180-max_angle)/2, r/20, cx,cy),
        TFT_YELLOW);

      // 細い針
      //img.drawLine(
      //  cx,cy,
      //  rl2x(angle-max_angle-(180-max_angle)/2, r, cx,cy), rl2y(angle-max_angle-(180-max_angle)/2, r, cx,cy),
      //  TFT_WHITE);
  }
  
  img.setTextDatum(MC_DATUM);
  img.setTextColor(TFT_WHITE, TFT_BLUE);
  img.setFreeFont(&FreeSans24pt7b);

/*
  // 中央に２行
  {
    char buf[20];
    sprintf(buf, "%.1fC", dht12.readTemperature());
    img.drawString(buf, M5.Lcd.width()/2, M5.Lcd.height()/3, 1);
    sprintf(buf, "%.1f%%", dht12.readHumidity());
    img.drawString(buf, M5.Lcd.width()/2, M5.Lcd.height()*2/3, 1);
  }
*/

  // したの方に2行  
  {
    char buf[20];
    sprintf(buf, "%.1fC", dht12.readTemperature());
    img.drawString(buf, M5.Lcd.width()/2, M5.Lcd.height()*3/5, 1);
    sprintf(buf, "%.1f%%", dht12.readHumidity());
    img.drawString(buf, M5.Lcd.width()/2, M5.Lcd.height()*5/6, 1);
  
  }

/*
  // 一番下に1行
  {
    char buf[20];
    sprintf(buf, "%.1fC %.1f%%", dht12.readTemperature(), dht12.readHumidity());
    img.drawString(buf, M5.Lcd.width()/2, M5.Lcd.height()*4/5, 1);
  }
*/

  img.pushSprite(0, 0);
  delay(5000);
}
