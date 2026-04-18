#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

class OLEDDisplay {
public:
  OLEDDisplay(int sdaPin, int sclPin);
  bool begin();
  void clear();
  void showWelcome();
  void showNetworkInfo(String ip, String ssid, int rssi);
  void showDeviceList(int selectedIndex);
  void showWakeStatus(String deviceName, bool success);
  void showMessage(String line1, String line2 = "", String line3 = "", String line4 = "");
  
private:
  Adafruit_SSD1306 display;
  void drawHeader(String title);
};

#endif