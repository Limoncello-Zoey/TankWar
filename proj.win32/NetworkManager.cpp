#include "NetworkManager.h"

//监听与发送

//两种类型的包：
//1.每隔10ms同步己方坦克的坐标和方向角
//2.按照事件响应子弹发射
//以及可能存在的状态包

//双方监听端口统一为1145

using namespace std;
using namespace cocos2d;

NetworkManager* NetworkManager::getInstance() {
    static NetworkManager instance;
    return &instance;
}

// 主机：监听8080端口
void NetworkManager::startHost() {
    if (_socket) return; //如果_socket已经有了值，就返回
    _isHost = true;
    _socket = new cocos2d::network::WebSocket();
    _socket->init(*this, "ws://0.0.0.0:1145", nullptr, "chat"); // 必须设置协议名（如"chat"）
}

// 客户端：连接指定IP
void NetworkManager::connectToHost(const std::string& ip) {
    if (_socket) return;
    _isHost = false;
    _socket = new cocos2d::network::WebSocket();
    std::string url = "ws://" + ip + ":1145";
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
    };s
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