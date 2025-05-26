#include "Gameover.h"
#include "Gamemode.h"
#include "Tank.h"
#include "Bullet.h"
#include "SimpleAudioEngine.h"
#include "ui/CocosGUI.h"
#include <random> 
#include "AudioEngine.h"


USING_NS_CC;

const float Gamemode::GRID_SIZE = 45.0f;
const float ROTATION_SPEED = 360.0f;   
const float ANGLE_THRESHOLD = 5.0f;
Tank* Gamemode::Tank1 = nullptr;
Tank* Gamemode::Tank2 = nullptr;
Health* Gamemode::Heart1 = nullptr;
Layer* Gamemode::statusLayer = nullptr;
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
    /*MapSetUp();*/
    int bgmId = cocos2d::experimental::AudioEngine::AudioEngine::play2d("background.mp3", true);
    cocos2d::experimental::AudioEngine::setVolume(bgmId, 0.4f);

    initTank();
    initHeart();
    //ֻ��tank1��ֱ���ɼ��̿��Ƶģ�tank2�����ݰ�����
    
    Camera::getDefaultCamera()->setVisible(false);//����Ĭ�����
    auto visiblesize = Director::getInstance()->getVisibleSize();

    _camera=Camera::createOrthographic(visiblesize.width, visiblesize.height, 0, 1000);//�Զ������
	_camera->setCameraFlag(CameraFlag::USER1);
    this->addChild(_camera);

   _camera->setPosition(Vec2(-130, 35));// �޸ĳ�ʼ����
    _camera->setScale(0.6f);
	Tracing = Vec2(0, 0); 
    
    this->setCameraMask((unsigned short)CameraFlag::USER1,true);


    frameindex = 0;
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
    float targetScale = 0.5f;
    if (scale > targetScale)
    {
        scale += (targetScale - scale)*0.05f;
        _camera->setScale(scale);
    }

    auto target = Gamemode::Self();
    if (target) 
    {
        Vec2 compensate = Director::getInstance()->getVisibleSize() / 2 * _camera->getScale();
		Tracing = Tracing + (target->getPosition() - Tracing) * 0.03f;
        _camera->setPosition(Tracing - compensate);
    }

    if (frameindex < 1000)
		frameindex++;
    else 
        NetworkManager::getInstance()->SendGameMessage(MessageType::Position, tankPos);
}

//void Gamemode::MapSetUp() 
//{
//    walls = {
//    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
//    {0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
//    {0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,1,0},
//    {0,1,0,0,0,1,1,0,0,1,0,0,0,1,1,1,0,0,1,0},
//    {0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,1,0},
//    {0,1,0,0,0,0,0,0,0,1,1,1,0,0,0,1,0,0,1,0},
//    {0,1,1,1,1,1,1,0,0,1,0,0,0,0,0,1,0,0,1,0},
//    {0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0},
//    {0,1,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,1,0},
//    {0,1,0,1,1,0,1,0,0,1,0,0,0,1,1,1,0,0,1,0},
//    {0,1,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0},
//    {0,1,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0},
//    {0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0},
//    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
//    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//    };
//
//    const Color4F WALL_COLOR(0.3f, 0.3f, 0.3f, 1.0f);
//    auto drawNode = DrawNode::create();
//
//    for (int y = 0; y < walls.size(); ++y) 
//    {
//        for (int x = 0; x < walls[y].size(); ++x) 
//        {
//            if (walls[y][x] == 1) 
//            {
//                Vec2 pos(x * GRID_SIZE, + y * GRID_SIZE);
//                drawNode->drawSolidRect(
//                    pos,
//                    pos + Vec2(GRID_SIZE, GRID_SIZE),
//                    WALL_COLOR
//                );
//            }
//        }
//    }
//    addChild(drawNode);
//}


