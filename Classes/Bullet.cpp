#include "Bullet.h"
#include "Gamemode.h"
#include "audio/include/AudioEngine.h"

USING_NS_CC;

bool Bullet::init() {
    if (!Sprite::init()) return false;
    this->scheduleUpdate();
    return true;
}

void Bullet::setup(const Vec2& startPos, const cocos2d::Vec2& direction, float angle) {
    std::string texture = "Bullet.png";
    this->initWithFile(texture);
    velocity = direction.getNormalized() * 50.0f;
    this->setPosition(startPos);
    this->setRotation(CC_RADIANS_TO_DEGREES(-velocity.getAngle()));
    this->setAnchorPoint(Vec2(0.5f, 0.5f));
    maxBounce = 5;

    // ���㷽������
    float radians = CC_DEGREES_TO_RADIANS(-angle);
}

void Bullet::update(float delta) {
    Vec2 newPos = this->getPosition() + velocity * delta;

    // Ԥ��λ�ü��
    auto scene = dynamic_cast<Gamemode*>(this->getParent());
    if (scene && !scene->CheckPosition(newPos)) {
        handleWallCollision();
    }
    else {
        this->setPosition(newPos);
    }
}
void Bullet::handleWallCollision() {
    
    auto scene = dynamic_cast<Gamemode*>(this->getParent());
    bool bounceX = false, bounceY = false;
    Vec2 tilePos = scene->NowPosition(this->getPosition());
    if ((scene->isWall(tilePos + Vec2(1, 0)) || scene->isWall(tilePos + Vec2(-1, 0)))&& (scene->isWall(tilePos + Vec2(0, 1)) || scene->isWall(tilePos + Vec2(0, -1)))) {
        float closedisx = scene->distancex(tilePos);
        float closedisy = scene->distancey(tilePos);
        if (closedisx > closedisy) velocity.y *= -1;
        else velocity.x *= -1;
        currentBounce++;
        if (currentBounce >= maxBounce) {
            destroyBullet();
        }
        //system("pause");
    }
    // ������ܵ�ǽ
    else {
        if (scene->isWall(tilePos + Vec2(1, 0)) || scene->isWall(tilePos + Vec2(-1, 0))) {
            velocity.x *= -1;
            bounceX = true;
        }
        if (scene->isWall(tilePos + Vec2(0, 1)) || scene->isWall(tilePos + Vec2(0, -1))) {
            velocity.y *= -1;
            bounceY = true;
        }

        if (bounceX || bounceY) {
            currentBounce++;
            //system("pause");
            // ������ת�Ƕ�
            this->setRotation(CC_RADIANS_TO_DEGREES(-velocity.getAngle()));

            if (currentBounce >= maxBounce) {
                destroyBullet();
            }
        }
    }
}
void Bullet::destroyBullet() {
    this->removeFromParent();
}
