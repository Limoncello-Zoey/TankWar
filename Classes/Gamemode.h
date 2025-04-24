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
    const float GRID_SIZE = 45.0f;
    std::vector<std::vector<int>> walls;

    //tank
    cocos2d::Vector<Bullet*> activeBullets;
    Tank* Tank1;
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

    //map
    void MapSetUp();

    //tanks & bullets
    void initTank();
    bool CheckPosition(const cocos2d::Vec2& pos);           //����Ƿ�ײǽ��f���꣩
    bool isWall(const cocos2d::Vec2& tilePos);              //����Ƿ�ײǽ��int���꣩
    float distancex(const cocos2d::Vec2& Pos);
    float distancey(const cocos2d::Vec2& Pos);
    cocos2d::Vec2 NowPosition(const cocos2d::Vec2& pos);    //f����תint����
    bool checkCollision(const cocos2d::Vec2& pos);          //���̹���Ƿ�װǽ
    void spawnBullet(const cocos2d::Vec2& spawnpos,float radians);//����
    void checkBulletCollisions();                           //����ӵ���ײ

    bool isCircleCollision(const cocos2d::Vec2& center1, float radius1, const cocos2d::Vec2& center2, float radius2);
    /*void Shoot();*/
    
};

#endif