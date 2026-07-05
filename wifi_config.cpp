#include "wifi_config.h"
#include "secrets.h"
#include <WiFi.h>

// 使用 static 限制作用域，解决全局变量污染问题
static IPAddress local_IP(192, 168, 1, 100);
static IPAddress gateway(192, 168, 1, 1);
static IPAddress subnet(255, 255, 255, 0);
static IPAddress primaryDNS(8, 8, 8, 8);

const unsigned long WIFI_TIMEOUT_MS = 15000; 

bool setupWiFi() {
    Serial.println();
    Serial.print("正在配置静态 IP...");
    
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);
    
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS)) {
        Serial.println("静态 IP 配置失败！");
    }

    Serial.print("正在连接到 WiFi: ");
    Serial.println(WIFI_SSID); // 使用 constexpr 引入常量

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long startAttemptTime = millis();

    // Boot-time 采用阻塞等待是合理的（明确的设计意图）
    while (WiFi.status() != WL_CONNECTED && (millis() - startAttemptTime < WIFI_TIMEOUT_MS)) {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\n❌ WiFi 连接超时失败！");
        return false;
    } 

    Serial.println("\n✅ WiFi 连接成功!");
    Serial.print("当前 IP 地址: ");
    Serial.println(WiFi.localIP());
    return true;
}