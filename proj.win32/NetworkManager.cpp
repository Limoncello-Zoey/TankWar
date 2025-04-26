#include "NetworkManager.h"

//�����뷢��

//�������͵İ���
//1.ÿ��10msͬ������̹�˵�����ͷ����
//2.�����¼���Ӧ�ӵ�����
//�Լ����ܴ��ڵ�״̬��

//˫�������˿�ͳһΪ1145

using namespace std;
using namespace cocos2d;


USING_NS_CC;
using namespace network;

NetworkManager* NetworkManager::getInstance() {
    static NetworkManager instance;
    return &instance;
}

// ��������
void NetworkManager::startHost() {
    if (_socket) return;
    _isHost = true;
    _socket = new WebSocket();
    _socket->init(*this, "ws://0.0.0.0:1145", nullptr, "");
}

// �ͻ�������
void NetworkManager::connectToHost(const std::string& ip) {
    if (_socket) return;
    _isHost = false;
    _socket = new WebSocket();
    std::string url = "ws://" + ip + ":1145";
    _socket->init(*this, url, nullptr, "");
}

// ����̹��״̬
void NetworkManager::sendTankState(float x, float y, float rotation) {
    if (!_socket || _socket->getReadyState() != WebSocket::State::OPEN) return;

#pragma pack(push,1)
    struct TankPacket {
        float x, y, rotation;
    };
#pragma pack(pop)

    TankPacket packet{ x, y, rotation };
    _socket->send((const unsigned char*)&packet, sizeof(packet));
}

// �ص�ʵ��
void NetworkManager::onOpen(WebSocket* ws) {
    CCLOG("Connected!");
}

void NetworkManager::onMessage(WebSocket* ws, const WebSocket::Data& data) {
    if (data.len != 12) return;

    struct TankPacket { float x, y, rotation; };
    auto packet = reinterpret_cast<const TankPacket*>(data.bytes);

    Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]() {
        auto scene = Director::getInstance()->getRunningScene();
        if (auto enemy = scene->getChildByName("enemy_tank")) {
            enemy->setPosition(packet->x, packet->y);
            enemy->setRotation(packet->rotation);
        }
        });
}

void NetworkManager::onClose(WebSocket* ws) {
    CCLOG("Connection closed");
    _socket = nullptr;
}

void NetworkManager::onError(WebSocket* ws, const WebSocket::ErrorCode& error) {
    CCLOG("Error: %d", (int)error);
    _socket = nullptr;
}