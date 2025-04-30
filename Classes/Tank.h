#pragma once
#include "cocos2d.h"
/// <summary>
/// 
/// </summary>
class Gamemode;

class Tank : public cocos2d::Sprite {
public:
    float targetRotation;
    float fireCooldown = 0.0f;
    cocos2d::Vec2 velocity;
    static float radius;

    CREATE_FUNC(Tank);
    bool init() override;
    void fire();

    void RegisterControls();


    void update(float delta) override;
    void updateRotation(float delta);
};