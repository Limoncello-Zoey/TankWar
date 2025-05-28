#include "HelloWorldScene.h"
#include "Gamemode.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;

Scene* HelloWorld::createScene()
{
    return HelloWorld::create();
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Scene::init() )
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    
    // 创建关闭菜单项
    auto closeItem = MenuItemLabel::create(
        Label::createWithTTF("Quit", "fonts/arial.ttf", 60),
        CC_CALLBACK_1(HelloWorld::menuCloseCallback, this)
    );
    closeItem->setPosition(Vec2(visibleSize.width / 2, 150));

    // 创建游戏开始菜单项
    auto gamestartItem = MenuItemLabel::create(
        Label::createWithTTF("Start", "fonts/arial.ttf", 60),
        CC_CALLBACK_1(HelloWorld::menuStartCallback, this)
    );
    gamestartItem->setPosition(Vec2(visibleSize.width / 2, 250));

    // 创建包含两个菜单项的菜单
    auto menu = Menu::create(closeItem, gamestartItem, NULL);
    menu->setPosition(Vec2(0,50));
    this->addChild(menu, 2);


    Sprite* background = Sprite::create("background.png");
    background->setPosition(visibleSize.width / 2, visibleSize.height / 2);
    this->addChild(background, 0); 
    return true;
}



void HelloWorld::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}

void HelloWorld::menuStartCallback(Ref* pSender)
{
    //Scene* gamemode = Gamemode::createScene();
    //CCDirector::sharedDirector()->replaceScene(gamemode);
	auto waitinghall = WaitingHall::createScene();
	Director::getInstance()->replaceScene(waitinghall);
}
