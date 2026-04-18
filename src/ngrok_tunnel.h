#ifndef NGROK_TUNNEL_H
#define NGROK_TUNNEL_H

#include <HTTPClient.h>
#include <ArduinoJson.h>

class NgrokTunnel {
public:
  NgrokTunnel();
  void begin();
  void update();
  bool isTunnelActive();
  String getPublicUrl();
  String getTunnelStatus();
  
private:
  String publicUrl;
  bool tunnelActive;
  unsigned long lastCheck;
  
  bool checkNgrokStatus();
  void updateOLEDStatus();
};

#endif