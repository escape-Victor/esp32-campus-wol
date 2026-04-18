#ifndef CONFIG_H
#define CONFIG_H

extern const char* WIFI_SSID;
extern const char* WIFI_PASSWORD;
extern const char* PORTAL_USERNAME;
extern const char* PORTAL_PASSWORD;

extern const int OLED_SDA_PIN;
extern const int OLED_SCL_PIN;
extern const int STATUS_LED_PIN;

struct WOLDevice {
  const char* name;
  const char* mac;
};

extern const char* NGROK_AUTH_TOKEN;
extern const char* NGROK_TUNNEL_NAME;
extern const int NGROK_CHECK_INTERVAL;
extern const char* COMPUTER_IP;

// 预定义的WOL设备列表 - 请修改为您的设备MAC地址
extern WOLDevice devices[];
extern const int DEVICE_COUNT;

// 显示配置
extern const int DISPLAY_UPDATE_INTERVAL;

#endif