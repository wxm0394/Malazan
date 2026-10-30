#pragma once
#include <cstdint>

namespace Malazan {

// 奥塔拉尔反魔场组件 (Otataral Anti-Magic Field Component)
// 原著设定：产自七城次大陆红荒原（Otataral Waste）的锈红矿石，吸附并扼杀一切法门波长。
struct OtataralComponent {
    float radius{3.5f};            // 压制力场半径（以实体为圆心）
    float deaden_intensity{1.0f};   // 压制效能 (1.0 = 100% 绝对灭魔)
    bool is_active{true};           // 刀刃出鞘 / 矿石共鸣状态
};

} // namespace Malazan
