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
    if (!Sprite::init()) return false;
	this->setCameraMask((unsigned short)CameraFlag::USER1, true);
    this->scheduleUpdate();
	radius = this->getContentSize().width / 2;
    return true;
}

void Bullet::setup(const Vec2& startPos, const cocos2d::Vec2& direction)
{
    std::string texture = "Bullet.png";
    this->initWithFile(texture);
    velocity = direction.getNormalized() * 150.0f;
    this->setPosition(startPos);
    this->setRotation(CC_RADIANS_TO_DEGREES(-velocity.getAngle()));
    this->setAnchorPoint(Vec2(0.5f, 0.5f));
    maxBounce = 5;
}

void Bullet::update(float delta) 
{
    Vec2 newPos = this->getPosition() + velocity * delta;
    //¼ì²â×²Ç½
    // Ô¤²âÎ»ÖÃ¼ì²é
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
    //¼ì²â×²Ì¹
    if (!isRemoved)
        if(this->isRunning())
            checkBulletCollisions();
}


void Bullet::checkBulletCollisions()//¼ì²âÓëÌ¹¿ËÅö×²
{
    
    Vec2 center1 = this->getPosition();
    Vec2 center2 = Gamemode::Self()->getPosition();
    
    if (Gamemode::isCircleCollision(center1, Bullet::radius, center2, Tank::radius))
    {
        auto gameover = dynamic_cast<Gameover*>(Gameover::createScene());
		gameover->_win = false;
		gameover->ChangeText("You Lose!");
        Director::getInstance()->replaceScene(gameover);

		NetworkManager::getInstance()->SendGameMessage(MessageType::Die, true);
    }
 
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
    // ¼ì²éËÄÖÜµÄÇ½
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
            // ¸üÐÂÐý×ª½Ç¶È
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
