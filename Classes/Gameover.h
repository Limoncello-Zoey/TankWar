#pragma once
#ifndef __GAMEOVER_H__
#define __GAMEOVER_H__

#include "cocos2d.h"

class Gameover : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();

    // a selector callback
    void ReturnToTitle(cocos2d::Ref* pSender);

    // implement the "static create()" method manually
    CREATE_FUNC(Gameover);
};

#endif // __HELLOWORLD_SCENE_H__