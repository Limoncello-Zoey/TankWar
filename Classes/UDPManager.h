#pragma once
#include <functional>
#include <string>
#include <thread>
#include "cocos2d.h"

class UdpManager {
public:
    static UdpManager* getInstance(); // ����ģʽ��ȡʵ��

    // ���������㲥��������WebSocket����˿ڣ�
    void startHostBroadcast(int wsPort);

    // �ͻ�����������������������������Ļص�������
    void searchHosts(std::function<void(std::string ip, int port)> callback);

    // ֹͣ����UDP������������Դ��
    void stop();

private:
    bool _running = false;          // �߳����б�־
    std::thread _udpThread;         // UDP�����߳�
    std::function<void(std::string, int)> _discoveryCallback; // ���������Ļص�

    // �����㲥�߼���������WebSocket�˿ڣ�
    void hostRoutine(int wsPort);

    // �ͻ��������߼�
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