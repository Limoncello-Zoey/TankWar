#pragma once
#include <functional>
#include <string>
#include <thread>
#include "cocos2d.h"

class UdpManager {
public:
    static UdpManager* getInstance(); // 单例模式获取实例

    // 启动主机广播（参数：WebSocket服务端口）
    void startHostBroadcast(int wsPort);

    // 客户端搜索主机（参数：发现主机后的回调函数）
    void searchHosts(std::function<void(std::string ip, int port)> callback);

    // 停止所有UDP操作（清理资源）
    void stop();

private:
    bool _running = false;          // 线程运行标志
    std::thread _udpThread;         // UDP工作线程
    std::function<void(std::string, int)> _discoveryCallback; // 发现主机的回调

    // 主机广播逻辑（参数：WebSocket端口）
    void hostRoutine(int wsPort);

    // 客户端搜索逻辑
    void clientRoutine();
};

#pragma pack(push, 1) // 内存对齐
struct DiscoveryPacket {
    char magic[4] = { 'C','O','C','O' }; // 协议标识
    uint16_t version = 0x0001;         // 协议版本
    uint8_t  type;                     // 0:请求 1:响应
    uint32_t ip;                       // 主机IP(网络字节序)
    uint16_t port;                     // 主机WebSocket端口
};
#pragma pack(pop)