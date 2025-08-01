#include "Bullet.h"
#include "Gamemode.h"
#include "Gameover.h"
#include "Tank.h"
#include "audio/include/AudioEngine.h"
#include <iostream>

USING_NS_CC;

float Bullet::radius;

bool Bullet::init() 
{
    std::string texture = "Bullet.png";
    if (!Sprite::initWithFile(texture)) return false;
	this->setCameraMask((unsigned short)CameraFlag::USER1, true);
    this->scheduleUpdate();
	radius = this->getContentSize().width / 2;
    return true;
}

void Bullet::setup(const Vec2& startPos, const cocos2d::Vec2& direction)
{
    
    velocity = direction.getNormalized() * 150.0f;
    this->setPosition(startPos);
    this->setRotation(CC_RADIANS_TO_DEGREES(-velocity.getAngle()));
    this->setAnchorPoint(Vec2(0.5f, 0.5f));
    maxBounce = 5;
}

void Bullet::update(float delta) 
{
    Vec2 newPos = this->getPosition() + velocity * delta;
    
    
    //���ײǽ
    // Ԥ��λ�ü��
    auto scene = dynamic_cast<Gamemode*>(Director::getInstance()->getRunningScene());
    bool isRemoved = false;
    if (scene && !scene->CheckPosition(newPos)) 
    {
        isRemoved = handleWallCollision();
    }
    else 
    {
        this->setPosition(newPos);
    }
    //���ײ̹
    if (!isRemoved)
        if(this->isRunning())
            checkBulletCollisions();
}


void Bullet::checkBulletCollisions()//�����̹����ײ
{
    
    Vec2 center1 = this->getPosition();
    Vec2 center2 = Gamemode::Self()->getPosition();
    
    if (Gamemode::isCircleCollision(center1, Bullet::radius, center2, Tank::radius))
    {
        if (losecondition()) {
            cocos2d::experimental::AudioEngine::stopAll();
            auto gameover = dynamic_cast<Gameover*>(Gameover::createScene());
            gameover->ShowResultImage(false); // ��ʾʧ��ͼƬ
            Director::getInstance()->replaceScene(gameover);
            NetworkManager::getInstance()->SendGameMessage(MessageType::Die, true);
        }
        this->removeFromParent();
    }

	center2 = Gamemode::Other()->getPosition();
    if (Gamemode::isCircleCollision(center1, Bullet::radius, center2, Tank::radius))
    {
        this->removeFromParent();
    }
 
}
bool Bullet::losecondition() {
    int lastheart = Gamemode::Self()->heart;
    if (lastheart > 1) {
        Gamemode::Self()->heart--;
        return false;
    }
    else return true;
}

bool Bullet::handleWallCollision() 
{
    auto scene = dynamic_cast<Gamemode*>(this->getParent());
    bool bounceX = false, bounceY = false;
    Vec2 Pos = this->getPosition();
    Vec2 tilePos = scene->NowPosition(this->getPosition());
    if ((scene->isWall(tilePos + Vec2(1, 0)) || scene->isWall(tilePos + Vec2(-1, 0))) 
        && (scene->isWall(tilePos + Vec2(0, 1)) || scene->isWall(tilePos + Vec2(0, -1)))) 
    {
        float closedisx = scene->distancex(Pos);
        float closedisy = scene->distancey(Pos);
        //std::cout << closedisx << "  " << closedisy << std::endl;
        if (closedisx > closedisy) velocity.y *= -1;
        else velocity.x *= -1;
        currentBounce++;
        if (currentBounce >= maxBounce) 
        {
            this->removeFromParent();
            return true;
        }
        //system("pause");
        
    }
    // ������ܵ�ǽ
    else 
    {
        if (scene->isWall(tilePos + Vec2(1, 0)) || scene->isWall(tilePos + Vec2(-1, 0))) 
        {
            velocity.x *= -1;
            bounceX = true;
        }
        if (scene->isWall(tilePos + Vec2(0, 1)) || scene->isWall(tilePos + Vec2(0, -1))) 
        {
            velocity.y *= -1;
            bounceY = true;
        }

        if (bounceX || bounceY) 
        {
            currentBounce++;
            //system("pause");
            // ������ת�Ƕ�
            this->setRotation(CC_RADIANS_TO_DEGREES(-velocity.getAngle()));

            if (currentBounce >= maxBounce) 
            {
                this->removeFromParent();
                return true;
            }
        }
    }
    return false;
}
