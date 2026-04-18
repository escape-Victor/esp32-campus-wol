#include "config.h"

const char *WIFI_SSID = "ZJUWLAN";
const char *WIFI_PASSWORD = "";

const char *PORTAL_USERNAME = "YOUR_ID";
const char *PORTAL_PASSWORD = "YOUR_PASSPORT";

WOLDevice devices[] = {
    {"宿舍电脑", "YOUR_MAC"}};

const char *NGROK_AUTH_TOKEN = "34r9ZaQSXhvM1q8Edb0JMxHDMDL_pzfUsaFfUSi4R8C57bmA";
const char *NGROK_TUNNEL_NAME = "esp32-wol-controller";
const int NGROK_CHECK_INTERVAL = 30000;

const int OLED_SDA_PIN = 21;
const int OLED_SCL_PIN = 22;
const int STATUS_LED_PIN = 2;

const int DEVICE_COUNT = sizeof(devices) / sizeof(devices[0]);

const int DISPLAY_UPDATE_INTERVAL = 2000;

const char *COMPUTER_IP = "YOUR_CAMPUS_IP";
