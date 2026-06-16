#include "SetIDPopup.hpp"

bool HookedSetIDPopup::init(int current, int begin, int end, gd::string title, gd::string button, bool resetButton, int defaultValue, float offset, bool numberInput, bool arrows) {
    if (!SetIDPopup::init(current, begin, end, title, button, resetButton, defaultValue, offset, numberInput, arrows)) return false;

    auto titleLabel = this->getChildByType<cocos2d::CCLabelBMFont*>(0);
    if (titleLabel) {
        titleLabel->removeFromParent();
        m_mainLayer->addChild(titleLabel);
    }

    return true;
}
