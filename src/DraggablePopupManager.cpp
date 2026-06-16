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

geode::ListenerResult DraggablePopupManager::input(const geode::MouseInputData& event) {
    static const std::unordered_map<std::string, geode::MouseInputData::Button> settingToButtonMap = {
        { "Left", geode::MouseInputData::Button::Left },
        { "Middle", geode::MouseInputData::Button::Middle },
        { "Right", geode::MouseInputData::Button::Right },
    };

    static const std::unordered_map<std::string, geode::KeyboardModifier> settingToModifierMap = {
        { "None", geode::KeyboardModifier::None },
        { "Ctrl", geode::KeyboardModifier::Control },
        { "Alt", geode::KeyboardModifier::Alt },
        { "Shift", geode::KeyboardModifier::Shift },
        { "Ctrl+Shift", geode::KeyboardModifier::Control | geode::KeyboardModifier::Shift },
        { "Ctrl+Alt", geode::KeyboardModifier::Control | geode::KeyboardModifier::Alt },
        { "Alt+Shift", geode::KeyboardModifier::Alt | geode::KeyboardModifier::Shift },
        { "Ctrl+Alt+Shift", geode::KeyboardModifier::Control | geode::KeyboardModifier::Alt | geode::KeyboardModifier::Shift },
    };

    if (settingToButtonMap.at(geode::Mod::get()->getSettingValue<std::string>("mouse-button")) != event.button) {
        return geode::ListenerResult::Propagate;
    }

    if (settingToModifierMap.at(geode::Mod::get()->getSettingValue<std::string>("modifier-keys")) != event.modifiers) {
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

        for (auto layer : layers | std::views::reverse) {
            if (layer->getUserFlag("undraggable-popup"_spr)) continue;
            this->beginDragOn(layer);
            break;
        }

        return geode::ListenerResult::Stop;
    }

    return geode::ListenerResult::Propagate;
}

geode::ListenerResult DraggablePopupManager::move() {
    if (!m_dragging) {
        return geode::ListenerResult::Propagate;
    }

    if (auto layer = m_layer.lock()) {
        layer->m_mainLayer->setPosition(m_popupInitialPos + (geode::cocos::getMousePos() - m_dragStart));
        return geode::ListenerResult::Stop;
    } else {
        this->stopDrag();
        return geode::ListenerResult::Propagate;
    }
}

geode::ListenerResult DraggablePopupManager::scroll(double y) {
    if (!m_dragging) {
        return geode::ListenerResult::Propagate;
    }

    // we don't care about this but also we do want to "handle" it
    if (y == 0.0) {
        return geode::ListenerResult::Stop;
    }

    if (auto layer = m_layer.lock()) {
        float scale = y < 0.0 ? .75f : 1.3333f;
        float finalScale = layer->m_mainLayer->getScale() * scale;
        if (finalScale < .25f || finalScale > 1.5f) {
            return geode::ListenerResult::Stop;
        }

        layer->m_mainLayer->setScale(finalScale);
        return geode::ListenerResult::Stop;
    } else {
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
    geode::log::trace("begin drag on {}", layer);

    m_dragging = true;
    m_layer = layer;
    m_dragStart = geode::cocos::getMousePos();
    m_popupInitialPos = layer->m_mainLayer->getPosition();

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
    m_popupRenderNode->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCFadeTo::create(.2f, 80)));
    layer->getParent()->addChild(m_popupRenderNode, layer->getZOrder());

    auto bg = this->findPopupBackground(layer);
    if (bg) {
        bg->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCScaleTo::create(.1f, 1.025f)));
    }

    layer->setVisible(false);

    layer->runAction(cocos2d::CCFadeTo::create(.2f, 0));
    if (!layer->getUserObject("initial-bg-opacity"_spr)) {
        layer->setUserObject("initial-bg-opacity"_spr, cocos2d::CCInteger::create(layer->getOpacity()));
    }

    if (!layer->getUserObject("initial-scale"_spr)) {
        layer->setUserObject("initial-scale"_spr, cocos2d::CCFloat::create(layer->m_mainLayer->getScale()));
    }
}

void DraggablePopupManager::stopDrag() {
    geode::log::trace("stop drag");

    if (auto layer = m_layer.lock()) {
        auto bg = this->findPopupBackground(layer);
        if (bg) {
            bg->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCScaleTo::create(.1f, 1.f)));
        }

        layer->setVisible(true);

        // if it's close enough to 0, 0 then snap it back and set the background opacity back
        // for a smaller scale we want a larger area to be able to snap it to
        if (layer->m_mainLayer->getPosition().getDistance({ 0.f, 0.f }) < 35.f * (1.f / layer->m_mainLayer->getScale())) {
            layer->m_mainLayer->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCMoveTo::create(.2f, { 0.f, 0.f })));

            auto origOpacity = geode::cast::typeinfo_cast<cocos2d::CCInteger*>(layer->getUserObject("initial-bg-opacity"_spr));
            if (origOpacity) {
                layer->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCFadeTo::create(.2f, origOpacity->getValue())));
                layer->setUserObject("initial-bg-opacity"_spr, nullptr);
            }

            auto origScale = geode::cast::typeinfo_cast<cocos2d::CCFloat*>(layer->getUserObject("initial-scale"_spr));
            if (origScale) {
                layer->m_mainLayer->runAction(cocos2d::CCEaseExponentialOut::create(cocos2d::CCScaleTo::create(.2f, origScale->getValue())));
                layer->setUserObject("initial-scale"_spr, nullptr);
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
    geode::MouseInputEvent().listen([](const geode::MouseInputData& event) {
        return DraggablePopupManager::get().input(event);
    }).leak();

    geode::ScrollWheelEvent().listen([](double x, double y) {
        return DraggablePopupManager::get().scroll(y);
    }).leak();

    geode::MouseMoveEvent().listen([](int32_t x, int32_t y) {
        return DraggablePopupManager::get().move();
    }).leak();

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
