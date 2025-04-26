#pragma once
#include "cocos2d.h"
#include "network/WebSocket.h"
#include "ui/CocosGUI.h"
using namespace std;
using namespace cocos2d;
using namespace network;
using namespace ui;

class WaitingHall : public Scene
{
public:
//veriables


//functions
	virtual bool init() override;
	static Scene* createScene();
	CREATE_FUNC(WaitingHall);
	void onCreateRoomClicked(Ref* sender, cocos2d::ui::Widget::TouchEventType type);
	void onJoinRoomClicked(Ref* sender, cocos2d::ui::Widget::TouchEventType type);

};
