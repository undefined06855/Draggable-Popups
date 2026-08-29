#pragma once
#include "NodeVisitWrapper.hpp"
#include "ClosePopupTarget.hpp"
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

    ClosePopupTarget* m_closeTarget;
    bool m_hoveringCloseTarget;

    alpha::ui::RenderNode* m_popupRenderNode;
    geode::Ref<NodeVisitWrapper> m_nodeVisitWrapper;

    bool input(bool down, cocos2d::CCPoint pos);
    bool move(cocos2d::CCPoint pos);
    bool scroll(float scale);

    void stopAllAnimsOnPopup(FLAlertLayer* popup);

    std::vector<FLAlertLayer*> findPopups(cocos2d::CCNode* parent);
    void beginDragOn(FLAlertLayer* layer, cocos2d::CCPoint pos);
    void stopDrag();
};
