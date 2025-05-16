#pragma once
#include "cocos2d.h"
#include "network/WebSocket.h"
#include "ui/CocosGUI.h"
#include "Network.h"

class WaitingHall : public cocos2d::Scene
{
public:
//veriables
	static void* outputbar;

//functions
	virtual bool init() override;
	static cocos2d::Scene* createScene();
	CREATE_FUNC(WaitingHall);
	void onCreateRoomClicked(Ref* sender, cocos2d::ui::Widget::TouchEventType type);
	void onJoinRoomClicked(Ref* sender, cocos2d::ui::Widget::TouchEventType type);
	void onDemoClicked(Ref* sender, cocos2d::ui::Widget::TouchEventType type);
	//void onRoomFound();
	void printLog(const std::string& message);
};
