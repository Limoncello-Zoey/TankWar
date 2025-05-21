#pragma once


#include "cocos2d.h"
#include <cstring>

class Gameover : public cocos2d::Scene
{
public:
    bool _win;
    cocos2d::Label* label;

    static cocos2d::Scene* createScene();

    virtual bool init();

    // a selector callback
    void ReturnToTitle(cocos2d::Ref* pSender);

    // implement the "static create()" method manually
    CREATE_FUNC(Gameover);
    void ChangeText(std::string a);
};
