# ESP32简易电子鸡汤
![1](/img/eg.JPG)

本项目基于ESP32

使用Arduino作为编译器

使用一言api作为句子接口

使用ILI9341屏幕240x320

### 使用到的库

WiFi.h

HTTPClient.h

ArduinoJson.h

SPI.h

Adafruit_GFX.h

Adafruit_ILI9341.h

U8g2_for_Adafruit_GFX.h


### 注意

需进行如下配置

**工具——Rartition Scheme——No OTA (2MB APP/2MB SPIFFS)**

否则编译时会报**内存不足**

~~喜欢就给个星吧~~