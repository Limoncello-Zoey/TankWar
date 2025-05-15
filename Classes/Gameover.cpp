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

	label = Label::createWithTTF("Game Over", "fonts/Marker Felt.ttf", 60);
	label->setPosition(Vec2(visiblesize.width / 2, visiblesize.height / 2 + 100));
	label->setColor(Color3B(255, 255, 255));
	this->addChild(label, 1);
	/*if (_win) {
		label->setString("You Win!");
	}
	else {
		label->setString("You Lose!");
	}*/

    CCMenuItemLabel* continueItem = CCMenuItemLabel::create(CCLabelTTF::create("Click here to continue.", "Arial", 60), this, menu_selector(Gameover::ReturnToTitle));
    continueItem->setPosition(Vec2(visiblesize.width / 2, 135));

    CCMenu* continuemenu = CCMenu::createWithItem(continueItem);
    continuemenu->setPosition(Vec2::ZERO);
    this->addChild(continuemenu, 2);

    return true;
}

void Gameover::ChangeText(std::string a)
{
	label->setString(a);
}

void Gameover::ReturnToTitle(Ref* pSender)
{
    Scene* helloworldscene = HelloWorld::createScene();
    CCDirector::sharedDirector()->replaceScene(helloworldscene);
}