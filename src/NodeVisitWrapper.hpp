#pragma once

class NodeVisitWrapper : public cocos2d::CCNode {
public:
    static NodeVisitWrapper* create(geode::Function<void()> callback);
    bool init(geode::Function<void()> callback);

    geode::Function<void()> m_callback;

    virtual void visit() override;
};
