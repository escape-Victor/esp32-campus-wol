#ifndef WEBSOCKET_SERVER_H
#define WEBSOCKET_SERVER_H

#include <WebSocketsServer.h>
#include "wol_manager.h"

class WebSocketServer {
public:
  WebSocketServer();
  void begin();
  void loop();
  void sendStatusToAll();
  void sendWakeResult(uint8_t num, String deviceName, bool success);
  
private:
  WebSocketsServer webSocket;
  
  void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
  void handleWebSocketMessage(uint8_t num, String message);
  void sendDeviceList(uint8_t num);
};

#endif