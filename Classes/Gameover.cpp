#include "HelloWorldScene.h"
#include "Gamemode.h"
#include "SimpleAudioEngine.h"
#include "Gameover.h"

USING_NS_CC;

Scene* Gameover::createScene()
{
    return Gameover::create();
}

bool Gameover::init() {
    if (!Scene::init()) return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 创建返回按钮（保留原有逻辑）
    CCMenuItemLabel* continueItem = CCMenuItemLabel::create(
        CCLabelTTF::create("Click here to continue.", "Arial", 60),
        this,
        menu_selector(Gameover::ReturnToTitle)
    );
    continueItem->setPosition(Vec2(visibleSize.width / 2, 135));
    CCMenu* continuemenu = CCMenu::createWithItem(continueItem);
    continuemenu->setPosition(Vec2::ZERO);
    this->addChild(continuemenu, 2);

    return true;
}

void Gameover::ShowResultImage(bool isWin) {
    std::string imagePath = isWin ? "YouWin.png" : "YouLose.png";

    // 创建图片 Sprite
    _resultImage = Sprite::create(imagePath);
    if (_resultImage) {
        auto visibleSize = Director::getInstance()->getVisibleSize();
        _resultImage->setPosition(
            Vec2(visibleSize.width / 2, visibleSize.height / 2) + Vec2(0, 0)
       
        );
        this->addChild(_resultImage, 1);
    }
}
void Gameover::ReturnToTitle(Ref* pSender)
{
    Scene* helloworldscene = HelloWorld::createScene();
    CCDirector::sharedDirector()->replaceScene(helloworldscene);
}