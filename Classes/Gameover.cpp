#include "HelloWorldScene.h"
#include "Gamemode.h"
#include "SimpleAudioEngine.h"
#include "Gameover.h"

USING_NS_CC;

Scene* Gameover::createScene()
{
    return Gameover::create();
}

bool Gameover::init()
{
    if (!Scene::init()) return false;
	auto visiblesize = Director::getInstance()->getVisibleSize();
    CCMenuItemLabel* continueItem = CCMenuItemLabel::create(CCLabelTTF::create("Click here to continue.", "Arial", 60), this, menu_selector(Gameover::ReturnToTitle));
    continueItem->setPosition(Vec2(visiblesize.width / 2, 135));
    CCMenu* continuemenu = CCMenu::createWithItem(continueItem);
    continuemenu->setPosition(Vec2::ZERO);
    this->addChild(continuemenu, 2);
    return true;
}

void Gameover::ReturnToTitle(Ref* pSender)
{
    Scene* helloworldscene = HelloWorld::createScene();
    CCDirector::sharedDirector()->replaceScene(helloworldscene);
}