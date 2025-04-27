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

void UdpManager::hostRoutine(int wsPort) {
    // ================= 网络初始化阶段 =================
#ifdef _WIN32
    // Windows 特有的 Winsock 初始化
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        OutputDebugStringA("WSAStartup failed");
        return;
    }
#endif

    // 创建 UDP socket（IPv4，数据报类型）
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
#ifdef _WIN32
        OutputDebugStringA("Create socket failed");
        WSACleanup();
#endif
        return;
    }

    // 设置广播选项（关键步骤）
    int broadcastEnable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST,
        (char*)&broadcastEnable, sizeof(broadcastEnable)) == -1) {
#ifdef _WIN32
        closesocket(sock);
        WSACleanup();
#else
        close(sock);
#endif
        return;
    }

    // 绑定本地端口
    sockaddr_in recvAddr;
    memset(&recvAddr, 0, sizeof(recvAddr));
    recvAddr.sin_family = AF_INET;
    recvAddr.sin_port = htons(12345);      // 监听固定端口
    recvAddr.sin_addr.s_addr = INADDR_ANY; // 监听所有网络接口

    if (bind(sock, (sockaddr*)&recvAddr, sizeof(recvAddr)) == -1) {
#ifdef _WIN32
        closesocket(sock);
        WSACleanup();
#else
        close(sock);
#endif
        return;
    }

    // ================= 主循环阶段 =================
    while (_running) {
        DiscoveryPacket packet;     // 接收缓冲区
        sockaddr_in clientAddr;     // 客户端地址
        socklen_t addrLen = sizeof(clientAddr);

        // 阻塞接收客户端请求（线程可在此处等待）
        ssize_t recvLen = recvfrom(sock, (char*)&packet, sizeof(packet), 0,
            (sockaddr*)&clientAddr, &addrLen);

        // 校验数据包有效性
        if (recvLen == sizeof(DiscoveryPacket) &&
            memcmp(packet.magic, "COCO", 4) == 0 &&
            packet.type == 0) { // 类型0为发现请求

            // 构建响应包
            DiscoveryPacket response;
            response.type = 1;  // 类型1为响应
            response.ip = clientAddr.sin_addr.s_addr; // 客户端的IP
            response.port = htons(wsPort); // 转换为网络字节序

            // 设置广播目标地址
            sockaddr_in broadcastAddr;
            memset(&broadcastAddr, 0, sizeof(broadcastAddr));
            broadcastAddr.sin_family = AF_INET;
            broadcastAddr.sin_port = htons(12345); // 目标端口
            broadcastAddr.sin_addr.s_addr = INADDR_BROADCAST; // 广播地址

            // 发送响应（忽略错误，允许部分失败）
            sendto(sock, (char*)&response, sizeof(response), 0,
                (sockaddr*)&broadcastAddr, sizeof(broadcastAddr));
        }

        // 避免CPU满载（可根据需求调整间隔）
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // ================= 资源清理阶段 =================
#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
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