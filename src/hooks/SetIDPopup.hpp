#pragma once
#include <Geode/modify/SetIDPopup.hpp>

class $modify(HookedSetIDPopup, SetIDPopup) {
    bool init(int current, int begin, int end, gd::string title, gd::string button, bool resetButton, int defaultValue, float offset, bool numberInput, bool arrows);
};
