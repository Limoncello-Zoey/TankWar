#pragma once
#include "cocos2d.h"

class Bullet : public cocos2d::Sprite {
public:
    CREATE_FUNC(Bullet);
    bool init() override;
    void setup(const cocos2d::Vec2& startPos, const cocos2d::Vec2& direction);

private:
    cocos2d::Vec2 velocity;
    void update(float delta) override;
    int maxBounce;
    int currentBounce = 0;
    void handleWallCollision();
    cocos2d::Vec2 _direction;
    void Bullet::destroyBullet();
};
