#include "DraggablePopupManager.hpp"

$on_mod(Loaded) {
    // low priority so it runs before aup's touchdispatcher
    geode::MouseInputEvent().listen([](const geode::MouseInputData& event) {
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

        return DraggablePopupManager::get().input(event.action == geode::MouseInputData::Action::Press, geode::cocos::getMousePos());
    }, -10).leak();

    geode::ScrollWheelEvent().listen([](double x, double y) {
        if (y == 0.0) return geode::ListenerResult::Propagate;
        
        float scale = y < 0.0 ? .75f : 1.3333f;
        return DraggablePopupManager::get().scroll(y);
    }, -10).leak();

    geode::MouseMoveEvent().listen([](int32_t x, int32_t y) {
        return DraggablePopupManager::get().move(geode::cocos::getMousePos());
    }, -10).leak();
}
