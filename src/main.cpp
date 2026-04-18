#include <Arduino.h>
#include "config.h"
#include "oled_display.h"
#include "wol_manager.h"
#include "network_manager.h"
#include <esp_task_wdt.h>
#include <esp_sleep.h>
#include "ngrok_tunnel.h"
#include "websocket_server.h"

// 全局对象
OLEDDisplay oled(OLED_SDA_PIN, OLED_SCL_PIN);
WOLManager wolManager;
NetworkManager networkManager;
NgrokTunnel ngrokTunnel;
WebSocketServer webSocketServer;

// 状态变量
int selectedDevice = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastSystemCheck = 0;
unsigned long lastWiFiCheck = 0;
enum AppState { BOOT, NETWORK_INFO, DEVICE_LIST, WAKING, MAINTENANCE, TUNNEL_STATUS};
AppState currentState = BOOT;

// 系统状态
bool systemStable = true;
int consecutiveWiFiFailures = 0;
unsigned long systemStartTime = 0;

// 函数声明
void updateDisplay();
void handleSerialInput();
void wakeDevice(int deviceId);
void systemMonitor();
void checkWiFiConnection();
void handleLowMemory();
void enterLightSleep();
void safeRestart();
void smartLEDIndicator();
void checkAuthExpiry();

void setup() {
  systemStartTime = millis();
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("ESP32 WOL控制器启动中...");
  
  // 初始化看门狗
  esp_task_wdt_init(30, true); // 30秒超时
  esp_task_wdt_add(NULL);
  
  // 初始化硬件
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, HIGH);
  
  // 显示启动信息
  Serial.println("初始化OLED显示屏...");
  if(!oled.begin()) {
    Serial.println("OLED初始化失败!");
    // 继续运行，只是没有显示
  } else {
    Serial.println("OLED初始化成功");
    oled.showWelcome();
    delay(2000);
  }
  
  // 连接WiFi
  Serial.println("connecting WiFi...");
  oled.showMessage("connecting wifi", WIFI_SSID, "", "waiting...");
  networkManager.connectToWiFi();
  
  // 启动Web服务器
  if(networkManager.isConnected()) {
    networkManager.setupWebServer();

    webSocketServer.begin();
    
    // 启动Ngrok隧道
    ngrokTunnel.begin();
    
    currentState = TUNNEL_STATUS;
    Serial.println("all services started successfully");
  } else {
    oled.showMessage("WiFi fail to connect", "");
    currentState = DEVICE_LIST;
  }
}


void loop() {
  // 喂狗 - 保持系统活跃
  esp_task_wdt_reset();
  
  // 处理网络请求
  networkManager.handleClient();

  webSocketServer.loop();

  ngrokTunnel.update();
  
  // 更新显示
  if(millis() - lastDisplayUpdate > DISPLAY_UPDATE_INTERVAL) {
    updateDisplay();
    lastDisplayUpdate = millis();
  }
  
  // 系统监控（每10秒一次）
  if(millis() - lastSystemCheck > 10000) {
    systemMonitor();
    lastSystemCheck = millis();
  }
  
  // WiFi状态检查（每30秒一次）
  if(millis() - lastWiFiCheck > 30000) {
    checkWiFiConnection();
    lastWiFiCheck = millis();
  }
  
  // 处理串口输入
  handleSerialInput();
  
  
  // 内存管理
  handleLowMemory();
  
  delay(100);
}

void updateDisplay() {
  switch(currentState) {
    case TUNNEL_STATUS:
      if (ngrokTunnel.isTunnelActive()) {
        String url = ngrokTunnel.getPublicUrl();
        url.replace("https://", "");
        oled.showMessage("remote control ready", "add:", url, "phone can access");
      } else {
        oled.showMessage("Ngrok turnel", "setting...", "check PC", "ngrok service");
      }
      break;
      
    case DEVICE_LIST:
      oled.showDeviceList(selectedDevice);
      break;
      
    case WAKING:
      // 唤醒状态显示由唤醒函数处理
      break;
      
    default:
      break;
  }
}

void smartLEDIndicator() {
  static unsigned long lastLedToggle = 0;
  static int ledPattern = 0;
  
  unsigned long currentTime = millis();
  int blinkInterval = 1000; // 默认1秒
  
  // 根据系统状态调整LED模式
  if(!networkManager.isConnected()) {
    blinkInterval = 300; // 快速闪烁：网络断开
  } else if(!systemStable) {
    blinkInterval = 100; // 极快闪烁：系统不稳定
  }
  
  if(currentTime - lastLedToggle > blinkInterval) {
    digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
    lastLedToggle = currentTime;
  }
}

