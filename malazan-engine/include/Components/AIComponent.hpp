#pragma once
#include "Core/Types.hpp"
#include <vector>

namespace Malazan {

enum class AIBehaviorState : uint8_t {
    Patrol,      // 常规警戒巡逻
    Suspicious,  // 听到异响/受击，调查疑似坐标
    Combat,      // 发现目标，全速接战并释放刺杀术
    Flee         // 重伤或被沃伦巨力震慑，撤退寻求增援
};

struct AIComponent {
    AIBehaviorState state{AIBehaviorState::Patrol};
    Vec2i target_interest_pos{0, 0}; // 当前巡逻/追踪的目标兴趣点
    int32_t sight_range{6};          // 基础视距
    int32_t hearing_range{8};        // 听觉范围（感知爆炸、屋顶跳跃声）
    int32_t attack_range{2};         // 攻击攻击距离（2格：毒飞刀/暗影弩）
    int32_t attack_power{28};        // 基础近战/暗器伤害
    float alert_meter{0.0f};         // 警觉槽 (0 ~ 100)

    std::vector<Vec2i> patrol_waypoints; // 预设巡逻路径节点
    size_t current_waypoint_idx{0};
};

} // namespace Malazan
