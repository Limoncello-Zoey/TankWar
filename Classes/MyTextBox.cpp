#include "MyTextBox.h"

class OutputTextBox : public Node {
public:
    CREATE_FUNC(OutputTextBox);
    bool init() override {
        // ��������
        _bg = DrawNode::create();
        Vec2 points[4] = { Vec2(0,0), Vec2(400,0), Vec2(400,150), Vec2(0,150) };
        _bg->drawPolygon(points, 4, Color4F(0.1, 0.1, 0.1, 0.9), 2, Color4F::GRAY);
        this->addChild(_bg);

        // �ı���ǩ
        _label = Label::createWithTTF("", "fonts/arial.ttf", 24);
        _label->setDimensions(380, 130); // ���߾�
        _label->setPosition(200, 75);
        this->addChild(_label);
        return true;
    }

    void setText(const std::string& text) {
        _label->setString(text);
    }

private:
    DrawNode* _bg;
    Label* _label;
};

// ʹ��ʾ��
auto box = OutputTextBox::create();
box->setPosition(100, 200);
box->setText("Read-only content");
this->addChild(box);