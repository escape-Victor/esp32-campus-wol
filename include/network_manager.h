#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <WiFi.h>
#include <WebServer.h>
#include "portal_auth.h"

class NetworkManager {
public:
  void connectToWiFi();
  void setupWebServer();
  void handleClient();
  String getLocalIP();
  String getSSID();
  int getRSSI();
  bool isConnected();
  bool isInternetAvailable();
  
private:
  WebServer server;
  PortalAuth portalAuth;
  void handleRoot();
  void handleWake();
  void handleStatus();
  void handlePortalLogin();
  bool checkAndHandlePortal();
};

#endif