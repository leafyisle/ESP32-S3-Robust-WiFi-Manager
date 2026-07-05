ESP32-S3 Robust WiFi Manager
一个为 ESP32-S3 打造的生产级、高可用 WiFi 连接与状态守护模块。本模块专为需要高稳定性、长时间离线容忍度的物联网边缘设备设计，彻底解决了传统 WiFi 脚本容易导致的系统“假死”、阻塞业务逻辑以及敏感凭证泄露等痛点。
✨ 核心特性
非阻塞自适应重连：主循环 loop() 彻底摒弃阻塞式的 delay()，基于 millis() 实现定时心跳检测。无论网络处于何种波动状态，都不会阻塞本地传感器读取、电机控制等核心业务逻辑的运行。
精细化状态机与指数退避：精准识别底层 WL_DISCONNECTED 与 WL_NO_SSID_AVAIL 状态，避免在握手中途（如 WL_IDLE_STATUS）强行触发重连导致无限重置。引入指数退避（Exponential Backoff）机制，若重连失败，重试间隔将自动翻倍（基础 10 秒，最高封顶 60 秒），有效保护空口资源并降低设备功耗。
零安全隐患的凭证隔离：引入 secrets.h.example 脚手架模板。真实网络凭证被强制抽离至独立的 secrets.h 中，配合 .gitignore 杜绝 SSID 与密码泄露到版本库中。
现代 C++ 类型安全：全面使用 constexpr const char* 替代传统的宏定义，提供严格的类型检查。模块内部的静态 IP 配置文件使用 static 限制作用域，杜绝全局命名空间污染。
Fail-Fast 启动策略：在 setup() 初始化阶段，采用带有明确超时机制（15000 毫秒）的阻塞等待。确保初始网络状态清晰可控后再进入主循环，遵循工业级设备的“快速失败”启动原则。内置静态 IP（192.168.1.100）配置，进一步提升网络寻址与重连速度。
🚀 快速上手
1. 克隆代码与配置依赖
将本仓库代码拉取到你的本地 Arduino 项目目录下。确保项目根目录包含以下核心文件：
wifi_config.h
wifi_config.cpp
secrets.h.example
2. 配置本地凭证
复制模板文件并重命名为 secrets.h：
Bash  
cp secrets.h.example secrets.h
打开 secrets.h，填入你真实的 WiFi SSID 和密码：
C++  
constexpr const char* WIFI_SSID = "YOUR_SSID_HERE";  
constexpr const char* WIFI_PASSWORD = "YOUR_PASSWORD_HERE";
⚠️ 重要提示： 务必在你的项目根目录的 .gitignore 文件中添加 secrets.h，以防止真实密码被提交。
3. 自定义网络参数 (可选)
如果需要更改静态 IP，请在 wifi_config.cpp 中修改以下 static 变量：
C++  
static IPAddress local_IP(192, 168, 1, 100);  // 默认 ESP32 IP  
static IPAddress gateway(192, 168, 1, 1);     // 默认网关  
static IPAddress subnet(255, 255, 255, 0);    // 默认子网掩码  
static IPAddress primaryDNS(8, 8, 8, 8);      // 默认 DNS
🛠 代码架构说明
本模块采用状态与执行解耦的设计：
状态维护层：通过 WiFi.status() 获取底层真实状态，更新应用层缓存标志位 isNetworkAvailable。
重连执行层：仅在检测到明显的掉线状态的下降沿（从连通到断开）时，重置重试计数器；并在离线状态下按照当前退避延迟间隔安全地调用 WiFi.reconnect()。
业务接入层：在你的 loop() 中，将所有强依赖网络的任务（如 MQTT 发布、HTTP 请求）包裹在 if (isNetworkAvailable) 判断中即可，无需关心底层的连接状态维护细节。
