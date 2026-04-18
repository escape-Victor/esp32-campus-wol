#include "portal_auth.h"
#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>

#ifndef HTTPC_DISABLE_FOLLOW_REDIRECTS
#define HTTPC_DISABLE_FOLLOW_REDIRECTS 0
#endif

PortalAuth::PortalAuth() {
}

bool PortalAuth::loginPortal(String username, String password) {
  Serial.println("Trying ZJU Portal authentication...");
  
  HTTPClient http;
  bool success = false;
  
  String loginUrl = "https://net3.zju.edu.cn";
  
  http.begin(loginUrl);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  String postData = "username=" + username + "&password=" + password;
  
  Serial.print("Sending auth request to: ");
  Serial.println(loginUrl);
  
  int httpCode = http.POST(postData);
  
  if (httpCode > 0) {
    String response = http.getString();
    Serial.print("HTTP response code: ");
    Serial.println(httpCode);
    
    if (response.indexOf("success") != -1 || httpCode == 200) {
      Serial.println("ZJU Portal auth successful");
      success = true;
    } else {
      Serial.println("Auth failed");
    }
  } else {
    Serial.println("HTTP request failed");
  }
  
  http.end();
  return success;
}

bool PortalAuth::detectPortal() {
  HTTPClient http;
  
  // Try to access external site
  http.begin("http://www.baidu.com");
  int httpCode = http.GET();
  http.end();
  
  // If we can't access external site, probably need portal auth
  return (httpCode != 200);
}

bool PortalAuth::isInternetAvailable() {
  HTTPClient http;
  http.begin("http://www.baidu.com");
  int httpCode = http.GET();
  http.end();
  
  return (httpCode == 200);
}

String PortalAuth::getRedirectUrl() {
  return "https://net3.zju.edu.cn";
}