#include "cocos2d.h"
#include "network/WebSocket.h"

class NetworkManager : public cocos2d::network::WebSocket::Delegate {
public:
    static NetworkManager* getInstance();

    void startHost();
    void connectToHost(const std::string& ip);
    void sendTankState(float x, float y, float rotation);

private:
    cocos2d::network::WebSocket* _socket = nullptr;
    bool _isHost = false;

    // WebSocket»Øµ÷
    virtual void onOpen(cocos2d::network::WebSocket* ws) override;
    virtual void onMessage(cocos2d::network::WebSocket* ws, const cocos2d::network::WebSocket::Data& data) override;
    virtual void onClose(cocos2d::network::WebSocket* ws) override;
    virtual void onError(cocos2d::network::WebSocket* ws, const cocos2d::network::WebSocket::ErrorCode& error) override;
};