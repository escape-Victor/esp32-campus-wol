#ifndef PORTAL_AUTH_H
#define PORTAL_AUTH_H

#include <HTTPClient.h>
#include <WiFi.h>

class PortalAuth {
public:
  PortalAuth();
  bool detectPortal();
  bool loginPortal(String username, String password);
  bool isInternetAvailable();
  String getRedirectUrl();
  
private:
  String findPortalUrl();
  String extractFormFields(String html);
  String getFieldValue(String html, String fieldName);
};

#endif