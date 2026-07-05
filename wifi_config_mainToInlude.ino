#include <Arduino.h>
#include "wifi_config.h"
#include <WiFi.h> // 需要用到 wl_status_t 结构体

bool isNetworkAvailable = false;

// 轮询控制变量
unsigned long previousWifiCheckTime = 0;
unsigned long currentCheckInterval = 10000;      // 动态变化的检查间隔

// 常量配置
const unsigned long BASE_CHECK_INTERVAL = 10000; // 基础检测与重连延迟 (10s)
const unsigned long MAX_BACKOFF_INTERVAL = 60000;// 指数退避的最大间隔 (60s)

void setup() {
    Serial.begin(115200);
    delay(1000); 
    
    isNetworkAvailable = setupWiFi();
    
    if (!isNetworkAvailable) {
        Serial.println("警告：设备初始进入离线运行模式。");
    }
}

void loop() {
    unsigned long currentMillis = millis();

    // 1. 网络状态检测与自适应重连
    if (currentMillis - previousWifiCheckTime >= currentCheckInterval) {
        previousWifiCheckTime = currentMillis;
        
        wl_status_t currentStatus = WiFi.status();
        
        // 状态下降沿：刚刚断开
        if (isNetworkAvailable && currentStatus != WL_CONNECTED) {
            Serial.println("⚠ 警告：WiFi 连接丢失！");
            isNetworkAvailable = false;
            currentCheckInterval = BASE_CHECK_INTERVAL; // 掉线时重置退避时间
        }
        
        // 离线状态下的精细化重连逻辑
        if (!isNetworkAvailable) {
            // 严格过滤状态：只有在明确是断开状态时才触发重连，防止打断正在进行的握手
            if (currentStatus == WL_DISCONNECTED) {
                Serial.print("尝试重新连接 WiFi... (下次重试将在 ");
                Serial.print(currentCheckInterval / 1000);
                Serial.println("s 后)");
                
                WiFi.reconnect();
                
                // 指数退避：重连失败则等待时间翻倍，最高封顶至 MAX_BACKOFF_INTERVAL
                currentCheckInterval = min(currentCheckInterval * 2, MAX_BACKOFF_INTERVAL);
            } 
            else if (currentStatus == WL_NO_SSID_AVAIL) {
                // 如果是路由器彻底无信号（如停电），同样应用退避策略
                Serial.println("未找到目标 SSID，等待环境恢复...");
                currentCheckInterval = min(currentCheckInterval * 2, MAX_BACKOFF_INTERVAL);
            }
        }
        
        // 状态上升沿：重新连接成功
        if (!isNetworkAvailable && currentStatus == WL_CONNECTED) {
            Serial.println("✅ WiFi 已恢复连接！");
            isNetworkAvailable = true;
            currentCheckInterval = BASE_CHECK_INTERVAL; // 恢复基础心跳检测频率
        }
    }

    // 2. 本地核心逻辑（始终无阻碍运行）
    // readSensors();
    // controlMotors();

    // 3. 严格受控的网络任务
    if (isNetworkAvailable) {
        // publishMQTT();
    }
}