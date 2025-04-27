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
void UdpManager::hostRoutine(int wsPort) 
{
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == -1) return;

    // 允许广播
    int broadcast = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST,
        (char*)&broadcast, sizeof(broadcast));

    struct sockaddr_in recvAddr;
    recvAddr.sin_family = AF_INET;
    recvAddr.sin_port = htons(12345); // 监听端口
    recvAddr.sin_addr.s_addr = INADDR_ANY;

    bind(sock, (struct sockaddr*)&recvAddr, sizeof(recvAddr));

    while (_running) 
    {
        // 接收发现请求
        DiscoveryPacket packet;
        struct sockaddr_in clientAddr;
        socklen_t addrLen = sizeof(clientAddr);
        ssize_t len = recvfrom(sock, (char*)&packet, sizeof(packet), 0,
            (struct sockaddr*)&clientAddr, &addrLen);

        if (len == sizeof(DiscoveryPacket) &&
            memcmp(packet.magic, "COCO", 4) == 0 &&
            packet.type == 0) { // 请求包

            // 发送响应
            DiscoveryPacket response;
            response.type = 1;
            response.ip = clientAddr.sin_addr.s_addr;
            response.port = htons(wsPort);

            struct sockaddr_in broadcastAddr;
            broadcastAddr.sin_family = AF_INET;
            broadcastAddr.sin_port = htons(12345);
            broadcastAddr.sin_addr.s_addr = INADDR_BROADCAST;

            sendto(sock, (char*)&response, sizeof(response), 0,
                (struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    closesocket(sock);
#ifdef _WIN32
    WSACleanup();
#endif
}

// ----------------- 客户端搜索线程 -----------------
void UdpManager::clientRoutine() 
{
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == -1) return;

    // 设置超时
#ifdef _WIN32
    DWORD timeout = 2000;
#else
    struct timeval timeout { 2, 0 };
#endif
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
        (char*)&timeout, sizeof(timeout));

    // 发送发现请求
    DiscoveryPacket request;
    request.type = 0;

    struct sockaddr_in broadcastAddr;
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(12345);
    broadcastAddr.sin_addr.s_addr = INADDR_BROADCAST;

    sendto(sock, (char*)&request, sizeof(request), 0,
        (struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr));

    while (_running) 
    {
        // 接收响应
        DiscoveryPacket response;
        struct sockaddr_in serverAddr;
        socklen_t addrLen = sizeof(serverAddr);

        ssize_t len = recvfrom(sock, (char*)&response, sizeof(response), 0,
            (struct sockaddr*)&serverAddr, &addrLen);

        if (len == sizeof(DiscoveryPacket) &&
            memcmp(response.magic, "COCO", 4) == 0 &&
            response.type == 1) {

            std::string ip = inet_ntoa(serverAddr.sin_addr);
            int port = ntohs(response.port);

            // 主线程回调
            Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]() 
                {
                if (_discoveryCallback) {
                    _discoveryCallback(ip, port);
                }
                });

            break; // 找到主机后退出
        }
    }

    closesocket(sock);
#ifdef _WIN32
    WSACleanup();
#endif
}