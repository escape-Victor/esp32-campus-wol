#ifndef WOL_MANAGER_H
#define WOL_MANAGER_H

#include <WiFiUdp.h>
#include "config.h"

class WOLManager {
public:
  bool wakeDevice(const WOLDevice& device);
  bool parseMAC(const char* macStr, uint8_t* mac);
  void sendMagicPacket(const uint8_t* mac);
  
private:
  WiFiUDP udp;
};

#endif