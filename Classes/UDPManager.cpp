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

void UdpManager::stop() 
{
    _running = false;
    if (_udpThread.joinable()) _udpThread.join();
}

/*不带重发的UDP广播
 * 1. 主机广播线程：创建UDP socket，设置为广播模式，绑定端口，接收客户端请求并发送响应。
 * 2. 客户端搜索线程：创建UDP socket，设置接收超时，发送广播请求，等待主机响应。
 * 3. 资源清理：关闭socket，清理Winsock（Windows特有）。
 
// ----------------- 主机广播线程 -----------------
void UdpManager::hostRoutine(int wsPort) 
{
    // ================= 网络初始化阶段 =================
#ifdef _WIN32
    // Windows 特有的 Winsock 初始化
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
    {
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
        (char*)&broadcastEnable, sizeof(broadcastEnable)) == -1) 
    {
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

    if (bind(sock, (sockaddr*)&recvAddr, sizeof(recvAddr)) == -1) 
    {
#ifdef _WIN32
        closesocket(sock);
        WSACleanup();
#else
        close(sock);
#endif
        return;
    }

    // ================= 主循环阶段 =================
    while (_running) 
    {
        DiscoveryPacket packet;     // 接收缓冲区
        sockaddr_in clientAddr;     // 客户端地址
        socklen_t addrLen = sizeof(clientAddr);

        // 阻塞接收客户端请求（线程可在此处等待）
        ssize_t recvLen = recvfrom(sock, (char*)&packet, sizeof(packet), 0,
            (sockaddr*)&clientAddr, &addrLen);

        // 校验数据包有效性
        if (recvLen == sizeof(DiscoveryPacket) &&
            memcmp(packet.magic, "COCO", 4) == 0 &&
            packet.type == 0) 
        { // 类型0为发现请求

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
}*/

void UdpManager::hostRoutine(int wsPort) {
    // ========== Windows网络库初始化 ==========
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa); // 初始化Winsock
#endif

    // ========== 创建监听Socket ==========
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        cocos2d::log("[UDP] Host socket create failed");
        return;
    }

    // ========== 允许地址重用 ==========
    int reuse = 1;
    // SO_REUSEADDR允许多个socket绑定到相同端口
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
        (char*)&reuse, sizeof(reuse));

    // ========== 绑定监听端口 ==========
    sockaddr_in recvAddr;
    memset(&recvAddr, 0, sizeof(recvAddr)); // 清空结构体
    recvAddr.sin_family = AF_INET;         // IPv4
    recvAddr.sin_port = htons(12345);      // 监听端口
    recvAddr.sin_addr.s_addr = INADDR_ANY; // 监听所有网卡
    if (bind(sock, (sockaddr*)&recvAddr, sizeof(recvAddr)) == -1) {
        cocos2d::log("[UDP] Bind failed: %d", getSocketError());
        closesocket(sock);
        return;
    }

    // ========== 客户端状态管理 ==========
    std::mutex clientMutex; // 保护clientMap的互斥锁
    // 记录客户端最后响应时间 <客户端IP:Port, 最后响应时间>
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> clientMap;

    // ========== 主监听循环 ==========
    while (_running) {
        DiscoveryPacket request;
        sockaddr_in clientAddr;
        socklen_t addrLen = sizeof(clientAddr);

        // 阻塞接收客户端请求
        ssize_t recvLen = recvfrom(sock, (char*)&request, sizeof(request), 0,
            (sockaddr*)&clientAddr, &addrLen);

        if (recvLen == sizeof(DiscoveryPacket)) { // 收到完整数据包
            // 校验数据包
            uint32_t received_crc = ntohl(request.crc); // 获取接收到的CRC
            request.crc = 0; // 重置为0后重新计算
            if (memcmp(request.magic, "COCO", 4) == 0 && // 魔数校验
                ntohl(request.type) == 0 &&               // 类型0为请求
                received_crc == calculateCRC(request)) {  // CRC校验

                // 生成客户端唯一标识
                std::string clientKey = std::string(inet_ntoa(clientAddr.sin_addr))
                    + ":" + std::to_string(ntohs(clientAddr.sin_port));

                // 检查是否需要响应（5秒内不重复响应）
                auto now = std::chrono::steady_clock::now();
                bool needRespond = false;
                {
                    std::lock_guard<std::mutex> lock(clientMutex); // 加锁
                    auto it = clientMap.find(clientKey);
                    if (it == clientMap.end() || // 新客户端
                        (now - it->second) > std::chrono::seconds(5)) { // 超过5秒
                        clientMap[clientKey] = now; // 更新最后响应时间
                        needRespond = true;
                    }
                }

                if (needRespond) {
                    // ===== 构造响应包 =====
                    DiscoveryPacket response;
                    memcpy(response.magic, "COCO", 4);
                    response.type = htonl(1);           // 类型1表示响应
                    response.sequence = request.sequence; // 回显序列号
                    response.port = htonl(wsPort);        // WebSocket端口
                    response.crc = 0; // 先填0再计算
                    response.crc = htonl(calculateCRC(response));

                    // 设置目标地址（使用客户端的地址和端口）
                    sockaddr_in targetAddr = clientAddr;
                    targetAddr.sin_port = htons(12345); // 客户端监听端口

                    // 发送响应包
                    if (sendto(sock, (char*)&response, sizeof(response), 0,
                        (sockaddr*)&targetAddr, sizeof(targetAddr)) == -1) {
                        cocos2d::log("[UDP] Respond failed: %d", getSocketError());
                    }
                }
            }
        }

        // ========== 定期清理过期客户端（每60秒） ==========
        static auto lastClean = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - lastClean).count() > 60) {
            std::lock_guard<std::mutex> lock(clientMutex); // 加锁
            for (auto it = clientMap.begin(); it != clientMap.end();) {
                // 删除5分钟内无活动的客户端
                if (now - it->second > std::chrono::minutes(5)) {
                    it = clientMap.erase(it); // 删除过期项
                }
                else {
                    ++it;
                }
            }
            lastClean = now; // 更新清理时间
        }
    }

    // ========== 资源清理 ==========
    closesocket(sock); // 关闭socket
