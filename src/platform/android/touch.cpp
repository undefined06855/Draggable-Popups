#include "DraggablePopupManager.hpp"
#include <Geode/utils/AndroidEvent.hpp>

$on_mod(Loaded) {
    static std::unordered_map<int, cocos2d::CCPoint> s_touches;
    static float s_lastTouchDistance = 0.f;

    geode::AndroidRichInputEvent().listen([](int64_t timestamp, int deviceID, int eventSource, geode::AndroidRichInput data) {
        if (!std::holds_alternative<geode::AndroidTouchInput>(data)) return geode::ListenerResult::Propagate;
        auto touchData = std::get<geode::AndroidTouchInput>(data);
        auto winSize = cocos2d::CCDirector::get()->getWinSize();
        auto scaleFactorX = cocos2d::CCDirector::get()->getOpenGLView()->m_fScaleX;
        auto scaleFactorY = cocos2d::CCDirector::get()->getOpenGLView()->m_fScaleY;

        // add touches and update (called at beginning)
        auto addNecessaryTouches = [&] {
            if (touchData.type() == geode::AndroidTouchInput::Type::Began
             || touchData.type() == geode::AndroidTouchInput::Type::Moved) {
                for (auto touch : touchData.touches()) {
                    if (touch.x == 0.f && touch.y == 0.f) continue;
                    s_touches[touch.id] = cocos2d::CCPoint{ touch.x, touch.y } / cocos2d::CCPoint{ scaleFactorX, scaleFactorY };
                    s_touches[touch.id].y = winSize.height - s_touches[touch.id].y;
                }
           }
        };

        // remove touches (called at end)
        auto removeNecessaryTouches = [&] {
            if (touchData.type() == geode::AndroidTouchInput::Type::Ended
             || touchData.type() == geode::AndroidTouchInput::Type::Cancelled) {
                for (auto touch : touchData.touches()) {
                    if (touch.x == 0.f && touch.y == 0.f) continue;
                    s_touches.erase(touch.id);
                }
            }
        };

        addNecessaryTouches();

        if (s_touches.size() <= 1) {
            removeNecessaryTouches();
            return geode::ListenerResult::Propagate;
        }

        auto touchCenter = (s_touches[0] + s_touches[1]) / 2.f;
        auto touchDistance = s_touches[0].getDistance(s_touches[1]);

        bool result;
        switch (touchData.type()) {
            case geode::AndroidTouchInput::Type::Began: {
                if (s_touches.size() == 2) {
                    result = DraggablePopupManager::get().input(true, touchCenter);
                }
            } break;

            case geode::AndroidTouchInput::Type::Moved: {
                result = DraggablePopupManager::get().move(touchCenter);
                DraggablePopupManager::get().scroll(touchDistance / s_lastTouchDistance);
            } break;

            case geode::AndroidTouchInput::Type::Ended:
            case geode::AndroidTouchInput::Type::Cancelled: {
                if (s_touches.size() == 2) {
                    result = DraggablePopupManager::get().input(false, touchCenter);
                }
            } break;
        }

        s_lastTouchDistance = touchDistance;

        removeNecessaryTouches();

        return result;
    }, -10).leak();
}
