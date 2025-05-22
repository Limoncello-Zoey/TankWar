#pragma once
#include "cocos2d.h"
#include "Network.h"

class Gamemode;

class Tank : public cocos2d::Sprite {
public:
    int heart = 3;
    float targetRotation;
    float fireCooldown = 0.0f;
    cocos2d::Vec2 velocity;
    static float radius;
    //Scene* scene;

    CREATE_FUNC(Tank);
    bool init() override;
    void fire();

    void RegisterControls();


    void update(float delta) override;
    void updateRotation(float delta);
};