#ifdef _WIN32
    WSACleanup(); // Windows清理网络库
#endif
}

void UdpManager::clientRoutine() {
    // ========== Windows 特有的网络库初始化 ==========
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { // 初始化 Winsock 2.2
        cocos2d::log("[UDP] WSAStartup failed"); // 输出错误日志
        return;
    }
#endif

    // ========== 创建 UDP Socket ==========
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) { // 检查socket是否创建成功
        cocos2d::log("[UDP] Create socket failed");
#ifdef _WIN32
        WSACleanup(); // Windows 需要清理WSA
#endif
        return;
    }

    // ========== 设置广播权限 ==========
    int broadcast = 1;
    // SO_BROADCAST 选项允许发送广播包
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST,
        (char*)&broadcast, sizeof(broadcast)) == -1) {
        cocos2d::log("[UDP] Set broadcast failed");
        closesocket(sock); // 关闭无效socket
        return;
    }

    // ========== 指数退避参数 ==========
    const int MAX_RETRIES = 5;       // 最多重试5次
    const int BASE_TIMEOUT_MS = 500; // 基础超时500毫秒
    int current_retry = 0;          // 当前重试次数
    uint32_t sequence = time(nullptr) % 0xFFFF; // 生成初始序列号(基于时间戳)

    // ========== 多播地址列表 ==========
    const std::vector<std::string> MULTICAST_ADDRS = {
        "239.255.255.250",  // SSDP协议标准地址
        "224.0.0.1"         // 本地网络所有主机
    };

    // ========== 主重试循环 ==========
    while (_running && current_retry < MAX_RETRIES) {
        // ===== 构造请求包 =====
        DiscoveryPacket request;
        request.type = htonl(0);    // 类型0表示请求
        request.sequence = htonl(sequence++); // 序列号递增
        memcpy(request.magic, "COCO", 4); // 4字节魔数
        request.crc = 0; // 先填0再计算CRC
        request.crc = htonl(calculateCRC(request)); // 计算并填充CRC

        // ===== 向所有多播地址发送请求 =====
        for (const auto& addr : MULTICAST_ADDRS) {
            sockaddr_in destAddr;
            memset(&destAddr, 0, sizeof(destAddr)); // 清空结构体
            destAddr.sin_family = AF_INET;         // IPv4
            destAddr.sin_port = htons(12345);      // 目标端口
            destAddr.sin_addr.s_addr = inet_addr(addr.c_str()); // 转换地址

            // 发送请求包
            if (sendto(sock, (char*)&request, sizeof(request), 0,
                (sockaddr*)&destAddr, sizeof(destAddr)) == -1) {
                cocos2d::log("[UDP] Send error: %d", getSocketError());
            }
        }

        // ===== 动态调整超时（指数退避算法） =====
        int current_timeout = BASE_TIMEOUT_MS * (1 << current_retry); // 计算当前超时
#ifdef _WIN32
        DWORD timeout = current_timeout; // Windows使用DWORD类型
#else
        struct timeval timeout {         // Linux/Mac使用timeval
            current_timeout / 1000,     // 秒
                (current_timeout % 1000) * 1000 // 微秒
        };
#endif
        // 设置socket接收超时
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
            (char*)&timeout, sizeof(timeout));

        // ===== 接收响应循环 =====
        time_t start_time = time(nullptr); // 记录开始时间
        while (_running && (time(nullptr) - start_time) < 2) { // 最多等待2秒
            DiscoveryPacket response;
            sockaddr_in serverAddr;
            socklen_t addrLen = sizeof(serverAddr);

            // 阻塞接收数据（受超时设置影响）
            ssize_t recvLen = recvfrom(sock, (char*)&response, sizeof(response), 0,
                (sockaddr*)&serverAddr, &addrLen);

            // 收到完整数据包
            if (recvLen == sizeof(DiscoveryPacket)) {
                // 校验数据包有效性
                uint32_t received_crc = ntohl(response.crc); // 获取接收到的CRC
                response.crc = 0; // 重置后重新计算
                if (memcmp(response.magic, "COCO", 4) == 0 &&       // 魔数校验
                    ntohl(response.type) == 1 &&                     // 类型1为响应
                    received_crc == calculateCRC(response)) {       // CRC校验

                    // 转换网络字节序到本机格式
                    std::string ip = inet_ntoa(serverAddr.sin_addr);
                    int port = ntohl(response.port);

                    // 在主线程执行回调（Cocos2d-x要求UI操作在主线程）
                    Director::getInstance()->getScheduler()
                        ->performFunctionInCocosThread([=]() {
                        if (_discoveryCallback) {
                            _discoveryCallback(ip, port);
                        }
                            });

                    closesocket(sock); // 关闭socket
                    return; // 成功收到响应，退出函数
                }
            }
        }

        // ===== 重试计数 =====
        ++current_retry;
        cocos2d::log("[UDP] Retry %d/%d", current_retry, MAX_RETRIES);
    }

    // ========== 所有重试失败后的处理 ==========
    Director::getInstance()->getScheduler()
        ->performFunctionInCocosThread([=]() {
        if (_discoveryCallback) {
            _discoveryCallback("", -1); // 通过空参数通知失败
        }
            });

    closesocket(sock); // 关闭socket
#ifdef _WIN32
    WSACleanup(); // Windows清理网络库
#endif
}