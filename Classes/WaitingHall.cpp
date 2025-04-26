#include "WaitingHall.h"
using namespace std;
using namespace cocos2d;
using namespace network;

bool WaitingHall::init()
{
	if (!Scene::init()) return false;

	auto VisibleSize = Director::getInstance()->getVisibleSize();
	auto origin = Director::getInstance()->getVisibleOrigin();

	auto CreateButton = Button::create("CreateRoomNormal.png", "CreateRoomSelected.png");
	auto JoinButton = Button::create("JoinRoomNormal.png", "JoinRoomSelected.png");

	CreateButton->setPosition(Vec2(VisibleSize.width / 2, VisibleSize.height / 2 + 50));
	JoinButton->setPosition(Vec2(VisibleSize.width / 2, VisibleSize.height / 2 - 50));

	CreateButton->addTouchEventListener(CC_CALLBACK_2(WaitingHall::onIamHostClicked, this));
	
}

Scene* WaitingHall::createScene()
{
	return WaitingHall::create();
}
