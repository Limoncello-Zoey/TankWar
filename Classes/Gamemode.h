#pragma once

#include "cocos2d.h"
#include "Bullet.h"
#include "Tank.h"
#include "Health.h"

class Gamemode : public cocos2d::Scene
{
public:
    //////////VERIABLES//////////
    //map
    static const float GRID_SIZE;
    static std::vector<std::vector<int>> walls;
    uint16_t _mapid;

    //tank
    static Tank* Tank1;
    static Tank* Tank2;
    static Health* Heart1;
    static int _self;
    cocos2d::Vec2 velocity;
    
    //camera
    cocos2d::Camera* _camera;
    cocos2d::Vec2 Tracing;

    //settings
    float fireCooldown = 0.0f;
    float targetRotation;
    const cocos2d::Size BULLET_SIZE = cocos2d::Size(10, 10);
    const cocos2d::Size TANK1_SIZE = cocos2d::Size(40, 40);
	uint16_t frameindex = 0;


    //////////FUNCTIONS//////////
    //scene
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    CREATE_FUNC(Gamemode);
	virtual void update(float delta) override;

    //map
    int MapSetUp();
    void MapSetUp(uint16_t);
    int randomInt(int min, int max);

    // 检查区域是否安全（没有墙）
    bool isAreaSafe(int startX, int startY, int width, int height);

    // 设置安全的坦克生成区域
    void setSafeSpawnAreas();

    //tanks & bullets
    static Tank* Self();
    static Tank* Other();
    void initTank();
    void initHeart();
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
