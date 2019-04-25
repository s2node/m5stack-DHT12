
/*
M5Stack で DHT12を表示しつつ Ambient にデータを送信する

ToDo:

□目盛りの数値描画
□ボタンで表示モード切替
□ボタンでメーターの上下とゼロリセット
□ボタンでスケール変更
☑温度アナログメータ
□温度によって色を変える。
□湿度アナログメータ
□湿度バーのメータ
□横軸を時間でグラフ表示を出す
☑WiFi接続
☑Ambientデータ送信

*/

// https://github.com/AmbientDataInc/Ambient_ESP8266_lib
#include <Ambient.h>

#include <WiFi.h>
#include <M5Stack.h>

// https://github.com/m5stack/M5Stack/blob/master/examples/Modules/DHT12/DHT12.ino
#include "DHT12.h"
#include <Wire.h>     //The DHT12 uses I2C comunication.
DHT12 dht12;          //Preset scale CELSIUS and ID 0x5c.
TFT_eSprite img = TFT_eSprite(&M5.Lcd);

#include "WiFiSSID.h"
const char* ssid     = MY_WIFI_SSID;          // SSID は WiFiSSID.h に書く： #define MY_WIFI_SSID "ABCD" 
const char* password = MY_WIFI_PASSWORD;      // パスワード は WiFiSSID.h  に書く： #define MY_WIFI_PASSWORD "ABCD" 

Ambient ambient;
WiFiClient ambientWiFiClient;
unsigned int ambientChannelId = AMBIENT_CHANNEL_ID;   // WiFiSSID.h に書く：#define AMBIENT_CHANNEL_ID 10000000
const char* ambientWriteKey  = AMBINET_WRITE_KEY;     // WiFiSSID.h に書く：#define AMBINET_WRITE_KEY "abcd12345"

int uploadIntervalCounter = 0; // Ambient へアップロードするインターバルを管理する変数。12回に1回だけアップロードするとか。
// 1分毎にアップロード
//#define UPLOAD_INTERVAL_COUNT 12 
// 5分毎にアップロード
#define UPLOAD_INTERVAL_COUNT (12*5) 

// WiFi接続確認とAmbientへの接続
bool IsWiFiConnected()
{
  static bool bConnected = false;

  if(bConnected) {
    if (WiFi.status() != WL_CONNECTED) {
      bConnected = false;
    }
  }else {
    if (WiFi.status() == WL_CONNECTED) {
      bConnected = true;
      ambient.begin(ambientChannelId, ambientWriteKey, &ambientWiFiClient);
    }
  }
  return bConnected;
}

void setup() {
  M5.begin();
  Wire.begin();
  img.setColorDepth(8);
  img.createSprite(M5.Lcd.width(), M5.Lcd.height());  
  img.fillSprite(TFT_BLUE);
  img.pushSprite(0, 0);
  
  M5.Lcd.setBrightness(60);
  WiFi.begin(ssid, password);
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
    const int16_t max_angle=140;  // メモリの角度範囲　140度

    int16_t i;
    int16_t cx,cy, r; // 温度計針とメモリの中央の座標とメモリの大きさ
    cx = M5.Lcd.width()/2;
    cy = M5.Lcd.height()/2;
    r = cy*7/8;

    // 温度計の目盛り 15個　温度1度単位
    for(i = 0; i <= max_angle; i+=10) {
      img.drawLine(
        rl2x(i-max_angle-(180-max_angle)/2, r*4/5, cx,cy), rl2y(i-max_angle-(180-max_angle)/2, r*4/5, cx,cy),
        rl2x(i-max_angle-(180-max_angle)/2, r, cx,cy), rl2y(i-max_angle-(180-max_angle)/2, r, cx,cy),
        TFT_WHITE);
    }

    // 角度 -max_angle-(180-max_angle)/2 は 　　0度の角度のを補正している。補正量0で→。補正量90度で↓，補正量180度で←。補正量270度↑
    // max_angleは140度。とりあえずメータのメモリ幅分140度分右に回すことで0度の位置を目盛りの左端。
    // 目盛り自体のゼロ位置が少し傾いているのでその分を計算しているのが -(180-max_angle)/2 。180-140=40度。左右のそれぞれの目盛りが水平から傾いている
    // 分の合計なので半分にして　40度÷2で20度の右にまわす。
    // 右向きに 140+20で160度回している。これで，左端のメモリ


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


  float nowTemperature, nowHumidity;

  nowTemperature = dht12.readTemperature();
  nowHumidity = dht12.readHumidity();

/*
  // 中央に２行
  {
    char buf[20];
    sprintf(buf, "%.1fC", nowTemperature);
    img.drawString(buf, M5.Lcd.width()/2, M5.Lcd.height()/3, 1);
    sprintf(buf, "%.1f%%", nowHumidity);
    img.drawString(buf, M5.Lcd.width()/2, M5.Lcd.height()*2/3, 1);
  }
*/

  // したの方に2行  
  {
    char buf[20];
    sprintf(buf, "%.1fC", nowTemperature);
    img.drawString(buf, M5.Lcd.width()/2, M5.Lcd.height()*3/5, 1);
    sprintf(buf, "%.1f%%", nowHumidity);
    img.drawString(buf, M5.Lcd.width()/2, M5.Lcd.height()*5/6, 1);
  
  }

/*
  // 一番下に1行
  {
    char buf[20];
    sprintf(buf, "%.1fC %.1f%%", nowTemperature, nowHumidity);
    img.drawString(buf, M5.Lcd.width()/2, M5.Lcd.height()*4/5, 1);
  }
*/

  img.pushSprite(0, 0);
  delay(2500);
  // 描画とWiFi通信を同時にすると，描画が遅くなってチカチカするので，時間差(2.5秒差)で処理
  uploadIntervalCounter++;
  if(uploadIntervalCounter >= UPLOAD_INTERVAL_COUNT) { 
    if(IsWiFiConnected()) {
      ambient.set(1, nowTemperature); 
      ambient.set(2, nowHumidity);
      ambient.send();
      uploadIntervalCounter = 0;
    }
  }
  delay(2500);
}
