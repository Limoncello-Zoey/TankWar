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

    
    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = CCMenuItemLabel::create(CCLabelTTF::create("Quit", "Arial", 60), this, menu_selector(HelloWorld::menuCloseCallback));

    if (closeItem == nullptr ||
        closeItem->getContentSize().width <= 0 ||
        closeItem->getContentSize().height <= 0)
    {
        problemLoading("'CloseNormal.png' and 'CloseSelected.png'");
    }
    else
    {
        float x = origin.x + visibleSize.width - closeItem->getContentSize().width/2;
        float y = origin.y + closeItem->getContentSize().height/2;
        closeItem->setPosition(Vec2(visibleSize.width / 2,55));
    }
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 2);
    Sprite* background = Sprite::create("background.png");
    background->setPosition(visibleSize.width / 2, visibleSize.height / 2);
    this->addChild(background, 0); 

    CCMenuItemLabel* gamestartItem = CCMenuItemLabel::create(CCLabelTTF::create("GameStart","Arial",60),this,menu_selector(HelloWorld::menuStartCallback));
    gamestartItem->setPosition(Vec2(visibleSize.width / 2, 135));
    CCMenu* startmenu = CCMenu::createWithItem(gamestartItem);
    startmenu->setPosition(Vec2::ZERO);
    this->addChild(startmenu,2);
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
	auto waitinghall = Gamemode::createScene();
	Director::getInstance()->replaceScene(waitinghall);
}
