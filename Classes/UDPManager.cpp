//��ƽ̨����
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

/*�����ط���UDP�㲥
 * 1. �����㲥�̣߳�����UDP socket������Ϊ�㲥ģʽ���󶨶˿ڣ����տͻ������󲢷�����Ӧ��
 * 2. �ͻ��������̣߳�����UDP socket�����ý��ճ�ʱ�����͹㲥���󣬵ȴ�������Ӧ��
 * 3. ��Դ�����ر�socket������Winsock��Windows���У���
 
// ----------------- �����㲥�߳� -----------------
void UdpManager::hostRoutine(int wsPort) 
{
    // ================= �����ʼ���׶� =================
#ifdef _WIN32
    // Windows ���е� Winsock ��ʼ��
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
    {
        OutputDebugStringA("WSAStartup failed");
        return;
    }
#endif

    // ���� UDP socket��IPv4�����ݱ����ͣ�
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
#ifdef _WIN32
        OutputDebugStringA("Create socket failed");
        WSACleanup();
#endif
        return;
    }

    // ���ù㲥ѡ��ؼ����裩
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

    // �󶨱��ض˿�
    sockaddr_in recvAddr;
    memset(&recvAddr, 0, sizeof(recvAddr));
    recvAddr.sin_family = AF_INET;
    recvAddr.sin_port = htons(12345);      // �����̶��˿�
    recvAddr.sin_addr.s_addr = INADDR_ANY; // ������������ӿ�

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

    // ================= ��ѭ���׶� =================
    while (_running) 
    {
        DiscoveryPacket packet;     // ���ջ�����
        sockaddr_in clientAddr;     // �ͻ��˵�ַ
        socklen_t addrLen = sizeof(clientAddr);

        // �������տͻ��������߳̿��ڴ˴��ȴ���
        ssize_t recvLen = recvfrom(sock, (char*)&packet, sizeof(packet), 0,
            (sockaddr*)&clientAddr, &addrLen);

        // У�����ݰ���Ч��
        if (recvLen == sizeof(DiscoveryPacket) &&
            memcmp(packet.magic, "COCO", 4) == 0 &&
            packet.type == 0) 
        { // ����0Ϊ��������

            // ������Ӧ��
            DiscoveryPacket response;
            response.type = 1;  // ����1Ϊ��Ӧ
            response.ip = clientAddr.sin_addr.s_addr; // �ͻ��˵�IP
            response.port = htons(wsPort); // ת��Ϊ�����ֽ���

            // ���ù㲥Ŀ���ַ
            sockaddr_in broadcastAddr;
            memset(&broadcastAddr, 0, sizeof(broadcastAddr));
            broadcastAddr.sin_family = AF_INET;
            broadcastAddr.sin_port = htons(12345); // Ŀ��˿�
            broadcastAddr.sin_addr.s_addr = INADDR_BROADCAST; // �㲥��ַ

            // ������Ӧ�����Դ���������ʧ�ܣ�
            sendto(sock, (char*)&response, sizeof(response), 0,
                (sockaddr*)&broadcastAddr, sizeof(broadcastAddr));
        }

        // ����CPU���أ��ɸ���������������
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // ================= ��Դ����׶� =================
#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif
}

// ----------------- �ͻ��������߳� -----------------
void UdpManager::clientRoutine() {
    // Windows ��ʼ�� Winsock
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

    // ����UDP Socket
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == -1) return;

    // ���ý��ճ�ʱ��2���ֹͣ�ȴ���Ӧ��
#ifdef _WIN32
    DWORD timeout = 2000; // ����
#else
    struct timeval timeout { 2, 0 }; // ��+΢��
#endif
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
        (char*)&timeout, sizeof(timeout));

    // ���͹㲥�����
    DiscoveryPacket request;
    request.type = 0; // ��������

    // Ŀ���ַΪ�㲥��ַ
    struct sockaddr_in broadcastAddr;
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(12345);    // ���������˿�
    broadcastAddr.sin_addr.s_addr = INADDR_BROADCAST; // 255.255.255.255

    sendto(sock, (char*)&request, sizeof(request), 0,
        (struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr));

    // ��ѭ�����ȴ�������Ӧ
    while (_running) {
        DiscoveryPacket response;
        struct sockaddr_in serverAddr;
        socklen_t addrLen = sizeof(serverAddr);

        // ����������Ӧ
        ssize_t len = recvfrom(sock, (char*)&response, sizeof(response), 0,
            (struct sockaddr*)&serverAddr, &addrLen);

        // ��֤��Ӧ��Ч��
        if (len == sizeof(DiscoveryPacket) &&
            memcmp(response.magic, "COCO", 4) == 0 &&
            response.type == 1) {

            // ����������Ϣ
            std::string ip = inet_ntoa(serverAddr.sin_addr); // ת�ַ���IP
            int port = ntohs(response.port); // ת�����ֽ���

            // �� Cocos2d-x ���߳���ִ�лص���������߳����⣩
            Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]() {
                if (_discoveryCallback) {
                    _discoveryCallback(ip, port); // ���磺����WebSocket
                }
                });

            break; // �ҵ��������˳�ѭ��
        }
    }

    // �ر�Socket
    closesocket(sock);
#ifdef _WIN32
    WSACleanup();
#endif
}*/

