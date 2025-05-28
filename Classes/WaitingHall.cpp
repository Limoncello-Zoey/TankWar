#include "WaitingHall.h"
#include "HelloWorldScene.h"
#include "Gamemode.h"
#include "ui/CocosGUI.h"
using namespace cocos2d;
using namespace ui;
using namespace network;

bool WaitingHall::init()
{
	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	if (!Scene::init()) return false;

	auto VisibleSize = Director::getInstance()->getVisibleSize();


	// 修改后的创建房间按钮
	auto creatMatch = MenuItemImage::create(
		"button_normal.png",
		"button_pressed.png",
		CC_CALLBACK_1(WaitingHall::onCreateRoomClicked, this)
	);
	creatMatch->setPosition(Vec2(visibleSize.width / 2, 400));
	auto createLabel = Label::createWithTTF("Create Match", "fonts/arial.ttf", 60);
	createLabel->setPosition(creatMatch->getContentSize().width / 2, creatMatch->getContentSize().height / 2);
	creatMatch->addChild(createLabel);

	// 修改后的加入房间按钮
	auto joinMatch = MenuItemImage::create(
		"button_normal.png",
		"button_pressed.png",
		CC_CALLBACK_1(WaitingHall::onJoinRoomClicked, this)
	);
	joinMatch->setPosition(Vec2(visibleSize.width / 2, 650));
	auto joinLabel = Label::createWithTTF("Join Match", "fonts/arial.ttf", 60);
	joinLabel->setPosition(joinMatch->getContentSize().width / 2, joinMatch->getContentSize().height / 2);
	joinMatch->addChild(joinLabel);

	// 创建包含两个菜单项的菜单
	auto menu = Menu::create(creatMatch, joinMatch, NULL);
	menu->setPosition(Vec2(0, 50));
	this->addChild(menu, 2);

	// add buttons
	/*auto CreateButton = Button::create("CreateRoomNormal.png", "CreateRoomSelected.png");
	auto JoinButton = Button::create("JoinRoomNormal.png", "JoinRoomSelected.png");

	CreateButton->setPosition(Vec2(VisibleSize.width / 2, VisibleSize.height / 2 + 100-300));
	JoinButton->setPosition(Vec2(VisibleSize.width / 2, VisibleSize.height / 2 - 100-300));

	CreateButton->addTouchEventListener(CC_CALLBACK_2(WaitingHall::onCreateRoomClicked, this));
	JoinButton->addTouchEventListener(CC_CALLBACK_2(WaitingHall::onJoinRoomClicked, this));

	this->addChild(CreateButton);
	this->addChild(JoinButton);*/



	Sprite* background = Sprite::create("background2.png");
	background->setPosition(visibleSize.width / 2, visibleSize.height / 2);
	this->addChild(background, 0);

	NetworkManager::getInstance()->Reset();

	return true;
}

Scene* WaitingHall::createScene()
{
	return WaitingHall::create();
}

void WaitingHall::onCreateRoomClicked(Ref* pSender)
{
	/*if (type != ui::Widget::TouchEventType::ENDED)
		return;*/
	// Handle create room button click
	NetworkManager::getInstance()->HostMain();


}

void WaitingHall::onJoinRoomClicked(Ref* pSender)
{
	/*if (type != ui::Widget::TouchEventType::ENDED) 
		return;*/
	NetworkManager::getInstance()->ClientMain();
	// Handle join room button click
}

void WaitingHall::onDemoClicked(Ref* sender, cocos2d::ui::Widget::TouchEventType type)
{
	auto s = Gamemode::create();
	Director::getInstance()->replaceScene(s);
}

