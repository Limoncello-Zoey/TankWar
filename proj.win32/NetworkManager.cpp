#include "NetworkManager.h"

//监听与发送

//两种类型的包：
//1.每隔10ms同步己方坦克的坐标和方向角
//2.按照事件响应子弹发射
//以及可能存在的状态包

//双方监听端口统一为1145

using namespace std;
using namespace cocos2d;


USING_NS_CC;
using namespace network;

NetworkManager* NetworkManager::getInstance() {
    static NetworkManager instance;
    return &instance;
}

// 主机监听
void NetworkManager::startHost() {
    if (_socket) return;
    _isHost = true;
    _socket = new WebSocket();
    _socket->init(*this, "ws://0.0.0.0:1145", nullptr, "");
}

// 客户端连接
void NetworkManager::connectToHost(const std::string& ip) {
    if (_socket) return;
    _isHost = false;
    _socket = new WebSocket();
    std::string url = "ws://" + ip + ":1145";
    _socket->init(*this, url, nullptr, "");
}

// 发送坦克状态
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

// 回调实现
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