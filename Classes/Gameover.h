#pragma once


#include "cocos2d.h"
#include <cstring>

class Gameover : public cocos2d::Scene {
public:
    bool _win;
    cocos2d::Sprite* _resultImage; // �滻 Label Ϊ Sprite

    static cocos2d::Scene* createScene();
    virtual bool init();
    void ReturnToTitle(cocos2d::Ref* pSender);
    CREATE_FUNC(Gameover);

    // ��������������ʤ������ͼƬ
    void ShowResultImage(bool isWin);
};
