#include "network_manager.h"
#include "config.h"
#include <cstring>

void NetworkManager::connectToWiFi() {
  Serial.print("Connecting to ZJUWLAN: ");
  Serial.println(WIFI_SSID);
  
  // For open network, no password needed
  if (strlen(WIFI_PASSWORD) == 0) {
    WiFi.begin(WIFI_SSID);
    Serial.println("Using open network connection");
  } else {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }
  
  unsigned long startTime = millis();
  int connectionAttempts = 0;
  
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 45000) {
    delay(500);
    Serial.print(".");
    connectionAttempts++;
    
    if (connectionAttempts % 20 == 0) {
      Serial.println();
      Serial.print("Still trying to connect... ");
      Serial.print((millis() - startTime) / 1000);
      Serial.println(" seconds");
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected successfully!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi connection failed!");
  }
}

bool NetworkManager::checkAndHandlePortal() {
  if (portalAuth.detectPortal()) {
    Serial.println("Trying automatic Portal authentication...");
    
    if (strlen(PORTAL_USERNAME) > 0 && strlen(PORTAL_PASSWORD) > 0) {
      Serial.println("Using configured account for auto authentication");
      bool success = portalAuth.loginPortal(PORTAL_USERNAME, PORTAL_PASSWORD);
      
      if (success) {
        delay(3000);
        return portalAuth.isInternetAvailable();
      }
    } else {
      Serial.println("No Portal authentication info configured");
    }
  } else {
    Serial.println("No Portal detected, network might be normal");
  }
  
  return portalAuth.isInternetAvailable();
}

bool NetworkManager::isInternetAvailable() {
  return portalAuth.isInternetAvailable();
}

void NetworkManager::setupWebServer() {
  server.on("/", std::bind(&NetworkManager::handleRoot, this));
  server.on("/wake", std::bind(&NetworkManager::handleWake, this));
  server.on("/status", std::bind(&NetworkManager::handleStatus, this));
  server.on("/portal_login", HTTP_POST, std::bind(&NetworkManager::handlePortalLogin, this));
  
  server.begin();
  Serial.println("HTTP server started");
}

void NetworkManager::handleClient() {
  server.handleClient();
}

String NetworkManager::getLocalIP() {
  return WiFi.localIP().toString();
}

String NetworkManager::getSSID() {
  return WiFi.SSID();
}

int NetworkManager::getRSSI() {
  return WiFi.RSSI();
}

bool NetworkManager::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

void NetworkManager::handleRoot() {
  // Simple HTML without Chinese characters
  String html = "<!DOCTYPE html><html><head>";
  html += "<title>ESP32 WOL Controller - ZJU</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body { font-family: Arial; margin: 20px; background: #f0f0f0; }";
  html += ".container { max-width: 500px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; }";
  html += ".device { background: #e8f4fd; margin: 10px 0; padding: 15px; border-radius: 5px; }";
  html += "button { background: #4CAF50; color: white; border: none; padding: 10px 20px; border-radius: 5px; cursor: pointer; }";
  html += ".portal-form { margin: 20px 0; }";
  html += ".portal-form input { width: 100%; padding: 10px; margin: 5px 0; border: 1px solid #ddd; border-radius: 4px; }";
  html += "</style></head><body>";
  html += "<div class='container'>";
  html += "<h1>ZJU WOL Controller</h1>";
  html += "<div style='background: #fff3cd; padding: 10px; margin: 10px 0; border-radius: 5px;'>";
  html += "<strong>Network Status:</strong> ";
  html += isConnected() ? "Connected" : "Disconnected";
  html += " | <strong>IP:</strong> " + getLocalIP();
  html += "</div>";
  html += "<h3>Device Wake:</h3>";
  
  for(int i = 0; i < DEVICE_COUNT; i++) {
    html += "<div class='device'>";
    html += "<strong>" + String(devices[i].name) + "</strong><br>";
    html += "MAC: " + String(devices[i].mac);
    html += "<br><button onclick=\"wakeDevice('" + String(i) + "')\">Wake Device</button>";
    html += "</div>";
  }
  
  html += "<div class='portal-form'>";
  html += "<h3>Campus Network Login</h3>";
  html += "<input type='text' id='portalUser' placeholder='Student ID'>";
  html += "<input type='password' id='portalPass' placeholder='Password'>";
  html += "<button onclick='portalLogin()'>Login</button>";
  html += "<div id='portalResult'></div>";
  html += "</div>";
  html += "<script>";
  html += "function wakeDevice(deviceId) {";
  html += "  fetch('/wake?device=' + deviceId)";
  html += "    .then(response => response.text())";
  html += "    .then(data => alert(data));";
  html += "}";
  html += "function portalLogin() {";
  html += "  var username = document.getElementById('portalUser').value;";
  html += "  var password = document.getElementById('portalPass').value;";
  html += "  fetch('/portal_login', {";
  html += "    method: 'POST',";
  html += "    headers: {'Content-Type': 'application/x-www-form-urlencoded'},";
  html += "    body: 'username=' + encodeURIComponent(username) + '&password=' + encodeURIComponent(password)";
  html += "  })";
  html += "  .then(response => response.json())";
  html += "  .then(data => {";
  html += "    var result = document.getElementById('portalResult');";
  html += "    result.innerHTML = data.message;";
  html += "    result.style.color = data.status === 'success' ? 'green' : 'red';";
  html += "  });";
  html += "}";
  html += "</script>";
  html += "</div></body></html>";
  
  server.send(200, "text/html", html);
}

void NetworkManager::handleWake() {
  if(server.hasArg("device")) {
    int deviceId = server.arg("device").toInt();
    if(deviceId >= 0 && deviceId < DEVICE_COUNT) {
      server.send(200, "text/plain", "Wake command sent to " + String(devices[deviceId].name));
    } else {
      server.send(400, "text/plain", "Invalid device ID");
    }
  } else {
    server.send(400, "text/plain", "Missing device parameter");
  }
}

void NetworkManager::handleStatus() {
  String json = "{";
  json += "\"connected\":" + String(isConnected() ? "true" : "false") + ",";
  json += "\"ip\":\"" + getLocalIP() + "\",";
  json += "\"ssid\":\"" + getSSID() + "\",";
  json += "\"rssi\":" + String(getRSSI()) + ",";
  json += "\"internet\":" + String(isInternetAvailable() ? "true" : "false");
  json += "}";
  
  server.send(200, "application/json", json);
}

void NetworkManager::handlePortalLogin() {
  if(server.hasArg("username") && server.hasArg("password")) {
    String username = server.arg("username");
    String password = server.arg("password");
    
    bool success = portalAuth.loginPortal(username, password);
    
    if(success) {
      server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Login successful\"}");
    } else {
      server.send(200, "application/json", "{\"status\":\"error\",\"message\":\"Login failed\"}");
    }
  } else {
    server.send(400, "text/plain", "Missing username or password");
  }
}