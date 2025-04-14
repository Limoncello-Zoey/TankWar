#include "Gamemode.h"
#include "SimpleAudioEngine.h"
#include "ui/CocosGUI.h"

USING_NS_CC;

const float ROTATION_SPEED = 360.0f;   
const float ANGLE_THRESHOLD = 5.0f;

Scene* Gamemode::createScene()
{
    return Gamemode::create();
}


// Print useful error message instead of segfaulting when files are not there.


// on "init" you need to initialize your instance
bool Gamemode::init()
{

    if (!Scene::init()) return false;
    MapSetUp();
    initTank();
    //Tank1 = Sprite::create("Tankbody.png");
    //Tank1->setAnchorPoint(Vec2(0.5f, 0.5f));
    //Tank1->setPosition(40.0f + GRID_SIZE * 1.1f,20.0f + GRID_SIZE * 1.1f);
    //addChild(Tank1);

    Tank1->Controls();
    //bool checkCollision(const cocos2d::Vec2& pos);
    scheduleUpdate();
    Shoot();
    return true;


}

void Gamemode::MapSetUp() {
    walls = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
    {0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,1,0},
    {0,1,0,0,0,1,1,0,0,1,0,0,0,1,1,1,0,0,1,0},
    {0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,1,0},
    {0,1,0,0,0,0,0,0,0,1,1,1,0,0,0,1,0,0,1,0},
    {0,1,1,1,1,1,1,0,0,1,0,0,0,0,0,1,0,0,1,0},
    {0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0},
    {0,1,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,1,0},
    {0,1,0,1,1,0,1,0,0,1,0,0,0,1,1,1,0,0,1,0},
    {0,1,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0},
    {0,1,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0},
    {0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    };

    const Color4F WALL_COLOR(0.3f, 0.3f, 0.3f, 1.0f);
    auto drawNode = DrawNode::create();

    for (int y = 0; y < walls.size(); ++y) {
        for (int x = 0; x < walls[y].size(); ++x) {
            if (walls[y][x] == 1) {
                Vec2 pos(x * GRID_SIZE, + y * GRID_SIZE);
                drawNode->drawSolidRect(
                    pos,
                    pos + Vec2(GRID_SIZE, GRID_SIZE),
                    WALL_COLOR
                );
            }
        }
    }
    addChild(drawNode);
}

void Gamemode::initTank() {
    Tank1 = Tank::create();
    Tank1->setPosition(80.0f + GRID_SIZE * 1.1f, 60.0f + GRID_SIZE * 1.1f);
    addChild(Tank1);
}

/*void Gamemode::Controls() {
    auto listener = EventListenerKeyboard::create();

    // ��������
    listener->onKeyPressed = [=](EventKeyboard::KeyCode key, Event* e) {
        switch (key) {
        case EventKeyboard::KeyCode::KEY_W:
            velocity.y = 100.0f; // �����ٶ�
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

    // �����ͷ�
    listener->onKeyReleased = [=](EventKeyboard::KeyCode key, Event* e) {
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
}*/
void Gamemode::Shoot() {
    auto listener = EventListenerMouse::create();

    // ���������
    listener->onMouseDown = [=](EventMouse* event) {
        //if (fireCooldown <= 0) {
            Tank1->fire();
            //fireCooldown = 0.2f; // 200ms��ȴ
        //}
        };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}
