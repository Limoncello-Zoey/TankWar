#pragma once


#include "cocos2d.h"
#include <cstring>

class Gameover : public cocos2d::Scene {
public:
    bool _win;
    cocos2d::Sprite* _resultImage; // 替换 Label 为 Sprite

    static cocos2d::Scene* createScene();
    virtual bool init();
    void ReturnToTitle(cocos2d::Ref* pSender);
    CREATE_FUNC(Gameover);

    // 新增方法：根据胜负加载图片
    void ShowResultImage(bool isWin);
};
