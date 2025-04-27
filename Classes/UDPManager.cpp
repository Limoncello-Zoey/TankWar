//跨平台适用
#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SOCKET int
#define closesocket close
#endif

#include <cstring>
#include "UdpManager.h"
#include "base/CCScheduler.h"
#define socklen_t int

using namespace cocos2d;

UdpManager* UdpManager::getInstance() 
{
    static UdpManager instance;
    return &instance;
}

void UdpManager::startHostBroadcast(int wsPort) 
{
    if (_running) return;
    _running = true;
    _udpThread = std::thread(&UdpManager::hostRoutine, this, wsPort);
}

void UdpManager::searchHosts(std::function<void(std::string, int)> callback) 
{
    if (_running) return;
    _running = true;
    _discoveryCallback = callback;
    _udpThread = std::thread(&UdpManager::clientRoutine, this);
}

void UdpManager::stop() {
    _running = false;
    if (_udpThread.joinable()) _udpThread.join();
}

// ----------------- 主机广播线程 -----------------
void UdpManager::hostRoutine(int wsPort) {
    // Windows 初始化 Winsock
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

    // 创建UDP Socket
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == -1) return;

    // 关键设置：允许广播（否则无法发送广播包）
    int broadcast = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST,
        (char*)&broadcast, sizeof(broadcast));

    // 绑定本地端口（监听12345端口）
    struct sockaddr_in recvAddr;
    recvAddr.sin_family = AF_INET;            // IPv4
    recvAddr.sin_port = htons(12345);        // 端口号转网络字节序
    recvAddr.sin_addr.s_addr = INADDR_ANY;   // 监听所有网卡
    bind(sock, (struct sockaddr*)&recvAddr, sizeof(recvAddr));

    // 主循环：持续监听客户端请求
    while (_running) {
        // 接收客户端发现请求
        DiscoveryPacket packet;
        struct sockaddr_in clientAddr;
        socklen_t addrLen = sizeof(clientAddr);

        // 阻塞接收数据（直到有客户端发来请求）
        ssize_t len = recvfrom(sock, (char*)&packet, sizeof(packet), 0,
            (struct sockaddr*)&clientAddr, &addrLen);

        // 验证数据包有效性
        if (len == sizeof(DiscoveryPacket) &&
            memcmp(packet.magic, "COCO", 4) == 0 &&
            packet.type == 0) {

            // 构造响应包
            DiscoveryPacket response;
            response.type = 1;
            response.ip = clientAddr.sin_addr.s_addr; // 客户端的IP
            response.port = htons(wsPort); // WebSocket端口转网络字节序

            // 向广播地址发送响应（255.255.255.255）
            struct sockaddr_in broadcastAddr;
            broadcastAddr.sin_family = AF_INET;
            broadcastAddr.sin_port = htons(12345);
            broadcastAddr.sin_addr.s_addr = INADDR_BROADCAST;

            sendto(sock, (char*)&response, sizeof(response), 0,
                (struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr));
        }

        // 避免CPU占用过高
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // 关闭Socket和清理资源
    closesocket(sock);
#ifdef _WIN32
    WSACleanup();
#endif
}

// ----------------- 客户端搜索线程 -----------------
void UdpManager::clientRoutine() {
    // Windows 初始化 Winsock
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

    // 创建UDP Socket
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == -1) return;

    // 设置接收超时（2秒后停止等待响应）
#ifdef _WIN32
    DWORD timeout = 2000; // 毫秒
#else
    struct timeval timeout { 2, 0 }; // 秒+微秒
#endif
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
        (char*)&timeout, sizeof(timeout));

    // 发送广播请求包
    DiscoveryPacket request;
    request.type = 0; // 请求类型

    // 目标地址为广播地址
    struct sockaddr_in broadcastAddr;
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(12345);    // 主机监听端口
    broadcastAddr.sin_addr.s_addr = INADDR_BROADCAST; // 255.255.255.255

    sendto(sock, (char*)&request, sizeof(request), 0,
        (struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr));

    // 主循环：等待主机响应
    while (_running) {
        DiscoveryPacket response;
        struct sockaddr_in serverAddr;
        socklen_t addrLen = sizeof(serverAddr);

        // 接收主机响应
        ssize_t len = recvfrom(sock, (char*)&response, sizeof(response), 0,
            (struct sockaddr*)&serverAddr, &addrLen);

        // 验证响应有效性
        if (len == sizeof(DiscoveryPacket) &&
            memcmp(response.magic, "COCO", 4) == 0 &&
            response.type == 1) {

            // 解析主机信息
            std::string ip = inet_ntoa(serverAddr.sin_addr); // 转字符串IP
            int port = ntohs(response.port); // 转本地字节序

            // 在 Cocos2d-x 主线程中执行回调（避免多线程问题）
            Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]() {
                if (_discoveryCallback) {
                    _discoveryCallback(ip, port); // 例如：连接WebSocket
                }
                });

            break; // 找到主机后退出循环
        }
    }

    // 关闭Socket
    closesocket(sock);
#ifdef _WIN32
    WSACleanup();
#endif
}