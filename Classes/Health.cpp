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
    this->scheduleUpdate();
    return true;
}

void Health::update(float delta) {
    int lastheart = Gamemode::Self()->heart;
    if (lastheart == 2) {
        this->setTexture("two_thirds_health.png");
        
    }
    if (lastheart == 1) {
        this->setTexture("one_third_health.png");
    }
}
