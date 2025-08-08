#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <U8g2_for_Adafruit_GFX.h>

#define TFT_BL 21//背光引脚

Adafruit_ILI9341 tft = Adafruit_ILI9341(15, 2, 13, 14, -1, 12);//TFT引进对应（cs, dc, mosi, clk, rst, miso）
const char* ssid = "***";//WiFi名称
const char* password = "***";//WiFi密码

U8G2_FOR_ADAFRUIT_GFX u8g2;

typedef struct {
  char hitokoto[256];
  char from[32];
  char who[32];
} sentences;

sentences Hitokoto;

// WiFi连接
bool connectWiFi(unsigned long timeout = 20000) {
  Serial.print("Connecting to WiFi");
  WiFi.disconnect(true);
  WiFi.begin(ssid, password);
  
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < timeout) {
    delay(500);
    Serial.print(".");
    yield();  // 喂看门狗
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nConnection failed!");
    return false;
  }
  
  Serial.println("\nConnected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  return true;
}

void get_Hitokoto() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.setConnectTimeout(10000);  // 设置超时
  http.begin("https://v1.hitokoto.cn/");
  
  int httpCode = http.GET();
  if (httpCode <= 0) {
    Serial.printf("HTTP error: %s\n", http.errorToString(httpCode).c_str());
    http.end();
    return;
  }

  const size_t capacity = JSON_OBJECT_SIZE(5) + 768;
  DynamicJsonDocument doc(capacity);
  
  DeserializationError error = deserializeJson(doc, http.getString());
  http.end();

  if (error) {
    Serial.print("JSON error: ");
    Serial.println(error.c_str());
    return;
  }

  // 复制数据
  strlcpy(Hitokoto.hitokoto, doc["hitokoto"] | "获取失败", sizeof(Hitokoto.hitokoto));
  strlcpy(Hitokoto.from, doc["from"] | "未知出处", sizeof(Hitokoto.from));
  strlcpy(Hitokoto.who, doc["from_who"] | "未知作者", sizeof(Hitokoto.who));

  // 显示内容
  tft.fillScreen(ILI9341_BLACK);
  u8g2.setCursor(0, 20);
  u8g2.print(Hitokoto.hitokoto);
  u8g2.print("\n\n出处：");
  u8g2.print(Hitokoto.from);
  u8g2.print("\n\n作者：");
  u8g2.print(Hitokoto.who);
}

void setup() {
  Serial.begin(115200);
  
  // 初始化显示屏
  tft.begin();
  u8g2.begin(tft);
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  tft.setRotation(1);
  u8g2.setFontMode(1);
  u8g2.setFont(u8g2_font_wqy14_t_gb2312);
  u8g2.setForegroundColor(ILI9341_WHITE);
  u8g2.setBackgroundColor(ILI9341_BLACK);
  tft.fillScreen(ILI9341_BLACK);

  // 显示初始信息
  u8g2.setCursor(0, 20);
  u8g2.print("启动中...");

  // 非阻塞连接WiFi
  if (!connectWiFi()) {
    u8g2.print("\nWiFi连接失败!");
    delay(3000);
    ESP.restart();  // 重启设备
  }

  // 获取内容
  get_Hitokoto();
}

void loop() {
  // 每10分钟更新一次
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 10 * 60 * 1000) {
    get_Hitokoto();
    lastUpdate = millis();
  }
  delay(100);
}
