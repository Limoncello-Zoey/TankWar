#include"Health.h"
#include"Tank.h"
#include"Gamemode.h"
#include "Bullet.h"
#include "SimpleAudioEngine.h"
#include "ui/CocosGUI.h"

USING_NS_CC;

bool Health::init()
{
    if (!Sprite::initWithFile("full_health.png")) return false;
    this->setPosition(45.0f * 18.0f, 45.0f * 14.0f);
    this->setCameraMask((unsigned short)CameraFlag::DEFAULT, true);
    this->scheduleUpdate();
    return true;
}

void Health::update(float delta) {
    int lastheart = Gamemode::Self()->heart;
    this->setPosition(Gamemode::Self()->getPositionX(), Gamemode::Self()->getPositionY() + 60);
    if (lastheart == 3) 
    {
        this->setTexture("full_health.png");
    }
    if (lastheart == 2) 
    {
        this->setTexture("two_thirds_health.png");
    }
    if (lastheart == 1) 
    {
        this->setTexture("one_third_health.png");
    }
}
