#pragma once
#include "cocos2d.h"
#include "Network.h"

class Health : public cocos2d::Sprite {
public:
    CREATE_FUNC(Health);
    bool init() override;
    void update(float delta) override;
};