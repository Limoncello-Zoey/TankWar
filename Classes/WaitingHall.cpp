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

	CreateButton->setPosition(Vec2(VisibleSize.width / 2, VisibleSize.height / 2 + 100-200));
	JoinButton->setPosition(Vec2(VisibleSize.width / 2, VisibleSize.height / 2 - 100-200));

	CreateButton->addTouchEventListener(CC_CALLBACK_2(WaitingHall::onCreateRoomClicked, this));
	JoinButton->addTouchEventListener(CC_CALLBACK_2(WaitingHall::onJoinRoomClicked, this));

	this->addChild(CreateButton);
	this->addChild(JoinButton);

	// Create labels for buttons
	//auto label1 = Label::createWithTTF("Create Room", "fonts/Marker Felt.ttf", 24);
	//label1->setPosition(Vec2(VisibleSize.width / 2, VisibleSize.height / 2 + 100 - 200));
	//auto label2 = Label::createWithTTF("Join Room", "fonts/Marker Felt.ttf", 24);
	//label2->setPosition(Vec2(VisibleSize.width / 2, VisibleSize.height / 2 - 100 - 200));
	//CreateButton->addChild(label1);
	//JoinButton->addChild(label2);

	// create background
	//auto background = Sprite::create("background.png");
	//background->setPosition(Vec2(VisibleSize.width / 2, VisibleSize.height / 2));


	
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

}

void WaitingHall::onJoinRoomClicked(Ref* sender, cocos2d::ui::Widget::TouchEventType type)
{
	if (type != ui::Widget::TouchEventType::ENDED) 
		return;
	// Handle join room button click
	
}