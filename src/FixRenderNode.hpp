#pragma once
#include <alphalaneous.alphas-ui-pack/include/nodes/RenderNode.hpp>

class FixRenderNode : public alpha::ui::RenderNode {
public:
    static FixRenderNode* create(cocos2d::CCNode* node, bool constrain) {
        auto ret = new FixRenderNode;
        if (ret->init(node, constrain)) {
            ret->autorelease();
            ret->m_fakeChildren = cocos2d::CCArray::create();
            return ret;
        }

        delete ret;
        return nullptr;
    }

    geode::Ref<cocos2d::CCArray> m_fakeChildren;

    cocos2d::CCArray* getChildren() { return m_fakeChildren; }
    unsigned int getChildrenCount() { return 0; }
};
