#include "WaitingHall.h"
using namespace std;
using namespace cocos2d;
using namespace network;

bool WaitingHall::init()
{
	if (!Scene::init()) return false;

	auto VisibleSize = Director::getInstance()->getVisibleSize();
	auto origin = Director::getInstance()->getVisibleOrigin();


	// add buttons
	auto CreateButton = Button::create("CreateRoomNormal.png", "CreateRoomSelected.png");
	auto JoinButton = Button::create("JoinRoomNormal.png", "JoinRoomSelected.png");

	CreateButton->setPosition(Vec2(VisibleSize.width / 2, VisibleSize.height / 2 + 100-300));
	JoinButton->setPosition(Vec2(VisibleSize.width / 2, VisibleSize.height / 2 - 100-300));

	CreateButton->addTouchEventListener(CC_CALLBACK_2(WaitingHall::onCreateRoomClicked, this));
	JoinButton->addTouchEventListener(CC_CALLBACK_2(WaitingHall::onJoinRoomClicked, this));

	this->addChild(CreateButton);
	this->addChild(JoinButton);

	// Create labels for buttons
	
	//auto label1 = Label::createWithTTF("Create Room", "fonts/Marker Felt.ttf", 24);
	//label1->setPosition(Vec2(VisibleSize.width / 2, VisibleSize.height / 2 + 100 - 200));
	//auto label2 = Label::createWithTTF("Join Room", "fonts/Marker Felt.ttf", 24);
	//label2->setPosition(Vec2(VisibleSize.width / 2, VisibleSize.height / 2 - 100 - 200));
	auto label1 = Label::createWithTTF("Create Room", "fonts/Marker Felt.ttf", 48);
	label1->setPosition(Vec2(0, 0));
	auto label2 = Label::createWithTTF("Join Room", "fonts/Marker Felt.ttf", 48);
	label2->setPosition(Vec2(0, 0));
	CreateButton->addChild(label1);
	JoinButton->addChild(label2);

	// create background
	//auto background = Sprite::create("background.png");
	//background->setPosition(Vec2(VisibleSize.width / 2, VisibleSize.height / 2));

	return true;
}

Scene* WaitingHall::createScene()
{
	return WaitingHall::create();
}

void WaitingHall::onCreateRoomClicked(Ref* sender, cocos2d::ui::Widget::TouchEventType type)
{
	if (type != ui::Widget::TouchEventType::ENDED)
		return;
	// Handle create room button click
	//UdpManager::getInstance()->startHostBroadcast(12345);


}

void WaitingHall::onJoinRoomClicked(Ref* sender, cocos2d::ui::Widget::TouchEventType type)
{
	if (type != ui::Widget::TouchEventType::ENDED) 
		return;
	// Handle join room button click
	auto recall = [=](std::string ip, int port) 
	{
		this->onRoomFound(ip, port);
	};
	//UdpManager::getInstance()->searchHosts(recall);
	//UdpManager::getInstance()->searchHosts(CC_CALLBACK_2(WaitingHall::onRoomFound,UdpManager::getInstance()));
}

void WaitingHall::onRoomFound(std::string ip, int port)
{
	// Handle room found
	log("Room found at %s:%d", ip.c_str(), port);
	// 这里可以添加代码来处理找到的房间，例如显示在UI上
}