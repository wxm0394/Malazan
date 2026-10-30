#pragma once
#include "Core/Types.hpp"
#include <string>
#include <vector>
#include <cstdint>

namespace Malazan {

struct TrappedSoul {
    std::string name;
    Faction former_faction{Faction::NeutralCivilians};
    int32_t combat_power{0};
    float pull_effort{1.0f}; // 亡魂拉车劳力贡献值
};

// 提斯·安迪尔族黑夜之王阿诺曼德·雷克专属神器组件
// 德拉格尼普尔 (Dragnipur - 巨剑·纯暗大车之囚笼)
struct DragnipurComponent {
    std::vector<TrappedSoul> trapped_souls;
    float wagon_weight{1200.0f};         // 纯暗大车的古老自重
    float chaos_pursuit_distance{85.0f}; // 混沌风暴与巨车的当前安全距 (百分比)
    float terror_aura_radius{5.0f};      // 太古纯暗威压恐惧光环（敌方命中与士气骤降）
    bool is_drawn{true};                 // 黑剑出鞘状态（出鞘时引发全图阴影低语）

    [[nodiscard]] float get_total_pull_power() const {
        float total = 100.0f; // 基础拉力
        for (const auto& s : trapped_souls) {
            total += s.pull_effort * 15.0f;
        }
        return total;
    }
};

} // namespace Malazan
