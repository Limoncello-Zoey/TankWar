#pragma once
#include "cocos2d.h"
#include "network/WebSocket.h"

class NetworkManager : public cocos2d::network::WebSocket::Delegate {
public:
    static NetworkManager* getInstance();

    void startHost();       // 作为主机监听
    void connectToHost(const std::string& ip); // 作为客户端连接
    void sendTankState(float x, float y, float rotation); // 发送坦克状态

private:
    cocos2d::network::WebSocket* _socket = nullptr;
    bool _isHost = false;

    // WebSocket回调
    void onOpen(cocos2d::network::WebSocket* ws) override;
    void onMessage(cocos2d::network::WebSocket* ws, const cocos2d::network::WebSocket::Data& data) override;
    void onError(cocos2d::network::WebSocket* ws, const cocos2d::network::WebSocket::ErrorCode& error) override;
    void onClose(cocos2d::network::WebSocket* ws) override;
};