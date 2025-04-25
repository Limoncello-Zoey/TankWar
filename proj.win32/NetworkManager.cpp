#include "NetworkManager.h"

//�����뷢��

//�������͵İ���
//1.ÿ��10msͬ������̹�˵�����ͷ����
//2.�����¼���Ӧ�ӵ�����
//�Լ����ܴ��ڵ�״̬��

//˫�������˿�ͳһΪ1145

using namespace std;
using namespace cocos2d;

NetworkManager* NetworkManager::getInstance() {
    static NetworkManager instance;
    return &instance;
}

// ����������8080�˿�
void NetworkManager::startHost() {
    if (_socket) return; //���_socket�Ѿ�����ֵ���ͷ���
    _isHost = true;
    _socket = new cocos2d::network::WebSocket();
    _socket->init(*this, "ws://0.0.0.0:1145", nullptr, "chat"); // ��������Э��������"chat"��
}

// �ͻ��ˣ�����ָ��IP
void NetworkManager::connectToHost(const std::string& ip) {
    if (_socket) return;
    _isHost = false;
    _socket = new cocos2d::network::WebSocket();
    std::string url = "ws://" + ip + ":1145";
    _socket->init(*this, url.c_str(), nullptr, "chat");
}

// ����̹��״̬�����������ݣ�
void NetworkManager::sendTankState(float x, float y, float rotation) {
    if (!_socket || !_socket->isConnected()) return;

#pragma pack(push,1)
    struct TankPacket {
        float x;
        float y;
        float rotation;
    };s
#pragma pack(pop)

    TankPacket packet{ x, y, rotation };
    _socket->send((const char*)&packet, sizeof(packet));
}

//--- WebSocket�ص� ---
void NetworkManager::onOpen(WebSocket* ws) {
    CCLOG("Connected! Waiting for peer...");
}

void NetworkManager::onMessage(WebSocket* ws, const Data& data) {
    if (data.len != 12) return; // ����Ƿ�Ϊ�Ϸ�����3��float=12�ֽڣ�

    struct TankPacket {
        float x;
        float y;
        float rotation;
    };
    auto packet = reinterpret_cast<const TankPacket*>(data.bytes);

    // ���̸߳��µз�̹��
    cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]() {
        auto scene = cocos2d::Director::getInstance()->getRunningScene();
        auto enemy = scene->getChildByName("enemy_tank");
        if (enemy) {
            enemy->setPosition(packet->x, packet->y);
            enemy->setRotation(packet->rotation);
        }
        });
}

void NetworkManager::onError(WebSocket* ws, const ErrorCode& error) {
    CCLOG("Network error: %d", (int)error);
}

void NetworkManager::onClose(WebSocket* ws) {
    CCLOG("Connection closed");
    _socket = nullptr;
}