#include "websocket_server.h"
#include "config.h"
#include "oled_display.h"
#include <ArduinoJson.h>

extern OLEDDisplay oled;

WebSocketServer::WebSocketServer() : webSocket(81) {}

void WebSocketServer::begin() {
  webSocket.begin();
  webSocket.onEvent([this](uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    this->webSocketEvent(num, type, payload, length);
  });
  
  Serial.println("WebSocket server started on port 81");
}

void WebSocketServer::loop() {
  webSocket.loop();
}

void WebSocketServer::webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected\n", num);
      break;
      
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] New connection: %s\n", num, ip.toString().c_str());
        oled.showMessage("Remote Connect", ip.toString(), "Established", "");
        
        // Send device list
        sendDeviceList(num);
      }
      break;
      
    case WStype_TEXT:
      {
        String message = String((char*)payload);
        Serial.printf("[%u] Received: %s\n", num, message);
        handleWebSocketMessage(num, message);
      }
      break;
  }
}

void WebSocketServer::handleWebSocketMessage(uint8_t num, String message) {
  // Simple command processing
  if (message == "get_devices") {
    sendDeviceList(num);
  } 
  else if (message.startsWith("wake:")) {
    int deviceId = message.substring(5).toInt();
    if (deviceId >= 0 && deviceId < DEVICE_COUNT) {
      String deviceName = devices[deviceId].name;
      Serial.printf("Remote wake: %s\n", deviceName.c_str());
      
      oled.showMessage("Remote Wake", deviceName, "Processing...", "");
      
      // Here we need to call WOL manager
      // bool success = wolManager.wakeDevice(devices[deviceId]);
      bool success = true; // Temporarily set to success
      
      // Send result
      String result = "result:" + String(deviceId) + ":" + (success ? "success" : "fail");
      webSocket.sendTXT(num, result);
      
      if (success) {
        oled.showMessage("Wake Success", deviceName, "Remote operation", "completed");
      } else {
        oled.showMessage("Wake Failed", deviceName, "Check config", "");
      }
    }
  }
}

void WebSocketServer::sendDeviceList(uint8_t num) {
  String deviceList = "devices:";
  for (int i = 0; i < DEVICE_COUNT; i++) {
    deviceList += String(i) + ":" + devices[i].name + ":" + devices[i].mac;
    if (i < DEVICE_COUNT - 1) deviceList += ",";
  }
  webSocket.sendTXT(num, deviceList);
}

void WebSocketServer::sendStatusToAll() {
  String status = "status:connected";
  webSocket.broadcastTXT(status);
}

void WebSocketServer::sendWakeResult(uint8_t num, String deviceName, bool success) {
  String result = "wake_result:" + deviceName + ":" + (success ? "success" : "fail");
  webSocket.sendTXT(num, result);
}