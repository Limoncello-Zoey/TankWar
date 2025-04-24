#ifndef __GAMEMODE_H__
#define __GAMEMODE_H__

#include "cocos2d.h"
#include "Bullet.h"
#include "Tank.h"

class Gamemode : public cocos2d::Scene
{
public:
    //////////VERIABLES//////////
    //map
    static const float GRID_SIZE;
    static std::vector<std::vector<int>> walls;

    //tank
    cocos2d::Vector<Bullet*> activeBullets;
    static Tank* Tank1;
    cocos2d::Vec2 velocity;

    //camera
    cocos2d::Camera* _camera;

    //settings
    float fireCooldown = 0.0f;
    float targetRotation;
    const cocos2d::Size BULLET_SIZE = cocos2d::Size(10, 10);
    const cocos2d::Size TANK1_SIZE = cocos2d::Size(40, 40);


    //////////FUNCTIONS//////////
    //scene
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    CREATE_FUNC(Gamemode);
	virtual void update(float delta) override;

    //map
    void MapSetUp();

    //tanks & bullets
    void initTank();
    static bool CheckPosition(const cocos2d::Vec2& pos);           //检查是否撞墙（f坐标）
    static bool isWall(const cocos2d::Vec2& tilePos);              //检查是否撞墙（int坐标）
    float distancex(const cocos2d::Vec2& Pos);
    float distancey(const cocos2d::Vec2& Pos);
    cocos2d::Vec2 NowPosition(const cocos2d::Vec2& pos);    //f坐标转int坐标
    bool checkCollision(const cocos2d::Vec2& pos);          //检查坦克是否装墙
    void spawnBullet(const cocos2d::Vec2& spawnpos,float radians);//开火
    //void checkBulletCollisions();                           //检查子弹碰撞(墙以及车）


    static bool isCircleCollision(const cocos2d::Vec2& center1, float radius1, const cocos2d::Vec2& center2, float radius2);
    /*void Shoot();*/
    
};

#endif