#include "ClosePopupTarget.hpp"


ClosePopupTarget* ClosePopupTarget::create() {
    auto ret = new ClosePopupTarget;
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool ClosePopupTarget::init() {
    if (!CCSprite::initWithFile("bin.png"_spr)) return false;

    this->setScale(1.f);
    this->setOpacity(50);
    this->setColor({ 0, 0, 0 });

    m_gradient = cocos2d::CCLayerGradient::create({ 0, 0, 0, 0 }, { 0, 0, 0, 255 }, { 0.f, -1.f });
    m_gradient->setID("close-gradient");
    m_gradient->setZOrder(-1);
    m_gradient->setOpacity(40);
    m_gradient->setContentHeight(80.f);
    m_gradient->ignoreAnchorPointForPosition(false);
    this->addChildAtPosition(m_gradient, geode::Anchor::Center);

    return true;
}

bool ClosePopupTarget::pointIsCloseEnough(cocos2d::CCPoint pos) {
    return this->getPosition().getDistance(pos) < GEODE_DESKTOP(20.f) GEODE_MOBILE(40.f);
}

void ClosePopupTarget::hover() {
    this->stopAllActions();
    this->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCScaleTo::create(.2f, 1.1f)));
    this->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCFadeTo::create(.2f, 255)));
    this->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCTintTo::create(.2f, 255, 0, 0)));

    m_gradient->stopAllActions();
    m_gradient->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCFadeTo::create(.4f, 180)));
}

void ClosePopupTarget::unhover() {
    this->stopAllActions();
    this->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCScaleTo::create(.2f, 1.f)));
    this->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCFadeTo::create(.2f, 50)));
    this->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCTintTo::create(.2f, 0, 0, 0)));

    m_gradient->stopAllActions();
    m_gradient->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCFadeTo::create(.4f, 40)));
}
