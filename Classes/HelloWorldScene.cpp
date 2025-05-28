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

    
    // 修改后的关闭按钮
    auto closeItem = MenuItemImage::create(
        "button_normal.png",      // 正常状态图片
        "button_pressed.png",     // 按下状态图片
        CC_CALLBACK_1(HelloWorld::menuCloseCallback, this)
    );
    // 设置按钮位置
    closeItem->setPosition(Vec2(visibleSize.width / 2, 180));
    // 添加文字标签
    auto quitLabel = Label::createWithTTF("Quit", "fonts/arial.ttf", 60);
    quitLabel->setPosition(closeItem->getContentSize().width / 2, closeItem->getContentSize().height / 2);
    closeItem->addChild(quitLabel);

    // 修改后的开始按钮
    auto gamestartItem = MenuItemImage::create(
        "button_normal.png",
        "button_pressed.png",
        CC_CALLBACK_1(HelloWorld::menuStartCallback, this)
    );
    gamestartItem->setPosition(Vec2(visibleSize.width / 2, 320));
    auto startLabel = Label::createWithTTF("Start", "fonts/arial.ttf", 60);
    startLabel->setPosition(gamestartItem->getContentSize().width / 2, gamestartItem->getContentSize().height / 2);
    gamestartItem->addChild(startLabel);

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
