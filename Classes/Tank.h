#pragma once
#include "cocos2d.h"
/// <summary>
/// 
/// </summary>
class Gamemode;
/// <summary>
/// 
/// </summary>

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

    // 处理坦克受到伤害的方法声明
    void takeDamage();
    // 更新血量条显示的方法声明
    void updateHealthBar();

private:
    // 坦克当前血量
    int currentHealth;
    // 血量条精灵指针
    cocos2d::Sprite* healthBar;
};