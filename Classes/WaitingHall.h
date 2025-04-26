#pragma once
#include "cocos2d.h"
#include "network/WebSocket.h"
using namespace std;
using namespace cocos2d;
using namespace network;

class WaitingHall : public Scene
{
public:
//veriables


//functions
	virtual bool init() override;
	static Scene* createScene();
	CREATE_FUNC(WaitingHall);
	
};
