#include "ngrok_tunnel.h"
#include "config.h"
#include "oled_display.h"
#include <Arduino.h>

extern OLEDDisplay oled;

NgrokTunnel::NgrokTunnel() : tunnelActive(false), lastCheck(0) {}

void NgrokTunnel::begin() {
  Serial.println("Initializing Ngrok client...");
  Serial.print("Target computer IP: ");
  Serial.println(COMPUTER_IP);
  
  oled.showMessage("Ngrok Init", "Computer IP:", COMPUTER_IP, "");
  
  // Check status immediately
  checkNgrokStatus();
}

void NgrokTunnel::update() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastCheck > NGROK_CHECK_INTERVAL) {
    checkNgrokStatus();
    lastCheck = currentTime;
  }
}

bool NgrokTunnel::checkNgrokStatus() {
  HTTPClient http;
  
  // Use computer IP address to query Ngrok status
  String url = "http://" + String(COMPUTER_IP) + ":4040/api/tunnels";
  
  Serial.print("Querying Ngrok status: ");
  Serial.println(url);
  
  http.begin(url);
  http.setTimeout(8000); // 8 second timeout
  
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String payload = http.getString();
    
    // Parse JSON response
    int httpsIndex = payload.indexOf("https://");
    if (httpsIndex != -1) {
      int urlEnd = payload.indexOf("\"", httpsIndex);
      publicUrl = payload.substring(httpsIndex, urlEnd);
      tunnelActive = true;
      
      Serial.print("Public URL obtained: ");
      Serial.println(publicUrl);
      
      updateOLEDStatus();
      http.end();
      return true;
    } else {
      Serial.println("HTTPS tunnel not found");
    }
  } else {
    Serial.print("HTTP Error: ");
    Serial.println(httpCode);
    
    // Show specific error information
    if (httpCode == -1) {
      Serial.println("Connection failed, please check:");
      Serial.println("1. Computer IP correct: " + String(COMPUTER_IP));
      Serial.println("2. Computer firewall allows port 4040");
      Serial.println("3. Ngrok running on computer");
      Serial.println("4. Computer and ESP32 on same network");
    }
  }
  
  http.end();
  tunnelActive = false;
  return false;
}

void NgrokTunnel::updateOLEDStatus() {
  if (tunnelActive) {
    String displayUrl = publicUrl;
    displayUrl.replace("https://", "");
    
    // Truncate long URL for OLED
    if (displayUrl.length() > 16) {
      displayUrl = displayUrl.substring(0, 13) + "...";
    }
    
    oled.showMessage("Remote Ready", displayUrl, "Mobile Access", "");
  } else {
    oled.showMessage("Ngrok Not Connected", "Check:", COMPUTER_IP, "Use Serial");
  }
}

bool NgrokTunnel::isTunnelActive() {
  return tunnelActive;
}

String NgrokTunnel::getPublicUrl() {
  return publicUrl;
}

String NgrokTunnel::getTunnelStatus() {
  if (tunnelActive) {
    return "Active: " + publicUrl;
  } else {
    return "Not Connected - Computer IP: " + String(COMPUTER_IP);
  }
}