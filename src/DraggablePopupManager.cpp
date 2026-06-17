#include "DraggablePopupManager.hpp"
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/utils/VMTHookManager.hpp>
#include <ranges>

DraggablePopupManager::DraggablePopupManager()
    : m_layer(nullptr)
    , m_dragging(false)
    , m_dragStart(0.f, 0.f)
    , m_popupInitialPos(0.f, 0.f)
    , m_popupRenderNode(nullptr)
    , m_popupInitialScale(0.f) {}

DraggablePopupManager& DraggablePopupManager::get() {
    static DraggablePopupManager instance;
    return instance;
}

bool DraggablePopupManager::input(bool down, cocos2d::CCPoint pos) {
    // dragging but button released, stop
    if (!down && m_dragging) {
        this->stopDrag();
        return geode::ListenerResult::Stop;
    }

    // not dragging but button held, start
    if (down && !m_dragging) {
        auto layers = this->findPopups(cocos2d::CCScene::get());
        if (layers.size() == 0) {
            return geode::ListenerResult::Propagate;
        }

        for (auto layer : layers | std::views::reverse) {
            if (layer->getUserFlag("undraggable-popup"_spr)) continue;
            this->beginDragOn(layer, pos);
            break;
        }

        return geode::ListenerResult::Stop;
    }

    return geode::ListenerResult::Propagate;
}

bool DraggablePopupManager::move(cocos2d::CCPoint pos) {
    if (!m_dragging) {
        return geode::ListenerResult::Propagate;
    }

    if (auto layer = m_layer.lock()) {
        layer->setPosition(m_popupInitialPos + (pos - m_dragStart));
        return geode::ListenerResult::Stop;
    } else {
        this->stopDrag();
        return geode::ListenerResult::Propagate;
    }
}

bool DraggablePopupManager::scroll(float scale) {
    if (!m_dragging) {
        return geode::ListenerResult::Propagate;
    }

    if (auto layer = m_layer.lock()) {
        float finalScale = layer->getScale() * scale;
        if (finalScale < .25f || finalScale > 1.f) {
            return geode::ListenerResult::Stop;
        }

        layer->setScale(finalScale);
        return geode::ListenerResult::Stop;
    } else {
        this->stopDrag();
        return geode::ListenerResult::Propagate;
    }
}

void DraggablePopupManager::stopAllAnimsOnPopup(FLAlertLayer* popup) {
    cocos2d::CCAction* action;
    while ((action = popup->getActionByTag(6855))) {
        action->update(1.f);
        popup->stopAction(action);
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

void DraggablePopupManager::beginDragOn(FLAlertLayer* layer, cocos2d::CCPoint pos) {
    geode::log::trace("begin drag on {}", layer);

    m_dragging = true;
    m_layer = layer;
    m_dragStart = pos;
    m_popupInitialPos = layer->getPosition();

    m_nodeVisitWrapper = NodeVisitWrapper::create(
        [this] {
            if (auto layer = m_layer.lock()) {
                layer->setVisible(true);
                layer->visit();
                layer->setVisible(false);
            }
        }
    );

    m_popupRenderNode = alpha::ui::RenderNode::create(m_nodeVisitWrapper, /* constrain */ false);
    m_popupRenderNode->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCFadeTo::create(.2f, 100)));
    layer->getParent()->addChild(m_popupRenderNode, layer->getZOrder());

    this->stopAllAnimsOnPopup(layer);
    auto scaleAnim = cocos2d::CCEaseExponentialOut::create(cocos2d::CCScaleBy::create(.1f, .95f));
    scaleAnim->setTag(6855);
    layer->runAction(scaleAnim);
    layer->setVisible(false);

    layer->runAction(cocos2d::CCFadeTo::create(.2f, 0));
    if (!layer->getUserObject("initial-bg-opacity"_spr)) {
        layer->setUserObject("initial-bg-opacity"_spr, cocos2d::CCInteger::create(layer->getOpacity()));
    }

    if (!layer->getUserObject("initial-scale"_spr)) {
        layer->setUserObject("initial-scale"_spr, cocos2d::CCFloat::create(layer->getScale()));
    }
}

