#include "DraggablePopupManager.hpp"

DraggablePopupManager::DraggablePopupManager()
    : m_layer(nullptr)
    , m_dragging(false)
    , m_dragStart(0.f, 0.f)
    , m_popupInitialPos(0.f, 0.f) {}

DraggablePopupManager& DraggablePopupManager::get() {
    static DraggablePopupManager instance;
    return instance;
}

geode::ListenerResult DraggablePopupManager::input(const geode::MouseInputData& event) {
    if (event.button != geode::MouseInputData::Button::Middle) {
        return geode::ListenerResult::Propagate;
    }

    // dragging but button released, stop
    if (event.action == geode::MouseInputData::Action::Release && m_dragging) {
        this->stopDrag();
        return geode::ListenerResult::Stop;
    }

    // not dragging but button held, start
    if (event.action == geode::MouseInputData::Action::Press && !m_dragging) {
        auto layers = this->findPopups(cocos2d::CCScene::get());
        if (layers.size() == 0) {
            return geode::ListenerResult::Propagate;
        }

        this->beginDragOn(layers.back());

        return geode::ListenerResult::Stop;
    }

    return geode::ListenerResult::Propagate;
}

geode::ListenerResult DraggablePopupManager::move() {
    if (!m_dragging) {
        return geode::ListenerResult::Propagate;
    }

    if (auto layer = m_layer.lock()) {
        // we have a valid layer, keep dragging
        layer->m_mainLayer->setPosition(m_popupInitialPos + (geode::cocos::getMousePos() - m_dragStart));
        return geode::ListenerResult::Stop;
    } else {
        // we have no valid layer, probably got destroyed idk
        this->stopDrag();
        return geode::ListenerResult::Propagate;
    }
}

std::vector<FLAlertLayer*> DraggablePopupManager::findPopups(cocos2d::CCNode* parent) {
    std::vector<FLAlertLayer*> ret;

    // this should in theory already be ordered by z order
    for (auto child : parent->getChildrenExt()) {
        if (auto cast = geode::cast::typeinfo_cast<FLAlertLayer*>(child)) {
            ret.push_back(cast);
        } else if (parent == cocos2d::CCScene::get()) { // else go one layer deeper max
            auto layers = this->findPopups(child);
            ret.insert(ret.end(), layers.begin(), layers.end());
        }
    }

    return ret;
}

cocos2d::extension::CCScale9Sprite* DraggablePopupManager::findPopupBackground(FLAlertLayer* popup) {
    if (auto nodeIDs = geode::cast::typeinfo_cast<cocos2d::extension::CCScale9Sprite*>(popup->m_mainLayer->getChildByID("background"))) return nodeIDs;
    return popup->m_mainLayer->getChildByType<cocos2d::extension::CCScale9Sprite*>(0);
}

void DraggablePopupManager::beginDragOn(FLAlertLayer* layer) {
    auto bg = this->findPopupBackground(layer);
    if (bg) {
        bg->runAction(cocos2d::CCFadeTo::create(.2f, 0));
    }

    m_dragging = true;
    m_layer = layer;
    m_dragStart = geode::cocos::getMousePos();

    if (!layer->getUserObject("initial-bg-opacity"_spr)) {
        layer->setUserObject("initial-bg-opacity"_spr, cocos2d::CCInteger::create(layer->getOpacity()));
    }
    
    layer->runAction(cocos2d::CCFadeTo::create(.2f, 50));

    m_popupInitialPos = layer->m_mainLayer->getPosition();
}

void DraggablePopupManager::stopDrag() {
    if (auto layer = m_layer.lock()) {
        auto bg = this->findPopupBackground(layer);
        if (bg) {
            bg->runAction(cocos2d::CCFadeTo::create(.2f, 255));
        }

        // if it's close enough to 0, 0 then snap it back and set the background opacity back
        if (layer->m_mainLayer->getPosition().getDistance({ 0.f, 0.f }) < 50.f) {
            layer->m_mainLayer->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCMoveTo::create(.2f, { 0.f, 0.f })));

            auto origOpacity = geode::cast::typeinfo_cast<cocos2d::CCInteger*>(layer->getUserObject("initial-bg-opacity"_spr));
            if (origOpacity) {
                layer->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCFadeTo::create(.2f, origOpacity->getValue())));
                layer->setUserObject("initial-bg-opacity"_spr, nullptr);
            }
        } else {
            layer->runAction(cocos2d::CCFadeTo::create(.2f, 0));
        }
    }

    m_layer = nullptr;
    m_dragging = false;
}

$on_mod(Loaded) {
    geode::MouseInputEvent().listen([](const geode::MouseInputData& event) {
        return DraggablePopupManager::get().input(event);
    }).leak();

    geode::MouseMoveEvent().listen([](int32_t x, int32_t y) {
        return DraggablePopupManager::get().move();
    }).leak();
}
