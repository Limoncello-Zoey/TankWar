#include "Gameover.h"
#include "Gamemode.h"
#include "Tank.h"
#include "Bullet.h"
#include "SimpleAudioEngine.h"
#include "ui/CocosGUI.h"

USING_NS_CC;

const float Gamemode::GRID_SIZE = 45.0f;
const float ROTATION_SPEED = 360.0f;   
const float ANGLE_THRESHOLD = 5.0f;
Tank* Gamemode::Tank1 = nullptr;
Tank* Gamemode::Tank2 = nullptr;
int Gamemode::_self = 0;

std::vector<std::vector<int>> Gamemode::walls;

Tank* Gamemode::Self()
{
    if (_self == 1)
        return Tank1;
    if (_self == 2)
        return Tank2;
    return nullptr;
}

Tank* Gamemode::Other()
{
    if (_self == 1)
        return Tank2;
    if (_self == 2)
        return Tank1;
    return nullptr;
}

Scene* Gamemode::createScene()
{
    return Gamemode::create();
}

bool Gamemode::init()
{
    if (!Scene::init()) return false;
    MapSetUp();
    
    initTank();
    
    //只有tank1是直接由键盘控制的，tank2由数据包控制
    
    Camera::getDefaultCamera()->setVisible(false);//禁用默认相机
    auto visiblesize = Director::getInstance()->getVisibleSize();

    _camera=Camera::createOrthographic(visiblesize.width, visiblesize.height, 0, 1000);//自定义相机
	_camera->setCameraFlag(CameraFlag::USER1);
    this->addChild(_camera);

   _camera->setPosition(Vec2(-130, 35));// 修改初始坐标
    _camera->setScale(0.6f);
	Tracing = Vec2(0, 0); 
    
    this->setCameraMask((unsigned short)CameraFlag::USER1,true);
    
	this->scheduleUpdate();

    return true;
}

void Gamemode::update(float delta)
{
    TankPosition tankPos;
    tankPos.x = Gamemode::Self()->getPositionX();
    tankPos.y = Gamemode::Self()->getPositionY();
	tankPos.angle = Gamemode::Self()->getRotation();

    float scale = _camera->getScale();
    float targetScale = 0.4f;
    if (scale > targetScale)
    {
        scale += (targetScale - scale)*0.05f;
        _camera->setScale(scale);
    }

    auto target = Gamemode::Self();
    if (target) 
    {
        Vec2 compensate = Director::getInstance()->getVisibleSize() / 2 * _camera->getScale();
		Tracing = Tracing + (target->getPosition() - Tracing) * 0.1f;
        _camera->setPosition(Tracing - compensate);
    }

    NetworkManager::getInstance()->SendGameMessage(MessageType::Position, tankPos);
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
    Gamemode::Tank1 = Tank::create();
    Gamemode::Tank1->setPosition(80.0f + GRID_SIZE * 1.1f, 60.0f + GRID_SIZE * 1.1f);
    addChild(Gamemode::Tank1);
    Gamemode::Tank2 = Tank::create();
    Gamemode::Tank2->setPosition(120.0f + GRID_SIZE * 13.1f, 80.0f + GRID_SIZE * 11.1f);
    addChild(Gamemode::Tank2);
    Gamemode::Self()->RegisterControls();
}

void Gamemode::spawnBullet(const Vec2& spawnPos,float radians) 
{
    auto bullet = Bullet::create();
    Vec2 direction = Vec2(cos(radians), sin(radians));
    bullet->setup(spawnPos, direction);
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


bool Gamemode::CheckPosition(const Vec2& pos)//输入float坐标，检查是否撞墙
{
    Vec2 tilePos = Vec2(
        floor((pos.x) / GRID_SIZE),
        floor(pos.y / GRID_SIZE));
    return !isWall(tilePos);
}
bool Gamemode::isWall(const Vec2& tilePos)
{
    int x = static_cast<int>(tilePos.x);
    if (x < 0||x>=20) return false;
    int y = static_cast<int>(tilePos.y);
    if (y < 0|| y >= 15) return false;
    return walls[y][x];
}
bool Gamemode::isCircleCollision(const Vec2& center1, float radius1, const Vec2& center2, float radius2) 
{
    float distance = center1.distance(center2);
    return distance <= (radius1 + radius2);
}
float Gamemode::distancex(const Vec2& Pos) {
    Vec2 tilePos = NowPosition(Pos);
    int x = static_cast<int>(tilePos.x);
    int y = static_cast<int>(tilePos.y);
    if (walls[y][x]) return 0;
    float closedisx = 1e7;
    if (walls[y][x - 1]) closedisx = Pos.x - x * GRID_SIZE;
    if (walls[y][x + 1]) closedisx = closedisx > (x + 1) * GRID_SIZE - Pos.x ? (x + 1) * GRID_SIZE - Pos.x : closedisx;
    return closedisx;
}
float Gamemode::distancey(const Vec2& Pos) {
    Vec2 tilePos = NowPosition(Pos);
    int x = static_cast<int>(tilePos.x);
    int y = static_cast<int>(tilePos.y);
    if (walls[y][x]) return 0;
    float closedisy = 1e7;
    if (walls[y - 1][x]) closedisy = Pos.y - y * GRID_SIZE;
    if (walls[y + 1][x]) closedisy = closedisy > (y + 1) * GRID_SIZE -Pos.y  ? (y + 1) * GRID_SIZE - Pos.y : closedisy;
    return closedisy;
}

void Gamemode::SetCamera(float radians, float scale)
{

}