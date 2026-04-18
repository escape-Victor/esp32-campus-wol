#include "oled_display.h"

OLEDDisplay::OLEDDisplay(int sdaPin, int sclPin) 
  : display(128, 64, &Wire, -1) {
  Wire.begin(sdaPin, sclPin);
}

bool OLEDDisplay::begin() {
  return display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
}

void OLEDDisplay::clear() {
  display.clearDisplay();
}

void OLEDDisplay::showWelcome() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 10);
  display.println("WOL");
  display.setCursor(10, 30);
  display.println("Controller");
  display.display();
  delay(2000);
}

void OLEDDisplay::showNetworkInfo(String ip, String ssid, int rssi) {
  display.clearDisplay();
  drawHeader("net status");
  
  display.setTextSize(1);
  display.setCursor(0, 10);
  display.print("SSID: ");
  display.println(ssid);
  
  display.setCursor(0, 20);
  display.print("IP: ");
  display.println(ip);
  
  display.setCursor(0, 30);
  display.print("sign: ");
  display.print(rssi);
  display.println(" dBm");
  
  display.setCursor(0, 40);
  display.print("dev num: ");
  display.print(DEVICE_COUNT);
  
  display.display();
}

void OLEDDisplay::showDeviceList(int selectedIndex) {
  display.clearDisplay();
  drawHeader("Select Device");
  
  display.setTextSize(1);
  for(int i = 0; i < min(3, DEVICE_COUNT); i++) {
    int deviceIdx = (selectedIndex + i) % DEVICE_COUNT;
    int yPos = 12 + i * 15;
    
    if(i == 0) {
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }
    
    display.setCursor(5, yPos);
    display.print("> ");
    display.println(devices[deviceIdx].name);  // 移除 ::
  }
  
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 55);
  display.print(selectedIndex + 1);
  display.print("/");
  display.print(DEVICE_COUNT);
  
  display.display();
}

void OLEDDisplay::showWakeStatus(String deviceName, bool success) {
  display.clearDisplay();
  drawHeader("wake status");
  
  display.setTextSize(1);
  display.setCursor(0, 15);
  display.print("device: ");
  display.println(deviceName);
  
  display.setCursor(0, 30);
  if(success) {
    display.setTextColor(SSD1306_WHITE);
    display.println("wol sent!");
  } else {
    display.setTextColor(SSD1306_WHITE);
    display.println("send fail!");
  }
  
  display.display();
  delay(2000);
}

void OLEDDisplay::showMessage(String line1, String line2, String line3, String line4) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0, 0);
  display.println(line1);
  display.setCursor(0, 15);
  display.println(line2);
  display.setCursor(0, 30);
  display.println(line3);
  display.setCursor(0, 45);
  display.println(line4);
  
  display.display();
}

void OLEDDisplay::drawHeader(String title) {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.fillRect(0, 0, 128, 10, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(5, 2);
  display.println(title);
  display.setTextColor(SSD1306_WHITE);
}