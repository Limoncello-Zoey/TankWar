#pragma once
#include "cocos2d.h"

class Gamemode;

class Tank : public cocos2d::Sprite {
public:
    CREATE_FUNC(Tank);
    bool init() override;
    void fire();
    //void Controls();
    void RegisterControls();
private:
    //void Shoot();
    float targetRotation;
    float fireCooldown = 0.0f;
    cocos2d::Vec2 velocity;
    void update(float delta) override;
    void updateRotation(float delta);
};