void UdpManager::hostRoutine(int wsPort) {
    // ========== Windows������ʼ�� ==========
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa); // ��ʼ��Winsock
#endif

    // ========== ��������Socket ==========
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        cocos2d::log("[UDP] Host socket create failed");
        return;
    }

    // ========== �����ַ���� ==========
    int reuse = 1;
    // SO_REUSEADDR������socket�󶨵���ͬ�˿�
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
        (char*)&reuse, sizeof(reuse));

    // ========== �󶨼����˿� ==========
    sockaddr_in recvAddr;
    memset(&recvAddr, 0, sizeof(recvAddr)); // ��սṹ��
    recvAddr.sin_family = AF_INET;         // IPv4
    recvAddr.sin_port = htons(12345);      // �����˿�
    recvAddr.sin_addr.s_addr = INADDR_ANY; // ������������
    if (bind(sock, (sockaddr*)&recvAddr, sizeof(recvAddr)) == -1) {
        cocos2d::log("[UDP] Bind failed: %d", getSocketError());
        closesocket(sock);
        return;
    }

    // ========== �ͻ���״̬���� ==========
    std::mutex clientMutex; // ����clientMap�Ļ�����
    // ��¼�ͻ��������Ӧʱ�� <�ͻ���IP:Port, �����Ӧʱ��>
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> clientMap;

    // ========== ������ѭ�� ==========
    while (_running) {
        DiscoveryPacket request;
        sockaddr_in clientAddr;
        socklen_t addrLen = sizeof(clientAddr);

        // �������տͻ�������
        ssize_t recvLen = recvfrom(sock, (char*)&request, sizeof(request), 0,
            (sockaddr*)&clientAddr, &addrLen);

        if (recvLen == sizeof(DiscoveryPacket)) { // �յ��������ݰ�
            // У�����ݰ�
            uint32_t received_crc = ntohl(request.crc); // ��ȡ���յ���CRC
            request.crc = 0; // ����Ϊ0�����¼���
            if (memcmp(request.magic, "COCO", 4) == 0 && // ħ��У��
                ntohl(request.type) == 0 &&               // ����0Ϊ����
                received_crc == calculateCRC(request)) {  // CRCУ��

                // ���ɿͻ���Ψһ��ʶ
                std::string clientKey = std::string(inet_ntoa(clientAddr.sin_addr))
                    + ":" + std::to_string(ntohs(clientAddr.sin_port));

                // ����Ƿ���Ҫ��Ӧ��5���ڲ��ظ���Ӧ��
                auto now = std::chrono::steady_clock::now();
                bool needRespond = false;
                {
                    std::lock_guard<std::mutex> lock(clientMutex); // ����
                    auto it = clientMap.find(clientKey);
                    if (it == clientMap.end() || // �¿ͻ���
                        (now - it->second) > std::chrono::seconds(5)) { // ����5��
                        clientMap[clientKey] = now; // ���������Ӧʱ��
                        needRespond = true;
                    }
                }

                if (needRespond) {
                    // ===== ������Ӧ�� =====
                    DiscoveryPacket response;
                    memcpy(response.magic, "COCO", 4);
                    response.type = htonl(1);           // ����1��ʾ��Ӧ
                    response.sequence = request.sequence; // �������к�
                    response.port = htonl(wsPort);        // WebSocket�˿�
                    response.crc = 0; // ����0�ټ���
                    response.crc = htonl(calculateCRC(response));

                    // ����Ŀ���ַ��ʹ�ÿͻ��˵ĵ�ַ�Ͷ˿ڣ�
                    sockaddr_in targetAddr = clientAddr;
                    targetAddr.sin_port = htons(12345); // �ͻ��˼����˿�

                    // ������Ӧ��
                    if (sendto(sock, (char*)&response, sizeof(response), 0,
                        (sockaddr*)&targetAddr, sizeof(targetAddr)) == -1) {
                        cocos2d::log("[UDP] Respond failed: %d", getSocketError());
                    }
                }
            }
        }

        // ========== ����������ڿͻ��ˣ�ÿ60�룩 ==========
        static auto lastClean = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - lastClean).count() > 60) {
            std::lock_guard<std::mutex> lock(clientMutex); // ����
            for (auto it = clientMap.begin(); it != clientMap.end();) {
                // ɾ��5�������޻�Ŀͻ���
                if (now - it->second > std::chrono::minutes(5)) {
                    it = clientMap.erase(it); // ɾ��������
                }
                else {
                    ++it;
                }
            }
            lastClean = now; // ��������ʱ��
        }
    }

    // ========== ��Դ���� ==========
    closesocket(sock); // �ر�socket
