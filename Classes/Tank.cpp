#include "Bullet.h"
#include "Tank.h"
#include "Gamemode.h"
#include "SimpleAudioEngine.h"
#include "ui/CocosGUI.h"

//
USING_NS_CC;

const float ROTATION_SPEED = 360.0f;
const float ANGLE_THRESHOLD = 5.0f;
float Tank::radius;
//shitrgherg
bool Tank::init() 
{
    if (!Sprite::initWithFile("Tankbody.png")) return false;
    setAnchorPoint(Vec2(0.5f, 0.5f));
    this->scheduleUpdate();
	radius = this->getContentSize().width / 2;
    return true;
}

void Tank::fire() 
{
    auto scene = dynamic_cast<Gamemode*>(this->getParent());
    const float offset = 40.0f;
    float radians = CC_DEGREES_TO_RADIANS(-this->getRotation());

    /*Vec2 tilePos;
    Vec2 newPos;
    do {
        newPos = this->getPosition();
        tilePos = Vec2(
            floor((newPos.x) / 45.0f),
            floor(newPos.y / 45.0f));
    }
    while (tilePos.x < 0 || tilePos.x >= 20 || tilePos.y < 0 || tilePos.y >= 15);*/


    Vec2 spawnPos = /*newPos*/this->getPosition() +
        Vec2(cos(radians), sin(radians)) * offset;

    scene->spawnBullet(spawnPos,radians);
}
void Tank::update(float delta) 
{
    auto scene = dynamic_cast<Gamemode*>(this->getParent());
    // 计算新位置
    float radians = CC_DEGREES_TO_RADIANS(-this->getRotation());
    float forwardx = cos(radians);
    float forwardy = sin(radians);
    
    Vec2 newPosx(this->getPositionX() + forwardx * velocity.length() * delta, this->getPositionY());
    if (!scene->checkCollision(newPosx)) 
    {
        this->setPosition(newPosx);
    }
    Vec2 newPosy(this->getPositionX(), this->getPositionY() + forwardy * velocity.length() * delta);
    if (!scene->checkCollision(newPosy)) 
    {
        this->setPosition(newPosy);
    }
    updateRotation(delta);
    if (fireCooldown > 0)
    {
        fireCooldown -= delta;
    }
}
void Tank::updateRotation(float delta) 
{
    if (velocity != Vec2::ZERO) 
    {
        // 计算目标角度（弧度转角度，转换为顺时针）
        float newAngle = CC_RADIANS_TO_DEGREES(velocity.getAngle()) * -1;

        // 处理角度差值
        float angleDiff = newAngle - targetRotation;
        if (angleDiff > 180) angleDiff -= 360;
        if (angleDiff < -180) angleDiff += 360;

        // 渐进旋转
        if (fabs(angleDiff) > ANGLE_THRESHOLD) {
            float rotateAmount = copysign(ROTATION_SPEED * delta, angleDiff);
            targetRotation += rotateAmount;
        }
        else {
            targetRotation = newAngle;
        }

        // 应用旋转
        this->setRotation(targetRotation);
    }
}


//控制的是自己的，所以对应着发包
void Tank::RegisterControls()
{
    auto listener = EventListenerKeyboard::create();

    // 按键按下
    listener->onKeyPressed = [=](EventKeyboard::KeyCode key, Event* e) 
    {
        switch (key) {
        case EventKeyboard::KeyCode::KEY_W:
            velocity.y = 100.0f; // 上移速度
            break;
        case EventKeyboard::KeyCode::KEY_S:
            velocity.y = -100.0f;
            break;
        case EventKeyboard::KeyCode::KEY_A:
            velocity.x = -100.0f;
            break;
        case EventKeyboard::KeyCode::KEY_D:
            velocity.x = 100.0f;
            break;
        }
    };

    // 按键释放
    listener->onKeyReleased = [=](EventKeyboard::KeyCode key, Event* e) 
    {
        switch (key) {
        case EventKeyboard::KeyCode::KEY_W:
        case EventKeyboard::KeyCode::KEY_S:
            velocity.y = 0.0f;
            break;
        case EventKeyboard::KeyCode::KEY_A:
        case EventKeyboard::KeyCode::KEY_D:
            velocity.x = 0.0f;
            break;
        }
    };
    //Gamemode::update(1.0f);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);


    auto listener2 = EventListenerMouse::create();

    // 鼠标左键射击
    listener2->onMouseDown = [=](EventMouse* event)
    {
        NetworkManager::getInstance()->NetworkManager::SendGameMessage(MessageType::Attack, AttackInfo{true});
        fire();
        auto scene = dynamic_cast<Gamemode*> (Director::getInstance()->getRunningScene());
        auto camera = scene->_camera;
        float radians = CC_DEGREES_TO_RADIANS(-this->getRotation());
        auto direction = Vec2(cos(radians), sin(radians));
        camera->setScale(camera->getScale() + 0.03);
        
		scene->Tracing = scene->Tracing - direction * 20.0f;

    };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener2, this);
}