void DraggablePopupManager::stopDrag() {
    geode::log::trace("stop drag");

    if (auto layer = m_layer.lock()) {
        this->stopAllAnimsOnPopup(layer);
        auto scaleAnim = cocos2d::CCEaseExponentialOut::create(cocos2d::CCScaleBy::create(.1f, 1.f / .95f));
        scaleAnim->setTag(6855);
        layer->runAction(scaleAnim);
        layer->setVisible(true);

        // if it's close enough to 0, 0 then snap it back and set the background opacity back
        // for a smaller scale we want a larger area to be able to snap it to
        if (layer->getPosition().getDistance({ 0.f, 0.f }) < 35.f * (1.f / layer->getScale())) {
            layer->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCMoveTo::create(.2f, { 0.f, 0.f })));

            auto origOpacity = geode::cast::typeinfo_cast<cocos2d::CCInteger*>(layer->getUserObject("initial-bg-opacity"_spr));
            if (origOpacity) {
                auto opacityAnim = cocos2d::CCEaseExponentialOut::create(cocos2d::CCFadeTo::create(.2f, origOpacity->getValue()));
                opacityAnim->setTag(6855);
                layer->runAction(opacityAnim);
                layer->setUserObject("initial-bg-opacity"_spr, nullptr);
            }

            auto origScale = geode::cast::typeinfo_cast<cocos2d::CCFloat*>(layer->getUserObject("initial-scale"_spr));
            if (origScale) {
                auto revertScaleAnim = cocos2d::CCEaseExponentialOut::create(cocos2d::CCScaleTo::create(.2f, origScale->getValue()));
                revertScaleAnim->setTag(6855);
                layer->runAction(revertScaleAnim);
                layer->setUserObject("initial-scale"_spr, nullptr);
            }
        }

        auto bg = layer->m_mainLayer->getChildByID("background");
        if (!bg) bg = layer->m_mainLayer->getChildByType<cocos2d::extension::CCScale9Sprite>(0);
        if (bg) {
            auto bottomLeft = layer->m_mainLayer->convertToWorldSpace(bg->getPosition() - bg->getScaledContentSize() / 2.f);
            auto topRight = layer->m_mainLayer->convertToWorldSpace(bg->getPosition() + bg->getScaledContentSize() / 2.f);
            auto bgRect = cocos2d::CCRect{ bottomLeft, topRight - bottomLeft };

            auto winSize = cocos2d::CCDirector::get()->getWinSize();
            auto winRect = cocos2d::CCRect{ { 40.f, 40.f }, winSize - 80.f };

            if (!bgRect.intersectsRect(winRect)) {
                float dx = 0.f;
                float dy = 0.f;

                if (bgRect.getMinX() > winRect.getMaxX()) dx = winRect.getMaxX() - bgRect.getMinX();
                else if (bgRect.getMaxX() < winRect.getMinX()) dx = winRect.getMinX() - bgRect.getMaxX();

                if (bgRect.getMinY() > winRect.getMaxY()) dy = winRect.getMaxY() - bgRect.getMinY();
                else if (bgRect.getMaxY() < winRect.getMinY()) dy = winRect.getMinY() - bgRect.getMaxY();

                auto adjustAnim = cocos2d::CCEaseExponentialOut::create(cocos2d::CCMoveBy::create(.2f, { dx, dy }));
                adjustAnim->setTag(6855);
                layer->runAction(adjustAnim);
            }
        }
    }

    m_popupRenderNode->removeFromParent();
    m_popupRenderNode = nullptr;
    m_nodeVisitWrapper = nullptr;
    m_layer = nullptr;

    m_dragging = false;
}

$on_mod(Loaded) {
    static bool s_queuedThisFrame = false;
    geode::listenForAllSettingChanges([&](std::string_view, std::shared_ptr<geode::SettingV3>) {
        if (s_queuedThisFrame) return;
        s_queuedThisFrame = true;

        geode::Loader::get()->queueInMainThread([&] {
            s_queuedThisFrame = false;

            if (
                geode::Mod::get()->getSettingValue<std::string>("mouse-button") == "Left"
             && geode::Mod::get()->getSettingValue<std::string>("modifier-keys") == "None") {
                geode::createQuickPopup(
                    "Draggable Popups",
                    "Hey! You <cr>can't</c> use the <cy>Left</c> mouse button with\n"
                    "<co>no modifier keys!</c> <cj>Sorry!</c>",
                    "Ok",
                    nullptr,
                    [](FLAlertLayer*, bool) {
                        geode::Mod::get()->setSettingValue<std::string>("mouse-button", "Right");
                        geode::openSettingsPopup(geode::Mod::get(), false);
                    }
                );
            }
        });
    });
}
