#pragma once
#include "Core/Types.hpp"
#include <string>
#include <cstdint>

namespace Malazan {

// 贾格特上古暴君 (Jaghut Tyrant) 专属首领组件
struct JaghutTyrantComponent {
    bool is_empowered{true};           // 是否持有“精力果 (Finnest)”全力状态
    float omtose_blizzard_timer{0.0f}; // 上古坚冰 (Omtose Phellack) 暴风雪光环冷却
    int32_t frost_armor{80};           // 坚冰护甲值（吸收物理与普通魔法）
    int32_t aura_radius{4};            // 极度严寒光环波及半径
    bool is_sealed_in_azath{false};    // 是否已被阿扎斯之屋（Azath House）缠绕封印
};

// 沉睡的精力果 (The Finnest) 遗物组件
// 原著中 Raest 的肉身被封印，其绝大部分法力被剥离存放在精力果中，引来全城争夺
struct FinnestRelicComponent {
    Vec2i current_coord{10, 6};
    bool is_unsealed{false};
    float chaotic_attraction{100.0f}; // 对阿扎斯地脉与妖魔的引力值
};

} // namespace Malazan
