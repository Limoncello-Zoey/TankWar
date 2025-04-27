#pragma once
#include <functional>
#include <string>
#include <thread>
#include "cocos2d.h"

class UdpManager {
public:
    static UdpManager* getInstance();

    // ���������㲥
    void startHostBroadcast(int wsPort);

    // �ͻ�����������
    void searchHosts(std::function<void(std::string ip, int port)> callback);

    // ֹͣ����UDP����
    void stop();

private:
    bool _running = false;
    std::thread _udpThread;
    std::function<void(std::string, int)> _discoveryCallback;

    void hostRoutine(int wsPort);
    void clientRoutine();
};

#pragma pack(push, 1) // �ڴ����
struct DiscoveryPacket {
    char magic[4] = { 'C','O','C','O' }; // Э���ʶ
    uint16_t version = 0x0001;         // Э��汾
    uint8_t  type;                     // 0:���� 1:��Ӧ
    uint32_t ip;                       // ����IP(�����ֽ���)
    uint16_t port;                     // ����WebSocket�˿�
};
#pragma pack(pop)