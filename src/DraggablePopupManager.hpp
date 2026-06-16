#pragma once
#include "NodeVisitWrapper.hpp"
#include <alphalaneous.alphas-ui-pack/include/nodes/RenderNode.hpp>

class DraggablePopupManager {
    DraggablePopupManager();
public:
    static DraggablePopupManager& get();

    geode::WeakRef<FLAlertLayer> m_layer;
    bool m_dragging;
    cocos2d::CCPoint m_dragStart;
    cocos2d::CCPoint m_popupInitialPos;
    float m_popupInitialScale;

    alpha::ui::RenderNode* m_popupRenderNode;
    geode::Ref<NodeVisitWrapper> m_nodeVisitWrapper;

    geode::ListenerResult input(const geode::MouseInputData& event);
    geode::ListenerResult move();
    geode::ListenerResult scroll(double y);

    std::vector<FLAlertLayer*> findPopups(cocos2d::CCNode* parent);
    cocos2d::extension::CCScale9Sprite* findPopupBackground(FLAlertLayer* popup);
    void beginDragOn(FLAlertLayer* layer);
    void stopDrag();
};
