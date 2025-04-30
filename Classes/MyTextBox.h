#pragma once
// MyTextBox.h

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "ui/UITextField.h"
class MyTextBox : public cocos2d::Node {
public:
    CREATE_FUNC(MyTextBox);
    virtual bool init() override;
    void setText(const std::string& text);
private:
    cocos2d::Label* _label;       // �ı���ǩ��Ա
    cocos2d::DrawNode* _bg;       // ����ͼ�γ�Ա
};