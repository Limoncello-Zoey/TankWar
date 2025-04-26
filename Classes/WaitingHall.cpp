#include "WaitingHall.h"
using namespace std;
using namespace cocos2d;
using namespace network;

bool WaitingHall::init()
{
	if (!Scene::init()) return false;

}

Scene* WaitingHall::createScene()
{
	return WaitingHall::create();
}
