#include "wol_manager.h"
#include <Arduino.h>
#include <WiFi.h>

bool WOLManager::wakeDevice(const WOLDevice& device) {
  uint8_t mac[6];
  
  if(!parseMAC(device.mac, mac)) {
    Serial.println("MAC add wrong format");
    return false;
  }
  
  sendMagicPacket(mac);
  Serial.print("has sent wol to: ");
  Serial.println(device.name);
  return true;
}

bool WOLManager::parseMAC(const char* macStr, uint8_t* mac) {
  if(sscanf(macStr, "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx",
            &mac[0], &mac[1], &mac[2], 
            &mac[3], &mac[4], &mac[5]) == 6) {
    return true;
  }
  
  // 尝试其他格式
  if(sscanf(macStr, "%2hhx-%2hhx-%2hhx-%2hhx-%2hhx-%2hhx",
            &mac[0], &mac[1], &mac[2], 
            &mac[3], &mac[4], &mac[5]) == 6) {
    return true;
  }
  
  return false;
}

void WOLManager::sendMagicPacket(const uint8_t* mac) {
  uint8_t magicPacket[102];
  
  // 前6字节为0xFF
  for(int i = 0; i < 6; i++) {
    magicPacket[i] = 0xFF;
  }
  
  // 重复16次MAC地址
  for(int i = 1; i <= 16; i++) {
    for(int j = 0; j < 6; j++) {
      magicPacket[i * 6 + j] = mac[j];
    }
  }
  
  // 发送到广播地址
  udp.beginPacket("255.255.255.255", 9);
  udp.write(magicPacket, sizeof(magicPacket));
  udp.endPacket();
  
  // 也尝试发送到子网广播
  IPAddress localIP = WiFi.localIP();
  IPAddress broadcastIP = localIP;
  broadcastIP[3] = 255;
  
  udp.beginPacket(broadcastIP, 9);
  udp.write(magicPacket, sizeof(magicPacket));
  udp.endPacket();
}