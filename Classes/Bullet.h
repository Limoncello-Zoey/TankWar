#pragma once
#include "cocos2d.h"



class Bullet : public cocos2d::Sprite {
public:
    CREATE_FUNC(Bullet);
    bool init() override;
    void setup(const cocos2d::Vec2& startPos, const cocos2d::Vec2& direction);

    cocos2d::Vec2 velocity;
    cocos2d::Vec2 _direction;

    static float radius;
    int maxBounce;
    int currentBounce = 0;

    void update(float delta) override;
    void handleWallCollision();
	void checkBulletCollisions();
};