#ifdef _WIN32
    WSACleanup(); // Windows���������
#endif
}

void UdpManager::clientRoutine() {
    // ========== Windows ���е�������ʼ�� ==========
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { // ��ʼ�� Winsock 2.2
        cocos2d::log("[UDP] WSAStartup failed"); // ���������־
        return;
    }
#endif

    // ========== ���� UDP Socket ==========
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) { // ���socket�Ƿ񴴽��ɹ�
        cocos2d::log("[UDP] Create socket failed");
#ifdef _WIN32
        WSACleanup(); // Windows ��Ҫ����WSA
#endif
        return;
    }

    // ========== ���ù㲥Ȩ�� ==========
    int broadcast = 1;
    // SO_BROADCAST ѡ�������͹㲥��
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST,
        (char*)&broadcast, sizeof(broadcast)) == -1) {
        cocos2d::log("[UDP] Set broadcast failed");
        closesocket(sock); // �ر���Чsocket
        return;
    }

    // ========== ָ���˱ܲ��� ==========
    const int MAX_RETRIES = 5;       // �������5��
    const int BASE_TIMEOUT_MS = 500; // ������ʱ500����
    int current_retry = 0;          // ��ǰ���Դ���
    uint32_t sequence = time(nullptr) % 0xFFFF; // ���ɳ�ʼ���к�(����ʱ���)

    // ========== �ಥ��ַ�б� ==========
    const std::vector<std::string> MULTICAST_ADDRS = {
        "239.255.255.250",  // SSDPЭ���׼��ַ
        "224.0.0.1"         // ����������������
    };

    // ========== ������ѭ�� ==========
    while (_running && current_retry < MAX_RETRIES) {
        // ===== ��������� =====
        DiscoveryPacket request;
        request.type = htonl(0);    // ����0��ʾ����
        request.sequence = htonl(sequence++); // ���кŵ���
        memcpy(request.magic, "COCO", 4); // 4�ֽ�ħ��
        request.crc = 0; // ����0�ټ���CRC
        request.crc = htonl(calculateCRC(request)); // ���㲢���CRC

        // ===== �����жಥ��ַ�������� =====
        for (const auto& addr : MULTICAST_ADDRS) {
            sockaddr_in destAddr;
            memset(&destAddr, 0, sizeof(destAddr)); // ��սṹ��
            destAddr.sin_family = AF_INET;         // IPv4
            destAddr.sin_port = htons(12345);      // Ŀ��˿�
            destAddr.sin_addr.s_addr = inet_addr(addr.c_str()); // ת����ַ

            // ���������
            if (sendto(sock, (char*)&request, sizeof(request), 0,
                (sockaddr*)&destAddr, sizeof(destAddr)) == -1) {
                cocos2d::log("[UDP] Send error: %d", getSocketError());
            }
        }

        // ===== ��̬������ʱ��ָ���˱��㷨�� =====
        int current_timeout = BASE_TIMEOUT_MS * (1 << current_retry); // ���㵱ǰ��ʱ
#ifdef _WIN32
        DWORD timeout = current_timeout; // Windowsʹ��DWORD����
#else
        struct timeval timeout {         // Linux/Macʹ��timeval
            current_timeout / 1000,     // ��
                (current_timeout % 1000) * 1000 // ΢��
        };
#endif
        // ����socket���ճ�ʱ
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
            (char*)&timeout, sizeof(timeout));

        // ===== ������Ӧѭ�� =====
        time_t start_time = time(nullptr); // ��¼��ʼʱ��
        while (_running && (time(nullptr) - start_time) < 2) { // ���ȴ�2��
            DiscoveryPacket response;
            sockaddr_in serverAddr;
            socklen_t addrLen = sizeof(serverAddr);

            // �����������ݣ��ܳ�ʱ����Ӱ�죩
            ssize_t recvLen = recvfrom(sock, (char*)&response, sizeof(response), 0,
                (sockaddr*)&serverAddr, &addrLen);

            // �յ��������ݰ�
            if (recvLen == sizeof(DiscoveryPacket)) {
                // У�����ݰ���Ч��
                uint32_t received_crc = ntohl(response.crc); // ��ȡ���յ���CRC
                response.crc = 0; // ���ú����¼���
                if (memcmp(response.magic, "COCO", 4) == 0 &&       // ħ��У��
                    ntohl(response.type) == 1 &&                     // ����1Ϊ��Ӧ
                    received_crc == calculateCRC(response)) {       // CRCУ��

                    // ת�������ֽ��򵽱�����ʽ
                    std::string ip = inet_ntoa(serverAddr.sin_addr);
                    int port = ntohl(response.port);

                    // �����߳�ִ�лص���Cocos2d-xҪ��UI���������̣߳�
                    Director::getInstance()->getScheduler()
                        ->performFunctionInCocosThread([=]() {
                        if (_discoveryCallback) {
                            _discoveryCallback(ip, port);
                        }
                            });

                    closesocket(sock); // �ر�socket
                    return; // �ɹ��յ���Ӧ���˳�����
                }
            }
        }

        // ===== ���Լ��� =====
        ++current_retry;
        cocos2d::log("[UDP] Retry %d/%d", current_retry, MAX_RETRIES);
    }

    // ========== ��������ʧ�ܺ�Ĵ��� ==========
    Director::getInstance()->getScheduler()
        ->performFunctionInCocosThread([=]() {
        if (_discoveryCallback) {
            _discoveryCallback("", -1); // ͨ���ղ���֪ͨʧ��
        }
            });

    closesocket(sock); // �ر�socket
#ifdef _WIN32
    WSACleanup(); // Windows���������
#endif
}