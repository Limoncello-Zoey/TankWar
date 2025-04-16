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
    Tank1->Controls();

    scheduleUpdate();
    Shoot();
    return true;


}

void Gamemode::MapSetUp() 
{
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

    for (int y = 0; y < walls.size(); ++y) 
    {
        for (int x = 0; x < walls[y].size(); ++x) 
        {
            if (walls[y][x] == 1) 
            {
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

void Gamemode::initTank() 
{
    Tank1 = Tank::create();
    Tank1->setPosition(80.0f + GRID_SIZE * 1.1f, 60.0f + GRID_SIZE * 1.1f);
    addChild(Tank1);
}


void Gamemode::Shoot() 
{
    auto listener = EventListenerMouse::create();

    // 鼠标左键射击
    listener->onMouseDown = [=](EventMouse* event) 
        {
            Tank1->fire();
        };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}
void Gamemode::spawnBullet(const Vec2& spawnPos,float radians) 
{
    auto bullet = Bullet::create();
    Vec2 direction = Vec2(cos(radians), sin(radians));
    bullet->setup(spawnPos, direction, 800.0f);
    activeBullets.pushBack(bullet);
    addChild(bullet);
}

Vec2 Gamemode::NowPosition(const Vec2& pos) 
{
    return Vec2(
        static_cast<int>(pos.x / GRID_SIZE),
        static_cast<int>(pos.y / GRID_SIZE)
    );
}
bool Gamemode::checkCollision(const Vec2& pos) 
{
    
    Rect playerRect(
        pos.x - TANK1_SIZE.width / 2,  // 左边界
        pos.y - TANK1_SIZE.height / 2, // 下边界
        TANK1_SIZE.width,            // 宽度
        TANK1_SIZE.height            // 高度
    );

    // 检测周围3x3网格
    int centerX = static_cast<int>(pos.x / GRID_SIZE);
    int centerY = static_cast<int>(pos.y / GRID_SIZE);

    for (int y = centerY - 1; y <= centerY + 1; ++y) 
    {
        for (int x = centerX - 1; x <= centerX + 1; ++x) 
        {
            // 边界检查（同上，保持不变）
            if (y < 0 || x < 0 || y >= walls.size() || x >= walls[y].size())
                continue;

            if (walls[y][x] == 1) 
            {
                // 创建墙体矩形
                Rect wallRect(
                    x * GRID_SIZE,
                    y * GRID_SIZE,
                    GRID_SIZE,
                    GRID_SIZE
                );

                // 矩形相交检测
                if (playerRect.intersectsRect(wallRect)) 
                {
                    return true;
                }
            }
        }
    }
    return false;
}

void Gamemode::checkBulletCollisions() 
{
    for (ssize_t i = 0; i < activeBullets.size(); ++i) 
    {
        auto bullet = activeBullets.at(i);
        Vec2 center1 = bullet->getPosition();
        Vec2 center2 = Tank1->getPosition();
        float radius1 = bullet->getContentSize().width / 2;
        float radius2 = Tank1->getContentSize().width / 2;
        // 检测墙壁碰撞
        if (isCircleCollision(center1, radius1, center2, radius2)) 
        {
            bullet->removeFromParent();
            activeBullets.erase(i--);
            continue;
        }
    }
}

bool Gamemode::CheckPosition(const Vec2& pos) 
{
    Vec2 tilePos = Vec2(
        static_cast<int>((pos.x) / GRID_SIZE),
        static_cast<int>(pos.y / GRID_SIZE));
    return !isWall(tilePos);
}
bool Gamemode::isWall(const Vec2& tilePos) 
{
    
    int x = static_cast<int>(tilePos.x);
    int y = static_cast<int>(tilePos.y);
    return walls[y][x];
}
bool Gamemode::isCircleCollision(const Vec2& center1, float radius1, const Vec2& center2, float radius2) 
{
    float distance = center1.distance(center2);
    return distance <= (radius1 + radius2);
}
