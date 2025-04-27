#pragma once
#include <functional>
#include <string>
#include <thread>
#include "cocos2d.h"

class UdpManager {
public:
    static UdpManager* getInstance();

    // 启动主机广播
    void startHostBroadcast(int wsPort);

    // 客户端搜索主机
    void searchHosts(std::function<void(std::string ip, int port)> callback);

    // 停止所有UDP操作
    void stop();

private:
    bool _running = false;
    std::thread _udpThread;
    std::function<void(std::string, int)> _discoveryCallback;

    void hostRoutine(int wsPort);
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