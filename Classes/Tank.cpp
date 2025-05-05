#include "Bullet.h"
#include "Tank.h"
#include "Gamemode.h"
#include "SimpleAudioEngine.h"
#include "ui/CocosGUI.h"
#include "Gameover.h"

USING_NS_CC;

const float ROTATION_SPEED = 360.0f;
const float ANGLE_THRESHOLD = 5.0f;
float Tank::radius;

// 定义血量条图片文件名
const std::string FULL_HEALTH_IMAGE = "full_health.png";
const std::string TWO_THIRDS_HEALTH_IMAGE = "two_thirds_health.png";
const std::string ONE_THIRD_HEALTH_IMAGE = "one_third_health.png";

bool Tank::init()
{
    if (!Sprite::initWithFile("Tankbody.png")) return false;
    setAnchorPoint(Vec2(0.5f, 0.5f));
    RegisterControls();
    this->scheduleUpdate();
    radius = this->getContentSize().width / 2;

    // 初始化血量
    currentHealth = 3;

    // 初始化血量条精灵
    healthBar = Sprite::create(FULL_HEALTH_IMAGE);
    if (healthBar) {
        healthBar->setPosition(Vec2(this->getPositionX(), this->getPositionY() + this->getContentSize().height / 2 + 10));
        this->addChild(healthBar);
    }

    return true;
}

void Tank::fire()
{
    auto scene = dynamic_cast<Gamemode*>(this->getParent());
    const float offset = 40.0f;
    float radians = CC_DEGREES_TO_RADIANS(-this->getRotation());
    Vec2 spawnPos = this->getPosition() +
        Vec2(cos(radians), sin(radians)) * offset;
    scene->spawnBullet(spawnPos, radians);
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

    // 更新血量条位置
    if (healthBar) {
        healthBar->setPosition(Vec2(this->getPositionX(), this->getPositionY() + this->getContentSize().height / 2 + 10));
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
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);


    auto listener2 = EventListenerMouse::create();

    // 鼠标左键射击
    listener2->onMouseDown = [=](EventMouse* event)
        {
            fire();
        };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener2, this);
}

// 处理坦克受到伤害的方法
void Tank::takeDamage()
{
    if (currentHealth > 0) {
        currentHealth--;
        updateHealthBar();
        if (currentHealth == 0) {
            // 坦克死亡逻辑，切换到游戏结束场景
            auto scene = dynamic_cast<Gamemode*>(this->getParent());
            if (scene) {
                Scene* gameover = Gameover::createScene();
                CCDirector::sharedDirector()->replaceScene(gameover);
            }
        }
    }
}

// 更新血量条显示的方法
void Tank::updateHealthBar()
{
    if (healthBar) {
        switch (currentHealth) {
        case 3:
            healthBar->setTexture(FULL_HEALTH_IMAGE);
            break;
        case 2:
            healthBar->setTexture(TWO_THIRDS_HEALTH_IMAGE);
            break;
        case 1:
            healthBar->setTexture(ONE_THIRD_HEALTH_IMAGE);
            break;
        case 0:
            healthBar->setVisible(false);
            break;
        }
    }
}