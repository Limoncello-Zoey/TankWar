#include "NetworkManager.h"


NetworkManager* NetworkManager::getInstance() {
    static NetworkManager instance;
    return &instance;
}

// 主机：监听8080端口
void NetworkManager::startHost() {
    if (_socket) return;
    _isHost = true;
    _socket = new cocos2d::network::WebSocket();
    _socket->init(*this, "ws://0.0.0.0:8080", nullptr, "chat"); // 必须设置协议名（如"chat"）
}

// 客户端：连接指定IP
void NetworkManager::connectToHost(const std::string& ip) {
    if (_socket) return;
    _isHost = false;
    _socket = new cocos2d::network::WebSocket();
    std::string url = "ws://" + ip + ":8080";
    _socket->init(*this, url.c_str(), nullptr, "chat");
}

// 发送坦克状态（二进制数据）
void NetworkManager::sendTankState(float x, float y, float rotation) {
    if (!_socket || !_socket->isConnected()) return;

#pragma pack(push,1)
    struct TankPacket {
        float x;
        float y;
        float rotation;
    };
#pragma pack(pop)

    TankPacket packet{ x, y, rotation };
    _socket->send((const char*)&packet, sizeof(packet));
}

//--- WebSocket回调 ---
void NetworkManager::onOpen(WebSocket* ws) {
    CCLOG("Connected! Waiting for peer...");
}

void NetworkManager::onMessage(WebSocket* ws, const Data& data) {
    if (data.len != 12) return; // 检查是否为合法包（3个float=12字节）

    struct TankPacket {
        float x;
        float y;
        float rotation;
    };
    auto packet = reinterpret_cast<const TankPacket*>(data.bytes);

    // 主线程更新敌方坦克
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