void Gamemode::spawnBullet(const Vec2& spawnPos,float radians) {
    auto bullet = Bullet::create();
    Vec2 direction = Vec2(cos(radians), sin(radians));
    bullet->setup(spawnPos, direction, 800.0f);
    activeBullets.pushBack(bullet);
    addChild(bullet);
}
/*void Gamemode::update(float delta) {
    // ������λ��
    float radians = CC_DEGREES_TO_RADIANS(-Tank1->getRotation());
    float forwardx = cos(radians);
    float forwardy = sin(radians);
   
    Vec2 newPosx(Tank1->getPositionX() + forwardx *velocity.length() * delta, Tank1->getPositionY());
    if (!checkCollision(newPosx)) {
        Tank1->setPosition(newPosx);
    }
    Vec2 newPosy(Tank1->getPositionX(), Tank1->getPositionY() + forwardy *velocity.length() * delta);
    if (!checkCollision(newPosy)) {
        Tank1->setPosition(newPosy);
    }
    updateRotation(delta);
    if (fireCooldown > 0) {
        fireCooldown -= delta;
    }
    checkBulletCollisions();
}
void Gamemode::updateRotation(float delta) {
    if (velocity != Vec2::ZERO) {
        // ����Ŀ��Ƕȣ�����ת�Ƕȣ�ת��Ϊ˳ʱ�룩
        float newAngle = CC_RADIANS_TO_DEGREES(velocity.getAngle()) * -1;

        // ����ǶȲ�ֵ
        float angleDiff = newAngle - targetRotation;
        if (angleDiff > 180) angleDiff -= 360;
        if (angleDiff < -180) angleDiff += 360;

        // ������ת
        if (fabs(angleDiff) > ANGLE_THRESHOLD) {
            float rotateAmount = copysign(ROTATION_SPEED * delta, angleDiff);
            targetRotation += rotateAmount;
        }
        else {
            targetRotation = newAngle;
        }

        // Ӧ����ת
        Tank1->setRotation(targetRotation);
    }
}*/
Vec2 Gamemode::NowPosition(const Vec2& pos) {
    return Vec2(
        static_cast<int>(pos.x / GRID_SIZE),
        static_cast<int>(pos.y / GRID_SIZE)
    );
}
bool Gamemode::checkCollision(const Vec2& pos) {
    /*const float TANK1_RADIUS = 20.0f;

    // �����Χ3x3����
    int centerX = static_cast<int>(pos.x / GRID_SIZE);
    int centerY = static_cast<int>(pos.y / GRID_SIZE);

    for (int y = centerY - 1; y <= centerY + 1; ++y) {
        for (int x = centerX - 1; x <= centerX + 1; ++x) {
            // �߽���
            if (y < 0 || x < 0 || y >= walls.size() || x >= walls[y].size())
                continue;

            // ǽ���
            if (walls[y][x] == 1) {
                Rect wallRect(
                    x * GRID_SIZE - 15.0f,
                    y * GRID_SIZE - 45.0f,
                    GRID_SIZE,
                    GRID_SIZE
                );

                // Բ���������ײ���
                Vec2 closestPoint(
                    std::max(wallRect.getMinX(), std::min(pos.x, wallRect.getMaxX())),
                    std::max(wallRect.getMinY(), std::min(pos.y, wallRect.getMaxY()))
                );

                Vec2 distVec = pos - closestPoint;
                if (distVec.lengthSquared() <= TANK1_RADIUS * TANK1_RADIUS) {
                    return true;
                }
            }
        }
    }
    return false;
    */
    Rect playerRect(
        pos.x - TANK1_SIZE.width / 2,  // ��߽�
        pos.y - TANK1_SIZE.height / 2, // �±߽�
        TANK1_SIZE.width,            // ���
        TANK1_SIZE.height            // �߶�
    );

    // �����Χ3x3����
    int centerX = static_cast<int>(pos.x / GRID_SIZE);
    int centerY = static_cast<int>(pos.y / GRID_SIZE);

    for (int y = centerY - 1; y <= centerY + 1; ++y) {
        for (int x = centerX - 1; x <= centerX + 1; ++x) {
            // �߽��飨ͬ�ϣ����ֲ��䣩
            if (y < 0 || x < 0 || y >= walls.size() || x >= walls[y].size())
                continue;

            if (walls[y][x] == 1) {
                // ����ǽ�����
                Rect wallRect(
                    x * GRID_SIZE,
                    y * GRID_SIZE,
                    GRID_SIZE,
                    GRID_SIZE
                );

                // �����ཻ���
                if (playerRect.intersectsRect(wallRect)) {
                    return true;
                }
            }
        }
    }
    return false;
}
/*bool Gamemode::BcheckCollision(const Vec2& pos) {
    Rect playerRect(
        pos.x - BULLET_SIZE.width / 2,  // ��߽�
        pos.y - BULLET_SIZE.height / 2, // �±߽�
        BULLET_SIZE.width,            // ���
        BULLET_SIZE.height            // �߶�
    );

    // �����Χ3x3����
    int centerX = static_cast<int>(pos.x / GRID_SIZE);
    int centerY = static_cast<int>(pos.y / GRID_SIZE);

    for (int y = centerY - 1; y <= centerY + 1; ++y) {
        for (int x = centerX - 1; x <= centerX + 1; ++x) {
            // �߽��飨ͬ�ϣ����ֲ��䣩
            if (y < 0 || x < 0 || y >= walls.size() || x >= walls[y].size())
                continue;

            if (walls[y][x] == 1) {
                // ����ǽ�����
                Rect wallRect(
                    x * GRID_SIZE + 8.5f,
                    y * GRID_SIZE,
                    GRID_SIZE,
                    GRID_SIZE
                );

                // �����ཻ���
                if (playerRect.intersectsRect(wallRect)) {
                    return true;
                }
            }
        }
    }
    return false;
}*/
void Gamemode::checkBulletCollisions() {
    for (ssize_t i = 0; i < activeBullets.size(); ++i) {
        auto bullet = activeBullets.at(i);
        Vec2 center1 = bullet->getPosition();
        Vec2 center2 = Tank1->getPosition();
        float radius1 = bullet->getContentSize().width / 2;
        float radius2 = Tank1->getContentSize().width / 2;
        // ���ǽ����ײ
        if (isCircleCollision(center1, radius1, center2, radius2)) {
            bullet->removeFromParent();
            activeBullets.erase(i--);
            continue;
        }

        
    }
    /*for (auto bullet : activeBullets) {
        system("pause");
        // ������ɫ����ײ
        if (bullet->getBoundingBox().intersectsRect(Tank1->getBoundingBox())) {
            bullet->removeFromParent();
            //activeBullets.erase(i--);
            continue;
        }
    }*/
}
/*bool Gamemode::BcheckCollision(const Vec2& pos) {

    for (auto bullet : activeBullets) {
        // ������ɫ����ײ
        if (bullet->getBoundingBox().intersectsRect(Tank1->getBoundingBox())) {
            return true;
        }
        return false;
    }
}*/
bool Gamemode::CheckPosition(const Vec2& pos) {
    Vec2 tilePos = Vec2(
        static_cast<int>((pos.x) / GRID_SIZE),
        static_cast<int>(pos.y / GRID_SIZE));
    return !isWall(tilePos);
}
bool Gamemode::isWall(const Vec2& tilePos) {
    
    int x = static_cast<int>(tilePos.x);
    int y = static_cast<int>(tilePos.y);
    return walls[y][x];
}
bool Gamemode::isCircleCollision(const Vec2& center1, float radius1, const Vec2& center2, float radius2) {
    float distance = center1.distance(center2);
    return distance <= (radius1 + radius2);
}
