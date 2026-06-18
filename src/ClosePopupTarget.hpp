#pragma once

class ClosePopupTarget : public cocos2d::CCSprite {
public:
    static ClosePopupTarget* create();
    bool init();

    cocos2d::CCLayerGradient* m_gradient;

    bool pointIsCloseEnough(cocos2d::CCPoint pos);
    void hover();
    void unhover();
};
