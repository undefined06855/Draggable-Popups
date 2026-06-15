#pragma once

class DraggablePopupManager {
    DraggablePopupManager();
public:
    static DraggablePopupManager& get();

    geode::WeakRef<FLAlertLayer> m_layer;
    bool m_dragging;
    cocos2d::CCPoint m_dragStart;
    cocos2d::CCPoint m_popupInitialPos;

    geode::ListenerResult input(const geode::MouseInputData& event);
    geode::ListenerResult move();

    std::vector<FLAlertLayer*> findPopups(cocos2d::CCNode* parent);
    cocos2d::extension::CCScale9Sprite* findPopupBackground(FLAlertLayer* popup);
    void beginDragOn(FLAlertLayer* layer);
    void stopDrag();
};
