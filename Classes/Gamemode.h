#ifndef __GAMEMODE_H__
#define __GAMEMODE_H__

#include "cocos2d.h"
#include "Bullet.h"
#include "Tank.h"

class Gamemode : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    bool CheckPosition(const cocos2d::Vec2& pos);
    bool isWall(const cocos2d::Vec2& tilePos);
    cocos2d::Vec2 NowPosition(const cocos2d::Vec2& pos);
    bool checkCollision(const cocos2d::Vec2& pos);
    void spawnBullet(const cocos2d::Vec2& spawnpos,float radians);
    virtual bool init();
    CREATE_FUNC(Gamemode);
    cocos2d::Vector<Bullet*> activeBullets;
    void checkBulletCollisions();
private:
    Tank* Tank1;
    cocos2d::Vec2 velocity;
    bool isCircleCollision(const cocos2d::Vec2& center1, float radius1, const cocos2d::Vec2& center2, float radius2);
    float fireCooldown = 0.0f;
    float targetRotation;
    void Shoot();
    //void update(float delta) override;
    //void updateRotation(float delta);
    
    bool BcheckCollision(const cocos2d::Vec2& pos);
    std::vector<std::vector<int>> walls;
    void MapSetUp();
    void initTank();
    //void Gamemode::Controls();
    const float GRID_SIZE = 45.0f;
    const cocos2d::Size BULLET_SIZE = cocos2d::Size(10, 10);
    const cocos2d::Size TANK1_SIZE = cocos2d::Size(40, 40);
};

#endif