void systemMonitor() {
  unsigned long currentTime = millis();
  int freeHeap = ESP.getFreeHeap();
  
  // 输出系统状态到串口
  Serial.println("=== system status moniter ===");
  Serial.printf("running time: %lu 秒\n", currentTime / 1000);
  Serial.printf("available ram: %d bytes\n", freeHeap);
  Serial.printf("temperature: %.1f°C\n", temperatureRead());
  
  if(networkManager.isConnected()) {
    Serial.printf("WiFi sign: %d dBm\n", networkManager.getRSSI());
    Serial.printf("IP add: %s\n", networkManager.getLocalIP().c_str());
  } else {
    Serial.println("WiFi disconnected");
  }
  
  // 内存警告
  if(freeHeap < 15000) {
    Serial.println("ram shorty!");
  }
  
  // 24小时自动维护重启
  if(currentTime - systemStartTime > 86400000) { // 24小时
    Serial.println("execute regular maintain restart");
    currentState = MAINTENANCE;
    oled.showMessage("regular maintain", "restarting", "", "");
    delay(2000);
    safeRestart();
  }



}

void checkAuthExpiry() {
  static unsigned long lastAuthCheck = 0;
  
  if (millis() - lastAuthCheck > 3600000) { // 每小时检查一次
    // 使用networkManager的方法检查互联网连接状态
    if (!networkManager.isInternetAvailable()) {
      Serial.println("reidentify in web portal");
      oled.showMessage("web identity failed", "use web", "reidentify", "");
    }
    lastAuthCheck = millis();
  }
}

void checkWiFiConnection() {
  if(!networkManager.isConnected()) {
    consecutiveWiFiFailures++;
    Serial.printf("WiFi disconnect,try reconnecting (#%d)\n", consecutiveWiFiFailures);
    
    oled.showMessage("WiFi reconnecting", String(consecutiveWiFiFailures) + "times attempt", "", "");
    
    networkManager.connectToWiFi();
    
    if(networkManager.isConnected()) {
      consecutiveWiFiFailures = 0;
      Serial.println("WiFi reconnected successfully");
      systemStable = true;
    } else if(consecutiveWiFiFailures >= 5) {
      Serial.println("WiFi failed to reconnect after multiple attempts, system unstable");
      systemStable = false;
    }
  } else {
    consecutiveWiFiFailures = 0;
    systemStable = true;
  }
}

void handleLowMemory() {
  int freeHeap = ESP.getFreeHeap();
  
  if(freeHeap < 10000) { // 内存严重不足
    Serial.println("内存严重不足，执行紧急清理!");
    oled.showMessage("内存不足", "执行清理...", "", "");
    
    // 尝试清理资源
    delay(1000);
    
    if(ESP.getFreeHeap() < 8000) { // 仍然不足，重启
      Serial.println("内存不足，安全重启...");
      safeRestart();
    }
  } else if(freeHeap < 20000) { // 内存警告
    // 可以在这里添加资源清理逻辑
    Serial.println("内存较低，建议优化");
  }
}

void enterLightSleep() {
  // 轻睡眠模式，节省功耗
  Serial.println("进入轻睡眠模式");
  oled.showMessage("节能模式", "进入睡眠", " press EN to wakeup", "");
  
  // 关闭OLED以省电
  oled.clear();
  
  // 设置唤醒源
  esp_sleep_enable_timer_wakeup(30 * 1000000); // 30秒后唤醒
  
  // 进入轻睡眠
  esp_light_sleep_start();
  
  // 唤醒后
  Serial.println("从睡眠中唤醒");
  oled.showMessage("系统唤醒", "恢复运行", "", "");
  delay(1000);
}

void safeRestart() {
  Serial.println("执行安全重启...");
  oled.showMessage("安全重启", "系统重新启动", "", "");
  delay(2000);
  
  // 清理资源
  oled.clear();
  
  // 重启
  ESP.restart();
}

void handleSerialInput() {
  if(Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if(command == "list") {
      Serial.println("available devices:");
      for(int i = 0; i < DEVICE_COUNT; i++) {
        Serial.printf("%d: %s (%s)\n", i, devices[i].name, devices[i].mac);
      }
    }
    else if(command.startsWith("wake")) {
      int deviceId = command.substring(5).toInt();
      if(deviceId >= 0 && deviceId < DEVICE_COUNT) {
        wakeDevice(deviceId);
      }
    }
    else if(command == "status") {
      Serial.print("Ngrok status: ");
      Serial.println(ngrokTunnel.getTunnelStatus());
    }
    else if(command == "net") {
      currentState = TUNNEL_STATUS;
    }
    else if(command == "dev") {
      currentState = DEVICE_LIST;
    }
    else if(command == "help") {
      Serial.println("可用命令:");
      Serial.println("list    - 显示设备列表");
      Serial.println("wake [id] - 唤醒设备");
      Serial.println("ngrok   - 显示Ngrok状态");
      Serial.println("net     - 显示网络状态");
      Serial.println("dev     - 显示设备列表");
    }
  }
}

void wakeDevice(int deviceId) {
  currentState = WAKING;
  
  Serial.printf("try wakeup: %s\n", devices[deviceId].name);
  oled.showMessage("wol senting", devices[deviceId].name, "addressing...", "");
  
  bool success = wolManager.wakeDevice(devices[deviceId]);
  
  if(success) {
    Serial.println("wol sent successfully");
    oled.showMessage("wakeup success", devices[deviceId].name, "wol sent", "");
  } else {
    Serial.println("wol fail to send");
    oled.showMessage("wakeup fail", devices[deviceId].name, "check mac add", "");
  }
  
  delay(2000);
  currentState = DEVICE_LIST;
}