// ����������ĸ�������
int Gamemode::randomInt(int min, int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

// ��������Ƿ�ȫ��û��ǽ��
bool Gamemode::isAreaSafe(int startX, int startY, int width, int height) {
    for (int y = startY; y < startY + height; y++) {
        for (int x = startX; x < startX + width; x++) {
            if (y >= walls.size() || x >= walls[y].size() || walls[y][x] == 1) {
                return false;
            }
        }
    }
    return true;
}

// ���ð�ȫ��̹����������
void Gamemode::setSafeSpawnAreas() {
    //// ���½ǰ�ȫ����
    //for (int y = 0; y < SAFE_DISTANCE; y++) {
    //    for (int x = 0; x < SAFE_DISTANCE; x++) {
    //        if (y < walls.size() && x < walls[y].size()) {
    //            walls[y][x] = 0;
    //        }
    //    }
    //}

    //// ���Ͻǰ�ȫ����
    //int mapHeight = walls.size();
    //int mapWidth = mapHeight > 0 ? walls[0].size() : 0;
    //for (int y = mapHeight - SAFE_DISTANCE; y < mapHeight; y++) {
    //    for (int x = mapWidth - SAFE_DISTANCE; x < mapWidth; x++) {
    //        if (y >= 0 && x >= 0 && y < walls.size() && x < walls[y].size()) {
    //            walls[y][x] = 0;
    //        }
    //    }
    //}
}

int Gamemode::MapSetUp()//���ص�ͼid
{
    // Ԥ����ĵ�ͼ����
    std::vector<std::vector<std::vector<int>>> maps = {
        // ��ͼ1��ԭ���ͼ�޸�
        {
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
        },
        {
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
            {0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0},
            {0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0},
            {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
            {0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0},
            {0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0},
            {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
            {0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0},
            {0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0},
            {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
            {0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0},
            {0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0},
            {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
            {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        },
        // ��ͼ2���Խ�ͨ��
        {
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
            {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
            {0,1,0,0,1,1,1,1,1,0,1,1,1,1,1,1,0,0,1,0},
            {0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0},
            {0,1,0,0,0,1,1,0,0,0,1,0,1,1,1,0,0,0,1,0},
            {0,1,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,1,0},
            {0,1,0,0,0,1,1,1,1,1,1,1,1,0,1,0,0,0,1,0},
            {0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0},
            {0,1,0,0,1,0,0,0,1,0,0,1,1,1,1,0,0,0,1,0},
            {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
            {0,1,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,1,0},
            {0,1,0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,1,0},
            {0,1,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,1,0},
            {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        },
        // ��ͼ3�����뱤��
        {
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
            {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
            {0,1,0,0,1,1,1,0,1,1,1,0,1,1,1,1,0,0,1,0},
            {0,1,0,0,0,0,1,0,1,0,1,0,1,0,0,1,0,0,1,0},
            {0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,1,0,0,1,0},
            {0,1,0,0,0,0,1,0,1,1,1,0,1,0,0,1,0,0,1,0},
            {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
            {0,1,0,1,0,0,1,0,1,1,1,0,1,0,0,0,0,0,1,0},
            {0,1,0,1,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0},
            {0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,0,0,0,1,0},
            {0,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,0,0,1,0},
            {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
            {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
            {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        },
        {
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
            {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
            {0,1,0,0,1,0,1,0,0,0,0,0,1,0,1,1,1,0,1,0},
            {0,1,0,0,1,0,1,0,1,0,0,0,1,0,1,1,1,0,1,0},
            {0,1,0,0,0,0,1,0,1,0,0,0,1,0,0,0,0,0,1,0},
            {0,1,0,1,1,1,1,0,1,0,0,0,1,1,1,1,1,0,1,0},
            {0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0},
            {0,1,0,0,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,0},
            {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0},
            {0,1,0,0,0,1,1,1,1,0,0,1,1,1,1,0,1,0,1,0},
            {0,1,0,0,0,1,0,0,0,0,0,0,0,0,1,0,1,0,1,0},
            {0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0},
            {0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0},
            {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        }
    
    };

    // ���ѡ��һ�ŵ�ͼ
    int mapIndex = randomInt(0, maps.size() - 1);

    walls = maps[mapIndex];

    // ȷ��̹�����ɵ���Χû��ǽ
    setSafeSpawnAreas();

    const Color4F WALL_COLOR(0.3f, 0.3f, 0.3f, 1.0f);
    auto drawNode = DrawNode::create();

    for (int y = 0; y < walls.size(); ++y)
    {
        for (int x = 0; x < walls[y].size(); ++x)
        {
            if (walls[y][x] == 1)
            {
                Vec2 pos(x * GRID_SIZE, +y * GRID_SIZE);
                drawNode->drawSolidRect(
                    pos,
                    pos + Vec2(GRID_SIZE, GRID_SIZE),
                    WALL_COLOR
                );
            }
        }
    }
    addChild(drawNode);
	return mapIndex; // ���ص�ͼID
}

void Gamemode::MapSetUp(uint16_t mapid)
{
    // Ԥ����ĵ�ͼ����
    std::vector<std::vector<std::vector<int>>> maps = {
        {
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
        },
        {
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
            {0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0},
            {0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0},
            {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
            {0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0},
            {0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0},
            {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
            {0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0},
            {0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0},
            {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
            {0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0},
            {0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0},
            {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
            {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        },
        // ��ͼ2���Խ�ͨ��
        {
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
            {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
            {0,1,0,0,1,1,1,1,1,0,1,1,1,1,1,1,0,0,1,0},
            {0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0},
            {0,1,0,0,0,1,1,0,0,0,1,0,1,1,1,0,0,0,1,0},
            {0,1,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,1,0},
            {0,1,0,0,0,1,1,1,1,1,1,1,1,0,1,0,0,0,1,0},
            {0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0},
            {0,1,0,0,1,0,0,0,1,0,0,1,1,1,1,0,0,0,1,0},
            {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
            {0,1,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,1,0},
            {0,1,0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,1,0},
            {0,1,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,1,0},
            {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        },
        // ��ͼ3�����뱤��
        {
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
            {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
            {0,1,0,0,1,1,1,0,1,1,1,0,1,1,1,1,0,0,1,0},
            {0,1,0,0,0,0,1,0,1,0,1,0,1,0,0,1,0,0,1,0},
            {0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,1,0,0,1,0},
            {0,1,0,0,0,0,1,0,1,1,1,0,1,0,0,1,0,0,1,0},
            {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
            {0,1,0,1,0,0,1,0,1,1,1,0,1,0,0,0,0,0,1,0},
            {0,1,0,1,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0},
            {0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,0,0,0,1,0},
            {0,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,0,0,1,0},
            {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
            {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
            {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        },
        {
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
            {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
            {0,1,0,0,1,0,1,0,0,0,0,0,1,0,1,1,1,0,1,0},
            {0,1,0,0,1,0,1,0,1,0,0,0,1,0,1,1,1,0,1,0},
            {0,1,0,0,0,0,1,0,1,0,0,0,1,0,0,0,0,0,1,0},
            {0,1,0,1,1,1,1,0,1,0,0,0,1,1,1,1,1,0,1,0},
            {0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0},
            {0,1,0,0,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,0},
            {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0},
            {0,1,0,0,0,1,1,1,1,0,0,1,1,1,1,0,1,0,1,0},
            {0,1,0,0,0,1,0,0,0,0,0,0,0,0,1,0,1,0,1,0},
            {0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0},
            {0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0},
            {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        }
    };

    // ���ѡ��һ�ŵ�ͼ
    walls = maps[mapid];

    // ȷ��̹�����ɵ���Χû��ǽ
    setSafeSpawnAreas();

    const Color4F WALL_COLOR(0.3f, 0.3f, 0.3f, 1.0f);
    auto drawNode = DrawNode::create();

    for (int y = 0; y < walls.size(); ++y)
    {
        for (int x = 0; x < walls[y].size(); ++x)
        {
            if (walls[y][x] == 1)
            {
                Vec2 pos(x * GRID_SIZE, +y * GRID_SIZE);
                drawNode->drawSolidRect(
                    pos,
                    pos + Vec2(GRID_SIZE, GRID_SIZE),
                    WALL_COLOR
                );
            }
        }
    }
    addChild(drawNode);
    return;
}

void Gamemode::initTank() 
{
    Gamemode::Tank1 = Tank::create();
    Gamemode::Tank1->setPosition(GRID_SIZE * 2.6f, GRID_SIZE * 2.6f);
    addChild(Gamemode::Tank1,5);
    Gamemode::Tank2 = Tank::create();
    Gamemode::Tank2->setPosition(GRID_SIZE * 17.4f, GRID_SIZE * 13.4f);
    addChild(Gamemode::Tank2,5);
    Gamemode::Self()->RegisterControls();
}
void Gamemode::initHeart()
{
	/*statusLayer = Layer::create();
    Gamemode::Heart1 = Health::create();
    statusLayer->addChild(Gamemode::Heart1);*/

	Gamemode::Heart1 = Health::create();
    //Gamemode::Heart1->setPosition(GRID_SIZE * 18.0f, GRID_SIZE * 14.0f);
	addChild(Gamemode::Heart1,10);

    Gamemode::Heart1->setPosition(0, 0);
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
        pos.x - TANK1_SIZE.width / 2,  // ��߽�
        pos.y - TANK1_SIZE.height / 2, // �±߽�
        TANK1_SIZE.width,            // ���
        TANK1_SIZE.height            // �߶�
    );

    // �����Χ3x3����
    int centerX = static_cast<int>(pos.x / GRID_SIZE);
    int centerY = static_cast<int>(pos.y / GRID_SIZE);

    for (int y = centerY - 1; y <= centerY + 1; ++y) 
    {
        for (int x = centerX - 1; x <= centerX + 1; ++x) 
        {
            // �߽��飨ͬ�ϣ����ֲ��䣩
            if (y < 0 || x < 0 || y >= walls.size() || x >= walls[y].size())
                continue;

            if (walls[y][x] == 1) 
            {
                // ����ǽ�����
                Rect wallRect(
                    x * GRID_SIZE,
                    y * GRID_SIZE,
                    GRID_SIZE,
                    GRID_SIZE
                );

                // �����ཻ���
                if (playerRect.intersectsRect(wallRect)) 
                {
                    return true;
                }
            }
        }
    }
    return false;
}


bool Gamemode::CheckPosition(const Vec2& pos)//����float���꣬����Ƿ�ײǽ
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
    if (y < 0|| y >= 16) return false;
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
