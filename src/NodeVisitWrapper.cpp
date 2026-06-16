#include "NodeVisitWrapper.hpp"

NodeVisitWrapper* NodeVisitWrapper::create(geode::Function<void()> callback) {
    auto ret = new NodeVisitWrapper;
    if (ret->init(std::move(callback))) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool NodeVisitWrapper::init(geode::Function<void()> callback) {
    if (!CCNode::init()) return false;

    m_callback = std::move(callback);

    return true;
}

void NodeVisitWrapper::visit() {
    m_callback();
}
